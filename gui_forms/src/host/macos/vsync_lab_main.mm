#include "macos_host.hpp"
#include "vsync_lab.hpp"

#include <utility>

int main() {
    gui_forms::host::MacHostOptions options;
    options.title = "GUI.Forms VSync + Waterfall Lab";
    options.initial_size = {1280.0, 780.0};
    options.minimum_size = {1080.0, 680.0};
    options.print_metrics_on_close = true;
    return gui_forms::host::run_macos(gui_forms::vsync_lab::make_vsync_lab(),
                                      std::move(options));
}
