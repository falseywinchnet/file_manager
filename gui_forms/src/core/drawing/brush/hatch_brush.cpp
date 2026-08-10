#include "../support/drawing_support.hpp"

HatchBrush::HatchBrush(HatchStyle style, Color foreground, Color background)
    : style_(style), foreground_(foreground), background_(background) {
    require_enum(style, 5U, "hatch style");
}

BrushSnapshot HatchBrush::snapshot() const {
    require_alive();
    BrushSnapshot result;
    result.kind = BrushKind::hatch;
    result.primary = foreground_;
    result.secondary = background_;
    result.hatch_style = style_;
    return result;
}


} // namespace gui_drawing

