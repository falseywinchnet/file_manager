#pragma once

#include "gui_forms/control.hpp"

#include <algorithm>
#include <cmath>

namespace gui_forms::container_layout_detail {

[[nodiscard]] inline double horizontal_extent(Insets insets) noexcept {
    return insets.left + insets.right;
}

[[nodiscard]] inline double vertical_extent(Insets insets) noexcept {
    return insets.top + insets.bottom;
}

[[nodiscard]] inline Size preferred_child_size(
    const Control::Ptr& child, Size available) {
    const Rect requested = (*child).requested_bounds();
    const Size measure_available{
        std::max(std::max(0.0, available.width), requested.width),
        std::max(std::max(0.0, available.height), requested.height)};
    Size result = (*child).measure(measure_available);
    if (!std::isfinite(result.width) || result.width < 0.0) result.width = 0.0;
    if (!std::isfinite(result.height) || result.height < 0.0) result.height = 0.0;
    return result;
}

} // namespace gui_forms::container_layout_detail
