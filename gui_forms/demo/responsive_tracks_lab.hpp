#pragma once

#include "gui_forms/window.hpp"

#include <memory>

namespace gui_forms::responsive_tracks_lab {

inline constexpr double controller_height = 44.0;

[[nodiscard]] std::unique_ptr<Window> make_responsive_tracks_lab();

} // namespace gui_forms::responsive_tracks_lab
