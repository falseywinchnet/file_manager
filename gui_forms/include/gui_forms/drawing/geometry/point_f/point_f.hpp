#pragma once

namespace gui_drawing {

struct PointF final {
    double x{};
    double y{};
    void offset(double dx, double dy);
    friend constexpr bool operator==(const PointF& left,
                                     const PointF& right) noexcept {
        return left.x == right.x && left.y == right.y;
    }
};

} // namespace gui_drawing
