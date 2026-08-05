#include <windows.h>

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
    if (GetWindowTextW(window, title, 512) == 0 || title[0] == L'\0') return TRUE;
    wchar_t class_name[128]{};
    GetClassNameW(window, class_name, 128);
    char title_utf8[2048]{};
    char class_utf8[512]{};
    DWORD process_id{};
    GetWindowThreadProcessId(window, &process_id);
    WideCharToMultiByte(CP_UTF8, 0, title, -1, title_utf8, 2048, nullptr, nullptr);
    WideCharToMultiByte(CP_UTF8, 0, class_name, -1, class_utf8, 512, nullptr, nullptr);
    std::printf("window=%p pid=%lu class=%s title=%s enabled=%d\n",
                static_cast<void*>(window), static_cast<unsigned long>(process_id),
                class_utf8, title_utf8,
                IsWindowEnabled(window));
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
    if (std::strcmp(argv[1], "native-close") == 0) {
        if (!PostMessageW(window, WM_CLOSE, 0, 0)) {
            std::fprintf(stderr, "failed to post WM_CLOSE\n");
            return 4;
        }
        std::printf("posted=WM_CLOSE\n");
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
    std::string command = "GUI.Forms.Automation/1 ";
    command += argv[1];
    for (int index = 2; index < argc; ++index) {
        command += ' ';
        command += argv[index];
    }
    COPYDATASTRUCT data{};
    data.dwData = 0x47464131U;
    data.cbData = static_cast<DWORD>(command.size() + 1);
    data.lpData = command.data();
    DWORD_PTR result{};
    if (!SendMessageTimeoutW(window, WM_COPYDATA, 0,
                             reinterpret_cast<LPARAM>(&data),
                             SMTO_ABORTIFHUNG | SMTO_BLOCK, 5000, &result)) {
        std::fprintf(stderr, "automation command timed out\n");
        return 4;
    }
    std::printf("accepted=%lu command=%s\n",
                static_cast<unsigned long>(result), command.c_str());
    return result == TRUE ? 0 : 5;
}
