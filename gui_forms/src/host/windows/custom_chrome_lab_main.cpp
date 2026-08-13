#include "custom_chrome_lab.hpp"
#include "windows_host.hpp"

#include <utility>

int main() {
    gui_forms::host::WindowsHostOptions options;
    options.title = "File Manager — GUI.Forms Custom Chrome Lab";
    options.initial_size = {1120.0, 580.0};
    options.minimum_size = {760.0, 430.0};
    // The retained lab is cross-platform. Win32 currently presents it below a
    // standard native title bar; Win32 full-client caption composition remains
    // explicitly unavailable rather than replacing caption controls with paint.
    return gui_forms::host::run_windows(
        gui_forms::custom_chrome_lab::make_custom_chrome_lab(),
        std::move(options));
}
