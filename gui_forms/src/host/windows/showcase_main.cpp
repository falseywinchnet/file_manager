#include "showcase.hpp"
#include "windows_host.hpp"

#include <string_view>

int main(int argc, char** argv) {
    gui_forms::host::WindowsHostOptions options;
    options.title = "GUI.Forms Complete Showcase";
    options.initial_size = {1280.0, 820.0};
    options.minimum_size = {980.0, 680.0};
    for (int index = 1; index < argc; ++index) {
        if (std::string_view(argv[index]) == "--automation") {
            options.automation_enabled = true;
        }
    }
    return gui_forms::host::run_windows(gui_forms::showcase::make_showcase(),
                                        std::move(options));
}
