#include "visual_inspector_lab.hpp"
#include "windows_host.hpp"

#include <string_view>
#include <utility>

int main(int argc, char** argv) {
    gui_forms::host::WindowsHostOptions options;
    options.title = "GUI.Forms Visual Inspector Lab";
    options.initial_size = {1160.0, 700.0};
    options.minimum_size = {980.0, 620.0};
    for (int index = 1; index < argc; ++index) {
        if (std::string_view(argv[index]) == "--automation") {
            options.automation_enabled = true;
        }
    }
    return gui_forms::host::run_windows(
        gui_forms::visual_inspector_lab::make_visual_inspector_lab(),
        std::move(options));
}
