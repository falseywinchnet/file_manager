#include "gui_forms/binding.hpp"

#include "gui_forms/control.hpp"
#include "gui_forms/text.hpp"
#include "gui_forms/window.hpp"

#include "../support/binding_support.hpp"

#include <algorithm>
#include <bit>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace gui_forms {

std::string_view property_value_origin_name(
    PropertyValueOrigin origin) noexcept {
    switch (origin) {
    case PropertyValueOrigin::defaulted: return "default";
    case PropertyValueOrigin::local: return "local";
    case PropertyValueOrigin::inherited: return "inherited";
    case PropertyValueOrigin::ambient: return "ambient";
    case PropertyValueOrigin::computed: return "computed";
    }
    return "computed";
}

struct PropertyValueFactoryAccess final {
    static PropertyObjectValue object(
        std::shared_ptr<const PropertyObjectData> data) {
        return PropertyObjectValue(std::move(data));
    }
    static PropertyCollectionValue collection(
        std::shared_ptr<const PropertyCollectionData> data) {
        return PropertyCollectionValue(std::move(data));
    }
};

std::span<const BindingValue> property_collection_items(
    const PropertyCollectionValue& value) noexcept {
    return value.data() ? std::span<const BindingValue>(value.data()->items)
                        : std::span<const BindingValue>{};
}

namespace {

void bump(std::uint64_t& value) noexcept {
    ++value;
    if (value == 0U) ++value;
}

template <typename Number>
std::optional<Number> parse_number(std::string_view text) noexcept {
    if (text.empty()) return std::nullopt;
    if constexpr (std::is_floating_point_v<Number>) {
        Number result{};
        try {
            std::istringstream input(std::string{text});
            input.imbue(std::locale::classic());
            input >> std::noskipws >> result;
            if (!input || input.peek() != std::char_traits<char>::eof()) {
                return std::nullopt;
            }
        } catch (...) {
            return std::nullopt;
        }
        if (!std::isfinite(result)) return std::nullopt;
        return result;
    } else {
        Number result{};
        const char* begin = text.data();
        const char* end = begin + text.size();
        const auto parsed = std::from_chars(begin, end, result);
        if (parsed.ec != std::errc{} || parsed.ptr != end) return std::nullopt;
        return result;
    }
}

std::optional<unsigned> fixed_precision(std::string_view format) noexcept {
    if (format.size() < 2U || (format.front() != 'F' && format.front() != 'f')) {
        return std::nullopt;
    }
    unsigned precision{};
    const auto parsed = std::from_chars(
        format.data() + 1, format.data() + format.size(), precision);
    if (parsed.ec != std::errc{} || parsed.ptr != format.data() + format.size() ||
        precision > 12U) {
        return std::nullopt;
    }
    return precision;
}

BindingValue apply_format_string(const BindingValue& value,
                                 std::string_view format) {
    if (format.empty()) return value;
    const auto precision = fixed_precision(format);
    if (!precision) {
        throw std::invalid_argument(
            "GUI.Forms native binding currently supports invariant F0..F12 formats");
    }
    const auto number = binding_value_to_number(value);
    if (!number) {
        throw std::invalid_argument(
            "GUI.Forms fixed binding format requires a numeric source value");
    }
    std::ostringstream output;
    output.setf(std::ios::fixed, std::ios::floatfield);
    output.precision(static_cast<std::streamsize>(*precision));
    output << *number;
    return output.str();
}

std::string failure_text(std::string_view direction,
                         std::string_view property,
                         std::string_view field) {
    return "GUI.Forms binding " + std::string(direction) + " failed for " +
        std::string(property) + " <-/-> " + std::string(field);
}

bool valid_data_source_update_mode(DataSourceUpdateMode mode) noexcept {
    return mode == DataSourceUpdateMode::on_validation ||
        mode == DataSourceUpdateMode::on_property_changed ||
        mode == DataSourceUpdateMode::never;
}

bool valid_control_update_mode(ControlUpdateMode mode) noexcept {
    return mode == ControlUpdateMode::on_property_changed ||
        mode == ControlUpdateMode::never;
}

void validate_options(const BindingOptions& options) {
    if (!valid_data_source_update_mode(options.data_source_update_mode) ||
        !valid_control_update_mode(options.control_update_mode)) {
        throw std::invalid_argument("GUI.Forms binding update mode is invalid");
    }
    if (!options.format_string.empty() && !fixed_precision(options.format_string)) {
        throw std::invalid_argument(
            "GUI.Forms native binding format must be invariant F0..F12");
    }
}

std::string_view trimmed(std::string_view value) noexcept {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1U);
}

bool ascii_name_equal(std::string_view left, std::string_view right) noexcept {
    left = trimmed(left);
    right = trimmed(right);
    if (left.size() != right.size()) return false;
    for (std::size_t index = 0; index < left.size(); ++index) {
        auto fold = [](unsigned char byte) noexcept {
            return byte >= 'A' && byte <= 'Z'
                ? static_cast<unsigned char>(byte + ('a' - 'A')) : byte;
        };
        if (fold(static_cast<unsigned char>(left[index])) !=
            fold(static_cast<unsigned char>(right[index]))) return false;
    }
    return true;
}

const PropertyEnumChoice* enum_choice_by_name(
    const PropertyEnumDescriptor& descriptor, std::string_view name) noexcept {
    const auto found = std::find_if(
        descriptor.choices.begin(), descriptor.choices.end(),
        [name](const PropertyEnumChoice& choice) {
            return ascii_name_equal(choice.name, name);
        });
    return found == descriptor.choices.end() ? nullptr : &*found;
}

const PropertyEnumChoice* enum_choice_by_value(
    const PropertyEnumDescriptor& descriptor, std::int64_t value) noexcept {
    const auto found = std::find_if(
        descriptor.choices.begin(), descriptor.choices.end(),
        [value](const PropertyEnumChoice& choice) {
            return choice.value == value;
        });
    return found == descriptor.choices.end() ? nullptr : &*found;
}

std::optional<std::int64_t> enum_numeric_value(
    const BindingValue& value,
    const PropertyEnumDescriptor& descriptor) noexcept {
    if (const auto* item = std::get_if<PropertyEnumValue>(&value)) {
        return item->type_name == descriptor.type_name
            ? std::optional<std::int64_t>{item->value} : std::nullopt;
    }
    if (const auto* item = std::get_if<std::int64_t>(&value)) return *item;
    if (const auto* item = std::get_if<std::uint64_t>(&value)) {
        return *item <= static_cast<std::uint64_t>(
                            std::numeric_limits<std::int64_t>::max())
            ? std::optional<std::int64_t>{static_cast<std::int64_t>(*item)}
            : std::nullopt;
    }
    if (const auto* item = std::get_if<double>(&value)) {
        if (!std::isfinite(*item) || std::trunc(*item) != *item ||
            *item < static_cast<double>(std::numeric_limits<std::int64_t>::min()) ||
            *item > static_cast<double>(std::numeric_limits<std::int64_t>::max())) {
            return std::nullopt;
        }
        return static_cast<std::int64_t>(*item);
    }
    if (const auto* item = std::get_if<std::string>(&value)) {
        if (!descriptor.flags) {
            const PropertyEnumChoice* choice = enum_choice_by_name(
                descriptor, *item);
            return choice ? std::optional<std::int64_t>{choice->value}
                          : std::nullopt;
        }
        std::uint64_t combined{};
        std::size_t offset{};
        bool saw_choice = false;
        while (offset <= item->size()) {
            const std::size_t separator = item->find_first_of(",|", offset);
            const std::string_view token = trimmed(std::string_view(*item).substr(
                offset, separator == std::string::npos
                    ? std::string::npos : separator - offset));
            if (token.empty()) return std::nullopt;
            const PropertyEnumChoice* choice = enum_choice_by_name(
                descriptor, token);
            if (!choice || choice->value < 0) return std::nullopt;
            combined |= static_cast<std::uint64_t>(choice->value);
            saw_choice = true;
            if (separator == std::string::npos) break;
            offset = separator + 1U;
        }
        return saw_choice && combined <= static_cast<std::uint64_t>(
                                         std::numeric_limits<std::int64_t>::max())
            ? std::optional<std::int64_t>{static_cast<std::int64_t>(combined)}
            : std::nullopt;
    }
    return std::nullopt;
}

std::optional<PropertyEnumValue> normalize_enum_value(
    const BindingValue& value,
    const PropertyEnumDescriptor& descriptor) {
    const auto numeric = enum_numeric_value(value, descriptor);
    if (!numeric) return std::nullopt;
    if (const PropertyEnumChoice* exact = enum_choice_by_value(
            descriptor, *numeric)) {
        return PropertyEnumValue{
            descriptor.type_name, exact->name, exact->value};
    }
    if (!descriptor.flags || *numeric < 0) return std::nullopt;
    std::uint64_t allowed{};
    for (const PropertyEnumChoice& choice : descriptor.choices) {
        if (choice.value > 0) {
            allowed |= static_cast<std::uint64_t>(choice.value);
        }
    }
    const auto bits = static_cast<std::uint64_t>(*numeric);
    if ((bits & ~allowed) != 0U) return std::nullopt;
    std::string name;
    for (const PropertyEnumChoice& choice : descriptor.choices) {
        if (choice.value <= 0 ||
            !std::has_single_bit(static_cast<std::uint64_t>(choice.value)) ||
            (bits & static_cast<std::uint64_t>(choice.value)) == 0U) continue;
        if (!name.empty()) name += ", ";
        name += choice.name;
    }
    if (name.empty()) name = std::to_string(*numeric);
    return PropertyEnumValue{descriptor.type_name, std::move(name), *numeric};
}

bool valid_nested_kind(BindingValueKind kind) noexcept {
    return kind == BindingValueKind::boolean ||
        kind == BindingValueKind::signed_integer ||
        kind == BindingValueKind::unsigned_integer ||
        kind == BindingValueKind::number ||
        kind == BindingValueKind::text ||
        kind == BindingValueKind::point ||
        kind == BindingValueKind::size ||
        kind == BindingValueKind::rectangle ||
        kind == BindingValueKind::insets ||
        kind == BindingValueKind::color ||
        kind == BindingValueKind::font ||
        kind == BindingValueKind::image ||
        kind == BindingValueKind::enumeration ||
        kind == BindingValueKind::object ||
        kind == BindingValueKind::collection;
}

bool valid_property_value_tree_impl(const BindingValue& value,
                                    std::size_t depth,
                                    std::size_t& nodes) {
    if (++nodes > maximum_property_value_nodes ||
        depth > maximum_property_value_depth) {
        return false;
    }
    if (const auto* object = std::get_if<PropertyObjectValue>(&value)) {
        if (!*object || object->type_name().empty() ||
            object->type_name().size() > 256U ||
            trimmed(object->type_name()).size() != object->type_name().size() ||
            !validate_utf8(object->type_name()).valid() ||
            object->members().size() > maximum_property_object_members) {
            return false;
        }
        std::set<std::string> identities;
        for (const PropertyObjectMember& member : object->members()) {
            const BindingValueKind runtime_kind = binding_value_kind(member.value);
            const BindingValueKind declared_kind =
                member.declared_kind.value_or(runtime_kind);
            const bool schema_valid = valid_nested_kind(declared_kind) &&
                (runtime_kind != BindingValueKind::null
                     ? runtime_kind == declared_kind
                     : member.nullable && member.declared_kind.has_value()) &&
                ((declared_kind == BindingValueKind::enumeration) ==
                 static_cast<bool>(member.enumeration)) &&
                (!member.enumeration ||
                 valid_property_enum_descriptor(*member.enumeration)) &&
                member.standard_values.size() <=
                    maximum_property_standard_values &&
                (!member.standard_values_exclusive ||
                 !member.standard_values.empty()) &&
                member.converter_name.size() <= 256U &&
                member.editor_name.size() <= 256U &&
                validate_utf8(member.converter_name).valid() &&
                validate_utf8(member.editor_name).valid();
            if (trimmed(member.name).size() != member.name.size() ||
                member.name.empty() || member.name.size() > 256U ||
                !validate_utf8(member.name).valid() ||
                !validate_utf8(member.description).valid() ||
                !identities.insert(canonical_binding_name(member.name)).second ||
                !schema_valid ||
                !valid_property_value_tree_impl(member.value, depth + 1U,
                                                nodes)) {
                return false;
            }
            PropertyDescriptor member_descriptor;
            member_descriptor.kind = declared_kind;
            member_descriptor.nullable = member.nullable;
            member_descriptor.enumeration = member.enumeration;
            member_descriptor.standard_values = member.standard_values;
            member_descriptor.standard_values_exclusive =
                member.standard_values_exclusive;
            std::vector<BindingValue> normalized;
            normalized.reserve(member.standard_values.size());
            for (const BindingValue& standard : member.standard_values) {
                const auto converted = convert_property_value(
                    standard, member_descriptor);
                if (!converted ||
                    std::find(normalized.begin(), normalized.end(),
                              *converted) != normalized.end()) {
                    return false;
                }
                normalized.push_back(*converted);
            }
        }
    } else if (const auto* collection =
                   std::get_if<PropertyCollectionValue>(&value)) {
        const auto items = property_collection_items(*collection);
        if (!*collection || collection->item_type_name().empty() ||
            collection->item_type_name().size() > 256U ||
            trimmed(collection->item_type_name()).size() !=
                collection->item_type_name().size() ||
            !validate_utf8(collection->item_type_name()).valid() ||
            !valid_nested_kind(collection->item_kind()) ||
            items.size() > maximum_property_collection_items) {
            return false;
        }
        for (const BindingValue& item : items) {
            if (binding_value_kind(item) != collection->item_kind() ||
                !valid_property_value_tree_impl(item, depth + 1U, nodes)) {
                return false;
            }
        }
    } else if (!convert_binding_value(value, binding_value_kind(value))) {
        return false;
    }
    return true;
}

} // namespace

namespace detail {

void validate_binding_options(const BindingOptions& options) {
    validate_options(options);
}

void bump_counter(std::uint64_t& value) noexcept {
    bump(value);
}

BindingValue format_binding_value(const BindingValue& value,
                                  std::string_view format) {
    return apply_format_string(value, format);
}

std::string binding_failure_text(std::string_view direction,
                                 std::string_view property,
                                 std::string_view member) {
    return failure_text(direction, property, member);
}

} // namespace detail

BindingValueKind binding_value_kind(const BindingValue& value) noexcept {
    return std::visit([](const auto& item) {
        using Type = std::decay_t<decltype(item)>;
        if constexpr (std::is_same_v<Type, std::monostate>) {
            return BindingValueKind::null;
        } else if constexpr (std::is_same_v<Type, bool>) {
            return BindingValueKind::boolean;
        } else if constexpr (std::is_same_v<Type, std::int64_t>) {
            return BindingValueKind::signed_integer;
        } else if constexpr (std::is_same_v<Type, std::uint64_t>) {
            return BindingValueKind::unsigned_integer;
        } else if constexpr (std::is_same_v<Type, double>) {
            return BindingValueKind::number;
        } else if constexpr (std::is_same_v<Type, std::string>) {
            return BindingValueKind::text;
        } else if constexpr (std::is_same_v<Type, Point>) {
            return BindingValueKind::point;
        } else if constexpr (std::is_same_v<Type, Size>) {
            return BindingValueKind::size;
        } else if constexpr (std::is_same_v<Type, Rect>) {
            return BindingValueKind::rectangle;
        } else if constexpr (std::is_same_v<Type, Insets>) {
            return BindingValueKind::insets;
        } else if constexpr (std::is_same_v<Type, Color>) {
            return BindingValueKind::color;
        } else if constexpr (std::is_same_v<Type, FontSpec>) {
            return BindingValueKind::font;
        } else if constexpr (std::is_same_v<Type, ImageId>) {
            return BindingValueKind::image;
        } else if constexpr (std::is_same_v<Type, PropertyEnumValue>) {
            return BindingValueKind::enumeration;
        } else if constexpr (std::is_same_v<Type, PropertyObjectValue>) {
            return BindingValueKind::object;
        } else {
            return BindingValueKind::collection;
        }
    }, value);
}

std::string binding_value_to_string(const BindingValue& value) {
    return std::visit([](const auto& item) -> std::string {
        using Type = std::decay_t<decltype(item)>;
        if constexpr (std::is_same_v<Type, std::monostate>) {
            return {};
        } else if constexpr (std::is_same_v<Type, bool>) {
            return item ? "true" : "false";
        } else if constexpr (std::is_same_v<Type, std::string>) {
            return item;
        } else if constexpr (std::is_arithmetic_v<Type>) {
            std::ostringstream output;
            output.imbue(std::locale::classic());
            output.precision(std::numeric_limits<double>::max_digits10);
            output << item;
            return output.str();
        } else if constexpr (std::is_same_v<Type, Point>) {
            return std::to_string(item.x) + "," + std::to_string(item.y);
        } else if constexpr (std::is_same_v<Type, Size>) {
            return std::to_string(item.width) + "," +
                   std::to_string(item.height);
        } else if constexpr (std::is_same_v<Type, Rect>) {
            return std::to_string(item.x) + "," + std::to_string(item.y) +
                   "," + std::to_string(item.width) + "," +
                   std::to_string(item.height);
        } else if constexpr (std::is_same_v<Type, Insets>) {
            return std::to_string(item.left) + "," + std::to_string(item.top) +
                   "," + std::to_string(item.right) + "," +
                   std::to_string(item.bottom);
        } else if constexpr (std::is_same_v<Type, Color>) {
            std::ostringstream output;
            output << '#' << std::hex << std::uppercase << std::setfill('0')
                   << std::setw(2) << static_cast<unsigned>(item.red)
                   << std::setw(2) << static_cast<unsigned>(item.green)
                   << std::setw(2) << static_cast<unsigned>(item.blue)
                   << std::setw(2) << static_cast<unsigned>(item.alpha);
            return output.str();
        } else if constexpr (std::is_same_v<Type, FontSpec>) {
            return "font(" + std::to_string(static_cast<unsigned>(item.role)) +
                "," + std::to_string(item.size) + "," +
                std::to_string(item.weight) + "," +
                (item.italic ? "italic" : "regular") + "," +
                std::to_string(item.letter_spacing) + ")";
        } else if constexpr (std::is_same_v<Type, ImageId>) {
            return "image:" + std::to_string(item.value);
        } else if constexpr (std::is_same_v<Type, PropertyEnumValue>) {
            return item.name.empty() ? std::to_string(item.value) : item.name;
        } else if constexpr (std::is_same_v<Type, PropertyObjectValue>) {
            return item ? std::string(item.type_name()) + " {" +
                    std::to_string(item.members().size()) + "}"
                : std::string("<invalid object>");
        } else {
            return item ? std::string(item.item_type_name()) + " [" +
                    std::to_string(property_collection_items(item).size()) + "]"
                : std::string("<invalid collection>");
        }
    }, value);
}

std::optional<bool> binding_value_to_bool(const BindingValue& value) noexcept {
    return std::visit([](const auto& item) -> std::optional<bool> {
        using Type = std::decay_t<decltype(item)>;
        if constexpr (std::is_same_v<Type, std::monostate>) {
            return std::nullopt;
        } else if constexpr (std::is_same_v<Type, bool>) {
            return item;
        } else if constexpr (std::is_same_v<Type, std::string>) {
            std::string lowered;
            lowered.reserve(item.size());
            for (const unsigned char byte : item) {
                lowered.push_back(byte >= 'A' && byte <= 'Z'
                    ? static_cast<char>(byte + ('a' - 'A'))
                    : static_cast<char>(byte));
            }
            if (lowered == "true" || lowered == "1") return true;
            if (lowered == "false" || lowered == "0") return false;
            return std::nullopt;
        } else if constexpr (std::is_floating_point_v<Type>) {
            return std::isfinite(item) ? std::optional<bool>{item != 0.0}
                                       : std::nullopt;
        } else if constexpr (std::is_integral_v<Type>) {
            return item != 0;
        } else {
            return std::nullopt;
        }
    }, value);
}

std::optional<std::int64_t> binding_value_to_signed(
    const BindingValue& value) noexcept {
    return std::visit([](const auto& item) -> std::optional<std::int64_t> {
        using Type = std::decay_t<decltype(item)>;
        if constexpr (std::is_same_v<Type, std::monostate>) {
            return std::nullopt;
        } else if constexpr (std::is_same_v<Type, std::string>) {
            return parse_number<std::int64_t>(item);
        } else if constexpr (std::is_same_v<Type, std::uint64_t>) {
            if (item > static_cast<std::uint64_t>(
                    std::numeric_limits<std::int64_t>::max())) return std::nullopt;
            return static_cast<std::int64_t>(item);
        } else if constexpr (std::is_same_v<Type, double>) {
            if (!std::isfinite(item) || std::trunc(item) != item ||
                item < static_cast<double>(std::numeric_limits<std::int64_t>::min()) ||
                item > static_cast<double>(std::numeric_limits<std::int64_t>::max())) {
                return std::nullopt;
            }
            return static_cast<std::int64_t>(item);
        } else if constexpr (std::is_same_v<Type, std::int64_t> ||
                             std::is_same_v<Type, bool>) {
            return static_cast<std::int64_t>(item);
        } else if constexpr (std::is_same_v<Type, PropertyEnumValue>) {
            return item.value;
        } else {
            return std::nullopt;
        }
    }, value);
}

std::optional<std::uint64_t> binding_value_to_unsigned(
    const BindingValue& value) noexcept {
    return std::visit([](const auto& item) -> std::optional<std::uint64_t> {
        using Type = std::decay_t<decltype(item)>;
        if constexpr (std::is_same_v<Type, std::monostate>) {
            return std::nullopt;
        } else if constexpr (std::is_same_v<Type, std::string>) {
            return parse_number<std::uint64_t>(item);
        } else if constexpr (std::is_same_v<Type, std::int64_t>) {
            return item < 0 ? std::nullopt
                            : std::optional<std::uint64_t>{
                                  static_cast<std::uint64_t>(item)};
        } else if constexpr (std::is_same_v<Type, double>) {
            if (!std::isfinite(item) || std::trunc(item) != item || item < 0.0 ||
                item > static_cast<double>(std::numeric_limits<std::uint64_t>::max())) {
                return std::nullopt;
            }
            return static_cast<std::uint64_t>(item);
        } else if constexpr (std::is_same_v<Type, std::uint64_t> ||
                             std::is_same_v<Type, bool>) {
            return static_cast<std::uint64_t>(item);
        } else if constexpr (std::is_same_v<Type, ImageId>) {
            return item.value;
        } else if constexpr (std::is_same_v<Type, PropertyEnumValue>) {
            return item.value < 0 ? std::nullopt
                : std::optional<std::uint64_t>{
                      static_cast<std::uint64_t>(item.value)};
        } else {
            return std::nullopt;
        }
    }, value);
}

std::optional<double> binding_value_to_number(const BindingValue& value) noexcept {
    return std::visit([](const auto& item) -> std::optional<double> {
        using Type = std::decay_t<decltype(item)>;
        if constexpr (std::is_same_v<Type, std::monostate>) {
            return std::nullopt;
        } else if constexpr (std::is_same_v<Type, std::string>) {
            return parse_number<double>(item);
        } else if constexpr (std::is_same_v<Type, bool> ||
                             std::is_same_v<Type, std::int64_t> ||
                             std::is_same_v<Type, std::uint64_t> ||
                             std::is_same_v<Type, double>) {
            const double result = static_cast<double>(item);
            return std::isfinite(result) ? std::optional<double>{result}
                                         : std::nullopt;
        } else {
            return std::nullopt;
        }
    }, value);
}

std::optional<BindingValue> convert_binding_value(
    const BindingValue& value, BindingValueKind target_kind) {
    if (target_kind == BindingValueKind::null) return BindingValue{};
    if (binding_value_kind(value) == target_kind) {
        switch (target_kind) {
        case BindingValueKind::null:
        case BindingValueKind::boolean:
        case BindingValueKind::signed_integer:
        case BindingValueKind::unsigned_integer:
        case BindingValueKind::text:
        case BindingValueKind::color:
        case BindingValueKind::image:
            return value;
        case BindingValueKind::number:
            return std::isfinite(std::get<double>(value))
                ? std::optional<BindingValue>{value} : std::nullopt;
        case BindingValueKind::point: {
            const Point item = std::get<Point>(value);
            return std::isfinite(item.x) && std::isfinite(item.y)
                ? std::optional<BindingValue>{value} : std::nullopt;
        }
        case BindingValueKind::size: {
            const Size item = std::get<Size>(value);
            return std::isfinite(item.width) && std::isfinite(item.height) &&
                    item.width >= 0.0 && item.height >= 0.0
                ? std::optional<BindingValue>{value} : std::nullopt;
        }
        case BindingValueKind::rectangle: {
            const Rect item = std::get<Rect>(value);
            return item.finite() && item.width >= 0.0 && item.height >= 0.0
                ? std::optional<BindingValue>{value} : std::nullopt;
        }
        case BindingValueKind::insets: {
            const Insets item = std::get<Insets>(value);
            return std::isfinite(item.left) && std::isfinite(item.top) &&
                    std::isfinite(item.right) && std::isfinite(item.bottom)
                ? std::optional<BindingValue>{value} : std::nullopt;
        }
        case BindingValueKind::font: {
            const FontSpec item = std::get<FontSpec>(value);
            const bool valid_role = item.role == FontRole::control ||
                item.role == FontRole::content ||
                item.role == FontRole::monospace;
            return valid_role && valid_font_spec(item)
                ? std::optional<BindingValue>{value} : std::nullopt;
        }
        case BindingValueKind::enumeration:
            return !std::get<PropertyEnumValue>(value).type_name.empty()
                ? std::optional<BindingValue>{value} : std::nullopt;
        case BindingValueKind::object:
        case BindingValueKind::collection:
            return valid_property_value_tree(value)
                ? std::optional<BindingValue>{value} : std::nullopt;
        }
        return std::nullopt;
    }
    switch (target_kind) {
    case BindingValueKind::null: return BindingValue{};
    case BindingValueKind::boolean:
        if (const auto converted = binding_value_to_bool(value)) {
            return BindingValue{*converted};
        }
        break;
    case BindingValueKind::signed_integer:
        if (const auto converted = binding_value_to_signed(value)) {
            return BindingValue{*converted};
        }
        break;
    case BindingValueKind::unsigned_integer:
        if (const auto converted = binding_value_to_unsigned(value)) {
            return BindingValue{*converted};
        }
        break;
    case BindingValueKind::number:
        if (const auto converted = binding_value_to_number(value)) {
            return BindingValue{*converted};
        }
        break;
    case BindingValueKind::text:
        return BindingValue{binding_value_to_string(value)};
    case BindingValueKind::image:
        if (const auto converted = binding_value_to_unsigned(value)) {
            return BindingValue{ImageId{*converted}};
        }
        break;
    case BindingValueKind::point:
    case BindingValueKind::size:
    case BindingValueKind::rectangle:
    case BindingValueKind::insets:
    case BindingValueKind::color:
    case BindingValueKind::font:
    case BindingValueKind::enumeration:
    case BindingValueKind::object:
    case BindingValueKind::collection:
        break;
    }
    return std::nullopt;
}

bool valid_property_value_tree(const BindingValue& value) noexcept {
    try {
        std::size_t nodes{};
        return valid_property_value_tree_impl(value, 0U, nodes);
    } catch (...) {
        return false;
    }
}

PropertyObjectValue make_property_object(
    std::string type_name, std::vector<PropertyObjectMember> members) {
    auto data = std::make_shared<PropertyObjectData>();
    data->type_name = std::move(type_name);
    data->members = std::move(members);
    PropertyObjectValue result = PropertyValueFactoryAccess::object(
        std::move(data));
    if (!valid_property_value_tree(BindingValue{result})) {
        throw std::invalid_argument(
            "GUI.Forms property object schema/value tree is invalid or exceeds its bound");
    }
    return result;
}

PropertyCollectionValue make_property_collection(
    std::string item_type_name, BindingValueKind item_kind,
    std::vector<BindingValue> items) {
    if (!valid_nested_kind(item_kind)) {
        throw std::invalid_argument(
            "GUI.Forms property collection requires one non-null item kind");
    }
    for (BindingValue& item : items) {
        const auto converted = convert_binding_value(item, item_kind);
        if (!converted) {
            throw std::invalid_argument(
                "GUI.Forms property collection item cannot convert to its declared kind");
        }
        item = *converted;
    }
    auto data = std::make_shared<PropertyCollectionData>();
    data->item_type_name = std::move(item_type_name);
    data->item_kind = item_kind;
    data->items = std::move(items);
    PropertyCollectionValue result = PropertyValueFactoryAccess::collection(
        std::move(data));
    if (!valid_property_value_tree(BindingValue{result})) {
        throw std::invalid_argument(
            "GUI.Forms property collection schema/value tree is invalid or exceeds its bound");
    }
    return result;
}

bool valid_property_enum_descriptor(
    const PropertyEnumDescriptor& descriptor) noexcept {
    if (trimmed(descriptor.type_name).size() != descriptor.type_name.size() ||
        descriptor.type_name.empty() ||
        descriptor.type_name.size() > maximum_property_enum_text_bytes ||
        !validate_utf8(descriptor.type_name).valid() ||
        descriptor.choices.empty() ||
        descriptor.choices.size() > maximum_property_enum_choices) {
        return false;
    }
    for (std::size_t index = 0; index < descriptor.choices.size(); ++index) {
        const PropertyEnumChoice& choice = descriptor.choices[index];
        if (trimmed(choice.name).size() != choice.name.size() ||
            choice.name.empty() ||
            choice.name.size() > maximum_property_enum_text_bytes ||
            !validate_utf8(choice.name).valid()) return false;
        for (std::size_t peer = index + 1U;
             peer < descriptor.choices.size(); ++peer) {
            if (ascii_name_equal(choice.name,
                                 descriptor.choices[peer].name)) return false;
        }
    }
    return true;
}

std::optional<BindingValue> convert_property_value(
    const BindingValue& value, const PropertyDescriptor& descriptor) {
    const auto normalize = [&descriptor](const BindingValue& candidate)
            -> std::optional<BindingValue> {
        if (binding_value_kind(candidate) == BindingValueKind::null) {
            return descriptor.nullable
                ? std::optional<BindingValue>{BindingValue{std::monostate{}}}
                : std::optional<BindingValue>{};
        }
        if (descriptor.kind != BindingValueKind::enumeration) {
            if (descriptor.enumeration) return {};
            return convert_binding_value(candidate, descriptor.kind);
        }
        if (!descriptor.enumeration ||
            !valid_property_enum_descriptor(*descriptor.enumeration)) {
            return {};
        }
        const auto enumeration = normalize_enum_value(
            candidate, *descriptor.enumeration);
        return enumeration
            ? std::optional<BindingValue>{BindingValue{*enumeration}}
            : std::optional<BindingValue>{};
    };
    const auto normalized = normalize(value);
    if (!normalized || !descriptor.standard_values_exclusive) {
        return normalized;
    }
    for (const BindingValue& standard : descriptor.standard_values) {
        const auto normalized_standard = normalize(standard);
        if (normalized_standard && *normalized_standard == *normalized) {
            return normalized;
        }
    }
    return {};
}

std::string canonical_binding_name(std::string_view name) {
    const auto first = name.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) {
        throw std::invalid_argument("GUI.Forms binding name may not be empty");
    }
    const auto last = name.find_last_not_of(" \t\r\n");
    std::string result(name.substr(first, last - first + 1U));
    for (char& value : result) {
        const unsigned char byte = static_cast<unsigned char>(value);
        if (byte >= 'A' && byte <= 'Z') {
            value = static_cast<char>(byte + ('a' - 'A'));
        }
    }
    return result;
}

} // namespace gui_forms
