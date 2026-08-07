#include "gui_forms/types.hpp"

#include <algorithm>
#include <cmath>

namespace gui_forms {
namespace {

Color interpolate(Color first, Color second, double amount) noexcept {
    amount = std::clamp(amount, 0.0, 1.0);
    const auto channel = [amount](std::uint8_t left, std::uint8_t right) {
        return static_cast<std::uint8_t>(std::lround(
            static_cast<double>(left) +
            (static_cast<double>(right) - static_cast<double>(left)) * amount));
    };
    return Color::rgba(channel(first.red, second.red),
                       channel(first.green, second.green),
                       channel(first.blue, second.blue),
                       channel(first.alpha, second.alpha));
}

Color sample_gradient(std::span<const GradientStop> stops, double offset) noexcept {
    if (stops.empty()) return {};
    offset = std::clamp(offset, 0.0, 1.0);
    for (std::size_t index = 1U; index < stops.size(); ++index) {
        if (offset <= stops[index].offset) {
            const double width = stops[index].offset - stops[index - 1U].offset;
            const double local = width <= 0.0
                ? 1.0 : (offset - stops[index - 1U].offset) / width;
            return interpolate(stops[index - 1U].color, stops[index].color, local);
        }
    }
    return stops.back().color;
}

double clamped_radius(Rect rect, double radius) noexcept {
    return std::clamp(radius, 0.0,
                      std::max(0.0, std::min(rect.width, rect.height) * 0.5));
}

} // namespace

bool valid_gradient_stops(std::span<const GradientStop> stops) noexcept {
    if (stops.size() < 2U || stops.size() > maximum_gradient_stops ||
        !std::isfinite(stops.front().offset) ||
        !std::isfinite(stops.back().offset) ||
        stops.front().offset != 0.0 || stops.back().offset != 1.0) {
        return false;
    }
    double previous = -1.0;
    for (const GradientStop& stop : stops) {
        if (!std::isfinite(stop.offset) || stop.offset < 0.0 ||
            stop.offset > 1.0 || stop.offset <= previous) {
            return false;
        }
        previous = stop.offset;
    }
    return true;
}

void Painter::clip_rounded_rect(Rect rect, double) {
    clip_rect(rect);
}

void Painter::fill_rounded_rect(Rect rect, double radius, Color color) {
    if (rect.empty()) return;
    radius = clamped_radius(rect, radius);
    if (radius <= 0.0) {
        fill_rect(rect, color);
        return;
    }
    fill_rect({rect.x, rect.y + radius, rect.width,
               std::max(0.0, rect.height - radius * 2.0)}, color);
    const unsigned slices = std::max(1U, static_cast<unsigned>(std::ceil(radius)));
    const double step = radius / static_cast<double>(slices);
    for (unsigned slice = 0; slice < slices; ++slice) {
        const double sample_y = (static_cast<double>(slice) + 0.5) * step;
        const double dy = radius - sample_y;
        const double chord = std::sqrt(std::max(0.0, radius * radius - dy * dy));
        const double left = rect.x + radius - chord;
        const double width = std::max(0.0, rect.width - 2.0 * (radius - chord));
        fill_rect({left, rect.y + sample_y - step * 0.5, width, step}, color);
        fill_rect({left, rect.y + rect.height - sample_y - step * 0.5,
                   width, step}, color);
    }
}

void Painter::stroke_rounded_rect(Rect rect, double, Color color, double width) {
    stroke_rect(rect, color, width);
}

void Painter::fill_linear_gradient(Rect rect, Point start, Point end,
                                   std::span<const GradientStop> stops) {
    if (rect.empty() || !valid_gradient_stops(stops)) return;
    constexpr unsigned slices = 48U;
    const bool horizontal = std::abs(end.x - start.x) >= std::abs(end.y - start.y);
    for (unsigned slice = 0; slice < slices; ++slice) {
        const double first = static_cast<double>(slice) / slices;
        const double next = static_cast<double>(slice + 1U) / slices;
        const Color color = sample_gradient(stops, (first + next) * 0.5);
        if (horizontal) {
            fill_rect({rect.x + rect.width * first, rect.y,
                       rect.width * (next - first), rect.height}, color);
        } else {
            fill_rect({rect.x, rect.y + rect.height * first, rect.width,
                       rect.height * (next - first)}, color);
        }
    }
}

void Painter::fill_linear_gradient_spread(
    Rect rect, Point start, Point end, std::span<const GradientStop> stops,
    GradientSpreadMode spread) {
    // Minimal painters remain source-compatible and coherent. Production and
    // recording painters override this operation so repeat/reflect survive the
    // retained display list exactly.
    static_cast<void>(spread);
    fill_linear_gradient(rect, start, end, stops);
}

void Painter::fill_radial_gradient(Rect rect, Point center, Size radii,
                                   std::span<const GradientStop> stops) {
    if (rect.empty() || radii.width <= 0.0 || radii.height <= 0.0 ||
        !valid_gradient_stops(stops)) {
        return;
    }
    // The approximation may be centred outside the target rectangle. Keep
    // the fallback inside the same authored bounds as a native shader.
    save();
    clip_rect(rect);
    constexpr unsigned rings = 40U;
    for (unsigned ring = rings; ring > 0U; --ring) {
        const double amount = static_cast<double>(ring) / rings;
        const double width = std::min(rect.width, radii.width * 2.0 * amount);
        const double height = std::min(rect.height, radii.height * 2.0 * amount);
        fill_rounded_rect(
            {center.x - width * 0.5, center.y - height * 0.5, width, height},
            std::min(width, height) * 0.5,
            sample_gradient(stops, amount));
    }
    restore();
}

void Painter::draw_box_shadow(Rect rect, double corner_radius, Point offset,
                              double blur_radius, double spread, Color color) {
    if (rect.empty() || color.alpha == 0U || !std::isfinite(blur_radius) ||
        !std::isfinite(spread) || blur_radius < 0.0) {
        return;
    }
    constexpr unsigned rings = 12U;
    const double extent = std::max(0.0, spread) + blur_radius * 1.5;
    for (unsigned ring = rings; ring > 0U; --ring) {
        const double amount = static_cast<double>(ring) / rings;
        const double expansion = spread + extent * amount;
        Color ring_color = color;
        ring_color.alpha = static_cast<std::uint8_t>(std::lround(
            static_cast<double>(color.alpha) * (1.0 - amount) /
            static_cast<double>(rings)));
        stroke_rounded_rect(
            {rect.x + offset.x - expansion, rect.y + offset.y - expansion,
             rect.width + expansion * 2.0, rect.height + expansion * 2.0},
            std::max(0.0, corner_radius + expansion), ring_color,
            std::max(0.5, extent / rings));
    }
}

void Painter::draw_image_region(ImageId image, Rect source, Rect destination,
                                double opacity) {
    if (image.value == 0U || source.empty() || destination.empty() ||
        !source.finite() || !destination.finite() ||
        !std::isfinite(opacity) || opacity <= 0.0) {
        return;
    }
    draw_image(image, destination, std::clamp(opacity, 0.0, 1.0));
}

void Painter::fill_image_pattern(ImageId image, Size source_pixel_size,
                                 Rect destination, Size logical_tile_size,
                                 ImagePatternWrap wrap, double opacity) {
    const bool known_wrap = wrap == ImagePatternWrap::tile;
    if (!known_wrap || image.value == 0U || destination.empty() ||
        !destination.finite() || !std::isfinite(source_pixel_size.width) ||
        !std::isfinite(source_pixel_size.height) ||
        !std::isfinite(logical_tile_size.width) ||
        !std::isfinite(logical_tile_size.height) ||
        source_pixel_size.width <= 0.0 || source_pixel_size.height <= 0.0 ||
        logical_tile_size.width <= 0.0 || logical_tile_size.height <= 0.0 ||
        !std::isfinite(opacity) || opacity <= 0.0) {
        return;
    }
    constexpr std::size_t maximum_fallback_tiles = 4096U;
    const std::size_t columns = static_cast<std::size_t>(
        std::ceil(destination.width / logical_tile_size.width));
    const std::size_t rows = static_cast<std::size_t>(
        std::ceil(destination.height / logical_tile_size.height));
    if (columns == 0U || rows == 0U ||
        columns > maximum_fallback_tiles / rows) {
        draw_image_region(image,
            {0.0, 0.0, source_pixel_size.width, source_pixel_size.height},
            destination, opacity);
        return;
    }
    for (std::size_t row = 0; row < rows; ++row) {
        const double y = destination.y +
            static_cast<double>(row) * logical_tile_size.height;
        const double height = std::min(logical_tile_size.height,
                                       destination.bottom() - y);
        const double source_height = std::min(
            source_pixel_size.height,
            height * source_pixel_size.height / logical_tile_size.height);
        for (std::size_t column = 0; column < columns; ++column) {
            const double x = destination.x +
                static_cast<double>(column) * logical_tile_size.width;
            const double width = std::min(logical_tile_size.width,
                                          destination.right() - x);
            const double source_width = std::min(
                source_pixel_size.width,
                width * source_pixel_size.width / logical_tile_size.width);
            draw_image_region(image,
                {0.0, 0.0, source_width, source_height},
                {x, y, width, height}, opacity);
        }
    }
}

Rect Rect::intersection(Rect left, Rect right) noexcept {
    const double x0 = std::max(left.x, right.x);
    const double y0 = std::max(left.y, right.y);
    const double x1 = std::min(left.x + left.width, right.x + right.width);
    const double y1 = std::min(left.y + left.height, right.y + right.height);
    return {x0, y0, std::max(0.0, x1 - x0), std::max(0.0, y1 - y0)};
}

Rect Rect::united(Rect left, Rect right) noexcept {
    if (left.empty()) {
        return right;
    }
    if (right.empty()) {
        return left;
    }
    const double x0 = std::min(left.x, right.x);
    const double y0 = std::min(left.y, right.y);
    const double x1 = std::max(left.x + left.width, right.x + right.width);
    const double y1 = std::max(left.y + left.height, right.y + right.height);
    return {x0, y0, x1 - x0, y1 - y0};
}

void DamageRegion::add(Rect rect) {
    if (rect.empty()) {
        return;
    }

    bool merged = true;
    while (merged) {
        merged = false;
        for (auto iterator = rectangles_.begin(); iterator != rectangles_.end(); ++iterator) {
            const Rect united = Rect::united(*iterator, rect);
            const double exact_union_area = iterator->area() + rect.area() -
                                            Rect::intersection(*iterator, rect).area();
            const double tolerance = std::max(1.0, exact_union_area) * 1.0e-12;
            if (std::abs(united.area() - exact_union_area) <= tolerance) {
                rect = united;
                rectangles_.erase(iterator);
                ++compaction_count_;
                merged = true;
                break;
            }
        }
    }
    rectangles_.push_back(rect);

    if (rectangles_.size() > maximum_rectangles) {
        const Rect collapsed = bounds();
        rectangles_.assign(1, collapsed);
        ++collapse_count_;
    }
}

void DamageRegion::clear() noexcept {
    rectangles_.clear();
}

Rect DamageRegion::bounds() const noexcept {
    Rect result{};
    for (const Rect rect : rectangles_) {
        result = Rect::united(result, rect);
    }
    return result;
}

double DamageRegion::area() const noexcept {
    if (rectangles_.empty()) {
        return 0.0;
    }

    std::vector<double> edges;
    edges.reserve(rectangles_.size() * 2U);
    for (const Rect rect : rectangles_) {
        if (!rect.empty()) {
            edges.push_back(rect.x);
            edges.push_back(rect.x + rect.width);
        }
    }
    std::sort(edges.begin(), edges.end());
    edges.erase(std::unique(edges.begin(), edges.end()), edges.end());

    double result = 0.0;
    for (std::size_t index = 1; index < edges.size(); ++index) {
        const double x0 = edges[index - 1U];
        const double x1 = edges[index];
        if (x1 <= x0) {
            continue;
        }
        std::vector<std::pair<double, double>> intervals;
        for (const Rect rect : rectangles_) {
            if (!rect.empty() && rect.x < x1 && rect.x + rect.width > x0) {
                intervals.emplace_back(rect.y, rect.y + rect.height);
            }
        }
        std::sort(intervals.begin(), intervals.end());
        double covered = 0.0;
        if (!intervals.empty()) {
            double start = intervals.front().first;
            double end = intervals.front().second;
            for (std::size_t interval = 1; interval < intervals.size(); ++interval) {
                if (intervals[interval].first <= end) {
                    end = std::max(end, intervals[interval].second);
                } else {
                    covered += end - start;
                    start = intervals[interval].first;
                    end = intervals[interval].second;
                }
            }
            covered += end - start;
        }
        result += (x1 - x0) * covered;
    }
    return result;
}

} // namespace gui_forms
