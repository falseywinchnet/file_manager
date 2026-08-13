#include "layered_material_lab.hpp"
#include "macos_host.hpp"

#include <utility>

int main() {
    gui_forms::host::MacHostOptions options;
    options.title = "GUI.Forms Layered Material Fidelity Lab";
    options.initial_size = {1400.0, 760.0};
    options.minimum_size = {1200.0, 680.0};
    return gui_forms::host::run_macos(
        gui_forms::layered_material_lab::make_layered_material_lab(),
        std::move(options));
}
