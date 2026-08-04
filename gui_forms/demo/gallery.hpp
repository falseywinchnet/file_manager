#pragma once

#include "gui_forms/window.hpp"

#include <memory>

namespace gui_forms::gallery {

// Builds the compiled-DML control gallery. The retained controls own their
// semantic GalleryModel; callers only own the native Window boundary.
[[nodiscard]] std::unique_ptr<Window> make_gallery();

}  // namespace gui_forms::gallery
