#include "gui_forms/inspection_controls.hpp"

#include "gui_forms/text.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <iterator>
#include <limits>
#include <map>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace gui_forms {
namespace {

constexpr double property_group_height = 25.0;
constexpr double property_row_height = 29.0;
constexpr double property_editor_height = 34.0;
constexpr double property_validation_height = 18.0;
constexpr double property_padding = 8.0;
constexpr double property_gap = 7.0;

class PropertyEditorDropDownLayer final : public Panel {
public:
    explicit PropertyEditorDropDownLayer(StableId stable_id)
        : Panel(std::move(stable_id)) {
        set_paint_plane(PaintPlane::overlay);
        set_border_style(BorderStyle::none);
        set_background(Color::rgba(0, 0, 0, 0));
    }

    [[nodiscard]] Event<>& dismissed() noexcept { return dismissed_; }

    void on_pointer(PointerEvent& event) override {
        if (event.action == PointerAction::down &&
            event.button == PointerButton::primary) {
            dismissed_.emit();
            event.handled = true;
        }
    }

    void on_key_preview(KeyEvent& event) override {
        if (event.action == KeyAction::down &&
            event.physical_key == PhysicalKey::escape) {
            dismissed_.emit();
            event.handled = true;
        }
    }

private:
    Event<> dismissed_;
};

std::string property_service_name(std::string_view name) {
    if (name.empty() || name.size() > 256U ||
        !validate_utf8(name).valid()) {
        throw std::invalid_argument(
            "Property service names must be nonempty bounded UTF-8");
    }
    const std::string canonical = canonical_binding_name(name);
    if (canonical.empty()) {
        throw std::invalid_argument(
            "Property service names must contain a visible character");
    }
    return canonical;
}

void require_conversion_context(const PropertyConversionContext& context) {
    if (context.culture_name.empty() || context.culture_name.size() > 128U ||
        context.decimal_separator.empty() ||
        context.decimal_separator.size() > 8U ||
        context.group_separator.size() > 8U ||
        context.decimal_separator == context.group_separator ||
        !validate_utf8(context.culture_name).valid() ||
        !validate_utf8(context.decimal_separator).valid() ||
        !validate_utf8(context.group_separator).valid()) {
        throw std::invalid_argument(
            "Property conversion context must use distinct bounded UTF-8 separators");
    }
}

void replace_text(std::string& text, std::string_view from,
                  std::string_view to) {
    if (from.empty()) return;
    std::size_t position = 0U;
    while ((position = text.find(from, position)) != std::string::npos) {
        text.replace(position, from.size(), to);
        position += to.size();
    }
}

std::string localize_number(std::string text,
                            const PropertyConversionContext& context) {
    const std::size_t decimal = text.find('.');
    const std::size_t integer_end = decimal == std::string::npos
        ? text.size() : decimal;
    if (context.use_grouping && !context.group_separator.empty()) {
        const std::size_t first_digit = !text.empty() &&
                (text.front() == '-' || text.front() == '+')
            ? 1U : 0U;
        std::size_t digits = integer_end - first_digit;
        while (digits > 3U) {
            const std::size_t insertion = first_digit + digits - 3U;
            text.insert(insertion, context.group_separator);
            digits -= 3U;
        }
    }
    if (context.decimal_separator != ".") {
        const std::size_t localized_decimal = text.find('.', integer_end);
        if (localized_decimal != std::string::npos) {
            text.replace(localized_decimal, 1U, context.decimal_separator);
        }
    }
    return text;
}

std::optional<std::string> invariant_number_text(
    std::string_view text, const PropertyConversionContext& context) {
    std::string normalized(text);
    if (!context.group_separator.empty()) {
        replace_text(normalized, context.group_separator, {});
    }
    if (context.decimal_separator != ".") {
        if (normalized.find('.') != std::string::npos) return {};
        replace_text(normalized, context.decimal_separator, ".");
    }
    return normalized;
}

std::uint64_t property_virtual_runtime_id(std::string_view id) noexcept {
    std::uint64_t hash = 1469598103934665603ULL;
    for (const unsigned char byte : id) {
        hash ^= byte;
        hash *= 1099511628211ULL;
    }
    return hash | (1ULL << 63U);
}

void require_property_text(std::string_view value, const char* field) {
    if (!validate_utf8(value).valid()) {
        throw std::invalid_argument(std::string("PropertyList ") + field +
                                    " must be valid UTF-8");
    }
}

struct CompoundFieldSpec final {
    CompoundFieldSpec(std::string_view authored_name,
                      BindingValueKind authored_kind,
                      PropertyEditorKind authored_editor =
                          PropertyEditorKind::text,
                      std::vector<std::string> authored_choices = {})
        : name(authored_name), kind(authored_kind), editor(authored_editor),
          choices(std::move(authored_choices)) {}

    std::string_view name;
    BindingValueKind kind;
    PropertyEditorKind editor{PropertyEditorKind::text};
    std::vector<std::string> choices;
};

std::vector<CompoundFieldSpec> compound_fields(BindingValueKind kind) {
    switch (kind) {
    case BindingValueKind::point:
        return {{"X", BindingValueKind::number},
                {"Y", BindingValueKind::number}};
    case BindingValueKind::size:
        return {{"Width", BindingValueKind::number},
                {"Height", BindingValueKind::number}};
    case BindingValueKind::rectangle:
        return {{"X", BindingValueKind::number},
                {"Y", BindingValueKind::number},
                {"Width", BindingValueKind::number},
                {"Height", BindingValueKind::number}};
    case BindingValueKind::insets:
        return {{"Left", BindingValueKind::number},
                {"Top", BindingValueKind::number},
                {"Right", BindingValueKind::number},
                {"Bottom", BindingValueKind::number}};
    case BindingValueKind::color:
        return {{"Red", BindingValueKind::unsigned_integer},
                {"Green", BindingValueKind::unsigned_integer},
                {"Blue", BindingValueKind::unsigned_integer},
                {"Alpha", BindingValueKind::unsigned_integer}};
    case BindingValueKind::font:
        return {{"Role", BindingValueKind::text, PropertyEditorKind::choice,
                 {"Control", "Content", "Monospace"}},
                {"Size", BindingValueKind::number},
                {"Weight", BindingValueKind::unsigned_integer},
                {"Italic", BindingValueKind::boolean,
                 PropertyEditorKind::boolean},
                {"LetterSpacing", BindingValueKind::number}};
    default:
        return {};
    }
}

std::optional<BindingValue> compound_field_value(
    const BindingValue& value, std::string_view field_name) {
    const std::string field = canonical_binding_name(field_name);
    if (const auto* item = std::get_if<Point>(&value)) {
        if (field == "x") return BindingValue{item->x};
        if (field == "y") return BindingValue{item->y};
    } else if (const auto* item = std::get_if<Size>(&value)) {
        if (field == "width") return BindingValue{item->width};
        if (field == "height") return BindingValue{item->height};
    } else if (const auto* item = std::get_if<Rect>(&value)) {
        if (field == "x") return BindingValue{item->x};
        if (field == "y") return BindingValue{item->y};
        if (field == "width") return BindingValue{item->width};
        if (field == "height") return BindingValue{item->height};
    } else if (const auto* item = std::get_if<Insets>(&value)) {
        if (field == "left") return BindingValue{item->left};
        if (field == "top") return BindingValue{item->top};
        if (field == "right") return BindingValue{item->right};
        if (field == "bottom") return BindingValue{item->bottom};
    } else if (const auto* item = std::get_if<Color>(&value)) {
        if (field == "red") return BindingValue{static_cast<std::uint64_t>(item->red)};
        if (field == "green") return BindingValue{static_cast<std::uint64_t>(item->green)};
        if (field == "blue") return BindingValue{static_cast<std::uint64_t>(item->blue)};
        if (field == "alpha") return BindingValue{static_cast<std::uint64_t>(item->alpha)};
    } else if (const auto* item = std::get_if<FontSpec>(&value)) {
        if (field == "role") {
            switch (item->role) {
            case FontRole::control: return BindingValue{std::string("Control")};
            case FontRole::content: return BindingValue{std::string("Content")};
            case FontRole::monospace: return BindingValue{std::string("Monospace")};
            }
        }
        if (field == "size") return BindingValue{item->size};
        if (field == "weight") {
            return BindingValue{static_cast<std::uint64_t>(item->weight)};
        }
        if (field == "italic") return BindingValue{item->italic};
        if (field == "letterspacing") return BindingValue{item->letter_spacing};
    }
    return {};
}

std::optional<BindingValue> replace_compound_field(
    BindingValue value, std::string_view field_name,
    const BindingValue& replacement) {
    const std::string field = canonical_binding_name(field_name);
    if (auto* item = std::get_if<Point>(&value)) {
        const auto converted = convert_binding_value(
            replacement, BindingValueKind::number);
        if (!converted) return {};
        if (field == "x") item->x = std::get<double>(*converted);
        else if (field == "y") item->y = std::get<double>(*converted);
        else return {};
    } else if (auto* item = std::get_if<Size>(&value)) {
        const auto converted = convert_binding_value(
            replacement, BindingValueKind::number);
        if (!converted) return {};
        if (field == "width") item->width = std::get<double>(*converted);
        else if (field == "height") item->height = std::get<double>(*converted);
        else return {};
    } else if (auto* item = std::get_if<Rect>(&value)) {
        const auto converted = convert_binding_value(
            replacement, BindingValueKind::number);
        if (!converted) return {};
        if (field == "x") item->x = std::get<double>(*converted);
        else if (field == "y") item->y = std::get<double>(*converted);
        else if (field == "width") item->width = std::get<double>(*converted);
        else if (field == "height") item->height = std::get<double>(*converted);
        else return {};
    } else if (auto* item = std::get_if<Insets>(&value)) {
        const auto converted = convert_binding_value(
            replacement, BindingValueKind::number);
        if (!converted) return {};
        if (field == "left") item->left = std::get<double>(*converted);
        else if (field == "top") item->top = std::get<double>(*converted);
        else if (field == "right") item->right = std::get<double>(*converted);
        else if (field == "bottom") item->bottom = std::get<double>(*converted);
        else return {};
    } else if (auto* item = std::get_if<Color>(&value)) {
        const auto converted = convert_binding_value(
            replacement, BindingValueKind::unsigned_integer);
        if (!converted || std::get<std::uint64_t>(*converted) > 255U) return {};
        const auto channel = static_cast<std::uint8_t>(
            std::get<std::uint64_t>(*converted));
        if (field == "red") item->red = channel;
        else if (field == "green") item->green = channel;
        else if (field == "blue") item->blue = channel;
        else if (field == "alpha") item->alpha = channel;
        else return {};
    } else if (auto* item = std::get_if<FontSpec>(&value)) {
        if (field == "role") {
            const std::string role = canonical_binding_name(
                binding_value_to_string(replacement));
            if (role == "control") item->role = FontRole::control;
            else if (role == "content") item->role = FontRole::content;
            else if (role == "monospace") item->role = FontRole::monospace;
            else return {};
        } else if (field == "size" || field == "letterspacing") {
            const auto converted = convert_binding_value(
                replacement, BindingValueKind::number);
            if (!converted) return {};
            if (field == "size") item->size = std::get<double>(*converted);
            else item->letter_spacing = std::get<double>(*converted);
        } else if (field == "weight") {
            const auto converted = convert_binding_value(
                replacement, BindingValueKind::unsigned_integer);
            if (!converted || std::get<std::uint64_t>(*converted) == 0U ||
                std::get<std::uint64_t>(*converted) > 1000U) return {};
            item->weight = static_cast<std::uint16_t>(
                std::get<std::uint64_t>(*converted));
        } else if (field == "italic") {
            const auto converted = convert_binding_value(
                replacement, BindingValueKind::boolean);
            if (!converted) return {};
            item->italic = std::get<bool>(*converted);
        } else {
            return {};
        }
    } else {
        return {};
    }
    return value;
}

} // namespace

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
    list->set_requested_bounds({editor.x, y, editor.width, height});
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

ColorValueEditor::ColorValueEditor(StableId stable_id, Color value)
    : Panel(std::move(stable_id)), value_(value) {
    set_paint_plane(PaintPlane::control);
    set_border_style(BorderStyle::none);
}

void ColorValueEditor::initialize_control_tree() {
    if (editor_) return;
    editor_ = make_control<TextBox>(
        StableId(std::string(stable_id().value()) + ".text"),
        format_value(value_));
    editor_->set_font({FontRole::monospace, 9.5, 400, false});
    add_child(editor_);
    const std::weak_ptr<ColorValueEditor> weak =
        std::static_pointer_cast<ColorValueEditor>(shared_from_this());
    committed_ = editor_->committed().subscribe(
        *this, [weak](const std::string& text) {
            if (const auto retained = weak.lock()) retained->commit(text);
        });
    cancelled_ = editor_->cancelled().subscribe(*this, [weak] {
        if (const auto retained = weak.lock()) retained->cancel();
    });
}

std::string ColorValueEditor::format_value(Color value) {
    constexpr char digits[] = "0123456789ABCDEF";
    std::string result{"#00000000"};
    const std::uint8_t channels[]{value.red, value.green, value.blue, value.alpha};
    for (std::size_t index = 0U; index < 4U; ++index) {
        result[1U + index * 2U] = digits[channels[index] >> 4U];
        result[2U + index * 2U] = digits[channels[index] & 0x0fU];
    }
    return result;
}

std::optional<Color> ColorValueEditor::parse_value(std::string_view text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) return {};
    const auto last = text.find_last_not_of(" \t\r\n");
    text = text.substr(first, last - first + 1U);
    if ((text.size() != 7U && text.size() != 9U) || text.front() != '#') {
        return {};
    }
    const auto nibble = [](char value) -> std::optional<std::uint8_t> {
        if (value >= '0' && value <= '9') {
            return static_cast<std::uint8_t>(value - '0');
        }
        if (value >= 'a' && value <= 'f') {
            return static_cast<std::uint8_t>(value - 'a' + 10);
        }
        if (value >= 'A' && value <= 'F') {
            return static_cast<std::uint8_t>(value - 'A' + 10);
        }
        return {};
    };
    std::uint8_t channels[4]{0U, 0U, 0U, 255U};
    const std::size_t count = text.size() == 9U ? 4U : 3U;
    for (std::size_t index = 0U; index < count; ++index) {
        const auto high = nibble(text[1U + index * 2U]);
        const auto low = nibble(text[2U + index * 2U]);
        if (!high || !low) return {};
        channels[index] = static_cast<std::uint8_t>((*high << 4U) | *low);
    }
    return Color::rgba(channels[0], channels[1], channels[2], channels[3]);
}

void ColorValueEditor::set_value(Color value) {
    require_mutable();
    if (value_ == value) {
        if (editor_ && editor_->text() != format_value(value)) {
            synchronizing_ = true;
            editor_->set_text(format_value(value));
            synchronizing_ = false;
        }
        if (editor_) editor_->set_visual_status(ControlVisualStatus::normal);
        set_visual_status(ControlVisualStatus::normal);
        invalidate(Dirty::paint | Dirty::semantics);
        return;
    }
    value_ = value;
    if (editor_) {
        synchronizing_ = true;
        editor_->set_text(format_value(value_));
        editor_->set_visual_status(ControlVisualStatus::normal);
        synchronizing_ = false;
    }
    set_visual_status(ControlVisualStatus::normal);
    invalidate(Dirty::paint | Dirty::semantics);
}

void ColorValueEditor::commit(std::string_view text) {
    if (synchronizing_) return;
    const auto parsed = parse_value(text);
    if (!parsed) {
        set_visual_status(ControlVisualStatus::invalid);
        if (editor_) editor_->set_visual_status(ControlVisualStatus::invalid);
        const PropertyEditorInputError failure{
            std::string(text), "Color must be #RRGGBB or #RRGGBBAA"};
        edit_failed_.emit(failure);
        return;
    }
    const bool changed = *parsed != value_;
    value_ = *parsed;
    set_visual_status(ControlVisualStatus::normal);
    if (editor_) {
        synchronizing_ = true;
        editor_->set_text(format_value(value_));
        editor_->set_visual_status(ControlVisualStatus::normal);
        synchronizing_ = false;
    }
    invalidate(Dirty::paint | Dirty::semantics);
    if (changed) publish_change(value_changed_, value_);
}

void ColorValueEditor::cancel() {
    if (!editor_) return;
    synchronizing_ = true;
    editor_->set_text(format_value(value_));
    editor_->set_visual_status(ControlVisualStatus::normal);
    synchronizing_ = false;
    set_visual_status(ControlVisualStatus::normal);
    invalidate(Dirty::paint | Dirty::semantics);
}

Size ColorValueEditor::measure(Size available) {
    return {std::max(0.0, available.width), 28.0 * effective_text_scale()};
}

void ColorValueEditor::arrange(Rect final_bounds) {
    arrange_self(final_bounds);
    const double swatch = std::min(28.0, std::max(0.0, final_bounds.width));
    if (editor_) {
        set_child_layout(editor_, {swatch, 0.0,
            std::max(0.0, final_bounds.width - swatch), final_bounds.height});
    }
}

void ColorValueEditor::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    const double swatch_width = std::min(28.0, bounds.width);
    const Rect swatch{2.0, 2.0, std::max(0.0, swatch_width - 5.0),
                      std::max(0.0, bounds.height - 4.0)};
    const double half_width = swatch.width * 0.5;
    const double half_height = swatch.height * 0.5;
    painter.fill_rect({swatch.x, swatch.y, half_width, half_height},
                      Color::rgba(232, 232, 232));
    painter.fill_rect({swatch.x + half_width, swatch.y, half_width, half_height},
                      Color::rgba(178, 178, 178));
    painter.fill_rect({swatch.x, swatch.y + half_height, half_width, half_height},
                      Color::rgba(178, 178, 178));
    painter.fill_rect({swatch.x + half_width, swatch.y + half_height,
                       half_width, half_height}, Color::rgba(232, 232, 232));
    painter.fill_rect(swatch, value_);
    painter.stroke_rect({swatch.x + 0.5, swatch.y + 0.5,
                         std::max(0.0, swatch.width - 1.0),
                         std::max(0.0, swatch.height - 1.0)},
                        style().dark_border, 1.0);
}

SemanticDescriptor ColorValueEditor::semantic_descriptor() const {
    SemanticDescriptor result;
    result.role = SemanticRole::group;
    result.name = accessible_name();
    result.value = format_value(value_);
    result.description = accessible_description();
    result.exposed = true;
    if (visual_status() == ControlVisualStatus::invalid) {
        result.states |= SemanticState::invalid;
    }
    return result;
}

void ColorValueEditor::on_dispose() noexcept {
    committed_.disconnect();
    cancelled_.disconnect();
    editor_.reset();
    Control::on_dispose();
}

bool PropertyValueConverterRegistry::register_converter(
    std::string name, PropertyValueConverter converter) {
    const std::string canonical = property_service_name(name);
    if ((!converter.format && !converter.format_with_context) ||
        (!converter.parse && !converter.parse_with_context)) {
        throw std::invalid_argument(
            "Property converters require format and parse callbacks");
    }
    return converters_.emplace(canonical, std::move(converter)).second;
}

bool PropertyValueConverterRegistry::unregister_converter(
    std::string_view name) {
    const std::string canonical = property_service_name(name);
    const bool removed = converters_.erase(canonical) != 0U;
    if (removed) {
        std::erase_if(kind_mappings_, [&canonical](const auto& item) {
            return item.second == canonical;
        });
    }
    return removed;
}

void PropertyValueConverterRegistry::map_kind(
    BindingValueKind kind, std::string converter_name) {
    const std::string canonical = property_service_name(converter_name);
    if (!converters_.contains(canonical)) {
        throw std::invalid_argument(
            "Property converter kind mapping requires a registered converter");
    }
    kind_mappings_.insert_or_assign(kind, canonical);
}

void PropertyValueConverterRegistry::clear_kind(BindingValueKind kind) {
    kind_mappings_.erase(kind);
}

std::optional<std::string> PropertyValueConverterRegistry::converter_for(
    BindingValueKind kind) const {
    const auto found = kind_mappings_.find(kind);
    return found == kind_mappings_.end()
        ? std::optional<std::string>{}
        : std::optional<std::string>{found->second};
}

const PropertyValueConverter* PropertyValueConverterRegistry::find(
    std::string_view name) const noexcept {
    if (name.empty() || !validate_utf8(name).valid()) return nullptr;
    const auto found = converters_.find(canonical_binding_name(name));
    return found == converters_.end() ? nullptr : &found->second;
}

std::string PropertyValueConverterRegistry::format(
    const BindingValue& value, const PropertyDescriptor& descriptor) const {
    if (binding_value_kind(value) == BindingValueKind::null) return "(none)";
    std::string service = descriptor.converter_name;
    if (service.empty()) {
        const auto mapped = converter_for(descriptor.kind);
        if (mapped) service = *mapped;
    }
    const PropertyValueConverter* converter = find(service);
    std::string result = converter
        ? (converter->format_with_context
               ? converter->format_with_context(value, descriptor, context_)
               : converter->format(value, descriptor))
        : binding_value_to_string(value);
    if (result.size() > 64U * 1024U || !validate_utf8(result).valid()) {
        throw std::invalid_argument(
            "Property converter produced invalid or unbounded display text");
    }
    return result;
}

std::optional<BindingValue> PropertyValueConverterRegistry::parse(
    std::string_view text, const BindingValue& current,
    const PropertyDescriptor& descriptor) const {
    if (text.size() > 64U * 1024U || !validate_utf8(text).valid()) return {};
    if (descriptor.nullable && (text.empty() || text == "(none)" ||
                                text == "(None)")) {
        return BindingValue{std::monostate{}};
    }
    for (const BindingValue& standard : descriptor.standard_values) {
        if (format(standard, descriptor) == text) return standard;
    }
    if (descriptor.standard_values_exclusive) return {};
    std::string service = descriptor.converter_name;
    if (service.empty()) {
        const auto mapped = converter_for(descriptor.kind);
        if (mapped) service = *mapped;
    }
    const PropertyValueConverter* converter = find(service);
    if (!converter) return {};
    const auto result = converter->parse_with_context
        ? converter->parse_with_context(text, current, descriptor, context_)
        : converter->parse(text, current, descriptor);
    const auto converted = result
        ? convert_property_value(*result, descriptor)
        : std::optional<BindingValue>{};
    return converted && valid_property_value_tree(*converted)
        ? converted : std::optional<BindingValue>{};
}

void PropertyValueConverterRegistry::set_context(
    PropertyConversionContext context) {
    require_conversion_context(context);
    context_ = std::move(context);
}

std::shared_ptr<PropertyValueConverterRegistry>
PropertyValueConverterRegistry::create_default() {
    auto result = std::make_shared<PropertyValueConverterRegistry>();
    PropertyValueConverter invariant;
    invariant.format = [](const BindingValue& value,
                          const PropertyDescriptor&) {
        if (const auto* boolean = std::get_if<bool>(&value)) {
            return std::string(*boolean ? "True" : "False");
        }
        return binding_value_to_string(value);
    };
    invariant.parse = [](std::string_view text, const BindingValue&,
                         const PropertyDescriptor& descriptor) {
        return convert_property_value(BindingValue{std::string(text)},
                                      descriptor);
    };
    invariant.format_with_context = [](
        const BindingValue& value, const PropertyDescriptor& descriptor,
        const PropertyConversionContext& context) {
        if (const auto* boolean = std::get_if<bool>(&value)) {
            return std::string(*boolean ? "True" : "False");
        }
        std::string formatted = binding_value_to_string(value);
        return descriptor.kind == BindingValueKind::signed_integer ||
                descriptor.kind == BindingValueKind::unsigned_integer ||
                descriptor.kind == BindingValueKind::number
            ? localize_number(std::move(formatted), context)
            : formatted;
    };
    invariant.parse_with_context = [](
        std::string_view text, const BindingValue&,
        const PropertyDescriptor& descriptor,
        const PropertyConversionContext& context)
            -> std::optional<BindingValue> {
        if (descriptor.kind == BindingValueKind::signed_integer ||
            descriptor.kind == BindingValueKind::unsigned_integer ||
            descriptor.kind == BindingValueKind::number) {
            const auto normalized = invariant_number_text(text, context);
            return normalized
                ? convert_property_value(BindingValue{*normalized}, descriptor)
                : std::optional<BindingValue>{};
        }
        return convert_property_value(BindingValue{std::string(text)},
                                      descriptor);
    };
    static_cast<void>(result->register_converter("invariant", invariant));
    for (const BindingValueKind kind : {
             BindingValueKind::boolean, BindingValueKind::signed_integer,
             BindingValueKind::unsigned_integer, BindingValueKind::number,
             BindingValueKind::text, BindingValueKind::enumeration}) {
        result->map_kind(kind, "invariant");
    }
    PropertyValueConverter color_hex;
    color_hex.format = [](const BindingValue& value,
                          const PropertyDescriptor&) {
        const auto* color = std::get_if<Color>(&value);
        return color ? ColorValueEditor::format_value(*color) : std::string{};
    };
    color_hex.parse = [](std::string_view text, const BindingValue&,
                         const PropertyDescriptor&) -> std::optional<BindingValue> {
        const auto color = ColorValueEditor::parse_value(text);
        return color ? std::optional<BindingValue>{BindingValue{*color}}
                     : std::optional<BindingValue>{};
    };
    static_cast<void>(result->register_converter("color-hex", color_hex));
    result->map_kind(BindingValueKind::color, "color-hex");
    return result;
}

bool PropertyEditorRegistry::register_factory(
    std::string name, PropertyEditorFactory factory) {
    const std::string canonical = property_service_name(name);
    if (!factory) {
        throw std::invalid_argument(
            "Property editor factories require a callback");
    }
    return factories_.emplace(canonical, std::move(factory)).second;
}

bool PropertyEditorRegistry::unregister_factory(std::string_view name) {
    const std::string canonical = property_service_name(name);
    const bool removed = factories_.erase(canonical) != 0U;
    if (removed) {
        std::erase_if(kind_mappings_, [&canonical](const auto& item) {
            return item.second == canonical;
        });
    }
    return removed;
}

void PropertyEditorRegistry::map_kind(BindingValueKind kind,
                                      std::string factory_name) {
    const std::string canonical = property_service_name(factory_name);
    if (!factories_.contains(canonical)) {
        throw std::invalid_argument(
            "Property editor kind mapping requires a registered factory");
    }
    kind_mappings_.insert_or_assign(kind, canonical);
}

void PropertyEditorRegistry::clear_kind(BindingValueKind kind) {
    kind_mappings_.erase(kind);
}

std::optional<std::string> PropertyEditorRegistry::factory_for(
    BindingValueKind kind) const {
    const auto found = kind_mappings_.find(kind);
    return found == kind_mappings_.end()
        ? std::optional<std::string>{}
        : std::optional<std::string>{found->second};
}

std::optional<PropertyEditorBinding> PropertyEditorRegistry::create(
    const PropertyEditorRequest& request) const {
    if (!request.writable) return {};
    const bool explicit_service = request.top_level &&
        !request.descriptor.editor_name.empty();
    std::string service = explicit_service
        ? request.descriptor.editor_name : std::string{};
    if (service.empty()) {
        const auto mapped = factory_for(binding_value_kind(request.value));
        if (mapped) service = *mapped;
    }
    if (service.empty()) return {};
    const auto found = factories_.find(canonical_binding_name(service));
    if (found == factories_.end()) {
        if (explicit_service) {
            throw std::invalid_argument(
                "Declared property editor factory is not registered");
        }
        return {};
    }
    auto result = found->second(request);
    if (!result) return {};
    if (!result->control || !result->control->is_alive() ||
        result->control->parent() || result->control->attached_window() ||
        !result->synchronize || !result->connect_committed) {
        throw std::invalid_argument(
            "Property editor factory returned an invalid retained binding");
    }
    return result;
}

std::shared_ptr<PropertyEditorRegistry> PropertyEditorRegistry::create_default() {
    auto result = std::make_shared<PropertyEditorRegistry>();
    static_cast<void>(result->register_factory(
        "numeric-up-down", [](const PropertyEditorRequest& request)
            -> std::optional<PropertyEditorBinding> {
            const auto number = binding_value_to_number(request.value);
            if (!number) return {};
            auto editor = make_control<NumericUpDown>(StableId(request.stable_id));
            editor->set_range(std::numeric_limits<double>::lowest(),
                              std::numeric_limits<double>::max());
            editor->set_increment(0.1);
            editor->set_decimal_places(4U);
            editor->set_value(*number);
            editor->set_enabled(request.writable);
            editor->set_accessible_name(request.property_path);
            editor->set_accessible_description(request.descriptor.description);
            auto synchronizing = std::make_shared<bool>(false);
            PropertyEditorBinding binding;
            binding.control = editor;
            binding.synchronize =
                [weak = std::weak_ptr<NumericUpDown>(editor), synchronizing](
                    const BindingValue& value) {
                    const auto retained = weak.lock();
                    const auto converted = binding_value_to_number(value);
                    if (!retained || !converted) return;
                    *synchronizing = true;
                    retained->set_value(*converted);
                    *synchronizing = false;
                };
            binding.connect_committed =
                [weak = std::weak_ptr<NumericUpDown>(editor), synchronizing](
                    Component& owner,
                    std::function<void(BindingValue)> committed) {
                    const auto retained = weak.lock();
                    return retained
                        ? retained->value_changed().subscribe(
                              owner,
                              [synchronizing,
                               committed = std::move(committed)](double value) {
                                  if (!*synchronizing) {
                                      committed(BindingValue{value});
                                  }
                              })
                        : SubscriptionToken{};
                };
            return binding;
        }));
    static_cast<void>(result->register_factory(
        "flags-value", [](const PropertyEditorRequest& request)
            -> std::optional<PropertyEditorBinding> {
            if (!request.descriptor.enumeration ||
                !request.descriptor.enumeration->flags) return {};
            const auto* value = std::get_if<PropertyEnumValue>(&request.value);
            if (!value) return {};
            const bool has_bit = std::any_of(
                request.descriptor.enumeration->choices.begin(),
                request.descriptor.enumeration->choices.end(),
                [](const PropertyEnumChoice& choice) {
                    return choice.value > 0 && std::has_single_bit(
                        static_cast<std::uint64_t>(choice.value));
                });
            if (!has_bit) return {};
            auto editor = make_control<FlagsValueEditor>(
                StableId(request.stable_id), *request.descriptor.enumeration,
                *value);
            editor->set_enabled(request.writable);
            editor->set_accessible_name(request.property_path);
            editor->set_accessible_description(request.descriptor.description);
            auto synchronizing = std::make_shared<bool>(false);
            PropertyEditorBinding binding;
            binding.control = editor;
            binding.synchronize =
                [weak = std::weak_ptr<FlagsValueEditor>(editor), synchronizing](
                    const BindingValue& value) {
                    const auto retained = weak.lock();
                    const auto* flags = std::get_if<PropertyEnumValue>(&value);
                    if (!retained || !flags) return;
                    *synchronizing = true;
                    retained->set_value(*flags);
                    *synchronizing = false;
                };
            binding.connect_committed =
                [weak = std::weak_ptr<FlagsValueEditor>(editor), synchronizing](
                    Component& owner,
                    std::function<void(BindingValue)> committed) {
                    const auto retained = weak.lock();
                    return retained
                        ? retained->value_changed().subscribe(
                              owner,
                              [synchronizing,
                               committed = std::move(committed)](
                                  const PropertyEnumValue& value) {
                                  if (!*synchronizing) {
                                      committed(BindingValue{value});
                                  }
                              })
                        : SubscriptionToken{};
                };
            return binding;
        }));
    static_cast<void>(result->register_factory(
        "color-value", [](const PropertyEditorRequest& request)
            -> std::optional<PropertyEditorBinding> {
            const auto* value = std::get_if<Color>(&request.value);
            if (!value) return {};
            auto editor = make_control<ColorValueEditor>(
                StableId(request.stable_id), *value);
            editor->set_enabled(request.writable);
            editor->set_accessible_name(request.property_path);
            editor->set_accessible_description(request.descriptor.description);
            if (editor->editor()) {
                editor->editor()->set_accessible_name(request.property_path);
                editor->editor()->set_accessible_description(
                    request.descriptor.description);
            }
            auto synchronizing = std::make_shared<bool>(false);
            PropertyEditorBinding binding;
            binding.control = editor;
            binding.synchronize =
                [weak = std::weak_ptr<ColorValueEditor>(editor), synchronizing](
                    const BindingValue& value) {
                    const auto retained = weak.lock();
                    const auto* color = std::get_if<Color>(&value);
                    if (!retained || !color) return;
                    *synchronizing = true;
                    retained->set_value(*color);
                    *synchronizing = false;
                };
            binding.connect_committed =
                [weak = std::weak_ptr<ColorValueEditor>(editor), synchronizing](
                    Component& owner,
                    std::function<void(BindingValue)> committed) {
                    const auto retained = weak.lock();
                    return retained
                        ? retained->value_changed().subscribe(
                              owner,
                              [synchronizing,
                               committed = std::move(committed)](Color value) {
                                  if (!*synchronizing) {
                                      committed(BindingValue{value});
                                  }
                              })
                        : SubscriptionToken{};
                };
            binding.connect_failed =
                [weak = std::weak_ptr<ColorValueEditor>(editor)](
                    Component& owner,
                    std::function<void(const PropertyEditorInputError&)> failed) {
                    const auto retained = weak.lock();
                    return retained
                        ? retained->edit_failed().subscribe(owner,
                              std::move(failed))
                        : SubscriptionToken{};
                };
            return binding;
        }));
    result->map_kind(BindingValueKind::number, "numeric-up-down");
    result->map_kind(BindingValueKind::enumeration, "flags-value");
    result->map_kind(BindingValueKind::color, "color-value");
    return result;
}

struct PropertyList::Impl final {
    struct RowState final {
        std::size_t group_index{};
        std::size_t row_index{};
        Control::Ptr editor;
        std::shared_ptr<Button> reset_button;
        SubscriptionToken value_subscription;
        SubscriptionToken commit_subscription;
        SubscriptionToken cancel_subscription;
        SubscriptionToken focus_subscription;
        SubscriptionToken reset_subscription;
        SubscriptionToken reset_focus_subscription;
        std::string committed_value;
        Rect row_bounds{};
        Rect name_bounds{};
        Rect value_bounds{};
        Rect reset_bounds{};
        Rect validation_bounds{};
    };

    explicit Impl(PropertyList& public_owner) : owner(public_owner) {}

    [[nodiscard]] double scale() const noexcept {
        return owner.effective_text_scale();
    }

    PropertyRowSpec& spec(RowState& state) {
        return groups[state.group_index].rows[state.row_index];
    }
    const PropertyRowSpec& spec(const RowState& state) const {
        return groups[state.group_index].rows[state.row_index];
    }

    RowState* find_row(std::string_view id) noexcept {
        const auto found = std::find_if(rows.begin(), rows.end(),
            [this, id](const RowState& row) { return spec(row).stable_id == id; });
        return found == rows.end() ? nullptr : &*found;
    }
    const RowState* find_row(std::string_view id) const noexcept {
        const auto found = std::find_if(rows.begin(), rows.end(),
            [this, id](const RowState& row) { return spec(row).stable_id == id; });
        return found == rows.end() ? nullptr : &*found;
    }

    [[nodiscard]] bool row_visible(const RowState& state) const noexcept {
        const PropertyRowSpec* current = &spec(state);
        std::size_t remaining = groups[state.group_index].rows.size();
        while (!current->parent_id.empty() && remaining-- > 0U) {
            const RowState* parent = find_row(current->parent_id);
            if (!parent || parent->group_index != state.group_index ||
                !spec(*parent).expandable || !spec(*parent).expanded) {
                return false;
            }
            current = &spec(*parent);
        }
        return current->parent_id.empty();
    }

    void clear_editors() noexcept {
        for (RowState& row : rows) {
            if (row.editor && row.editor->parent().get() == &owner) {
                Control::Ptr removed = owner.remove_child(row.editor->runtime_id());
                if (removed && removed->is_alive()) removed->dispose();
            }
            if (row.reset_button && row.reset_button->parent().get() == &owner) {
                Control::Ptr removed = owner.remove_child(
                    row.reset_button->runtime_id());
                if (removed && removed->is_alive()) removed->dispose();
            }
        }
        rows.clear();
    }

    void rebuild_editors() {
        clear_editors();
        for (std::size_t group_index = 0; group_index < groups.size(); ++group_index) {
            for (std::size_t row_index = 0;
                 row_index < groups[group_index].rows.size(); ++row_index) {
                PropertyRowSpec& row = groups[group_index].rows[row_index];
                RowState state;
                state.group_index = group_index;
                state.row_index = row_index;
                state.committed_value = row.value;
                if (row.editor == PropertyEditorKind::text) {
                    auto editor = make_control<TextBox>(
                        StableId(row.stable_id + ".editor"), row.value);
                    editor->set_font({FontRole::content, 10.0, 400, false});
                    editor->set_accessible_name(row.name);
                    editor->set_accessible_description(row.description);
                    editor->set_enabled(row.enabled);
                    state.editor = editor;
                    owner.add_child(editor);
                } else if (row.editor == PropertyEditorKind::choice) {
                    auto editor = make_control<ComboBox>(
                        StableId(row.stable_id + ".editor"));
                    editor->set_items(row.choices);
                    const auto selected = std::find(row.choices.begin(),
                                                    row.choices.end(), row.value);
                    if (selected != row.choices.end()) {
                        editor->set_selected_index(static_cast<std::size_t>(
                            std::distance(row.choices.begin(), selected)));
                    }
                    editor->set_font({FontRole::content, 10.0, 400, false});
                    editor->set_accessible_name(row.name);
                    editor->set_accessible_description(row.description);
                    editor->set_enabled(row.enabled);
                    state.editor = editor;
                    owner.add_child(editor);
                } else if (row.editor == PropertyEditorKind::boolean) {
                    const bool checked = binding_value_to_bool(
                        BindingValue{row.value}).value_or(false);
                    auto editor = make_control<CheckBox>(
                        StableId(row.stable_id + ".editor"),
                        checked ? "True" : "False");
                    editor->set_checked(checked);
                    editor->set_font({FontRole::content, 10.0, 400, false});
                    editor->set_accessible_name(row.name);
                    editor->set_accessible_description(row.description);
                    editor->set_enabled(row.enabled);
                    state.editor = editor;
                    owner.add_child(editor);
                }
                if (row.resettable) {
                    auto reset = make_control<Button>(
                        StableId(row.stable_id + ".reset"), "Reset");
                    reset->set_font({FontRole::control, 8.5, 600, false});
                    reset->set_enabled(row.enabled && row.reset_enabled);
                    reset->set_accessible_name("Reset " + row.name);
                    reset->set_accessible_description(
                        "Restore " + row.name + " to its declared default");
                    state.reset_button = reset;
                    owner.add_child(reset);
                }
                rows.push_back(std::move(state));
                connect_row(rows.back());
            }
        }
    }

    void connect_row(RowState& state) {
        const std::string row_id = spec(state).stable_id;
        if (const auto text = std::dynamic_pointer_cast<TextBox>(state.editor)) {
            state.value_subscription = text->text_changed().subscribe(
                owner, [this, row_id](const std::string& value) {
                    if (synchronizing) return;
                    RowState* row = find_row(row_id);
                    if (!row) return;
                    PropertyRowSpec& model = spec(*row);
                    const std::string previous = model.value;
                    model.value = value;
                    owner.publish_change(owner.value_changed_,
                        PropertyValueChange{
                            row_id, previous, model.value, false});
                });
            state.commit_subscription = text->committed().subscribe(
                owner, [this, row_id](const std::string& value) {
                    RowState* row = find_row(row_id);
                    if (!row) return;
                    PropertyRowSpec& model = spec(*row);
                    const std::string previous = row->committed_value;
                    row->committed_value = value;
                    model.value = value;
                    if (model.required && value.empty()) {
                        model.validation_message = model.name + " is required";
                    }
                    recompute_geometry();
                    owner.value_committed_.emit(
                        {row_id, previous, value, true});
                    owner.invalidate(Dirty::measure | Dirty::layout | Dirty::paint |
                                     Dirty::semantics);
                });
            state.cancel_subscription = text->cancelled().subscribe(
                owner, [this, row_id] {
                    RowState* row = find_row(row_id);
                    if (!row) return;
                    synchronizing = true;
                    if (const auto editor =
                            std::dynamic_pointer_cast<TextBox>(row->editor)) {
                        editor->set_text(row->committed_value);
                    }
                    spec(*row).value = row->committed_value;
                    synchronizing = false;
                    owner.invalidate(Dirty::paint | Dirty::semantics);
                });
        } else if (const auto choice =
                       std::dynamic_pointer_cast<ComboBox>(state.editor)) {
            state.value_subscription = choice->selected_index_changed().subscribe(
                owner, [this, row_id, weak_choice = std::weak_ptr<ComboBox>(choice)](
                    std::optional<std::size_t>) {
                    if (synchronizing) return;
                    RowState* row = find_row(row_id);
                    const auto editor = weak_choice.lock();
                    if (!row || !editor) return;
                    PropertyRowSpec& model = spec(*row);
                    const std::string previous = row->committed_value;
                    model.value = std::string(editor->selected_text());
                    row->committed_value = model.value;
                    PropertyValueChange change{row_id, previous, model.value, true};
                    owner.publish_change(owner.value_changed_, change);
                    owner.value_committed_.emit(change);
                });
        } else if (const auto check =
                       std::dynamic_pointer_cast<CheckBox>(state.editor)) {
            state.value_subscription = check->checked_changed().subscribe(
                owner, [this, row_id,
                        weak_check = std::weak_ptr<CheckBox>(check)](bool checked) {
                    if (synchronizing) return;
                    RowState* row = find_row(row_id);
                    const auto editor = weak_check.lock();
                    if (!row || !editor) return;
                    PropertyRowSpec& model = spec(*row);
                    const std::string previous = row->committed_value;
                    model.value = checked ? "True" : "False";
                    row->committed_value = model.value;
                    editor->set_text(model.value);
                    PropertyValueChange change{
                        row_id, previous, model.value, true};
                    owner.publish_change(owner.value_changed_, change);
                    owner.value_committed_.emit(change);
                });
        }
        if (state.editor) {
            state.focus_subscription = state.editor->focus_observed().subscribe(
                owner, [this, row_id](bool focused) {
                    if (focused) ensure_visible(row_id);
                });
        }
        if (state.reset_button) {
            state.reset_subscription = state.reset_button->clicked().subscribe(
                owner, [this, row_id](ButtonBase&) {
                    owner.reset_requested_.emit({row_id});
                });
            state.reset_focus_subscription =
                state.reset_button->focus_observed().subscribe(
                owner, [this, row_id](bool focused) {
                    if (focused) ensure_visible(row_id);
                });
        }
    }

    void recompute_geometry() {
        const double width = std::max(0.0, owner.committed_arranged_bounds().width);
        const double viewport = std::max(0.0, owner.committed_arranged_bounds().height);
        const double s = scale();
        const double padding = property_padding * s;
        const double gap = property_gap * s;
        const double scaled_label_width = label_width * s;
        const double validation_height = property_validation_height * s;
        const bool stacked = width < std::max(230.0 * s,
                                              scaled_label_width + 116.0 * s);
        double y{header_height};
        for (std::size_t group_index = 0; group_index < groups.size(); ++group_index) {
            y += property_group_height * s;
            if (!groups[group_index].expanded) continue;
            for (RowState& state : rows) {
                if (state.group_index != group_index) continue;
                const PropertyRowSpec& row = spec(state);
                if (!row_visible(state)) {
                    state.row_bounds = {};
                    state.name_bounds = {};
                    state.value_bounds = {};
                    state.reset_bounds = {};
                    state.validation_bounds = {};
                    continue;
                }
                const bool editable = row.editor != PropertyEditorKind::read_only;
                const bool interactive = editable || row.resettable;
                double height = (interactive ? property_editor_height
                                             : property_row_height) * s;
                if (stacked) height += (interactive ? 21.0 : 13.0) * s;
                if (!row.validation_message.empty()) height += validation_height;
                state.row_bounds = {0.0, y, width, height};
                const double indent = static_cast<double>(row.depth) * 15.0 * s +
                    (row.expandable ? 14.0 * s : 0.0);
                const double reset_width = row.resettable ? 48.0 * s : 0.0;
                const double reset_gap = row.resettable ? 5.0 * s : 0.0;
                if (stacked) {
                    state.name_bounds = {padding + indent, y + 3.0 * s,
                                         std::max(0.0, width - padding * 2.0 -
                                                           indent),
                                         18.0 * s};
                    state.value_bounds = {padding, y + 21.0 * s,
                        std::max(0.0, width - padding * 2.0 - reset_width -
                                          reset_gap),
                        (interactive ? 28.0 : 19.0) * s};
                } else {
                    state.name_bounds = {padding + indent, y + 5.0 * s,
                                         std::max(0.0, scaled_label_width - indent),
                                         21.0 * s};
                    state.value_bounds = {
                        padding + scaled_label_width + gap, y + 3.0 * s,
                        std::max(0.0, width - padding * 2.0 -
                                          scaled_label_width - gap - reset_width -
                                          reset_gap),
                        (interactive ? 28.0 : 22.0) * s};
                }
                state.reset_bounds = row.resettable
                    ? Rect{std::max(padding, width - padding - reset_width),
                           state.value_bounds.y, reset_width,
                           state.value_bounds.height}
                    : Rect{};
                state.validation_bounds = {
                    state.value_bounds.x,
                    y + height - validation_height,
                    state.value_bounds.width, validation_height};
                y += height;
            }
        }
        content_height = y;
        scroll_offset = std::clamp(scroll_offset, 0.0,
            std::max(0.0, content_height - viewport));
    }

    void arrange_editors() {
        if (header) {
            owner.set_child_layout(header,
                {0.0, -scroll_offset,
                 std::max(0.0, owner.committed_arranged_bounds().width),
                 header_height});
        }
        for (RowState& state : rows) {
            const bool visible = groups[state.group_index].expanded &&
                row_visible(state);
            if (state.editor) {
                state.editor->set_visible(visible);
            }
            if (state.reset_button) {
                state.reset_button->set_visible(visible);
            }
            if (visible && state.editor) {
                owner.set_child_layout(state.editor,
                    {state.value_bounds.x,
                     state.value_bounds.y - scroll_offset,
                     state.value_bounds.width, state.value_bounds.height});
            }
            if (visible && state.reset_button) {
                owner.set_child_layout(state.reset_button,
                    {state.reset_bounds.x,
                     state.reset_bounds.y - scroll_offset,
                     state.reset_bounds.width, state.reset_bounds.height});
            }
        }
    }

    void ensure_visible(std::string_view row_id) {
        RowState* row = find_row(row_id);
        if (!row) return;
        const double viewport = owner.committed_arranged_bounds().height;
        if (row->row_bounds.y < scroll_offset) {
            scroll_offset = row->row_bounds.y;
        } else if (row->row_bounds.y + row->row_bounds.height >
                   scroll_offset + viewport) {
            scroll_offset = row->row_bounds.y + row->row_bounds.height - viewport;
        }
        scroll_offset = std::clamp(scroll_offset, 0.0,
            std::max(0.0, content_height - viewport));
        arrange_editors();
        owner.invalidate(Dirty::paint | Dirty::hit_test | Dirty::semantics);
    }

    std::optional<std::size_t> group_at(Point absolute) const noexcept {
        const Rect bounds = owner.absolute_bounds();
        if (!bounds.contains(absolute)) return {};
        const double y_target = absolute.y - bounds.y + scroll_offset;
        const double group_height = property_group_height * scale();
        double y{header_height};
        for (std::size_t group_index = 0; group_index < groups.size(); ++group_index) {
            if (y_target >= y && y_target < y + group_height) {
                return group_index;
            }
            y += group_height;
            if (!groups[group_index].expanded) continue;
            for (const RowState& state : rows) {
                if (state.group_index == group_index) y += state.row_bounds.height;
            }
        }
        return {};
    }

    RowState* disclosure_at(Point absolute) noexcept {
        const Rect bounds = owner.absolute_bounds();
        if (!bounds.contains(absolute)) return nullptr;
        const Point local{absolute.x - bounds.x,
                          absolute.y - bounds.y + scroll_offset};
        for (RowState& state : rows) {
            const PropertyRowSpec& row = spec(state);
            if (!groups[state.group_index].expanded || !row_visible(state) ||
                !row.expandable || !state.row_bounds.contains(local)) {
                continue;
            }
            const double disclosure_right = state.name_bounds.x;
            const double disclosure_left = std::max(
                0.0, disclosure_right - 16.0 * scale());
            if (local.x >= disclosure_left &&
                local.x <= disclosure_right + state.name_bounds.width) {
                return &state;
            }
        }
        return nullptr;
    }

    PropertyList& owner;
    std::vector<PropertyGroupSpec> groups;
    std::vector<RowState> rows;
    Control::Ptr header;
    double header_height{};
    double label_width{76.0};
    double scroll_offset{};
    double content_height{};
    bool synchronizing{};
};

PropertyList::PropertyList(StableId stable_id)
    : Panel(std::move(stable_id)), impl_(std::make_unique<Impl>(*this)) {
    set_background(Color::rgba(244, 247, 251));
    set_border_style(BorderStyle::none);
    set_focusable(true);
    set_paint_plane(PaintPlane::control);
}

PropertyList::~PropertyList() = default;

const std::vector<PropertyGroupSpec>& PropertyList::groups() const noexcept {
    return impl_->groups;
}

void PropertyList::set_groups(std::vector<PropertyGroupSpec> groups) {
    require_mutable();
    std::unordered_set<std::string> identities;
    for (const auto& group : groups) {
        require_property_text(group.stable_id, "group ID");
        require_property_text(group.title, "group title");
        if (group.stable_id.empty() || group.title.empty() ||
            !identities.insert(group.stable_id).second) {
            throw std::invalid_argument(
                "PropertyList groups require unique nonempty identities and titles");
        }
        std::unordered_map<std::string, const PropertyRowSpec*> prior_rows;
        for (const auto& row : group.rows) {
            require_property_text(row.stable_id, "row ID");
            require_property_text(row.name, "row name");
            require_property_text(row.value, "row value");
            require_property_text(row.description, "row description");
            require_property_text(row.validation_message, "validation message");
            if (row.stable_id.empty() || row.name.empty() ||
                !identities.insert(row.stable_id).second) {
                throw std::invalid_argument(
                    "PropertyList rows require unique nonempty identities and names");
            }
            if (row.depth > 8U ||
                (row.expanded && !row.expandable) ||
                (row.reset_enabled && !row.resettable)) {
                throw std::invalid_argument(
                    "PropertyList row hierarchy/reset state is inconsistent");
            }
            if (row.parent_id.empty()) {
                if (row.depth != 0U) {
                    throw std::invalid_argument(
                        "PropertyList root rows must have depth zero");
                }
            } else {
                const auto parent = prior_rows.find(row.parent_id);
                if (parent == prior_rows.end() ||
                    !parent->second->expandable ||
                    row.depth != parent->second->depth + 1U) {
                    throw std::invalid_argument(
                        "PropertyList child rows require an earlier expandable parent at the preceding depth");
                }
            }
            if (row.editor == PropertyEditorKind::choice && row.choices.empty()) {
                throw std::invalid_argument(
                    "PropertyList choice rows require at least one choice");
            }
            for (const auto& choice : row.choices) {
                require_property_text(choice, "choice");
            }
            if (row.editor == PropertyEditorKind::choice && !row.value.empty() &&
                std::find(row.choices.begin(), row.choices.end(), row.value) ==
                    row.choices.end()) {
                throw std::invalid_argument(
                    "PropertyList choice value must be empty or one declared choice");
            }
            if (row.editor == PropertyEditorKind::boolean &&
                !binding_value_to_bool(BindingValue{row.value})) {
                throw std::invalid_argument(
                    "PropertyList Boolean value must be True, False, 1, or 0");
            }
            prior_rows.emplace(row.stable_id, &row);
        }
    }
    impl_->groups = std::move(groups);
    impl_->scroll_offset = 0.0;
    impl_->rebuild_editors();
    impl_->recompute_geometry();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
}

bool PropertyList::set_value(std::string_view row_id, std::string value) {
    require_mutable();
    require_property_text(value, "row value");
    Impl::RowState* row = impl_->find_row(row_id);
    if (!row) return false;
    PropertyRowSpec& model = impl_->spec(*row);
    if (model.editor == PropertyEditorKind::choice && !value.empty() &&
        std::find(model.choices.begin(), model.choices.end(), value) ==
            model.choices.end()) {
        throw std::invalid_argument(
            "PropertyList choice value must be empty or one declared choice");
    }
    if (model.editor == PropertyEditorKind::boolean &&
        !binding_value_to_bool(BindingValue{value})) {
        throw std::invalid_argument(
            "PropertyList Boolean value must be True, False, 1, or 0");
    }
    if (model.value == value && row->committed_value == value) return true;
    impl_->synchronizing = true;
    model.value = value;
    row->committed_value = value;
    if (const auto text = std::dynamic_pointer_cast<TextBox>(row->editor)) {
        text->set_text(value);
    } else if (const auto choice = std::dynamic_pointer_cast<ComboBox>(row->editor)) {
        const auto found = std::find(model.choices.begin(), model.choices.end(), value);
        choice->set_selected_index(found == model.choices.end()
            ? std::optional<std::size_t>{}
            : std::optional<std::size_t>{static_cast<std::size_t>(
                std::distance(model.choices.begin(), found))});
    } else if (const auto check =
                   std::dynamic_pointer_cast<CheckBox>(row->editor)) {
        const bool checked = binding_value_to_bool(
            BindingValue{value}).value_or(false);
        check->set_checked(checked);
        check->set_text(checked ? "True" : "False");
    }
    impl_->synchronizing = false;
    invalidate(Dirty::paint | Dirty::semantics);
    return true;
}

bool PropertyList::set_description(std::string_view row_id,
                                   std::string description) {
    require_mutable();
    require_property_text(description, "row description");
    Impl::RowState* row = impl_->find_row(row_id);
    if (!row) return false;
    PropertyRowSpec& model = impl_->spec(*row);
    if (model.description == description) return true;
    model.description = std::move(description);
    if (row->editor) {
        std::string accessible = model.description;
        if (!model.validation_message.empty()) {
            if (!accessible.empty()) accessible += " · ";
            accessible += "Error: " + model.validation_message;
        }
        row->editor->set_accessible_description(std::move(accessible));
    }
    invalidate(Dirty::paint | Dirty::semantics | Dirty::accessibility);
    return true;
}

bool PropertyList::set_validation(std::string_view row_id, std::string message) {
    require_mutable();
    require_property_text(message, "validation message");
    Impl::RowState* row = impl_->find_row(row_id);
    if (!row) return false;
    PropertyRowSpec& model = impl_->spec(*row);
    if (model.validation_message == message) return true;
    model.validation_message = std::move(message);
    if (row->editor) {
        std::string description = model.description;
        if (!model.validation_message.empty()) {
            if (!description.empty()) description += " · ";
            description += "Error: " + model.validation_message;
        }
        row->editor->set_accessible_description(std::move(description));
    }
    impl_->recompute_geometry();
    impl_->arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
    return true;
}

bool PropertyList::set_reset_enabled(std::string_view row_id, bool enabled) {
    require_mutable();
    Impl::RowState* row = impl_->find_row(row_id);
    if (!row || !impl_->spec(*row).resettable || !row->reset_button) return false;
    PropertyRowSpec& model = impl_->spec(*row);
    if (model.reset_enabled == enabled) return true;
    model.reset_enabled = enabled;
    row->reset_button->set_enabled(model.enabled && enabled);
    invalidate(Dirty::paint | Dirty::semantics | Dirty::accessibility);
    return true;
}

bool PropertyList::set_group_expanded(std::string_view id, bool expanded) {
    require_mutable();
    const auto found = std::find_if(impl_->groups.begin(), impl_->groups.end(),
        [id](const PropertyGroupSpec& group) { return group.stable_id == id; });
    if (found == impl_->groups.end()) return false;
    if (found->expanded == expanded) return true;
    found->expanded = expanded;
    impl_->recompute_geometry();
    impl_->arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
    publish_change(group_changed_,
                   PropertyGroupChange{found->stable_id, expanded});
    return true;
}

bool PropertyList::set_row_expanded(std::string_view id, bool expanded) {
    require_mutable();
    Impl::RowState* row = impl_->find_row(id);
    if (!row || !impl_->spec(*row).expandable) return false;
    PropertyRowSpec& model = impl_->spec(*row);
    if (model.expanded == expanded) return true;
    model.expanded = expanded;
    impl_->recompute_geometry();
    impl_->arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
    publish_change(row_expansion_changed_,
                   PropertyRowExpansionChange{model.stable_id, expanded});
    return true;
}

std::optional<bool> PropertyList::row_expanded(std::string_view id) const {
    const Impl::RowState* row = impl_->find_row(id);
    return row && impl_->spec(*row).expandable
        ? std::optional<bool>{impl_->spec(*row).expanded}
        : std::optional<bool>{};
}

std::optional<std::string> PropertyList::value(std::string_view id) const {
    const Impl::RowState* row = impl_->find_row(id);
    return row ? std::optional<std::string>{impl_->spec(*row).value}
               : std::optional<std::string>{};
}

Control::Ptr PropertyList::editor(std::string_view id) const {
    const Impl::RowState* row = impl_->find_row(id);
    return row ? row->editor : Control::Ptr{};
}

bool PropertyList::replace_editor(std::string_view id, Control::Ptr editor) {
    require_mutable();
    Impl::RowState* row = impl_->find_row(id);
    if (!row) return false;
    if (!editor || !editor->is_alive() || editor->parent() ||
        editor->attached_window()) {
        throw std::invalid_argument(
            "PropertyList replacement editor must be an unattached live control");
    }
    PropertyRowSpec& model = impl_->spec(*row);
    if (!model.enabled) editor->set_enabled(false);
    if (editor->accessible_name().empty()) {
        editor->set_accessible_name(model.name);
    }
    if (editor->accessible_description().empty()) {
        editor->set_accessible_description(model.description);
    }

    // Attach first so an identity/lifecycle failure leaves the existing editor
    // and its subscriptions intact.
    add_child(editor);
    Control::Ptr previous = row->editor;
    row->value_subscription.disconnect();
    row->commit_subscription.disconnect();
    row->cancel_subscription.disconnect();
    row->focus_subscription.disconnect();
    row->editor = editor;
    model.editor = PropertyEditorKind::custom;
    row->focus_subscription = editor->focus_observed().subscribe(
        *this, [implementation = impl_.get(), row_id = std::string(id)](
                   bool focused) {
            if (focused) implementation->ensure_visible(row_id);
        });
    if (previous && previous->parent().get() == this) {
        Control::Ptr removed = remove_child(previous->runtime_id());
        if (removed && removed->is_alive()) removed->dispose();
    }
    impl_->recompute_geometry();
    impl_->arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
    return true;
}

std::shared_ptr<Button> PropertyList::reset_button(std::string_view id) const {
    const Impl::RowState* row = impl_->find_row(id);
    return row ? row->reset_button : std::shared_ptr<Button>{};
}

void PropertyList::set_header_content(Control::Ptr content, double height) {
    require_mutable();
    if (!std::isfinite(height) || height < 0.0 || height > 4096.0 ||
        (!content && height != 0.0)) {
        throw std::invalid_argument(
            "PropertyList header requires content and a 0 through 4096 height");
    }
    if (impl_->header == content && impl_->header_height == height) return;
    if (impl_->header && impl_->header->parent().get() == this) {
        Control::Ptr previous = remove_child(impl_->header->runtime_id());
        if (previous && previous->is_alive()) previous->dispose();
    }
    impl_->header = std::move(content);
    impl_->header_height = height;
    if (impl_->header) add_child(impl_->header);
    impl_->recompute_geometry();
    impl_->arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
}

Control::Ptr PropertyList::header_content() const noexcept {
    return impl_->header;
}

double PropertyList::header_height() const noexcept { return impl_->header_height; }

void PropertyList::set_header_height(double height) {
    require_mutable();
    if (!std::isfinite(height) || height < 0.0 || height > 4096.0 ||
        (!impl_->header && height != 0.0)) {
        throw std::invalid_argument("PropertyList header height must be 0 through 4096");
    }
    if (impl_->header_height == height) return;
    impl_->header_height = height;
    impl_->recompute_geometry();
    impl_->arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
}

double PropertyList::label_width() const noexcept { return impl_->label_width; }

void PropertyList::set_label_width(double width) {
    require_mutable();
    if (!std::isfinite(width) || width < 40.0 || width > 320.0) {
        throw std::invalid_argument("PropertyList label width must be 40 through 320");
    }
    if (impl_->label_width == width) return;
    impl_->label_width = width;
    impl_->recompute_geometry();
    impl_->arrange_editors();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
}

double PropertyList::scroll_offset() const noexcept { return impl_->scroll_offset; }

void PropertyList::set_scroll_offset(double offset) {
    require_mutable();
    if (!std::isfinite(offset)) {
        throw std::invalid_argument("PropertyList scroll offset must be finite");
    }
    const double maximum = std::max(0.0, impl_->content_height -
                                          committed_arranged_bounds().height);
    const double next = std::clamp(offset, 0.0, maximum);
    if (next == impl_->scroll_offset) return;
    impl_->scroll_offset = next;
    impl_->arrange_editors();
    invalidate(Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

double PropertyList::content_height() const noexcept { return impl_->content_height; }

Size PropertyList::measure(Size available) {
    return {available.width, std::min(available.height,
        std::max(property_group_height * effective_text_scale(),
                 impl_->content_height))};
}

void PropertyList::arrange(Rect final_bounds) {
    arrange_self(final_bounds);
    impl_->recompute_geometry();
    impl_->arrange_editors();
}

void PropertyList::on_paint(Painter& painter, Rect) {
    const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                      committed_arranged_bounds().height};
    const BasicControlStyle style;
    const double s = effective_text_scale();
    const double group_height = property_group_height * s;
    const FontSpec group_glyph = effective_font(
        {FontRole::control, 8.5, 600, false});
    const FontSpec group_font = effective_font(
        {FontRole::control, 8.5, 700, false, 0.24});
    const FontSpec name_font = effective_font(
        {FontRole::content, 9.5, 600, false});
    const FontSpec value_font = effective_font(
        {FontRole::content, 9.5, 400, false});
    const FontSpec validation_font = effective_font(
        {FontRole::content, 8.5, 600, false});
    painter.fill_rect(bounds, background());
    double y = impl_->header_height - impl_->scroll_offset;
    for (std::size_t group_index = 0;
         group_index < impl_->groups.size(); ++group_index) {
        const PropertyGroupSpec& group = impl_->groups[group_index];
        painter.fill_rect({0.0, y, bounds.width, group_height},
                          Color::rgba(216, 224, 236));
        painter.draw_line({0.0, y + group_height - 1.0},
                          {bounds.width, y + group_height - 1.0},
                          style.border, 1.0);
        const double group_baseline = y + std::max(group_font.size,
                                                    group_height * 0.5 +
                                                        group_font.size * 0.35);
        painter.draw_text_utf8({7.0 * s, group_baseline},
                               group.expanded ? "▼" : "▶",
                               group_glyph, style.text);
        painter.draw_text_utf8({22.0 * s, group_baseline}, group.title,
                               group_font,
                               style.text);
        y += group_height;
        if (!group.expanded) continue;
        for (const Impl::RowState& row_state : impl_->rows) {
            if (row_state.group_index != group_index) continue;
            const PropertyRowSpec& row = impl_->spec(row_state);
            if (!impl_->row_visible(row_state)) continue;
            const Rect name{row_state.name_bounds.x,
                            row_state.name_bounds.y - impl_->scroll_offset,
                            row_state.name_bounds.width,
                            row_state.name_bounds.height};
            const Rect value_bounds{row_state.value_bounds.x,
                                    row_state.value_bounds.y - impl_->scroll_offset,
                                    row_state.value_bounds.width,
                                    row_state.value_bounds.height};
            if (row.expandable) {
                painter.draw_text_utf8({std::max(2.0 * s, name.x - 13.0 * s),
                                        name.y + 15.0 * s},
                    row.expanded ? "▼" : "▶", group_glyph, style.text);
            }
            painter.draw_text_utf8({name.x, name.y + 15.0 * s}, row.name,
                                   name_font, style.disabled_text);
            if (row.editor == PropertyEditorKind::read_only) {
                painter.draw_text_utf8({value_bounds.x,
                                        value_bounds.y + 15.0 * s},
                    row.value, value_font,
                    row.enabled ? style.text : style.disabled_text);
            }
            if (!row.validation_message.empty()) {
                const Rect validation{row_state.validation_bounds.x,
                    row_state.validation_bounds.y - impl_->scroll_offset,
                    row_state.validation_bounds.width,
                    row_state.validation_bounds.height};
                painter.draw_text_utf8({validation.x,
                                        validation.y + 13.0 * s},
                    "! " + row.validation_message,
                    validation_font,
                    Color::rgba(151, 45, 43));
            }
            painter.draw_line({property_padding * s,
                               row_state.row_bounds.y + row_state.row_bounds.height -
                                   impl_->scroll_offset - 1.0},
                              {std::max(property_padding * s,
                                        bounds.width - property_padding * s),
                               row_state.row_bounds.y + row_state.row_bounds.height -
                                   impl_->scroll_offset - 1.0},
                              Color::rgba(211, 219, 229), 1.0);
            y += row_state.row_bounds.height;
        }
    }
    if (impl_->content_height > bounds.height && bounds.height > 12.0) {
        const double ratio = bounds.height / impl_->content_height;
        const double thumb_height = std::max(18.0, bounds.height * ratio);
        const double maximum = impl_->content_height - bounds.height;
        const double thumb_y = maximum <= 0.0 ? 0.0 :
            (bounds.height - thumb_height) * impl_->scroll_offset / maximum;
        painter.fill_rect({std::max(0.0, bounds.width - 4.0), thumb_y,
                           3.0, thumb_height}, style.border);
    }
}

void PropertyList::on_pointer(PointerEvent& event) {
    if (event.action == PointerAction::wheel && event.wheel_delta.y != 0.0) {
        set_scroll_offset(impl_->scroll_offset +
            (event.wheel_delta.y > 0.0 ? -54.0 : 54.0) *
                effective_text_scale());
        event.handled = true;
        return;
    }
    if (event.action == PointerAction::down &&
        event.button == PointerButton::primary) {
        if (Impl::RowState* row = impl_->disclosure_at(event.position)) {
            const PropertyRowSpec& model = impl_->spec(*row);
            static_cast<void>(set_row_expanded(model.stable_id,
                                               !model.expanded));
            event.handled = true;
            return;
        }
        const auto group = impl_->group_at(event.position);
        if (group) {
            static_cast<void>(set_group_expanded(
                impl_->groups[*group].stable_id,
                !impl_->groups[*group].expanded));
            event.handled = true;
        }
    }
}

void PropertyList::on_key(KeyEvent& event) {
    if (event.action != KeyAction::down) return;
    double next = impl_->scroll_offset;
    if (event.physical_key == PhysicalKey::home) next = 0.0;
    else if (event.physical_key == PhysicalKey::end) next = impl_->content_height;
    else if (event.physical_key == PhysicalKey::page_up) {
        next -= committed_arranged_bounds().height;
    } else if (event.physical_key == PhysicalKey::page_down) {
        next += committed_arranged_bounds().height;
    } else return;
    set_scroll_offset(next);
    event.handled = true;
}

SemanticDescriptor PropertyList::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::property_grid;
    descriptor.name = accessible_name().empty() ? "Properties" : accessible_name();
    descriptor.description = accessible_description();
    descriptor.actions = {SemanticAction::focus};
    descriptor.exposed = true;
    descriptor.include_descendants = true;
    return descriptor;
}

std::vector<SemanticNode> PropertyList::semantic_virtual_children() const {
    std::vector<SemanticNode> nodes;
    const Rect absolute = absolute_bounds();
    const double group_height = property_group_height * effective_text_scale();
    double y = impl_->header_height - impl_->scroll_offset;
    for (std::size_t group_index = 0;
         group_index < impl_->groups.size(); ++group_index) {
        const PropertyGroupSpec& group = impl_->groups[group_index];
        SemanticNode header;
        header.stable_id = group.stable_id;
        header.runtime_id = property_virtual_runtime_id(header.stable_id);
        header.role = SemanticRole::property_group;
        header.name = group.title;
        header.bounds = {absolute.x, absolute.y + y, absolute.width,
                         group_height};
        header.states = SemanticState::visible | SemanticState::focusable |
                        SemanticState::enabled;
        if (group.expanded) header.states |= SemanticState::expanded;
        header.actions = {SemanticAction::focus,
            group.expanded ? SemanticAction::collapse : SemanticAction::expand};
        nodes.push_back(std::move(header));
        y += group_height;
        if (!group.expanded) continue;
        for (const Impl::RowState& row_state : impl_->rows) {
            if (row_state.group_index != group_index) continue;
            const PropertyRowSpec& row = impl_->spec(row_state);
            if (!impl_->row_visible(row_state)) continue;
            if (row.editor == PropertyEditorKind::read_only) {
                SemanticNode node;
                node.stable_id = row.stable_id;
                node.runtime_id = property_virtual_runtime_id(node.stable_id);
                node.role = SemanticRole::property_row;
                node.name = row.name;
                node.value = row.value;
                node.description = row.validation_message.empty()
                    ? row.description : row.description + " · Error: " +
                                            row.validation_message;
                node.bounds = {absolute.x,
                    absolute.y + row_state.row_bounds.y - impl_->scroll_offset,
                    absolute.width, row_state.row_bounds.height};
                node.states = SemanticState::visible;
                if (row.enabled) node.states |= SemanticState::enabled;
                if (row.expandable) {
                    if (row.expanded) node.states |= SemanticState::expanded;
                    node.actions.push_back(row.expanded
                        ? SemanticAction::collapse : SemanticAction::expand);
                }
                nodes.push_back(std::move(node));
            }
            y += row_state.row_bounds.height;
        }
    }
    return nodes;
}

bool PropertyList::on_semantic_child_action(std::string_view id,
                                            SemanticAction action,
                                            std::string_view) {
    const auto found = std::find_if(impl_->groups.begin(), impl_->groups.end(),
        [id](const PropertyGroupSpec& group) { return group.stable_id == id; });
    if (found != impl_->groups.end()) {
        if (action == SemanticAction::focus) {
            if (window()) {
                static_cast<void>(window()->request_focus(shared_from_this()));
            }
            return true;
        }
        if (action == SemanticAction::expand ||
            action == SemanticAction::collapse) {
            return set_group_expanded(id, action == SemanticAction::expand);
        }
        return false;
    }
    const Impl::RowState* row = impl_->find_row(id);
    if (row && impl_->spec(*row).expandable &&
        (action == SemanticAction::expand ||
         action == SemanticAction::collapse)) {
        return set_row_expanded(id, action == SemanticAction::expand);
    }
    return false;
}

void PropertyList::on_dispose() noexcept {
    // Component::dispose marks the control disposing before entering this
    // callback. Control::on_dispose owns the disposal-safe child detach path.
    impl_->rows.clear();
    impl_->header.reset();
    Control::on_dispose();
}

struct PropertyGrid::Impl final {
    explicit Impl(PropertyGrid& public_owner)
        : owner(public_owner),
          converters(PropertyValueConverterRegistry::create_default()),
          editors(PropertyEditorRegistry::create_default()) {}

    enum class PathSegmentKind : std::uint8_t {
        compound_field,
        object_member,
        collection_index,
    };

    struct PathSegment final {
        PathSegmentKind kind{PathSegmentKind::compound_field};
        std::string name;
        std::size_t index{};
    };

    struct EditTarget final {
        std::string property_name;
        std::vector<PathSegment> path;
        bool writable{true};
        PropertyDescriptor descriptor;
    };

    struct InstalledEditor final {
        PropertyEditorBinding binding;
        SubscriptionToken commit;
        SubscriptionToken failure;
    };

    [[nodiscard]] static bool expandable_value(const BindingValue& value) {
        if (!compound_fields(binding_value_kind(value)).empty()) return true;
        if (const auto* object = std::get_if<PropertyObjectValue>(&value)) {
            return *object && !object->members().empty();
        }
        if (const auto* collection =
                std::get_if<PropertyCollectionValue>(&value)) {
            return *collection && !property_collection_items(*collection).empty();
        }
        return false;
    }

    [[nodiscard]] static std::string append_member_path(
        std::string_view parent, std::string_view member) {
        return std::string(parent) + "." + std::string(member);
    }

    [[nodiscard]] static std::string append_index_path(
        std::string_view parent, std::size_t index) {
        return std::string(parent) + "[" + std::to_string(index) + "]";
    }

    [[nodiscard]] static std::optional<BindingValue> value_at(
        BindingValue value, std::span<const PathSegment> path) {
        for (const PathSegment& segment : path) {
            if (segment.kind == PathSegmentKind::compound_field) {
                const auto child = compound_field_value(value, segment.name);
                if (!child) return {};
                value = *child;
                continue;
            }
            if (segment.kind == PathSegmentKind::object_member) {
                const auto* object = std::get_if<PropertyObjectValue>(&value);
                if (!object || !*object) return {};
                const std::string wanted = canonical_binding_name(segment.name);
                const auto member = std::find_if(
                    object->members().begin(), object->members().end(),
                    [&wanted](const PropertyObjectMember& candidate) {
                        return canonical_binding_name(candidate.name) == wanted;
                    });
                if (member == object->members().end()) return {};
                value = member->value;
                continue;
            }
            const auto* collection = std::get_if<PropertyCollectionValue>(&value);
            if (!collection || !*collection) return {};
            const auto items = property_collection_items(*collection);
            if (segment.index >= items.size()) return {};
            value = items[segment.index];
        }
        return value;
    }

    [[nodiscard]] static std::optional<BindingValue> replace_at(
        const BindingValue& value, std::span<const PathSegment> path,
        const BindingValue& replacement) {
        if (path.empty()) {
            return valid_property_value_tree(replacement)
                ? std::optional<BindingValue>{replacement}
                : std::optional<BindingValue>{};
        }
        const PathSegment& segment = path.front();
        const auto remainder = path.subspan(1U);
        if (segment.kind == PathSegmentKind::compound_field) {
            if (!remainder.empty()) return {};
            return replace_compound_field(value, segment.name, replacement);
        }
        if (segment.kind == PathSegmentKind::object_member) {
            const auto* object = std::get_if<PropertyObjectValue>(&value);
            if (!object || !*object) return {};
            std::vector<PropertyObjectMember> members(
                object->members().begin(), object->members().end());
            const std::string wanted = canonical_binding_name(segment.name);
            const auto member = std::find_if(
                members.begin(), members.end(),
                [&wanted](const PropertyObjectMember& candidate) {
                    return canonical_binding_name(candidate.name) == wanted;
                });
            if (member == members.end() || !member->writable) return {};
            const auto child = replace_at(member->value, remainder, replacement);
            if (!child) return {};
            member->value = *child;
            return BindingValue{make_property_object(
                std::string(object->type_name()), std::move(members))};
        }
        const auto* collection = std::get_if<PropertyCollectionValue>(&value);
        if (!collection || !*collection) return {};
        const auto current = property_collection_items(*collection);
        if (segment.index >= current.size()) return {};
        std::vector<BindingValue> items(current.begin(), current.end());
        const auto child = replace_at(items[segment.index], remainder, replacement);
        if (!child) return {};
        items[segment.index] = *child;
        return BindingValue{make_property_collection(
            std::string(collection->item_type_name()), collection->item_kind(),
            std::move(items))};
    }

    [[nodiscard]] static bool same_shape(const BindingValue& left,
                                         const BindingValue& right) {
        if (binding_value_kind(left) != binding_value_kind(right)) return false;
        const auto* left_object = std::get_if<PropertyObjectValue>(&left);
        const auto* right_object = std::get_if<PropertyObjectValue>(&right);
        if (left_object || right_object) {
            if (!left_object || !right_object || !*left_object || !*right_object ||
                left_object->type_name() != right_object->type_name() ||
                left_object->members().size() != right_object->members().size()) {
                return false;
            }
            for (std::size_t index = 0U;
                 index < left_object->members().size(); ++index) {
                const PropertyObjectMember& left_member =
                    left_object->members()[index];
                const PropertyObjectMember& right_member =
                    right_object->members()[index];
                if (canonical_binding_name(left_member.name) !=
                        canonical_binding_name(right_member.name) ||
                    left_member.description != right_member.description ||
                    left_member.writable != right_member.writable ||
                    left_member.declared_kind != right_member.declared_kind ||
                    left_member.nullable != right_member.nullable ||
                    left_member.enumeration != right_member.enumeration ||
                    left_member.standard_values != right_member.standard_values ||
                    left_member.standard_values_exclusive !=
                        right_member.standard_values_exclusive ||
                    left_member.converter_name != right_member.converter_name ||
                    left_member.editor_name != right_member.editor_name ||
                    !same_shape(left_member.value, right_member.value)) {
                    return false;
                }
            }
            return true;
        }
        const auto* left_collection =
            std::get_if<PropertyCollectionValue>(&left);
        const auto* right_collection =
            std::get_if<PropertyCollectionValue>(&right);
        if (left_collection || right_collection) {
            if (!left_collection || !right_collection || !*left_collection ||
                !*right_collection ||
                left_collection->item_type_name() !=
                    right_collection->item_type_name() ||
                left_collection->item_kind() != right_collection->item_kind()) {
                return false;
            }
            const auto left_items = property_collection_items(*left_collection);
            const auto right_items = property_collection_items(*right_collection);
            if (left_items.size() != right_items.size()) return false;
            for (std::size_t index = 0U; index < left_items.size(); ++index) {
                if (!same_shape(left_items[index], right_items[index])) return false;
            }
        }
        return true;
    }

    [[nodiscard]] static bool compatible_descriptor(
        const PropertyDescriptor& left,
        const PropertyDescriptor& right) noexcept {
        const bool enum_equal = (!left.enumeration && !right.enumeration) ||
            (left.enumeration && right.enumeration &&
             *left.enumeration == *right.enumeration);
        return left.kind == right.kind && left.nullable == right.nullable &&
            enum_equal && left.standard_values == right.standard_values &&
            left.standard_values_exclusive ==
                right.standard_values_exclusive &&
            left.converter_name == right.converter_name &&
            left.editor_name == right.editor_name &&
            left.readable == right.readable && left.browsable == right.browsable;
    }

    [[nodiscard]] std::string row_id(std::string_view property_name) const {
        return std::string(owner.stable_id().value()) + ".property." +
               canonical_binding_name(property_name);
    }

    [[nodiscard]] std::string group_id(std::string_view category) const {
        return std::string(owner.stable_id().value()) + ".category." +
               canonical_binding_name(category);
    }

    [[nodiscard]] std::string display_value(
        const BindingValue& value, const PropertyDescriptor& descriptor,
        bool) const {
        if (converters) return converters->format(value, descriptor);
        if (const auto* boolean = std::get_if<bool>(&value)) {
            return *boolean ? "True" : "False";
        }
        if (const auto* image = std::get_if<ImageId>(&value)) {
            return std::to_string(image->value);
        }
        return binding_value_to_string(value);
    }

    [[nodiscard]] static PropertyEditorKind editor_kind(
        const PropertyDescriptor& descriptor) noexcept {
        if (!descriptor.writable) return PropertyEditorKind::read_only;
        if (descriptor.nullable &&
            !descriptor.standard_values_exclusive) {
            return PropertyEditorKind::text;
        }
        switch (descriptor.kind) {
        case BindingValueKind::boolean:
            return PropertyEditorKind::boolean;
        case BindingValueKind::signed_integer:
        case BindingValueKind::unsigned_integer:
        case BindingValueKind::number:
        case BindingValueKind::text:
            return PropertyEditorKind::text;
        case BindingValueKind::enumeration:
            return descriptor.enumeration && !descriptor.enumeration->flags
                ? PropertyEditorKind::choice : PropertyEditorKind::text;
        case BindingValueKind::null:
        case BindingValueKind::point:
        case BindingValueKind::size:
        case BindingValueKind::rectangle:
        case BindingValueKind::insets:
        case BindingValueKind::color:
        case BindingValueKind::font:
        case BindingValueKind::image:
        case BindingValueKind::object:
        case BindingValueKind::collection:
            return PropertyEditorKind::read_only;
        }
        return PropertyEditorKind::read_only;
    }

    [[nodiscard]] static PropertyEditorKind editor_kind(
        BindingValueKind kind, bool writable) noexcept {
        if (!writable) return PropertyEditorKind::read_only;
        switch (kind) {
        case BindingValueKind::boolean:
            return PropertyEditorKind::boolean;
        case BindingValueKind::signed_integer:
        case BindingValueKind::unsigned_integer:
        case BindingValueKind::number:
        case BindingValueKind::text:
            return PropertyEditorKind::text;
        case BindingValueKind::null:
        case BindingValueKind::point:
        case BindingValueKind::size:
        case BindingValueKind::rectangle:
        case BindingValueKind::insets:
        case BindingValueKind::color:
        case BindingValueKind::font:
        case BindingValueKind::image:
        case BindingValueKind::enumeration:
        case BindingValueKind::object:
        case BindingValueKind::collection:
            return PropertyEditorKind::read_only;
        }
        return PropertyEditorKind::read_only;
    }

    [[nodiscard]] std::string description_for(
        const PropertyDescriptor& descriptor,
        PropertyValueOrigin origin) const {
        std::string result = descriptor.description;
        if (!result.empty()) result += " · ";
        result += "Origin: ";
        result += property_value_origin_name(origin);
        if (descriptor.resettable) result += " · Reset available";
        return result;
    }

    void clear_subscriptions() noexcept {
        property_subscriptions.clear();
    }

    [[nodiscard]] Control::Ptr target() const noexcept {
        if (selected.empty()) return {};
        const Control::Ptr result = selected.front().lock();
        return result && result->is_alive() ? result : Control::Ptr{};
    }

    [[nodiscard]] std::vector<Control::Ptr> targets() const {
        std::vector<Control::Ptr> result;
        result.reserve(selected.size());
        for (const Control::WeakPtr& candidate : selected) {
            Control::Ptr retained = candidate.lock();
            if (!retained || !retained->is_alive()) return {};
            result.push_back(std::move(retained));
        }
        return result;
    }

    void append_projected_rows(
        std::vector<PropertyRowSpec>& rows,
        const PropertyDescriptor& descriptor,
        const BindingValue& value,
        std::string path,
        std::string display_name,
        std::string description,
        std::string parent_id,
        std::uint8_t depth,
        bool writable,
        std::vector<PathSegment> segments,
        bool top_level = false,
        std::optional<PropertyEditorKind> authored_editor = {},
        std::vector<std::string> authored_choices = {},
        std::string owner_property_name = {}) {
        if (owner_property_name.empty()) owner_property_name = descriptor.name;
        const std::string canonical = canonical_binding_name(path);
        const std::string id = row_id(path);
        const bool expandable = expandable_value(value);
        const bool expanded = expanded_properties.contains(canonical);
        const bool resettable = top_level && descriptor.resettable;
        const Control::Ptr object = target();
        const bool reset_enabled = resettable && object &&
            object->should_serialize_property(descriptor.name);
        PropertyEditorKind editor = authored_editor.value_or(
            editor_kind(descriptor));
        if (!writable && editor != PropertyEditorKind::read_only) {
            editor = PropertyEditorKind::read_only;
        }
        if (authored_choices.empty() && descriptor.enumeration &&
            !descriptor.enumeration->flags) {
            authored_choices.reserve(descriptor.enumeration->choices.size());
            for (const PropertyEnumChoice& choice :
                 descriptor.enumeration->choices) {
                authored_choices.push_back(choice.name);
            }
        }
        if (authored_choices.empty() &&
            descriptor.standard_values_exclusive) {
            authored_choices.reserve(descriptor.standard_values.size());
            for (const BindingValue& standard : descriptor.standard_values) {
                authored_choices.push_back(display_value(
                    standard, descriptor, top_level));
            }
            editor = writable ? PropertyEditorKind::choice
                              : PropertyEditorKind::read_only;
        }
        rows.emplace_back(
            id, std::move(display_name),
            display_value(value, descriptor, top_level),
            std::move(description), editor, std::move(authored_choices),
            std::string{}, writable || expandable, false,
            std::move(parent_id), depth, expandable, expanded,
            resettable, reset_enabled);
        property_to_row.insert_or_assign(canonical, id);
        row_to_property.insert_or_assign(id, path);
        edit_targets.insert_or_assign(
            canonical, EditTarget{owner_property_name, segments, writable,
                                  descriptor});
        values.insert_or_assign(canonical, value);

        if (!expandable || depth >= maximum_property_value_depth) return;
        for (const CompoundFieldSpec& field :
             compound_fields(binding_value_kind(value))) {
            const auto field_value = compound_field_value(value, field.name);
            if (!field_value) continue;
            std::vector<PathSegment> child_segments = segments;
            child_segments.push_back(
                {PathSegmentKind::compound_field, std::string(field.name), 0U});
            const std::string child_path = append_member_path(path, field.name);
            PropertyDescriptor child_descriptor;
            child_descriptor.name = std::string(field.name);
            child_descriptor.kind = field.kind;
            child_descriptor.category = descriptor.category;
            child_descriptor.description =
                std::string(field.name) + " component of " + path;
            child_descriptor.writable = writable;
            append_projected_rows(
                rows, child_descriptor, *field_value, child_path,
                std::string(field.name),
                child_descriptor.description,
                id, static_cast<std::uint8_t>(depth + 1U), writable,
                std::move(child_segments), false, field.editor, field.choices,
                owner_property_name);
        }
        if (const auto* nested_object =
                std::get_if<PropertyObjectValue>(&value);
            nested_object && *nested_object) {
            for (const PropertyObjectMember& member : nested_object->members()) {
                std::vector<PathSegment> child_segments = segments;
                child_segments.push_back(
                    {PathSegmentKind::object_member, member.name, 0U});
                const std::string child_path = append_member_path(path, member.name);
                PropertyDescriptor member_descriptor;
                member_descriptor.name = member.name;
                member_descriptor.kind = member.declared_kind.value_or(
                    binding_value_kind(member.value));
                member_descriptor.category = descriptor.category;
                member_descriptor.description = member.description;
                member_descriptor.writable = writable && member.writable;
                member_descriptor.nullable = member.nullable;
                member_descriptor.enumeration = member.enumeration;
                member_descriptor.standard_values = member.standard_values;
                member_descriptor.standard_values_exclusive =
                    member.standard_values_exclusive;
                member_descriptor.converter_name = member.converter_name;
                member_descriptor.editor_name = member.editor_name;
                append_projected_rows(
                    rows, member_descriptor, member.value, child_path, member.name,
                    member.description.empty()
                        ? member.name + " member of " +
                              std::string(nested_object->type_name())
                        : member.description,
                    id, static_cast<std::uint8_t>(depth + 1U),
                    writable && member.writable, std::move(child_segments),
                    false, {}, {}, owner_property_name);
            }
        }
        if (const auto* collection =
                std::get_if<PropertyCollectionValue>(&value);
            collection && *collection) {
            const auto items = property_collection_items(*collection);
            for (std::size_t index = 0U; index < items.size(); ++index) {
                std::vector<PathSegment> child_segments = segments;
                child_segments.push_back(
                    {PathSegmentKind::collection_index, {}, index});
                const std::string child_path = append_index_path(path, index);
                PropertyDescriptor item_descriptor;
                item_descriptor.name = "[" + std::to_string(index) + "]";
                item_descriptor.kind = collection->item_kind();
                item_descriptor.category = descriptor.category;
                item_descriptor.description =
                    "Item " + std::to_string(index) + " of " +
                    std::string(collection->item_type_name());
                item_descriptor.writable = writable;
                append_projected_rows(
                    rows, item_descriptor, items[index], child_path,
                    "[" + std::to_string(index) + "]",
                    item_descriptor.description,
                    id, static_cast<std::uint8_t>(depth + 1U), writable,
                    std::move(child_segments), false, {}, {},
                    owner_property_name);
            }
        }
    }

    void rebuild() {
        if (!list) return;
        ++projection_revision;
        clear_subscriptions();
        installed_editors.clear();
        descriptors.clear();
        property_to_row.clear();
        row_to_property.clear();
        edit_targets.clear();
        values.clear();
        last_error.reset();

        const Control::Ptr object = target();
        if (!object) {
            selected.clear();
            list->set_groups({});
            list->set_accessible_name("Properties");
            return;
        }

        std::vector<PropertyDescriptor> visible;
        for (PropertyDescriptor descriptor : object->property_descriptors()) {
            if (!descriptor.browsable || !descriptor.readable) continue;
            bool common = true;
            for (const Control::Ptr& peer : targets()) {
                if (peer.get() == object.get()) continue;
                const auto peer_descriptor = peer->property_descriptor(
                    descriptor.name);
                const auto left_value = object->property_value(descriptor.name);
                const auto right_value = peer->property_value(descriptor.name);
                if (!peer_descriptor ||
                    !compatible_descriptor(descriptor, *peer_descriptor) ||
                    !left_value || !right_value ||
                    !same_shape(*left_value, *right_value)) {
                    common = false;
                    break;
                }
                descriptor.writable = descriptor.writable &&
                    peer_descriptor->writable;
            }
            if (!common) continue;
            visible.push_back(std::move(descriptor));
        }
        std::sort(visible.begin(), visible.end(),
            [this](const PropertyDescriptor& left,
                   const PropertyDescriptor& right) {
                if (sort == PropertySort::alphabetical) {
                    return canonical_binding_name(left.name) <
                           canonical_binding_name(right.name);
                }
                const std::string left_category =
                    canonical_binding_name(left.category);
                const std::string right_category =
                    canonical_binding_name(right.category);
                if (left_category != right_category) {
                    return left_category < right_category;
                }
                return canonical_binding_name(left.name) <
                       canonical_binding_name(right.name);
            });

        std::map<std::string, PropertyGroupSpec> categorized;
        PropertyGroupSpec alphabetical;
        alphabetical.stable_id = group_id("properties");
        alphabetical.title = "PROPERTIES";
        for (const PropertyDescriptor& descriptor : visible) {
            const auto value = object->property_value(descriptor.name);
            if (!value) continue;
            const std::string canonical = canonical_binding_name(descriptor.name);
            const PropertyValueOrigin origin =
                object->property_value_origin(descriptor.name);
            std::vector<PropertyRowSpec> projected_rows;
            append_projected_rows(
                projected_rows, descriptor, *value, descriptor.name,
                descriptor.name, description_for(descriptor, origin), {}, 0U,
                descriptor.writable, {}, true);
            if (sort == PropertySort::alphabetical) {
                alphabetical.rows.insert(alphabetical.rows.end(),
                    std::make_move_iterator(projected_rows.begin()),
                    std::make_move_iterator(projected_rows.end()));
            } else {
                const std::string category = descriptor.category.empty()
                    ? std::string("Misc") : descriptor.category;
                auto [position, inserted] = categorized.try_emplace(category);
                if (inserted) {
                    position->second.stable_id = group_id(category);
                    position->second.title = category;
                }
                position->second.rows.insert(position->second.rows.end(),
                    std::make_move_iterator(projected_rows.begin()),
                    std::make_move_iterator(projected_rows.end()));
            }
            descriptors.emplace(canonical, descriptor);
        }

        std::vector<PropertyGroupSpec> groups;
        if (sort == PropertySort::alphabetical) {
            if (!alphabetical.rows.empty()) {
                groups.push_back(std::move(alphabetical));
            }
        } else {
            groups.reserve(categorized.size());
            for (auto& [category, group] : categorized) {
                static_cast<void>(category);
                groups.push_back(std::move(group));
            }
        }
        list->set_groups(std::move(groups));
        list->set_accessible_name(
            std::string(object->stable_id().value()) + " properties");

        install_custom_editors();

        for (const auto& [canonical, descriptor] : descriptors) {
            if (!descriptor.change_notifications) continue;
            SubscriptionToken subscription = object->subscribe_property_changed(
                descriptor.name, owner, [this, canonical] {
                    if (!committing) refresh_one(canonical, true);
                });
            if (subscription.connected()) {
                property_subscriptions.push_back(std::move(subscription));
            }
        }
    }

    void install_custom_editors() {
        if (!list || !editors) return;
        for (const auto& [canonical, target_spec] : edit_targets) {
            if (!target_spec.writable) continue;
            const auto value = values.find(canonical);
            const auto row = property_to_row.find(canonical);
            if (value == values.end() || row == property_to_row.end()) {
                continue;
            }
            PropertyEditorRequest request;
            request.stable_id = row->second + ".custom-editor";
            request.property_path = row_to_property.at(row->second);
            request.descriptor = target_spec.descriptor;
            request.value = value->second;
            request.writable = target_spec.writable;
            request.top_level = target_spec.path.empty();
            try {
                auto binding = editors->create(request);
                if (!binding) continue;
                const Control::WeakPtr owner_lifetime = owner.weak_from_this();
                SubscriptionToken committed = binding->connect_committed(
                    owner, [this, owner_lifetime,
                            path = request.property_path](
                               BindingValue proposed) {
                        const Control::Ptr retained = owner_lifetime.lock();
                        if (!retained || !retained->is_alive()) return;
                        static_cast<void>(set_value(path, std::move(proposed),
                                                    false));
                    });
                if (!committed.connected()) {
                    throw std::invalid_argument(
                        "Property editor factory returned no commit subscription");
                }
                SubscriptionToken failure;
                if (binding->connect_failed) {
                    failure = binding->connect_failed(
                        owner, [this, owner_lifetime,
                                path = request.property_path](
                                   const PropertyEditorInputError& error) {
                            const Control::Ptr retained = owner_lifetime.lock();
                            if (!retained || !retained->is_alive()) return;
                            report_error(path, error.attempted_value,
                                         error.message);
                        });
                    if (!failure.connected()) {
                        throw std::invalid_argument(
                            "Property editor factory returned no failure subscription");
                    }
                }
                if (!list->replace_editor(row->second, binding->control)) {
                    throw std::logic_error(
                        "Property editor row disappeared during installation");
                }
                installed_editors.insert_or_assign(
                    canonical,
                    InstalledEditor{std::move(*binding), std::move(committed),
                                    std::move(failure)});
            } catch (const std::exception& error) {
                report_error(request.property_path, "<editor>", error.what());
            }
        }
    }

    void refresh_one(std::string_view canonical_name, bool publish) {
        const Control::Ptr object = target();
        if (!object) {
            rebuild();
            return;
        }
        const std::string canonical = canonical_binding_name(canonical_name);
        const auto descriptor = descriptors.find(canonical);
        const auto row = property_to_row.find(canonical);
        if (descriptor == descriptors.end() || row == property_to_row.end()) {
            rebuild();
            return;
        }
        const auto current = object->property_value(descriptor->second.name);
        if (!current) return;
        const BindingValue previous = values.contains(canonical)
            ? values.at(canonical) : *current;
        const PropertyValueOrigin origin =
            object->property_value_origin(descriptor->second.name);
        const std::string authored_name = descriptor->second.name;
        if (!same_shape(previous, *current)) {
            rebuild();
        } else {
            for (const auto& [path_name, target_spec] : edit_targets) {
                if (canonical_binding_name(target_spec.property_name) != canonical) {
                    continue;
                }
                const auto path_value = value_at(*current, target_spec.path);
                const auto path_row = property_to_row.find(path_name);
                if (!path_value || path_row == property_to_row.end()) {
                    rebuild();
                    break;
                }
                values.insert_or_assign(path_name, *path_value);
                static_cast<void>(list->set_value(
                    path_row->second,
                    display_value(*path_value, target_spec.descriptor,
                                  target_spec.path.empty())));
                if (const auto installed = installed_editors.find(path_name);
                    installed != installed_editors.end()) {
                    installed->second.binding.synchronize(*path_value);
                }
                static_cast<void>(list->set_validation(path_row->second, {}));
            }
            if (const auto top_row = property_to_row.find(canonical);
                top_row != property_to_row.end()) {
                static_cast<void>(list->set_description(
                    top_row->second,
                    description_for(descriptor->second, origin)));
                if (descriptor->second.resettable) {
                    static_cast<void>(list->set_reset_enabled(
                        top_row->second,
                        object->should_serialize_property(
                            descriptor->second.name)));
                }
            }
        }
        if (publish && previous != *current) {
            PropertyGridValueChange change{
                authored_name, previous, *current, origin, false};
            owner.publish_change(owner.property_value_changed_, change);
        }
    }

    void report_error(std::string property_name, std::string attempted,
                      std::string message) {
        last_error = PropertyGridEditError{
            std::move(property_name), std::move(attempted), std::move(message)};
        const std::string canonical =
            canonical_binding_name(last_error->property_name);
        if (const auto row = property_to_row.find(canonical);
            row != property_to_row.end()) {
            static_cast<void>(list->set_validation(row->second,
                                                   last_error->message));
        }
        owner.edit_failed_.emit(*last_error);
    }

    bool set_value(std::string_view property_name, BindingValue value,
                   bool reset, bool structural = false) {
        const Control::Ptr object = target();
        const std::vector<Control::Ptr> objects = targets();
        if (!object || objects.empty()) return false;
        const std::string canonical = canonical_binding_name(property_name);
        const auto edit_target = edit_targets.find(canonical);
        if (edit_target == edit_targets.end()) {
            report_error(std::string(property_name),
                         binding_value_to_string(value),
                         "Property is not available for editing");
            return false;
        }
        const std::string property_canonical = canonical_binding_name(
            edit_target->second.property_name);
        const auto descriptor = descriptors.find(property_canonical);
        if (descriptor == descriptors.end()) return false;
        const EditTarget target_spec = edit_target->second;
        const PropertyDescriptor descriptor_spec = descriptor->second;
        const std::uint64_t expected_projection = projection_revision;
        const std::string authored_path(property_name);
        if (!descriptor_spec.writable || !target_spec.writable ||
            (reset && !target_spec.path.empty())) {
            report_error(authored_path, binding_value_to_string(value),
                         reset ? "Only the owning property can be reset"
                               : "Property is not available for editing");
            return false;
        }
        struct OwnerEdit final {
            Control::Ptr object;
            PropertyDescriptor descriptor;
            BindingValue previous_property;
            BindingValue previous_value;
            BindingValue next_property;
        };
        std::vector<OwnerEdit> edits;
        edits.reserve(objects.size());
        try {
            for (const Control::Ptr& candidate : objects) {
                const auto candidate_descriptor = candidate->property_descriptor(
                    descriptor_spec.name);
                const auto previous_property = candidate->property_value(
                    descriptor_spec.name);
                if (!candidate_descriptor || !candidate_descriptor->writable ||
                    !previous_property ||
                    !compatible_descriptor(descriptor_spec,
                                           *candidate_descriptor)) {
                    throw std::invalid_argument(
                        "Every selected owner must expose the same writable property schema");
                }
                const auto previous_value = value_at(
                    *previous_property, target_spec.path);
                if (!previous_value) {
                    throw std::invalid_argument(
                        "Every selected owner must expose the same property path");
                }
                BindingValue next_property = *previous_property;
                if (reset) {
                    if (!candidate_descriptor->resettable) {
                        throw std::invalid_argument(
                            "Every selected owner must expose the same reset contract");
                    }
                } else {
                    const auto converted = convert_property_value(
                        value, target_spec.descriptor);
                    const auto next = converted
                        ? (target_spec.path.empty()
                               ? std::optional<BindingValue>{*converted}
                               : replace_at(*previous_property,
                                            target_spec.path, *converted))
                        : std::optional<BindingValue>{};
                    if (!next) {
                        throw std::invalid_argument(
                            "Property path value cannot convert to its declared kind");
                    }
                    const auto owner_normalized = convert_property_value(
                        *next, *candidate_descriptor);
                    if (!owner_normalized) {
                        throw std::invalid_argument(
                            "Property owner rejected the preflight value schema");
                    }
                    next_property = *owner_normalized;
                }
                edits.push_back({candidate, *candidate_descriptor,
                                 *previous_property, *previous_value,
                                 std::move(next_property)});
            }
        } catch (const std::exception& error) {
            report_error(authored_path,
                         reset ? std::string("<reset>")
                               : binding_value_to_string(value),
                         error.what());
            return false;
        }
        const BindingValue previous_value = edits.front().previous_value;
        committing = true;
        struct CommitReset final {
            bool& flag;
            ~CommitReset() { flag = false; }
        } commit_reset{committing};
        try {
            std::size_t applied = 0U;
            try {
                for (; applied < edits.size(); ++applied) {
                    if (reset) {
                        if (!edits[applied].object->reset_property(
                                edits[applied].descriptor.name)) {
                            throw std::invalid_argument(
                                "A selected owner rejected its reset contract");
                        }
                    } else {
                        edits[applied].object->set_property_value(
                            edits[applied].descriptor.name,
                            edits[applied].next_property);
                    }
                }
            } catch (...) {
                bool rollback_failed = false;
                const std::size_t rollback_count =
                    std::min(applied + 1U, edits.size());
                for (std::size_t index = rollback_count; index > 0U; --index) {
                    try {
                        edits[index - 1U].object->set_property_value(
                            edits[index - 1U].descriptor.name,
                            edits[index - 1U].previous_property);
                    } catch (...) {
                        rollback_failed = true;
                    }
                }
                if (rollback_failed) {
                    throw std::runtime_error(
                        "Atomic property commit failed and a hostile setter also rejected rollback");
                }
                throw;
            }
            if (!owner.is_alive() || !object->is_alive()) return true;
            if (projection_revision != expected_projection ||
                target().get() != object.get()) {
                return true;
            }
            const auto current_property = object->property_value(
                descriptor_spec.name);
            if (!current_property) return false;
            const auto current_value = value_at(*current_property,
                                                target_spec.path);
            if (!current_value) return false;
            if (structural) rebuild();
            else refresh_one(property_canonical, false);
            last_error.reset();
            if (*current_value != previous_value || reset) {
                const PropertyValueOrigin origin =
                    object->property_value_origin(descriptor_spec.name);
                PropertyGridValueChange change{
                    authored_path, previous_value, *current_value,
                    origin, reset};
                owner.publish_change(owner.property_value_changed_, change);
            }
            return true;
        } catch (const std::exception& error) {
            if (!owner.is_alive()) return false;
            if (projection_revision != expected_projection ||
                target().get() != object.get()) {
                return false;
            }
            if (object->is_alive()) {
                refresh_one(property_canonical, false);
            }
            report_error(authored_path,
                         reset ? std::string("<reset>")
                               : binding_value_to_string(value),
                         error.what());
            return false;
        }
    }

    [[nodiscard]] std::optional<PropertyCollectionValue> collection_at_path(
        std::string_view property_name) {
        const Control::Ptr object = target();
        if (!object) return {};
        const std::string canonical = canonical_binding_name(property_name);
        const auto target_entry = edit_targets.find(canonical);
        if (target_entry == edit_targets.end()) return {};
        const auto property = object->property_value(
            target_entry->second.property_name);
        if (!property) return {};
        const auto current = value_at(*property, target_entry->second.path);
        if (!current) return {};
        const auto* collection = std::get_if<PropertyCollectionValue>(&*current);
        return collection && *collection
            ? std::optional<PropertyCollectionValue>{*collection}
            : std::optional<PropertyCollectionValue>{};
    }

    bool insert_collection_item(std::string_view property_name,
                                std::size_t index, BindingValue value) {
        const auto collection = collection_at_path(property_name);
        if (!collection) {
            report_error(std::string(property_name), binding_value_to_string(value),
                         "Property path is not a collection");
            return false;
        }
        const auto current = property_collection_items(*collection);
        if (index > current.size() ||
            current.size() >= maximum_property_collection_items) {
            report_error(std::string(property_name), binding_value_to_string(value),
                         "Collection insertion index or capacity is invalid");
            return false;
        }
        try {
            std::vector<BindingValue> items(current.begin(), current.end());
            items.insert(items.begin() + static_cast<std::ptrdiff_t>(index),
                         std::move(value));
            return set_value(
                property_name,
                BindingValue{make_property_collection(
                    std::string(collection->item_type_name()),
                    collection->item_kind(), std::move(items))},
                false, true);
        } catch (const std::exception& error) {
            report_error(std::string(property_name), "<insert>", error.what());
            return false;
        }
    }

    bool remove_collection_item(std::string_view property_name,
                                std::size_t index) {
        const auto collection = collection_at_path(property_name);
        if (!collection) {
            report_error(std::string(property_name), "<remove>",
                         "Property path is not a collection");
            return false;
        }
        const auto current = property_collection_items(*collection);
        if (index >= current.size()) {
            report_error(std::string(property_name), "<remove>",
                         "Collection removal index is invalid");
            return false;
        }
        std::vector<BindingValue> items(current.begin(), current.end());
        items.erase(items.begin() + static_cast<std::ptrdiff_t>(index));
        return set_value(
            property_name,
            BindingValue{make_property_collection(
                std::string(collection->item_type_name()),
                collection->item_kind(), std::move(items))},
            false, true);
    }

    bool move_collection_item(std::string_view property_name,
                              std::size_t from, std::size_t to) {
        const auto collection = collection_at_path(property_name);
        if (!collection) {
            report_error(std::string(property_name), "<move>",
                         "Property path is not a collection");
            return false;
        }
        const auto current = property_collection_items(*collection);
        if (from >= current.size() || to >= current.size()) {
            report_error(std::string(property_name), "<move>",
                         "Collection move index is invalid");
            return false;
        }
        if (from == to) return true;
        std::vector<BindingValue> items(current.begin(), current.end());
        BindingValue moving = std::move(items[from]);
        items.erase(items.begin() + static_cast<std::ptrdiff_t>(from));
        items.insert(items.begin() + static_cast<std::ptrdiff_t>(to),
                     std::move(moving));
        return set_value(
            property_name,
            BindingValue{make_property_collection(
                std::string(collection->item_type_name()),
                collection->item_kind(), std::move(items))},
            false, true);
    }

    void commit_row(const PropertyValueChange& change) {
        const auto property = row_to_property.find(change.row_id);
        if (property == row_to_property.end()) return;
        const std::string canonical = canonical_binding_name(property->second);
        const auto target_spec = edit_targets.find(canonical);
        const auto current = values.find(canonical);
        if (target_spec == edit_targets.end() || current == values.end()) return;
        const PropertyDescriptor& effective = target_spec->second.descriptor;
        const auto converted = converters
            ? converters->parse(change.current_value, current->second, effective)
            : convert_property_value(BindingValue{change.current_value},
                                     effective);
        if (!converted) {
            report_error(property->second, change.current_value,
                         "Property text cannot convert through its declared converter");
            refresh_one(canonical_binding_name(
                target_spec->second.property_name), false);
            return;
        }
        static_cast<void>(set_value(property->second, *converted, false));
    }

    PropertyGrid& owner;
    std::shared_ptr<PropertyList> list;
    std::shared_ptr<PropertyValueConverterRegistry> converters;
    std::shared_ptr<PropertyEditorRegistry> editors;
    std::vector<Control::WeakPtr> selected;
    PropertySort sort{PropertySort::categorized};
    std::map<std::string, PropertyDescriptor> descriptors;
    std::map<std::string, std::string> property_to_row;
    std::map<std::string, std::string> row_to_property;
    std::map<std::string, EditTarget> edit_targets;
    std::map<std::string, BindingValue> values;
    std::unordered_set<std::string> expanded_properties;
    std::vector<SubscriptionToken> property_subscriptions;
    std::map<std::string, InstalledEditor> installed_editors;
    SubscriptionToken list_commit;
    SubscriptionToken list_expansion;
    SubscriptionToken list_reset;
    std::optional<PropertyGridEditError> last_error;
    std::uint64_t projection_revision{};
    bool committing{};
};

PropertyGrid::PropertyGrid(StableId stable_id)
    : Panel(std::move(stable_id)), impl_(std::make_unique<Impl>(*this)) {
    set_background(Color::rgba(244, 247, 251));
    set_border_style(BorderStyle::line);
    set_paint_plane(PaintPlane::control);
}

PropertyGrid::~PropertyGrid() = default;

void PropertyGrid::initialize_control_tree() {
    require_mutable();
    if (impl_->list) return;
    impl_->list = make_control<PropertyList>(
        StableId(std::string(stable_id().value()) + ".list"));
    impl_->list->set_label_width(112.0);
    add_child(impl_->list);
    impl_->list_commit = impl_->list->value_committed().subscribe(
        *this, [implementation = impl_.get()](const PropertyValueChange& change) {
            implementation->commit_row(change);
        });
    impl_->list_expansion = impl_->list->row_expansion_changed().subscribe(
        *this, [implementation = impl_.get()](
                   const PropertyRowExpansionChange& change) {
            const auto property = implementation->row_to_property.find(
                change.row_id);
            if (property == implementation->row_to_property.end()) return;
            const std::string canonical = canonical_binding_name(
                property->second);
            if (change.expanded) {
                implementation->expanded_properties.insert(canonical);
            } else {
                implementation->expanded_properties.erase(canonical);
            }
        });
    impl_->list_reset = impl_->list->reset_requested().subscribe(
        *this, [implementation = impl_.get()](
                   const PropertyResetRequest& request) {
            const auto property = implementation->row_to_property.find(
                request.row_id);
            if (property == implementation->row_to_property.end()) return;
            static_cast<void>(implementation->set_value(
                property->second, BindingValue{}, true));
        });
}

Control::Ptr PropertyGrid::selected_object() const noexcept {
    return impl_->target();
}

void PropertyGrid::set_selected_object(Control::Ptr object) {
    set_selected_objects(object ? std::vector<Control::Ptr>{std::move(object)}
                                : std::vector<Control::Ptr>{});
}

std::vector<Control::Ptr> PropertyGrid::selected_objects() const {
    return impl_->targets();
}

void PropertyGrid::set_selected_objects(std::vector<Control::Ptr> objects) {
    require_mutable();
    if (!impl_->list) initialize_control_tree();
    std::unordered_set<Control*> identities;
    for (const Control::Ptr& object : objects) {
        if (!object || !object->is_alive() ||
            !identities.insert(object.get()).second) {
            throw std::invalid_argument(
                "PropertyGrid selection requires unique live controls");
        }
    }
    const std::vector<Control::Ptr> previous = impl_->targets();
    if (previous == objects) {
        refresh_properties();
        return;
    }
    impl_->selected.clear();
    impl_->selected.reserve(objects.size());
    for (const Control::Ptr& object : objects) {
        impl_->selected.push_back(object);
    }
    impl_->rebuild();
    publish_change(selected_object_changed_,
                   objects.empty() ? Control::Ptr{} : objects.front());
}

PropertySort PropertyGrid::property_sort() const noexcept {
    return impl_->sort;
}

void PropertyGrid::set_property_sort(PropertySort sort) {
    require_mutable();
    if (sort != PropertySort::categorized &&
        sort != PropertySort::alphabetical) {
        throw std::invalid_argument("PropertyGrid sort mode is invalid");
    }
    if (impl_->sort == sort) return;
    impl_->sort = sort;
    if (!impl_->list) initialize_control_tree();
    impl_->rebuild();
}

void PropertyGrid::refresh_properties() {
    require_mutable();
    if (!impl_->list) initialize_control_tree();
    impl_->rebuild();
}

std::shared_ptr<PropertyList> PropertyGrid::property_list() const noexcept {
    return impl_->list;
}

std::shared_ptr<PropertyValueConverterRegistry>
PropertyGrid::converter_registry() const noexcept {
    return impl_->converters;
}

void PropertyGrid::set_converter_registry(
    std::shared_ptr<PropertyValueConverterRegistry> registry) {
    require_mutable();
    if (!registry) {
        throw std::invalid_argument(
            "PropertyGrid requires a converter registry");
    }
    if (impl_->converters == registry) return;
    impl_->converters = std::move(registry);
    if (!impl_->list) initialize_control_tree();
    impl_->rebuild();
}

std::shared_ptr<PropertyEditorRegistry>
PropertyGrid::editor_registry() const noexcept {
    return impl_->editors;
}

void PropertyGrid::set_editor_registry(
    std::shared_ptr<PropertyEditorRegistry> registry) {
    require_mutable();
    if (!registry) {
        throw std::invalid_argument(
            "PropertyGrid requires an editor registry");
    }
    if (impl_->editors == registry) return;
    impl_->editors = std::move(registry);
    if (!impl_->list) initialize_control_tree();
    impl_->rebuild();
}

Control::Ptr PropertyGrid::editor(std::string_view property_name) const {
    if (!impl_->list) return {};
    const auto found = impl_->property_to_row.find(
        canonical_binding_name(property_name));
    return found == impl_->property_to_row.end()
        ? Control::Ptr{} : impl_->list->editor(found->second);
}

std::shared_ptr<Button> PropertyGrid::reset_button(
    std::string_view property_name) const {
    if (!impl_->list) return {};
    const auto found = impl_->property_to_row.find(
        canonical_binding_name(property_name));
    return found == impl_->property_to_row.end()
        ? std::shared_ptr<Button>{}
        : impl_->list->reset_button(found->second);
}

std::optional<PropertyDescriptor> PropertyGrid::selected_descriptor(
    std::string_view property_name) const {
    const std::string canonical = canonical_binding_name(property_name);
    const auto target = impl_->edit_targets.find(canonical);
    return target == impl_->edit_targets.end()
        ? std::optional<PropertyDescriptor>{}
        : std::optional<PropertyDescriptor>{target->second.descriptor};
}

std::optional<PropertyValueOrigin> PropertyGrid::selected_origin(
    std::string_view property_name) const {
    const auto selected = selected_object();
    if (!selected) return {};
    const auto target = impl_->edit_targets.find(
        canonical_binding_name(property_name));
    return target != impl_->edit_targets.end()
        ? std::optional<PropertyValueOrigin>{
              selected->property_value_origin(target->second.property_name)}
        : std::optional<PropertyValueOrigin>{};
}

bool PropertyGrid::set_property_expanded(std::string_view property_name,
                                         bool expanded) {
    require_mutable();
    if (!impl_->list) initialize_control_tree();
    const std::string canonical = canonical_binding_name(property_name);
    const auto found = impl_->property_to_row.find(canonical);
    const auto target = impl_->edit_targets.find(canonical);
    if (found == impl_->property_to_row.end() ||
        target == impl_->edit_targets.end()) {
        return false;
    }
    return impl_->list->set_row_expanded(found->second, expanded);
}

std::optional<bool> PropertyGrid::property_expanded(
    std::string_view property_name) const {
    if (!impl_->list) return {};
    const std::string canonical = canonical_binding_name(property_name);
    const auto found = impl_->property_to_row.find(canonical);
    return found == impl_->property_to_row.end()
        ? std::optional<bool>{}
        : impl_->list->row_expanded(found->second);
}

bool PropertyGrid::try_set_property_value(std::string_view property_name,
                                          BindingValue value) {
    require_mutable();
    return impl_->set_value(property_name, std::move(value), false);
}

bool PropertyGrid::try_set_property_text(std::string_view property_name,
                                         std::string_view text) {
    require_mutable();
    if (!impl_->list) initialize_control_tree();
    const std::string canonical = canonical_binding_name(property_name);
    const auto target = impl_->edit_targets.find(canonical);
    const auto current = impl_->values.find(canonical);
    if (target == impl_->edit_targets.end() || current == impl_->values.end()) {
        impl_->report_error(std::string(property_name), std::string(text),
                            "Property is not available for text editing");
        return false;
    }
    const auto parsed = impl_->converters->parse(
        text, current->second, target->second.descriptor);
    if (!parsed) {
        impl_->report_error(std::string(property_name), std::string(text),
                            "Property converter rejected the submitted text");
        return false;
    }
    return impl_->set_value(property_name, *parsed, false);
}

bool PropertyGrid::activate_property_editor(std::string_view property_name) {
    require_mutable();
    const Control::Ptr retained = editor(property_name);
    return retained && retained->is_alive() && retained->enabled() &&
        retained->on_semantic_action(SemanticAction::press, {});
}

bool PropertyGrid::reset_property(std::string_view property_name) {
    require_mutable();
    return impl_->set_value(property_name, BindingValue{}, true);
}

bool PropertyGrid::insert_collection_item(std::string_view property_name,
                                          std::size_t index,
                                          BindingValue value) {
    require_mutable();
    return impl_->insert_collection_item(property_name, index,
                                         std::move(value));
}

bool PropertyGrid::remove_collection_item(std::string_view property_name,
                                          std::size_t index) {
    require_mutable();
    return impl_->remove_collection_item(property_name, index);
}

bool PropertyGrid::move_collection_item(std::string_view property_name,
                                        std::size_t from, std::size_t to) {
    require_mutable();
    return impl_->move_collection_item(property_name, from, to);
}

std::optional<PropertyGridEditError> PropertyGrid::last_error() const {
    return impl_->last_error;
}

Size PropertyGrid::measure(Size available) {
    return available;
}

void PropertyGrid::arrange(Rect final_bounds) {
    arrange_self(final_bounds);
    if (impl_->list) {
        set_child_layout(impl_->list,
            {0.0, 0.0, std::max(0.0, final_bounds.width),
             std::max(0.0, final_bounds.height)});
    }
}

SemanticDescriptor PropertyGrid::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.exposed = false;
    descriptor.include_descendants = true;
    return descriptor;
}

void PropertyGrid::on_dispose() noexcept {
    impl_->clear_subscriptions();
    impl_->installed_editors.clear();
    impl_->list_commit.disconnect();
    impl_->list_expansion.disconnect();
    impl_->list_reset.disconnect();
    impl_->selected.clear();
    impl_->list.reset();
    Control::on_dispose();
}

} // namespace gui_forms
