#pragma once

#include <cmath>
#include <cstdint>
#include <string>
#include <string_view>

namespace gui_forms::scrolling_detail {

inline constexpr double comparison_epsilon = 0.0001;

[[nodiscard]] inline std::uint64_t virtual_runtime_id(
    std::string_view stable_id) noexcept {
    std::uint64_t value = 1469598103934665603ULL;
    for (const unsigned char byte : stable_id) {
        value ^= byte;
        value *= 1099511628211ULL;
    }
    return value | (std::uint64_t{1} << 63U);
}

[[nodiscard]] inline bool approximately_greater(
    double left, double right) noexcept {
    return left > right + comparison_epsilon;
}

[[nodiscard]] inline bool parse_finite_number(
    std::string_view text, double& value) noexcept {
    try {
        std::size_t consumed{};
        const std::string copy(text);
        const double parsed = std::stod(copy, &consumed);
        if (consumed != copy.size() || !std::isfinite(parsed)) return false;
        value = parsed;
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace gui_forms::scrolling_detail
