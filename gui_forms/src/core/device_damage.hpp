#pragma once

#include "gui_forms/types.hpp"

#include <cmath>

namespace gui_forms::detail {

// Internal host/renderer contract: repaint whole device pixels even when a
// retained control reports fractional logical bounds.
[[nodiscard]] inline Rect align_damage_outward(Rect rect, double scale) noexcept {
    if (rect.empty() || !std::isfinite(scale) || scale <= 0.0) {
        return {};
    }
    const double left = std::floor(rect.x * scale) / scale;
    const double top = std::floor(rect.y * scale) / scale;
    const double right = std::ceil((rect.x + rect.width) * scale) / scale;
    const double bottom = std::ceil((rect.y + rect.height) * scale) / scale;
    return {left, top, right - left, bottom - top};
}

} // namespace gui_forms::detail
