#include <gui_forms/host/services/host_services.hpp>

#include <cstddef>
#include <utility>

// Call on the thread that owns services. The application's document stays
// straight RGBA; no display bitmap participates in copying or pasting.
gui_forms::HostServiceStatus copy_pixels(gui_forms::HostServices& services,
                                        gui_forms::HostImageView selection) {
    return services.write_clipboard_image(selection);
}

gui_forms::HostServiceStatus paste_pixels(gui_forms::HostServices& services,
                                         gui_forms::HostImage& document,
                                         bool& changed) {
    changed = false;
    gui_forms::HostClipboardImageResult result = services.read_clipboard_image();
    if (!result.status.accepted() || !result.has_image) return result.status;
    document = std::move(result.image);
    changed = true;
    return result.status;
}
