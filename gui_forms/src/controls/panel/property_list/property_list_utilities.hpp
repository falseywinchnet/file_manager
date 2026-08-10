#pragma once

#include "gui_forms/text.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

namespace gui_forms::detail {

inline constexpr double property_group_height = 25.0;
inline constexpr double property_row_height = 29.0;
inline constexpr double property_editor_height = 34.0;
inline constexpr double property_validation_height = 18.0;
inline constexpr double property_padding = 8.0;
inline constexpr double property_gap = 7.0;

inline std::uint64_t property_virtual_runtime_id(
    std::string_view id) noexcept {
    std::uint64_t hash = 1469598103934665603ULL;
    for (const unsigned char byte : id) {
        hash ^= byte;
        hash *= 1099511628211ULL;
    }
    return hash | (1ULL << 63U);
}

inline void require_property_text(std::string_view value, const char* field) {
    if (!validate_utf8(value).valid()) {
        throw std::invalid_argument(std::string("PropertyList ") + field +
                                    " must be valid UTF-8");
    }
}

} // namespace gui_forms::detail
