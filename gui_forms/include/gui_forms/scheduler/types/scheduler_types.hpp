#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

namespace gui_forms {

class Window;

namespace detail {
class ScheduledFrameRequest;
struct WindowLifetime final {
    Window* window{};
};
}

using FrameClock = std::chrono::steady_clock;
using FrameTime = FrameClock::time_point;
using FrameInterval = FrameClock::duration;

inline constexpr std::size_t maximum_scheduled_frame_requests = 256;
inline constexpr std::size_t maximum_active_surfaces = 32;
inline constexpr FrameInterval minimum_active_surface_interval =
    std::chrono::milliseconds(8);
inline constexpr FrameInterval minimum_ui_timer_interval =
    std::chrono::milliseconds(1);

struct FramePollResult final {
    std::uint64_t deadlines_fired{};
    std::uint64_t active_surface_ticks{};
    std::uint64_t ui_timer_ticks{};
    std::uint64_t coalesced_requests{};
    // Throwing callbacks are disconnected individually so one animated
    // surface cannot repeatedly escape through a native timer callback.
    std::uint64_t callback_faults{};
    bool reentrant_poll_deferred{};
    bool damage_pending{};
    bool suppressed_by_occlusion{};
    std::optional<FrameTime> next_wake;
};

} // namespace gui_forms
