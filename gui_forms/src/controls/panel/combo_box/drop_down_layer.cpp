#include "drop_down_layer.hpp"

#include <utility>

namespace gui_forms {

DropDownLayer::DropDownLayer(StableId stable_id) : Panel(std::move(stable_id)) {
    set_paint_plane(PaintPlane::overlay);
    set_border_style(BorderStyle::none);
    set_background(Color::rgba(0, 0, 0, 0));
}

void DropDownLayer::on_pointer(PointerEvent& event) {
    if (event.action == PointerAction::down &&
        event.button == PointerButton::primary) {
        dismissed_.emit();
        event.handled = true;
    }
}

void DropDownLayer::on_key_preview(KeyEvent& event) {
    if (event.action == KeyAction::down &&
        event.physical_key == PhysicalKey::escape) {
        dismissed_.emit();
        event.handled = true;
    }
}

} // namespace gui_forms
