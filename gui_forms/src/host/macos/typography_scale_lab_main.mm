#include "macos_host.hpp"
#include "typography_scale_lab.hpp"

#include <utility>

int main() {
    gui_forms::host::MacHostOptions options;
    options.title = "GUI.Forms Typography + Scale Lab";
    options.initial_size = {1320.0, 840.0};
    options.minimum_size = {1040.0, 720.0};
    return gui_forms::host::run_macos(
        gui_forms::typography_scale_lab::make_typography_scale_lab(),
        std::move(options));
}
