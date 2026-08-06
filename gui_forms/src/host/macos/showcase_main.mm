#include "macos_host.hpp"
#include "showcase.hpp"

#include <utility>

int main() {
    gui_forms::host::MacHostOptions options;
    options.title = "GUI.Forms Complete Showcase";
    options.initial_size = {1280.0, 820.0};
    options.minimum_size = {980.0, 680.0};
    return gui_forms::host::run_macos(gui_forms::showcase::make_showcase(),
                                      std::move(options));
}
