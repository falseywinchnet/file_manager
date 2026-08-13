#pragma once

#include "gui_forms/window.hpp"

#include <memory>

namespace gui_forms::connected_controls_lab {

[[nodiscard]] std::unique_ptr<Window> make_connected_controls_lab();

} // namespace gui_forms::connected_controls_lab
