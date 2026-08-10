#include "tool_tip_layer.hpp"

namespace gui_forms {

ToolTipLayer::ToolTipLayer(StableId stable_id)
    : Control(std::move(stable_id)) {
    set_paint_plane(PaintPlane::overlay);
    set_focusable(false);
}

bool ToolTipLayer::hit_test_local(Point) const {
    return false;
}

} // namespace gui_forms
