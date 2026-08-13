#include "gui_forms/controls/button_base/button/drop_down_button/drop_down_button.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace gui_forms {

DropDownButton::DropDownButton(StableId stable_id, std::string text,
                               const DropDownButtonMode mode)
    : Button(std::move(stable_id), std::move(text)), mode_(mode) {
    if (mode_ != DropDownButtonMode::menu &&
        mode_ != DropDownButtonMode::split) {
        throw std::invalid_argument("drop-down button mode is unknown");
    }
    set_content_padding({6.0, 4.0, 22.0, 4.0});
}

void DropDownButton::set_drop_down_mode(const DropDownButtonMode mode) {
    require_mutable();
    if (mode != DropDownButtonMode::menu && mode != DropDownButtonMode::split) {
        throw std::invalid_argument("drop-down button mode is unknown");
    }
    if (mode_ == mode) return;
    mode_ = mode;
    pointer_drop_down_ = false;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void DropDownButton::set_drop_down_width(const double width) {
    require_mutable();
    if (!std::isfinite(width) || width < 12.0 || width > 64.0) {
        throw std::invalid_argument(
            "drop-down button width must be finite and between 12 and 64");
    }
    if (drop_down_width_ == width) return;
    drop_down_width_ = width;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void DropDownButton::set_drop_down_open(const bool open) {
    require_mutable();
    if (drop_down_open_ == open) return;
    drop_down_open_ = open;
    invalidate(Dirty::paint | Dirty::semantics);
}

bool DropDownButton::perform_drop_down() {
    if (!prepare_command_activation()) return false;
    drop_down_requested_.emit(*this);
    return true;
}

void DropDownButton::on_paint(Painter& painter, const Rect local_damage) {
    Button::on_paint(painter, local_damage);
    const Rect bounds = local_bounds();
    if (bounds.width <= 0.0 || bounds.height <= 0.0) return;

    const double disclosure_width = std::min(drop_down_width_, bounds.width);
    const double disclosure_left = bounds.width - disclosure_width;
    const Color foreground = effectively_enabled() ? style().text
                                                   : style().disabled_text;
    if (mode_ == DropDownButtonMode::split && disclosure_left > 0.0) {
        painter.draw_line({disclosure_left, 3.0},
                          {disclosure_left, std::max(3.0, bounds.height - 3.0)},
                          style().border, 1.0);
    }
    const double center_x = disclosure_left + disclosure_width * 0.5;
    const double center_y = bounds.height * 0.5 +
        (pressed_visual() || drop_down_open_ ? 1.0 : 0.0);
    painter.draw_line({center_x - 3.0, center_y - 1.0},
                      {center_x, center_y + 2.0}, foreground, 1.0);
    painter.draw_line({center_x, center_y + 2.0},
                      {center_x + 3.0, center_y - 1.0}, foreground, 1.0);
    if (drop_down_open_) {
        painter.draw_line({1.0, std::max(0.0, bounds.height - 2.0)},
                          {std::max(1.0, bounds.width - 1.0),
                           std::max(0.0, bounds.height - 2.0)},
                          style().dark_border, 2.0);
    }
}

bool DropDownButton::point_in_drop_down(const Point window_point) const noexcept {
    if (mode_ == DropDownButtonMode::menu) return true;
    const Point local = point_from_window(window_point);
    const Rect bounds = local_bounds();
    return local.x >= std::max(0.0, bounds.width - drop_down_width_) &&
           local.x <= bounds.width && local.y >= 0.0 && local.y <= bounds.height;
}

void DropDownButton::on_pointer(PointerEvent& event) {
    if (event.button == PointerButton::primary &&
        event.action == PointerAction::down) {
        pointer_drop_down_ = point_in_drop_down(event.position);
    } else if (event.action == PointerAction::leave && !pressed_visual()) {
        pointer_drop_down_ = false;
    }
    ButtonBase::on_pointer(event);
}

void DropDownButton::on_key(KeyEvent& event) {
    if (event.action == KeyAction::down && !event.repeat &&
        event.physical_key == PhysicalKey::down &&
        has_modifier(event.modifiers, Modifier::alt)) {
        event.handled = perform_drop_down();
        return;
    }
    pointer_drop_down_ = false;
    ButtonBase::on_key(event);
}

SemanticDescriptor DropDownButton::semantic_descriptor() const {
    SemanticDescriptor descriptor = Button::semantic_descriptor();
    descriptor.actions.push_back(SemanticAction::show_menu);
    descriptor.actions.push_back(drop_down_open_ ? SemanticAction::collapse
                                                 : SemanticAction::expand);
    if (drop_down_open_) descriptor.states |= SemanticState::expanded;
    return descriptor;
}

bool DropDownButton::on_semantic_action(const SemanticAction action,
                                        const std::string_view value) {
    if (action == SemanticAction::show_menu ||
        (action == SemanticAction::expand && !drop_down_open_)) {
        return perform_drop_down();
    }
    if (action == SemanticAction::collapse && drop_down_open_) {
        drop_down_close_requested_.emit(*this);
        return true;
    }
    return Button::on_semantic_action(action, value);
}

void DropDownButton::on_activate() {
    if (mode_ == DropDownButtonMode::menu || pointer_drop_down_) {
        pointer_drop_down_ = false;
        static_cast<void>(perform_drop_down());
        return;
    }
    pointer_drop_down_ = false;
    ButtonBase::on_activate();
}

} // namespace gui_forms
