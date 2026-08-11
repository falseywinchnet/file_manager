#include "gui_forms/controls/button_base/radio_button/radio_button.hpp"
#include "gui_forms/detail/property_binding_adapters.hpp"
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

RadioButton::RadioButton(StableId stable_id, std::string text)
    : ButtonBase(std::move(stable_id), std::move(text)) {
    set_text_alignment(ContentAlignment::middle_left);
    set_image_alignment(ContentAlignment::middle_left);
    define_bindable_property({
        {"Checked", BindingValueKind::boolean, "Behavior",
         "Whether the radio button is selected within its group.",
         BindingValue{false}, Dirty::paint | Dirty::semantics},
        detail::BindingMemberGetter<RadioButton, bool>(
            *this, &RadioButton::checked_),
        detail::ConvertedPropertySetter<RadioButton, bool>(
            *this, &RadioButton::set_checked, BindingValueKind::boolean,
            "RadioButton.Checked binding requires Boolean"),
        detail::EventChangeConnector<bool>(checked_changed_), {}, {}});
}

void RadioButton::set_checked_without_exclusion(bool checked_value) {
    require_mutable();
    if (checked_ == checked_value) {
        return;
    }
    checked_ = checked_value;
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(checked_changed_, checked_);
}

void RadioButton::set_checked(bool checked_value) {
    require_mutable();
    if (checked_ == checked_value) {
        return;
    }
    if (checked_value) {
        if (const Control::Ptr owner = parent()) {
            for (const Control::Ptr& sibling : (*owner).children()) {
                std::shared_ptr<gui_forms::RadioButton> peer = std::dynamic_pointer_cast<RadioButton>(sibling);
                if (peer && peer.get() != this && (*peer).group_name_ == group_name_ &&
                    (*peer).checked_) {
                    (*peer).set_checked_without_exclusion(false);
                    if (!is_alive()) {
                        return;
                    }
                }
            }
        }
    }
    set_checked_without_exclusion(checked_value);
}

void RadioButton::set_group_name(std::string name) {
    require_mutable();
    if (group_name_ == name) {
        return;
    }
    group_name_ = std::move(name);
    invalidate(Dirty::semantics);
    if (checked_) {
        if (const Control::Ptr owner = parent()) {
            for (const Control::Ptr& sibling : (*owner).children()) {
                std::shared_ptr<gui_forms::RadioButton> peer = std::dynamic_pointer_cast<RadioButton>(sibling);
                if (peer && peer.get() != this && (*peer).group_name_ == group_name_ &&
                    (*peer).checked_) {
                    (*peer).set_checked_without_exclusion(false);
                    if (!is_alive()) {
                        return;
                    }
                }
            }
        }
    }
}

void RadioButton::set_auto_check(bool enabled_value) {
    require_mutable();
    if (auto_check_ == enabled_value) {
        return;
    }
    auto_check_ = enabled_value;
    invalidate(Dirty::semantics);
}

void RadioButton::set_indicator_style(ChoiceIndicatorStyle style_value) {
    require_mutable();
    if (indicator_style_ == style_value) {
        return;
    }
    indicator_style_ = style_value;
    invalidate(Dirty::paint | Dirty::semantics);
}

void RadioButton::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    if (!has_style_override()) {
        const double indicator_width =
            indicator_style_ == ChoiceIndicatorStyle::toggle ? 30.0 : 15.0;
        const double top = std::max(1.0, (bounds.height - 15.0) * 0.5);
        const Rect indicator_bounds{1.0, top, indicator_width, 15.0};
        const ControlVisualContext context = visual_context(
            hovered_visual(), pressed_visual(), checked_, focus_cue_visible());
        const ControlVisualRecipe& recipe = effective_theme().resolve(
            ControlVisualRole::choice, context);
        SurfaceMaterial indicator = recipe.material;
        indicator.corner_radius = 7.5;
        paint_surface_material(painter, indicator_bounds, indicator);
        if (indicator_style_ == ChoiceIndicatorStyle::toggle) {
            const double knob_x = checked_ ? indicator_bounds.x +
                    indicator_bounds.width - 13.0 : indicator_bounds.x + 2.0;
            painter.fill_rounded_rect(
                {knob_x, indicator_bounds.y + 2.0, 11.0, 11.0}, 5.5,
                recipe.text);
        } else if (checked_) {
            painter.fill_rounded_rect(
                {indicator_bounds.x + 4.0, indicator_bounds.y + 4.0,
                 7.0, 7.0}, 3.5, recipe.glyph);
        }
        paint_button_content(
            painter,
            {indicator_bounds.x + indicator_bounds.width + 1.0, 0.0,
             std::max(0.0, bounds.width - indicator_bounds.x -
                                   indicator_bounds.width - 1.0),
             bounds.height},
            display_text(), recipe.text, {}, checked_);
        paint_theme_cues(painter, bounds, recipe, context);
        return;
    }
    const BasicControlStyle& colors = style();
    const double indicator_width =
        indicator_style_ == ChoiceIndicatorStyle::toggle ? 30.0 : 15.0;
    const double top = std::max(1.0, (bounds.height - 15.0) * 0.5);
    if (indicator_style_ == ChoiceIndicatorStyle::toggle) {
        const Rect track{1.0, top, indicator_width, 15.0};
        painter.fill_rect(track, checked_ ? colors.accent : colors.border);
        const double knob_x = checked_ ? track.x + track.width - 13.0
                                       : track.x + 2.0;
        painter.fill_rect({knob_x, track.y + 2.0, 11.0, 11.0}, colors.paper);
        paint_button_content(
            painter,
            {track.x + track.width + 1.0, 0.0,
             std::max(0.0, bounds.width - track.x - track.width - 1.0),
             bounds.height},
            display_text(), effectively_enabled() ? colors.text : colors.disabled_text, {}, checked_);
        return;
    }
    const Color ring = indicator_style_ == ChoiceIndicatorStyle::modern && checked_
        ? colors.accent : colors.border;
    fill_radio_disc(painter, 1.0, top, ring, false);
    fill_radio_face(painter, 1.0, top, colors.paper);
    if (checked_) {
        fill_radio_disc(painter, 1.0, top,
                        indicator_style_ == ChoiceIndicatorStyle::modern
                            ? colors.accent : colors.dark_border,
                        true);
    }
    paint_button_content(
        painter, {17.0, 0.0, std::max(0.0, bounds.width - 17.0), bounds.height},
        display_text(), effectively_enabled() ? colors.text : colors.disabled_text, {}, checked_);
}

Insets RadioButton::visual_outsets() const noexcept {
    if (has_style_override()) return {};
    const ControlVisualContext context = visual_context(
        hovered_visual(), pressed_visual(), checked_, focus_cue_visible());
    return surface_material_visual_outsets(
        effective_theme().resolve(ControlVisualRole::choice, context).material);
}

void RadioButton::on_activate() {
    if (auto_check_ && !checked_) {
        set_checked(true);
        if (!is_alive()) {
            return;
        }
    }
    ButtonBase::on_activate();
}

SemanticDescriptor RadioButton::semantic_descriptor() const {
    SemanticDescriptor descriptor = ButtonBase::semantic_descriptor();
    descriptor.role = SemanticRole::radio_button;
    descriptor.value = checked_ ? "selected" : "not selected";
    if (checked_) {
        descriptor.states |= SemanticState::checked;
        descriptor.states |= SemanticState::selected;
    }
    return descriptor;
}

} // namespace gui_forms
