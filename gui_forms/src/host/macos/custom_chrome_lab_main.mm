#include "custom_chrome_lab.hpp"
#include "macos_host.hpp"

#include <utility>

int main() {
    gui_forms::host::MacHostOptions options;
    options.title = "File Manager — GUI.Forms Custom Chrome Lab";
    options.initial_size = {1120.0, 580.0};
    options.minimum_size = {760.0, 430.0};
    options.titlebar_presentation =
        gui_forms::host::MacTitlebarPresentation::transparent_full_size_content;
    options.window_drag_region_ids = {"custom-chrome.title-drag"};
    return gui_forms::host::run_macos(
        gui_forms::custom_chrome_lab::make_custom_chrome_lab(),
        std::move(options));
}
