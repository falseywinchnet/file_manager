#pragma once

#include "../context/gallery_context.hpp"
#include "gui_forms/gui_forms.hpp"

#include <memory>

namespace gui_forms::gallery {

struct GalleryTree final {
    Control::Ptr root;
    std::shared_ptr<GalleryContext> context;
};

[[nodiscard]] GalleryTree build_gallery_tree();

} // namespace gui_forms::gallery
