#include "gui_forms/controls/panel/numeric_up_down/numeric_up_down.hpp"

#include "spin_buttons.hpp"
#include "gui_forms/detail/property_binding_adapters.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace gui_forms {

NumericUpDown::NumericUpDown(StableId stable_id) : Panel(std::move(stable_id)) {
    set_border_style(BorderStyle::line);
    set_background(style().paper);
    define_bindable_property({
        {"Value", BindingValueKind::number, "Behavior",
         "Current numeric value.", BindingValue{0.0},
         Dirty::paint | Dirty::semantics},
        detail::BindingMemberGetter<NumericUpDown, double>(
            *this, &NumericUpDown::value_),
        detail::ConvertedPropertySetter<NumericUpDown, double>(
            *this, &NumericUpDown::set_value, BindingValueKind::number,
            "NumericUpDown.Value binding requires a number"),
        detail::EventChangeConnector<double>(value_changed_), {}, {}});
}

void NumericUpDown::EditorChangeCallback::operator()(
    const std::string&) const {
    const std::shared_ptr<NumericUpDown> numeric = target.lock();
    if (numeric && !(*numeric).synchronizing_) {
        (*numeric).commit_editor_text();
    }
}

void NumericUpDown::SpinnerStepCallback::operator()(int direction) const {
    const std::shared_ptr<NumericUpDown> numeric = target.lock();
    if (numeric) (*numeric).step(direction);
}

void NumericUpDown::initialize_control_tree() {
    const std::string prefix(stable_id().value());
    editor_ = make_control<TextBox>(StableId(prefix + ".editor"));
    (*editor_).set_border_style(BorderStyle::none);
    spinner_ = make_control<SpinButtons>(StableId(prefix + ".spinner"));
    add_child(editor_);
    add_child(spinner_);
    const std::weak_ptr<NumericUpDown> weak =
        std::static_pointer_cast<NumericUpDown>(shared_from_this());
    editor_change_ = (*editor_).text_changed().subscribe(
        *this, EditorChangeCallback{weak});
    std::shared_ptr<gui_forms::SpinButtons> spin = std::dynamic_pointer_cast<SpinButtons>(spinner_);
    spinner_step_ = (*spin).stepped().subscribe(
        *this, SpinnerStepCallback{weak});
    synchronize_editor();
}

void NumericUpDown::set_range(double minimum, double maximum) {
    require_mutable();
    if (!std::isfinite(minimum) || !std::isfinite(maximum) || minimum > maximum) {
        throw std::invalid_argument("NumericUpDown range must be finite and ordered");
    }
    if (minimum_ == minimum && maximum_ == maximum) return;
    minimum_ = minimum;
    maximum_ = maximum;
    const double clamped = std::clamp(value_, minimum_, maximum_);
    if (clamped != value_) {
        value_ = clamped;
        synchronize_editor();
        publish_change(value_changed_, value_);
    }
    invalidate(Dirty::paint | Dirty::semantics);
}

void NumericUpDown::set_value(double value) {
    require_mutable();
    if (!std::isfinite(value) || value < minimum_ || value > maximum_) {
        throw std::out_of_range("NumericUpDown value is outside its range");
    }
    if (value_ == value) return;
    value_ = value;
    synchronize_editor();
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(value_changed_, value_);
}

void NumericUpDown::set_increment(double increment) {
    require_mutable();
    if (!std::isfinite(increment) || increment <= 0.0) {
        throw std::invalid_argument("NumericUpDown increment must be finite and positive");
    }
    increment_ = increment;
}

void NumericUpDown::set_decimal_places(std::uint8_t places) {
    require_mutable();
    if (places > 12U) {
        throw std::invalid_argument("NumericUpDown decimal places may not exceed 12");
    }
    if (decimal_places_ == places) return;
    decimal_places_ = places;
    if (hexadecimal_ && places != 0U) hexadecimal_ = false;
    synchronize_editor();
    invalidate(Dirty::paint | Dirty::semantics);
}

void NumericUpDown::set_hexadecimal(bool hexadecimal) {
    require_mutable();
    if (hexadecimal_ == hexadecimal) return;
    hexadecimal_ = hexadecimal;
    if (hexadecimal_) decimal_places_ = 0U;
    synchronize_editor();
    invalidate(Dirty::paint | Dirty::semantics);
}

void NumericUpDown::set_button_width(double width) {
    require_mutable();
    if (!std::isfinite(width) || width < 12.0 || width > 64.0) {
        throw std::out_of_range(
            "NumericUpDown button width must be finite and between 12 and 64");
    }
    if (button_width_ == width) return;
    button_width_ = width;
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
}

std::string NumericUpDown::formatted_value() const {
    std::ostringstream stream;
    if (hexadecimal_) {
        stream << std::uppercase << std::hex << static_cast<std::int64_t>(std::llround(value_));
    } else {
        stream << std::fixed << std::setprecision(decimal_places_) << value_;
    }
    return stream.str();
}

void NumericUpDown::synchronize_editor() {
    if (!editor_) return;
    synchronizing_ = true;
    (*editor_).set_text(formatted_value());
    synchronizing_ = false;
}

void NumericUpDown::commit_editor_text() {
    if (!editor_ || (*editor_).text().empty()) return;
    try {
        std::size_t consumed{};
        double parsed{};
        if (hexadecimal_) {
            parsed = static_cast<double>(std::stoll(std::string((*editor_).text()),
                                                    &consumed, 16));
        } else {
            parsed = std::stod(std::string((*editor_).text()), &consumed);
        }
        if (consumed == (*editor_).text().size() && std::isfinite(parsed) &&
            parsed >= minimum_ && parsed <= maximum_ && parsed != value_) {
            value_ = parsed;
            invalidate(Dirty::paint | Dirty::semantics);
            publish_change(value_changed_, value_);
        }
    } catch (const std::exception&) {
    }
}

void NumericUpDown::step(int direction) {
    const double next = std::clamp(value_ + static_cast<double>(direction) * increment_,
                                   minimum_, maximum_);
    if (next != value_) set_value(next);
    if (editor_ && window() != nullptr) {
        static_cast<void>((*window()).request_focus(editor_));
        (*editor_).select_all();
    }
}

void NumericUpDown::arrange(Rect final_bounds) {
    arrange_self(final_bounds);
    const double button_width =
        std::min(button_width_, std::max(0.0, final_bounds.width));
    if (editor_) set_child_layout(
        editor_, {0.0, 0.0, std::max(0.0, final_bounds.width - button_width),
                  final_bounds.height});
    if (spinner_) set_child_layout(
        spinner_, {std::max(0.0, final_bounds.width - button_width), 0.0,
                   button_width, final_bounds.height});
}

void NumericUpDown::on_key_preview(KeyEvent& event) {
    if (!enabled() || event.action != KeyAction::down) return;
    if (event.physical_key == PhysicalKey::up || event.physical_key == PhysicalKey::down) {
        step(event.physical_key == PhysicalKey::up ? 1 : -1);
        event.handled = true;
    } else if (event.physical_key == PhysicalKey::enter) {
        commit_editor_text();
        synchronize_editor();
        event.handled = true;
    }
}

void NumericUpDown::on_pointer_preview(PointerEvent& event) {
    if (!enabled() || event.action != PointerAction::wheel || event.wheel_delta.y == 0.0) return;
    step(event.wheel_delta.y > 0.0 ? 1 : -1);
    event.handled = true;
}

SemanticDescriptor NumericUpDown::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::numeric_field;
    descriptor.name = accessible_name();
    descriptor.value = formatted_value();
    descriptor.numeric_value = value_;
    descriptor.minimum_value = minimum_;
    descriptor.maximum_value = maximum_;
    descriptor.description = accessible_description();
    descriptor.actions = {SemanticAction::focus, SemanticAction::increment,
                          SemanticAction::decrement, SemanticAction::set_value};
    descriptor.exposed = true;
    descriptor.include_descendants = false;
    return descriptor;
}

bool NumericUpDown::on_semantic_action(SemanticAction action,
                                       std::string_view value_text) {
    if (action == SemanticAction::increment || action == SemanticAction::decrement) {
        step(action == SemanticAction::increment ? 1 : -1);
        return true;
    }
    if (action == SemanticAction::set_value) {
        try {
            std::size_t consumed{};
            const double parsed = std::stod(std::string(value_text), &consumed);
            if (consumed != value_text.size()) return false;
            set_value(parsed);
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
    if (action == SemanticAction::focus && editor_ && window() != nullptr) {
        return (*window()).request_focus(editor_);
    }
    return Panel::on_semantic_action(action, value_text);
}

} // namespace gui_forms
