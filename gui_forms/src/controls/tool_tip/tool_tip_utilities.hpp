#pragma once

#include "gui_forms/types.hpp"

#include <algorithm>
#include <cmath>
#include <string_view>

namespace gui_forms::tool_tip_detail {

[[nodiscard]] inline Size tool_tip_size(std::string_view text,
                                        double maximum_text_width) noexcept {
    constexpr double advance = 6.8;
    constexpr double line_height = 17.0;
    std::size_t longest = 0U;
    std::size_t lines = 1U;
    std::size_t current = 0U;
    for (char value : text) {
        if (value == '\n') {
            longest = std::max(longest, current);
            current = 0U;
            ++lines;
        } else {
            ++current;
        }
    }
    longest = std::max(longest, current);
    const double unwrapped = static_cast<double>(longest) * advance;
    const double text_width = std::clamp(unwrapped, 60.0, maximum_text_width);
    if (unwrapped > maximum_text_width) {
        lines += static_cast<std::size_t>(
                     std::ceil(unwrapped / maximum_text_width)) -
                 1U;
    }
    return {text_width + 20.0,
            static_cast<double>(lines) * line_height + 12.0};
}

} // namespace gui_forms::tool_tip_detail
