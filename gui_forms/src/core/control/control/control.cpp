#include "gui_forms/control.hpp"

#include "gui_forms/binding.hpp"
#include "gui_forms/text.hpp"
#include "gui_forms/window.hpp"
#include "../../display/chunk/display_chunk.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>

namespace gui_forms {

namespace {

[[nodiscard]] constexpr bool valid_bounds_specified(
    BoundsSpecified value) noexcept {
    return (static_cast<std::uint8_t>(value) &
            ~static_cast<std::uint8_t>(BoundsSpecified::all)) == 0U;
}

[[nodiscard]] constexpr bool valid_child_skip(
    GetChildAtPointSkip value) noexcept {
    constexpr std::uint8_t all =
        static_cast<std::uint8_t>(GetChildAtPointSkip::invisible) |
        static_cast<std::uint8_t>(GetChildAtPointSkip::disabled) |
        static_cast<std::uint8_t>(GetChildAtPointSkip::transparent);
    return (static_cast<std::uint8_t>(value) & ~all) == 0U;
}

[[nodiscard]] constexpr bool valid_binding_value_kind(
    BindingValueKind value) noexcept {
    return value == BindingValueKind::null ||
        value == BindingValueKind::boolean ||
        value == BindingValueKind::signed_integer ||
        value == BindingValueKind::unsigned_integer ||
        value == BindingValueKind::number ||
        value == BindingValueKind::text ||
        value == BindingValueKind::point ||
        value == BindingValueKind::size ||
        value == BindingValueKind::rectangle ||
        value == BindingValueKind::insets ||
        value == BindingValueKind::color ||
        value == BindingValueKind::font ||
        value == BindingValueKind::image ||
        value == BindingValueKind::enumeration ||
        value == BindingValueKind::object ||
        value == BindingValueKind::collection;
}

std::shared_ptr<const PropertyEnumDescriptor> dock_style_property_enum_impl() {
    static const std::shared_ptr<const gui_forms::PropertyEnumDescriptor> value = std::make_shared<const PropertyEnumDescriptor>(
        PropertyEnumDescriptor{
            "System.Windows.Forms.DockStyle",
            {{"None", 0}, {"Top", 1}, {"Bottom", 2}, {"Left", 3},
             {"Right", 4}, {"Fill", 5}},
            false});
    return value;
}

std::shared_ptr<const PropertyEnumDescriptor> anchor_styles_property_enum_impl() {
    static const std::shared_ptr<const gui_forms::PropertyEnumDescriptor> value = std::make_shared<const PropertyEnumDescriptor>(
        PropertyEnumDescriptor{
            "System.Windows.Forms.AnchorStyles",
            {{"None", 0}, {"Top", 1}, {"Bottom", 2}, {"Left", 4},
             {"Right", 8}},
            true});
    return value;
}

std::shared_ptr<const PropertyEnumDescriptor> auto_size_mode_property_enum_impl() {
    static const std::shared_ptr<const gui_forms::PropertyEnumDescriptor> value = std::make_shared<const PropertyEnumDescriptor>(
        PropertyEnumDescriptor{
            "System.Windows.Forms.AutoSizeMode",
            {{"GrowAndShrink", 0}, {"GrowOnly", 1}},
            false});
    return value;
}

BindingValue current_enum_property_value_impl(
    std::shared_ptr<const PropertyEnumDescriptor> enumeration,
    std::int64_t value) {
    PropertyDescriptor descriptor;
    descriptor.kind = BindingValueKind::enumeration;
    descriptor.enumeration = std::move(enumeration);
    const std::optional<BindingValue> normalized = convert_property_value(BindingValue{value}, descriptor);
    if (!normalized) {
        throw std::logic_error(
            "GUI.Forms retained enum state is outside its property schema");
    }
    return *normalized;
}

[[nodiscard]] Size unconstrained_if_zero(Size proposed) {
    if (!std::isfinite(proposed.width) || !std::isfinite(proposed.height) ||
        proposed.width < 0.0 || proposed.height < 0.0) {
        throw std::invalid_argument(
            "GUI.Forms proposed size must be finite and nonnegative");
    }
    constexpr double practical_unbounded_extent = 1.0e9;
    if (proposed.width == 0.0) proposed.width = practical_unbounded_extent;
    if (proposed.height == 0.0) proposed.height = practical_unbounded_extent;
    return proposed;
}

[[nodiscard]] char32_t decode_utf8_scalar(std::string_view text,
                                          std::size_t offset,
                                          std::size_t& length) noexcept {
    const unsigned char lead = static_cast<unsigned char>(text[offset]);
    if (lead < 0x80U) {
        length = 1U;
        return static_cast<char32_t>(lead);
    }
    if ((lead & 0xE0U) == 0xC0U) length = 2U;
    else if ((lead & 0xF0U) == 0xE0U) length = 3U;
    else if ((lead & 0xF8U) == 0xF0U) length = 4U;
    else {
        length = 1U;
        return static_cast<char32_t>(lead);
    }
    if (offset + length > text.size()) {
        length = 1U;
        return static_cast<char32_t>(lead);
    }
    char32_t value = static_cast<char32_t>(
        lead & (length == 2U ? 0x1FU : length == 3U ? 0x0FU : 0x07U));
    for (std::size_t index = 1U; index < length; ++index) {
        const unsigned char continuation =
            static_cast<unsigned char>(text[offset + index]);
        if ((continuation & 0xC0U) != 0x80U) {
            length = 1U;
            return static_cast<char32_t>(lead);
        }
        value = (value << 6U) | static_cast<char32_t>(continuation & 0x3FU);
    }
    return value;
}

[[nodiscard]] constexpr char32_t fold_mnemonic(char32_t value) noexcept {
    return value >= U'A' && value <= U'Z' ? value + (U'a' - U'A') : value;
}

} // namespace

std::shared_ptr<const PropertyEnumDescriptor>
Control::dock_style_property_enum() {
    return dock_style_property_enum_impl();
}

std::shared_ptr<const PropertyEnumDescriptor>
Control::anchor_styles_property_enum() {
    return anchor_styles_property_enum_impl();
}

std::shared_ptr<const PropertyEnumDescriptor>
Control::auto_size_mode_property_enum() {
    return auto_size_mode_property_enum_impl();
}

BindingValue Control::current_enum_property_value(
    std::shared_ptr<const PropertyEnumDescriptor> enumeration,
    std::int64_t value) {
    return current_enum_property_value_impl(std::move(enumeration), value);
}

MnemonicText parse_mnemonic_text(std::string_view text) {
    MnemonicText result;
    result.display_text.reserve(text.size());
    for (std::size_t offset = 0U; offset < text.size();) {
        if (text[offset] != '&') {
            std::size_t length{};
            static_cast<void>(decode_utf8_scalar(text, offset, length));
            result.display_text.append(text.substr(offset, length));
            offset += length;
            continue;
        }
        if (offset + 1U < text.size() && text[offset + 1U] == '&') {
            result.display_text.push_back('&');
            offset += 2U;
            continue;
        }
        ++offset;
        if (offset >= text.size()) {
            // A terminal marker has no target and is retained literally.
            result.display_text.push_back('&');
            break;
        }
        std::size_t length{};
        const char32_t scalar = decode_utf8_scalar(text, offset, length);
        if (!result.mnemonic) result.mnemonic = scalar;
        result.display_text.append(text.substr(offset, length));
        offset += length;
    }
    return result;
}

bool is_mnemonic(char32_t character, std::string_view text) noexcept {
    if (character == U'&') return false;
    for (std::size_t offset = 0U; offset < text.size();) {
        if (text[offset] != '&') {
            std::size_t length{};
            static_cast<void>(decode_utf8_scalar(text, offset, length));
            offset += length;
            continue;
        }
        if (offset + 1U < text.size() && text[offset + 1U] == '&') {
            offset += 2U;
            continue;
        }
        ++offset;
        if (offset >= text.size()) return false;
        std::size_t length{};
        const char32_t scalar = decode_utf8_scalar(text, offset, length);
        if (fold_mnemonic(character) == fold_mnemonic(scalar)) return true;
        offset += length;
    }
    return false;
}

void append_semantic_description(SemanticDescriptor& descriptor,
                                 std::string_view prefix,
                                 std::string_view value) {
    if (value.empty()) return;
    if (!descriptor.description.empty()) descriptor.description += "\n";
    descriptor.description.append(prefix);
    descriptor.description.append(value);
}

std::atomic<std::uint64_t> Control::next_runtime_id_{1};

Control::Control(StableId stable_id)
    : runtime_id_{next_runtime_id_.fetch_add(1, std::memory_order_relaxed)},
      stable_id_(std::move(stable_id)), name_(stable_id_.value()) {
    define_bindable_property({
        {"Name", BindingValueKind::text, "Design",
         "Stable authoring name used by inspection and generated surfaces.",
         BindingValue{name_}, Dirty::semantics | Dirty::accessibility},
        RegisteredPropertyGetter<RegisteredProperty::name>{this},
        RegisteredPropertySetter<RegisteredProperty::name>{this},
        RegisteredPropertyConnector<RegisteredProperty::name>{this}, {}, {}});
    define_bindable_property({
        {"Visible", BindingValueKind::boolean, "Behavior",
         "Whether this control participates in retained presentation and input.",
         BindingValue{true}, invalidation::visibility, true},
        RegisteredPropertyGetter<RegisteredProperty::visible>{this},
        RegisteredPropertySetter<RegisteredProperty::visible>{this},
        RegisteredPropertyConnector<RegisteredProperty::visible>{this}, {},
        {}});
    define_bindable_property({
        {"Enabled", BindingValueKind::boolean, "Behavior",
         "Whether this control can receive ordinary user input.",
         BindingValue{true}, invalidation::enabled},
        RegisteredPropertyGetter<RegisteredProperty::enabled>{this},
        RegisteredPropertySetter<RegisteredProperty::enabled>{this},
        RegisteredPropertyConnector<RegisteredProperty::enabled>{this}, {},
        {}});
    define_bindable_property({
        {"AutoSize", BindingValueKind::boolean, "Layout",
         "Whether retained measurement determines this control's size.",
         BindingValue{false}, Dirty::measure | Dirty::arrange |
             Dirty::hit_test | Dirty::semantics | Dirty::accessibility},
        RegisteredPropertyGetter<RegisteredProperty::auto_size>{this},
        RegisteredPropertySetter<RegisteredProperty::auto_size>{this},
        RegisteredPropertyConnector<RegisteredProperty::auto_size>{this}, {},
        {}});
    define_bindable_property({
        {"CausesValidation", BindingValueKind::boolean, "Behavior",
         "Whether moving focus from this control initiates validation.",
         BindingValue{true}, Dirty::none},
        RegisteredPropertyGetter<RegisteredProperty::causes_validation>{this},
        RegisteredPropertySetter<RegisteredProperty::causes_validation>{this},
        RegisteredPropertyConnector<RegisteredProperty::causes_validation>{
            this},
        {}, {}});

    PropertyDescriptor bounds;
    bounds.name = "Bounds";
    bounds.kind = BindingValueKind::rectangle;
    bounds.category = "Layout";
    bounds.description = "Authored logical bounds before parent layout.";
    bounds.default_value = BindingValue{Rect{}};
    bounds.invalidation_effects = invalidation::bounds;
    define_structural_property(
        std::move(bounds),
        RegisteredPropertyGetter<RegisteredProperty::bounds>{this},
        RegisteredPropertySetter<RegisteredProperty::bounds>{this});

    PropertyDescriptor minimum_size;
    minimum_size.name = "MinimumSize";
    minimum_size.kind = BindingValueKind::size;
    minimum_size.category = "Layout";
    minimum_size.description = "Minimum retained layout size.";
    minimum_size.default_value = BindingValue{Size{}};
    minimum_size.invalidation_effects = invalidation::bounds;
    define_structural_property(
        std::move(minimum_size),
        RegisteredPropertyGetter<RegisteredProperty::minimum_size>{this},
        RegisteredPropertySetter<RegisteredProperty::minimum_size>{this});

    PropertyDescriptor maximum_size;
    maximum_size.name = "MaximumSize";
    maximum_size.kind = BindingValueKind::size;
    maximum_size.category = "Layout";
    maximum_size.description =
        "Maximum retained layout size; zero dimensions are unbounded.";
    maximum_size.default_value = BindingValue{Size{}};
    maximum_size.invalidation_effects = invalidation::bounds;
    define_structural_property(
        std::move(maximum_size),
        RegisteredPropertyGetter<RegisteredProperty::maximum_size>{this},
        RegisteredPropertySetter<RegisteredProperty::maximum_size>{this});

    PropertyDescriptor margin;
    margin.name = "Margin";
    margin.kind = BindingValueKind::insets;
    margin.category = "Layout";
    margin.description = "External logical spacing requested from a layout owner.";
    margin.default_value = BindingValue{Insets{3.0, 3.0, 3.0, 3.0}};
    margin.invalidation_effects = Dirty::measure | Dirty::arrange |
        Dirty::hit_test | Dirty::semantics | Dirty::accessibility;
    define_structural_property(
        std::move(margin),
        RegisteredPropertyGetter<RegisteredProperty::margin>{this},
        RegisteredPropertySetter<RegisteredProperty::margin>{this});

    PropertyDescriptor padding;
    padding.name = "Padding";
    padding.kind = BindingValueKind::insets;
    padding.category = "Layout";
    padding.description = "Internal logical spacing around child content.";
    padding.default_value = BindingValue{Insets{}};
    padding.invalidation_effects = invalidation::bounds;
    define_structural_property(
        std::move(padding),
        RegisteredPropertyGetter<RegisteredProperty::padding>{this},
        RegisteredPropertySetter<RegisteredProperty::padding>{this});

    PropertyDescriptor auto_scroll_offset;
    auto_scroll_offset.name = "AutoScrollOffset";
    auto_scroll_offset.kind = BindingValueKind::point;
    auto_scroll_offset.category = "Layout";
    auto_scroll_offset.description =
        "Logical offset used when revealing this control in a scroll owner.";
    auto_scroll_offset.default_value = BindingValue{Point{}};
    auto_scroll_offset.invalidation_effects =
        Dirty::semantics | Dirty::accessibility;
    define_structural_property(
        std::move(auto_scroll_offset),
        RegisteredPropertyGetter<RegisteredProperty::auto_scroll_offset>{this},
        RegisteredPropertySetter<RegisteredProperty::auto_scroll_offset>{
            this});

    PropertyDescriptor dock;
    dock.name = "Dock";
    dock.kind = BindingValueKind::enumeration;
    dock.category = "Layout";
    dock.description = "Edge or fill participation in parent layout.";
    dock.default_value = BindingValue{PropertyEnumValue{
        "System.Windows.Forms.DockStyle", "None", 0}};
    dock.invalidation_effects = invalidation::bounds;
    dock.enumeration = dock_style_property_enum();
    define_structural_property(
        std::move(dock),
        RegisteredPropertyGetter<RegisteredProperty::dock>{this},
        RegisteredPropertySetter<RegisteredProperty::dock>{this});

    PropertyDescriptor anchor;
    anchor.name = "Anchor";
    anchor.kind = BindingValueKind::enumeration;
    anchor.category = "Layout";
    anchor.description = "Flags retaining distances to parent client edges.";
    anchor.default_value = BindingValue{PropertyEnumValue{
        "System.Windows.Forms.AnchorStyles", "Top, Left", 5}};
    anchor.invalidation_effects = invalidation::bounds;
    anchor.enumeration = anchor_styles_property_enum();
    define_structural_property(
        std::move(anchor),
        RegisteredPropertyGetter<RegisteredProperty::anchor>{this},
        RegisteredPropertySetter<RegisteredProperty::anchor>{this});

    PropertyDescriptor auto_size_mode;
    auto_size_mode.name = "AutoSizeMode";
    auto_size_mode.kind = BindingValueKind::enumeration;
    auto_size_mode.category = "Layout";
    auto_size_mode.description = "Whether automatic sizing may shrink.";
    auto_size_mode.default_value = BindingValue{PropertyEnumValue{
        "System.Windows.Forms.AutoSizeMode", "GrowOnly", 1}};
    auto_size_mode.invalidation_effects = Dirty::measure | Dirty::arrange |
        Dirty::hit_test | Dirty::semantics | Dirty::accessibility;
    auto_size_mode.enumeration = auto_size_mode_property_enum();
    define_structural_property(
        std::move(auto_size_mode),
        RegisteredPropertyGetter<RegisteredProperty::auto_size_mode>{this},
        RegisteredPropertySetter<RegisteredProperty::auto_size_mode>{this});

    PropertyDescriptor tab_index;
    tab_index.name = "TabIndex";
    tab_index.kind = BindingValueKind::unsigned_integer;
    tab_index.category = "Behavior";
    tab_index.description = "Keyboard traversal order within the retained tree.";
    tab_index.default_value = BindingValue{std::uint64_t{0}};
    tab_index.invalidation_effects = Dirty::semantics | Dirty::accessibility;
    define_structural_property(
        std::move(tab_index),
        RegisteredPropertyGetter<RegisteredProperty::tab_index>{this},
        RegisteredPropertySetter<RegisteredProperty::tab_index>{this});

    PropertyDescriptor tab_stop;
    tab_stop.name = "TabStop";
    tab_stop.kind = BindingValueKind::boolean;
    tab_stop.category = "Behavior";
    tab_stop.description = "Whether keyboard traversal may focus this control.";
    tab_stop.default_value = BindingValue{true};
    tab_stop.invalidation_effects = Dirty::semantics | Dirty::accessibility;
    define_structural_property(
        std::move(tab_stop),
        RegisteredPropertyGetter<RegisteredProperty::tab_stop>{this},
        RegisteredPropertySetter<RegisteredProperty::tab_stop>{this});

    PropertyDescriptor allow_drop;
    allow_drop.name = "AllowDrop";
    allow_drop.kind = BindingValueKind::boolean;
    allow_drop.category = "Behavior";
    allow_drop.description = "Whether this control may receive drag data.";
    allow_drop.default_value = BindingValue{false};
    allow_drop.invalidation_effects = Dirty::semantics | Dirty::hit_test;
    define_structural_property(
        std::move(allow_drop),
        RegisteredPropertyGetter<RegisteredProperty::allow_drop>{this},
        RegisteredPropertySetter<RegisteredProperty::allow_drop>{this});

    PropertyDescriptor hit_test_transparent;
    hit_test_transparent.name = "HitTestTransparent";
    hit_test_transparent.kind = BindingValueKind::boolean;
    hit_test_transparent.category = "Behavior";
    hit_test_transparent.description =
        "Whether ordinary pointer routing passes through this control.";
    hit_test_transparent.default_value = BindingValue{false};
    hit_test_transparent.invalidation_effects =
        Dirty::hit_test | Dirty::semantics | Dirty::accessibility;
    hit_test_transparent.browsable = false;
    define_structural_property(
        std::move(hit_test_transparent),
        RegisteredPropertyGetter<RegisteredProperty::hit_test_transparent>{
            this},
        RegisteredPropertySetter<RegisteredProperty::hit_test_transparent>{
            this});

    PropertyDescriptor accessible_name;
    accessible_name.name = "AccessibleName";
    accessible_name.kind = BindingValueKind::text;
    accessible_name.category = "Accessibility";
    accessible_name.description = "Authored assistive name override.";
    accessible_name.default_value = BindingValue{std::string{}};
    accessible_name.invalidation_effects = Dirty::semantics;
    define_structural_property(
        std::move(accessible_name),
        RegisteredPropertyGetter<RegisteredProperty::accessible_name>{this},
        RegisteredPropertySetter<RegisteredProperty::accessible_name>{this});

    PropertyDescriptor accessible_description;
    accessible_description.name = "AccessibleDescription";
    accessible_description.kind = BindingValueKind::text;
    accessible_description.category = "Accessibility";
    accessible_description.description = "Authored assistive description.";
    accessible_description.default_value = BindingValue{std::string{}};
    accessible_description.invalidation_effects = Dirty::semantics;
    define_structural_property(
        std::move(accessible_description),
        RegisteredPropertyGetter<RegisteredProperty::accessible_description>{
            this},
        RegisteredPropertySetter<RegisteredProperty::accessible_description>{
            this});
}

void Control::define_structural_property(
    PropertyDescriptor descriptor, PropertyRegistration::Getter get,
    PropertyRegistration::Setter set) {
    descriptor.bindable = false;
    define_bindable_property({std::move(descriptor), std::move(get),
                              std::move(set), {}, {}, {}});
}

Control::~Control() = default;

double Control::effective_text_scale() const noexcept {
    return window_ != nullptr
        ? (*window_).presentation_settings().text_scale
        : 1.0;
}

FontSpec Control::effective_font(FontSpec authored) const noexcept {
    const double scale = effective_text_scale();
    authored.size *= scale;
    authored.letter_spacing *= scale;
    return authored;
}

const Theme& Control::effective_theme() const noexcept {
    if (theme_override_) return *theme_override_;
    if (const Ptr visual_parent = parent_.lock()) {
        return (*visual_parent).effective_theme();
    }
    if (window_ != nullptr) return (*window_).theme();
    return *default_theme();
}

void Control::set_theme_override(std::shared_ptr<const Theme> theme) {
    require_mutable();
    if (!theme) {
        throw std::invalid_argument("control theme override may not be null");
    }
    if (theme_override_ == theme) return;
    theme_override_ = std::move(theme);
    invalidate_subtree(invalidation::conservative_subtree);
}

void Control::clear_theme_override() {
    require_mutable();
    if (!theme_override_) return;
    theme_override_.reset();
    invalidate_subtree(invalidation::conservative_subtree);
}

void Control::set_visual_status(ControlVisualStatus status) {
    require_mutable();
    if (status != ControlVisualStatus::normal &&
        status != ControlVisualStatus::pending &&
        status != ControlVisualStatus::invalid) {
        throw std::invalid_argument("control visual status is invalid");
    }
    if (visual_status_ == status) return;
    visual_status_ = status;
    invalidate(Dirty::style | Dirty::paint | Dirty::semantics);
}

ControlVisualContext Control::visual_context(
    bool hovered, bool pressed, bool selected, bool focused,
    bool defaulted) const noexcept {
    ControlVisualContext result;
    result.selected = selected;
    result.focused = focused;
    result.defaulted = defaulted;
    result.high_contrast = window_ != nullptr &&
                           (*window_).presentation_settings().high_contrast;
    if (!effectively_enabled()) {
        result.surface = ControlSurfaceState::disabled;
    } else if (visual_status_ == ControlVisualStatus::invalid) {
        result.surface = ControlSurfaceState::invalid;
    } else if (visual_status_ == ControlVisualStatus::pending) {
        result.surface = ControlSurfaceState::pending;
    } else if (pressed) {
        result.surface = ControlSurfaceState::pressed;
    } else if (hovered) {
        result.surface = ControlSurfaceState::hot;
    } else if (window_ != nullptr && !(*window_).active()) {
        result.surface = ControlSurfaceState::deactivated;
    }
    return result;
}

void Control::set_tag(std::any tag) {
    require_mutable();
    tag_ = std::move(tag);
}

void Control::define_bindable_property(BindableProperty property) {
    PropertyDescriptor& descriptor = property.descriptor;
    const std::string canonical = canonical_binding_name(descriptor.name);
    const std::string::size_type first =
        descriptor.name.find_first_not_of(" \t\r\n");
    const std::string::size_type last =
        descriptor.name.find_last_not_of(" \t\r\n");
    descriptor.name = descriptor.name.substr(first, last - first + 1U);
    if (!descriptor.readable && !descriptor.writable) {
        throw std::invalid_argument(
            "GUI.Forms property must be readable or writable");
    }
    if (!valid_binding_value_kind(descriptor.kind)) {
        throw std::invalid_argument(
            "GUI.Forms property value kind is invalid");
    }
    if ((descriptor.kind == BindingValueKind::enumeration) !=
        static_cast<bool>(descriptor.enumeration) ||
        (descriptor.enumeration &&
         !valid_property_enum_descriptor(*descriptor.enumeration))) {
        throw std::invalid_argument(
            "GUI.Forms enumeration property requires one valid enum descriptor");
    }
    if (!validate_utf8(descriptor.converter_name).valid() ||
        !validate_utf8(descriptor.editor_name).valid() ||
        !validate_utf8(descriptor.standard_values_provider_name).valid() ||
        descriptor.converter_name.size() > 256U ||
        descriptor.editor_name.size() > 256U ||
        descriptor.standard_values_provider_name.size() > 256U) {
        throw std::invalid_argument(
            "GUI.Forms property service names must be bounded valid UTF-8");
    }
    if (descriptor.standard_values.size() > maximum_property_standard_values ||
        (descriptor.standard_values_exclusive &&
         descriptor.standard_values.empty() &&
         descriptor.standard_values_provider_name.empty())) {
        throw std::invalid_argument(
            "GUI.Forms property standard-value contract is invalid or unbounded");
    }
    std::vector<BindingValue> normalized_standard_values;
    normalized_standard_values.reserve(descriptor.standard_values.size());
    for (const BindingValue& standard : descriptor.standard_values) {
        const std::optional<BindingValue> converted = convert_property_value(standard, descriptor);
        if (!converted ||
            std::find(normalized_standard_values.begin(),
                      normalized_standard_values.end(), *converted) !=
                normalized_standard_values.end()) {
            throw std::invalid_argument(
                "GUI.Forms property standard values must be unique and convertible");
        }
        normalized_standard_values.push_back(*converted);
    }
    descriptor.standard_values = std::move(normalized_standard_values);
    if (descriptor.readable && !property.get) {
        throw std::invalid_argument(
            "GUI.Forms readable property requires a getter");
    }
    if (descriptor.writable && !property.set) {
        throw std::invalid_argument(
            "GUI.Forms writable property requires a setter");
    }
    if (descriptor.serialization_visibility !=
            PropertySerializationVisibility::hidden &&
        descriptor.serialization_visibility !=
            PropertySerializationVisibility::visible &&
        descriptor.serialization_visibility !=
            PropertySerializationVisibility::content) {
        throw std::invalid_argument(
            "GUI.Forms property serialization visibility is invalid");
    }
    if (without_dirty(descriptor.invalidation_effects,
                      invalidation::conservative_subtree) != Dirty::none) {
        throw std::invalid_argument(
            "GUI.Forms property declares unknown invalidation effects");
    }
    if (descriptor.default_value) {
        const std::optional<BindingValue> converted = convert_property_value(
            *descriptor.default_value, descriptor);
        if (!converted) {
            throw std::invalid_argument(
                "GUI.Forms property default cannot convert to its declared kind");
        }
        descriptor.default_value = *converted;
    }
    descriptor.resettable = static_cast<bool>(property.reset) ||
        (descriptor.writable && descriptor.default_value.has_value());
    descriptor.change_notifications =
        static_cast<bool>(property.connect_changed);
    // Derived stock controls may replace a base descriptor when they own a
    // more specific implementation of the same canonical property.
    bindable_properties_.insert_or_assign(canonical, std::move(property));
}

void Control::clear_bindable_properties() {
    require_mutable();
    bindable_properties_.clear();
}

const BindableProperty* Control::find_bindable_property(
    std::string_view name) const {
    const BindablePropertyMap::const_iterator found =
        bindable_properties_.find(canonical_binding_name(name));
    return found == bindable_properties_.end() ||
            !(*found).second.descriptor.bindable
        ? nullptr : &(*found).second;
}

bool Control::has_bindable_property(std::string_view name) const {
    return find_bindable_property(name) != nullptr;
}

std::vector<std::string> Control::bindable_property_names() const {
    std::vector<std::string> result;
    result.reserve(bindable_properties_.size());
    for (const std::pair<const std::string, BindableProperty>& property_entry :
         bindable_properties_) {
        const BindableProperty& property = property_entry.second;
        if (property.descriptor.bindable) {
            result.push_back(property.descriptor.name);
        }
    }
    return result;
}

std::optional<PropertyDescriptor> Control::property_descriptor(
    std::string_view name) const {
    const BindablePropertyMap::const_iterator found =
        bindable_properties_.find(canonical_binding_name(name));
    if (found == bindable_properties_.end()) return std::nullopt;
    return (*found).second.descriptor;
}

std::vector<PropertyDescriptor> Control::property_descriptors() const {
    std::vector<PropertyDescriptor> result;
    result.reserve(bindable_properties_.size());
    for (const std::pair<const std::string, BindableProperty>& property_entry :
         bindable_properties_) {
        const BindableProperty& property = property_entry.second;
        result.push_back(property.descriptor);
    }
    return result;
}

std::optional<BindingValue> Control::property_value(std::string_view name) const {
    if (!is_alive()) {
        throw std::logic_error("GUI.Forms cannot query a disposed control property");
    }
    if (window_) (*window_).require_ui_thread("control property query");
    const BindablePropertyMap::const_iterator found =
        bindable_properties_.find(canonical_binding_name(name));
    if (found == bindable_properties_.end() ||
        !(*found).second.descriptor.readable || !(*found).second.get) {
        return std::nullopt;
    }
    const PropertyRegistration::Getter getter = (*found).second.get;
    const BindingValue value = getter();
    const std::optional<BindingValue> normalized = convert_property_value(
        value, (*found).second.descriptor);
    if (!normalized || !valid_property_value_tree(*normalized)) {
        throw std::logic_error(
            "GUI.Forms property getter violated its declared schema: " +
            (*found).second.descriptor.name);
    }
    return normalized;
}

void Control::set_property_value(std::string_view name, BindingValue value) {
    require_mutable();
    const std::string canonical = canonical_binding_name(name);
    const BindablePropertyMap::iterator found =
        bindable_properties_.find(canonical);
    if (found == bindable_properties_.end()) {
        throw std::invalid_argument("GUI.Forms property is not registered: " +
                                    canonical);
    }
    const PropertyRegistration registration = (*found).second;
    if (!registration.descriptor.writable || !registration.set) {
        throw std::logic_error("GUI.Forms property is read-only: " +
                               registration.descriptor.name);
    }
    const std::optional<BindingValue> converted = convert_property_value(
        value, registration.descriptor);
    if (!converted) {
        throw std::invalid_argument(
            "GUI.Forms property value cannot convert to its declared kind: " +
            registration.descriptor.name);
    }
    registration.set(*converted);
}

SubscriptionToken Control::subscribe_property_changed(
    std::string_view name, Component& owner, std::function<void()> changed) {
    require_mutable();
    const std::string canonical = canonical_binding_name(name);
    const BindablePropertyMap::iterator found =
        bindable_properties_.find(canonical);
    if (found == bindable_properties_.end()) {
        throw std::invalid_argument("GUI.Forms property is not registered: " +
                                    canonical);
    }
    const PropertyRegistration::ChangeConnector connector =
        (*found).second.connect_changed;
    return connector && changed
        ? connector(owner, std::move(changed)) : SubscriptionToken{};
}

bool Control::reset_property(std::string_view name) {
    require_mutable();
    const BindablePropertyMap::iterator found =
        bindable_properties_.find(canonical_binding_name(name));
    if (found == bindable_properties_.end()) return false;
    const PropertyRegistration registration = (*found).second;
    if (registration.reset) {
        registration.reset();
        return true;
    }
    if (!registration.descriptor.writable || !registration.set ||
        !registration.descriptor.default_value) {
        return false;
    }
    registration.set(*registration.descriptor.default_value);
    return true;
}

bool Control::should_serialize_property(std::string_view name) const {
    if (!is_alive()) {
        throw std::logic_error(
            "GUI.Forms cannot inspect a disposed control property");
    }
    if (window_) (*window_).require_ui_thread("control property inspection");
    const std::string canonical = canonical_binding_name(name);
    const BindablePropertyMap::const_iterator found =
        bindable_properties_.find(canonical);
    if (found == bindable_properties_.end()) {
        throw std::invalid_argument("GUI.Forms property is not registered: " +
                                    canonical);
    }
    const PropertyRegistration registration = (*found).second;
    if (registration.descriptor.serialization_visibility ==
        PropertySerializationVisibility::hidden) {
        return false;
    }
    if (registration.should_serialize) {
        return registration.should_serialize();
    }
    if (!registration.descriptor.readable || !registration.get) return false;
    if (!registration.descriptor.default_value) return true;
    return registration.get() != *registration.descriptor.default_value;
}

PropertyValueOrigin Control::property_value_origin(
    std::string_view name) const {
    if (!is_alive()) {
        throw std::logic_error(
            "GUI.Forms cannot inspect a disposed control property");
    }
    if (window_) {
        (*window_).require_ui_thread("control property origin inspection");
    }
    const std::string canonical = canonical_binding_name(name);
    const BindablePropertyMap::const_iterator found =
        bindable_properties_.find(canonical);
    if (found == bindable_properties_.end()) {
        throw std::invalid_argument("GUI.Forms property is not registered: " +
                                    canonical);
    }
    const PropertyRegistration registration = (*found).second;
    if (registration.origin) return registration.origin();
    if (registration.descriptor.readable && registration.get &&
        registration.descriptor.default_value) {
        return registration.get() == *registration.descriptor.default_value
            ? PropertyValueOrigin::defaulted
            : PropertyValueOrigin::local;
    }
    return PropertyValueOrigin::computed;
}

ControlBindingsCollection& Control::data_bindings() {
    require_mutable();
    if (!data_bindings_) {
        data_bindings_ = std::make_unique<ControlBindingsCollection>(*this);
    }
    return *data_bindings_;
}

const ControlBindingsCollection& Control::data_bindings() const {
    require_mutable();
    if (!data_bindings_) {
        data_bindings_ = std::make_unique<ControlBindingsCollection>(
            *const_cast<Control*>(this));
    }
    return *data_bindings_;
}

void Control::set_name(std::string name) {
    require_mutable();
    if (name_ == name) return;
    name_ = std::move(name);
    invalidate(Dirty::semantics | Dirty::accessibility);
    publish_change(name_changed_, name_);
}

void Control::set_style(ControlStyles style, bool enabled) {
    require_mutable();
    const std::uint32_t requested = static_cast<std::uint32_t>(style);
    constexpr unsigned int known = (1U << 17U) - 1U;
    if ((requested & ~known) != 0U) {
        throw std::invalid_argument("GUI.Forms control style contains unknown bits");
    }
    const std::uint32_t current = static_cast<std::uint32_t>(styles_);
    const std::uint32_t updated = enabled ? current | requested : current & ~requested;
    if (updated == current) return;
    styles_ = static_cast<ControlStyles>(updated);
}

void Control::set_double_buffered(bool enabled) {
    require_mutable();
    // Keep the two historical buffering requests synchronized. They are
    // compatibility facts; coherent retained release remains unconditional.
    set_style(ControlStyles::double_buffer | ControlStyles::optimized_double_buffer,
              enabled);
}

namespace {

void validate_insets(Insets value, const char* message) {
    if (!std::isfinite(value.left) || !std::isfinite(value.top) ||
        !std::isfinite(value.right) || !std::isfinite(value.bottom) ||
        value.left < 0.0 || value.top < 0.0 || value.right < 0.0 ||
        value.bottom < 0.0) {
        throw std::invalid_argument(message);
    }
}

[[nodiscard]] Rect client_rect(Size size, Insets padding) noexcept {
    return {padding.left, padding.top,
            std::max(0.0, size.width - padding.left - padding.right),
            std::max(0.0, size.height - padding.top - padding.bottom)};
}

[[nodiscard]] bool valid_dock(DockStyle dock) noexcept {
    switch (dock) {
    case DockStyle::none:
    case DockStyle::top:
    case DockStyle::bottom:
    case DockStyle::left:
    case DockStyle::right:
    case DockStyle::fill:
        return true;
    }
    return false;
}

[[nodiscard]] bool valid_anchor(AnchorStyles anchor) noexcept {
    constexpr std::uint8_t valid =
        static_cast<std::uint8_t>(AnchorStyles::top) |
        static_cast<std::uint8_t>(AnchorStyles::bottom) |
        static_cast<std::uint8_t>(AnchorStyles::left) |
        static_cast<std::uint8_t>(AnchorStyles::right);
    return (static_cast<std::uint8_t>(anchor) & ~valid) == 0U;
}

[[nodiscard]] Rect anchored_bounds(Rect reference, Rect old_client,
                                   Rect new_client, AnchorStyles anchor) noexcept {
    const double left = reference.x - old_client.x;
    const double right = old_client.x + old_client.width -
                         (reference.x + reference.width);
    const double top = reference.y - old_client.y;
    const double bottom = old_client.y + old_client.height -
                          (reference.y + reference.height);
    const bool anchored_left = has_anchor(anchor, AnchorStyles::left);
    const bool anchored_right = has_anchor(anchor, AnchorStyles::right);
    const bool anchored_top = has_anchor(anchor, AnchorStyles::top);
    const bool anchored_bottom = has_anchor(anchor, AnchorStyles::bottom);

    Rect result = reference;
    if (anchored_left && anchored_right) {
        result.x = new_client.x + left;
        result.width = std::max(0.0, new_client.width - left - right);
    } else if (anchored_right) {
        result.x = new_client.x + new_client.width - right - reference.width;
    } else if (anchored_left) {
        result.x = new_client.x + left;
    } else {
        const double offset = reference.x + reference.width * 0.5 -
                              (old_client.x + old_client.width * 0.5);
        result.x = new_client.x + new_client.width * 0.5 + offset -
                   reference.width * 0.5;
    }

    if (anchored_top && anchored_bottom) {
        result.y = new_client.y + top;
        result.height = std::max(0.0, new_client.height - top - bottom);
    } else if (anchored_bottom) {
        result.y = new_client.y + new_client.height - bottom - reference.height;
    } else if (anchored_top) {
        result.y = new_client.y + top;
    } else {
        const double offset = reference.y + reference.height * 0.5 -
                              (old_client.y + old_client.height * 0.5);
        result.y = new_client.y + new_client.height * 0.5 + offset -
                   reference.height * 0.5;
    }
    return result;
}

} // namespace

void Control::add_child(Ptr child) {
    require_mutable();
    if (lifecycle_notification_ ||
        (window_ != nullptr && (*window_).in_lifecycle_notification_)) {
        throw std::logic_error("GUI.Forms cannot mutate the visual tree during lifecycle notification");
    }
    if (!child) {
        throw std::invalid_argument("GUI.Forms cannot add a null child");
    }
    if (child.get() == this) {
        throw std::logic_error("GUI.Forms control cannot parent itself");
    }
    if (!(*child).is_alive()) {
        throw std::logic_error("GUI.Forms cannot attach a disposed control");
    }
    if ((*child).window_) {
        (*(*child).window_).require_ui_thread("visual-tree mutation");
        if ((*(*child).window_).in_lifecycle_notification_) {
            throw std::logic_error("GUI.Forms cannot mutate the visual tree during lifecycle notification");
        }
    }
    for (std::shared_ptr<gui_forms::Control> ancestor = shared_from_this(); ancestor; ancestor = (*ancestor).parent()) {
        if (ancestor == child) {
            throw std::logic_error("GUI.Forms control tree cannot contain a cycle");
        }
    }
    if ((*child).parent().get() == this) {
        return;
    }
    if ((*child).window_ && (*child).window_ != window_) {
        throw std::logic_error("GUI.Forms control belongs to another window");
    }
    if (Ptr previous_parent = (*child).parent()) {
        static_cast<void>((*previous_parent).remove_child((*child).runtime_id()));
    } else if ((*child).window_) {
        throw std::logic_error("GUI.Forms cannot reparent another window root");
    }
    if (!(*child).is_alive()) {
        throw std::logic_error("GUI.Forms callback disposed control during reparenting");
    }

    (*child).parent_ = weak_from_this();
    (*child).layout_slot_.reset();
    (*child).anchor_reference_.reset();
    children_.push_back(child);
    if (window_) {
        try {
            (*window_).attach_subtree(child, weak_from_this());
        } catch (...) {
            children_.pop_back();
            (*child).parent_.reset();
            throw;
        }
    }
    invalidate_declared(invalidation::visual_tree);
}

Control::Ptr Control::remove_child(RuntimeId child_id) {
    require_mutable();
    if (lifecycle_notification_ ||
        (window_ != nullptr && (*window_).in_lifecycle_notification_)) {
        throw std::logic_error("GUI.Forms cannot mutate the visual tree during lifecycle notification");
    }
    ChildList::iterator found = children_.begin();
    while (found != children_.end() &&
           (*(*found)).runtime_id() != child_id) {
        ++found;
    }
    if (found == children_.end()) {
        return {};
    }
    Ptr removed = *found;
    if (window_) {
        (*window_).detach_subtree(removed);
    }
    ChildList::iterator current = children_.begin();
    while (current != children_.end() &&
           (*(*current)).runtime_id() != child_id) {
        ++current;
    }
    if (current != children_.end()) {
        children_.erase(current);
    }
    if ((*removed).parent_.lock().get() == this) {
        (*removed).parent_.reset();
    }
    (*removed).layout_slot_.reset();
    (*removed).anchor_reference_.reset();
    if (window_) {
        static_cast<void>((*window_).recompute_subtree_dirty((*window_).root_));
    }
    invalidate_declared(invalidation::visual_tree);
    return removed;
}

bool Control::set_child_index(RuntimeId child_id, std::size_t index) {
    require_mutable();
    ChildList::iterator found = children_.begin();
    while (found != children_.end() &&
           (*(*found)).runtime_id() != child_id) {
        ++found;
    }
    if (found == children_.end()) {
        return false;
    }
    Ptr child = *found;
    children_.erase(found);
    // WinForms index zero is topmost. GUI.Forms paints from front to back, so
    // the topmost retained child is the final vector element.
    const std::size_t bounded = std::min(index, children_.size());
    children_.insert(children_.end() - static_cast<std::ptrdiff_t>(bounded),
                     std::move(child));
    invalidate_declared(invalidation::visual_tree);
    return true;
}

std::optional<std::size_t> Control::child_index(RuntimeId child_id) const noexcept {
    ChildList::const_iterator found = children_.begin();
    while (found != children_.end() &&
           (!*found || (*(*found)).runtime_id() != child_id)) {
        ++found;
    }
    if (found == children_.end()) return std::nullopt;
    const std::size_t painter_index = static_cast<std::size_t>(
        std::distance(children_.begin(), found));
    return children_.size() - 1U - painter_index;
}

void Control::clear_children() {
    require_mutable();
    if (lifecycle_notification_ ||
        (window_ != nullptr && (*window_).in_lifecycle_notification_)) {
        throw std::logic_error("GUI.Forms cannot mutate the visual tree during lifecycle notification");
    }
    while (!children_.empty()) {
        static_cast<void>(remove_child((*children_.back()).runtime_id()));
    }
}

void Control::set_requested_bounds(Rect bounds) {
    require_mutable();
    if (!std::isfinite(bounds.x) || !std::isfinite(bounds.y) ||
        !std::isfinite(bounds.width) || !std::isfinite(bounds.height) ||
        bounds.width < 0.0 || bounds.height < 0.0) {
        throw std::invalid_argument(
            "GUI.Forms control bounds must be finite with nonnegative size");
    }
    bounds.width = std::max(bounds.width, minimum_size_.width);
    bounds.height = std::max(bounds.height, minimum_size_.height);
    if (maximum_size_.width > 0.0) {
        bounds.width = std::min(bounds.width, maximum_size_.width);
    }
    if (maximum_size_.height > 0.0) {
        bounds.height = std::min(bounds.height, maximum_size_.height);
    }
    if (requested_bounds_ == bounds) {
        return;
    }
    requested_bounds_ = bounds;
    anchor_reference_.reset();
    invalidate_declared(invalidation::bounds);
}

void Control::set_bounds(Rect values, BoundsSpecified specified) {
    require_mutable();
    if (!valid_bounds_specified(specified)) {
        throw std::invalid_argument(
            "GUI.Forms BoundsSpecified contains unknown flags");
    }
    Rect next = requested_bounds_;
    if (has_bounds_specified(specified, BoundsSpecified::x)) next.x = values.x;
    if (has_bounds_specified(specified, BoundsSpecified::y)) next.y = values.y;
    if (has_bounds_specified(specified, BoundsSpecified::width)) {
        next.width = values.width;
    }
    if (has_bounds_specified(specified, BoundsSpecified::height)) {
        next.height = values.height;
    }
    set_requested_bounds(next);
}

void Control::set_minimum_size(Size size) {
    require_mutable();
    if (!std::isfinite(size.width) || !std::isfinite(size.height) ||
        size.width < 0.0 || size.height < 0.0 ||
        (maximum_size_.width > 0.0 && size.width > maximum_size_.width) ||
        (maximum_size_.height > 0.0 && size.height > maximum_size_.height)) {
        throw std::invalid_argument("GUI.Forms minimum size is invalid");
    }
    if (minimum_size_ == size) return;
    minimum_size_ = size;
    Rect next = requested_bounds_;
    next.width = std::max(next.width, size.width);
    next.height = std::max(next.height, size.height);
    if (next != requested_bounds_) {
        set_requested_bounds(next);
    } else {
        invalidate(invalidation::bounds);
    }
}

void Control::set_maximum_size(Size size) {
    require_mutable();
    if (!std::isfinite(size.width) || !std::isfinite(size.height) ||
        size.width < 0.0 || size.height < 0.0 ||
        (size.width > 0.0 && size.width < minimum_size_.width) ||
        (size.height > 0.0 && size.height < minimum_size_.height)) {
        throw std::invalid_argument("GUI.Forms maximum size is invalid");
    }
    if (maximum_size_ == size) return;
    maximum_size_ = size;
    Rect next = requested_bounds_;
    if (size.width > 0.0) next.width = std::min(next.width, size.width);
    if (size.height > 0.0) next.height = std::min(next.height, size.height);
    if (next != requested_bounds_) {
        set_requested_bounds(next);
    } else {
        invalidate(invalidation::bounds);
    }
}

void Control::set_child_layout(const Ptr& child, Rect bounds) {
    require_mutable();
    if (!child || !(*child).is_alive() || (*child).parent().get() != this) {
        throw std::invalid_argument(
            "GUI.Forms child layout requires a live direct child");
    }
    if (!std::isfinite(bounds.x) || !std::isfinite(bounds.y) ||
        !std::isfinite(bounds.width) || !std::isfinite(bounds.height) ||
        bounds.width < 0.0 || bounds.height < 0.0) {
        throw std::invalid_argument(
            "GUI.Forms child layout bounds must be finite and nonnegative");
    }
    if ((*child).layout_slot_ == bounds) return;
    (*child).layout_slot_ = bounds;
    if (window_ != nullptr) {
        (*window_).mark_child_layout_slot(*child);
    } else {
        constexpr Dirty effects = Dirty::arrange | Dirty::hit_test |
                                  Dirty::semantics | Dirty::accessibility;
        (*child).dirty_ |= effects;
        (*child).subtree_dirty_ |= effects;
        for (Ptr ancestor = (*child).parent(); ancestor;
             ancestor = (*ancestor).parent()) {
            (*ancestor).subtree_dirty_ |= effects;
        }
    }
}

void Control::arrange_self(Rect final_bounds) noexcept {
    final_bounds.width = std::max(final_bounds.width, minimum_size_.width);
    final_bounds.height = std::max(final_bounds.height, minimum_size_.height);
    if (maximum_size_.width > 0.0) {
        final_bounds.width = std::min(final_bounds.width, maximum_size_.width);
    }
    if (maximum_size_.height > 0.0) {
        final_bounds.height = std::min(final_bounds.height, maximum_size_.height);
    }
    arranged_bounds_ = final_bounds;
}

void Control::set_margin(Insets margin) {
    require_mutable();
    validate_insets(margin, "GUI.Forms margin must be finite and nonnegative");
    if (margin_ == margin) return;
    margin_ = margin;
    invalidate(Dirty::measure | Dirty::arrange | Dirty::hit_test |
               Dirty::semantics | Dirty::accessibility);
}

void Control::set_padding(Insets padding) {
    require_mutable();
    validate_insets(padding, "GUI.Forms padding must be finite and nonnegative");
    if (padding_ == padding) return;
    padding_ = padding;
    invalidate(invalidation::bounds);
}

void Control::set_auto_scroll_offset(Point offset) {
    require_mutable();
    if (!std::isfinite(offset.x) || !std::isfinite(offset.y)) {
        throw std::invalid_argument(
            "GUI.Forms AutoScrollOffset must be finite");
    }
    if (auto_scroll_offset_ == offset) return;
    auto_scroll_offset_ = offset;
    invalidate(Dirty::semantics | Dirty::accessibility);
}

void Control::set_dock(DockStyle dock) {
    require_mutable();
    if (!valid_dock(dock)) {
        throw std::invalid_argument("GUI.Forms DockStyle value is invalid");
    }
    if (dock_ == dock) return;
    dock_ = dock;
    anchor_reference_.reset();
    if (dock_ == DockStyle::none) {
        if (const Ptr owner = parent(); owner && (*owner).window_ != nullptr &&
            ((*owner).arranged_bounds_.width > 0.0 ||
             (*owner).arranged_bounds_.height > 0.0) &&
            (arranged_bounds_.width > 0.0 || arranged_bounds_.height > 0.0)) {
            anchor_reference_ = AnchorReference{
                arranged_bounds_,
                client_rect({(*owner).arranged_bounds_.width,
                             (*owner).arranged_bounds_.height}, (*owner).padding_)};
        }
    }
    invalidate(invalidation::bounds);
}

void Control::set_anchor(AnchorStyles anchor) {
    require_mutable();
    if (!valid_anchor(anchor)) {
        throw std::invalid_argument("GUI.Forms AnchorStyles contains unknown flags");
    }
    if (anchor_ == anchor) return;
    anchor_ = anchor;
    anchor_reference_.reset();
    if (dock_ == DockStyle::none) {
        if (const Ptr owner = parent(); owner && (*owner).window_ != nullptr &&
            ((*owner).arranged_bounds_.width > 0.0 ||
             (*owner).arranged_bounds_.height > 0.0) &&
            (arranged_bounds_.width > 0.0 || arranged_bounds_.height > 0.0)) {
            anchor_reference_ = AnchorReference{
                arranged_bounds_,
                client_rect({(*owner).arranged_bounds_.width,
                             (*owner).arranged_bounds_.height}, (*owner).padding_)};
        }
    }
    invalidate(invalidation::bounds);
}

void Control::set_auto_size(bool auto_size) {
    require_mutable();
    if (auto_size_ == auto_size) return;
    auto_size_ = auto_size;
    anchor_reference_.reset();
    invalidate(Dirty::measure | Dirty::arrange | Dirty::hit_test |
               Dirty::semantics | Dirty::accessibility);
    publish_change(auto_size_changed_, auto_size_);
}

void Control::set_auto_size_mode(AutoSizeMode mode) {
    require_mutable();
    if (mode != AutoSizeMode::grow_and_shrink &&
        mode != AutoSizeMode::grow_only) {
        throw std::invalid_argument("GUI.Forms AutoSizeMode value is invalid");
    }
    if (auto_size_mode_ == mode) return;
    auto_size_mode_ = mode;
    if (auto_size_) {
        anchor_reference_.reset();
        invalidate(Dirty::measure | Dirty::arrange | Dirty::hit_test |
                   Dirty::semantics | Dirty::accessibility);
    }
}

Size Control::get_preferred_size(Size proposed) {
    return measure(unconstrained_if_zero(proposed));
}

Rect Control::arranged_bounds() const {
    if (window_) {
        (*window_).ensure_layout(true);
    }
    return arranged_bounds_;
}

void Control::suspend_layout() {
    require_mutable();
    if (layout_suspend_depth_ == std::numeric_limits<std::uint32_t>::max()) {
        throw std::overflow_error("GUI.Forms layout suspension depth overflow");
    }
    if (layout_suspend_depth_ == 0U &&
        has_dirty(subtree_dirty_, Dirty::layout)) {
        layout_deferred_ = true;
        ++layout_requested_revision_;
        if (layout_requested_revision_ == 0U) ++layout_requested_revision_;
    }
    ++layout_suspend_depth_;
}

void Control::resume_layout(bool perform_pending_layout) {
    require_mutable();
    // WinForms tolerates an unmatched ResumeLayout. Keeping that behavior is
    // important for generated/designer code which may unwind conditionally.
    if (layout_suspend_depth_ == 0U) return;
    --layout_suspend_depth_;
    if (layout_suspend_depth_ != 0U || !perform_pending_layout ||
        !layout_deferred_) {
        return;
    }
    if (window_ != nullptr) {
        (*window_).ensure_layout(false);
    }
}

void Control::perform_layout() {
    require_mutable();
    // A suspended request is revisioned by the ordinary invalidation path so
    // every suspended ancestor observes the same mutation exactly once.
    if (layout_suspend_depth_ == 0U) {
        ++layout_requested_revision_;
        if (layout_requested_revision_ == 0U) ++layout_requested_revision_;
    }
    layout_deferred_ = true;
    invalidate(Dirty::measure | Dirty::arrange | Dirty::hit_test |
               Dirty::semantics | Dirty::accessibility);
    if (window_ != nullptr && layout_suspend_depth_ == 0U) {
        (*window_).ensure_layout(false);
    }
}

LayoutTransactionState Control::layout_transaction_state() const noexcept {
    return {layout_suspend_depth_, layout_deferred_, layout_requested_revision_,
            layout_committed_revision_};
}

Rect Control::absolute_bounds() const {
    if (window_) {
        (*window_).ensure_layout(true);
        return (*window_).absolute_bounds_of(*this);
    }
    Rect result = arranged_bounds_;
    for (Ptr ancestor = parent(); ancestor; ancestor = (*ancestor).parent()) {
        result.x += (*ancestor).arranged_bounds_.x;
        result.y += (*ancestor).arranged_bounds_.y;
    }
    return result;
}

Point Control::point_to_window(Point local) const {
    const Rect absolute = absolute_bounds();
    return {absolute.x + local.x, absolute.y + local.y};
}

Point Control::point_from_window(Point window_point) const {
    const Rect absolute = absolute_bounds();
    return {window_point.x - absolute.x, window_point.y - absolute.y};
}

Rect Control::rectangle_to_window(Rect local) const {
    const Point origin = point_to_window({local.x, local.y});
    return {origin.x, origin.y, local.width, local.height};
}

Rect Control::rectangle_from_window(Rect window_rectangle) const {
    const Point origin = point_from_window(
        {window_rectangle.x, window_rectangle.y});
    return {origin.x, origin.y, window_rectangle.width,
            window_rectangle.height};
}

bool Control::contains(const Control& candidate) const noexcept {
    for (Ptr current = candidate.parent(); current; current = (*current).parent()) {
        if (current.get() == this) return true;
    }
    return false;
}

Control::Ptr Control::get_child_at_point(
    Point client_point, GetChildAtPointSkip skip) const {
    if (!std::isfinite(client_point.x) || !std::isfinite(client_point.y)) {
        throw std::invalid_argument(
            "GUI.Forms child lookup point must be finite");
    }
    if (!valid_child_skip(skip)) {
        throw std::invalid_argument(
            "GUI.Forms GetChildAtPointSkip contains unknown flags");
    }
    if (window_ != nullptr) (*window_).ensure_layout(true);
    if (!child_viewport_rectangle().contains(client_point)) return {};
    for (ChildList::const_reverse_iterator current = children_.rbegin();
         current != children_.rend();
         ++current) {
        const Ptr& child = *current;
        if (!child || !(*child).is_alive()) continue;
        if (has_child_skip(skip, GetChildAtPointSkip::invisible) &&
            !(*child).visible_) continue;
        if (has_child_skip(skip, GetChildAtPointSkip::disabled) &&
            !(*child).enabled_) continue;
        if (has_child_skip(skip, GetChildAtPointSkip::transparent) &&
            (*child).hit_test_transparent_) continue;
        if ((*child).arranged_bounds_.contains(client_point)) return child;
    }
    return {};
}

Control::Ptr Control::get_next_control(const Ptr& control,
                                       bool forward) const {
    std::vector<Ptr> ordered;
    collect_tab_order_controls(*this, ordered);
    if (ordered.empty()) return {};
    if (!control || control.get() == this) {
        return forward ? ordered.front() : ordered.back();
    }
    const ChildList::iterator found =
        std::find(ordered.begin(), ordered.end(), control);
    if (found == ordered.end()) return {};
    if (forward) {
        const ChildList::iterator next = std::next(found);
        return next == ordered.end() ? Ptr{} : *next;
    }
    return found == ordered.begin() ? Ptr{} : *std::prev(found);
}

bool Control::tab_order_less(const Ptr& left, const Ptr& right) noexcept {
    if (!left) return false;
    if (!right) return true;
    return (*left).tab_index_ < (*right).tab_index_;
}

void Control::collect_tab_order_controls(const Control& owner,
                                         std::vector<Ptr>& ordered) {
    std::vector<Ptr> children(owner.children_.begin(), owner.children_.end());
    std::stable_sort(children.begin(), children.end(), &Control::tab_order_less);
    for (const Ptr& child : children) {
        if (!child || !(*child).is_alive() ||
            (*child).parent().get() != &owner) {
            continue;
        }
        ordered.push_back(child);
        collect_tab_order_controls(*child, ordered);
    }
}

void Control::bring_to_front() {
    require_mutable();
    if (Ptr owner = parent()) {
        static_cast<void>((*owner).set_child_index(runtime_id_, 0U));
    }
}

void Control::send_to_back() {
    require_mutable();
    if (Ptr owner = parent()) {
        const std::size_t index = (*owner).children_.empty()
            ? 0U : (*owner).children_.size() - 1U;
        static_cast<void>((*owner).set_child_index(runtime_id_, index));
    }
}

void Control::set_visible(bool visible) {
    require_mutable();
    if (visible_ == visible) {
        return;
    }
    visible_ = visible;
    if (window_ && !visible) {
        (*window_).on_eligibility_changed(shared_from_this());
    }
    if (!is_alive()) {
        return;
    }
    invalidate_subtree(invalidation::visibility);
    if (window_) (*window_).publish_control_availability(*this);
    if (!is_alive()) return;
    publish_change(visible_changed_, visible_);
}

void Control::set_enabled(bool enabled) {
    require_mutable();
    if (enabled_ == enabled) {
        return;
    }
    enabled_ = enabled;
    if (window_ && !enabled) {
        (*window_).on_eligibility_changed(shared_from_this());
    }
    if (!is_alive()) {
        return;
    }
    invalidate_declared(invalidation::enabled);
    if (window_) (*window_).publish_control_availability(*this);
    if (!is_alive()) return;
    publish_change(enabled_changed_, enabled_);
}

void Control::set_focusable(bool focusable) {
    require_mutable();
    if (focusable_ == focusable) {
        return;
    }
    focusable_ = focusable;
    if (window_ && !focusable) {
        (*window_).on_eligibility_changed(shared_from_this());
    }
    if (!is_alive()) {
        return;
    }
    invalidate_declared(invalidation::focusability);
}

void Control::set_causes_validation(bool causes_validation) {
    require_mutable();
    if (causes_validation_ == causes_validation) return;
    causes_validation_ = causes_validation;
    publish_change(causes_validation_changed_, causes_validation_);
}

bool Control::perform_validation(Control* destination, bool bulk) {
    require_mutable();
    if (!causes_validation_) return true;
    ControlValidationEvent event{this, destination, bulk, false};
    validating_.emit(event);
    if (!is_alive() || event.cancel) return !event.cancel;
    validated_.emit();
    return true;
}

void Control::set_tab_index(std::uint32_t index) {
    require_mutable();
    if (tab_index_ == index) return;
    tab_index_ = index;
    invalidate(Dirty::semantics | Dirty::accessibility);
}

void Control::set_tab_stop(bool enabled) {
    require_mutable();
    if (tab_stop_ == enabled) return;
    tab_stop_ = enabled;
    invalidate(Dirty::semantics | Dirty::accessibility);
}

void Control::set_allow_drop(bool allow_drop) {
    require_mutable();
    if (allow_drop_ == allow_drop) {
        return;
    }
    allow_drop_ = allow_drop;
    if (window_ && !allow_drop) {
        (*window_).on_eligibility_changed(shared_from_this());
    }
    if (!is_alive()) {
        return;
    }
    invalidate(Dirty::semantics | Dirty::hit_test);
}

void Control::set_hit_test_transparent(bool transparent) {
    require_mutable();
    if (hit_test_transparent_ == transparent) return;
    hit_test_transparent_ = transparent;
    if (window_ && transparent) {
        (*window_).on_hit_test_transparency_changed(shared_from_this());
    }
    if (!is_alive()) return;
    invalidate(Dirty::hit_test | Dirty::semantics | Dirty::accessibility);
}

void Control::set_cursor(std::optional<CursorKind> cursor) {
    require_mutable();
    if (cursor_ == cursor) {
        return;
    }
    cursor_ = cursor;
    invalidate(Dirty::semantics);
}

void Control::set_accessible_name(std::string name) {
    require_mutable();
    if (accessible_name_ == name) return;
    accessible_name_ = std::move(name);
    invalidate(Dirty::semantics);
}

void Control::set_accessible_description(std::string description) {
    require_mutable();
    if (accessible_description_ == description) return;
    accessible_description_ = std::move(description);
    invalidate(Dirty::semantics);
}

CursorKind Control::effective_cursor() const noexcept {
    const Control* current = this;
    Ptr owner;
    while (current != nullptr) {
        if ((*current).cursor_.has_value()) {
            return *(*current).cursor_;
        }
        owner = (*current).parent_.lock();
        current = owner.get();
    }
    return CursorKind::arrow;
}

bool Control::effectively_visible() const noexcept {
    if (!visible_ || !is_alive()) {
        return false;
    }
    for (Ptr ancestor = parent(); ancestor; ancestor = (*ancestor).parent()) {
        if (!(*ancestor).visible_ || !(*ancestor).is_alive()) {
            return false;
        }
    }
    return true;
}

bool Control::effectively_enabled() const noexcept {
    if (!enabled_ || !is_alive()) {
        return false;
    }
    for (Ptr ancestor = parent(); ancestor; ancestor = (*ancestor).parent()) {
        if (!(*ancestor).enabled_ || !(*ancestor).is_alive()) {
            return false;
        }
    }
    return true;
}

bool Control::eligible_for_input() const noexcept {
    return window_ != nullptr && !initialization_blocked() &&
           effectively_visible() && effectively_enabled();
}

void Control::set_pointer_capture(bool captured) {
    require_mutable();
    if (window_ == nullptr) {
        if (!captured) return;
        throw std::logic_error(
            "GUI.Forms pointer capture requires an attached control");
    }
    if (captured) {
        (*window_).capture_pointer(shared_from_this());
    } else if ((*window_).captured_control().get() == this) {
        (*window_).release_pointer();
    }
}

bool Control::has_pointer_capture() const noexcept {
    return window_ != nullptr && (*window_).captured_control().get() == this;
}

void Control::set_paint_plane(PaintPlane plane) {
    require_mutable();
    if (!is_valid_paint_plane(plane)) {
        throw std::invalid_argument("invalid paint plane");
    }
    if (paint_plane_ == plane) {
        return;
    }
    if (window_) {
        (*window_).change_paint_plane(*this, plane);
        return;
    }
    paint_plane_ = plane;
    display_chunk_.reset();
    invalidate(invalidation::paint_only);
}

std::optional<DisplayChunkInfo> Control::display_chunk_info() const noexcept {
    return display_chunk_ ? std::optional((*display_chunk_).info()) : std::nullopt;
}

void Control::invalidate(Dirty requested_dirty) {
    require_mutable();
    if (requested_dirty == Dirty::none) {
        return;
    }
    if (has_dirty(requested_dirty, Dirty::measure)) {
        requested_dirty |= Dirty::arrange;
    }
    if (initialization_depth_ != 0) {
        pending_initialization_dirty_ |= requested_dirty;
        return;
    }
    if (window_) {
        (*window_).mark_dirty(*this, requested_dirty);
        return;
    }
    dirty_ |= requested_dirty;
    for (Control* current = this; current != nullptr;) {
        (*current).subtree_dirty_ |= requested_dirty;
        if (has_dirty(requested_dirty, Dirty::layout) &&
            (*current).layout_suspend_depth_ != 0U) {
            (*current).layout_deferred_ = true;
            ++(*current).layout_requested_revision_;
            if ((*current).layout_requested_revision_ == 0U) {
                ++(*current).layout_requested_revision_;
            }
        }
        const std::shared_ptr<gui_forms::Control> visual_parent = (*current).parent_.lock();
        current = visual_parent.get();
    }
}

void Control::invalidate(Rect local_damage) {
    require_mutable();
    if (!local_damage.finite()) {
        throw std::invalid_argument("local paint damage must be finite");
    }
    local_damage = Rect::intersection(local_damage, client_rectangle());
    if (local_damage.empty()) return;
    if (initialization_depth_ != 0) {
        pending_initialization_dirty_ |= Dirty::paint;
        return;
    }
    if (window_) {
        (*window_).mark_paint_dirty(*this, local_damage);
        return;
    }
    dirty_ |= Dirty::paint;
    for (Control* current = this; current != nullptr;) {
        (*current).subtree_dirty_ |= Dirty::paint;
        const std::shared_ptr<gui_forms::Control> visual_parent = (*current).parent_.lock();
        current = visual_parent.get();
    }
}

void Control::invalidate_subtree(Dirty requested_dirty) {
    require_mutable();
    if (requested_dirty == Dirty::none) {
        return;
    }
    if (has_dirty(requested_dirty, Dirty::measure)) {
        requested_dirty |= Dirty::arrange;
    }
    if (initialization_depth_ != 0) {
        pending_initialization_dirty_ |= requested_dirty;
        pending_initialization_subtree_ = true;
        return;
    }
    if (window_) {
        (*window_).mark_subtree_dirty(*this, requested_dirty);
        return;
    }
    apply_subtree_dirty(*this, requested_dirty);
    for (Ptr visual_parent = parent(); visual_parent;
         visual_parent = (*visual_parent).parent()) {
        (*visual_parent).subtree_dirty_ |= requested_dirty;
        if (has_dirty(requested_dirty, Dirty::layout) &&
            (*visual_parent).layout_suspend_depth_ != 0U) {
            (*visual_parent).layout_deferred_ = true;
            ++(*visual_parent).layout_requested_revision_;
            if ((*visual_parent).layout_requested_revision_ == 0U) {
                ++(*visual_parent).layout_requested_revision_;
            }
        }
    }
}

void Control::apply_subtree_dirty(Control& control, Dirty requested_dirty) {
    control.dirty_ |= requested_dirty;
    control.subtree_dirty_ |= requested_dirty;
    if (has_dirty(requested_dirty, Dirty::layout) &&
        control.layout_suspend_depth_ != 0U) {
        control.layout_deferred_ = true;
        ++control.layout_requested_revision_;
        if (control.layout_requested_revision_ == 0U) {
            ++control.layout_requested_revision_;
        }
    }
    for (const std::shared_ptr<gui_forms::Control>& child :
         control.children_) {
        apply_subtree_dirty(*child, requested_dirty);
    }
}

void Control::invalidate_declared(Dirty declared_effects) {
    require_mutable();
    if (declared_effects != Dirty::none) {
        invalidate(declared_effects);
        return;
    }
    if (window_) {
        (*window_).metrics_.record_undeclared_mutation();
    }
#ifndef NDEBUG
    throw std::logic_error("GUI.Forms mutation has no declared invalidation effects");
#else
    invalidate_subtree(invalidation::conservative_subtree);
#endif
}

void Control::begin_init() {
    require_mutable();
    ++initialization_depth_;
    if (initialization_depth_ == 1U && window_ != nullptr) {
        (*window_).on_eligibility_changed(shared_from_this());
    }
}

void Control::publish_change(const void* event_key,
                             std::function<void()> publication) {
    if (!publication) return;
    if (initialization_depth_ == 0) {
        publication();
        return;
    }
    DeferredInitializationChangeList::iterator pending =
        pending_initialization_changes_.begin();
    while (pending != pending_initialization_changes_.end() &&
           (*pending).event_key != event_key) {
        ++pending;
    }
    if (pending != pending_initialization_changes_.end()) {
        (*pending).publication = std::move(publication);
        return;
    }
    pending_initialization_changes_.push_back(
        DeferredInitializationChange{event_key, std::move(publication)});
}

void Control::end_init() {
    require_mutable();
    if (initialization_depth_ == 0) {
        throw std::logic_error("GUI.Forms EndInit has no matching BeginInit");
    }
    --initialization_depth_;
    if (initialization_depth_ != 0) {
        return;
    }

    const Dirty pending = std::exchange(pending_initialization_dirty_, Dirty::none);
    const bool subtree = std::exchange(pending_initialization_subtree_, false);
    if (pending != Dirty::none) {
        if (subtree) {
            invalidate_subtree(pending);
        } else {
            invalidate(pending);
        }
    }
    std::vector<gui_forms::Control::DeferredInitializationChange> changes = std::exchange(pending_initialization_changes_, {});
    for (std::size_t index = 0; index < changes.size(); ++index) {
        if (!is_alive()) return;
        changes[index].publication();
        // A callback may deliberately begin a new initialization transaction.
        // Do not deliver the previous transaction's remaining observers into
        // that newly partial state; retain them in original order instead.
        if (initialization_depth_ != 0) {
            for (++index; index < changes.size(); ++index) {
                publish_change(changes[index].event_key,
                               std::move(changes[index].publication));
            }
            return;
        }
    }
    if (!is_alive()) return;
    initialization_completed_.emit(pending, subtree);
}

Size Control::measure(Size available) {
    available = {std::max(0.0, available.width),
                 std::max(0.0, available.height)};
    Size desired{requested_bounds_.width, requested_bounds_.height};
    if (auto_size_) {
        double content_width = padding_.left + padding_.right;
        double content_height = padding_.top + padding_.bottom;
        const std::vector<Ptr> retained = snapshot_layout_children();
        for (const Ptr& child : retained) {
            if (!is_current_layout_child(child) || !(*child).visible_) continue;
            const Size child_desired = (*child).get_preferred_size(available);
            if (!is_alive()) return {};
            if (!is_current_layout_child(child) || !(*child).visible_) continue;
            content_width = std::max(
                content_width,
                std::max(0.0, (*child).requested_bounds_.x) +
                    child_desired.width +
                    (*child).margin_.right + padding_.right);
            content_height = std::max(
                content_height,
                std::max(0.0, (*child).requested_bounds_.y) +
                    child_desired.height +
                    (*child).margin_.bottom + padding_.bottom);
        }
        if (auto_size_mode_ == AutoSizeMode::grow_only) {
            content_width = std::max(content_width, requested_bounds_.width);
            content_height = std::max(content_height, requested_bounds_.height);
        }
        desired = {content_width, content_height};
    }
    Size result{std::min(desired.width, available.width),
                std::min(desired.height, available.height)};
    result.width = std::max(result.width, minimum_size_.width);
    result.height = std::max(result.height, minimum_size_.height);
    if (maximum_size_.width > 0.0) result.width = std::min(result.width, maximum_size_.width);
    if (maximum_size_.height > 0.0) result.height = std::min(result.height, maximum_size_.height);
    return result;
}

void Control::arrange(Rect final_bounds) {
    arrange_self(final_bounds);
    if (children_.empty()) return;

    const std::vector<Ptr> retained = snapshot_layout_children();

    const Rect viewport = child_viewport_rectangle();
    const Rect client{
        viewport.x + padding_.left,
        viewport.y + padding_.top,
        std::max(0.0, viewport.width - padding_.left - padding_.right),
        std::max(0.0, viewport.height - padding_.top - padding_.bottom)};
    Rect remaining = client;
    // The retained vector is painter order: backmost first, topmost last.
    // Dock consumes that reverse public z-order (backmost to topmost), matching
    // Forms semantics while leaving index zero consistently topmost for public
    // child indexing, hit testing, BringToFront, and SendToBack.
    for (const Ptr& child : retained) {
        if (!is_current_layout_child(child) || !(*child).visible_ ||
            (*child).dock_ == DockStyle::none) {
            continue;
        }
        const Size desired = (*child).measure({remaining.width, remaining.height});
        if (!is_alive()) return;
        if (!is_current_layout_child(child) || !(*child).visible_ ||
            (*child).dock_ == DockStyle::none) {
            continue;
        }
        const double width = (*child).auto_size()
            ? ((*child).auto_size_mode_ == AutoSizeMode::grow_only
                   ? std::max((*child).requested_bounds_.width, desired.width)
                   : desired.width)
            : ((*child).requested_bounds_.width > 0.0
                   ? (*child).requested_bounds_.width : desired.width);
        const double height = (*child).auto_size()
            ? ((*child).auto_size_mode_ == AutoSizeMode::grow_only
                   ? std::max((*child).requested_bounds_.height, desired.height)
                   : desired.height)
            : ((*child).requested_bounds_.height > 0.0
                   ? (*child).requested_bounds_.height : desired.height);
        Rect slot = remaining;
        switch ((*child).dock_) {
        case DockStyle::top: {
            const double extent = std::clamp(height, 0.0, remaining.height);
            slot.height = extent;
            remaining.y += extent;
            remaining.height -= extent;
            break;
        }
        case DockStyle::bottom: {
            const double extent = std::clamp(height, 0.0, remaining.height);
            slot.y = remaining.y + remaining.height - extent;
            slot.height = extent;
            remaining.height -= extent;
            break;
        }
        case DockStyle::left: {
            const double extent = std::clamp(width, 0.0, remaining.width);
            slot.width = extent;
            remaining.x += extent;
            remaining.width -= extent;
            break;
        }
        case DockStyle::right: {
            const double extent = std::clamp(width, 0.0, remaining.width);
            slot.x = remaining.x + remaining.width - extent;
            slot.width = extent;
            remaining.width -= extent;
            break;
        }
        case DockStyle::fill:
            remaining = {remaining.x, remaining.y, 0.0, 0.0};
            break;
        case DockStyle::none:
            break;
        }
        (*child).anchor_reference_.reset();
        set_child_layout(child, slot);
    }

    for (const Ptr& child : retained) {
        if (!is_current_layout_child(child) || !(*child).visible_ ||
            (*child).dock_ != DockStyle::none) {
            continue;
        }
        Rect authored = (*child).requested_bounds_;
        if ((*child).auto_size()) {
            const Size preferred = (*child).get_preferred_size(
                {client.width, client.height});
            if (!is_alive()) return;
            if (!is_current_layout_child(child) || !(*child).visible_ ||
                (*child).dock_ != DockStyle::none) {
                continue;
            }
            if ((*child).auto_size_mode_ == AutoSizeMode::grow_only) {
                authored.width = std::max(authored.width, preferred.width);
                authored.height = std::max(authored.height, preferred.height);
            } else {
                authored.width = preferred.width;
                authored.height = preferred.height;
            }
        }
        if ((*child).auto_size()) {
            (*child).anchor_reference_ = AnchorReference{authored, client};
        } else if (!(*child).anchor_reference_) {
            (*child).anchor_reference_ = AnchorReference{
                authored, client};
        }
        set_child_layout(
            child, anchored_bounds((*(*child).anchor_reference_).bounds,
                                   (*(*child).anchor_reference_).client,
                                   client, (*child).anchor_));
    }
}

std::vector<Control::Ptr> Control::snapshot_layout_children() const {
    return children_;
}

bool Control::is_current_layout_child(const Ptr& child) const noexcept {
    return child && (*child).is_alive() && (*child).parent_.lock().get() == this &&
           (*child).window_ == window_;
}

void Control::on_paint(Painter&, Rect) {}
void Control::on_paint_overlay(Painter&, Rect) {}

Insets Control::visual_outsets() const noexcept {
    return {};
}

bool Control::hit_test_local(Point local_point) const {
    return Rect{0.0, 0.0, arranged_bounds_.width, arranged_bounds_.height}.contains(local_point);
}

void Control::on_pointer_preview(PointerEvent&) {}
void Control::on_pointer(PointerEvent&) {}
void Control::on_pointer_bubble(PointerEvent&) {}
void Control::on_key_preview(KeyEvent&) {}
void Control::on_key(KeyEvent&) {}
void Control::on_key_bubble(KeyEvent&) {}
void Control::on_text_input(TextInputEvent&) {}
bool Control::process_mnemonic(char32_t character) {
    if (!is_alive() || initialization_blocked() || !effectively_visible() ||
        !effectively_enabled()) {
        return false;
    }

    // Materialize the arbitration set before invoking application code. A
    // mnemonic may dispose/reparent controls, so recursive callback traversal
    // would otherwise make the winner depend on mutation during dispatch.
    std::vector<Ptr> candidates;
    collect_mnemonic_candidates(shared_from_this(), character, candidates);
    for (const Ptr& candidate : candidates) {
        if (candidate && (*candidate).is_alive() &&
            (*candidate).process_mnemonic_self(character)) {
            return true;
        }
    }
    return false;
}

void Control::collect_mnemonic_candidates(const Ptr& control,
                                          char32_t character,
                                          std::vector<Ptr>& candidates) {
    if (!control || !(*control).is_alive() ||
        !(*control).effectively_visible() ||
        !(*control).effectively_enabled()) {
        return;
    }
    if ((*control).mnemonic_matches(character)) {
        candidates.push_back(control);
    }
    std::vector<Ptr> retained((*control).children_.begin(),
                              (*control).children_.end());
    std::stable_sort(retained.begin(), retained.end(),
                     &Control::tab_order_less);
    for (const Ptr& child : retained) {
        if (!child || (*child).parent().get() != control.get() ||
            (*child).window_ != (*control).window_) {
            continue;
        }
        collect_mnemonic_candidates(child, character, candidates);
    }
}
void Control::on_frame(FrameTime) {}

bool Control::prepare_command_activation() {
    if (!is_alive() || initialization_blocked() || !effectively_visible() ||
        !effectively_enabled()) {
        return false;
    }
    return window_ == nullptr ||
           (*window_).validate_command_activation(shared_from_this());
}

bool Control::focus_next_after_self() {
    return window_ != nullptr &&
           (*window_).move_focus_after(shared_from_this());
}

bool Control::perform_dialog_command() { return false; }
bool Control::supports_dialog_command() const noexcept { return false; }
DialogResult Control::command_dialog_result() const noexcept {
    return DialogResult::none;
}
void Control::assign_cancel_dialog_result() {}
bool Control::mnemonic_matches(char32_t) const noexcept { return false; }
bool Control::process_mnemonic_self(char32_t) { return false; }
void Control::notify_default(bool) {}

SemanticDescriptor Control::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.name = accessible_name_;
    descriptor.description = accessible_description_;
    return descriptor;
}

void Control::apply_provider_semantics(SemanticDescriptor& descriptor) const {
    if ((!provider_errors_.empty() || !provider_help_.empty()) &&
        !descriptor.exposed) {
        descriptor.exposed = true;
        if (descriptor.role == SemanticRole::generic) {
            descriptor.role = SemanticRole::group;
        }
    }
    for (const std::pair<const std::uint64_t, std::string>& provider_error :
         provider_errors_) {
        const std::string& error = provider_error.second;
        append_semantic_description(descriptor, "Error: ", error);
        descriptor.states |= SemanticState::invalid;
    }
    for (const std::pair<const std::uint64_t, std::string>& provider_help :
         provider_help_) {
        const std::string& help = provider_help.second;
        append_semantic_description(descriptor, {}, help);
    }
}

std::vector<SemanticNode> Control::semantic_virtual_children() const {
    return {};
}

bool Control::on_semantic_action(SemanticAction action, std::string_view) {
    if (action == SemanticAction::focus && window_ != nullptr && focusable_) {
        return (*window_).request_focus(shared_from_this());
    }
    return false;
}
bool Control::on_semantic_child_action(std::string_view, SemanticAction,
                                       std::string_view) {
    return false;
}
void Control::on_drag_preview(DragEvent&) {}
void Control::on_drag(DragEvent&) {}
void Control::on_drag_bubble(DragEvent&) {}
void Control::on_focus_changed(bool) {}
void Control::on_activate() {}
void Control::on_attached_to_window() {}
void Control::on_attachment_committed() noexcept {}
void Control::on_detaching_from_window(Window&) noexcept {}
void Control::on_detached_from_window() noexcept {}

void Control::clear_dirty(Dirty cleared) noexcept {
    dirty_ = without_dirty(dirty_, cleared);
}

void Control::clear_subtree_dirty(Dirty cleared) noexcept {
    subtree_dirty_ = without_dirty(subtree_dirty_, cleared);
}

void Control::set_provider_error(std::uint64_t provider_id, std::string error) {
    require_mutable();
    if (error.empty()) {
        clear_provider_error(provider_id);
        return;
    }
    const ProviderTextMap::iterator found = provider_errors_.find(provider_id);
    if (found != provider_errors_.end() && (*found).second == error) return;
    provider_errors_[provider_id] = std::move(error);
    invalidate(Dirty::semantics | Dirty::accessibility);
}

void Control::clear_provider_error(std::uint64_t provider_id) {
    require_mutable();
    if (provider_errors_.erase(provider_id) != 0U) {
        invalidate(Dirty::semantics | Dirty::accessibility);
    }
}

void Control::set_provider_help(std::uint64_t provider_id, std::string help) {
    require_mutable();
    if (help.empty()) {
        clear_provider_help(provider_id);
        return;
    }
    const ProviderTextMap::iterator found = provider_help_.find(provider_id);
    if (found != provider_help_.end() && (*found).second == help) return;
    provider_help_[provider_id] = std::move(help);
    invalidate(Dirty::semantics | Dirty::accessibility);
}

void Control::clear_provider_help(std::uint64_t provider_id) {
    require_mutable();
    if (provider_help_.erase(provider_id) != 0U) {
        invalidate(Dirty::semantics | Dirty::accessibility);
    }
}

std::uint64_t Control::subtree_size() const noexcept {
    std::uint64_t result = 1;
    for (const Ptr& child : children_) {
        result += (*child).subtree_size();
    }
    return result;
}

bool Control::initialization_blocked() const noexcept {
    const Control* current = this;
    Ptr retained;
    while (current != nullptr) {
        if ((*current).initialization_depth_ != 0U) return true;
        retained = (*current).parent_.lock();
        current = retained.get();
    }
    return false;
}

void Control::require_mutable() const {
    if (!is_alive()) {
        throw std::logic_error("GUI.Forms cannot mutate a disposed control");
    }
    if (window_) {
        (*window_).require_ui_thread("control mutation");
    }
}

void Control::verify_dispose_thread() {
    if (lifecycle_notification_ ||
        (window_ != nullptr && (*window_).in_lifecycle_notification_)) {
        throw std::logic_error("GUI.Forms cannot dispose a control during lifecycle notification");
    }
    if (window_) {
        (*window_).require_ui_thread("control disposal");
    }
}

void Control::on_dispose() noexcept {
    if (data_bindings_) {
        (*data_bindings_).clear();
        data_bindings_.reset();
    }
    initialization_depth_ = 0;
    pending_initialization_dirty_ = Dirty::none;
    pending_initialization_subtree_ = false;
    pending_initialization_changes_.clear();
    tag_.reset();
    theme_override_.reset();
    provider_errors_.clear();
    provider_help_.clear();
    causes_validation_changed_.disconnect_all();
    validating_.disconnect_all();
    validated_.disconnect_all();
    Ptr self = weak_from_this().lock();
    if (window_ && self) {
        (*window_).dispose_subtree(self);
    } else if (std::shared_ptr<gui_forms::Control> visual_parent = parent_.lock()) {
        ChildList::iterator found = (*visual_parent).children_.begin();
        while (found != (*visual_parent).children_.end() &&
               (*found).get() != this) {
            ++found;
        }
        if (found != (*visual_parent).children_.end()) {
            (*visual_parent).children_.erase(found);
        }
        parent_.reset();
    }

    std::vector<Ptr> visual_children = std::move(children_);
    children_.clear();
    display_chunk_.reset();
    for (const std::shared_ptr<gui_forms::Control>& child : visual_children) {
        (*child).parent_.reset();
        (*child).dispose();
    }
}

} // namespace gui_forms
