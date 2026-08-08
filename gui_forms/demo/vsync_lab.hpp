#pragma once

#include "gui_forms/window.hpp"

#include <memory>

namespace gui_forms::vsync_lab {

// Native GUI.Forms high-rate presentation diagnostic. The producer owns a
// renderer-neutral LiveSurface and never calls the retained tree directly.
// Ordinary controls remain on the UI thread so the lab exposes scheduling or
// ownership contention instead of hiding it behind a compatibility facade.
[[nodiscard]] std::unique_ptr<Window> make_vsync_lab();

} // namespace gui_forms::vsync_lab
