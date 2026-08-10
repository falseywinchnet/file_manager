#include "gui_forms/controls/panel/combo_box/combo_box.hpp"

#include "drop_down_layer.hpp"
#include "../input_control_utilities.hpp"
#include "gui_forms/detail/bound_member_function.hpp"
#include "gui_forms/detail/property_binding_adapters.hpp"
#include "gui_forms/detail/weak_member_callback.hpp"
#include "gui_forms/text.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace gui_forms {
using namespace input_control_detail;

ComboBox::ComboBox(StableId stable_id) : Panel(std::move(stable_id)) {
    set_paint_plane(PaintPlane::control);
    set_border_style(BorderStyle::sunken);
    set_focusable(true);
    set_cursor(CursorKind::hand);
    PropertyDescriptor items_descriptor;
    items_descriptor.name = "Items";
    items_descriptor.kind = BindingValueKind::collection;
    items_descriptor.category = "Data";
    items_descriptor.description =
        "Ordered text items presented by the drop-down.";
    items_descriptor.default_value = BindingValue{make_property_collection(
        "String", BindingValueKind::text, {})};
    items_descriptor.invalidation_effects =
        Dirty::measure | Dirty::paint | Dirty::semantics;
    items_descriptor.serialization_visibility =
        PropertySerializationVisibility::content;
    items_descriptor.bindable = false;
    gui_forms::PropertyRegistration items_registration;
    items_registration.descriptor = std::move(items_descriptor);
    items_registration.get = detail::BoundMemberFunction<
        BindingValue (ComboBox::*)() const>(
            *this, &ComboBox::items_property_value);
    items_registration.set = detail::BoundMemberFunction<
        void (ComboBox::*)(const BindingValue&)>(
            *this, &ComboBox::set_items_property);
    items_registration.connect_changed =
        detail::EventChangeConnector<>(items_changed_);
    items_registration.reset = detail::BoundMemberFunction<
        void (ComboBox::*)()>(*this, &ComboBox::reset_items_property);
    items_registration.should_serialize = detail::BoundMemberFunction<
        bool (ComboBox::*)() const noexcept>(
            *this, &ComboBox::should_serialize_items_property);
    define_bindable_property(std::move(items_registration));

    gui_forms::PropertyRegistration index_registration;
    index_registration.descriptor = {
        "SelectedIndex", BindingValueKind::signed_integer, "Behavior",
        "Zero-based selected item index, or -1 when no item is selected.",
        BindingValue{std::int64_t{-1}}, Dirty::paint | Dirty::semantics};
    index_registration.get = detail::BoundMemberFunction<
        BindingValue (ComboBox::*)() const>(
            *this, &ComboBox::selected_index_property_value);
    index_registration.set = detail::BoundMemberFunction<
        void (ComboBox::*)(const BindingValue&)>(
            *this, &ComboBox::set_selected_index_property);
    index_registration.connect_changed =
        detail::EventChangeConnector<std::optional<std::size_t>>(
            selected_index_changed_);
    define_bindable_property(std::move(index_registration));

    gui_forms::PropertyRegistration text_registration;
    text_registration.descriptor = {
        "Text", BindingValueKind::text, "Appearance",
        "Text of the selected item, or empty when no item is selected.",
        BindingValue{std::string{}}, Dirty::paint | Dirty::semantics};
    text_registration.get = detail::BoundMemberFunction<
        BindingValue (ComboBox::*)() const>(
            *this, &ComboBox::text_property_value);
    text_registration.set = detail::BoundMemberFunction<
        void (ComboBox::*)(const BindingValue&)>(
            *this, &ComboBox::set_text_property);
    text_registration.connect_changed =
        detail::EventChangeConnector<std::optional<std::size_t>>(
            selected_index_changed_);
    text_registration.reset = detail::BoundMemberFunction<
        void (ComboBox::*)()>(*this, &ComboBox::reset_text_property);
    text_registration.should_serialize = detail::BoundMemberFunction<
        bool (ComboBox::*)() const noexcept>(
            *this, &ComboBox::should_serialize_text_property);
    define_bindable_property(std::move(text_registration));
}

BindingValue ComboBox::items_property_value() const {
    std::vector<BindingValue> values;
    values.reserve(items_.size());
    for (const std::string& item : items_) values.emplace_back(item);
    return BindingValue{make_property_collection(
        "String", BindingValueKind::text, std::move(values))};
}

void ComboBox::set_items_property(const BindingValue& value) {
    const PropertyCollectionValue* collection =
        std::get_if<PropertyCollectionValue>(&value);
    if (collection == nullptr || !*collection ||
        (*collection).item_kind() != BindingValueKind::text) {
        throw std::invalid_argument(
            "ComboBox.Items requires a homogeneous text collection");
    }
    std::vector<std::string> items;
    const std::span<const BindingValue> values =
        property_collection_items(*collection);
    items.reserve(values.size());
    for (const BindingValue& item : values) {
        items.push_back(std::get<std::string>(item));
    }
    set_items(std::move(items));
}

void ComboBox::reset_items_property() {
    set_items({});
}

bool ComboBox::should_serialize_items_property() const noexcept {
    return !items_.empty();
}

BindingValue ComboBox::selected_index_property_value() const {
    return BindingValue{selected_index_
        ? static_cast<std::int64_t>(*selected_index_)
        : std::int64_t{-1}};
}

void ComboBox::set_selected_index_property(const BindingValue& value) {
    const std::optional<BindingValue> converted = convert_binding_value(
        value, BindingValueKind::signed_integer);
    if (!converted) {
        throw std::invalid_argument(
            "ComboBox.SelectedIndex binding requires an integer");
    }
    const std::int64_t index = std::get<std::int64_t>(*converted);
    if (index == -1) {
        set_selected_index(std::nullopt);
        return;
    }
    if (index < 0) {
        throw std::out_of_range(
            "ComboBox.SelectedIndex binding must be -1 or non-negative");
    }
    set_selected_index(static_cast<std::size_t>(index));
}

BindingValue ComboBox::text_property_value() const {
    return BindingValue{std::string(selected_text())};
}

void ComboBox::set_text_property(const BindingValue& value) {
    const std::optional<BindingValue> converted = convert_binding_value(
        value, BindingValueKind::text);
    if (!converted) {
        throw std::invalid_argument("ComboBox.Text binding requires text");
    }
    const std::string& text = std::get<std::string>(*converted);
    const std::vector<std::string>::iterator found =
        std::find(items_.begin(), items_.end(), text);
    set_selected_index(found == items_.end()
        ? std::optional<std::size_t>{}
        : std::optional<std::size_t>{static_cast<std::size_t>(
              std::distance(items_.begin(), found))});
}

void ComboBox::reset_text_property() {
    set_selected_index(std::nullopt);
}

bool ComboBox::should_serialize_text_property() const noexcept {
    return selected_index_.has_value();
}

void ComboBox::popup_selection_changed(const ListSelectionChange& change) {
    if (change.active_index) set_selected_index(change.active_index);
}

void ComboBox::set_items(std::vector<std::string> items) {
    require_mutable();
    for (const std::string& item : items) {
        if (!validate_utf8(item).valid()) {
            throw std::invalid_argument("ComboBox items must be valid UTF-8");
        }
    }
    if (items_ == items) return;
    items_ = std::move(items);
    if (selected_index_ && *selected_index_ >= items_.size()) {
        selected_index_.reset();
        publish_change(selected_index_changed_, selected_index_);
        if (!is_alive()) return;
    }
    if (popup_list_) (*popup_list_).set_items(items_);
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
    publish_change(items_changed_);
}

void ComboBox::add_item(std::string item) {
    require_mutable();
    if (!validate_utf8(item).valid()) {
        throw std::invalid_argument("ComboBox item must be valid UTF-8");
    }
    items_.push_back(std::move(item));
    if (popup_list_) (*popup_list_).set_items(items_);
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
    publish_change(items_changed_);
}

void ComboBox::set_selected_index(std::optional<std::size_t> index) {
    require_mutable();
    if (index && *index >= items_.size()) {
        throw std::out_of_range("ComboBox selection index is outside the collection");
    }
    if (selected_index_ == index) return;
    selected_index_ = index;
    if (popup_list_) {
        if (index) (*popup_list_).select_index(*index);
        else (*popup_list_).clear_selection();
    }
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(selected_index_changed_, selected_index_);
}

std::string_view ComboBox::selected_text() const noexcept {
    return selected_index_ ? std::string_view(items_[*selected_index_])
                           : std::string_view{};
}

void ComboBox::set_placeholder_text(std::string text) {
    require_mutable();
    if (!validate_utf8(text).valid()) {
        throw std::invalid_argument("ComboBox placeholder must be valid UTF-8");
    }
    if (placeholder_ == text) return;
    placeholder_ = std::move(text);
    invalidate(Dirty::paint | Dirty::semantics);
}

void ComboBox::set_maximum_drop_down_items(std::size_t count) {
    require_mutable();
    if (count == 0U || count > 64U) {
        throw std::invalid_argument("ComboBox drop-down row count must be between 1 and 64");
    }
    if (maximum_drop_down_items_ == count) return;
    maximum_drop_down_items_ = count;
    if (dropped_down_) {
        close_drop_down();
        open_drop_down();
    }
    invalidate(Dirty::semantics);
}

void ComboBox::set_drop_down_width(double width) {
    require_mutable();
    if (!std::isfinite(width) || width < 0.0 || width > 4096.0) {
        throw std::out_of_range(
            "ComboBox drop-down width must be finite and between 0 and 4096");
    }
    if (drop_down_width_ == width) return;
    drop_down_width_ = width;
    invalidate(Dirty::paint | Dirty::semantics);
}

void ComboBox::set_font(FontSpec font) {
    require_mutable();
    if (!valid_font_spec(font)) {
        throw std::invalid_argument("ComboBox font specification is invalid");
    }
    if (font_ == font) return;
    font_ = font;
    if (popup_list_) (*popup_list_).set_font(font);
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void ComboBox::set_dropped_down(bool dropped_down) {
    require_mutable();
    if (dropped_down == dropped_down_) return;
    if (dropped_down) open_drop_down();
    else close_drop_down();
}

void ComboBox::open_drop_down() {
    if (dropped_down_ || !attached() || window() == nullptr || items_.empty()) return;
    const Control::Ptr owner = (*window()).root();
    if (!owner) return;
    const Rect combo = absolute_bounds();
    const Size client = (*window()).client_size();
    const std::size_t rows = std::min(maximum_drop_down_items_, items_.size());
    const double popup_height = static_cast<double>(rows) *
        26.0 * effective_text_scale() + 4.0;
    const double requested_width = drop_down_width_ > 0.0
        ? drop_down_width_ : combo.width;
    const double popup_width = std::min(requested_width, client.width);
    const double popup_x = std::clamp(combo.x, 0.0,
        std::max(0.0, client.width - popup_width));
    const double popup_y = combo.y + combo.height + popup_height <= client.height
        ? combo.y + combo.height : std::max(0.0, combo.y - popup_height);

    const std::string prefix(stable_id().value());
    std::shared_ptr<gui_forms::DropDownLayer> layer = make_control<DropDownLayer>(StableId(prefix + ".popup.layer"));
    (*layer).set_requested_bounds({0.0, 0.0, client.width, client.height});
    std::shared_ptr<gui_forms::ListBox> list = make_control<ListBox>(StableId(prefix + ".popup.list"));
    (*list).set_paint_plane(PaintPlane::overlay);
    (*list).set_items(items_);
    (*list).set_font(font_);
    (*list).set_requested_bounds(
        {popup_x, popup_y, popup_width, popup_height});
    if (selected_index_) {
        (*list).select_index(*selected_index_);
    }
    (*layer).add_child(list);
    PopupToken popup_token = (*window()).open_popup(shared_from_this(), layer);

    popup_layer_ = layer;
    popup_list_ = list;
    popup_token_ = std::move(popup_token);
    const std::weak_ptr<ComboBox> weak =
        std::static_pointer_cast<ComboBox>(shared_from_this());
    popup_selection_ = (*list).selection_changed().subscribe(
        *this, detail::WeakMemberCallback<
            void (ComboBox::*)(const ListSelectionChange&)>(
                weak, &ComboBox::popup_selection_changed));
    popup_activation_ = (*list).item_activated().subscribe(
        *this, detail::WeakMemberCallback<
            void (ComboBox::*)(std::size_t)>(
                weak, &ComboBox::commit_popup_selection));
    popup_dismissal_ = (*layer).dismissed().subscribe(
        *this, detail::WeakMemberCallback<void (ComboBox::*)()>(
            weak, &ComboBox::close_drop_down));
    if (Event<>* closed = popup_token_.closed_event()) {
        popup_revocation_ = (*closed).subscribe(
            *this, detail::WeakMemberCallback<void (ComboBox::*)()>(
                weak, &ComboBox::on_popup_revoked));
    }
    popup_scope_ = (*window()).begin_focus_scope(layer, list).value;
    dropped_down_ = true;
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(drop_down_changed_, true);
}

void ComboBox::close_drop_down() {
    if (!dropped_down_ && !popup_layer_) return;
    closing_popup_ = true;
    popup_selection_.disconnect();
    popup_activation_.disconnect();
    popup_dismissal_.disconnect();
    if (window() != nullptr && popup_scope_ != 0U) {
        static_cast<void>((*window()).end_focus_scope(FocusScopeId{popup_scope_}));
    }
    popup_scope_ = 0U;
    popup_token_.disconnect();
    popup_revocation_.disconnect();
    popup_list_.reset();
    popup_layer_.reset();
    const bool changed = dropped_down_;
    dropped_down_ = false;
    closing_popup_ = false;
    invalidate(Dirty::paint | Dirty::semantics);
    if (changed) publish_change(drop_down_changed_, false);
}

void ComboBox::on_popup_revoked() {
    if (closing_popup_) return;
    popup_selection_.disconnect();
    popup_activation_.disconnect();
    popup_dismissal_.disconnect();
    popup_revocation_.disconnect();
    if (window() != nullptr && popup_scope_ != 0U) {
        static_cast<void>((*window()).end_focus_scope(FocusScopeId{popup_scope_},
                                                     FocusScopeCloseReason::owner_unavailable));
    }
    popup_scope_ = 0U;
    popup_list_.reset();
    popup_layer_.reset();
    const bool changed = dropped_down_;
    dropped_down_ = false;
    invalidate(Dirty::paint | Dirty::semantics);
    if (changed) publish_change(drop_down_changed_, false);
}

void ComboBox::commit_popup_selection(std::size_t index) {
    set_selected_index(index);
    close_drop_down();
}

void ComboBox::on_paint(Painter& painter, Rect damage) {
    const Rect bounds = local_bounds();
    const bool themed = !has_background_override() && !has_style_override();
    const ControlVisualRecipe& editor_recipe = effective_theme().resolve(
        ControlVisualRole::editor,
        visual_context(false, dropped_down_, false, focused_));
    if (themed) {
        paint_surface_material(painter, bounds, editor_recipe.material);
    } else {
        Panel::on_paint(painter, damage);
    }
    const FontSpec font = effective_font(font_);
    const double button_width = std::min(24.0, bounds.width);
    const Rect button_bounds{std::max(0.0, bounds.width - button_width), 1.0,
                             std::max(0.0, button_width - 1.0),
                             std::max(0.0, bounds.height - 2.0)};
    const ControlVisualRecipe& button_recipe = effective_theme().resolve(
        ControlVisualRole::choice,
        visual_context(false, dropped_down_, dropped_down_, focused_));
    if (themed) {
        paint_surface_material(painter, button_bounds, button_recipe.material);
    } else {
        painter.fill_rect(button_bounds, style().face);
    }
    painter.draw_line({bounds.width - button_width, 1.0},
                      {bounds.width - button_width, bounds.height - 1.0},
                      themed ? (button_recipe.material.border
                                    ? (*button_recipe.material.border).color
                                    : button_recipe.glyph)
                             : style().border,
                      1.0);
    const double center_x = bounds.width - button_width * 0.5;
    const double center_y = bounds.height * 0.5 + (dropped_down_ ? 2.0 : -1.0);
    const double direction = dropped_down_ ? -1.0 : 1.0;
    painter.draw_line({center_x - 4.0, center_y - direction * 2.0},
                      {center_x, center_y + direction * 2.0},
                      themed ? button_recipe.glyph : style().dark_border, 1.0);
    painter.draw_line({center_x, center_y + direction * 2.0},
                      {center_x + 4.0, center_y - direction * 2.0},
                      themed ? button_recipe.glyph : style().dark_border, 1.0);
    const std::string_view text = selected_index_ ? selected_text()
                                                  : std::string_view(placeholder_);
    painter.save();
    painter.clip_rect({5.0, 2.0,
                       std::max(0.0, bounds.width - button_width - 8.0),
                       std::max(0.0, bounds.height - 4.0)});
    painter.draw_text_utf8({7.0, std::max(font.size,
                            (bounds.height + font.size) * 0.5 - 1.0)},
                           text, font,
                           themed ? (selected_index_ ? editor_recipe.text
                                                     : editor_recipe.muted_text)
                                  : selected_index_ ? style().text
                                                    : style().disabled_text);
    painter.restore();
    if (focused_ || dropped_down_) {
        const Rect ring{1.5, 1.5, std::max(0.0, bounds.width - 3.0),
                        std::max(0.0, bounds.height - 3.0)};
        if (themed) {
            painter.stroke_rounded_rect(
                ring, std::max(0.0, editor_recipe.material.corner_radius - 1.0),
                editor_recipe.focus_ring, editor_recipe.focus_width);
        } else {
            painter.stroke_rect(ring, style().accent, 1.0);
        }
    }
}

void ComboBox::on_pointer(PointerEvent& event) {
    if (!eligible_for_input()) return;
    if (event.action == PointerAction::down && event.button == PointerButton::primary) {
        set_dropped_down(!dropped_down_);
        event.handled = true;
    } else if (event.action == PointerAction::up &&
               event.button == PointerButton::primary) {
        event.handled = true;
    }
}

void ComboBox::on_key(KeyEvent& event) {
    if (!focused_ || !enabled() || event.action != KeyAction::down) return;
    if (event.physical_key == PhysicalKey::escape && dropped_down_) {
        close_drop_down();
        event.handled = true;
        return;
    }
    if (event.physical_key == PhysicalKey::f4 ||
        (event.physical_key == PhysicalKey::down && includes(event.modifiers, Modifier::alt)) ||
        event.physical_key == PhysicalKey::space) {
        set_dropped_down(!dropped_down_);
        event.handled = true;
        return;
    }
    if (!dropped_down_ && !items_.empty() &&
        (event.physical_key == PhysicalKey::up ||
         event.physical_key == PhysicalKey::down ||
         event.physical_key == PhysicalKey::home ||
         event.physical_key == PhysicalKey::end)) {
        std::size_t next = selected_index_.value_or(0U);
        if (event.physical_key == PhysicalKey::up) next = next == 0U ? 0U : next - 1U;
        else if (event.physical_key == PhysicalKey::down) next = std::min(next + 1U, items_.size() - 1U);
        else if (event.physical_key == PhysicalKey::home) next = 0U;
        else next = items_.size() - 1U;
        set_selected_index(next);
        event.handled = true;
    }
}

void ComboBox::on_focus_changed(bool focused) {
    focused_ = focused;
    invalidate(Dirty::paint | Dirty::semantics);
}

SemanticDescriptor ComboBox::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::combo_box;
    descriptor.name = accessible_name();
    descriptor.value = std::string(selected_text());
    descriptor.description = accessible_description().empty()
        ? placeholder_ : accessible_description();
    if (dropped_down_) descriptor.states |= SemanticState::expanded;
    descriptor.actions = {SemanticAction::focus,
        dropped_down_ ? SemanticAction::collapse : SemanticAction::expand};
    descriptor.exposed = true;
    return descriptor;
}

bool ComboBox::on_semantic_action(SemanticAction action, std::string_view value) {
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

void ComboBox::on_detached_from_window() noexcept {
    popup_selection_.disconnect();
    popup_activation_.disconnect();
    popup_dismissal_.disconnect();
    popup_revocation_.disconnect();
    popup_scope_ = 0U;
    popup_token_.disconnect();
    popup_list_.reset();
    popup_layer_.reset();
    dropped_down_ = false;
    focused_ = false;
}

} // namespace gui_forms
