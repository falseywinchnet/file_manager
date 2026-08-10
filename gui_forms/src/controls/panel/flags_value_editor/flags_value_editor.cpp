#include "gui_forms/controls/panel/flags_value_editor/flags_value_editor.hpp"

#include "property_editor_drop_down_layer/property_editor_drop_down_layer.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace gui_forms {

using detail::PropertyEditorDropDownLayer;

FlagsValueEditor::FlagsValueEditor(
    StableId stable_id, PropertyEnumDescriptor descriptor,
    PropertyEnumValue value)
    : Panel(std::move(stable_id)) {
    set_paint_plane(PaintPlane::control);
    set_border_style(BorderStyle::sunken);
    set_focusable(true);
    set_cursor(CursorKind::hand);
    set_descriptor(std::move(descriptor));
    set_value(std::move(value));
}

void FlagsValueEditor::set_descriptor(PropertyEnumDescriptor descriptor) {
    require_mutable();
    if (!descriptor.flags || !valid_property_enum_descriptor(descriptor)) {
        throw std::invalid_argument(
            "FlagsValueEditor requires a valid finite flags descriptor");
    }
    const bool has_bit = std::any_of(
        descriptor.choices.begin(), descriptor.choices.end(),
        [](const PropertyEnumChoice& choice) {
            return choice.value > 0 &&
                std::has_single_bit(static_cast<std::uint64_t>(choice.value));
        });
    if (!has_bit) {
        throw std::invalid_argument(
            "FlagsValueEditor requires at least one positive single-bit choice");
    }
    if (descriptor_ == descriptor) return;
    std::optional<PropertyEnumValue> normalized_value;
    if (!value_.type_name.empty()) {
        PropertyDescriptor property;
        property.name = "Value";
        property.kind = BindingValueKind::enumeration;
        property.enumeration =
            std::make_shared<const PropertyEnumDescriptor>(descriptor);
        const auto normalized = convert_property_value(BindingValue{value_},
                                                        property);
        if (!normalized) {
            throw std::invalid_argument(
                "FlagsValueEditor descriptor cannot represent its current value");
        }
        normalized_value = std::get<PropertyEnumValue>(*normalized);
    }
    descriptor_ = std::move(descriptor);
    if (normalized_value) value_ = std::move(*normalized_value);
    if (dropped_down_) {
        close_drop_down();
        open_drop_down();
    }
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void FlagsValueEditor::set_value(PropertyEnumValue value) {
    require_mutable();
    PropertyDescriptor property;
    property.name = "Value";
    property.kind = BindingValueKind::enumeration;
    property.enumeration =
        std::make_shared<const PropertyEnumDescriptor>(descriptor_);
    const auto normalized = convert_property_value(BindingValue{std::move(value)},
                                                    property);
    if (!normalized) {
        throw std::invalid_argument(
            "FlagsValueEditor value does not belong to its descriptor");
    }
    const PropertyEnumValue next = std::get<PropertyEnumValue>(*normalized);
    if (value_ == next) return;
    value_ = next;
    synchronize_popup();
    invalidate(Dirty::paint | Dirty::semantics);
}

void FlagsValueEditor::set_dropped_down(bool dropped_down) {
    require_mutable();
    if (dropped_down == dropped_down_) return;
    if (dropped_down) open_drop_down();
    else close_drop_down();
}

void FlagsValueEditor::set_popup_width(double width) {
    require_mutable();
    if (!std::isfinite(width) || width < 80.0 || width > 1024.0) {
        throw std::invalid_argument(
            "flags editor popup width must be within [80, 1024]");
    }
    if (popup_width_ == width) return;
    popup_width_ = width;
    if (dropped_down_) {
        close_drop_down();
        open_drop_down();
    }
    invalidate(Dirty::paint | Dirty::semantics);
}

void FlagsValueEditor::open_drop_down() {
    if (dropped_down_ || !attached() || window() == nullptr) return;
    const Control::Ptr root = window()->root();
    if (!root) return;
    const Rect editor = absolute_bounds();
    const Size client = window()->client_size();
    popup_choice_indices_.clear();
    std::vector<std::string> items;
    for (std::size_t index = 0U; index < descriptor_.choices.size(); ++index) {
        const std::int64_t choice = descriptor_.choices[index].value;
        if (choice == 0 || (choice > 0 &&
            std::has_single_bit(static_cast<std::uint64_t>(choice)))) {
            popup_choice_indices_.push_back(index);
            items.push_back(descriptor_.choices[index].name);
        }
    }
    if (items.empty()) return;
    const std::size_t rows = std::min<std::size_t>(8U, items.size());
    const double height = static_cast<double>(rows) *
        26.0 * effective_text_scale() + 4.0;
    const double y = editor.y + editor.height + height <= client.height
        ? editor.y + editor.height : std::max(0.0, editor.y - height);
    const std::string prefix(stable_id().value());
    auto layer = make_control<PropertyEditorDropDownLayer>(
        StableId(prefix + ".popup.layer"));
    layer->set_requested_bounds({0.0, 0.0, client.width, client.height});
    auto list = make_control<CheckedListBox>(
        StableId(prefix + ".popup.list"));
    list->set_paint_plane(PaintPlane::overlay);
    list->set_items(std::move(items));
    list->set_check_on_click(true);
    const double popup_width = std::min(popup_width_, client.width);
    const double popup_x = std::clamp(
        editor.x, 0.0, std::max(0.0, client.width - popup_width));
    list->set_requested_bounds({popup_x, y, popup_width, height});
    layer->add_child(list);
    PopupToken token = window()->open_popup(shared_from_this(), layer);

    popup_layer_ = layer;
    popup_list_ = list;
    popup_token_ = std::move(token);
    const std::weak_ptr<FlagsValueEditor> weak =
        std::static_pointer_cast<FlagsValueEditor>(shared_from_this());
    popup_check_ = list->item_check_state_changed().subscribe(
        *this, [weak](std::size_t index, CheckState state) {
            if (const auto retained = weak.lock()) {
                retained->apply_popup_choice(index, state);
            }
        });
    popup_dismissal_ = layer->dismissed().subscribe(*this, [weak] {
        if (const auto retained = weak.lock()) retained->close_drop_down();
    });
    if (Event<>* closed = popup_token_.closed_event()) {
        popup_revocation_ = closed->subscribe(*this, [weak] {
            if (const auto retained = weak.lock()) retained->on_popup_revoked();
        });
    }
    popup_scope_ = window()->begin_focus_scope(layer, list).value;
    dropped_down_ = true;
    synchronize_popup();
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(drop_down_changed_, true);
}

void FlagsValueEditor::close_drop_down() {
    if (!dropped_down_ && !popup_layer_) return;
    closing_popup_ = true;
    popup_check_.disconnect();
    popup_dismissal_.disconnect();
    if (window() != nullptr && popup_scope_ != 0U) {
        static_cast<void>(window()->end_focus_scope(FocusScopeId{popup_scope_}));
    }
    popup_scope_ = 0U;
    popup_token_.disconnect();
    popup_revocation_.disconnect();
    popup_list_.reset();
    popup_layer_.reset();
    popup_choice_indices_.clear();
    const bool changed = dropped_down_;
    dropped_down_ = false;
    closing_popup_ = false;
    invalidate(Dirty::paint | Dirty::semantics);
    if (changed) publish_change(drop_down_changed_, false);
}

void FlagsValueEditor::on_popup_revoked() {
    if (closing_popup_) return;
    popup_check_.disconnect();
    popup_dismissal_.disconnect();
    popup_revocation_.disconnect();
    if (window() != nullptr && popup_scope_ != 0U) {
        static_cast<void>(window()->end_focus_scope(
            FocusScopeId{popup_scope_},
            FocusScopeCloseReason::owner_unavailable));
    }
    popup_scope_ = 0U;
    popup_list_.reset();
    popup_layer_.reset();
    popup_choice_indices_.clear();
    const bool changed = dropped_down_;
    dropped_down_ = false;
    invalidate(Dirty::paint | Dirty::semantics);
    if (changed) publish_change(drop_down_changed_, false);
}

void FlagsValueEditor::synchronize_popup() {
    if (!popup_list_) return;
    synchronizing_popup_ = true;
    const auto bits = static_cast<std::uint64_t>(value_.value);
    for (std::size_t popup_index = 0U;
         popup_index < popup_choice_indices_.size(); ++popup_index) {
        const std::int64_t choice =
            descriptor_.choices[popup_choice_indices_[popup_index]].value;
        const bool checked = choice == 0
            ? value_.value == 0
            : (bits & static_cast<std::uint64_t>(choice)) ==
                  static_cast<std::uint64_t>(choice);
        popup_list_->set_item_checked(popup_index, checked);
    }
    synchronizing_popup_ = false;
}

void FlagsValueEditor::apply_popup_choice(std::size_t popup_index,
                                           CheckState state) {
    if (synchronizing_popup_ || state == CheckState::indeterminate ||
        popup_index >= popup_choice_indices_.size()) return;
    const PropertyEnumChoice& choice =
        descriptor_.choices[popup_choice_indices_[popup_index]];
    std::uint64_t bits = static_cast<std::uint64_t>(value_.value);
    if (choice.value == 0) {
        if (state == CheckState::checked) bits = 0U;
    } else if (state == CheckState::checked) {
        bits |= static_cast<std::uint64_t>(choice.value);
    } else {
        bits &= ~static_cast<std::uint64_t>(choice.value);
    }
    PropertyDescriptor property;
    property.name = "Value";
    property.kind = BindingValueKind::enumeration;
    property.enumeration =
        std::make_shared<const PropertyEnumDescriptor>(descriptor_);
    const auto normalized = convert_property_value(
        BindingValue{static_cast<std::int64_t>(bits)}, property);
    if (!normalized) {
        synchronize_popup();
        return;
    }
    const PropertyEnumValue next = std::get<PropertyEnumValue>(*normalized);
    if (next == value_) {
        synchronize_popup();
        return;
    }
    value_ = next;
    synchronize_popup();
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(value_changed_, value_);
}

void FlagsValueEditor::on_paint(Painter& painter, Rect damage) {
    Panel::on_paint(painter, damage);
    const Rect bounds = local_bounds();
    const BasicControlStyle& colors = style();
    const double button_width = std::min(24.0, bounds.width);
    const double divider = std::max(0.0, bounds.width - button_width);
    painter.fill_rect({divider, 1.0, std::max(0.0, button_width - 1.0),
                       std::max(0.0, bounds.height - 2.0)}, colors.face);
    painter.draw_line({divider, 1.0}, {divider, bounds.height - 1.0},
                      colors.border, 1.0);
    const double center_x = bounds.width - button_width * 0.5;
    const double center_y = bounds.height * 0.5;
    const double direction = dropped_down_ ? -1.0 : 1.0;
    painter.draw_line({center_x - 4.0, center_y - direction * 2.0},
                      {center_x, center_y + direction * 2.0},
                      colors.dark_border, 1.0);
    painter.draw_line({center_x, center_y + direction * 2.0},
                      {center_x + 4.0, center_y - direction * 2.0},
                      colors.dark_border, 1.0);
    const FontSpec font = effective_font({FontRole::content, 10.0, 400, false});
    painter.save();
    painter.clip_rect({4.0, 2.0, std::max(0.0, divider - 8.0),
                       std::max(0.0, bounds.height - 4.0)});
    painter.draw_text_utf8({7.0, std::max(font.size,
                            (bounds.height + font.size) * 0.5 - 1.0)},
                           value_.name, font,
                           enabled() ? colors.text : colors.disabled_text);
    painter.restore();
    if (focused_ || dropped_down_) {
        painter.stroke_rect({1.5, 1.5, std::max(0.0, bounds.width - 3.0),
                             std::max(0.0, bounds.height - 3.0)},
                            colors.accent, 1.0);
    }
}

void FlagsValueEditor::on_pointer(PointerEvent& event) {
    if (!eligible_for_input()) return;
    if (event.action == PointerAction::down &&
        event.button == PointerButton::primary) {
        set_dropped_down(!dropped_down_);
        event.handled = true;
    } else if (event.action == PointerAction::up &&
               event.button == PointerButton::primary) {
        event.handled = true;
    }
}

void FlagsValueEditor::on_key(KeyEvent& event) {
    if (!focused_ || !enabled() || event.action != KeyAction::down) return;
    if (event.physical_key == PhysicalKey::escape && dropped_down_) {
        close_drop_down();
        event.handled = true;
    } else if (event.physical_key == PhysicalKey::f4 ||
               event.physical_key == PhysicalKey::space ||
               (event.physical_key == PhysicalKey::down &&
                has_modifier(event.modifiers, Modifier::alt))) {
        set_dropped_down(!dropped_down_);
        event.handled = true;
    }
}

void FlagsValueEditor::on_focus_changed(bool focused) {
    focused_ = focused;
    invalidate(Dirty::paint | Dirty::semantics);
}

SemanticDescriptor FlagsValueEditor::semantic_descriptor() const {
    SemanticDescriptor result;
    result.role = SemanticRole::combo_box;
    result.name = accessible_name();
    result.value = value_.name;
    result.description = accessible_description();
    if (dropped_down_) result.states |= SemanticState::expanded;
    result.actions = {SemanticAction::focus,
        dropped_down_ ? SemanticAction::collapse : SemanticAction::expand};
    result.exposed = true;
    return result;
}

bool FlagsValueEditor::on_semantic_action(SemanticAction action,
                                           std::string_view value) {
    if (action == SemanticAction::expand) {
        set_dropped_down(true);
        return true;
    }
    if (action == SemanticAction::collapse) {
        set_dropped_down(false);
        return true;
    }
    return Panel::on_semantic_action(action, value);
}

void FlagsValueEditor::on_detached_from_window() noexcept {
    popup_check_.disconnect();
    popup_dismissal_.disconnect();
    popup_revocation_.disconnect();
    popup_scope_ = 0U;
    popup_token_.disconnect();
    popup_list_.reset();
    popup_layer_.reset();
    popup_choice_indices_.clear();
    dropped_down_ = false;
    focused_ = false;
}

} // namespace gui_forms
