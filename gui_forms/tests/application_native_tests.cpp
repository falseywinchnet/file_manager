#include "gui_forms/application.hpp"
#include "gui_forms/control.hpp"
#include "gui_forms/controls/panel/text_box/text_box.hpp"

#include <iostream>
#include <stdexcept>
#include <thread>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {
using namespace gui_forms;
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
std::unique_ptr<Window> make_window() {
    return std::make_unique<Window>(make_control<Control>(StableId("native.application.root")), Size{360, 240});
}
struct Probe final {
    ApplicationWindowHandle handle;
    unsigned ready{};
    unsigned closing{};
    unsigned closed{};
    bool fail{};
    bool closed_active{};
    bool attached_services{};
    bool primary_hide_rejected{};
    bool recursive_rejected{};
    bool system_close{};
};
struct Ready final {
    Probe* probe{};
    void operator()(Window& window, ApplicationWindowHandle handle) const {
        (*probe).handle = handle;
        ++(*probe).ready;
        (*probe).attached_services = window.host_services() != nullptr;
        (*probe).primary_hide_rejected = handle.hide().error == HostServiceError::invalid_argument;
        (*probe).recursive_rejected = Application::run(make_window()).error == ApplicationError::already_running;
        if ((*probe).fail) throw std::runtime_error("intentional ready failure");
#if defined(_WIN32)
        if ((*probe).system_close) {
            const HWND native = FindWindowW(L"GUIForms.Window.v1", L"GUI.Forms portable application contract");
            require(native != nullptr && PostMessageW(native, WM_SYSKEYDOWN, VK_F4,
                static_cast<LPARAM>(1ULL << 29U)) != 0, "native close gesture was not posted");
            return;
        }
#endif
        require(handle.active() && handle.request_close().accepted(), "ready close request failed");
    }
};
struct Closing final {
    Probe* probe{};
    void operator()(HostCloseRequest& request) const {
        ++(*probe).closing;
        if ((*probe).closing == 1U) {
            request.cancel = true;
            require((*probe).handle.request_close().accepted(), "second close request failed");
        }
    }
};
struct Closed final {
    Probe* probe{};
    void operator()() const {
        ++(*probe).closed;
        (*probe).closed_active = (*probe).handle.active();
    }
};
ApplicationResult run_probe(Probe& probe) {
    ApplicationWindowOptions options;
    options.title = "GUI.Forms portable application contract";
    options.initial_size = {360, 240};
    options.ready = Ready{&probe};
    options.closing = Closing{&probe};
    options.closed = Closed{&probe};
    return Application::run(make_window(), std::move(options));
}
struct DispatchProbe final {
    std::function<void()> wake;
    ApplicationWindowHandle handle;
    std::thread::id ui_thread{std::this_thread::get_id()};
    unsigned dispatched{};
    bool fail{};
};
struct WakeReady final {
    DispatchProbe* probe{};
    void operator()(std::function<void()> wake) const { (*probe).wake = std::move(wake); }
};
struct DispatchReady final {
    DispatchProbe* probe{};
    void operator()(Window&, ApplicationWindowHandle handle) const {
        require(static_cast<bool>((*probe).wake), "wake was not published before ready");
        (*probe).handle = handle;
        // Exercise the worker-to-host boundary without retaining a worker after
        // the window closes. The callback itself must execute on the UI thread.
        std::thread worker((*probe).wake);
        worker.join();
    }
};
struct DrainPending final {
    DispatchProbe* probe{};
    void operator()() const {
        if (!(*probe).handle.active()) return;
        require(std::this_thread::get_id() == (*probe).ui_thread, "dispatch ran off the UI thread");
        ++(*probe).dispatched;
        if ((*probe).fail) throw std::runtime_error("intentional dispatch failure");
        require((*probe).handle.request_close().accepted(), "dispatch close failed");
    }
};
ApplicationResult run_dispatch_probe(DispatchProbe& probe) {
    ApplicationWindowOptions options;
    options.title = "GUI.Forms portable worker dispatch";
    options.initial_size = {360, 240};
    options.wake_ready = WakeReady{&probe};
    options.ready = DispatchReady{&probe};
    options.dispatch_pending = DrainPending{&probe};
    return Application::run(make_window(), std::move(options));
}
#if defined(_WIN32)
struct CharacterReady final {
    std::shared_ptr<TextBox> editor;
    void operator()(Window& window, ApplicationWindowHandle handle) const {
        require(window.request_focus(editor), "character fixture focus failed");
        const HWND native = FindWindowW(L"GUIForms.Window.v1", L"GUI.Forms character normalization fixture");
        require(native != nullptr, "character fixture window missing");
        DWORD process_id = 0;
        GetWindowThreadProcessId(native, &process_id);
        require(process_id == GetCurrentProcessId(), "character fixture belongs to another process");
        // Messages target only this fixture. Command-generated WM_CHAR values
        // must not enter the text store; ordinary UTF-16 text still must.
        for (WPARAM character = 0; character < 0x20; ++character) {
            PostMessageW(native, WM_CHAR, character, 0);
        }
        PostMessageW(native, WM_CHAR, 0x7F, 0);
        PostMessageW(native, WM_CHAR, L'A', 0);
        PostMessageW(native, WM_CHAR, 0xD83D, 0);
        PostMessageW(native, WM_CHAR, 0xDE00, 0);
        PostMessageW(native, WM_CHAR, L'Z', 0);
        require(handle.request_close().accepted(), "character fixture close failed");
    }
};
void check_character_normalization() {
    const std::shared_ptr<TextBox> editor = make_control<TextBox>(StableId("native.character.editor"));
    ApplicationWindowOptions options;
    options.title = "GUI.Forms character normalization fixture";
    options.initial_size = {360, 240};
    options.ready = CharacterReady{editor};
    const ApplicationResult result = Application::run(std::make_unique<Window>(editor, Size{360, 240}), std::move(options));
    if (result.callback_exception) std::rethrow_exception(result.callback_exception);
    require(result.accepted(), "native character normalization fixture failed");
        if ((*editor).text() != "A\xF0\x9F\x98\x80Z") {
            std::cerr << "character fixture byte count=" << (*editor).text().size() << " bytes:";
            for (const unsigned char byte : (*editor).text()) std::cerr << ' ' << static_cast<unsigned>(byte);
            std::cerr << '\n';
            throw std::runtime_error("native text normalization changed text or inserted command bytes");
        }

}
#endif
}
int main() {
#if defined(_WIN32)
    check_character_normalization();
    Probe system_close;
    system_close.system_close = true;
    require(run_probe(system_close).accepted() && system_close.closing == 2U && system_close.closed == 1U,
            "Alt+F4 must reach ordinary close cancellation and teardown");
#endif
    Probe normal;
    const ApplicationResult result = run_probe(normal);
    require(result.accepted(), "native application run failed");
    require(normal.ready == 1U && normal.closing == 2U && normal.closed == 1U, "native close/cancel order wrong");
    require(normal.attached_services && normal.primary_hide_rejected && normal.recursive_rejected, "ready contract incomplete");
    require(!normal.closed_active && !normal.handle.active() && normal.handle.show().error == HostServiceError::after_shutdown,
            "native window handle survived close");
    Probe failure;
    failure.fail = true;
    const ApplicationResult failed = run_probe(failure);
    require(failed.error == ApplicationError::callback_failure && failed.callback_exception && failure.closed == 1U,
            "callback failure escaped or left native window open");
    bool observed = false;
    try { std::rethrow_exception(failed.callback_exception); }
    catch (const std::runtime_error& error) { observed = std::string(error.what()) == "intentional ready failure"; }
    require(observed, "callback exception identity lost");
    DispatchProbe dispatch;
    require(run_dispatch_probe(dispatch).accepted() && dispatch.dispatched > 0U,
            "worker wake did not dispatch UI work");
    DispatchProbe dispatch_failure;
    dispatch_failure.fail = true;
    const ApplicationResult failed_dispatch = run_dispatch_probe(dispatch_failure);
    require(failed_dispatch.error == ApplicationError::callback_failure && failed_dispatch.callback_exception,
            "dispatch exception escaped the portable application boundary");
    std::cout << "portable application API: native attach, cancel/retry, close, expiry, recursion and exception containment passed\n";
}
