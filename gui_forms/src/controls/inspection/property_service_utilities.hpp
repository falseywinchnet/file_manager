#pragma once

#include "gui_forms/inspection/property_value_converter_registry/property_value_converter_registry.hpp"
#include "gui_forms/text.hpp"

#include <stdexcept>
#include <string>
#include <string_view>

namespace gui_forms::detail {

inline std::string property_service_name(std::string_view name) {
    if (name.empty() || name.size() > 256U || !validate_utf8(name).valid()) {
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

inline void require_conversion_context(
    const PropertyConversionContext& context) {
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

inline void replace_text(std::string& text, std::string_view from,
                         std::string_view to) {
    if (from.empty()) return;
    std::size_t position = 0U;
    while ((position = text.find(from, position)) != std::string::npos) {
        text.replace(position, from.size(), to);
        position += to.size();
    }
}

inline std::string localize_number(
    std::string text, const PropertyConversionContext& context) {
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

inline std::optional<std::string> invariant_number_text(
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

} // namespace gui_forms::detail
