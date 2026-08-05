#include "windows_host.hpp"

#include <windows.h>
#include <windowsx.h>
#include <wincodec.h>
#include <commdlg.h>
#include <shlobj.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace gui_forms::host {
namespace {

constexpr wchar_t window_class_name[] = L"GUIForms.Window.v1";
constexpr wchar_t tooltip_class_name[] = L"GUIForms.ToolTip.v1";
constexpr UINT_PTR scheduler_timer = 1;
constexpr UINT managed_dispatch_message = WM_APP + 0x41U;
constexpr std::size_t maximum_automation_command = 4096;

bool trace_win32_input() noexcept {
    static const bool enabled = [] {
        const char* value = std::getenv("GUI_FORMS_TRACE_WIN32_INPUT");
        return value != nullptr && std::strcmp(value, "1") == 0;
    }();
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
    for (const auto& filter : filters) {
        std::wstring label = wide_from_utf8(filter.label);
        if (label.empty()) label = L"Files";
        std::wstring pattern;
        for (const auto& extension_utf8 : filter.extensions) {
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

struct TooltipPopup final {
    HWND window{};
    std::wstring text;
    HFONT font{};
};

LRESULT CALLBACK tooltip_window_procedure(HWND window, UINT message,
                                          WPARAM wparam, LPARAM lparam) {
    auto* state = reinterpret_cast<TooltipPopup*>(
        GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        const auto* create = reinterpret_cast<const CREATESTRUCTW*>(lparam);
        state = static_cast<TooltipPopup*>(create->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
        state->window = window;
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
        HGDIOBJ old_font = state && state->font ? SelectObject(dc, state->font) : nullptr;
        RECT text_bounds{8, 5, std::max(8L, bounds.right - 8),
                         std::max(5L, bounds.bottom - 5)};
        if (state) DrawTextW(dc, state->text.c_str(), -1, &text_bounds,
                             DT_WORDBREAK | DT_NOPREFIX | DT_LEFT);
        if (old_font) SelectObject(dc, old_font);
        EndPaint(window, &paint);
        return 0;
    }
    case WM_NCDESTROY:
        if (state) state->window = nullptr;
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
        const auto function = load_function<GetDpiForWindowFunction>(
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
        const auto set_context = load_function<SetContextFunction>(
            user32, "SetProcessDpiAwarenessContext");
        if (set_context != nullptr && set_context(reinterpret_cast<HANDLE>(-4))) return;
        const auto set_aware = load_function<SetAwareFunction>(
            user32, "SetProcessDPIAware");
        if (set_aware != nullptr) static_cast<void>(set_aware());
    }
}

class DibPainter final : public Painter {
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
        states_.push_back({0.0, 0.0, {0.0, 0.0, logical_size_.width, logical_size_.height}});
        SetBkMode(memory_dc_, TRANSPARENT);
    }

    bool synchronize_images(const ImageRegistry& registry) {
        const ImageRegistrySnapshot snapshot = registry.snapshot();
        if (&registry == image_registry_ && snapshot.revision == image_revision_) {
            return images_synchronized_;
        }
        if (&registry != image_registry_) images_.clear();
        IWICImagingFactory* factory{};
        if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                                    CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)))) {
            images_synchronized_ = false;
            return false;
        }
        bool synchronized = true;
        std::unordered_set<std::uint64_t> active;
        for (const ImageId id : registry.image_ids()) {
            active.insert(id.value);
            const auto resource = registry.find(id);
            if (!resource) { synchronized = false; continue; }
            const auto existing = images_.find(id.value);
            if (existing != images_.end() &&
                existing->second.content_hash == resource->content_hash) continue;
            DecodedImage decoded;
            decoded.content_hash = resource->content_hash;
            decoded.width = resource->metadata.width;
            decoded.height = resource->metadata.height;
            if (!decode_png(*factory, *resource, decoded)) {
                images_.erase(id.value);
                synchronized = false;
                continue;
            }
            images_.insert_or_assign(id.value, std::move(decoded));
        }
        for (auto iterator = images_.begin(); iterator != images_.end();) {
            iterator = active.contains(iterator->first) ? std::next(iterator)
                                                        : images_.erase(iterator);
        }
        factory->Release();
        image_registry_ = &registry;
        image_revision_ = snapshot.revision;
        images_synchronized_ = synchronized;
        return synchronized;
    }

    void present(HDC target) const {
        if (target == nullptr || memory_dc_ == nullptr) return;
        BitBlt(target, 0, 0, width_, height_, memory_dc_, 0, 0, SRCCOPY);
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

    void fill_rect(Rect rect, Color color) override {
        const PixelRect area = pixel_rect(rect);
        if (area.empty()) return;
        const unsigned alpha = color.alpha;
        for (int y = area.top; y < area.bottom; ++y) {
            auto* row = pixels_ + static_cast<std::size_t>(y) * width_;
            for (int x = area.left; x < area.right; ++x) {
                const std::uint32_t destination = row[x];
                const unsigned db = destination & 0xffU;
                const unsigned dg = (destination >> 8U) & 0xffU;
                const unsigned dr = (destination >> 16U) & 0xffU;
                const unsigned inverse = 255U - alpha;
                const unsigned b = (color.blue * alpha + db * inverse + 127U) / 255U;
                const unsigned g = (color.green * alpha + dg * inverse + 127U) / 255U;
                const unsigned r = (color.red * alpha + dr * inverse + 127U) / 255U;
                row[x] = b | (g << 8U) | (r << 16U) | 0xff000000U;
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
        const std::wstring wide = wide_from_utf8(text);
        if (wide.empty() || memory_dc_ == nullptr) return;
        const wchar_t* family = font.role == FontRole::control
            ? L"Portsmouth Rapids"
            : font.role == FontRole::monospace ? L"Consolas" : L"Lucida Grande";
        HFONT native_font = CreateFontW(
            -std::max(1, static_cast<int>(std::lround(font.size * scale_))), 0, 0, 0,
            font.weight, font.italic ? TRUE : FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, family);
        if (native_font == nullptr) return;
        const int saved = SaveDC(memory_dc_);
        apply_gdi_clip();
        HGDIOBJ old = SelectObject(memory_dc_, native_font);
        SetTextColor(memory_dc_, RGB(color.red, color.green, color.blue));
        SetBkMode(memory_dc_, TRANSPARENT);
        TEXTMETRICW metrics{};
        GetTextMetricsW(memory_dc_, &metrics);
        const int baseline = logical_y(origin.y);
        TextOutW(memory_dc_, logical_x(origin.x), baseline - metrics.tmAscent,
                 wide.data(), static_cast<int>(wide.size()));
        SelectObject(memory_dc_, old);
        RestoreDC(memory_dc_, saved);
        DeleteObject(native_font);
    }

    void draw_image(ImageId image, Rect destination, double opacity) override {
        const auto found = images_.find(image.value);
        const PixelRect area = pixel_rect(destination);
        if (found == images_.end() || area.empty() || opacity <= 0.0) return;
        const DecodedImage& source = found->second;
        const double left = (destination.x + state().tx) * scale_;
        const double top = (destination.y + state().ty) * scale_;
        const double width = std::max(1.0, destination.width * scale_);
        const double height = std::max(1.0, destination.height * scale_);
        const unsigned global_alpha = static_cast<unsigned>(
            std::lround(std::clamp(opacity, 0.0, 1.0) * 255.0));
        for (int y = area.top; y < area.bottom; ++y) {
            const auto source_y = std::min<std::uint32_t>(
                source.height - 1U,
                static_cast<std::uint32_t>(std::max(0.0, (y - top) * source.height / height)));
            auto* destination_row = pixels_ + static_cast<std::size_t>(y) * width_;
            for (int x = area.left; x < area.right; ++x) {
                const auto source_x = std::min<std::uint32_t>(
                    source.width - 1U,
                    static_cast<std::uint32_t>(std::max(0.0, (x - left) * source.width / width)));
                const std::uint32_t source_pixel =
                    source.pixels[static_cast<std::size_t>(source_y) * source.width + source_x];
                const unsigned source_alpha = ((source_pixel >> 24U) & 0xffU) * global_alpha / 255U;
                const unsigned inverse = 255U - source_alpha;
                const std::uint32_t destination_pixel = destination_row[x];
                const auto channel = [&](unsigned shift) {
                    const unsigned premultiplied = ((source_pixel >> shift) & 0xffU) * global_alpha / 255U;
                    const unsigned destination_channel = (destination_pixel >> shift) & 0xffU;
                    return std::min(255U, premultiplied +
                        (destination_channel * inverse + 127U) / 255U);
                };
                destination_row[x] = channel(0) | (channel(8) << 8U) |
                                     (channel(16) << 16U) | 0xff000000U;
            }
        }
    }

private:
    struct State { double tx{}; double ty{}; Rect clip{}; };
    struct DecodedImage {
        std::uint64_t content_hash{};
        std::uint32_t width{};
        std::uint32_t height{};
        std::vector<std::uint32_t> pixels;
    };
    struct PixelRect {
        int left{}; int top{}; int right{}; int bottom{};
        [[nodiscard]] bool empty() const noexcept { return right <= left || bottom <= top; }
    };

    const State& state() const noexcept { return states_.back(); }
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
            status = stream->InitializeFromMemory(
                reinterpret_cast<BYTE*>(const_cast<std::byte*>(resource.encoded.data())),
                static_cast<DWORD>(resource.encoded.size()));
        }
        if (SUCCEEDED(status)) {
            status = factory.CreateDecoderFromStream(stream, nullptr,
                WICDecodeMetadataCacheOnLoad, &decoder);
        }
        if (SUCCEEDED(status)) status = decoder->GetFrame(0, &frame);
        UINT width{};
        UINT height{};
        if (SUCCEEDED(status)) status = frame->GetSize(&width, &height);
        if (SUCCEEDED(status) && (width != output.width || height != output.height)) {
            status = E_FAIL;
        }
        if (SUCCEEDED(status)) status = factory.CreateFormatConverter(&converter);
        if (SUCCEEDED(status)) {
            status = converter->Initialize(frame, GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
        }
        if (SUCCEEDED(status)) {
            output.pixels.resize(static_cast<std::size_t>(output.width) * output.height);
            const UINT stride = output.width * 4U;
            const std::size_t bytes = output.pixels.size() * sizeof(std::uint32_t);
            if (bytes > MAXDWORD) status = E_OUTOFMEMORY;
            else status = converter->CopyPixels(nullptr, stride, static_cast<UINT>(bytes),
                                                reinterpret_cast<BYTE*>(output.pixels.data()));
        }
        if (converter != nullptr) converter->Release();
        if (frame != nullptr) frame->Release();
        if (decoder != nullptr) decoder->Release();
        if (stream != nullptr) stream->Release();
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
    std::unordered_map<std::uint64_t, DecodedImage> images_;
    const ImageRegistry* image_registry_{};
    std::uint64_t image_revision_{std::numeric_limits<std::uint64_t>::max()};
    bool images_synchronized_{true};
};

class WindowsHostState final {
public:
    WindowsHostState(std::unique_ptr<Window> model, WindowsHostOptions options)
        : model_(std::move(model)), options_(std::move(options)),
          session_(*model_, windows_capabilities()) {}

    ~WindowsHostState() {
        hide_tooltip();
        if (tooltip_.font != nullptr) DeleteObject(tooltip_.font);
        session_.shutdown();
        if (!font_regular_path_.empty()) RemoveFontResourceExW(font_regular_path_.c_str(), FR_PRIVATE, nullptr);
        if (!font_bold_path_.empty()) RemoveFontResourceExW(font_bold_path_.c_str(), FR_PRIVATE, nullptr);
    }

    void bind_window(HWND window) noexcept { hwnd_ = window; }

    bool initialize(HWND window) {
        hwnd_ = window;
        scale_ = query_scale(window);
        load_private_fonts();
        RECT client{};
        GetClientRect(window, &client);
        const Size logical{(client.right - client.left) / scale_,
                           (client.bottom - client.top) / scale_};
        raster_.resize(logical, scale_);
        model_->metrics().set_renderer(
            "Win32 DIB CPU · GDI text · WIC PNG · Rapids UI", true);
        dispatch(HostAttachEvent{logical, scale_});
        dispatch(HostActivationEvent{GetActiveWindow() == window});
        if (options_.host_ready) {
            options_.host_ready(
                [this] { PostMessageW(hwnd_, managed_dispatch_message, 0, 0); },
                [this] { PostMessageW(hwnd_, WM_CLOSE, 0, 0); },
                [this](const HostDialogRequest& request) {
                    return show_dialog(request);
                },
                [this](const HostTooltipRequest& request) {
                    return show_tooltip(request);
                },
                [this] { hide_tooltip(); });
        }
        collect_damage();
        return true;
    }

    LRESULT message(UINT message, WPARAM wparam, LPARAM lparam) {
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
        case WM_PAINT: paint(); return 0;
        case managed_dispatch_message:
            if (options_.dispatch_pending) options_.dispatch_pending();
            collect_damage(); return 0;
        case WM_TIMER:
            if (wparam == scheduler_timer) { KillTimer(hwnd_, scheduler_timer); collect_damage(); return 0; }
            break;
        case WM_MOUSEMOVE: pointer(message, PointerAction::move, wparam, lparam); return 0;
        case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN:
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
                dispatch(HostClosedEvent{HostCloseReason::user});
                closed_ = true;
                if (options_.closed) options_.closed();
            }
            session_.shutdown();
            if (options_.quit_thread_on_close) PostQuitMessage(0);
            return 0;
        default: break;
        }
        return DefWindowProcW(hwnd_, message, wparam, lparam);
    }

    std::string metrics_json() const { return model_->metrics_snapshot().to_json(); }
    std::string host_json() const { return session_.snapshot().to_json(); }

private:
    template <class Payload>
    HostDispatchResult dispatch(Payload payload) {
        HostEvent event;
        event.sequence = next_sequence_++;
        event.timestamp_nanoseconds = now_nanoseconds();
        event.payload = std::move(payload);
        return session_.dispatch(std::move(event));
    }

    HostDialogResult show_dialog(const HostDialogRequest& request) {
        model_->release_pointer();
        synchronize_capture();
        hide_tooltip();
        return std::visit([this, &request](const auto& payload) -> HostDialogResult {
            using Payload = std::decay_t<decltype(payload)>;
            if constexpr (std::is_same_v<Payload, HostOpenFileDialogRequest>) {
                return native_open_dialog(hwnd_, request.request_id, payload);
            } else if constexpr (std::is_same_v<Payload, HostSaveFileDialogRequest>) {
                return native_save_dialog(hwnd_, request.request_id, payload);
            } else if constexpr (std::is_same_v<Payload, HostFolderDialogRequest>) {
                return native_folder_dialog(hwnd_, request.request_id, payload);
            } else if constexpr (std::is_same_v<Payload, HostColorDialogRequest>) {
                return {{HostServiceError::unsupported}, request.request_id,
                        HostColorDialogResult{}};
            } else {
                return {{HostServiceError::unsupported}, request.request_id,
                        HostMessageDialogResult{}};
            }
        }, request.payload);
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
        native_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
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
        MONITORINFO monitor{sizeof(monitor)};
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
            SetWindowPos(hwnd_, nullptr, suggested->left, suggested->top,
                         suggested->right - suggested->left,
                         suggested->bottom - suggested->top,
                         SWP_NOACTIVATE | SWP_NOZORDER);
        }
        resize();
    }

    void collect_damage() {
        static_cast<void>(model_->poll_frame_schedule(FrameClock::now()));
        DamageRegion damage = model_->take_damage();
        if (!damage.empty()) {
            pending_damage_.add(damage.bounds());
            InvalidateRect(hwnd_, nullptr, FALSE);
        }
        KillTimer(hwnd_, scheduler_timer);
        if (const auto wake = model_->next_wake()) {
            const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                *wake - FrameClock::now()).count();
            SetTimer(hwnd_, scheduler_timer,
                     static_cast<UINT>(std::clamp<std::int64_t>(milliseconds, 1, 60'000)), nullptr);
        }
    }

    void paint() {
        PAINTSTRUCT paint_state{};
        HDC dc = BeginPaint(hwnd_, &paint_state);
        const auto started = std::chrono::steady_clock::now();
        if (pending_damage_.empty()) {
            pending_damage_.add({paint_state.rcPaint.left / scale_, paint_state.rcPaint.top / scale_,
                                 (paint_state.rcPaint.right - paint_state.rcPaint.left) / scale_,
                                 (paint_state.rcPaint.bottom - paint_state.rcPaint.top) / scale_});
        }
        raster_.begin_frame();
        static_cast<void>(raster_.synchronize_images(model_->image_resources()));
        model_->paint(raster_, pending_damage_.bounds());
        raster_.present(dc);
        pending_damage_.clear();
        EndPaint(hwnd_, &paint_state);
        const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now() - started);
        model_->metrics().record_present(static_cast<std::uint64_t>(elapsed.count()));
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
        if (trace_win32_input()) {
            const auto target = model_->hit_test(event.position);
            const auto target_id = target ? target->stable_id().value() : std::string_view{"<none>"};
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
        update_cursor(client_point(lparam));
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
        const bool requested = model_->captured_control() != nullptr;
        if (requested && GetCapture() != hwnd_) SetCapture(hwnd_);
        if (!requested && GetCapture() == hwnd_) ReleaseCapture();
    }

    void update_cursor(Point position) const {
        const auto target = model_->hit_test(position);
        const CursorKind cursor = target ? target->effective_cursor() : CursorKind::arrow;
        LPCWSTR identifier = IDC_ARROW;
        switch (cursor) {
        case CursorKind::text: identifier = IDC_IBEAM; break;
        case CursorKind::hand: identifier = IDC_HAND; break;
        case CursorKind::crosshair: identifier = IDC_CROSS; break;
        case CursorKind::resize_horizontal: identifier = IDC_SIZEWE; break;
        case CursorKind::resize_vertical: identifier = IDC_SIZENS; break;
        case CursorKind::wait: identifier = IDC_WAIT; break;
        case CursorKind::forbidden: identifier = IDC_NO; break;
        case CursorKind::arrow: break;
        }
        SetCursor(LoadCursorW(nullptr, identifier));
    }

    void minimum_size(MINMAXINFO* info) const {
        if (info == nullptr) return;
        RECT rect{0, 0,
                  static_cast<LONG>(std::ceil(options_.minimum_size.width * scale_)),
                  static_cast<LONG>(std::ceil(options_.minimum_size.height * scale_))};
        AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);
        info->ptMinTrackSize.x = rect.right - rect.left;
        info->ptMinTrackSize.y = rect.bottom - rect.top;
    }

    LRESULT close() {
        HostCloseRequest request{HostCloseReason::user, false};
        if (options_.close_request) options_.close_request(request);
        const HostDispatchResult result = dispatch(request);
        if (result.accepted() && result.close_allowed) DestroyWindow(hwnd_);
        return 0;
    }

    LRESULT automation(const COPYDATASTRUCT* data) {
        if (!options_.automation_enabled || data == nullptr || data->lpData == nullptr ||
            data->cbData == 0 || data->cbData > maximum_automation_command) return FALSE;
        const char* bytes = static_cast<const char*>(data->lpData);
        std::string command(bytes, bytes + data->cbData);
        while (!command.empty() && command.back() == '\0') command.pop_back();
        constexpr std::string_view prefix = "GUI.Forms.Automation/1 ";
        if (!command.starts_with(prefix)) return FALSE;
        command.erase(0, prefix.size());
        if (command == "snapshot") {
            std::fprintf(stdout, "{\"automation\":\"snapshot\",\"window\":%s,\"host\":%s}\n",
                         metrics_json().c_str(), host_json().c_str());
            std::fflush(stdout);
            return TRUE;
        }
        if (command == "capture") {
            const std::wstring path = executable_directory() + L"gallery-automation.bmp";
            const bool saved = raster_.save_bmp(path);
            std::fprintf(stdout, "{\"automation\":\"capture\",\"saved\":%s}\n",
                         saved ? "true" : "false");
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
            const auto control = options_.automation_resolve
                ? options_.automation_resolve(id) : model_->find(id);
            if (!control || !control->eligible_for_input()) return FALSE;
            const Rect bounds = control->absolute_bounds();
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
            const auto control = options_.automation_resolve
                ? options_.automation_resolve(id) : model_->find(id);
            if (!control || !control->eligible_for_input() ||
                !model_->request_focus(control)) return FALSE;
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
            const auto control = options_.automation_resolve
                ? options_.automation_resolve(id) : model_->find(id);
            if (!control || !control->eligible_for_input() ||
                !model_->request_focus(control)) return FALSE;
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
            const auto control = options_.automation_resolve
                ? options_.automation_resolve(id) : model_->find(id);
            if (!control || !control->eligible_for_input() ||
                !model_->request_focus(control)) return FALSE;
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
            const auto control = options_.automation_resolve
                ? options_.automation_resolve(id) : model_->find(id);
            if (!control || !control->eligible_for_input()) {
                std::fprintf(stdout,
                             "{\"automation\":\"activate\",\"id\":\"%s\",\"accepted\":false,\"reason\":\"%s\"}\n",
                             id.c_str(), !control ? "not-found" : "ineligible");
                std::fflush(stdout);
                return FALSE;
            }
            control->on_activate();
            collect_damage();
            std::fprintf(stdout,
                         "{\"automation\":\"activate\",\"id\":\"%s\",\"stable_id\":\"%s\"}\n",
                         id.c_str(), control->stable_id().value().data());
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
            const auto control = options_.automation_resolve
                ? options_.automation_resolve(id) : model_->find(id);
            if (!control || !control->eligible_for_input()) {
                std::fprintf(stdout,
                             "{\"automation\":\"click-at\",\"id\":\"%s\",\"accepted\":false,\"reason\":\"%s\"}\n",
                             id.c_str(), !control ? "not-found" : "ineligible");
                std::fflush(stdout);
                return FALSE;
            }
            const Rect bounds = control->absolute_bounds();
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
            const auto control = options_.automation_resolve
                ? options_.automation_resolve(id) : model_->find(id);
            if (!control || !control->eligible_for_input()) {
                std::fprintf(stdout,
                             "{\"automation\":\"drag\",\"id\":\"%s\",\"accepted\":false,\"reason\":\"%s\"}\n",
                             id.c_str(), !control ? "not-found" : "ineligible");
                std::fflush(stdout);
                return FALSE;
            }
            const Rect bounds = control->absolute_bounds();
            const auto point_at = [&](double local_x, double local_y) {
                return Point{bounds.x + std::clamp(local_x, 0.0, bounds.width),
                             bounds.y + std::clamp(local_y, 0.0, bounds.height)};
            };
            const Point start = point_at(start_x, start_y);
            const Point end = point_at(end_x, end_y);
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
            const auto control = options_.automation_resolve
                ? options_.automation_resolve(id) : model_->find(id);
            if (!control || !control->eligible_for_input()) return FALSE;
            std::fprintf(stdout, "{\"automation\":\"click-phase\",\"id\":\"%s\",\"phase\":\"resolved\"}\n",
                         id.c_str());
            std::fflush(stdout);
            const Rect bounds = control->absolute_bounds();
            const Point center{bounds.x + bounds.width / 2.0, bounds.y + bounds.height / 2.0};
            const auto hit = model_->hit_test(center);
            std::fprintf(stdout,
                         "{\"automation\":\"click-target\",\"id\":\"%s\",\"resolved\":\"%s\",\"hit\":\"%s\",\"x\":%.3f,\"y\":%.3f}\n",
                         id.c_str(), control->stable_id().value().data(),
                         hit ? hit->stable_id().value().data() : "", center.x,
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
        font_regular_path_ = directory + L"fonts\\PortsmouthRapids.ttf";
        font_bold_path_ = directory + L"fonts\\PortsmouthRapids-Bold.ttf";
        AddFontResourceExW(font_regular_path_.c_str(), FR_PRIVATE, nullptr);
        AddFontResourceExW(font_bold_path_.c_str(), FR_PRIVATE, nullptr);
    }

    static std::wstring executable_directory() {
        wchar_t executable[MAX_PATH]{};
        const DWORD count = GetModuleFileNameW(nullptr, executable, MAX_PATH);
        if (count == 0 || count >= MAX_PATH) return {};
        std::wstring directory(executable, count);
        const auto slash = directory.find_last_of(L"\\/");
        directory.resize(slash == std::wstring::npos ? 0 : slash + 1);
        return directory;
    }

    std::unique_ptr<Window> model_;
    WindowsHostOptions options_;
    HostSession session_;
    HWND hwnd_{};
    DibPainter raster_;
    DamageRegion pending_damage_;
    double scale_{1.0};
    std::uint64_t next_sequence_{1};
    wchar_t pending_high_surrogate_{};
    bool closed_{};
    std::wstring font_regular_path_;
    std::wstring font_bold_path_;
    TooltipPopup tooltip_;
};

LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    auto* state = reinterpret_cast<WindowsHostState*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        const auto* create = reinterpret_cast<const CREATESTRUCTW*>(lparam);
        state = static_cast<WindowsHostState*>(create->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
        state->bind_window(window);
    }
    if (state != nullptr) return state->message(message, wparam, lparam);
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
                HostCapability::pointer_capture |
                HostCapability::cursor};
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
    native_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
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
    ShowWindow(window, SW_SHOWNORMAL);
    UpdateWindow(window);
    if (options.close_after_launch_for_testing) {
        PostMessageW(window, WM_CLOSE, 0, 0);
    }

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
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

} // namespace gui_forms::host
