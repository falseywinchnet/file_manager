#pragma once

#include "gui_forms/drawing/brush/brush.hpp"

namespace gui_drawing {

struct PenSnapshot final {
    Color color;
    double width{1.0};
    DashStyle dash_style{DashStyle::solid};
    std::vector<double> dash_pattern;
};

class Pen final : public DrawingObject {
public:
    explicit Pen(Color color, double width = 1.0);
    explicit Pen(const Brush& brush, double width = 1.0);

    void set_width(double width);
    void set_dash_style(DashStyle style);
    void set_dash_pattern(std::span<const double> pattern);
    [[nodiscard]] PenSnapshot snapshot() const;

private:
    Color color_;
    double width_{1.0};
    DashStyle dash_style_{DashStyle::solid};
    std::vector<double> dash_pattern_;
};

} // namespace gui_drawing
