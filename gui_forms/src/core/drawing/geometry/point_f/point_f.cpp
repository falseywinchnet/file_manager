#include "gui_forms/drawing/geometry/point_f/point_f.hpp"

#include "../../support/drawing_support.hpp"

void PointF::offset(double dx, double dy) {
    require_finite(dx, "point offset x");
    require_finite(dy, "point offset y");
    require_finite(*this, "point");
    const PointF result{x + dx, y + dy};
    require_finite(result, "offset point");
    *this = result;
}

} // namespace gui_drawing
