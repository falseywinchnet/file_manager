#include "windows_message_queue.hpp"

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

namespace {
using Clock = std::chrono::steady_clock;
constexpr wchar_t class_name[] = L"GUIForms.QueueOrder.NativeProbe";
constexpr UINT render_wake = WM_APP + 0x42U;

void require(const bool condition, const char* const message) {
    if (!condition) throw std::runtime_error(message);
}

class NativeWindow final {
public:
    NativeWindow() = default;
    ~NativeWindow() {
        if (window != nullptr) {
            const bool still_foreground = GetForegroundWindow() == window;
            DestroyWindow(window);
            if (still_foreground && IsWindow(previous_foreground)) SetForegroundWindow(previous_foreground);
        }
        if (registered != 0U) UnregisterClassW(class_name, instance);
    }
    NativeWindow(const NativeWindow&) = delete;
    NativeWindow& operator=(const NativeWindow&) = delete;
    HINSTANCE instance{GetModuleHandleW(nullptr)};
    HWND previous_foreground{GetForegroundWindow()};
    HWND window{nullptr};
    ATOM registered{0U};
};

struct InjectedKey final {
    WORD code{0U};
    bool admitted{false};
    bool release_needed{false};
    bool release_seen{false};
};

class InjectedKeys final {
public:
    // Borrows the native window; declare after its owner so cleanup precedes
    // window destruction and any restoration of the previous foreground.
    explicit InjectedKeys(const HWND window) : window_(window) {}
    ~InjectedKeys() {
        for (InjectedKey& key : keys_) {
            if (!key.release_needed) continue;
            INPUT release{};
            release.type = INPUT_KEYBOARD;
            release.ki.wVk = key.code;
            release.ki.dwFlags = KEYEVENTF_KEYUP;
            if (SendInput(1U, &release, static_cast<int>(sizeof(INPUT))) != 1U) {
                std::fprintf(stderr, "native input cleanup failed: key-up injection %u\n",
                             static_cast<unsigned int>(key.code));
            } else {
                key.release_needed = false;
            }
        }
        const Clock::time_point deadline = Clock::now() + std::chrono::seconds(2);
        while (pending_release() && Clock::now() < deadline) {
            MSG message{};
            if (PeekMessageW(&message, window_, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)) {
                observe(message);
            } else {
                Sleep(1U);
            }
        }
        if (pending_release()) {
            std::fprintf(stderr, "native input cleanup incomplete: release not observed in probe window\n");
        }
    }
    InjectedKeys(const InjectedKeys&) = delete;
    InjectedKeys& operator=(const InjectedKeys&) = delete;

    void record(const WORD code, const UINT accepted) noexcept {
        for (InjectedKey& key : keys_) {
            if (key.code == code) {
                key.admitted = accepted > 0U;
                key.release_needed = accepted == 1U;
            }
        }
    }
    void observe(const MSG& message) noexcept {
        if (message.hwnd != window_ || message.message != WM_KEYUP) return;
        for (InjectedKey& key : keys_) {
            if (message.wParam == key.code && key.admitted) key.release_seen = true;
        }
    }
    [[nodiscard]] bool complete() const noexcept {
        for (const InjectedKey& key : keys_) {
            if (!key.admitted || key.release_needed || !key.release_seen) return false;
        }
        return true;
    }
private:
    [[nodiscard]] bool pending_release() const noexcept {
        for (const InjectedKey& key : keys_) {
            if (key.admitted && !key.release_seen) return true;
        }
        return false;
    }
    HWND window_{nullptr};
    std::array<InjectedKey, 2> keys_{{
        {static_cast<WORD>('A'), false, false, false},
        {static_cast<WORD>(VK_BACK), false, false, false}}};
};

void prepare_window(NativeWindow& owner) {
    WNDCLASSW type{};
    type.lpfnWndProc = DefWindowProcW;
    type.hInstance = owner.instance;
    type.lpszClassName = class_name;
    owner.registered = RegisterClassW(&type);
    require(owner.registered != 0U, "cannot register probe window");
    owner.window = CreateWindowExW(WS_EX_TOOLWINDOW, class_name,
        L"File Manager input-order probe", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 420, 160, nullptr, nullptr, owner.instance, nullptr);
    require(owner.window != nullptr, "cannot create probe window");
    ShowWindow(owner.window, SW_SHOWNORMAL);
    SetForegroundWindow(owner.window);
    SetFocus(owner.window);
    UpdateWindow(owner.window);
    MSG pending{};
    for (std::size_t count = 0U; count < 256U && PeekMessageW(&pending, nullptr, 0U, 0U, PM_REMOVE); ++count) {
        DispatchMessageW(&pending);
    }
    require(GetForegroundWindow() == owner.window && GetFocus() == owner.window,
            "foreground ownership unavailable; no input injected");
}

void inject_key(const NativeWindow& owner, InjectedKeys& keys, const WORD key_code) {
    constexpr std::array<int, 7> modifiers{VK_SHIFT, VK_CONTROL, VK_MENU, VK_LWIN, VK_RWIN, 'A', VK_BACK};
    for (const int key : modifiers) {
        require((GetAsyncKeyState(key) & 0x8000) == 0, "a relevant key is held; no input injected");
    }
    std::array<INPUT, 2> input{};
    for (std::size_t index = 0U; index < input.size(); ++index) {
        input[index].type = INPUT_KEYBOARD;
        input[index].ki.wVk = key_code;
        input[index].ki.dwFlags = index % 2U == 0U ? 0U : KEYEVENTF_KEYUP;
    }
    require(GetForegroundWindow() == owner.window && GetFocus() == owner.window,
            "foreground changed; no input injected");
    const UINT inserted = SendInput(static_cast<UINT>(input.size()), input.data(), static_cast<int>(sizeof(INPUT)));
    keys.record(key_code, inserted);
    require(inserted == input.size(), "native input injection was incomplete");
}

[[nodiscard]] MSG take_key(const NativeWindow& owner, const UINT kind) {
    const Clock::time_point deadline = Clock::now() + std::chrono::seconds(2);
    MSG message{};
    while (Clock::now() < deadline) {
        require(GetForegroundWindow() == owner.window, "foreground changed during native probe");
        if (PeekMessageW(&message, nullptr, kind, kind, PM_REMOVE)) return message;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    const DWORD categories = GetQueueStatus(QS_ALLINPUT);
    MSG pending{};
    const BOOL has_key = PeekMessageW(&pending, nullptr, WM_KEYFIRST, WM_KEYLAST, PM_NOREMOVE);
    std::cerr << "native input timeout|requested=" << kind << "|queue_status=" << categories
              << "|key_available=" << has_key << "|key_message=" << pending.message
              << "|key_value=" << pending.wParam << '\n';
    throw std::runtime_error("native queue did not receive the injected key");
}

// Exact category-first retrieval of the rejected sibling patch, kept only as
// this explicit negative control. The production helper is range-first.
[[nodiscard]] bool take_rejected(MSG& message, const bool input_first) {
    if (input_first && PeekMessageW(&message, nullptr, 0U, 0U, PM_REMOVE | PM_QS_INPUT)) return true;
    if (input_first && PeekMessageW(&message, nullptr, WM_KEYFIRST, WM_MOUSELAST, PM_REMOVE)) return true;
    const BOOL found = PeekMessageW(&message, nullptr, 0U, 0U, PM_REMOVE);
    const bool available = found != FALSE;
    return available;
}

void run_case(const bool rejected_control) {
    NativeWindow owner{};
    prepare_window(owner);
    InjectedKeys keys(owner.window);
    for (WPARAM index = 0U; index < 64U; ++index) {
        require(PostThreadMessageW(GetCurrentThreadId(), render_wake, index, 0) != FALSE,
                "cannot prepare native render-wake backlog");
    }
    inject_key(owner, keys, static_cast<WORD>('A'));
    const MSG letter = take_key(owner, WM_KEYDOWN);
    require(letter.hwnd == owner.window && letter.wParam == 'A', "unexpected first hardware key");
    const MSG letter_up = take_key(owner, WM_KEYUP);
    keys.observe(letter_up);
    require(letter_up.wParam == 'A', "unexpected first hardware key release");
    require(TranslateMessage(&letter) != FALSE, "letter translation failed");
    inject_key(owner, keys, static_cast<WORD>(VK_BACK));

    // Translation has now posted a character behind the 64 private wakes,
    // while the later Backspace remains in the real native input category.
    MSG translated{};
    require(PeekMessageW(&translated, nullptr, WM_CHAR, WM_CHAR, PM_NOREMOVE) != FALSE &&
            (translated.wParam == 'a' || translated.wParam == 'A'),
            "keyboard layout did not produce the required Latin character");
    std::wstring text{};
    text.reserve(4U);
    UINT first_message{0U};
    std::size_t ordinary_count{0U};
    std::size_t characters{0U};
    std::size_t backspaces{0U};
    const Clock::time_point deadline = Clock::now() + std::chrono::seconds(2);
    for (std::size_t turn = 0U; turn < 512U; ++turn) {
        require(GetForegroundWindow() == owner.window, "foreground changed during retrieval");
        MSG message{};
        const bool available = rejected_control ? take_rejected(message, turn % 2U == 0U)
            : gui_forms::host::detail::take_window_message(message, turn % 2U == 0U);
        if (!available) {
            if (ordinary_count == 64U && characters == 1U && backspaces == 1U && keys.complete()) break;
            require(Clock::now() < deadline, "native message drain timed out");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        if (first_message == 0U) first_message = message.message;
        keys.observe(message);
        if (message.message == render_wake) ++ordinary_count;
        if (message.message == WM_KEYDOWN && message.wParam == VK_BACK) {
            ++backspaces;
            if (!text.empty()) text.pop_back();
        }
        if (message.message == WM_CHAR && (message.wParam == 'a' || message.wParam == 'A')) {
            ++characters;
            text.push_back(static_cast<wchar_t>(message.wParam));
        }
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    require(ordinary_count == 64U && characters == 1U && backspaces == 1U && keys.complete(),
            "native control did not consume the expected messages");
    std::cout << "native-queue|order=" << (rejected_control ? "category-first" : "range-first")
              << "|first_message=" << first_message << "|remaining_text_units=" << text.size()
              << "|private_wakes=" << ordinary_count << "|characters=" << characters
              << "|backspaces=" << backspaces << "|both_keyups_observed=yes\n";
    require(rejected_control ? first_message == WM_KEYDOWN && text.size() == 1U
                             : first_message == WM_CHAR && text.empty(),
            "native ordering differs from the declared control");
}
} // namespace

int main(const int count, char** const arguments) {
    try {
        require(count == 2, "explicit --native or --rejected-control is required");
        const std::string_view mode(arguments[1]);
        require(mode == "--native" || mode == "--rejected-control", "unknown native probe mode");
        run_case(mode == "--rejected-control");
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
