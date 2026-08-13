#include "seam_proximity_lab.hpp"
#include "macos_host.hpp"

#include <utility>

int main() {
    gui_forms::host::MacHostOptions options;
    options.title = "GUI.Forms Physical Seam + Proximity Lab";
    options.initial_size = {1360.0, 760.0};
    options.minimum_size = {1200.0, 720.0};
    return gui_forms::host::run_macos(
        gui_forms::seam_proximity_lab::make_seam_proximity_lab(),
        std::move(options));
}
