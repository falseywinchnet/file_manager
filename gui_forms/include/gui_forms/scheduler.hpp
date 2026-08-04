#pragma once

#include "gui_forms/component.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

namespace gui_forms {

class Window;
namespace detail {
class ScheduledFrameRequest;
}

using FrameClock = std::chrono::steady_clock;
using FrameTime = FrameClock::time_point;
using FrameInterval = FrameClock::duration;

inline constexpr std::size_t maximum_scheduled_frame_requests = 256;
inline constexpr std::size_t maximum_active_surfaces = 32;
inline constexpr FrameInterval minimum_active_surface_interval =
    std::chrono::milliseconds(8);

struct FramePollResult final {
    std::uint64_t deadlines_fired{};
    std::uint64_t active_surface_ticks{};
    std::uint64_t coalesced_requests{};
    bool damage_pending{};
    bool suppressed_by_occlusion{};
    std::optional<FrameTime> next_wake;
};

class FrameRequestToken final {
public:
    FrameRequestToken() = default;
    ~FrameRequestToken() { disconnect(); }
    FrameRequestToken(FrameRequestToken&& other) noexcept
        : revocable_(std::move(other.revocable_)) {}
    FrameRequestToken& operator=(FrameRequestToken&& other) noexcept {
        if (this != &other) {
            disconnect();
            revocable_ = std::move(other.revocable_);
        }
        return *this;
    }
    FrameRequestToken(const FrameRequestToken&) = delete;
    FrameRequestToken& operator=(const FrameRequestToken&) = delete;

    void disconnect() noexcept {
        if (revocable_) {
            revocable_->disconnect();
            revocable_.reset();
        }
    }
    [[nodiscard]] bool connected() const noexcept {
        return revocable_ != nullptr && revocable_->connected();
    }

private:
    friend class Window;
    explicit FrameRequestToken(std::shared_ptr<detail::Revocable> revocable)
        : revocable_(std::move(revocable)) {}

    std::shared_ptr<detail::Revocable> revocable_;
};

} // namespace gui_forms
