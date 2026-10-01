#include "prepared_text_compositor.hpp"

#include <algorithm>
#include <cmath>

namespace gui_forms::host::detail {
namespace {

bool valid_coordinate(const double value) noexcept {
    const bool valid = std::isfinite(value) && std::abs(value) <= 100'000'000.0;
    return valid;
}

bool valid_rect(const Rect rect) noexcept {
    const bool valid = valid_coordinate(rect.x) && valid_coordinate(rect.y) &&
        valid_coordinate(rect.width) && valid_coordinate(rect.height) && rect.width >= 0 && rect.height >= 0;
    return valid;
}

struct CompositeBounds final {
    std::int64_t left{};
    std::int64_t top{};
    std::int64_t right{};
    std::int64_t bottom{};
    std::int64_t mask_left{};
    std::int64_t mask_top{};
};

bool inside_round(const double x, const double y, const Rect rect, const double radius) noexcept {
    if (x < rect.x || y < rect.y || x >= rect.x + rect.width || y >= rect.y + rect.height) return false;
    const double nearest_x = std::clamp(x, rect.x + radius, rect.x + rect.width - radius);
    const double nearest_y = std::clamp(y, rect.y + radius, rect.y + rect.height - radius);
    const double dx = x - nearest_x;
    const double dy = y - nearest_y;
    const bool inside = dx * dx + dy * dy <= radius * radius;
    return inside;
}

template<bool Rounded>
void composite_rows(const std::span<const std::uint8_t> source, const std::size_t source_stride,
    const PreparedCompositeTarget target, const PreparedCompositeClip clip, const CompositeBounds bounds, const double scale, const Color color) noexcept {
    for (std::int64_t y = bounds.top; y < bounds.bottom; ++y) {
        const std::size_t source_row = static_cast<std::size_t>(y - bounds.mask_top) * source_stride;
        const std::size_t target_row = static_cast<std::size_t>(y) * target.stride;
        const double logical_y = (static_cast<double>(y) + 0.5) / scale;
        for (std::int64_t x = bounds.left; x < bounds.right; ++x) {
            if constexpr (Rounded) {
                const double logical_x = (static_cast<double>(x) + 0.5) / scale;
                if (!inside_round(logical_x, logical_y, clip.rounded_rect, clip.radius)) continue;
            }
            const std::size_t source_index = source_row + static_cast<std::size_t>(x - bounds.mask_left);
            const unsigned alpha = (static_cast<unsigned>(source[source_index]) * color.alpha + 127U) / 255U;
            if (alpha == 0) continue;
            const std::size_t destination_index = target_row + static_cast<std::size_t>(x);
            const std::uint32_t previous = target.pixels[destination_index];
            const unsigned inverse = 255U - alpha;
            const unsigned blue = (color.blue * alpha + (previous & 255U) * inverse + 127U) / 255U;
            const unsigned green = (color.green * alpha + ((previous >> 8U) & 255U) * inverse + 127U) / 255U;
            const unsigned red = (color.red * alpha + ((previous >> 16U) & 255U) * inverse + 127U) / 255U;
            target.pixels[destination_index] = blue | (green << 8U) | (red << 16U) | 0xff000000U;
        }
    }
}
} // namespace

PreparedTextStatus composite_prepared_mask(const GrayTextMask& mask, const PreparedCompositeTarget target,
    const PreparedCompositeClip clip, const Point baseline, const double scale, const Color color) noexcept {
    if (mask.empty() || target.width == 0 || target.height == 0 || target.stride < target.width ||
        target.height > target.pixels.size() / target.stride || !valid_rect(clip.rect) ||
        !valid_coordinate(baseline.x) || !valid_coordinate(baseline.y) || !std::isfinite(scale) ||
        scale < 0.5 || scale > 4 || scale != mask.metrics().device_scale) return PreparedTextStatus::invalid_geometry;
    if (clip.rounded && (!valid_rect(clip.rounded_rect) || !std::isfinite(clip.radius) || clip.radius < 0 ||
        clip.radius > std::min(clip.rounded_rect.width, clip.rounded_rect.height) * 0.5)) {
        return PreparedTextStatus::invalid_geometry;
    }
    if (mask.width() == 0 || mask.height() == 0 || color.alpha == 0 || clip.rect.empty()) return PreparedTextStatus::success;
    const std::span<const std::uint8_t> source = mask.pixels();
    if (mask.height() > source.size() / mask.width()) return PreparedTextStatus::invalid_geometry;
    CompositeBounds bounds{};
    bounds.mask_left = std::llround(baseline.x * scale) + mask.left();
    bounds.mask_top = std::llround(baseline.y * scale) + mask.top();
    // Pixel centers determine rectangular clipping, matching the rounded path.
    const std::int64_t clip_left = static_cast<std::int64_t>(std::ceil(clip.rect.x * scale - 0.5));
    const std::int64_t clip_top = static_cast<std::int64_t>(std::ceil(clip.rect.y * scale - 0.5));
    const std::int64_t clip_right = static_cast<std::int64_t>(std::ceil((clip.rect.x + clip.rect.width) * scale - 0.5));
    const std::int64_t clip_bottom = static_cast<std::int64_t>(std::ceil((clip.rect.y + clip.rect.height) * scale - 0.5));
    bounds.left = std::max<std::int64_t>({0, clip_left, bounds.mask_left});
    bounds.top = std::max<std::int64_t>({0, clip_top, bounds.mask_top});
    bounds.right = std::min<std::int64_t>({target.width, clip_right, bounds.mask_left + mask.width()});
    bounds.bottom = std::min<std::int64_t>({target.height, clip_bottom, bounds.mask_top + mask.height()});
    if (clip.rounded) composite_rows<true>(source, mask.width(), target, clip, bounds, scale, color);
    else composite_rows<false>(source, mask.width(), target, clip, bounds, scale, color);
    return PreparedTextStatus::success;
}
} // namespace gui_forms::host::detail
