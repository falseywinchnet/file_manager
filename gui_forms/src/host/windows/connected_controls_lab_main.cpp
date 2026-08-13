#include "connected_controls_lab.hpp"
#include "windows_host.hpp"

#include <string_view>
#include <utility>

int main(int argc, char** argv) {
    gui_forms::host::WindowsHostOptions options;
    options.title = "GUI.Forms Connected Controls Lab";
    options.initial_size = {1400.0, 760.0};
    options.minimum_size = {1280.0, 640.0};
    for (int index = 1; index < argc; ++index) {
        if (std::string_view(argv[index]) == "--automation") {
            options.automation_enabled = true;
        }
    }
    return gui_forms::host::run_windows(
        gui_forms::connected_controls_lab::make_connected_controls_lab(),
        std::move(options));
}
