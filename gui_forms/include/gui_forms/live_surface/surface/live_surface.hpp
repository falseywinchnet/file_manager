#pragma once

#include "gui_forms/live_surface/frame/live_surface_frame.hpp"
#include "gui_forms/live_surface/types/live_surface_types.hpp"
#include "gui_forms/live_surface/wake_connection/live_surface_wake_connection.hpp"
#include "gui_forms/live_surface/write_lease/live_surface_write_lease.hpp"

#include <functional>
#include <memory>

namespace gui_forms {

namespace detail {
struct LiveSurfaceState;
}

// Renderer-neutral latest-frame surface. It retains no window, input target,
// application callback, or presentation queue. A producer publishes complete
// candidates; terminal painters acquire only the newest immutable candidate.
class LiveSurface final : public std::enable_shared_from_this<LiveSurface> {
public:
    static std::shared_ptr<LiveSurface> create(
        LiveSurfaceDescription description);

    ~LiveSurface();
    LiveSurface(const LiveSurface&) = delete;
    LiveSurface& operator=(const LiveSurface&) = delete;

    [[nodiscard]] bool reconfigure(LiveSurfaceDescription description);
    [[nodiscard]] LiveSurfaceWriteLease try_acquire_write(
        bool preserve_published_contents = false) noexcept;
    [[nodiscard]] LiveSurfaceFrame acquire_latest() const noexcept;
    [[nodiscard]] LiveSurfaceSnapshot snapshot() const noexcept;
    [[nodiscard]] LiveSurfaceWakeConnection connect_presentation_wake(
        std::function<void()> wake);

private:
    explicit LiveSurface(std::shared_ptr<detail::LiveSurfaceState> state) noexcept;
    std::shared_ptr<detail::LiveSurfaceState> state_;
};

} // namespace gui_forms
