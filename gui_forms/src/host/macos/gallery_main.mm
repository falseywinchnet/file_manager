#include "macos_host.hpp"
#include "gallery.hpp"

int main() {
    return gui_forms::host::run_macos(gui_forms::gallery::make_gallery());
}

