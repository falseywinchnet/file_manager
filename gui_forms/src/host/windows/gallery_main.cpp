#include "gallery.hpp"
#include "windows_host.hpp"

#include <string_view>

int main(int argc, char** argv) {
    gui_forms::host::WindowsHostOptions options;
    for (int index = 1; index < argc; ++index) {
        if (std::string_view(argv[index]) == "--automation") {
            options.automation_enabled = true;
        }
    }
    return gui_forms::host::run_windows(gui_forms::gallery::make_gallery(),
                                        std::move(options));
}
