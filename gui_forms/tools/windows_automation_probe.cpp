#include <windows.h>

#include <cstdio>
#include <cstring>
#include <string>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: gui_forms_windows_probe <command> [argument]\n");
        return 2;
    }
    HWND window = FindWindowW(L"GUIForms.Window.v1", nullptr);
    if (window == nullptr) {
        std::fprintf(stderr, "GUI.Forms window not found\n");
        return 3;
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
