#include "responsive_tracks_lab.hpp"
#include "windows_host.hpp"

#include <utility>

int main() {
    gui_forms::host::WindowsHostOptions options;
    options.title = "GUI.Forms Responsive Tracks Lab";
    options.initial_size = {1450.0, 894.0};
    options.minimum_size = {150.0, 194.0};
    return gui_forms::host::run_windows(
        gui_forms::responsive_tracks_lab::make_responsive_tracks_lab(),
        std::move(options));
}
