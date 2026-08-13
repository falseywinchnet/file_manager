#pragma once

#include "gui_forms/window.hpp"

#include <memory>

namespace gui_forms::typography_scale_lab {

[[nodiscard]] std::unique_ptr<Window> make_typography_scale_lab();

} // namespace gui_forms::typography_scale_lab
