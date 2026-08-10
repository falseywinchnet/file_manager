#include "spin_buttons.hpp"

#include "gui_forms/basic_controls.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace gui_forms {

SpinButtons::SpinButtons(StableId stable_id) : Control(std::move(stable_id)) {
    set_focusable(false);
    set_cursor(CursorKind::hand);
}

void SpinButtons::on_paint(Painter& painter, Rect) {
    const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                      committed_arranged_bounds().height};
    const BasicControlStyle style;
    painter.fill_rect(bounds, style.face);
    painter.stroke_rect({0.5, 0.5, std::max(0.0, bounds.width - 1.0),
                         std::max(0.0, bounds.height - 1.0)}, style.border, 1.0);
    const double middle = std::floor(bounds.height * 0.5);
    painter.draw_line({0.0, middle}, {bounds.width, middle}, style.border, 1.0);
    const double x = bounds.width * 0.5;
    painter.draw_line({x - 3.5, middle - 4.0}, {x, middle - 7.0},
                      style.dark_border, 1.0);
    painter.draw_line({x, middle - 7.0}, {x + 3.5, middle - 4.0},
                      style.dark_border, 1.0);
    painter.draw_line({x - 3.5, middle + 4.0}, {x, middle + 7.0},
                      style.dark_border, 1.0);
    painter.draw_line({x, middle + 7.0}, {x + 3.5, middle + 4.0},
                      style.dark_border, 1.0);
}

void SpinButtons::on_pointer(PointerEvent& event) {
    if (!eligible_for_input() || event.button != PointerButton::primary) return;
    if (event.action == PointerAction::down) {
        const Rect bounds = absolute_bounds();
        stepped_.emit(event.position.y < bounds.y + bounds.height * 0.5 ? 1 : -1);
        event.handled = true;
    } else if (event.action == PointerAction::up) {
        event.handled = true;
    }
}

} // namespace gui_forms
