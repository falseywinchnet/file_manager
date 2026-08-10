#include "../support/drawing_support.hpp"

PathGradientBrush::PathGradientBrush(std::span<const PointF> points,
                                     WrapMode wrap_mode)
    : points_(points.begin(), points.end()), wrap_mode_(wrap_mode) {
    require_enum(wrap_mode, 4U, "path gradient wrap mode");
    if (points.size() < 3U || points.size() > GraphicsPath::maximum_elements) {
        throw std::invalid_argument("path gradient requires 3 through 1000000 points");
    }
    for (const PointF point : points_) {
        require_finite(point, "path gradient point");
        center_.x += point.x / static_cast<double>(points_.size());
        center_.y += point.y / static_cast<double>(points_.size());
    }
    require_finite(center_, "path gradient center");
}

void PathGradientBrush::set_center_color(Color color) {
    require_alive();
    center_color_ = color;
}

void PathGradientBrush::set_center_point(PointF point) {
    require_alive();
    require_finite(point, "path gradient center");
    center_ = point;
}

void PathGradientBrush::set_surround_colors(std::span<const Color> colors) {
    require_alive();
    if (colors.empty() || colors.size() > points_.size()) {
        throw std::invalid_argument(
            "path gradient surround colors require 1 through point-count entries");
    }
    surround_colors_.assign(colors.begin(), colors.end());
}

void PathGradientBrush::set_interpolation_colors(const ColorBlend& blend) {
    require_alive();
    require_blend_positions(blend.positions, blend.colors.size());
    colors_ = blend.colors;
    positions_ = blend.positions;
}

BrushSnapshot PathGradientBrush::snapshot() const {
    require_alive();
    BrushSnapshot result;
    result.kind = BrushKind::path_gradient;
    result.primary = center_color_;
    result.secondary = surround_colors_.front();
    result.wrap_mode = wrap_mode_;
    result.center = center_;
    result.points = points_;
    result.colors = colors_.empty() ?
        std::vector<Color>{center_color_, surround_colors_.front()} : colors_;
    result.positions = positions_.empty() ?
        std::vector<double>{0.0, 1.0} : positions_;
    return result;
}


} // namespace gui_drawing

