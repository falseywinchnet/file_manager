#pragma once

#include "gui_forms/window.hpp"

#include <memory>

namespace gui_forms::showcase {

// Builds the independent comprehensive framework showcase. It intentionally
// consumes only public GUI.Forms controls and extension points.
[[nodiscard]] std::unique_ptr<Window> make_showcase();

} // namespace gui_forms::showcase
