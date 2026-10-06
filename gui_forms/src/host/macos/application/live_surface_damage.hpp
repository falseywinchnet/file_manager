#pragma once

#include "../../../core/damage/device_damage/device_damage.hpp"

#include <algorithm>
#include <span>

namespace gui_forms::detail {

// Only whole device pixels inside the actual hard clip are guaranteed covered.
// Outward rounding a clip would incorrectly erase a fractional overlay gap.
[[nodiscard]] inline Rect opaque_live_clip(const Rect clip, const double scale) noexcept {
    if (!clip.finite() || clip.empty() || !std::isfinite(scale) || scale <= 0.0) return {};
    const double left = std::ceil(clip.x * scale) / scale;
    const double top = std::ceil(clip.y * scale) / scale;
    const double right = std::floor(clip.right() * scale) / scale;
    const double bottom = std::floor(clip.bottom() * scale) / scale;
    const Rect result{left, top, right - left, bottom - top};
    if (!result.finite() || result.empty()) return {};
    return result;
}

// Clips have already been rounded inward by opaque_live_clip. Sweep their
// vertical edges, extending each horizontal interval only through overlapping
// coverage. No bounding-box approximation, pixel loop or scratch allocation.
[[nodiscard]] inline bool opaque_live_damage_covers(
    const Rect dirty, const double scale, const std::span<const Rect> clips,
    const bool reusable_raster, const bool retained_damage) noexcept {
    if (!reusable_raster || retained_damage || clips.empty() || !dirty.finite()) return false;
    const Rect target = align_damage_outward(dirty, scale);
    if (target.empty() || !target.finite()) return false;
    double top = target.y;
    while (top < target.bottom()) {
        double bottom = target.bottom();
        for (const Rect clip : clips) {
            if (clip.y > top) bottom = std::min(bottom, clip.y);
            if (clip.bottom() > top) bottom = std::min(bottom, clip.bottom());
        }
        double left = target.x;
        while (left < target.right()) {
            double right = left;
            for (const Rect clip : clips) {
                if (clip.y <= top && clip.bottom() >= bottom && clip.x <= left) {
                    right = std::max(right, clip.right());
                }
            }
            if (right <= left) return false;
            left = right;
        }
        top = bottom;
    }
    return true;
}

} // namespace gui_forms::detail
