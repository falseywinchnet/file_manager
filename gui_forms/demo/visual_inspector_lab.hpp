#pragma once

#include "gui_forms/window.hpp"

#include <memory>

namespace gui_forms::visual_inspector_lab {

// Independent visual-state inspection demoboard. It consumes only public
// GUI.Forms APIs and deterministic in-memory controls.
[[nodiscard]] std::unique_ptr<Window> make_visual_inspector_lab();

} // namespace gui_forms::visual_inspector_lab
