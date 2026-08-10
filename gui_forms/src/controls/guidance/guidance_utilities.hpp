#pragma once

#include "gui_forms/components/error_provider/error_provider.hpp"
#include "gui_forms/components/help_provider/help_provider.hpp"

#include <atomic>

namespace gui_forms::guidance_detail {

inline std::atomic<std::uint64_t> next_error_provider_id{1U};
inline std::atomic<std::uint64_t> next_help_provider_id{1U};
inline constexpr std::size_t changed_error_blink_transitions = 6U;

[[nodiscard]] inline bool valid_alignment(
    ErrorIconAlignment alignment) noexcept {
    switch (alignment) {
    case ErrorIconAlignment::top_left:
    case ErrorIconAlignment::top_right:
    case ErrorIconAlignment::middle_left:
    case ErrorIconAlignment::middle_right:
    case ErrorIconAlignment::bottom_left:
    case ErrorIconAlignment::bottom_right:
        return true;
    }
    return false;
}

[[nodiscard]] inline bool valid_blink_style(ErrorBlinkStyle style) noexcept {
    switch (style) {
    case ErrorBlinkStyle::blink_if_different_error:
    case ErrorBlinkStyle::always_blink:
    case ErrorBlinkStyle::never_blink:
        return true;
    }
    return false;
}

[[nodiscard]] inline bool valid_help_navigator(HelpNavigator value) noexcept {
    switch (value) {
    case HelpNavigator::topic:
    case HelpNavigator::table_of_contents:
    case HelpNavigator::index:
    case HelpNavigator::find:
    case HelpNavigator::associate_index:
    case HelpNavigator::keyword_index:
        return true;
    }
    return false;
}

} // namespace gui_forms::guidance_detail
