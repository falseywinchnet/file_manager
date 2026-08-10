#pragma once

#include "../state/live_surface_state.hpp"

namespace gui_forms::detail {

[[nodiscard]] bool valid_live_surface_description(
    LiveSurfaceDescription description) noexcept;
[[nodiscard]] std::shared_ptr<LiveSurfaceBuffer> make_live_surface_buffer(
    LiveSurfaceDescription description);
[[nodiscard]] Rect full_live_surface_damage(
    LiveSurfaceDescription description) noexcept;

} // namespace gui_forms::detail
