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
    Insets padding = content_padding();
    if (edge_ == DropDownButtonEdge::bottom) padding.bottom = std::max(0.0, padding.bottom - drop_down_width_) + width;
    else padding.right = std::max(0.0, padding.right - drop_down_width_) + width;
    set_content_padding(padding);
    drop_down_width_ = width;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void DropDownButton::set_drop_down_edge(const DropDownButtonEdge edge) {
    require_mutable();
    if (edge != DropDownButtonEdge::right && edge != DropDownButtonEdge::bottom) {
        throw std::invalid_argument("drop-down button edge is unknown");
    }
    if (edge_ == edge) return;
    Insets padding = content_padding();
    if (edge_ == DropDownButtonEdge::right) {
        padding.right = std::max(0.0, padding.right - drop_down_width_);
        padding.bottom += drop_down_width_;
    } else {
        padding.bottom = std::max(0.0, padding.bottom - drop_down_width_);
        padding.right += drop_down_width_;
    }
    set_content_padding(padding);
    edge_ = edge;
    pointer_drop_down_ = false;
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

    const bool bottom = edge_ == DropDownButtonEdge::bottom;
    const double extent = std::min(drop_down_width_, bottom ? bounds.height : bounds.width);
    const double start = (bottom ? bounds.height : bounds.width) - extent;
    const Color foreground = effectively_enabled() ? style().text : style().disabled_text;
    if (mode_ == DropDownButtonMode::split && start > 0.0 &&
        (hovered_visual() || pressed_visual() || drop_down_open_)) {
        if (bottom) {
            painter.draw_line({3.0, start}, {std::max(3.0, bounds.width - 3.0), start}, style().border, 1.0);
        } else {
            painter.draw_line({start, 3.0}, {start, std::max(3.0, bounds.height - 3.0)}, style().border, 1.0);
        }
    }
    const double center_x = bottom ? bounds.width * 0.5 : start + extent * 0.5;
    const double center_y = (bottom ? start + extent * 0.5 : bounds.height * 0.5) +
        (pressed_visual() || drop_down_open_ ? 1.0 : 0.0);
    // A filled, pixel-aligned 7 x 4 triangle remains legible in the GDI host
    // and does not depend on diagonal line endpoint or pen rounding rules.
    const double left = std::floor(center_x) - 3.0;
    const double top = std::floor(center_y) - 1.0;
    for (int row = 0; row < 4; ++row) {
        painter.fill_rect({left + row, top + row, 7.0 - row * 2.0, 1.0}, foreground);
    }
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
    if (edge_ == DropDownButtonEdge::bottom) {
        return local.y >= std::max(0.0, bounds.height - drop_down_width_) &&
               local.y <= bounds.height && local.x >= 0.0 && local.x <= bounds.width;
    }
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
