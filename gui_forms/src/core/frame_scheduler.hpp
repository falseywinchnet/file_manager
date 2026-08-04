#pragma once

#include "gui_forms/control.hpp"
#include "gui_forms/scheduler.hpp"

namespace gui_forms::detail {

enum class FrameRequestKind : std::uint8_t {
    deadline,
    active_surface,
};

class ScheduledFrameRequest final : public Revocable {
public:
    ScheduledFrameRequest(FrameRequestKind request_kind,
                          Control::WeakPtr request_target,
                          FrameTime request_deadline,
                          FrameInterval request_interval = {}) noexcept
        : kind(request_kind), target(std::move(request_target)),
          deadline(request_deadline), interval(request_interval) {}

    void disconnect() noexcept override { connected_ = false; }
    [[nodiscard]] bool connected() const noexcept override { return connected_; }

    FrameRequestKind kind;
    Control::WeakPtr target;
    FrameTime deadline;
    FrameInterval interval;

private:
    bool connected_{true};
};

} // namespace gui_forms::detail
