#pragma once

#include "gui_forms/events.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace gui_forms::input_control_detail {

[[nodiscard]] inline std::uint64_t virtual_semantic_runtime_id(
    std::string_view stable_id) noexcept {
    std::uint64_t value = 1469598103934665603ULL;
    for (const unsigned char byte : stable_id) {
        value ^= byte;
        value *= 1099511628211ULL;
    }
    return value | (std::uint64_t{1} << 63U);
}

[[nodiscard]] inline bool includes(Modifier value, Modifier requested) noexcept {
    return (static_cast<std::uint8_t>(value) &
            static_cast<std::uint8_t>(requested)) != 0U;
}

[[nodiscard]] inline bool command_modifier(Modifier value) noexcept {
    return includes(value, Modifier::control) || includes(value, Modifier::meta);
}

enum class WordClass : std::uint8_t {
    spacing,
    word,
    punctuation,
};

[[nodiscard]] inline bool unicode_spacing(char32_t value) noexcept {
    return value == U' ' || (value >= U'\t' && value <= U'\r') ||
           value == U'\u0085' || value == U'\u00a0' || value == U'\u1680' ||
           (value >= U'\u2000' && value <= U'\u200a') ||
           value == U'\u2028' || value == U'\u2029' || value == U'\u202f' ||
           value == U'\u205f' || value == U'\u3000';
}

[[nodiscard]] inline bool unicode_punctuation_or_symbol(char32_t value) noexcept {
    if (value < U'\u0080') {
        return !((value >= U'a' && value <= U'z') ||
                 (value >= U'A' && value <= U'Z') ||
                 (value >= U'0' && value <= U'9') || value == U'_');
    }
    return (value >= U'\u2000' && value <= U'\u206f') ||
           (value >= U'\u2190' && value <= U'\u2bff') ||
           (value >= U'\u3001' && value <= U'\u303f') ||
           (value >= U'\ufe10' && value <= U'\ufe1f') ||
           (value >= U'\ufe30' && value <= U'\ufe4f') ||
           (value >= U'\uff01' && value <= U'\uff0f') ||
           (value >= U'\uff1a' && value <= U'\uff20') ||
           (value >= U'\uff3b' && value <= U'\uff40') ||
           (value >= U'\uff5b' && value <= U'\uff65') ||
           (value >= U'\U0001f000' && value <= U'\U0001faff');
}

[[nodiscard]] inline WordClass word_class(char32_t value) noexcept {
    if (unicode_spacing(value)) return WordClass::spacing;
    return unicode_punctuation_or_symbol(value)
        ? WordClass::punctuation : WordClass::word;
}

[[nodiscard]] inline bool valid_password_character(char32_t value) noexcept {
    return value == U'\0' ||
        (value >= U' ' && value <= U'\U0010ffff' &&
         !(value >= static_cast<char32_t>(0xd800U) &&
           value <= static_cast<char32_t>(0xdfffU)) &&
         value != U'\u2028' && value != U'\u2029');
}

[[nodiscard]] inline std::string utf8_scalar(char32_t value) {
    std::string result;
    if (value <= U'\u007f') {
        result.push_back(static_cast<char>(value));
    } else if (value <= U'\u07ff') {
        result.push_back(static_cast<char>(0xc0U | (value >> 6U)));
        result.push_back(static_cast<char>(0x80U | (value & 0x3fU)));
    } else if (value <= U'\uffff') {
        result.push_back(static_cast<char>(0xe0U | (value >> 12U)));
        result.push_back(static_cast<char>(0x80U | ((value >> 6U) & 0x3fU)));
        result.push_back(static_cast<char>(0x80U | (value & 0x3fU)));
    } else {
        result.push_back(static_cast<char>(0xf0U | (value >> 18U)));
        result.push_back(static_cast<char>(0x80U | ((value >> 12U) & 0x3fU)));
        result.push_back(static_cast<char>(0x80U | ((value >> 6U) & 0x3fU)));
        result.push_back(static_cast<char>(0x80U | (value & 0x3fU)));
    }
    return result;
}

} // namespace gui_forms::input_control_detail
