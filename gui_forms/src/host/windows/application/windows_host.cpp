#include "windows_host.hpp"

#include "gui_forms/live_surface.hpp"
#include "gui_forms/text.hpp"
#include "gui_forms/detail/bound_member_function.hpp"

#include "../paint_endpoint/windows_compatibility_paint_endpoint_metrics.hpp"

#include <windows.h>
#include <windowsx.h>
#include <wincodec.h>
#include <commdlg.h>
#include <shlobj.h>
#include <usp10.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <exception>
#include <iomanip>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace gui_forms::host {
namespace {

constexpr wchar_t window_class_name[] = L"GUIForms.Window.v1";
constexpr wchar_t tooltip_class_name[] = L"GUIForms.ToolTip.v1";
constexpr UINT_PTR scheduler_timer = 1;
constexpr UINT_PTR modal_live_surface_timer = 2;
constexpr UINT live_surface_period_milliseconds = 16;
constexpr UINT managed_dispatch_message = WM_APP + 0x41U;
constexpr UINT render_dispatch_message = WM_APP + 0x42U;
constexpr std::size_t maximum_automation_command = 4096;

bool environment_flag(const char* name) noexcept {
    const char* value = std::getenv(name);
    return value != nullptr && std::strcmp(value, "1") == 0;
}

bool trace_win32_input() noexcept {
    static const bool enabled =
        environment_flag("GUI_FORMS_TRACE_WIN32_INPUT");
    return enabled;
}

bool trace_win32_text() noexcept {
    static const bool enabled =
        environment_flag("GUI_FORMS_TRACE_WIN32_TEXT");
    return enabled;
}

std::uint64_t now_nanoseconds() noexcept {
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
}

std::wstring wide_from_utf8(std::string_view text) {
    if (text.empty()) return {};
    if (text.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) return {};
    const int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
                                          static_cast<int>(text.size()), nullptr, 0);
    if (count <= 0) return {};
    std::wstring result(static_cast<std::size_t>(count), L'\0');
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
                            static_cast<int>(text.size()), result.data(), count) != count) {
        return {};
    }
    return result;
}

std::string utf8_from_wide(std::wstring_view text) {
    if (text.empty()) return {};
    if (text.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) return {};
    const int count = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text.data(),
                                          static_cast<int>(text.size()), nullptr, 0,
                                          nullptr, nullptr);
    if (count <= 0) return {};
    std::string result(static_cast<std::size_t>(count), '\0');
    if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text.data(),
                            static_cast<int>(text.size()), result.data(), count,
                            nullptr, nullptr) != count) {
        return {};
    }
    return result;
}

std::wstring native_filter(const std::vector<HostFileDialogFilter>& filters) {
    std::wstring result;
    for (const gui_forms::HostFileDialogFilter& filter : filters) {
        std::wstring label = wide_from_utf8(filter.label);
        if (label.empty()) label = L"Files";
        std::wstring pattern;
        for (const std::basic_string<char>& extension_utf8 : filter.extensions) {
            std::wstring extension = wide_from_utf8(extension_utf8);
            while (!extension.empty() && (extension.front() == L'.' ||
                                           extension.front() == L'*')) {
                extension.erase(extension.begin());
            }
            if (extension.empty()) continue;
            if (!pattern.empty()) pattern.push_back(L';');
            pattern.append(L"*.").append(extension);
        }
        if (pattern.empty()) pattern = L"*.*";
        result.append(label).push_back(L'\0');
        result.append(pattern).push_back(L'\0');
    }
    if (result.empty()) {
        result.append(L"All files").push_back(L'\0');
        result.append(L"*.*").push_back(L'\0');
    }
    result.push_back(L'\0');
    return result;
}

HostDialogResult path_dialog_failure(std::uint64_t request_id,
                                     HostServiceError error) {
    return {{error}, request_id, HostPathDialogResult{}};
}

HostDialogResult native_open_dialog(HWND owner, std::uint64_t request_id,
                                    const HostOpenFileDialogRequest& request) {
    std::vector<wchar_t> path(65536, L'\0');
    const std::wstring title = wide_from_utf8(request.title);
    const std::wstring directory = wide_from_utf8(request.initial_directory);
    const std::wstring suggested = wide_from_utf8(request.suggested_name);
    const std::wstring filter = native_filter(request.filters);
    if (!suggested.empty()) {
        const std::size_t count = std::min(suggested.size(), path.size() - 2U);
        std::copy_n(suggested.data(), count, path.data());
    }
    OPENFILENAMEW native{};
    native.lStructSize = sizeof(native);
    native.hwndOwner = owner;
    native.lpstrFilter = filter.c_str();
    native.lpstrFile = path.data();
    native.nMaxFile = static_cast<DWORD>(path.size());
    native.lpstrInitialDir = directory.empty() ? nullptr : directory.c_str();
    native.lpstrTitle = title.empty() ? nullptr : title.c_str();
    native.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST |
                   OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
    if (request.allow_multiple) native.Flags |= OFN_ALLOWMULTISELECT;
    if (!GetOpenFileNameW(&native)) {
        const DWORD error = CommDlgExtendedError();
        return error == 0 ? HostDialogResult{{}, request_id,
                    HostPathDialogResult{HostDialogOutcome::cancelled, {}}}
            : path_dialog_failure(request_id, HostServiceError::backend_failure);
    }
    HostPathDialogResult value;
    value.outcome = HostDialogOutcome::accepted;
    const std::wstring first(path.data());
    const wchar_t* cursor = path.data() + first.size() + 1U;
    if (*cursor == L'\0') {
        value.paths.push_back(utf8_from_wide(first));
    } else {
        for (std::size_t count = 0; *cursor != L'\0' &&
             count < HostServices::maximum_dialog_paths; ++count) {
            const std::wstring name(cursor);
            std::wstring combined = first;
            if (!combined.empty() && combined.back() != L'\\' &&
                combined.back() != L'/') combined.push_back(L'\\');
            combined.append(name);
            value.paths.push_back(utf8_from_wide(combined));
            cursor += name.size() + 1U;
        }
    }
    return {{}, request_id, std::move(value)};
}

HostDialogResult native_save_dialog(HWND owner, std::uint64_t request_id,
                                    const HostSaveFileDialogRequest& request) {
    std::vector<wchar_t> path(32768, L'\0');
    const std::wstring title = wide_from_utf8(request.title);
    const std::wstring directory = wide_from_utf8(request.initial_directory);
    const std::wstring suggested = wide_from_utf8(request.suggested_name);
    const std::wstring extension = wide_from_utf8(request.default_extension);
    const std::wstring filter = native_filter(request.filters);
    if (!suggested.empty()) {
        const std::size_t count = std::min(suggested.size(), path.size() - 2U);
        std::copy_n(suggested.data(), count, path.data());
    }
    OPENFILENAMEW native{};
    native.lStructSize = sizeof(native);
    native.hwndOwner = owner;
    native.lpstrFilter = filter.c_str();
    native.lpstrFile = path.data();
    native.nMaxFile = static_cast<DWORD>(path.size());
    native.lpstrInitialDir = directory.empty() ? nullptr : directory.c_str();
    native.lpstrTitle = title.empty() ? nullptr : title.c_str();
    native.lpstrDefExt = extension.empty() ? nullptr : extension.c_str();
    native.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
                   OFN_NOCHANGEDIR;
    if (request.confirm_overwrite) native.Flags |= OFN_OVERWRITEPROMPT;
    if (!GetSaveFileNameW(&native)) {
        const DWORD error = CommDlgExtendedError();
        return error == 0 ? HostDialogResult{{}, request_id,
                    HostPathDialogResult{HostDialogOutcome::cancelled, {}}}
            : path_dialog_failure(request_id, HostServiceError::backend_failure);
    }
    HostPathDialogResult value;
    value.outcome = HostDialogOutcome::accepted;
    value.paths.push_back(utf8_from_wide(path.data()));
    return {{}, request_id, std::move(value)};
}

int CALLBACK select_initial_folder(HWND dialog, UINT message, LPARAM,
                                   LPARAM context) {
    if (message == BFFM_INITIALIZED && context != 0) {
        SendMessageW(dialog, BFFM_SETSELECTIONW, TRUE, context);
    }
    return 0;
}

HostDialogResult native_folder_dialog(HWND owner, std::uint64_t request_id,
                                      const HostFolderDialogRequest& request) {
    const std::wstring title = wide_from_utf8(request.title);
    const std::wstring directory = wide_from_utf8(request.initial_directory);
    BROWSEINFOW native{};
    native.hwndOwner = owner;
    native.lpszTitle = title.empty() ? nullptr : title.c_str();
    native.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE |
                     BIF_EDITBOX | BIF_VALIDATE;
    native.lpfn = select_initial_folder;
    native.lParam = directory.empty() ? 0 :
        reinterpret_cast<LPARAM>(directory.c_str());
    PIDLIST_ABSOLUTE selected = SHBrowseForFolderW(&native);
    if (selected == nullptr) {
        return {{}, request_id,
                HostPathDialogResult{HostDialogOutcome::cancelled, {}}};
    }
    std::vector<wchar_t> path(32768, L'\0');
    const bool converted = SHGetPathFromIDListW(selected, path.data()) != FALSE;
    CoTaskMemFree(selected);
    if (!converted) {
        return path_dialog_failure(request_id, HostServiceError::backend_failure);
    }
    HostPathDialogResult value;
    value.outcome = HostDialogOutcome::accepted;
    value.paths.push_back(utf8_from_wide(path.data()));
    return {{}, request_id, std::move(value)};
}

UINT native_message_style(const HostMessageDialogRequest& request) noexcept {
    UINT style = MB_APPLMODAL;
    switch (request.buttons) {
    case HostMessageButtons::ok: style |= MB_OK; break;
    case HostMessageButtons::ok_cancel: style |= MB_OKCANCEL; break;
    case HostMessageButtons::yes_no: style |= MB_YESNO; break;
    case HostMessageButtons::yes_no_cancel: style |= MB_YESNOCANCEL; break;
    case HostMessageButtons::retry_cancel: style |= MB_RETRYCANCEL; break;
    }
    switch (request.icon) {
    case HostMessageIcon::information: style |= MB_ICONINFORMATION; break;
    case HostMessageIcon::warning: style |= MB_ICONWARNING; break;
    case HostMessageIcon::error: style |= MB_ICONERROR; break;
    case HostMessageIcon::question: style |= MB_ICONQUESTION; break;
    case HostMessageIcon::none: break;
    }
    const bool second =
        (request.buttons == HostMessageButtons::ok_cancel &&
         request.default_choice == HostDialogChoice::cancel) ||
        (request.buttons == HostMessageButtons::yes_no &&
         request.default_choice == HostDialogChoice::no) ||
        (request.buttons == HostMessageButtons::yes_no_cancel &&
         request.default_choice == HostDialogChoice::no) ||
        (request.buttons == HostMessageButtons::retry_cancel &&
         request.default_choice == HostDialogChoice::cancel);
    const bool third = request.buttons == HostMessageButtons::yes_no_cancel &&
                       request.default_choice == HostDialogChoice::cancel;
    if (third) style |= MB_DEFBUTTON3;
    else if (second) style |= MB_DEFBUTTON2;
    return style;
}

HostDialogResult native_message_dialog(HWND owner, std::uint64_t request_id,
                                       const HostMessageDialogRequest& request) {
    const std::wstring title = wide_from_utf8(request.title);
    const std::wstring message = wide_from_utf8(request.message);
    const int native_result = MessageBoxW(owner, message.c_str(), title.c_str(),
                                          native_message_style(request));
    HostMessageDialogResult value;
    switch (native_result) {
    case IDOK: value.choice = HostDialogChoice::ok; break;
    case IDCANCEL:
        value.choice = HostDialogChoice::cancel;
        value.outcome = HostDialogOutcome::cancelled;
        break;
    case IDYES: value.choice = HostDialogChoice::yes; break;
    case IDNO: value.choice = HostDialogChoice::no; break;
    case IDRETRY: value.choice = HostDialogChoice::retry; break;
    default:
        return {{HostServiceError::backend_failure}, request_id,
                HostMessageDialogResult{}};
    }
    if (native_result != IDCANCEL) value.outcome = HostDialogOutcome::accepted;
    return {{}, request_id, value};
}

HostDialogResult native_color_dialog(HWND owner, std::uint64_t request_id,
                                     const HostColorDialogRequest& request,
                                     std::array<COLORREF, 16>& custom_colors) {
    const std::uint8_t red = static_cast<std::uint8_t>(request.initial_rgba >> 24U);
    const std::uint8_t green = static_cast<std::uint8_t>(request.initial_rgba >> 16U);
    const std::uint8_t blue = static_cast<std::uint8_t>(request.initial_rgba >> 8U);
    CHOOSECOLORW native{};
    native.lStructSize = sizeof(native);
    native.hwndOwner = owner;
    native.rgbResult = RGB(red, green, blue);
    native.lpCustColors = custom_colors.data();
    native.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
    if (!ChooseColorW(&native)) {
        const DWORD error = CommDlgExtendedError();
        return error == 0
            ? HostDialogResult{{}, request_id,
                  HostColorDialogResult{HostDialogOutcome::cancelled, 0U}}
            : HostDialogResult{{HostServiceError::backend_failure}, request_id,
                  HostColorDialogResult{}};
    }
    const std::uint32_t alpha = request.allow_alpha
        ? request.initial_rgba & 0xFFU : 0xFFU;
    const std::uint32_t rgba =
        (static_cast<std::uint32_t>(GetRValue(native.rgbResult)) << 24U) |
        (static_cast<std::uint32_t>(GetGValue(native.rgbResult)) << 16U) |
        (static_cast<std::uint32_t>(GetBValue(native.rgbResult)) << 8U) | alpha;
    return {{}, request_id,
            HostColorDialogResult{HostDialogOutcome::accepted, rgba}};
}

LPCWSTR native_cursor_identifier(CursorKind cursor) noexcept {
    switch (cursor) {
    case CursorKind::text: return IDC_IBEAM;
    case CursorKind::hand: return IDC_HAND;
    case CursorKind::crosshair: return IDC_CROSS;
    case CursorKind::resize_horizontal: return IDC_SIZEWE;
    case CursorKind::resize_vertical: return IDC_SIZENS;
    case CursorKind::wait: return IDC_WAIT;
    case CursorKind::forbidden: return IDC_NO;
    case CursorKind::arrow: return IDC_ARROW;
    }
    return IDC_ARROW;
}

HCURSOR load_system_cursor(LPCWSTR identifier) noexcept {
    HCURSOR cursor = LoadCursorW(nullptr, identifier);
    if (cursor == nullptr && identifier != IDC_ARROW) {
        cursor = LoadCursorW(nullptr, IDC_ARROW);
    }
    if (cursor == nullptr) {
        cursor = reinterpret_cast<HCURSOR>(LoadImageW(
            nullptr, IDC_ARROW, IMAGE_CURSOR, 0, 0,
            LR_DEFAULTSIZE | LR_SHARED));
    }
    return cursor;
}

bool apply_system_cursor(CursorKind kind) noexcept {
    const LPCWSTR identifier = native_cursor_identifier(kind);
    HCURSOR cursor = load_system_cursor(identifier);
    if (cursor == nullptr) return false;
    HCURSOR previous = SetCursor(cursor);
    HCURSOR installed = GetCursor();
    const bool applied = installed != nullptr;
    if (trace_win32_input()) {
        std::fprintf(stderr,
                     "win32-input=cursor|kind:%u|identifier:%p|loaded:%p|"
                     "previous:%p|installed:%p|applied:%d\n",
                     static_cast<unsigned>(kind),
                     static_cast<const void*>(identifier),
                     static_cast<void*>(cursor), static_cast<void*>(previous),
                     static_cast<void*>(installed), applied);
        std::fflush(stderr);
    }
    return applied;
}

using GetDpiForMonitorFunction =
    HRESULT(WINAPI*)(HMONITOR, int, UINT*, UINT*);

GetDpiForMonitorFunction load_get_dpi_for_monitor() noexcept {
    HMODULE library = LoadLibraryW(L"Shcore.dll");
    if (library == nullptr) return nullptr;
    const FARPROC symbol = GetProcAddress(library, "GetDpiForMonitor");
    GetDpiForMonitorFunction function{};
    static_assert(sizeof(function) == sizeof(symbol));
    std::memcpy(&function, &symbol, sizeof(function));
    return function;
}

double native_monitor_scale(HMONITOR monitor) noexcept {
    static const GetDpiForMonitorFunction get_dpi_for_monitor =
        load_get_dpi_for_monitor();
    UINT x = 96U;
    UINT y = 96U;
    if (get_dpi_for_monitor == nullptr ||
        FAILED(get_dpi_for_monitor(monitor, 0, &x, &y)) || x == 0U) {
        return 1.0;
    }
    return static_cast<double>(x) / 96.0;
}

#include "../services/windows_host_services.hpp"

struct TooltipPopup final {
    HWND window{};
    std::wstring text;
    HFONT font{};
};

LRESULT CALLBACK tooltip_window_procedure(HWND window, UINT message,
                                          WPARAM wparam, LPARAM lparam) {
    TooltipPopup* state = reinterpret_cast<TooltipPopup*>(
        GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        const CREATESTRUCTW* create =
            reinterpret_cast<const CREATESTRUCTW*>(lparam);
        state = static_cast<TooltipPopup*>((*create).lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
        (*state).window = window;
    }
    switch (message) {
    case WM_ERASEBKGND: return 1;
    case WM_TIMER: DestroyWindow(window); return 0;
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        HDC dc = BeginPaint(window, &paint);
        RECT bounds{};
        GetClientRect(window, &bounds);
        HBRUSH face = CreateSolidBrush(RGB(255, 255, 240));
        FillRect(dc, &bounds, face);
        DeleteObject(face);
        HPEN border = CreatePen(PS_SOLID, 1, RGB(105, 116, 128));
        HGDIOBJ old_pen = SelectObject(dc, border);
        HGDIOBJ old_brush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
        Rectangle(dc, bounds.left, bounds.top, bounds.right, bounds.bottom);
        SelectObject(dc, old_brush);
        SelectObject(dc, old_pen);
        DeleteObject(border);
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(31, 37, 44));
        HGDIOBJ old_font = state && (*state).font ? SelectObject(dc, (*state).font) : nullptr;
        RECT text_bounds{8, 5, std::max(8L, bounds.right - 8),
                         std::max(5L, bounds.bottom - 5)};
        if (state) DrawTextW(dc, (*state).text.c_str(), -1, &text_bounds,
                             DT_WORDBREAK | DT_NOPREFIX | DT_LEFT);
        if (old_font) SelectObject(dc, old_font);
        EndPaint(window, &paint);
        return 0;
    }
    case WM_NCDESTROY:
        if (state) (*state).window = nullptr;
        return 0;
    default: return DefWindowProcW(window, message, wparam, lparam);
    }
}

Modifier modifiers() noexcept {
    Modifier result = Modifier::none;
    if ((GetKeyState(VK_SHIFT) & 0x8000) != 0) result = result | Modifier::shift;
    if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) result = result | Modifier::control;
    if ((GetKeyState(VK_MENU) & 0x8000) != 0) result = result | Modifier::alt;
    if ((GetKeyState(VK_LWIN) & 0x8000) != 0 ||
        (GetKeyState(VK_RWIN) & 0x8000) != 0) result = result | Modifier::meta;
    return result;
}

PointerButton pointer_button(UINT message, WPARAM state) noexcept {
    switch (message) {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP: return PointerButton::primary;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP: return PointerButton::secondary;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP: return PointerButton::middle;
    default:
        // WM_MOUSEMOVE identifies held buttons through wParam rather than the
        // message number. Preserve that state so WinForms-compatible custom
        // sliders can continue a drag by checking MouseEventArgs.Button.
        if ((state & MK_LBUTTON) != 0U) return PointerButton::primary;
        if ((state & MK_RBUTTON) != 0U) return PointerButton::secondary;
        if ((state & MK_MBUTTON) != 0U) return PointerButton::middle;
        return PointerButton::none;
    }
}

std::uint32_t physical_key(WPARAM key) noexcept {
    if (key >= 'A' && key <= 'Z') return 0x04U + static_cast<std::uint32_t>(key - 'A');
    if (key >= '1' && key <= '9') return 0x1EU + static_cast<std::uint32_t>(key - '1');
    if (key == '0') return 0x27U;
    switch (key) {
    case VK_RETURN: return PhysicalKey::enter;
    case VK_ESCAPE: return PhysicalKey::escape;
    case VK_BACK: return PhysicalKey::backspace;
    case VK_TAB: return PhysicalKey::tab;
    case VK_SPACE: return PhysicalKey::space;
    case VK_HOME: return PhysicalKey::home;
    case VK_PRIOR: return PhysicalKey::page_up;
    case VK_END: return PhysicalKey::end;
    case VK_NEXT: return PhysicalKey::page_down;
    case VK_DELETE: return PhysicalKey::delete_forward;
    case VK_F1: return PhysicalKey::f1;
    case VK_F2: return PhysicalKey::f2;
    case VK_F4: return PhysicalKey::f4;
    case VK_RIGHT: return PhysicalKey::right;
    case VK_LEFT: return PhysicalKey::left;
    case VK_DOWN: return PhysicalKey::down;
    case VK_UP: return PhysicalKey::up;
    default: return 0;
    }
}

template <typename Function>
Function load_function(HMODULE module, const char* name) noexcept {
    static_assert(sizeof(Function) == sizeof(FARPROC));
    const FARPROC raw = GetProcAddress(module, name);
    Function function{};
    std::memcpy(&function, &raw, sizeof(function));
    return function;
}

double query_scale(HWND window) noexcept {
    using GetDpiForWindowFunction = UINT(WINAPI*)(HWND);
    if (HMODULE user32 = GetModuleHandleW(L"user32.dll")) {
        const GetDpiForWindowFunction function =
            load_function<GetDpiForWindowFunction>(
            user32, "GetDpiForWindow");
        if (function != nullptr) return std::max(1.0, function(window) / 96.0);
    }
    HDC dc = GetDC(window);
    const int dpi = dc == nullptr ? 96 : GetDeviceCaps(dc, LOGPIXELSX);
    if (dc != nullptr) ReleaseDC(window, dc);
    return std::max(1.0, dpi / 96.0);
}

void enable_best_dpi_awareness() noexcept {
    using SetContextFunction = BOOL(WINAPI*)(HANDLE);
    using SetAwareFunction = BOOL(WINAPI*)();
    if (HMODULE user32 = GetModuleHandleW(L"user32.dll")) {
        const SetContextFunction set_context =
            load_function<SetContextFunction>(
            user32, "SetProcessDpiAwarenessContext");
        if (set_context != nullptr && set_context(reinterpret_cast<HANDLE>(-4))) return;
        const SetAwareFunction set_aware = load_function<SetAwareFunction>(
            user32, "SetProcessDPIAware");
        if (set_aware != nullptr) static_cast<void>(set_aware());
    }
}

class DibPainter final : public Painter {
    struct DecodedImage final {
        std::uint64_t content_hash{};
        std::uint32_t width{};
        std::uint32_t height{};
        std::vector<std::uint32_t> pixels;
    };
    using ImageMap = std::unordered_map<std::uint64_t, DecodedImage>;

public:
    DibPainter() = default;
    ~DibPainter() override { reset(); }
    DibPainter(const DibPainter&) = delete;
    DibPainter& operator=(const DibPainter&) = delete;

    bool resize(Size logical_size, double scale) {
        const int width = std::max(1, static_cast<int>(std::ceil(logical_size.width * scale)));
        const int height = std::max(1, static_cast<int>(std::ceil(logical_size.height * scale)));
        logical_size_ = logical_size;
        scale_ = scale;
        if (width == width_ && height == height_ && memory_dc_ != nullptr) return false;
        reset_bitmap();
        BITMAPINFO info{};
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth = width;
        info.bmiHeader.biHeight = -height;
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;
        memory_dc_ = CreateCompatibleDC(nullptr);
        bitmap_ = CreateDIBSection(memory_dc_, &info, DIB_RGB_COLORS,
                                   reinterpret_cast<void**>(&pixels_), nullptr, 0);
        if (memory_dc_ == nullptr || bitmap_ == nullptr || pixels_ == nullptr) {
            reset_bitmap();
            return false;
        }
        old_bitmap_ = SelectObject(memory_dc_, bitmap_);
        width_ = width;
        height_ = height;
        std::memset(pixels_, 0, static_cast<std::size_t>(width_) * height_ * 4U);
        return true;
    }

    void begin_frame() {
        states_.clear();
        states_.push_back(State{
            0.0,
            0.0,
            {0.0, 0.0, logical_size_.width, logical_size_.height},
            std::nullopt});
        SetBkMode(memory_dc_, TRANSPARENT);
    }

    bool synchronize_images(const ImageRegistry& registry) {
        const ImageRegistrySnapshot snapshot = registry.snapshot();
        if (&registry == image_registry_ && snapshot.revision == image_revision_) {
            return images_synchronized_;
        }
        if (&registry != image_registry_) images_.clear();
        IWICImagingFactory* factory{};
        bool synchronized = true;
        std::unordered_set<std::uint64_t> active;
        for (const ImageId id : registry.image_ids()) {
            active.insert(id.value);
            const std::optional<ImageResourceView> resource = registry.find(id);
            if (!resource) { synchronized = false; continue; }
            const ImageMap::iterator existing = images_.find(id.value);
            if (existing != images_.end() &&
                (*existing).second.content_hash == (*resource).content_hash) continue;
            DecodedImage decoded;
            decoded.content_hash = (*resource).content_hash;
            decoded.width = (*resource).metadata.width;
            decoded.height = (*resource).metadata.height;
            if ((*resource).encoding == ImageResourceEncoding::bgra32_premultiplied) {
                const std::size_t expected =
                    static_cast<std::size_t>(decoded.width) * decoded.height * 4U;
                if ((*resource).row_bytes !=
                        static_cast<std::uint64_t>(decoded.width) * 4U ||
                    (*resource).encoded.size() != expected) {
                    images_.erase(id.value);
                    synchronized = false;
                    continue;
                }
                if (existing != images_.end() &&
                    (*existing).second.width == decoded.width &&
                    (*existing).second.height == decoded.height) {
                    (*existing).second.content_hash = decoded.content_hash;
                    std::memcpy((*existing).second.pixels.data(),
                                (*resource).encoded.data(), expected);
                    continue;
                }
                decoded.pixels.resize(
                    static_cast<std::size_t>(decoded.width) * decoded.height);
                std::memcpy(decoded.pixels.data(), (*resource).encoded.data(), expected);
            } else {
                if (factory == nullptr && FAILED(CoCreateInstance(
                        CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                        IID_PPV_ARGS(&factory)))) {
                    images_.erase(id.value);
                    synchronized = false;
                    continue;
                }
                if (!decode_png(*factory, *resource, decoded)) {
                    images_.erase(id.value);
                    synchronized = false;
                    continue;
                }
            }
            images_.insert_or_assign(id.value, std::move(decoded));
        }
        for (ImageMap::iterator iterator = images_.begin();
             iterator != images_.end();) {
            iterator = active.contains((*iterator).first) ? std::next(iterator)
                                                        : images_.erase(iterator);
        }
        if (factory != nullptr) (*factory).Release();
        image_registry_ = &registry;
        image_revision_ = snapshot.revision;
        images_synchronized_ = synchronized;
        return synchronized;
    }

    [[nodiscard]] bool present(HDC target, const RECT& damage) const {
        if (target == nullptr || memory_dc_ == nullptr) return false;
        const int left = std::clamp<int>(damage.left, 0, width_);
        const int top = std::clamp<int>(damage.top, 0, height_);
        const int right = std::clamp<int>(damage.right, left, width_);
        const int bottom = std::clamp<int>(damage.bottom, top, height_);
        if (right == left || bottom == top) return true;
        return BitBlt(target, left, top, right - left, bottom - top,
                      memory_dc_, left, top, SRCCOPY) != FALSE;
    }

    [[nodiscard]] bool present_live_surface(
        HDC target, const LiveSurfacePresentation& presentation) {
        if (target == nullptr || !presentation.surface ||
            presentation.destination.empty() || presentation.clip.empty()) {
            return false;
        }
        begin_frame();
        states_.back().clip = presentation.clip;
        draw_live_surface(
            presentation.surface, presentation.destination, 1.0);
        const PixelRect pixels = pixel_rect(presentation.clip);
        RECT damage{pixels.left, pixels.top, pixels.right, pixels.bottom};
        return present(target, damage);
    }

    bool save_bmp(std::wstring_view path) const {
        if (pixels_ == nullptr || width_ <= 0 || height_ <= 0 || path.empty()) return false;
        const std::uint64_t pixel_bytes = static_cast<std::uint64_t>(width_) * height_ * 4U;
        if (pixel_bytes > std::numeric_limits<DWORD>::max()) return false;
        BITMAPFILEHEADER file{};
        BITMAPINFOHEADER bitmap{};
        file.bfType = 0x4d42;
        file.bfOffBits = sizeof(file) + sizeof(bitmap);
        file.bfSize = file.bfOffBits + static_cast<DWORD>(pixel_bytes);
        bitmap.biSize = sizeof(bitmap);
        bitmap.biWidth = width_;
        bitmap.biHeight = -height_;
        bitmap.biPlanes = 1;
        bitmap.biBitCount = 32;
        bitmap.biCompression = BI_RGB;
        bitmap.biSizeImage = static_cast<DWORD>(pixel_bytes);
        HANDLE output = CreateFileW(std::wstring(path).c_str(), GENERIC_WRITE, 0, nullptr,
                                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (output == INVALID_HANDLE_VALUE) return false;
        DWORD written{};
        const bool ok = WriteFile(output, &file, sizeof(file), &written, nullptr) &&
                        written == sizeof(file) &&
                        WriteFile(output, &bitmap, sizeof(bitmap), &written, nullptr) &&
                        written == sizeof(bitmap) &&
                        WriteFile(output, pixels_, static_cast<DWORD>(pixel_bytes),
                                  &written, nullptr) &&
                        written == pixel_bytes;
        CloseHandle(output);
        return ok;
    }

    void save() override { states_.push_back(state()); }
    void restore() override { if (states_.size() > 1) states_.pop_back(); }
    void translate(Point offset) override {
        states_.back().tx += offset.x;
        states_.back().ty += offset.y;
    }
    void clip_rect(Rect rect) override {
        rect.x += state().tx;
        rect.y += state().ty;
        states_.back().clip = Rect::intersection(state().clip, rect);
    }
    void clip_rounded_rect(Rect rect, double radius) override {
        rect.x += state().tx;
        rect.y += state().ty;
        states_.back().clip = Rect::intersection(state().clip, rect);
        states_.back().rounded_clip = RoundedClip{
            rect, std::clamp(radius, 0.0,
                             std::max(0.0, std::min(rect.width, rect.height) * 0.5))};
    }

    void fill_rect(Rect rect, Color color) override {
        const PixelRect area = pixel_rect(rect);
        if (area.empty()) return;
        const unsigned alpha = color.alpha;
        for (int y = area.top; y < area.bottom; ++y) {
            std::uint32_t* row = pixels_ + static_cast<std::size_t>(y) * width_;
            for (int x = area.left; x < area.right; ++x) {
                if (pixel_allowed(x, y)) blend_pixel(row[x], color, alpha);
            }
        }
    }

    void fill_rounded_rect(Rect rect, double radius, Color color) override {
        const PixelRect area = pixel_rect(rect);
        if (area.empty()) return;
        rect.x += state().tx;
        rect.y += state().ty;
        radius = std::clamp(radius, 0.0,
                            std::max(0.0, std::min(rect.width, rect.height) * 0.5));
        for (int y = area.top; y < area.bottom; ++y) {
            std::uint32_t* row = pixels_ + static_cast<std::size_t>(y) * width_;
            for (int x = area.left; x < area.right; ++x) {
                const Point sample{(x + 0.5) / scale_, (y + 0.5) / scale_};
                if (pixel_allowed(x, y) && inside_rounded(sample, rect, radius)) {
                    blend_pixel(row[x], color, color.alpha);
                }
            }
        }
    }

    void stroke_rect(Rect rect, Color color, double width) override {
        const double line = std::max(width, 1.0 / scale_);
        fill_rect({rect.x, rect.y, rect.width, line}, color);
        fill_rect({rect.x, rect.y + rect.height - line, rect.width, line}, color);
        fill_rect({rect.x, rect.y, line, rect.height}, color);
        fill_rect({rect.x + rect.width - line, rect.y, line, rect.height}, color);
    }

    void stroke_rounded_rect(Rect rect, double radius, Color color,
                             double width) override {
        const PixelRect area = pixel_rect(rect);
        if (area.empty()) return;
        const double line = std::max(width, 1.0 / scale_);
        rect.x += state().tx;
        rect.y += state().ty;
        radius = std::clamp(radius, 0.0,
                            std::max(0.0, std::min(rect.width, rect.height) * 0.5));
        const Rect inner{rect.x + line, rect.y + line,
                         rect.width - line * 2.0, rect.height - line * 2.0};
        const double inner_radius = std::max(0.0, radius - line);
        for (int y = area.top; y < area.bottom; ++y) {
            std::uint32_t* row = pixels_ + static_cast<std::size_t>(y) * width_;
            for (int x = area.left; x < area.right; ++x) {
                const Point sample{(x + 0.5) / scale_, (y + 0.5) / scale_};
                const bool in_outer = inside_rounded(sample, rect, radius);
                const bool in_inner = !inner.empty() &&
                    inside_rounded(sample, inner, inner_radius);
                if (pixel_allowed(x, y) && in_outer && !in_inner) {
                    blend_pixel(row[x], color, color.alpha);
                }
            }
        }
    }

    void fill_linear_gradient(
        Rect rect, Point start, Point end,
        std::span<const GradientStop> stops) override {
        fill_linear_gradient_spread(rect, start, end, stops,
                                    GradientSpreadMode::pad);
    }

    void fill_linear_gradient_spread(
        Rect rect, Point start, Point end,
        std::span<const GradientStop> stops,
        GradientSpreadMode spread) override {
        const PixelRect area = pixel_rect(rect);
        if (area.empty() || !valid_gradient_stops(stops) ||
            (spread != GradientSpreadMode::pad &&
             spread != GradientSpreadMode::repeat &&
             spread != GradientSpreadMode::reflect)) {
            return;
        }
        start.x += state().tx;
        start.y += state().ty;
        end.x += state().tx;
        end.y += state().ty;
        const double dx = end.x - start.x;
        const double dy = end.y - start.y;
        const double length_squared = dx * dx + dy * dy;
        if (length_squared <= 0.0) return;
        for (int y = area.top; y < area.bottom; ++y) {
            std::uint32_t* row = pixels_ + static_cast<std::size_t>(y) * width_;
            for (int x = area.left; x < area.right; ++x) {
                if (!pixel_allowed(x, y)) continue;
                const Point sample{(x + 0.5) / scale_, (y + 0.5) / scale_};
                const double amount = spread_gradient_coordinate(
                    ((sample.x - start.x) * dx +
                     (sample.y - start.y) * dy) /
                    length_squared,
                    spread);
                const Color color = gradient_color(stops, amount);
                blend_pixel(row[x], color, color.alpha);
            }
        }
    }

    void fill_radial_gradient(
        Rect rect, Point center, Size radii,
        std::span<const GradientStop> stops) override {
        const PixelRect area = pixel_rect(rect);
        if (area.empty() || radii.width <= 0.0 || radii.height <= 0.0 ||
            !valid_gradient_stops(stops)) {
            return;
        }
        center.x += state().tx;
        center.y += state().ty;
        for (int y = area.top; y < area.bottom; ++y) {
            std::uint32_t* row = pixels_ + static_cast<std::size_t>(y) * width_;
            for (int x = area.left; x < area.right; ++x) {
                if (!pixel_allowed(x, y)) continue;
                const double sample_x = ((x + 0.5) / scale_ - center.x) /
                                        radii.width;
                const double sample_y = ((y + 0.5) / scale_ - center.y) /
                                        radii.height;
                const Color color = gradient_color(
                    stops, std::sqrt(sample_x * sample_x + sample_y * sample_y));
                blend_pixel(row[x], color, color.alpha);
            }
        }
    }

    void draw_box_shadow(Rect rect, double corner_radius, Point offset,
                         double blur_radius, double spread,
                         Color color) override {
        if (rect.empty() || color.alpha == 0U || blur_radius < 0.0) return;
        const double reach = blur_radius * 3.0;
        const Rect paint_bounds{rect.x + offset.x - spread - reach,
                                rect.y + offset.y - spread - reach,
                                rect.width + (spread + reach) * 2.0,
                                rect.height + (spread + reach) * 2.0};
        const PixelRect area = pixel_rect(paint_bounds);
        if (area.empty()) return;
        Rect shadow{rect.x + state().tx + offset.x - spread,
                    rect.y + state().ty + offset.y - spread,
                    rect.width + spread * 2.0,
                    rect.height + spread * 2.0};
        const double radius = std::max(0.0, corner_radius + spread);
        const double sigma = std::max(blur_radius * 0.5, 0.25 / scale_);
        for (int y = area.top; y < area.bottom; ++y) {
            std::uint32_t* row = pixels_ + static_cast<std::size_t>(y) * width_;
            for (int x = area.left; x < area.right; ++x) {
                if (!pixel_allowed(x, y)) continue;
                const Point sample{(x + 0.5) / scale_, (y + 0.5) / scale_};
                const double distance = rounded_distance(sample, shadow, radius);
                const double coverage = distance <= 0.0 ? 1.0
                    : std::exp(-0.5 * (distance / sigma) * (distance / sigma));
                if (coverage <= 0.001) continue;
                const unsigned alpha = static_cast<unsigned>(std::lround(
                    static_cast<double>(color.alpha) * coverage));
                blend_pixel(row[x], color, alpha);
            }
        }
    }

    void draw_line(Point from, Point to, Color color, double width) override {
        const int x0 = logical_x(from.x);
        const int y0 = logical_y(from.y);
        const int x1 = logical_x(to.x);
        const int y1 = logical_y(to.y);
        const int thickness = std::max(1, static_cast<int>(std::lround(width * scale_)));
        HPEN pen = CreatePen(PS_SOLID, thickness, RGB(color.red, color.green, color.blue));
        if (pen == nullptr) return;
        const int saved = SaveDC(memory_dc_);
        apply_gdi_clip();
        HGDIOBJ old = SelectObject(memory_dc_, pen);
        MoveToEx(memory_dc_, x0, y0, nullptr);
        LineTo(memory_dc_, x1, y1);
        SelectObject(memory_dc_, old);
        RestoreDC(memory_dc_, saved);
        DeleteObject(pen);
    }

    void draw_text_utf8(Point origin, std::string_view text,
                        FontSpec font, Color color) override {
        const std::vector<GdiTextRun> runs = text_runs(text, font);
        if (runs.empty() || memory_dc_ == nullptr) return;
        const int saved = SaveDC(memory_dc_);
        apply_gdi_clip();
        SetTextColor(memory_dc_, RGB(color.red, color.green, color.blue));
        SetBkMode(memory_dc_, TRANSPARENT);
        const int baseline = logical_y(origin.y);
        int x = logical_x(origin.x);
        for (const GdiTextRun& run : runs) {
            HFONT native_font = create_font(font, run.family.c_str());
            if (native_font == nullptr) continue;
            HGDIOBJ old = SelectObject(memory_dc_, native_font);
            SetTextCharacterExtra(memory_dc_,
                static_cast<int>(std::lround(font.letter_spacing * scale_)));
            TEXTMETRICW metrics{};
            SIZE measured{};
            GetTextMetricsW(memory_dc_, &metrics);
            const bool rendered = shape_text_run(run.text, x, baseline,
                                                 true, measured);
            if (trace_win32_text()) {
                std::fprintf(stderr,
                             "win32-text draw family=%s units=%zu rendered=%d\n",
                             utf8_from_wide(run.family).c_str(), run.text.size(),
                             rendered ? 1 : 0);
            }
            if (rendered) {
                x += measured.cx;
            }
            SelectObject(memory_dc_, old);
            DeleteObject(native_font);
        }
        RestoreDC(memory_dc_, saved);
    }

    Size measure_text_utf8(std::string_view text, FontSpec font) override {
        const std::vector<GdiTextRun> runs = text_runs(text, font);
        if (runs.empty() || memory_dc_ == nullptr) return {0.0, font.size};
        const int saved = SaveDC(memory_dc_);
        int width{};
        int height{};
        for (const GdiTextRun& run : runs) {
            HFONT native_font = create_font(font, run.family.c_str());
            if (native_font == nullptr) continue;
            HGDIOBJ old = SelectObject(memory_dc_, native_font);
            SetTextCharacterExtra(memory_dc_,
                static_cast<int>(std::lround(font.letter_spacing * scale_)));
            SIZE measured{};
            if (shape_text_run(run.text, 0, 0, false, measured)) {
                width += measured.cx;
                height = std::max(height, static_cast<int>(measured.cy));
            }
            SelectObject(memory_dc_, old);
            DeleteObject(native_font);
        }
        RestoreDC(memory_dc_, saved);
        return {width / scale_, height / scale_};
    }

    void draw_image(ImageId image, Rect destination, double opacity) override {
        const ImageMap::iterator found = images_.find(image.value);
        if (found == images_.end()) return;
        draw_image_region(
            image,
            {0.0, 0.0, static_cast<double>((*found).second.width),
             static_cast<double>((*found).second.height)},
            destination, opacity);
    }

    void draw_live_surface(std::shared_ptr<LiveSurface> surface,
                           Rect destination, double opacity) override {
        const PixelRect area = pixel_rect(destination);
        if (!surface || area.empty() || destination.empty() ||
            !destination.finite() ||
            !std::isfinite(opacity) || opacity <= 0.0 || memory_dc_ == nullptr) {
            return;
        }
        LiveSurfaceFrame frame = (*surface).acquire_latest();
        if (!frame || frame.width() == 0U || frame.height() == 0U ||
            frame.row_bytes() != static_cast<std::uint64_t>(frame.width()) * 4U ||
            frame.pixels().empty()) {
            return;
        }
        const int saved = SaveDC(memory_dc_);
        IntersectClipRect(memory_dc_, area.left, area.top, area.right, area.bottom);
        SetStretchBltMode(memory_dc_, COLORONCOLOR);
        const int destination_x = logical_x(destination.x);
        const int destination_y = logical_y(destination.y);
        const int destination_width = std::max(
            1, static_cast<int>(std::lround(destination.width * scale_)));
        const int destination_height = std::max(
            1, static_cast<int>(std::lround(destination.height * scale_)));
        if (opacity >= 1.0 &&
            destination_width == static_cast<int>(frame.width()) &&
            destination_height == static_cast<int>(frame.height())) {
            // The retained Win32 surface and compatibility/live producer are
            // both top-down packed BGRA DIBs. A same-size frame needs no GDI
            // object construction or resampling; copying only the active clip
            // keeps high-rate surfaces on the compositor's memory fast path.
            // This is equivalent to the existing SRCCOPY realization.
            const int copy_left = std::max(area.left, destination_x);
            const int copy_top = std::max(area.top, destination_y);
            const int copy_right = std::min(
                area.right, destination_x + destination_width);
            const int copy_bottom = std::min(
                area.bottom, destination_y + destination_height);
            if (copy_right > copy_left && copy_bottom > copy_top) {
                const std::size_t copy_bytes =
                    static_cast<std::size_t>(copy_right - copy_left) * 4U;
                const std::size_t source_stride =
                    static_cast<std::size_t>(frame.row_bytes());
                const std::byte* source = frame.pixels().data();
                for (int y = copy_top; y < copy_bottom; ++y) {
                    const std::size_t source_y =
                        static_cast<std::size_t>(y - destination_y);
                    const std::size_t source_x =
                        static_cast<std::size_t>(copy_left - destination_x) * 4U;
                    std::memcpy(
                        reinterpret_cast<std::byte*>(pixels_) +
                            (static_cast<std::size_t>(y) * width_ + copy_left) * 4U,
                        source + source_y * source_stride + source_x,
                        copy_bytes);
                }
            }
            if (saved != 0) RestoreDC(memory_dc_, saved);
            return;
        }
        BITMAPINFO info{};
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth = static_cast<LONG>(frame.width());
        info.bmiHeader.biHeight = -static_cast<LONG>(frame.height());
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;
        static_cast<void>(StretchDIBits(
            memory_dc_, destination_x, destination_y,
            destination_width, destination_height, 0, 0,
            static_cast<int>(frame.width()), static_cast<int>(frame.height()),
            frame.pixels().data(), &info, DIB_RGB_COLORS, SRCCOPY));
        if (saved != 0) RestoreDC(memory_dc_, saved);
    }

    void draw_image_region(ImageId image, Rect source_rect, Rect destination,
                           double opacity) override {
        const ImageMap::iterator found = images_.find(image.value);
        const PixelRect area = pixel_rect(destination);
        if (found == images_.end() || area.empty() || source_rect.empty() ||
            !source_rect.finite() || !destination.finite() ||
            !std::isfinite(opacity) || opacity <= 0.0) {
            return;
        }
        const DecodedImage& source = (*found).second;
        const Rect source_bounds{0.0, 0.0, static_cast<double>(source.width),
                                 static_cast<double>(source.height)};
        if (!source_bounds.contains(source_rect)) return;
        const double left = (destination.x + state().tx) * scale_;
        const double top = (destination.y + state().ty) * scale_;
        const double width = std::max(1.0, destination.width * scale_);
        const double height = std::max(1.0, destination.height * scale_);
        const unsigned global_alpha = static_cast<unsigned>(
            std::lround(std::clamp(opacity, 0.0, 1.0) * 255.0));
        for (int y = area.top; y < area.bottom; ++y) {
            const unsigned int source_y = std::min<std::uint32_t>(
                source.height - 1U,
                static_cast<std::uint32_t>(source_rect.y + std::max(
                    0.0, (y - top) * source_rect.height / height)));
            std::uint32_t* destination_row = pixels_ + static_cast<std::size_t>(y) * width_;
            for (int x = area.left; x < area.right; ++x) {
                if (!pixel_allowed(x, y)) continue;
                const unsigned int source_x = std::min<std::uint32_t>(
                    source.width - 1U,
                    static_cast<std::uint32_t>(source_rect.x + std::max(
                        0.0, (x - left) * source_rect.width / width)));
                const std::uint32_t source_pixel =
                    source.pixels[static_cast<std::size_t>(source_y) * source.width + source_x];
                const unsigned source_alpha = ((source_pixel >> 24U) & 0xffU) * global_alpha / 255U;
                const std::uint32_t destination_pixel = destination_row[x];
                destination_row[x] = blend_image_pixel(
                    source_pixel, destination_pixel, global_alpha,
                    source_alpha);
            }
        }
    }

    void fill_image_pattern(
        ImageId image, Size source_pixel_size, Rect destination,
        Size logical_tile_size, ImagePatternWrap wrap,
        double opacity) override {
        const ImageMap::iterator found = images_.find(image.value);
        const PixelRect area = pixel_rect(destination);
        if (found == images_.end() || area.empty() ||
            wrap != ImagePatternWrap::tile || !destination.finite() ||
            !std::isfinite(source_pixel_size.width) ||
            !std::isfinite(source_pixel_size.height) ||
            !std::isfinite(logical_tile_size.width) ||
            !std::isfinite(logical_tile_size.height) ||
            source_pixel_size.width != (*found).second.width ||
            source_pixel_size.height != (*found).second.height ||
            logical_tile_size.width <= 0.0 || logical_tile_size.height <= 0.0 ||
            !std::isfinite(opacity) || opacity <= 0.0) {
            return;
        }
        const DecodedImage& source = (*found).second;
        const double left = (destination.x + state().tx) * scale_;
        const double top = (destination.y + state().ty) * scale_;
        const double tile_width = logical_tile_size.width * scale_;
        const double tile_height = logical_tile_size.height * scale_;
        const unsigned global_alpha = static_cast<unsigned>(
            std::lround(std::clamp(opacity, 0.0, 1.0) * 255.0));
        for (int y = area.top; y < area.bottom; ++y) {
            const double tile_y = std::fmod(std::max(0.0, y - top), tile_height);
            const unsigned int source_y = std::min<std::uint32_t>(
                source.height - 1U,
                static_cast<std::uint32_t>(tile_y * source.height / tile_height));
            std::uint32_t* destination_row = pixels_ + static_cast<std::size_t>(y) * width_;
            for (int x = area.left; x < area.right; ++x) {
                if (!pixel_allowed(x, y)) continue;
                const double tile_x = std::fmod(std::max(0.0, x - left), tile_width);
                const unsigned int source_x = std::min<std::uint32_t>(
                    source.width - 1U,
                    static_cast<std::uint32_t>(tile_x * source.width / tile_width));
                const std::uint32_t source_pixel =
                    source.pixels[static_cast<std::size_t>(source_y) *
                                      source.width + source_x];
                const unsigned source_alpha =
                    ((source_pixel >> 24U) & 0xffU) * global_alpha / 255U;
                const std::uint32_t destination_pixel = destination_row[x];
                destination_row[x] = blend_image_pixel(
                    source_pixel, destination_pixel, global_alpha,
                    source_alpha);
            }
        }
    }

private:
    struct RoundedClip final {
        Rect rect{};
        double radius{};
    };
    struct State {
        double tx{};
        double ty{};
        Rect clip{};
        std::optional<RoundedClip> rounded_clip;
    };
    struct GdiTextRun {
        std::wstring family;
        std::wstring text;
    };
    struct PixelRect {
        int left{}; int top{}; int right{}; int bottom{};
        [[nodiscard]] bool empty() const noexcept { return right <= left || bottom <= top; }
    };

    const State& state() const noexcept { return states_.back(); }
    [[nodiscard]] static bool inside_rounded(Point point, Rect rect,
                                             double radius) noexcept {
        if (!rect.contains(point)) return false;
        radius = std::clamp(radius, 0.0,
                            std::max(0.0, std::min(rect.width, rect.height) * 0.5));
        if (radius <= 0.0) return true;
        const double nearest_x = std::clamp(
            point.x, rect.x + radius, rect.x + rect.width - radius);
        const double nearest_y = std::clamp(
            point.y, rect.y + radius, rect.y + rect.height - radius);
        const double dx = point.x - nearest_x;
        const double dy = point.y - nearest_y;
        return dx * dx + dy * dy <= radius * radius;
    }
    [[nodiscard]] static double rounded_distance(Point point, Rect rect,
                                                 double radius) noexcept {
        radius = std::clamp(radius, 0.0,
                            std::max(0.0, std::min(rect.width, rect.height) * 0.5));
        const double center_x = rect.x + rect.width * 0.5;
        const double center_y = rect.y + rect.height * 0.5;
        const double qx = std::abs(point.x - center_x) -
                          std::max(0.0, rect.width * 0.5 - radius);
        const double qy = std::abs(point.y - center_y) -
                          std::max(0.0, rect.height * 0.5 - radius);
        const double outside = std::hypot(std::max(qx, 0.0), std::max(qy, 0.0));
        const double inside = std::min(std::max(qx, qy), 0.0);
        return outside + inside - radius;
    }
    [[nodiscard]] bool pixel_allowed(int x, int y) const noexcept {
        if (!state().rounded_clip) return true;
        const Point point{(x + 0.5) / scale_, (y + 0.5) / scale_};
        return inside_rounded(point, (*state().rounded_clip).rect,
                              (*state().rounded_clip).radius);
    }
    static void blend_pixel(std::uint32_t& destination, Color color,
                            unsigned alpha) noexcept {
        alpha = std::min(alpha, 255U);
        const unsigned db = destination & 0xffU;
        const unsigned dg = (destination >> 8U) & 0xffU;
        const unsigned dr = (destination >> 16U) & 0xffU;
        const unsigned inverse = 255U - alpha;
        const unsigned b = (color.blue * alpha + db * inverse + 127U) / 255U;
        const unsigned g = (color.green * alpha + dg * inverse + 127U) / 255U;
        const unsigned r = (color.red * alpha + dr * inverse + 127U) / 255U;
        destination = b | (g << 8U) | (r << 16U) | 0xff000000U;
    }
    [[nodiscard]] static unsigned composite_image_channel(
        std::uint32_t source, std::uint32_t destination,
        unsigned global_alpha, unsigned inverse, unsigned shift) noexcept {
        const unsigned premultiplied =
            ((source >> shift) & 0xffU) * global_alpha / 255U;
        const unsigned destination_channel = (destination >> shift) & 0xffU;
        return std::min(
            255U,
            premultiplied +
                (destination_channel * inverse + 127U) / 255U);
    }
    [[nodiscard]] static std::uint32_t blend_image_pixel(
        std::uint32_t source, std::uint32_t destination,
        unsigned global_alpha, unsigned source_alpha) noexcept {
        const unsigned inverse = 255U - source_alpha;
        const unsigned blue = composite_image_channel(
            source, destination, global_alpha, inverse, 0U);
        const unsigned green = composite_image_channel(
            source, destination, global_alpha, inverse, 8U);
        const unsigned red = composite_image_channel(
            source, destination, global_alpha, inverse, 16U);
        return blue | (green << 8U) | (red << 16U) | 0xff000000U;
    }
    [[nodiscard]] static std::uint8_t interpolate_channel(
        std::uint8_t left, std::uint8_t right, double amount) noexcept {
        return static_cast<std::uint8_t>(std::lround(
            static_cast<double>(left) +
            (static_cast<double>(right) - left) * amount));
    }
    [[nodiscard]] static Color gradient_color(
        std::span<const GradientStop> stops, double amount) noexcept {
        amount = std::clamp(amount, 0.0, 1.0);
        for (std::size_t index = 1U; index < stops.size(); ++index) {
            if (amount <= stops[index].offset) {
                const GradientStop& first = stops[index - 1U];
                const GradientStop& second = stops[index];
                const double width = second.offset - first.offset;
                const double local = width <= 0.0 ? 1.0
                    : (amount - first.offset) / width;
                return Color::rgba(
                    interpolate_channel(
                        first.color.red, second.color.red, local),
                    interpolate_channel(
                        first.color.green, second.color.green, local),
                    interpolate_channel(
                        first.color.blue, second.color.blue, local),
                    interpolate_channel(
                        first.color.alpha, second.color.alpha, local));
            }
        }
        return stops.back().color;
    }
    [[nodiscard]] static double spread_gradient_coordinate(
        double amount, GradientSpreadMode spread) noexcept {
        switch (spread) {
        case GradientSpreadMode::pad:
            return std::clamp(amount, 0.0, 1.0);
        case GradientSpreadMode::repeat:
            return amount - std::floor(amount);
        case GradientSpreadMode::reflect: {
            double folded = std::fmod(amount, 2.0);
            if (folded < 0.0) folded += 2.0;
            return folded <= 1.0 ? folded : 2.0 - folded;
        }
        }
        return std::clamp(amount, 0.0, 1.0);
    }
    int logical_x(double value) const noexcept {
        return static_cast<int>(std::lround((value + state().tx) * scale_));
    }
    int logical_y(double value) const noexcept {
        return static_cast<int>(std::lround((value + state().ty) * scale_));
    }
    PixelRect pixel_rect(Rect rect) const noexcept {
        rect.x += state().tx;
        rect.y += state().ty;
        rect = Rect::intersection(rect, state().clip);
        return {std::clamp(static_cast<int>(std::floor(rect.x * scale_)), 0, width_),
                std::clamp(static_cast<int>(std::floor(rect.y * scale_)), 0, height_),
                std::clamp(static_cast<int>(std::ceil((rect.x + rect.width) * scale_)), 0, width_),
                std::clamp(static_cast<int>(std::ceil((rect.y + rect.height) * scale_)), 0, height_)};
    }
    void apply_gdi_clip() const {
        const Rect clip = state().clip;
        IntersectClipRect(memory_dc_,
            static_cast<int>(std::floor(clip.x * scale_)),
            static_cast<int>(std::floor(clip.y * scale_)),
            static_cast<int>(std::ceil((clip.x + clip.width) * scale_)),
            static_cast<int>(std::ceil((clip.y + clip.height) * scale_)));
    }
    [[nodiscard]] static const wchar_t* primary_font_family(FontRole role) {
        return role == FontRole::control ? L"Portsmouth Rapids"
            : role == FontRole::monospace ? L"Cousine" : L"Carlito";
    }
    [[nodiscard]] HFONT create_font(FontSpec font, const wchar_t* family) const {
        return CreateFontW(
            -std::max(1, static_cast<int>(std::lround(font.size * scale_))),
            0, 0, 0, font.weight, font.italic ? TRUE : FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, family);
    }
    [[nodiscard]] bool shape_text_run(std::wstring_view text, int x, int y,
                                      bool draw, SIZE& measured) const {
        if (text.empty() ||
            text.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
            return false;
        }
        const int length = static_cast<int>(text.size());
        std::vector<SCRIPT_ITEM> source_items(text.size() + 2U);
        int item_count{};
        if (FAILED(ScriptItemize(text.data(), length,
                                 static_cast<int>(source_items.size()), nullptr,
                                 nullptr, source_items.data(), &item_count)) ||
            item_count <= 0) {
            return false;
        }
        struct PlacedItem final {
            SCRIPT_ANALYSIS analysis{};
            std::vector<WORD> glyphs;
            std::vector<int> advances;
            std::vector<GOFFSET> offsets;
            int width{};
        };
        SCRIPT_CACHE cache{};
        std::vector<PlacedItem> placed;
        placed.reserve(static_cast<std::size_t>(item_count));
        bool valid = true;
        for (int item = 0; item < item_count && valid; ++item) {
            const int start = source_items[static_cast<std::size_t>(item)].iCharPos;
            const int end = source_items[static_cast<std::size_t>(item + 1)].iCharPos;
            const int item_length = end - start;
            if (item_length <= 0) continue;
            const int capacity = item_length * 3 / 2 + 16;
            PlacedItem output;
            output.analysis = source_items[static_cast<std::size_t>(item)].a;
            output.glyphs.resize(static_cast<std::size_t>(capacity));
            std::vector<WORD> clusters(static_cast<std::size_t>(item_length));
            std::vector<SCRIPT_VISATTR> attributes(
                static_cast<std::size_t>(capacity));
            int glyph_count{};
            if (FAILED(ScriptShape(
                    memory_dc_, &cache, text.data() + start, item_length,
                    capacity, &output.analysis, output.glyphs.data(),
                    clusters.data(), attributes.data(), &glyph_count)) ||
                glyph_count <= 0) {
                valid = false;
                break;
            }
            output.glyphs.resize(static_cast<std::size_t>(glyph_count));
            attributes.resize(static_cast<std::size_t>(glyph_count));
            output.advances.resize(static_cast<std::size_t>(glyph_count));
            output.offsets.resize(static_cast<std::size_t>(glyph_count));
            ABC extent{};
            if (FAILED(ScriptPlace(memory_dc_, &cache, output.glyphs.data(),
                                   glyph_count, attributes.data(),
                                   &output.analysis, output.advances.data(),
                                   output.offsets.data(), &extent))) {
                valid = false;
                break;
            }
            const int character_extra = GetTextCharacterExtra(memory_dc_);
            for (int glyph = 0; glyph < glyph_count; ++glyph) {
                if (character_extra != 0 &&
                    !(item + 1 == item_count && glyph + 1 == glyph_count)) {
                    output.advances[static_cast<std::size_t>(glyph)] +=
                        character_extra;
                }
                output.width += output.advances[static_cast<std::size_t>(glyph)];
            }
            placed.push_back(std::move(output));
        }
        TEXTMETRICW metrics{};
        GetTextMetricsW(memory_dc_, &metrics);
        if (valid) {
            measured = {0, metrics.tmHeight};
            if (draw) SetTextAlign(memory_dc_, TA_LEFT | TA_BASELINE);
            std::vector<BYTE> levels(placed.size());
            for (std::size_t item = 0; item < placed.size(); ++item) {
                levels[item] = placed[item].analysis.s.uBidiLevel;
            }
            std::vector<int> visual_to_logical(placed.size());
            std::vector<int> logical_to_visual(placed.size());
            if (!placed.empty()) {
                ScriptLayout(static_cast<int>(placed.size()), levels.data(),
                             visual_to_logical.data(), logical_to_visual.data());
            }
            for (std::size_t visual = 0; visual < placed.size(); ++visual) {
                const std::size_t logical = static_cast<std::size_t>(
                    visual_to_logical[visual]);
                PlacedItem& item = placed[logical];
                if (draw && FAILED(ScriptTextOut(
                        memory_dc_, &cache, x + measured.cx, y, 0, nullptr,
                        &item.analysis, nullptr, 0, item.glyphs.data(),
                        static_cast<int>(item.glyphs.size()), item.advances.data(),
                        nullptr, item.offsets.data()))) {
                    valid = false;
                    break;
                }
                measured.cx += item.width;
            }
        }
        ScriptFreeCache(&cache);
        if (valid) return true;
        if (!GetTextExtentPoint32W(memory_dc_, text.data(), length, &measured)) {
            return false;
        }
        if (draw) SetTextAlign(memory_dc_, TA_LEFT | TA_TOP);
        return !draw || TextOutW(memory_dc_, x, y - metrics.tmAscent,
                                 text.data(), length);
    }
    [[nodiscard]] bool selected_font_shapes(std::wstring_view text) const {
        if (text.empty() ||
            text.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
            return false;
        }
        const int length = static_cast<int>(text.size());
        std::vector<SCRIPT_ITEM> items(text.size() + 2U);
        int item_count{};
        if (FAILED(ScriptItemize(text.data(), length,
                                 static_cast<int>(items.size()), nullptr, nullptr,
                                 items.data(), &item_count)) || item_count <= 0) {
            return false;
        }
        SCRIPT_CACHE cache{};
        bool covered = true;
        for (int item = 0; item < item_count && covered; ++item) {
            const int start = items[static_cast<std::size_t>(item)].iCharPos;
            const int end = items[static_cast<std::size_t>(item + 1)].iCharPos;
            const int item_length = end - start;
            if (item_length <= 0) continue;
            const int capacity = item_length * 3 / 2 + 16;
            std::vector<WORD> glyphs(static_cast<std::size_t>(capacity));
            std::vector<WORD> clusters(static_cast<std::size_t>(item_length));
            std::vector<SCRIPT_VISATTR> attributes(
                static_cast<std::size_t>(capacity));
            int glyph_count{};
            const HRESULT shaped = ScriptShape(
                memory_dc_, &cache, text.data() + start, item_length, capacity,
                &items[static_cast<std::size_t>(item)].a, glyphs.data(),
                clusters.data(), attributes.data(), &glyph_count);
            if (FAILED(shaped) || glyph_count <= 0) {
                covered = false;
                continue;
            }
            SCRIPT_FONTPROPERTIES properties{};
            properties.cBytes = sizeof(properties);
            if (FAILED(ScriptGetFontProperties(memory_dc_, &cache, &properties))) {
                covered = false;
                continue;
            }
            for (int glyph = 0; glyph < glyph_count; ++glyph) {
                if (glyphs[static_cast<std::size_t>(glyph)] ==
                    properties.wgDefault) {
                    covered = false;
                    break;
                }
            }
            if (trace_win32_text()) {
                std::fprintf(stderr,
                             "win32-text shaped=%d glyph-count=%d default=%04x glyphs=",
                             covered ? 1 : 0, glyph_count,
                             static_cast<unsigned>(properties.wgDefault));
                for (int glyph = 0; glyph < glyph_count; ++glyph) {
                    std::fprintf(stderr, "%04x,", static_cast<unsigned>(
                        glyphs[static_cast<std::size_t>(glyph)]));
                }
                std::fputc('\n', stderr);
            }
        }
        ScriptFreeCache(&cache);
        return covered;
    }
    [[nodiscard]] bool font_covers(HFONT font, const wchar_t* requested_family,
                                   std::wstring_view text) const {
        if (font == nullptr || text.empty() ||
            text.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
            return false;
        }
        const int saved = SaveDC(memory_dc_);
        HGDIOBJ old = SelectObject(memory_dc_, font);
        const bool shaped_coverage = selected_font_shapes(text);
        std::vector<WORD> glyphs(text.size(), 0xffffU);
        const DWORD count = GetGlyphIndicesW(
            memory_dc_, text.data(), static_cast<int>(text.size()),
            glyphs.data(), GGI_MARK_NONEXISTING_GLYPHS);
        if (trace_win32_text()) {
            std::array<wchar_t, LF_FACESIZE> actual_family{};
            GetTextFaceW(memory_dc_, static_cast<int>(actual_family.size()),
                         actual_family.data());
            std::fprintf(stderr,
                         "win32-text requested=%s actual=%s units=%zu glyphs=",
                         utf8_from_wide(requested_family).c_str(),
                         utf8_from_wide(actual_family.data()).c_str(), text.size());
            for (const WORD glyph : glyphs) {
                std::fprintf(stderr, "%04x,", static_cast<unsigned>(glyph));
            }
            std::fputc('\n', stderr);
        }
        SelectObject(memory_dc_, old);
        RestoreDC(memory_dc_, saved);
        if (shaped_coverage) return true;
        if (count == GDI_ERROR) return false;
        for (std::size_t index = 0; index < text.size(); ++index) {
            const wchar_t unit = text[index];
            if (unit == 0x200d || (unit >= 0xfe00 && unit <= 0xfe0f)) continue;
            if (unit >= 0xd800 && unit <= 0xdbff && index + 1U < text.size() &&
                text[index + 1U] >= 0xdc00 && text[index + 1U] <= 0xdfff) {
                if (glyphs[index] == 0xffffU && glyphs[index + 1U] == 0xffffU) {
                    return false;
                }
                ++index;
                continue;
            }
            if (glyphs[index] == 0xffffU) return false;
        }
        return true;
    }
    [[nodiscard]] std::vector<GdiTextRun> text_runs(
        std::string_view utf8, FontSpec font) const {
        std::vector<GdiTextRun> runs;
        if (utf8.empty() || memory_dc_ == nullptr || !validate_utf8(utf8).valid()) {
            return runs;
        }
        constexpr std::array<const wchar_t*, 2> fallback_families{
            L"Noto Sans CJK JP", L"Noto Emoji"};
        std::array<HFONT, 3> candidates{
            create_font(font, primary_font_family(font.role)),
            create_font(font, fallback_families[0]),
            create_font(font, fallback_families[1])};
        const std::array<const wchar_t*, 3> families{
            primary_font_family(font.role), fallback_families[0],
            fallback_families[1]};
        TextStore store(utf8);
        for (std::size_t index = 0; index < store.grapheme_count().value();
             ++index) {
            const Utf8Range range = store.grapheme_range(GraphemeIndex(index));
            const std::wstring cluster = wide_from_utf8(
                utf8.substr(range.start.value(),
                            range.end.value() - range.start.value()));
            if (cluster.empty()) continue;
            std::size_t selected{};
            bool covered{};
            for (; selected < candidates.size(); ++selected) {
                covered = font_covers(candidates[selected], families[selected],
                                      cluster);
                if (covered) break;
            }
            if (!covered) {
                selected = 0U;
            }
            if (trace_win32_text()) {
                std::fprintf(stderr, "win32-text selected=%s\n",
                             utf8_from_wide(families[selected]).c_str());
            }
            if (!runs.empty() && runs.back().family == families[selected]) {
                runs.back().text.append(cluster);
            } else {
                runs.push_back({families[selected], cluster});
            }
        }
        for (HFONT candidate : candidates) {
            if (candidate != nullptr) DeleteObject(candidate);
        }
        return runs;
    }
    void reset_bitmap() noexcept {
        if (memory_dc_ != nullptr && old_bitmap_ != nullptr) SelectObject(memory_dc_, old_bitmap_);
        if (bitmap_ != nullptr) DeleteObject(bitmap_);
        if (memory_dc_ != nullptr) DeleteDC(memory_dc_);
        memory_dc_ = nullptr; bitmap_ = nullptr; old_bitmap_ = nullptr; pixels_ = nullptr;
        width_ = 0; height_ = 0;
    }
    void reset() noexcept { reset_bitmap(); }

    static bool decode_png(IWICImagingFactory& factory,
                           const ImageResourceView& resource,
                           DecodedImage& output) {
        if (resource.encoded.empty() || resource.encoded.size() > MAXDWORD ||
            output.width == 0 || output.height == 0 ||
            output.width > std::numeric_limits<std::size_t>::max() / output.height) return false;
        IWICStream* stream{};
        IWICBitmapDecoder* decoder{};
        IWICBitmapFrameDecode* frame{};
        IWICFormatConverter* converter{};
        HRESULT status = factory.CreateStream(&stream);
        if (SUCCEEDED(status)) {
            status = (*stream).InitializeFromMemory(
                reinterpret_cast<BYTE*>(const_cast<std::byte*>(resource.encoded.data())),
                static_cast<DWORD>(resource.encoded.size()));
        }
        if (SUCCEEDED(status)) {
            status = factory.CreateDecoderFromStream(stream, nullptr,
                WICDecodeMetadataCacheOnLoad, &decoder);
        }
        if (SUCCEEDED(status)) status = (*decoder).GetFrame(0, &frame);
        UINT width{};
        UINT height{};
        if (SUCCEEDED(status)) status = (*frame).GetSize(&width, &height);
        if (SUCCEEDED(status) && (width != output.width || height != output.height)) {
            status = E_FAIL;
        }
        if (SUCCEEDED(status)) status = factory.CreateFormatConverter(&converter);
        if (SUCCEEDED(status)) {
            status = (*converter).Initialize(frame, GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
        }
        if (SUCCEEDED(status)) {
            output.pixels.resize(static_cast<std::size_t>(output.width) * output.height);
            const UINT stride = output.width * 4U;
            const std::size_t bytes = output.pixels.size() * sizeof(std::uint32_t);
            if (bytes > MAXDWORD) status = E_OUTOFMEMORY;
            else status = (*converter).CopyPixels(nullptr, stride, static_cast<UINT>(bytes),
                                                reinterpret_cast<BYTE*>(output.pixels.data()));
        }
        if (converter != nullptr) (*converter).Release();
        if (frame != nullptr) (*frame).Release();
        if (decoder != nullptr) (*decoder).Release();
        if (stream != nullptr) (*stream).Release();
        return SUCCEEDED(status);
    }

    HDC memory_dc_{};
    HBITMAP bitmap_{};
    HGDIOBJ old_bitmap_{};
    std::uint32_t* pixels_{};
    int width_{};
    int height_{};
    Size logical_size_{};
    double scale_{1.0};
    std::vector<State> states_;
    ImageMap images_;
    const ImageRegistry* image_registry_{};
    std::uint64_t image_revision_{std::numeric_limits<std::uint64_t>::max()};
    bool images_synchronized_{true};
};

class WindowsHostState final {
public:
    WindowsHostState(std::unique_ptr<Window> model, WindowsHostOptions options)
        : model_(std::move(model)), options_(std::move(options)),
          services_(), session_(*model_, windows_capabilities(), &services_) {
        live_frame_clock_ = CreateWaitableTimerExW(
            nullptr, nullptr, 0, TIMER_ALL_ACCESS);
        if (live_frame_clock_ == nullptr) {
            live_frame_clock_ = CreateWaitableTimerW(nullptr, FALSE, nullptr);
        }
    }

    ~WindowsHostState() {
        if (live_frame_clock_ != nullptr) {
            CancelWaitableTimer(live_frame_clock_);
            CloseHandle(live_frame_clock_);
            live_frame_clock_ = nullptr;
        }
        hide_tooltip();
        if (tooltip_.font != nullptr) DeleteObject(tooltip_.font);
        session_.shutdown();
        services_.shutdown();
        for (const std::wstring& path : private_font_paths_) {
            RemoveFontResourceExW(path.c_str(), FR_PRIVATE, nullptr);
        }
    }

    void bind_window(HWND window) noexcept {
        if (native_phase_ != NativePhase::creating) return;
        hwnd_ = window;
        services_.bind_owner(window);
        native_phase_ = NativePhase::bound;
    }

    bool initialize(HWND window) {
        if (native_phase_ != NativePhase::bound || window == nullptr || hwnd_ != window) {
            return false;
        }
        hwnd_ = window;
        scale_ = query_scale(window);
        load_private_fonts();
        RECT client{};
        GetClientRect(window, &client);
        const Size logical{(client.right - client.left) / scale_,
                           (client.bottom - client.top) / scale_};
        raster_.resize(logical, scale_);
        (*model_).metrics().set_renderer(
            "Win32 DIB CPU · Uniscribe/GDI text · WIC PNG · bundled fonts", true);
        const HostDispatchResult attached = dispatch(HostAttachEvent{logical, scale_});
        if (!attached.accepted()) return false;
        native_phase_ = NativePhase::attached;
        (*model_).set_dispatch_wake_handler(
            gui_forms::detail::BoundMemberFunction<
                void (WindowsHostState::*)() noexcept>(
                    *this, &WindowsHostState::post_managed_dispatch));
        (*model_).set_paint_wake_handler(
            gui_forms::detail::BoundMemberFunction<
                void (WindowsHostState::*)() noexcept>(
                    *this, &WindowsHostState::request_render_wake));
        if (options_.host_ready) {
            options_.host_ready(
                gui_forms::detail::BoundMemberFunction<
                    void (WindowsHostState::*)() noexcept>(
                        *this, &WindowsHostState::post_managed_dispatch),
                gui_forms::detail::BoundMemberFunction<
                    void (WindowsHostState::*)() noexcept>(
                        *this, &WindowsHostState::post_close),
                gui_forms::detail::BoundMemberFunction<
                    HostDialogResult (HostServices::*)(
                        const HostDialogRequest&)>(
                            services_, &HostServices::show_dialog),
                gui_forms::detail::BoundMemberFunction<
                    HostServiceStatus (WindowsHostState::*)(
                        const HostTooltipRequest&)>(
                            *this, &WindowsHostState::show_tooltip),
                gui_forms::detail::BoundMemberFunction<
                    void (WindowsHostState::*)() noexcept>(
                        *this, &WindowsHostState::hide_tooltip),
                gui_forms::detail::BoundMemberFunction<
                    HostClipboardTextResult (HostServices::*)()>(
                        services_, &HostServices::read_clipboard_text),
                gui_forms::detail::BoundMemberFunction<
                    HostServiceStatus (HostServices::*)(std::string_view)>(
                        services_, &HostServices::write_clipboard_text));
        }
        // One FIFO initialization turn runs before presentation. Work posted by
        // those callbacks stays deferred to the ordinary next host turn.
        static_cast<void>((*model_).drain_posted_work());
        if (options_.dispatch_pending) options_.dispatch_pending();
        native_phase_ = NativePhase::ready;
        collect_damage();
        return true;
    }

    bool will_show() noexcept {
        if (native_phase_ != NativePhase::ready) return false;
        native_phase_ = NativePhase::visible;
        return true;
    }

    [[nodiscard]] HANDLE live_frame_clock() const noexcept {
        return live_frame_clock_;
    }

    void mark_live_frame_due() noexcept {
        ++live_clock_signals_;
        live_frame_due_ = true;
    }

    LRESULT message(UINT message, WPARAM wparam, LPARAM lparam) {
        const bool ready = native_phase_ == NativePhase::ready ||
            native_phase_ == NativePhase::visible ||
            native_phase_ == NativePhase::closing;
        if (!ready) {
            switch (message) {
            case WM_GETMINMAXINFO:
                minimum_size(reinterpret_cast<MINMAXINFO*>(lparam));
                return 0;
            case WM_PAINT: {
                PAINTSTRUCT paint_state{};
                BeginPaint(hwnd_, &paint_state);
                EndPaint(hwnd_, &paint_state);
                return 0;
            }
            case WM_DESTROY:
                break;
            default:
                return DefWindowProcW(hwnd_, message, wparam, lparam);
            }
        }
        switch (message) {
        case WM_SIZE: resize(); return 0;
        case WM_DPICHANGED: dpi_changed(wparam, lparam); return 0;
        case WM_ACTIVATE:
            if (trace_win32_input()) {
                std::fprintf(stderr, "win32-input=activate|state:%u|active:%d|focus:%d\n",
                             static_cast<unsigned>(LOWORD(wparam)),
                             GetActiveWindow() == hwnd_, GetFocus() == hwnd_);
                std::fflush(stderr);
            }
            dispatch(HostActivationEvent{LOWORD(wparam) != WA_INACTIVE});
            collect_damage(); return 0;
        case WM_MOUSEACTIVATE:
            if (trace_win32_input()) {
                std::fprintf(stderr, "win32-input=mouse-activate|hit:%u|message:%u|active:%d|focus:%d\n",
                             static_cast<unsigned>(LOWORD(lparam)),
                             static_cast<unsigned>(HIWORD(lparam)),
                             GetActiveWindow() == hwnd_, GetFocus() == hwnd_);
                std::fflush(stderr);
            }
            break;
        case WM_SETCURSOR:
            if (LOWORD(lparam) == HTCLIENT) {
                POINT point{};
                bool applied = false;
                if (GetCursorPos(&point) && ScreenToClient(hwnd_, &point)) {
                    applied = update_cursor({point.x / scale_, point.y / scale_});
                } else {
                    applied = apply_system_cursor(CursorKind::arrow);
                }
                // Only consume WM_SETCURSOR after installing a valid cursor.
                // Returning TRUE after SetCursor(nullptr) leaves the cursor
                // hidden and prevents DefWindowProc from restoring hCursor.
                if (applied) return TRUE;
            }
            break;
        case WM_PAINT:
            // A render wake remains coalesced until the host actually begins
            // the paint transaction. Releasing it here allows content that
            // changes during this frame to request one later frame without
            // turning the posted wake itself into a synchronous paint loop.
            render_dispatch_pending_ = false;
            presentation_pending_ = false;
            paint();
            return 0;
        case WM_ENTERSIZEMOVE:
            // DefWindowProc enters a nested modal loop while a top-level
            // window is moved or sized. The outer waitable-timer loop cannot
            // run there, but the live plane must keep presenting independently
            // of retained layout and input. A modal-loop timer is the native
            // Win32 clock bridge for exactly that interval.
            in_size_move_ = true;
            if ((*model_).has_live_surface_presentations()) {
                SetTimer(hwnd_, modal_live_surface_timer,
                         live_surface_period_milliseconds, nullptr);
            }
            return 0;
        case WM_EXITSIZEMOVE:
            KillTimer(hwnd_, modal_live_surface_timer);
            in_size_move_ = false;
            mark_live_frame_due();
            return 0;
        case managed_dispatch_message:
            static_cast<void>((*model_).drain_posted_work());
            if (options_.dispatch_pending) options_.dispatch_pending();
            collect_damage();
            return 0;
        case render_dispatch_message:
            // Gather damage in this asynchronous turn, but let the outer host
            // loop drain its bounded message batch before presenting. A
            // continuously animated surface must not monopolize the posted-
            // message lane ahead of pointer and keyboard input.
            render_dispatch_pending_ = false;
            collect_damage();
            presentation_pending_ =
                GetUpdateRect(hwnd_, nullptr, FALSE) != FALSE;
            return 0;
        case WM_TIMER:
            if (wparam == modal_live_surface_timer) {
                // Do not enter retained polling/layout from the native modal
                // loop. Sample and present only the newest live generation.
                present_live_surface_updates();
                return 0;
            }
            if (wparam == scheduler_timer) {
                KillTimer(hwnd_, scheduler_timer);
                try {
                    collect_damage();
                } catch (const std::exception& error) {
                    ++native_callback_faults_;
                    std::fprintf(
                        stderr,
                        "gui-forms-host=scheduled-wake-fault|what:%s\n",
                        error.what());
                    std::fflush(stderr);
                } catch (...) {
                    ++native_callback_faults_;
                    std::fprintf(
                        stderr,
                        "gui-forms-host=scheduled-wake-fault|what:unknown\n");
                    std::fflush(stderr);
                }
                return 0;
            }
            break;
        case WM_MOUSEMOVE: pointer(message, PointerAction::move, wparam, lparam); return 0;
        case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN:
            SetFocus(hwnd_); pointer(message, PointerAction::down, wparam, lparam); return 0;
        case WM_LBUTTONDBLCLK: case WM_RBUTTONDBLCLK: case WM_MBUTTONDBLCLK:
            SetFocus(hwnd_); pointer(message, PointerAction::down, wparam, lparam); return 0;
        case WM_LBUTTONUP: case WM_RBUTTONUP: case WM_MBUTTONUP:
            pointer(message, PointerAction::up, wparam, lparam); return 0;
        case WM_MOUSEWHEEL: wheel(wparam, lparam); return 0;
        case WM_KEYDOWN: case WM_SYSKEYDOWN: key(KeyAction::down, wparam, lparam); return 0;
        case WM_KEYUP: case WM_SYSKEYUP: key(KeyAction::up, wparam, lparam); return 0;
        case WM_CHAR: character(static_cast<wchar_t>(wparam)); return 0;
        case WM_GETMINMAXINFO: minimum_size(reinterpret_cast<MINMAXINFO*>(lparam)); return 0;
        case WM_COPYDATA: return automation(reinterpret_cast<const COPYDATASTRUCT*>(lparam));
        case WM_CLOSE: return close();
        case WM_DESTROY:
            if (!closed_) {
                const HostLifecyclePhase phase = session_.snapshot().phase;
                if (phase == HostLifecyclePhase::attached ||
                    phase == HostLifecyclePhase::close_authorized) {
                    dispatch(HostClosedEvent{HostCloseReason::user});
                }
                closed_ = true;
                native_phase_ = NativePhase::closed;
                if (options_.closed) options_.closed();
            }
            (*model_).set_paint_wake_handler({});
            session_.shutdown();
            native_phase_ = NativePhase::shutdown;
            services_.shutdown();
            if (options_.quit_thread_on_close) PostQuitMessage(0);
            return 0;
        default: break;
        }
        return DefWindowProcW(hwnd_, message, wparam, lparam);
    }

    std::string metrics_json() const { return (*model_).metrics_snapshot().to_json(); }
    std::string host_json() const {
        return "{\"session\":" + session_.snapshot().to_json() +
               ",\"services\":" + services_.snapshot().to_json() +
               ",\"compatibility_surfaces\":" +
               detail::compatibility_paint_metrics_json() +
               ",\"live_presentations\":{" +
               "\"clock_signals\":" + std::to_string(live_clock_signals_) +
               ",\"drains\":" + std::to_string(live_presentation_drains_) +
               ",\"updates_sampled\":" +
                   std::to_string(live_updates_sampled_) +
               ",\"updates_presented\":" +
                   std::to_string(live_updates_presented_) +
               ",\"updates_failed\":" +
                   std::to_string(live_updates_failed_) +
               ",\"duration_nanoseconds\":" +
                   std::to_string(live_present_duration_nanoseconds_) +
               ",\"worst_duration_nanoseconds\":" +
                   std::to_string(live_worst_present_duration_nanoseconds_) +
               "}}";
    }

    void present_pending_frame() {
        if (hwnd_ == nullptr) return;
        if (presentation_pending_) {
            presentation_pending_ = false;
            if (GetUpdateRect(hwnd_, nullptr, FALSE) != FALSE) paint();
        }
        if (live_frame_due_) {
            live_frame_due_ = false;
            present_live_surface_updates();
        }
    }

private:
    void post_managed_dispatch() noexcept {
        if (hwnd_ != nullptr) {
            PostMessageW(hwnd_, managed_dispatch_message, 0, 0);
        }
    }

    void post_close() noexcept {
        if (hwnd_ != nullptr) PostMessageW(hwnd_, WM_CLOSE, 0, 0);
    }

    template <class Payload>
    HostDispatchResult dispatch(Payload payload) {
        HostEvent event;
        event.sequence = next_sequence_++;
        event.timestamp_nanoseconds = now_nanoseconds();
        event.payload = std::move(payload);
        return session_.dispatch(std::move(event));
    }

    HostServiceStatus show_tooltip(const HostTooltipRequest& request) {
        hide_tooltip();
        if (request.text.empty()) return {};
        tooltip_.text = wide_from_utf8(request.text);
        if (tooltip_.text.empty()) return {HostServiceError::invalid_utf8};

        WNDCLASSEXW native_class{};
        native_class.cbSize = sizeof(native_class);
        native_class.lpfnWndProc = tooltip_window_procedure;
        native_class.hInstance = GetModuleHandleW(nullptr);
        native_class.hCursor = load_system_cursor(IDC_ARROW);
        native_class.lpszClassName = tooltip_class_name;
        if (RegisterClassExW(&native_class) == 0 &&
            GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
            return {HostServiceError::backend_failure};
        }

        if (tooltip_.font != nullptr) DeleteObject(tooltip_.font);
        const int font_height = -std::max(12, static_cast<int>(std::lround(13.0 * scale_)));
        tooltip_.font = CreateFontW(font_height, 0, 0, 0, FW_NORMAL, FALSE, FALSE,
                                    FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                    DEFAULT_PITCH | FF_SWISS, L"Lucida Grande");
        HDC dc = GetDC(hwnd_);
        HGDIOBJ old_font = tooltip_.font ? SelectObject(dc, tooltip_.font) : nullptr;
        RECT measured{0, 0, static_cast<LONG>(std::lround(360.0 * scale_)), 0};
        DrawTextW(dc, tooltip_.text.c_str(), -1, &measured,
                  DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX | DT_LEFT);
        if (old_font) SelectObject(dc, old_font);
        ReleaseDC(hwnd_, dc);
        const int width = std::clamp<int>(measured.right - measured.left + 16,
                                          40, static_cast<int>(std::lround(376.0 * scale_)));
        const int height = std::max(24L, measured.bottom - measured.top + 10);

        POINT anchor{static_cast<LONG>(std::lround(request.anchor.x * scale_)),
                     static_cast<LONG>(std::lround(request.anchor.y * scale_))};
        ClientToScreen(hwnd_, &anchor);
        RECT work{};
        MONITORINFO monitor{};
        monitor.cbSize = sizeof(monitor);
        if (GetMonitorInfoW(MonitorFromPoint(anchor, MONITOR_DEFAULTTONEAREST),
                            &monitor)) {
            work = monitor.rcWork;
        } else {
            SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
        }
        const int x = std::clamp<int>(anchor.x, work.left,
                                      std::max(work.left, work.right - width));
        const int y = std::clamp<int>(anchor.y, work.top,
                                      std::max(work.top, work.bottom - height));
        tooltip_.window = CreateWindowExW(
            WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
            tooltip_class_name, tooltip_.text.c_str(), WS_POPUP,
            x, y, width, height, hwnd_, nullptr, GetModuleHandleW(nullptr), &tooltip_);
        if (tooltip_.window == nullptr) return {HostServiceError::backend_failure};
        ShowWindow(tooltip_.window, SW_SHOWNOACTIVATE);
        UpdateWindow(tooltip_.window);
        if (request.duration_milliseconds != 0U) {
            SetTimer(tooltip_.window, 1,
                     std::max<UINT>(1U, request.duration_milliseconds), nullptr);
        }
        return {};
    }

    void hide_tooltip() noexcept {
        if (tooltip_.window != nullptr && IsWindow(tooltip_.window)) {
            DestroyWindow(tooltip_.window);
        }
        tooltip_.window = nullptr;
        tooltip_.text.clear();
    }

    void resize() {
        RECT client{};
        GetClientRect(hwnd_, &client);
        const Size logical{std::max(1.0, (client.right - client.left) / scale_),
                           std::max(1.0, (client.bottom - client.top) / scale_)};
        raster_.resize(logical, scale_);
        dispatch(HostResizeEvent{logical});
        collect_damage();
    }

    void dpi_changed(WPARAM wparam, LPARAM lparam) {
        const double next = std::max(1.0, HIWORD(wparam) / 96.0);
        if (next != scale_) { scale_ = next; dispatch(HostScaleEvent{scale_}); }
        if (const RECT* suggested = reinterpret_cast<const RECT*>(lparam)) {
            SetWindowPos(hwnd_, nullptr, (*suggested).left, (*suggested).top,
                         (*suggested).right - (*suggested).left,
                         (*suggested).bottom - (*suggested).top,
                         SWP_NOACTIVATE | SWP_NOZORDER);
        }
        resize();
    }

    void collect_damage() {
        static_cast<void>((*model_).poll_frame_schedule(FrameClock::now()));
        DamageRegion damage = (*model_).take_damage();
        if (!damage.empty()) {
            const Rect bounds = damage.bounds();
            pending_damage_.add(bounds);
            RECT client{};
            GetClientRect(hwnd_, &client);
            RECT native_damage{
                std::clamp<LONG>(static_cast<LONG>(std::floor(bounds.x * scale_)),
                                 client.left, client.right),
                std::clamp<LONG>(static_cast<LONG>(std::floor(bounds.y * scale_)),
                                 client.top, client.bottom),
                std::clamp<LONG>(static_cast<LONG>(std::ceil(
                                     (bounds.x + bounds.width) * scale_)),
                                 client.left, client.right),
                std::clamp<LONG>(static_cast<LONG>(std::ceil(
                                     (bounds.y + bounds.height) * scale_)),
                                 client.top, client.bottom)};
            if (native_damage.right > native_damage.left &&
                native_damage.bottom > native_damage.top) {
                InvalidateRect(hwnd_, &native_damage, FALSE);
            }
        }
        KillTimer(hwnd_, scheduler_timer);
        if (const std::optional<FrameTime> wake = (*model_).next_wake()) {
            const std::chrono::milliseconds::rep milliseconds =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    *wake - FrameClock::now()).count();
            SetTimer(hwnd_, scheduler_timer,
                     static_cast<UINT>(std::clamp<std::int64_t>(milliseconds, 1, 60'000)), nullptr);
        }
        const bool needs_live_clock = (*model_).has_live_surface_presentations();
        if (needs_live_clock && !live_clock_armed_) {
            LARGE_INTEGER first_due{};
            first_due.QuadPart = -static_cast<LONGLONG>(
                live_surface_period_milliseconds) * 10'000LL;
            live_clock_armed_ = live_frame_clock_ != nullptr &&
                SetWaitableTimer(live_frame_clock_, &first_due,
                    live_surface_period_milliseconds, nullptr, nullptr,
                    FALSE) != FALSE;
        } else if (!needs_live_clock && live_clock_armed_) {
            CancelWaitableTimer(live_frame_clock_);
            live_clock_armed_ = false;
            live_frame_due_ = false;
        }
    }

    void request_render_wake() noexcept {
        if (hwnd_ == nullptr || render_dispatch_pending_ ||
            presentation_pending_) return;
        render_dispatch_pending_ = true;
        if (!PostMessageW(hwnd_, render_dispatch_message, 0, 0)) {
            render_dispatch_pending_ = false;
        }
    }

    void present_live_surface_updates() {
        ++live_presentation_drains_;
        const std::chrono::steady_clock::time_point started =
            std::chrono::steady_clock::now();
        std::vector<LiveSurfacePresentation> updates =
            (*model_).take_live_surface_presentations();
        live_updates_sampled_ += updates.size();
        if (!(*model_).has_live_surface_presentations() && live_clock_armed_) {
            CancelWaitableTimer(live_frame_clock_);
            live_clock_armed_ = false;
        }
        if (updates.empty() || hwnd_ == nullptr) return;
        HDC target = GetDC(hwnd_);
        if (target == nullptr) {
            live_updates_failed_ += updates.size();
            return;
        }
        for (const gui_forms::LiveSurfacePresentation& update : updates) {
            if (raster_.present_live_surface(target, update)) {
                ++live_updates_presented_;
            } else {
                ++live_updates_failed_;
            }
        }
        ReleaseDC(hwnd_, target);
        const std::uint64_t duration = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now() - started).count());
        live_present_duration_nanoseconds_ += duration;
        live_worst_present_duration_nanoseconds_ =
            (std::max)(live_worst_present_duration_nanoseconds_, duration);
    }

    void paint() {
        PAINTSTRUCT paint_state{};
        HDC dc = BeginPaint(hwnd_, &paint_state);
        const std::chrono::steady_clock::time_point started =
            std::chrono::steady_clock::now();
        if (pending_damage_.empty()) {
            pending_damage_.add({paint_state.rcPaint.left / scale_, paint_state.rcPaint.top / scale_,
                                 (paint_state.rcPaint.right - paint_state.rcPaint.left) / scale_,
                                 (paint_state.rcPaint.bottom - paint_state.rcPaint.top) / scale_});
        }
        raster_.begin_frame();
        static_cast<void>(raster_.synchronize_images((*model_).image_resources()));
        const std::optional<PaintReceipt> receipt =
            (*model_).paint(raster_, pending_damage_.bounds());
        const bool presented = receipt && raster_.present(dc, paint_state.rcPaint);
        EndPaint(hwnd_, &paint_state);
        const std::chrono::nanoseconds elapsed =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now() - started);
        if (presented &&
            (*model_).notify_presented(*receipt,
                                     static_cast<std::uint64_t>(elapsed.count()))) {
            pending_damage_.clear();
        }
        collect_damage();
    }

    Point client_point(LPARAM lparam) const noexcept {
        return {GET_X_LPARAM(lparam) / scale_, GET_Y_LPARAM(lparam) / scale_};
    }

    void pointer(UINT message, PointerAction action, WPARAM wparam, LPARAM lparam) {
        PointerEvent event;
        event.action = action;
        event.button = pointer_button(message, wparam);
        event.position = client_point(lparam);
        event.modifiers = modifiers();
        event.pointer_id = 1;
        event.click_count = message == WM_LBUTTONDBLCLK ||
            message == WM_RBUTTONDBLCLK || message == WM_MBUTTONDBLCLK ? 2U : 1U;
        if (trace_win32_input()) {
            const Control::Ptr target = (*model_).hit_test(event.position);
            const std::string_view target_id = target ? (*target).stable_id().value() : std::string_view{"<none>"};
            std::fprintf(stderr,
                         "win32-input=pointer|message:%u|action:%u|x:%.2f|y:%.2f|target:%.*s|active:%d|focus:%d|capture:%d\n",
                         static_cast<unsigned>(message), static_cast<unsigned>(action),
                         event.position.x, event.position.y,
                         static_cast<int>(target_id.size()), target_id.data(),
                         GetActiveWindow() == hwnd_, GetFocus() == hwnd_, GetCapture() == hwnd_);
            std::fflush(stderr);
        }
        dispatch(std::move(event));
        synchronize_capture();
        collect_damage();
    }

    void wheel(WPARAM wparam, LPARAM lparam) {
        POINT point{GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};
        ScreenToClient(hwnd_, &point);
        PointerEvent event;
        event.action = PointerAction::wheel;
        event.position = {point.x / scale_, point.y / scale_};
        event.wheel_delta = {0.0, GET_WHEEL_DELTA_WPARAM(wparam) / static_cast<double>(WHEEL_DELTA)};
        event.modifiers = modifiers();
        event.pointer_id = 1;
        dispatch(std::move(event));
        collect_damage();
    }

    void key(KeyAction action, WPARAM wparam, LPARAM lparam) {
        KeyEvent event;
        event.action = action;
        event.physical_key = physical_key(wparam);
        event.modifiers = modifiers();
        event.repeat = action == KeyAction::down && (lparam & (1LL << 30)) != 0;
        dispatch(std::move(event));
        collect_damage();
    }

    void character(wchar_t character) {
        if (character >= 0xD800 && character <= 0xDBFF) {
            pending_high_surrogate_ = character;
            return;
        }
        std::wstring text;
        if (pending_high_surrogate_ != 0) {
            if (character >= 0xDC00 && character <= 0xDFFF) text.push_back(pending_high_surrogate_);
            pending_high_surrogate_ = 0;
        }
        text.push_back(character);
        TextInputEvent event;
        event.text_utf8 = utf8_from_wide(text);
        if (!event.text_utf8.empty()) dispatch(std::move(event));
        collect_damage();
    }

    void synchronize_capture() {
        const bool requested = (*model_).captured_control() != nullptr;
        if (requested && GetCapture() != hwnd_) SetCapture(hwnd_);
        if (!requested && GetCapture() == hwnd_) ReleaseCapture();
    }

    bool update_cursor(Point position) const {
        const Control::Ptr target = (*model_).hit_test(position);
        const CursorKind cursor = target ? (*target).effective_cursor() : CursorKind::arrow;
        // The class cursor is authoritative for the ordinary pointer. Let
        // DefWindowProc complete WM_SETCURSOR for that case instead of writing
        // process-global cursor state again from every WM_MOUSEMOVE turn.
        if (cursor == CursorKind::arrow) return false;
        return apply_system_cursor(cursor);
    }

    void minimum_size(MINMAXINFO* info) const {
        if (info == nullptr) return;
        RECT rect{0, 0,
                  static_cast<LONG>(std::ceil(options_.minimum_size.width * scale_)),
                  static_cast<LONG>(std::ceil(options_.minimum_size.height * scale_))};
        AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);
        (*info).ptMinTrackSize.x = rect.right - rect.left;
        (*info).ptMinTrackSize.y = rect.bottom - rect.top;
    }

    LRESULT close() {
        HostCloseRequest request{HostCloseReason::user, false};
        if (options_.close_request) options_.close_request(request);
        const HostDispatchResult result = dispatch(request);
        if (result.accepted() && result.close_allowed) {
            native_phase_ = NativePhase::closing;
            DestroyWindow(hwnd_);
        }
        return 0;
    }

    LRESULT automation(const COPYDATASTRUCT* data) {
        if (!options_.automation_enabled || data == nullptr || (*data).lpData == nullptr ||
            (*data).cbData == 0 || (*data).cbData > maximum_automation_command) return FALSE;
        const char* bytes = static_cast<const char*>((*data).lpData);
        std::string command(bytes, bytes + (*data).cbData);
        while (!command.empty() && command.back() == '\0') command.pop_back();
        constexpr std::string_view prefix = "GUI.Forms.Automation/1 ";
        if (!command.starts_with(prefix)) return FALSE;
        command.erase(0, prefix.size());
        if (command == "snapshot") {
            const DispatcherSnapshot dispatcher = (*model_).dispatcher_snapshot();
            std::fprintf(
                stdout,
                "{\"automation\":\"snapshot\",\"window\":%s,\"host\":%s,"
                "\"dispatcher\":{\"posted\":%llu,\"invoked\":%llu,"
                "\"cancelled\":%llu,\"faulted\":%llu,"
                "\"synchronous_invocations\":%llu,\"inline_invocations\":%llu,"
                "\"marshalled_invocations\":%llu,\"pending\":%zu,"
                "\"maximum_pending\":%zu,\"accepting\":%s}}\n",
                metrics_json().c_str(), host_json().c_str(),
                static_cast<unsigned long long>(dispatcher.posted),
                static_cast<unsigned long long>(dispatcher.invoked),
                static_cast<unsigned long long>(dispatcher.cancelled),
                static_cast<unsigned long long>(dispatcher.faulted),
                static_cast<unsigned long long>(dispatcher.synchronous_invocations),
                static_cast<unsigned long long>(dispatcher.inline_invocations),
                static_cast<unsigned long long>(dispatcher.marshalled_invocations),
                dispatcher.pending, dispatcher.maximum_pending,
                dispatcher.accepting ? "true" : "false");
            std::fflush(stdout);
            return TRUE;
        }
        constexpr std::string_view capture = "capture";
        if (command == capture || command.starts_with("capture ")) {
            const std::wstring path = command.size() == capture.size()
                ? executable_directory() + L"gallery-automation.bmp"
                : wide_from_utf8(command.substr(capture.size() + 1U));
            if (path.empty()) return FALSE;
            const bool saved = raster_.save_bmp(path);
            std::fprintf(stdout,
                         "{\"automation\":\"capture\",\"saved\":%s,\"explicit\":%s}\n",
                         saved ? "true" : "false",
                         command.size() == capture.size() ? "false" : "true");
            std::fflush(stdout);
            return saved ? TRUE : FALSE;
        }
        if (command == "close") { PostMessageW(hwnd_, WM_CLOSE, 0, 0); return TRUE; }
        constexpr std::string_view move_at = "move ";
        if (command.starts_with(move_at)) {
            std::istringstream input(command.substr(move_at.size()));
            std::string id;
            double local_x{};
            double local_y{};
            if (!(input >> id >> local_x >> local_y) || !std::isfinite(local_x) ||
                !std::isfinite(local_y)) return FALSE;
            const std::shared_ptr<gui_forms::Control> control = options_.automation_resolve
                ? options_.automation_resolve(id) : (*model_).find(id);
            if (!control || !(*control).eligible_for_input()) return FALSE;
            const Rect bounds = (*control).absolute_bounds();
            const Point point{bounds.x + std::clamp(local_x, 0.0, bounds.width),
                              bounds.y + std::clamp(local_y, 0.0, bounds.height)};
            const bool handled = dispatch(PointerEvent{
                PointerAction::move, PointerButton::none, point, {},
                Modifier::none, 1}).handled;
            collect_damage();
            std::fprintf(stdout,
                         "{\"automation\":\"move\",\"id\":\"%s\",\"handled\":%s}\n",
                         id.c_str(), handled ? "true" : "false");
            std::fflush(stdout);
            return TRUE;
        }
        constexpr std::string_view focus = "focus ";
        if (command.starts_with(focus)) {
            const std::string id = command.substr(focus.size());
            const std::shared_ptr<gui_forms::Control> control = options_.automation_resolve
                ? options_.automation_resolve(id) : (*model_).find(id);
            if (!control || !(*control).eligible_for_input() ||
                !(*model_).request_focus(control)) return FALSE;
            std::fprintf(stdout, "{\"automation\":\"focus\",\"id\":\"%s\"}\n",
                         id.c_str());
            std::fflush(stdout);
            return TRUE;
        }
        constexpr std::string_view text_input = "text ";
        if (command.starts_with(text_input)) {
            const std::string payload = command.substr(text_input.size());
            const std::size_t separator = payload.find(' ');
            if (separator == std::string::npos) return FALSE;
            const std::string id = payload.substr(0, separator);
            const std::string text = payload.substr(separator + 1U);
            const std::shared_ptr<gui_forms::Control> control = options_.automation_resolve
                ? options_.automation_resolve(id) : (*model_).find(id);
            if (!control || !(*control).eligible_for_input() ||
                !(*model_).request_focus(control)) return FALSE;
            const bool handled = dispatch(TextInputEvent{text}).handled;
            collect_damage();
            std::fprintf(stdout,
                         "{\"automation\":\"text\",\"id\":\"%s\",\"bytes\":%zu,\"handled\":%s}\n",
                         id.c_str(), text.size(), handled ? "true" : "false");
            std::fflush(stdout);
            return TRUE;
        }
        constexpr std::string_view key_input = "key ";
        if (command.starts_with(key_input)) {
            std::istringstream input(command.substr(key_input.size()));
            std::string id;
            std::uint32_t physical{};
            std::string action;
            std::uint32_t modifiers{};
            if (!(input >> id >> physical >> action) ||
                (action != "down" && action != "up")) return FALSE;
            if (!(input >> modifiers)) modifiers = 0;
            const std::shared_ptr<gui_forms::Control> control = options_.automation_resolve
                ? options_.automation_resolve(id) : (*model_).find(id);
            if (!control || !(*control).eligible_for_input() ||
                !(*model_).request_focus(control)) return FALSE;
            KeyEvent event;
            event.action = action == "down" ? KeyAction::down : KeyAction::up;
            event.physical_key = physical;
            event.modifiers = static_cast<Modifier>(modifiers);
            const bool handled = dispatch(std::move(event)).handled;
            collect_damage();
            std::fprintf(stdout,
                         "{\"automation\":\"key\",\"id\":\"%s\",\"physical\":%u,\"action\":\"%s\",\"modifiers\":%u,\"handled\":%s}\n",
                         id.c_str(), physical, action.c_str(), modifiers,
                         handled ? "true" : "false");
            std::fflush(stdout);
            return TRUE;
        }
        constexpr std::string_view activate = "activate ";
        if (command.starts_with(activate)) {
            const std::string id = command.substr(activate.size());
            const std::shared_ptr<gui_forms::Control> control = options_.automation_resolve
                ? options_.automation_resolve(id) : (*model_).find(id);
            if (!control || !(*control).eligible_for_input()) {
                std::fprintf(stdout,
                             "{\"automation\":\"activate\",\"id\":\"%s\",\"accepted\":false,\"reason\":\"%s\"}\n",
                             id.c_str(), !control ? "not-found" : "ineligible");
                std::fflush(stdout);
                return FALSE;
            }
            (*control).on_activate();
            collect_damage();
            std::fprintf(stdout,
                         "{\"automation\":\"activate\",\"id\":\"%s\",\"stable_id\":\"%s\"}\n",
                         id.c_str(), (*control).stable_id().value().data());
            std::fflush(stdout);
            return TRUE;
        }
        constexpr std::string_view click_at = "click-at ";
        if (command.starts_with(click_at)) {
            std::istringstream input(command.substr(click_at.size()));
            std::string id;
            double local_x{};
            double local_y{};
            if (!(input >> id >> local_x >> local_y) || !std::isfinite(local_x) ||
                !std::isfinite(local_y)) return FALSE;
            const std::shared_ptr<gui_forms::Control> control = options_.automation_resolve
                ? options_.automation_resolve(id) : (*model_).find(id);
            if (!control || !(*control).eligible_for_input()) {
                std::fprintf(stdout,
                             "{\"automation\":\"click-at\",\"id\":\"%s\",\"accepted\":false,\"reason\":\"%s\"}\n",
                             id.c_str(), !control ? "not-found" : "ineligible");
                std::fflush(stdout);
                return FALSE;
            }
            const Rect bounds = (*control).absolute_bounds();
            const Point point{bounds.x + std::clamp(local_x, 0.0, bounds.width),
                              bounds.y + std::clamp(local_y, 0.0, bounds.height)};
            const bool handled_down = dispatch(PointerEvent{
                PointerAction::down, PointerButton::primary, point, {},
                Modifier::none, 1}).handled;
            const bool handled_up = dispatch(PointerEvent{
                PointerAction::up, PointerButton::primary, point, {},
                Modifier::none, 1}).handled;
            synchronize_capture();
            collect_damage();
            std::fprintf(stdout,
                         "{\"automation\":\"click-at\",\"id\":\"%s\",\"local_x\":%.3f,\"local_y\":%.3f,\"handled\":%s}\n",
                         id.c_str(), local_x, local_y,
                         (handled_down || handled_up) ? "true" : "false");
            std::fflush(stdout);
            return TRUE;
        }
        constexpr std::string_view drag = "drag ";
        if (command.starts_with(drag)) {
            std::istringstream input(command.substr(drag.size()));
            std::string id;
            double start_x{};
            double start_y{};
            double end_x{};
            double end_y{};
            unsigned requested_steps{8U};
            if (!(input >> id >> start_x >> start_y >> end_x >> end_y) ||
                !std::isfinite(start_x) || !std::isfinite(start_y) ||
                !std::isfinite(end_x) || !std::isfinite(end_y)) return FALSE;
            if (!(input >> requested_steps)) requested_steps = 8U;
            const unsigned steps = std::clamp(requested_steps, 1U, 256U);
            const std::shared_ptr<gui_forms::Control> control = options_.automation_resolve
                ? options_.automation_resolve(id) : (*model_).find(id);
            if (!control || !(*control).eligible_for_input()) {
                std::fprintf(stdout,
                             "{\"automation\":\"drag\",\"id\":\"%s\",\"accepted\":false,\"reason\":\"%s\"}\n",
                             id.c_str(), !control ? "not-found" : "ineligible");
                std::fflush(stdout);
                return FALSE;
            }
            const Rect bounds = (*control).absolute_bounds();
            const Point start{
                bounds.x + std::clamp(start_x, 0.0, bounds.width),
                bounds.y + std::clamp(start_y, 0.0, bounds.height)};
            const Point end{
                bounds.x + std::clamp(end_x, 0.0, bounds.width),
                bounds.y + std::clamp(end_y, 0.0, bounds.height)};
            bool handled = dispatch(PointerEvent{
                PointerAction::down, PointerButton::primary, start, {},
                Modifier::none, 1}).handled;
            for (unsigned index = 1; index <= steps; ++index) {
                const double ratio = static_cast<double>(index) /
                                     static_cast<double>(steps);
                const Point position{start.x + (end.x - start.x) * ratio,
                                     start.y + (end.y - start.y) * ratio};
                handled = dispatch(PointerEvent{
                    PointerAction::move, PointerButton::primary, position, {},
                    Modifier::none, 1}).handled || handled;
            }
            handled = dispatch(PointerEvent{
                PointerAction::up, PointerButton::primary, end, {},
                Modifier::none, 1}).handled || handled;
            synchronize_capture();
            collect_damage();
            std::fprintf(stdout,
                         "{\"automation\":\"drag\",\"id\":\"%s\",\"steps\":%u,\"handled\":%s}\n",
                         id.c_str(), steps, handled ? "true" : "false");
            std::fflush(stdout);
            return TRUE;
        }
        constexpr std::string_view click = "click ";
        if (command.starts_with(click)) {
            const std::string id = command.substr(click.size());
            std::fprintf(stdout, "{\"automation\":\"click-phase\",\"id\":\"%s\",\"phase\":\"request\"}\n",
                         id.c_str());
            std::fflush(stdout);
            const std::shared_ptr<gui_forms::Control> control = options_.automation_resolve
                ? options_.automation_resolve(id) : (*model_).find(id);
            if (!control || !(*control).eligible_for_input()) return FALSE;
            std::fprintf(stdout, "{\"automation\":\"click-phase\",\"id\":\"%s\",\"phase\":\"resolved\"}\n",
                         id.c_str());
            std::fflush(stdout);
            const Rect bounds = (*control).absolute_bounds();
            const Point center{bounds.x + bounds.width / 2.0, bounds.y + bounds.height / 2.0};
            const Control::Ptr hit = (*model_).hit_test(center);
            std::fprintf(stdout,
                         "{\"automation\":\"click-target\",\"id\":\"%s\",\"resolved\":\"%s\",\"hit\":\"%s\",\"x\":%.3f,\"y\":%.3f}\n",
                         id.c_str(), (*control).stable_id().value().data(),
                         hit ? (*hit).stable_id().value().data() : "", center.x,
                         center.y);
            std::fflush(stdout);
            PointerEvent down{PointerAction::down, PointerButton::primary, center,
                              {}, Modifier::none, 1};
            PointerEvent up{PointerAction::up, PointerButton::primary, center,
                            {}, Modifier::none, 1};
            const bool handled_down = dispatch(std::move(down)).handled;
            std::fprintf(stdout, "{\"automation\":\"click-phase\",\"id\":\"%s\",\"phase\":\"down\"}\n",
                         id.c_str());
            std::fflush(stdout);
            const bool handled_up = dispatch(std::move(up)).handled;
            std::fprintf(stdout, "{\"automation\":\"click-phase\",\"id\":\"%s\",\"phase\":\"up\"}\n",
                         id.c_str());
            std::fflush(stdout);
            synchronize_capture();
            collect_damage();
            std::fprintf(stdout, "{\"automation\":\"click\",\"id\":\"%s\",\"handled\":%s}\n",
                         id.c_str(), (handled_down || handled_up) ? "true" : "false");
            std::fflush(stdout);
            return TRUE;
        }
        return FALSE;
    }

    void load_private_fonts() {
        const std::wstring directory = executable_directory();
        if (directory.empty()) return;
        constexpr std::array<const wchar_t*, 12> names{
            L"PortsmouthRapids.ttf", L"PortsmouthRapids-Bold.ttf",
            L"Carlito-Regular.ttf", L"Carlito-Bold.ttf",
            L"Carlito-Italic.ttf", L"Carlito-BoldItalic.ttf",
            L"Cousine-Regular.ttf", L"Cousine-Bold.ttf",
            L"Cousine-Italic.ttf", L"Cousine-BoldItalic.ttf",
            L"NotoSansCJKjp-Regular.otf", L"NotoEmoji-Regular.ttf"};
        private_font_paths_.reserve(names.size());
        for (const wchar_t* name : names) {
            std::wstring path = directory + L"fonts\\" + name;
            if (AddFontResourceExW(path.c_str(), FR_PRIVATE, nullptr) > 0) {
                private_font_paths_.push_back(std::move(path));
            }
        }
    }

    static std::wstring executable_directory() {
        wchar_t executable[MAX_PATH]{};
        const DWORD count = GetModuleFileNameW(nullptr, executable, MAX_PATH);
        if (count == 0 || count >= MAX_PATH) return {};
        std::wstring directory(executable, count);
        const std::wstring::size_type slash = directory.find_last_of(L"\\/");
        directory.resize(slash == std::wstring::npos ? 0 : slash + 1);
        return directory;
    }

    enum class NativePhase : std::uint8_t {
        creating,
        bound,
        attached,
        ready,
        visible,
        closing,
        closed,
        shutdown,
    };

    std::unique_ptr<Window> model_;
    WindowsHostOptions options_;
    WindowsHostServices services_;
    HostSession session_;
    HWND hwnd_{};
    DibPainter raster_;
    DamageRegion pending_damage_;
    double scale_{1.0};
    std::uint64_t next_sequence_{1};
    std::uint64_t native_callback_faults_{};
    wchar_t pending_high_surrogate_{};
    bool render_dispatch_pending_{};
    bool presentation_pending_{};
    HANDLE live_frame_clock_{};
    bool live_clock_armed_{};
    bool live_frame_due_{};
    std::uint64_t live_clock_signals_{};
    std::uint64_t live_presentation_drains_{};
    std::uint64_t live_updates_sampled_{};
    std::uint64_t live_updates_presented_{};
    std::uint64_t live_updates_failed_{};
    std::uint64_t live_present_duration_nanoseconds_{};
    std::uint64_t live_worst_present_duration_nanoseconds_{};
    bool in_size_move_{};
    bool closed_{};
    NativePhase native_phase_{NativePhase::creating};
    std::vector<std::wstring> private_font_paths_;
    TooltipPopup tooltip_;
};

LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    WindowsHostState* state = reinterpret_cast<WindowsHostState*>(
        GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        const CREATESTRUCTW* create =
            reinterpret_cast<const CREATESTRUCTW*>(lparam);
        state = static_cast<WindowsHostState*>((*create).lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
        (*state).bind_window(window);
    }
    if (state != nullptr) return (*state).message(message, wparam, lparam);
    return DefWindowProcW(window, message, wparam, lparam);
}

} // namespace

HostCapabilities windows_capabilities() {
    return {HostCapabilities::current_protocol_version,
            "win32-dib",
            HostCapability::lifecycle |
                HostCapability::scale_notifications |
                HostCapability::scheduled_wake |
                HostCapability::pointer_input |
                HostCapability::keyboard_input |
                HostCapability::text_composition |
                HostCapability::monitor_geometry |
                HostCapability::pointer_capture |
                HostCapability::cursor |
                HostCapability::clipboard |
                HostCapability::dialogs |
                HostCapability::sound_cues};
}

int run_windows(std::unique_ptr<Window> model, WindowsHostOptions options) {
    if (!model) return 2;
    const HRESULT com_status = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    enable_best_dpi_awareness();
    HINSTANCE instance = GetModuleHandleW(nullptr);
    WNDCLASSEXW native_class{};
    native_class.cbSize = sizeof(native_class);
    native_class.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    native_class.lpfnWndProc = window_procedure;
    native_class.hInstance = instance;
    native_class.hCursor = load_system_cursor(IDC_ARROW);
    native_class.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    native_class.hbrBackground = nullptr;
    native_class.lpszClassName = window_class_name;
    if (RegisterClassExW(&native_class) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        if (SUCCEEDED(com_status)) CoUninitialize();
        return 3;
    }

    WindowsHostState state(std::move(model), options);
    const DWORD style = options.popup_window ? (WS_POPUP | WS_BORDER) : WS_OVERLAPPEDWINDOW;
    RECT frame{0, 0, static_cast<LONG>(std::ceil(options.initial_size.width)),
               static_cast<LONG>(std::ceil(options.initial_size.height))};
    AdjustWindowRectEx(&frame, style, FALSE, 0);
    const std::wstring title = wide_from_utf8(options.title);
    const int initial_x = options.popup_window
        ? static_cast<int>(std::lround(options.initial_position.x)) : CW_USEDEFAULT;
    const int initial_y = options.popup_window
        ? static_cast<int>(std::lround(options.initial_position.y)) : CW_USEDEFAULT;
    HWND window = CreateWindowExW(0, window_class_name, title.c_str(),
                                  style, initial_x, initial_y,
                                  frame.right - frame.left, frame.bottom - frame.top,
                                  nullptr, nullptr, instance, &state);
    if (window == nullptr) {
        if (SUCCEEDED(com_status)) CoUninitialize();
        return 4;
    }
    if (!state.initialize(window)) {
        DestroyWindow(window);
        if (SUCCEEDED(com_status)) CoUninitialize();
        return 5;
    }
    if (!state.will_show()) {
        DestroyWindow(window);
        if (SUCCEEDED(com_status)) CoUninitialize();
        return 5;
    }
    ShowWindow(window, SW_SHOWNORMAL);
    UpdateWindow(window);
    if (options.close_after_launch_for_testing) {
        PostMessageW(window, WM_CLOSE, 0, 0);
    }

    MSG message{};
    bool quit{};
    while (!quit) {
        HANDLE clock = state.live_frame_clock();
        const DWORD handle_count = clock == nullptr ? 0U : 1U;
        const DWORD wait = MsgWaitForMultipleObjectsEx(
            handle_count, handle_count == 0U ? nullptr : &clock, INFINITE,
            QS_ALLINPUT, MWMO_INPUTAVAILABLE);
        if (wait == WAIT_FAILED) break;
        if (handle_count != 0U && wait == WAIT_OBJECT_0) {
            state.mark_live_frame_due();
        }

        // Input and managed callbacks receive a bounded turn, not an
        // unbounded queue drain. Presentation therefore has a maximum queue
        // latency even while pointer or plugin traffic remains continuous.
        const std::chrono::steady_clock::time_point batch_started =
            std::chrono::steady_clock::now();
        for (std::size_t drained = 0U; drained < 32U; ++drained) {
            MSG pending{};
            if (!PeekMessageW(&pending, nullptr, 0, 0, PM_REMOVE)) break;
            if (pending.message == WM_QUIT) {
                message = pending;
                quit = true;
                break;
            }
            TranslateMessage(&pending);
            DispatchMessageW(&pending);
            if (!options.quit_thread_on_close && !IsWindow(window)) {
                quit = true;
                break;
            }
            if (std::chrono::steady_clock::now() - batch_started >=
                std::chrono::milliseconds(4)) {
                break;
            }
        }
        if (clock != nullptr &&
            WaitForSingleObject(clock, 0U) == WAIT_OBJECT_0) {
            state.mark_live_frame_due();
        }
        if (!quit && IsWindow(window)) state.present_pending_frame();
        if (!options.quit_thread_on_close && !IsWindow(window)) break;
    }
    const std::string metrics = state.metrics_json();
    const std::string host = state.host_json();
    if (options.print_metrics_on_close) {
        std::fprintf(stdout, "{\"window\":%s,\"host\":%s}\n",
                     metrics.c_str(), host.c_str());
        std::fflush(stdout);
    }
    if (options.final_snapshot) options.final_snapshot(metrics, host);
    if (SUCCEEDED(com_status)) CoUninitialize();
    return static_cast<int>(message.wParam);
}

int run_windows_application(std::vector<WindowsApplicationWindow> windows) {
    if (windows.empty()) return 2;
    std::unordered_set<std::string> identities;
    std::size_t primary_count{};
    for (const WindowsApplicationWindow& entry : windows) {
        if (!entry.model || entry.stable_id.empty() ||
            !identities.insert(entry.stable_id).second ||
            (!entry.owner_id.empty() && entry.owner_id == entry.stable_id)) {
            return 2;
        }
        if (entry.owner_id.empty() && !entry.tool_window) ++primary_count;
    }
    for (const WindowsApplicationWindow& entry : windows) {
        if (!entry.owner_id.empty() && !identities.contains(entry.owner_id)) return 2;
    }
    if (primary_count != 1U) return 2;

    const HRESULT com_status = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    enable_best_dpi_awareness();
    HINSTANCE instance = GetModuleHandleW(nullptr);
    WNDCLASSEXW native_class{};
    native_class.cbSize = sizeof(native_class);
    native_class.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    native_class.lpfnWndProc = window_procedure;
    native_class.hInstance = instance;
    native_class.hCursor = load_system_cursor(IDC_ARROW);
    native_class.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    native_class.hbrBackground = nullptr;
    native_class.lpszClassName = window_class_name;
    if (RegisterClassExW(&native_class) == 0 &&
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        if (SUCCEEDED(com_status)) CoUninitialize();
        return 3;
    }

    std::vector<std::unique_ptr<WindowsHostState>> states(windows.size());
    std::vector<HWND> handles(windows.size(), nullptr);
    std::size_t created_count{};
    while (created_count < windows.size()) {
        bool made_progress{};
        for (std::size_t index = 0; index < windows.size(); ++index) {
            if (handles[index] != nullptr) continue;
            WindowsApplicationWindow& entry = windows[index];
            HWND owner{};
            if (!entry.owner_id.empty()) {
                std::size_t owner_index = 0U;
                while (owner_index < windows.size() &&
                       windows[owner_index].stable_id != entry.owner_id) {
                    ++owner_index;
                }
                if (owner_index == windows.size()) return 2;
                owner = handles[owner_index];
                if (owner == nullptr) continue;
            }

            WindowsHostOptions host_options = entry.options;
            host_options.quit_thread_on_close =
                entry.owner_id.empty() && !entry.tool_window;
            std::unique_ptr<WindowsHostState> state =
                std::make_unique<WindowsHostState>(
                    std::move(entry.model), std::move(host_options));
            const DWORD style = entry.options.popup_window
                ? (WS_POPUP | WS_BORDER) : WS_OVERLAPPEDWINDOW;
            const DWORD ex_style = entry.tool_window ? WS_EX_TOOLWINDOW : 0;
            RECT frame{0, 0,
                       static_cast<LONG>(std::ceil(entry.options.initial_size.width)),
                       static_cast<LONG>(std::ceil(entry.options.initial_size.height))};
            AdjustWindowRectEx(&frame, style, FALSE, ex_style);
            const std::wstring title = wide_from_utf8(entry.options.title);
            HWND window = CreateWindowExW(
                ex_style, window_class_name, title.c_str(), style, CW_USEDEFAULT,
                CW_USEDEFAULT, frame.right - frame.left, frame.bottom - frame.top,
                owner, nullptr, instance, state.get());
            if (window == nullptr || !(*state).initialize(window)) {
                if (window != nullptr) DestroyWindow(window);
                for (HWND created : handles) {
                    if (IsWindow(created)) DestroyWindow(created);
                }
                if (SUCCEEDED(com_status)) CoUninitialize();
                return window == nullptr ? 4 : 5;
            }
            states[index] = std::move(state);
            handles[index] = window;
            ++created_count;
            made_progress = true;
        }
        // A remaining ownership cycle is invalid even when a separate primary
        // exists. Reject it before entering the message loop.
        if (!made_progress) {
            for (HWND created : handles) {
                if (IsWindow(created)) DestroyWindow(created);
            }
            if (SUCCEEDED(com_status)) CoUninitialize();
            return 2;
        }
    }

    for (std::size_t index = 0; index < windows.size(); ++index) {
        if (!(*states[index]).will_show()) {
            for (HWND created : handles) {
                if (IsWindow(created)) DestroyWindow(created);
            }
            if (SUCCEEDED(com_status)) CoUninitialize();
            return 5;
        }
        ShowWindow(handles[index], windows[index].tool_window ? SW_SHOWNOACTIVATE
                                                              : SW_SHOWNORMAL);
        UpdateWindow(handles[index]);
        if (windows[index].options.close_after_launch_for_testing) {
            PostMessageW(handles[index], WM_CLOSE, 0, 0);
        }
    }

    MSG message{};
    bool quit{};
    while (!quit) {
        std::vector<HANDLE> clocks;
        clocks.reserve(states.size());
        for (const std::unique_ptr<WindowsHostState>& state : states) {
            if ((*state).live_frame_clock() != nullptr) {
                clocks.push_back((*state).live_frame_clock());
            }
        }
        if (clocks.size() >= MAXIMUM_WAIT_OBJECTS) {
            clocks.resize(MAXIMUM_WAIT_OBJECTS - 1U);
        }
        const DWORD wait = MsgWaitForMultipleObjectsEx(
            static_cast<DWORD>(clocks.size()),
            clocks.empty() ? nullptr : clocks.data(), INFINITE,
            QS_ALLINPUT, MWMO_INPUTAVAILABLE);
        if (wait == WAIT_FAILED) break;
        if (wait >= WAIT_OBJECT_0 &&
            wait < WAIT_OBJECT_0 + clocks.size()) {
            const HANDLE ready = clocks[wait - WAIT_OBJECT_0];
            for (const std::unique_ptr<WindowsHostState>& state : states) {
                if ((*state).live_frame_clock() == ready) {
                    (*state).mark_live_frame_due();
                    break;
                }
            }
        }

        const std::chrono::steady_clock::time_point batch_started =
            std::chrono::steady_clock::now();
        for (std::size_t drained = 0U; drained < 32U; ++drained) {
            MSG pending{};
            if (!PeekMessageW(&pending, nullptr, 0, 0, PM_REMOVE)) break;
            if (pending.message == WM_QUIT) {
                message = pending;
                quit = true;
                break;
            }
            TranslateMessage(&pending);
            DispatchMessageW(&pending);
            if (std::chrono::steady_clock::now() - batch_started >=
                std::chrono::milliseconds(4)) {
                break;
            }
        }
        for (const std::unique_ptr<WindowsHostState>& state : states) {
            const HANDLE clock = (*state).live_frame_clock();
            if (clock != nullptr &&
                WaitForSingleObject(clock, 0U) == WAIT_OBJECT_0) {
                (*state).mark_live_frame_due();
            }
        }
        if (!quit) {
            for (const std::unique_ptr<WindowsHostState>& state : states) {
                (*state).present_pending_frame();
            }
        }
    }
    for (HWND window : handles) if (IsWindow(window)) DestroyWindow(window);
    for (std::size_t index = 0; index < windows.size(); ++index) {
        const std::string metrics = (*states[index]).metrics_json();
        const std::string host = (*states[index]).host_json();
        if (windows[index].options.print_metrics_on_close) {
            std::fprintf(stdout,
                         "{\"stable_id\":\"%s\",\"window\":%s,\"host\":%s}\n",
                         windows[index].stable_id.c_str(), metrics.c_str(),
                         host.c_str());
        }
        if (windows[index].options.final_snapshot) {
            windows[index].options.final_snapshot(metrics, host);
        }
    }
    std::fflush(stdout);
    states.clear();
    if (SUCCEEDED(com_status)) CoUninitialize();
    return static_cast<int>(message.wParam);
}

} // namespace gui_forms::host
