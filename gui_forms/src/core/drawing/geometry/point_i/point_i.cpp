#include "gui_forms/drawing/geometry/point_i/point_i.hpp"

#include "../../support/drawing_support.hpp"

void PointI::offset(std::int32_t dx, std::int32_t dy) noexcept {
    x = clamp_i32(static_cast<std::int64_t>(x) + dx);
    y = clamp_i32(static_cast<std::int64_t>(y) + dy);
}

} // namespace gui_drawing
