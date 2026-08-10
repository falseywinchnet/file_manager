#pragma once

#include "gui_forms/live_surface/types/live_surface_types.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>

namespace gui_forms {

namespace detail {
struct LiveSurfaceState;
struct LiveSurfaceBuffer;
}

class LiveSurface;

// Exclusive producer lease. Destruction without publish abandons the candidate
// frame. Acquiring may fail instead of blocking when all buffers are retained
// by readers; high-rate producers should drop that frame.
class LiveSurfaceWriteLease final {
public:
    LiveSurfaceWriteLease() = default;
    ~LiveSurfaceWriteLease();
    LiveSurfaceWriteLease(LiveSurfaceWriteLease&& other) noexcept;
    LiveSurfaceWriteLease& operator=(LiveSurfaceWriteLease&& other) noexcept;
    LiveSurfaceWriteLease(const LiveSurfaceWriteLease&) = delete;
    LiveSurfaceWriteLease& operator=(const LiveSurfaceWriteLease&) = delete;

    [[nodiscard]] explicit operator bool() const noexcept {
        return state_ != nullptr && buffer_ != nullptr;
    }
    [[nodiscard]] std::uint32_t width() const noexcept;
    [[nodiscard]] std::uint32_t height() const noexcept;
    [[nodiscard]] std::uint64_t row_bytes() const noexcept;
    [[nodiscard]] std::span<std::byte> pixels() noexcept;
    [[nodiscard]] std::uint64_t publish(Rect damage = {});
    void abandon() noexcept;

private:
    friend class LiveSurface;
    LiveSurfaceWriteLease(std::shared_ptr<detail::LiveSurfaceState> state,
                          std::shared_ptr<detail::LiveSurfaceBuffer> buffer,
                          std::size_t slot, std::uint64_t epoch) noexcept;

    std::shared_ptr<detail::LiveSurfaceState> state_;
    std::shared_ptr<detail::LiveSurfaceBuffer> buffer_;
    std::size_t slot_{};
    std::uint64_t epoch_{};
};

} // namespace gui_forms
