#include "gui_forms/controls/button_base/button/button.hpp"
#include "../../basic/basic_control_rendering.hpp"
#include "gui_forms/text.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

namespace gui_forms {

Button::Button(StableId stable_id, std::string text)
    : ButtonBase(std::move(stable_id), std::move(text)) {}

void Button::set_default_button(bool is_default) {
    require_mutable();
    if (default_button_ == is_default) {
        return;
    }
    default_button_ = is_default;
    invalidate(Dirty::paint | Dirty::semantics);
}

void Button::notify_default(bool value) { set_default_button(value); }

void Button::set_dialog_result(DialogResult result) {
    require_mutable();
    switch (result) {
    case DialogResult::none:
    case DialogResult::ok:
    case DialogResult::cancel:
    case DialogResult::abort:
    case DialogResult::retry:
    case DialogResult::ignore:
    case DialogResult::yes:
    case DialogResult::no:
    case DialogResult::try_again:
    case DialogResult::continue_: break;
    default:
        throw std::invalid_argument("Button DialogResult is not a defined value");
    }
    if (dialog_result_ == result) return;
    dialog_result_ = result;
    invalidate(Dirty::semantics);
    publish_change(dialog_result_changed_, dialog_result_);
}

void Button::assign_cancel_dialog_result() {
    if (dialog_result_ == DialogResult::none) {
        set_dialog_result(DialogResult::cancel);
    }
}

void Button::on_activate() {
    Window* owner = attached_window();
    ButtonBase::on_activate();
    // Read the value after Click: a handler may deliberately replace or clear
    // DialogResult before the Form/Window observes it.
    if (dialog_result_ != DialogResult::none && is_alive() &&
        attached_window() == owner && owner != nullptr) {
        owner->set_dialog_result(dialog_result_);
    }
}

void Button::set_visual_style(ButtonVisualStyle style) {
    require_mutable();
    if (visual_style_ == style) {
        return;
    }
    visual_style_ = style;
    invalidate(Dirty::paint | Dirty::semantics);
}

void Button::set_flat_border_width(double width) {
    require_mutable();
    if (!std::isfinite(width) || width < 0.0) {
        throw std::invalid_argument("button flat border width must be finite and nonnegative");
    }
    if (flat_border_width_ == width) return;
    flat_border_width_ = width;
    invalidate(Dirty::paint | Dirty::semantics);
}

void Button::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    const std::string display = display_text();
    if (!has_style_override()) {
        ControlVisualRole role = ControlVisualRole::button;
        if (visual_style_ == ButtonVisualStyle::accent) {
            role = ControlVisualRole::accent_button;
        } else if (visual_style_ == ButtonVisualStyle::command) {
            role = ControlVisualRole::command_button;
        }
        paint_themed_button(painter, bounds, role, default_button_,
                            visual_style_ == ButtonVisualStyle::command);
        return;
    }
    switch (visual_style_) {
    case ButtonVisualStyle::standard:
        paint_button_frame(painter, bounds, default_button_);
        paint_button_text(painter, bounds, display);
        break;
    case ButtonVisualStyle::flat:
        painter.fill_rect(bounds, pressed_visual() ? style().accent_light
                                                   : style().face_light);
        if (flat_border_width_ > 0.0) {
            const double inset = flat_border_width_ * 0.5;
            painter.stroke_rect(
                {inset, inset,
                 std::max(0.0, bounds.width - flat_border_width_),
                 std::max(0.0, bounds.height - flat_border_width_)},
                default_button_ ? style().accent : style().border,
                flat_border_width_);
        }
        paint_button_text(painter, bounds, display);
        break;
    case ButtonVisualStyle::accent: {
        const Color fill = pressed_visual() ? style().link : style().accent;
        painter.fill_rect(bounds, fill);
        painter.draw_line({0.0, 0.0}, {bounds.width - 1.0, 0.0},
                          style().accent_light, 1.0);
        painter.stroke_rect({0.5, 0.5, std::max(0.0, bounds.width - 1.0),
                             std::max(0.0, bounds.height - 1.0)},
                            style().dark_border, 1.0);
        const double offset = pressed_visual() ? 1.0 : 0.0;
        paint_button_content(painter, bounds, display,
                             effectively_enabled() ? style().paper : style().disabled_text,
                             {offset, offset});
        break;
    }
    case ButtonVisualStyle::command:
        painter.fill_rect(bounds, pressed_visual() ? style().accent_light
                                                   : style().face);
        painter.fill_rect({0.0, 0.0, 4.0, bounds.height}, style().accent);
        painter.stroke_rect({0.5, 0.5, std::max(0.0, bounds.width - 1.0),
                             std::max(0.0, bounds.height - 1.0)},
                            style().border, 1.0);
        paint_button_content(painter, bounds, display,
                             effectively_enabled() ? style().text : style().disabled_text,
                             {}, false, true);
        break;
    }
}

Insets Button::visual_outsets() const noexcept {
    if (has_style_override()) return {};
    ControlVisualRole role = ControlVisualRole::button;
    if (visual_style_ == ButtonVisualStyle::accent) {
        role = ControlVisualRole::accent_button;
    } else if (visual_style_ == ButtonVisualStyle::command) {
        role = ControlVisualRole::command_button;
    }
    const ControlVisualContext context = visual_context(
        hovered_visual(), pressed_visual(), false, focused_visual(),
        default_button_);
    return surface_material_visual_outsets(
        effective_theme().resolve(role, context).material);
}

} // namespace gui_forms
