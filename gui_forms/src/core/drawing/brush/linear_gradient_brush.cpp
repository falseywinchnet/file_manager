#include "../support/drawing_support.hpp"

LinearGradientBrush::LinearGradientBrush(RectF bounds, Color first, Color second,
                                         double angle, WrapMode wrap_mode)
    : bounds_(bounds), first_(first), second_(second), angle_(angle),
      wrap_mode_(wrap_mode) {
    require_finite(bounds, "linear gradient bounds");
    require_finite(angle, "linear gradient angle");
    require_enum(wrap_mode, 4U, "linear gradient wrap mode");
    if (bounds.empty() || std::abs(angle) > 1'000'000.0) {
        throw std::invalid_argument("linear gradient geometry must be nonempty and bounded");
    }
}

void LinearGradientBrush::set_blend(std::span<const double> factors,
                                    std::span<const double> positions) {
    require_alive();
    require_blend_positions(positions, factors.size());
    std::vector<Color> colors;
    colors.reserve(factors.size());
    for (const double factor : factors) {
        require_finite(factor, "gradient blend factor");
        if (factor < 0.0 || factor > 1.0) {
            throw std::invalid_argument("gradient blend factors must be in the unit interval");
        }
        colors.push_back(interpolate_color(first_, second_, factor));
    }
    colors_ = std::move(colors);
    positions_.assign(positions.begin(), positions.end());
}

void LinearGradientBrush::set_interpolation_colors(const ColorBlend& blend) {
    require_alive();
    require_blend_positions(blend.positions, blend.colors.size());
    colors_ = blend.colors;
    positions_ = blend.positions;
}

void LinearGradientBrush::set_wrap_mode(WrapMode mode) {
    require_alive();
    require_enum(mode, 4U, "linear gradient wrap mode");
    wrap_mode_ = mode;
}

BrushSnapshot LinearGradientBrush::snapshot() const {
    require_alive();
    BrushSnapshot result;
    result.kind = BrushKind::linear_gradient;
    result.primary = first_;
    result.secondary = second_;
    result.wrap_mode = wrap_mode_;
    result.bounds = bounds_;
    result.angle = angle_;
    result.colors = colors_.empty() ? std::vector<Color>{first_, second_} : colors_;
    result.positions = positions_.empty() ? std::vector<double>{0.0, 1.0} : positions_;
    return result;
}


} // namespace gui_drawing

