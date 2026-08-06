#pragma once

#include "gui_forms/control.hpp"
#include "gui_forms/scheduler.hpp"

#include <functional>

namespace gui_forms::detail {

enum class FrameRequestKind : std::uint8_t {
    deadline,
    active_surface,
    ui_timer,
};

class ScheduledFrameRequest final : public Revocable {
public:
    ScheduledFrameRequest(FrameRequestKind request_kind,
                          Control::WeakPtr request_target,
                          FrameTime request_deadline,
                          FrameInterval request_interval = {}) noexcept
        : kind(request_kind), target(std::move(request_target)),
          deadline(request_deadline), interval(request_interval) {}

    ScheduledFrameRequest(FrameTime request_deadline,
                          FrameInterval request_interval,
                          std::function<void(FrameTime)> request_callback) noexcept
        : kind(FrameRequestKind::ui_timer), deadline(request_deadline),
          interval(request_interval), callback(std::move(request_callback)) {}

    void disconnect() noexcept override {
        connected_ = false;
        callback = {};
    }
    [[nodiscard]] bool connected() const noexcept override { return connected_; }

    FrameRequestKind kind;
    Control::WeakPtr target;
    FrameTime deadline;
    FrameInterval interval;
    std::function<void(FrameTime)> callback;

private:
    bool connected_{true};
};

} // namespace gui_forms::detail
