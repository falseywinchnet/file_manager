#include <windows.h>

#include <algorithm>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace {

std::wstring wide_from_utf8(const char* text) {
    if (text == nullptr || *text == '\0') return {};
    const int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1,
                                          nullptr, 0);
    if (count <= 1) return {};
    std::wstring result(static_cast<std::size_t>(count), L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1,
                        result.data(), count);
    result.pop_back();
    return result;
}

BOOL CALLBACK list_gui_forms_windows(HWND window, LPARAM) {
    wchar_t class_name[128]{};
    if (GetClassNameW(window, class_name, 128) == 0 ||
        std::wcscmp(class_name, L"GUIForms.Window.v1") != 0) return TRUE;
    wchar_t title[512]{};
    GetWindowTextW(window, title, 512);
    char utf8[2048]{};
    DWORD process_id{};
    GetWindowThreadProcessId(window, &process_id);
    WideCharToMultiByte(CP_UTF8, 0, title, -1, utf8, 2048, nullptr, nullptr);
    std::printf("window=%p pid=%lu title=%s visible=%d enabled=%d\n",
                static_cast<void*>(window), static_cast<unsigned long>(process_id),
                utf8, IsWindowVisible(window),
                IsWindowEnabled(window));
    return TRUE;
}

BOOL CALLBACK list_visible_windows(HWND window, LPARAM) {
    if (!IsWindowVisible(window)) return TRUE;
    wchar_t title[512]{};
    wchar_t class_name[128]{};
    GetClassNameW(window, class_name, 128);
    const bool gui_forms_endpoint =
        std::wcscmp(class_name, L"GUIForms.CompatibilityPaint.v1") == 0;
    if ((GetWindowTextW(window, title, 512) == 0 || title[0] == L'\0') &&
        !gui_forms_endpoint) return TRUE;
    char title_utf8[2048]{};
    char class_utf8[512]{};
    DWORD process_id{};
    RECT bounds{};
    BYTE alpha = 255;
    COLORREF color_key{};
    DWORD layer_flags{};
    GetWindowThreadProcessId(window, &process_id);
    GetWindowRect(window, &bounds);
    GetLayeredWindowAttributes(window, &color_key, &alpha, &layer_flags);
    WideCharToMultiByte(CP_UTF8, 0, title, -1, title_utf8, 2048, nullptr, nullptr);
    WideCharToMultiByte(CP_UTF8, 0, class_name, -1, class_utf8, 512, nullptr, nullptr);
    std::printf("window=%p pid=%lu class=%s title=%s enabled=%d parent=%p owner=%p style=%llx exstyle=%llx bounds=%ld,%ld,%ld,%ld alpha=%u layer-flags=%lu\n",
                static_cast<void*>(window), static_cast<unsigned long>(process_id),
                class_utf8, title_utf8,
                IsWindowEnabled(window), static_cast<void*>(GetParent(window)),
                static_cast<void*>(GetWindow(window, GW_OWNER)),
                static_cast<unsigned long long>(GetWindowLongPtrW(window, GWL_STYLE)),
                static_cast<unsigned long long>(GetWindowLongPtrW(window, GWL_EXSTYLE)),
                bounds.left, bounds.top,
                bounds.right, bounds.bottom, static_cast<unsigned>(alpha),
                static_cast<unsigned long>(layer_flags));
    return TRUE;
}

struct OwnedDialogSearch final {
    HWND owner{};
    HWND dialog{};
};

struct PaintEndpointOwnerSearch final {
    HWND owner{};
    DWORD process_id{};
    unsigned attached{};
    bool child{};
};

BOOL CALLBACK attach_paint_endpoint_owner(HWND candidate, LPARAM context_value) {
    PaintEndpointOwnerSearch& context =
        *reinterpret_cast<PaintEndpointOwnerSearch*>(context_value);
    wchar_t class_name[128]{};
    DWORD process_id{};
    GetWindowThreadProcessId(candidate, &process_id);
    if (process_id != context.process_id ||
        GetClassNameW(candidate, class_name, 128) == 0 ||
        std::wcscmp(class_name, L"GUIForms.CompatibilityPaint.v1") != 0) {
        return TRUE;
    }
    bool accepted = false;
    if (context.child) {
        const LONG_PTR style = GetWindowLongPtrW(candidate, GWL_STYLE);
        SetLastError(ERROR_SUCCESS);
        const LONG_PTR prior = SetWindowLongPtrW(
            candidate, GWL_STYLE, (style & ~WS_POPUP) | WS_CHILD | WS_DISABLED);
        accepted = prior != 0 || GetLastError() == ERROR_SUCCESS;
        if (accepted) accepted = SetParent(candidate, context.owner) != nullptr;
    } else {
        SetLastError(ERROR_SUCCESS);
        const LONG_PTR prior = SetWindowLongPtrW(
            candidate, GWLP_HWNDPARENT, reinterpret_cast<LONG_PTR>(context.owner));
        accepted = prior != 0 || GetLastError() == ERROR_SUCCESS;
    }
    if (accepted) {
        ++context.attached;
        SetWindowPos(candidate, HWND_BOTTOM, 0, 0, 0, 0, context.child
            ? SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED
            : SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    return TRUE;
}

BOOL CALLBACK find_owned_dialog(HWND window, LPARAM context_value) {
    OwnedDialogSearch& context =
        *reinterpret_cast<OwnedDialogSearch*>(context_value);
    if (!IsWindowVisible(window) || GetWindow(window, GW_OWNER) != context.owner) {
        return TRUE;
    }
    wchar_t class_name[32]{};
    if (GetClassNameW(window, class_name, 32) != 0 &&
        std::wcscmp(class_name, L"#32770") == 0) {
        context.dialog = window;
        return FALSE;
    }
    return TRUE;
}

bool capture_client_bitmap(HWND window, const std::wstring& path) {
    RECT client{};
    if (!GetClientRect(window, &client)) return false;
    const int width = client.right - client.left;
    const int height = client.bottom - client.top;
    if (width <= 0 || height <= 0) return false;
    ShowWindow(window, SW_RESTORE);
    SetForegroundWindow(window);
    UpdateWindow(window);
    Sleep(100);
    POINT origin{};
    ClientToScreen(window, &origin);
    HDC source = GetDC(nullptr);
    HDC memory = CreateCompatibleDC(source);
    HBITMAP bitmap = CreateCompatibleBitmap(source, width, height);
    HGDIOBJ previous = SelectObject(memory, bitmap);
    // Wine's WinForms/Vulkan path reports PrintWindow success while returning a
    // black surface. Capture the foreground client rectangle from the desktop.
    const bool painted = BitBlt(memory, 0, 0, width, height, source,
                                origin.x, origin.y,
                                SRCCOPY | CAPTUREBLT) != FALSE;

    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    std::vector<unsigned char> pixels(
        static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4U);
    const bool copied = GetDIBits(memory, bitmap, 0, static_cast<UINT>(height),
                                  pixels.data(), &info, DIB_RGB_COLORS) != 0;

    BITMAPFILEHEADER file{};
    file.bfType = 0x4d42;
    file.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    file.bfSize = file.bfOffBits + static_cast<DWORD>(pixels.size());
    HANDLE output = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr,
                                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    bool written = false;
    if (output != INVALID_HANDLE_VALUE && painted && copied) {
        DWORD count{};
        written = WriteFile(output, &file, sizeof(file), &count, nullptr) != FALSE &&
            count == sizeof(file) &&
            WriteFile(output, &info.bmiHeader, sizeof(info.bmiHeader), &count,
                      nullptr) != FALSE && count == sizeof(info.bmiHeader) &&
            WriteFile(output, pixels.data(), static_cast<DWORD>(pixels.size()),
                      &count, nullptr) != FALSE && count == pixels.size();
    }
    if (output != INVALID_HANDLE_VALUE) CloseHandle(output);
    SelectObject(memory, previous);
    DeleteObject(bitmap);
    DeleteDC(memory);
    ReleaseDC(nullptr, source);
    return written;
}

struct SurfaceStatistics {
    unsigned count{};
};

BOOL CALLBACK sample_control_surface(HWND window, LPARAM context_value) {
    wchar_t class_name[128]{};
    if (GetClassNameW(window, class_name, 128) == 0 ||
        std::wcscmp(class_name, L"GUIForms.ControlSurface.v1") != 0) return TRUE;
    SurfaceStatistics& context =
        *reinterpret_cast<SurfaceStatistics*>(context_value);
    ++context.count;
    RECT client{};
    RECT screen{};
    if (!GetClientRect(window, &client) || !GetWindowRect(window, &screen)) return TRUE;
    const int width = client.right - client.left;
    const int height = client.bottom - client.top;
    if (width <= 0 || height <= 0) return TRUE;
    HDC device = GetDC(window);
    if (device == nullptr) return TRUE;
    constexpr int grid = 24;
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = grid;
    info.bmiHeader.biHeight = -grid;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    void* captured_pixels{};
    HBITMAP captured = CreateDIBSection(device, &info, DIB_RGB_COLORS,
                                        &captured_pixels, nullptr, 0);
    HDC memory = captured == nullptr ? nullptr : CreateCompatibleDC(device);
    HGDIOBJ previous = memory == nullptr ? nullptr : SelectObject(memory, captured);
    const bool copied = previous != nullptr && previous != HGDI_ERROR &&
        StretchBlt(memory, 0, 0, grid, grid, device, 0, 0, width, height,
                   SRCCOPY) != FALSE;
    unsigned minimum_r = 255, minimum_g = 255, minimum_b = 255;
    unsigned maximum_r = 0, maximum_g = 0, maximum_b = 0;
    COLORREF first = CLR_INVALID;
    unsigned samples = 0;
    unsigned different = 0;
    if (copied) {
        const unsigned char* pixels =
            static_cast<const unsigned char*>(captured_pixels);
        for (int row = 0; row < grid; ++row) {
            for (int column = 0; column < grid; ++column) {
            const unsigned char* pixel =
                pixels + (row * grid + column) * 4;
            const unsigned b = pixel[0];
            const unsigned g = pixel[1];
            const unsigned r = pixel[2];
            minimum_r = std::min(minimum_r, r); maximum_r = std::max(maximum_r, r);
            minimum_g = std::min(minimum_g, g); maximum_g = std::max(maximum_g, g);
            minimum_b = std::min(minimum_b, b); maximum_b = std::max(maximum_b, b);
            const COLORREF packed = RGB(r, g, b);
            if (first == CLR_INVALID) first = packed;
            else if (packed != first) ++different;
            ++samples;
            }
        }
    }
    if (memory != nullptr && previous != nullptr && previous != HGDI_ERROR)
        SelectObject(memory, previous);
    if (memory != nullptr) DeleteDC(memory);
    if (captured != nullptr) DeleteObject(captured);
    ReleaseDC(window, device);
    std::printf("surface=%p position=%ld,%ld size=%dx%d visible=%d "
                "rgb=%u-%u,%u-%u,%u-%u different=%u/%u\n",
                static_cast<void*>(window), screen.left, screen.top, width, height,
                IsWindowVisible(window), minimum_r, maximum_r, minimum_g, maximum_g,
                minimum_b, maximum_b, different, samples);
    return TRUE;
}

bool send_automation(HWND window, const std::string& command, DWORD_PTR& result) {
    COPYDATASTRUCT data{};
    data.dwData = 0x47464131U;
    data.cbData = static_cast<DWORD>(command.size() + 1);
    data.lpData = const_cast<char*>(command.data());
    return SendMessageTimeoutW(window, WM_COPYDATA, 0,
                               reinterpret_cast<LPARAM>(&data),
                               SMTO_ABORTIFHUNG | SMTO_BLOCK, 5000, &result) != 0;
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: gui_forms_windows_probe <command> [argument]\n");
        return 2;
    }
    if (std::strcmp(argv[1], "list") == 0) {
        EnumWindows(list_gui_forms_windows, 0);
        return 0;
    }
    if (std::strcmp(argv[1], "list-all") == 0) {
        EnumWindows(list_visible_windows, 0);
        return 0;
    }
    const std::wstring requested_title =
        wide_from_utf8(std::getenv("GUI_FORMS_AUTOMATION_TITLE"));
    const std::wstring requested_class =
        wide_from_utf8(std::getenv("GUI_FORMS_AUTOMATION_CLASS"));
    HWND window = nullptr;
    if (const char* requested_handle = std::getenv("GUI_FORMS_AUTOMATION_HANDLE");
        requested_handle != nullptr && *requested_handle != '\0') {
        window = reinterpret_cast<HWND>(static_cast<std::uintptr_t>(
            std::strtoull(requested_handle, nullptr, 0)));
        if (!IsWindow(window)) window = nullptr;
    } else {
        window = FindWindowW(requested_class.empty() ? L"GUIForms.Window.v1"
                                                      : requested_class.c_str(),
                             requested_title.empty() ? nullptr : requested_title.c_str());
    }
    if (window == nullptr) {
        std::fprintf(stderr, "GUI.Forms window not found\n");
        return 3;
    }
    if (std::strcmp(argv[1], "cursor-target") == 0) {
        POINT screen{};
        if (!GetCursorPos(&screen)) return 4;
        HWND target = WindowFromPoint(screen);
        std::printf("cursor-position=%ld,%ld target=%p root=%p expected=%p\n",
                    screen.x, screen.y, static_cast<void*>(target),
                    static_cast<void*>(target == nullptr ? nullptr :
                        GetAncestor(target, GA_ROOT)), static_cast<void*>(window));
        for (unsigned depth = 0; target != nullptr && depth < 16; ++depth) {
            wchar_t class_name[128]{};
            GetClassNameW(target, class_name, 128);
            char class_utf8[512]{};
            WideCharToMultiByte(CP_UTF8, 0, class_name, -1, class_utf8, 512,
                                nullptr, nullptr);
            const HCURSOR class_cursor = reinterpret_cast<HCURSOR>(
                GetClassLongPtrW(target, GCLP_HCURSOR));
            std::printf("cursor-window=%p depth=%u class=%s parent=%p owner=%p "
                        "visible=%d enabled=%d class-cursor=%p style=%llx exstyle=%llx\n",
                        static_cast<void*>(target), depth, class_utf8,
                        static_cast<void*>(GetParent(target)),
                        static_cast<void*>(GetWindow(target, GW_OWNER)),
                        IsWindowVisible(target), IsWindowEnabled(target),
                        static_cast<void*>(class_cursor),
                        static_cast<unsigned long long>(
                            GetWindowLongPtrW(target, GWL_STYLE)),
                        static_cast<unsigned long long>(
                            GetWindowLongPtrW(target, GWL_EXSTYLE)));
            POINT client = screen;
            if (!ScreenToClient(target, &client)) break;
            HWND child = ChildWindowFromPointEx(
                target, client,
                CWP_SKIPINVISIBLE | CWP_SKIPDISABLED | CWP_SKIPTRANSPARENT);
            if (child == nullptr || child == target) break;
            target = child;
        }
        return 0;
    }
    if (std::strcmp(argv[1], "input-state") == 0) {
        DWORD process_id{};
        const DWORD thread_id = GetWindowThreadProcessId(window, &process_id);
        GUITHREADINFO state{};
        state.cbSize = sizeof(state);
        const bool queried = GetGUIThreadInfo(thread_id, &state) != FALSE;
        const HWND foreground = GetForegroundWindow();
        wchar_t foreground_class[128]{};
        wchar_t foreground_title[256]{};
        GetClassNameW(foreground, foreground_class, 128);
        GetWindowTextW(foreground, foreground_title, 256);
        char foreground_class_utf8[512]{};
        char foreground_title_utf8[1024]{};
        WideCharToMultiByte(CP_UTF8, 0, foreground_class, -1,
                            foreground_class_utf8, 512, nullptr, nullptr);
        WideCharToMultiByte(CP_UTF8, 0, foreground_title, -1,
                            foreground_title_utf8, 1024, nullptr, nullptr);
        DWORD foreground_process{};
        const DWORD foreground_thread =
            GetWindowThreadProcessId(foreground, &foreground_process);
        std::printf(
            "input-state=queried:%d thread:%lu process:%lu flags:%lx "
            "foreground:%p active:%p focus:%p capture:%p menu:%p "
            "move-size:%p caret:%p hung:%d foreground-thread:%lu "
            "foreground-process:%lu foreground-visible:%d "
            "foreground-enabled:%d foreground-class:%s foreground-title:%s\n",
            queried, static_cast<unsigned long>(thread_id),
            static_cast<unsigned long>(process_id),
            static_cast<unsigned long>(state.flags),
            static_cast<void*>(foreground),
            static_cast<void*>(state.hwndActive),
            static_cast<void*>(state.hwndFocus),
            static_cast<void*>(state.hwndCapture),
            static_cast<void*>(state.hwndMenuOwner),
            static_cast<void*>(state.hwndMoveSize),
            static_cast<void*>(state.hwndCaret), IsHungAppWindow(window),
            static_cast<unsigned long>(foreground_thread),
            static_cast<unsigned long>(foreground_process),
            IsWindowVisible(foreground), IsWindowEnabled(foreground),
            foreground_class_utf8, foreground_title_utf8);
        return queried ? 0 : 5;
    }
    if (std::strcmp(argv[1], "native-close") == 0) {
        if (!PostMessageW(window, WM_CLOSE, 0, 0)) {
            std::fprintf(stderr, "failed to post WM_CLOSE\n");
            return 4;
        }
        std::printf("posted=WM_CLOSE\n");
        return 0;
    }
    if (std::strcmp(argv[1], "own-paint-endpoints") == 0) {
        PaintEndpointOwnerSearch search{window};
        GetWindowThreadProcessId(window, &search.process_id);
        EnumWindows(attach_paint_endpoint_owner,
                    reinterpret_cast<LPARAM>(&search));
        std::printf("owned-paint-endpoints=%u owner=%p\n", search.attached,
                    static_cast<void*>(window));
        return search.attached == 0 ? 5 : 0;
    }
    if (std::strcmp(argv[1], "child-paint-endpoints") == 0) {
        PaintEndpointOwnerSearch search{window};
        search.child = true;
        GetWindowThreadProcessId(window, &search.process_id);
        EnumWindows(attach_paint_endpoint_owner,
                    reinterpret_cast<LPARAM>(&search));
        std::printf("child-paint-endpoints=%u parent=%p\n", search.attached,
                    static_cast<void*>(window));
        return search.attached == 0 ? 5 : 0;
    }
    if (std::strcmp(argv[1], "dismiss-dialog") == 0) {
        OwnedDialogSearch search{window, nullptr};
        EnumWindows(find_owned_dialog, reinterpret_cast<LPARAM>(&search));
        if (search.dialog == nullptr) {
            std::fprintf(stderr, "owned native dialog not found\n");
            return 4;
        }
        HWND cancel = GetDlgItem(search.dialog, IDCANCEL);
        if (!PostMessageW(search.dialog, WM_COMMAND,
                          MAKEWPARAM(IDCANCEL, BN_CLICKED),
                          reinterpret_cast<LPARAM>(cancel))) {
            std::fprintf(stderr, "failed to invoke native dialog cancellation\n");
            return 4;
        }
        std::printf("dismissed=%p owner=%p\n", static_cast<void*>(search.dialog),
                    static_cast<void*>(window));
        return 0;
    }
    if (std::strcmp(argv[1], "native-capture") == 0) {
        if (argc != 3 || !capture_client_bitmap(window, wide_from_utf8(argv[2]))) {
            std::fprintf(stderr, "native window capture failed\n");
            return 4;
        }
        std::printf("captured=%s\n", argv[2]);
        return 0;
    }
    if (std::strcmp(argv[1], "surface-stats") == 0) {
        SurfaceStatistics statistics;
        EnumChildWindows(window, sample_control_surface,
                         reinterpret_cast<LPARAM>(&statistics));
        std::printf("surfaces=%u\n", statistics.count);
        return statistics.count == 0 ? 5 : 0;
    }
    if (std::strcmp(argv[1], "native-click") == 0) {
        if (argc != 4) return 2;
        const long x = std::strtol(argv[2], nullptr, 10);
        const long y = std::strtol(argv[3], nullptr, 10);
        const LPARAM point = MAKELPARAM(static_cast<short>(x), static_cast<short>(y));
        if (!PostMessageW(window, WM_MOUSEMOVE, 0, point) ||
            !PostMessageW(window, WM_LBUTTONDOWN, MK_LBUTTON, point) ||
            !PostMessageW(window, WM_LBUTTONUP, 0, point)) {
            std::fprintf(stderr, "failed to post native click\n");
            return 4;
        }
        std::printf("clicked=%ld,%ld\n", x, y);
        return 0;
    }
    if (std::strcmp(argv[1], "native-move") == 0) {
        if (argc != 4) return 2;
        const long x = std::strtol(argv[2], nullptr, 10);
        const long y = std::strtol(argv[3], nullptr, 10);
        const LPARAM point = MAKELPARAM(static_cast<short>(x), static_cast<short>(y));
        if (!PostMessageW(window, WM_MOUSEMOVE, 0, point)) {
            std::fprintf(stderr, "failed to post native pointer move\n");
            return 4;
        }
        std::printf("moved=%ld,%ld\n", x, y);
        return 0;
    }
    if (std::strcmp(argv[1], "native-cursor") == 0) {
        if (argc != 4) return 2;
        const long x = std::strtol(argv[2], nullptr, 10);
        const long y = std::strtol(argv[3], nullptr, 10);
        const LPARAM point = MAKELPARAM(static_cast<short>(x), static_cast<short>(y));
        DWORD_PTR move_result{};
        DWORD_PTR cursor_result{};
        const bool moved = SendMessageTimeoutW(
            window, WM_MOUSEMOVE, 0, point,
            SMTO_ABORTIFHUNG | SMTO_BLOCK, 5000, &move_result) != 0;
        const bool handled = SendMessageTimeoutW(
            window, WM_SETCURSOR, reinterpret_cast<WPARAM>(window),
            MAKELPARAM(HTCLIENT, WM_MOUSEMOVE),
            SMTO_ABORTIFHUNG | SMTO_BLOCK, 5000, &cursor_result) != 0;
        CURSORINFO info{};
        info.cbSize = sizeof(info);
        const bool inspected = GetCursorInfo(&info) != FALSE;
        const HCURSOR class_cursor = reinterpret_cast<HCURSOR>(
            GetClassLongPtrW(window, GCLP_HCURSOR));
        std::printf("cursor=%p class=%p showing=%d message=%llu moved=%d inspected=%d\n",
                    static_cast<void*>(info.hCursor),
                    static_cast<void*>(class_cursor),
                    inspected && (info.flags & CURSOR_SHOWING) != 0,
                    static_cast<unsigned long long>(cursor_result),
                    moved, inspected);
        return moved && handled && inspected && info.hCursor != nullptr &&
                       (info.flags & CURSOR_SHOWING) != 0
            ? 0 : 5;
    }
    if (std::strcmp(argv[1], "native-press-capture") == 0) {
        if (argc != 5) return 2;
        const long x = std::strtol(argv[2], nullptr, 10);
        const long y = std::strtol(argv[3], nullptr, 10);
        const LPARAM point = MAKELPARAM(static_cast<short>(x), static_cast<short>(y));
        SendMessageW(window, WM_MOUSEMOVE, 0, point);
        SendMessageW(window, WM_LBUTTONDOWN, MK_LBUTTON, point);
        DWORD_PTR result{};
        const std::string capture_command =
            std::string("GUI.Forms.Automation/1 capture ") + argv[4];
        const bool captured = send_automation(window, capture_command, result) && result == TRUE;
        SendMessageW(window, WM_LBUTTONUP, 0, point);
        if (!captured) {
            std::fprintf(stderr, "pressed-state capture failed\n");
            return 4;
        }
        std::printf("pressed-capture=%ld,%ld,%s\n", x, y, argv[4]);
        return 0;
    }
    if (std::strcmp(argv[1], "native-resize") == 0) {
        if (argc != 4) return 2;
        const int width = static_cast<int>(std::strtol(argv[2], nullptr, 10));
        const int height = static_cast<int>(std::strtol(argv[3], nullptr, 10));
        if (width <= 0 || height <= 0 ||
            !SetWindowPos(window, nullptr, 0, 0, width, height,
                          SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE)) {
            std::fprintf(stderr, "native resize failed\n");
            return 4;
        }
        UpdateWindow(window);
        std::printf("resized=%dx%d\n", width, height);
        return 0;
    }
    if (std::strcmp(argv[1], "capture-burst") == 0) {
        if (argc != 5) return 2;
        const int count = static_cast<int>(std::strtol(argv[2], nullptr, 10));
        const DWORD interval = static_cast<DWORD>(
            std::max<long>(0, std::strtol(argv[3], nullptr, 10)));
        if (count <= 0 || count > 120) return 2;
        for (int index = 0; index < count; ++index) {
            const std::string path = std::string(argv[4]) + "-" +
                std::to_string(index) + ".bmp";
            DWORD_PTR result{};
            const std::string capture_command =
                std::string("GUI.Forms.Automation/1 capture ") + path;
            if (!send_automation(window, capture_command, result) || result != TRUE) {
                std::fprintf(stderr, "burst capture failed at frame %d\n", index);
                return 4;
            }
            if (index + 1 < count && interval != 0) Sleep(interval);
        }
        std::printf("captured-burst=%d interval-ms=%lu prefix=%s\n",
                    count, static_cast<unsigned long>(interval), argv[4]);
        return 0;
    }
    std::string command = "GUI.Forms.Automation/1 ";
    command += argv[1];
    for (int index = 2; index < argc; ++index) {
        command += ' ';
        command += argv[index];
    }
    DWORD_PTR result{};
    if (!send_automation(window, command, result)) {
        std::fprintf(stderr, "automation command timed out\n");
        return 4;
    }
    std::printf("accepted=%lu command=%s\n",
                static_cast<unsigned long>(result), command.c_str());
    return result == TRUE ? 0 : 5;
}
