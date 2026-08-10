#include "scheduled_frame_request.hpp"

#include <utility>

namespace gui_forms::detail {

ScheduledFrameRequest::ScheduledFrameRequest(
    FrameRequestKind request_kind, Control::WeakPtr request_target,
    FrameTime request_deadline, FrameInterval request_interval) noexcept
    : kind(request_kind), target(std::move(request_target)),
      deadline(request_deadline), interval(request_interval) {}

ScheduledFrameRequest::ScheduledFrameRequest(
    FrameTime request_deadline, FrameInterval request_interval,
    std::function<void(FrameTime)> request_callback) noexcept
    : kind(FrameRequestKind::ui_timer), deadline(request_deadline),
      interval(request_interval), callback(std::move(request_callback)) {}

void ScheduledFrameRequest::disconnect() noexcept {
    connected_ = false;
    callback = {};
}

bool ScheduledFrameRequest::connected() const noexcept { return connected_; }

} // namespace gui_forms::detail
