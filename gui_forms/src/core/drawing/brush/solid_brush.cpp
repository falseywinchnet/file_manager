#include "../support/drawing_support.hpp"

SolidBrush::SolidBrush(Color color) : color_(color) {}

Color SolidBrush::color() const {
    require_alive();
    return color_;
}

BrushSnapshot SolidBrush::snapshot() const {
    require_alive();
    BrushSnapshot result;
    result.kind = BrushKind::solid;
    result.primary = color_;
    return result;
}


} // namespace gui_drawing

