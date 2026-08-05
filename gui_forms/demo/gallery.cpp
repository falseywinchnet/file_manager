#include "gallery.hpp"

#include "gallery_assets.hpp"
#include "gallery_controls.hpp"

#include <chrono>
#include <span>
#include <stdexcept>

namespace gui_forms::gallery {

std::unique_ptr<Window> make_gallery()
{
    GalleryTree tree = build_gallery_tree();
    auto window = std::make_unique<Window>(std::move(tree.root), Size {900.0, 660.0});
    tree.context->window = window.get();
    tree.context->synchronize("gallery.initial");
    const ImageLoadResult status_badge =
        window->load_png(std::as_bytes(std::span{assets::validated_status_png}));
    if (!status_badge) {
        throw std::logic_error("compiled Gallery PNG failed core validation");
    }
    tree.context->status_badge = status_badge.image;
    window->perform_layout();
    if (const auto instrument = window->find("gallery.instrument")) {
        constexpr FrameInterval instrument_interval =
            std::chrono::nanoseconds(33'333'333);
        tree.context->instrument_frames = window->activate_surface(
            instrument, instrument_interval, FrameClock::now() + instrument_interval);
    }
    return window;
}

}  // namespace gui_forms::gallery
