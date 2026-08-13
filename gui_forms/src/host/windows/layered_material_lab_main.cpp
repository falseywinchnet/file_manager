#include "layered_material_lab.hpp"
#include "windows_host.hpp"

#include <string_view>
#include <utility>

int main(int argc, char** argv) {
    gui_forms::host::WindowsHostOptions options;
    options.title = "GUI.Forms Layered Material Fidelity Lab";
    options.initial_size = {1400.0, 760.0};
    options.minimum_size = {1200.0, 680.0};
    for (int index = 1; index < argc; ++index) {
        if (std::string_view(argv[index]) == "--automation") {
            options.automation_enabled = true;
        }
    }
    return gui_forms::host::run_windows(
        gui_forms::layered_material_lab::make_layered_material_lab(),
        std::move(options));
}
