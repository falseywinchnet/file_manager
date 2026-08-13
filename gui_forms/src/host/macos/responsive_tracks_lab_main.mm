#include "macos_host.hpp"
#include "responsive_tracks_lab.hpp"

#include <utility>

int main() {
    gui_forms::host::MacHostOptions options;
    options.title = "GUI.Forms Responsive Tracks Lab";
    options.initial_size = {1450.0, 894.0};
    options.minimum_size = {150.0, 194.0};
    return gui_forms::host::run_macos(
        gui_forms::responsive_tracks_lab::make_responsive_tracks_lab(),
        std::move(options));
}
