#pragma once

namespace gui_drawing {

struct PointF final {
    double x{};
    double y{};
    void offset(double dx, double dy);
    friend constexpr bool operator==(const PointF&, const PointF&) = default;
};

} // namespace gui_drawing
