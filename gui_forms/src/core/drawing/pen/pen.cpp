#include "../support/drawing_support.hpp"

Pen::Pen(Color color, double width) : color_(color), width_(width) {
    require_pen_width(width);
}

Pen::Pen(const Brush& brush, double width)
    : color_(brush.snapshot().primary), width_(width) {
    require_pen_width(width);
}

void Pen::set_width(double width) {
    require_alive();
    require_pen_width(width);
    width_ = width;
}

void Pen::set_dash_style(DashStyle style) {
    require_alive();
    require_enum(style, 5U, "dash style");
    dash_style_ = style;
    if (style != DashStyle::custom) dash_pattern_.clear();
}

void Pen::set_dash_pattern(std::span<const double> pattern) {
    require_alive();
    if (pattern.empty() || pattern.size() > 256) {
        throw std::invalid_argument("dash pattern must contain 1 through 256 entries");
    }
    std::vector<double> copy;
    copy.reserve(pattern.size());
    for (const double entry : pattern) {
        require_finite(entry, "dash pattern entry");
        if (entry <= 0.0) {
            throw std::invalid_argument("dash pattern entries must be positive");
        }
        copy.push_back(entry);
    }
    dash_pattern_ = std::move(copy);
    dash_style_ = DashStyle::custom;
}

PenSnapshot Pen::snapshot() const {
    require_alive();
    return {color_, width_, dash_style_, dash_pattern_};
}


} // namespace gui_drawing

