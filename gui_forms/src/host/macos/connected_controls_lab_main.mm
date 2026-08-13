#include "connected_controls_lab.hpp"
#include "macos_host.hpp"

#include <utility>

int main() {
    gui_forms::host::MacHostOptions options;
    options.title = "GUI.Forms Connected Controls Lab";
    options.initial_size = {1400.0, 760.0};
    options.minimum_size = {1280.0, 640.0};
    return gui_forms::host::run_macos(
        gui_forms::connected_controls_lab::make_connected_controls_lab(),
        std::move(options));
}
