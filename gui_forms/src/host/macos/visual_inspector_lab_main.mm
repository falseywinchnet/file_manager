#include "macos_host.hpp"
#include "visual_inspector_lab.hpp"

#include <utility>

int main() {
    gui_forms::host::MacHostOptions options;
    options.title = "GUI.Forms Visual Inspector Lab";
    options.initial_size = {1160.0, 700.0};
    options.minimum_size = {980.0, 620.0};
    return gui_forms::host::run_macos(
        gui_forms::visual_inspector_lab::make_visual_inspector_lab(),
        std::move(options));
}
