#pragma once

#include <cstdint>

namespace gui_forms {

enum class HostCapability : std::uint64_t {
    none = 0,
    lifecycle = 1ULL << 0U,
    scale_notifications = 1ULL << 1U,
    monitor_geometry = 1ULL << 2U,
    occlusion = 1ULL << 3U,
    scheduled_wake = 1ULL << 4U,
    pointer_input = 1ULL << 5U,
    keyboard_input = 1ULL << 6U,
    text_composition = 1ULL << 7U,
    pointer_capture = 1ULL << 8U,
    cursor = 1ULL << 9U,
    clipboard = 1ULL << 10U,
    typed_drag_destination = 1ULL << 11U,
    dialogs = 1ULL << 12U,
    menus = 1ULL << 13U,
    font_discovery = 1ULL << 14U,
    accessibility = 1ULL << 15U,
    typed_drag_source = 1ULL << 16U,
    sound_cues = 1ULL << 17U,
    clipboard_images = 1ULL << 18U,
};

[[nodiscard]] constexpr HostCapability operator|(HostCapability left,
                                                  HostCapability right) noexcept {
    return static_cast<HostCapability>(static_cast<std::uint64_t>(left) |
                                       static_cast<std::uint64_t>(right));
}

[[nodiscard]] constexpr bool has_capability(HostCapability available,
                                            HostCapability requested) noexcept {
    return (static_cast<std::uint64_t>(available) &
            static_cast<std::uint64_t>(requested)) ==
           static_cast<std::uint64_t>(requested);
}

} // namespace gui_forms
