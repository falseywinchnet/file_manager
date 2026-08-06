#include "gui_forms/timer.hpp"

#include "gui_forms/window.hpp"

#include <stdexcept>

namespace gui_forms {

Timer::Timer(Window& window, std::chrono::milliseconds interval)
    : window_lifetime_(window.lifetime_), interval_(interval),
      callback_state_(std::make_shared<CallbackState>()) {
    if (interval_ < minimum_ui_timer_interval) {
        throw std::invalid_argument("GUI.Forms Timer interval must be at least 1 ms");
    }
    window.verify_access("Timer construction");
    callback_state_->owner = this;
}

Timer::~Timer() {
    request_.disconnect();
    enabled_ = false;
    if (callback_state_) callback_state_->owner = nullptr;
}

Window* Timer::bound_window() const noexcept {
    const auto lifetime = window_lifetime_.lock();
    return lifetime ? lifetime->window : nullptr;
}

void Timer::require_mutable_timer(std::string_view operation) const {
    if (!is_alive()) {
        throw std::logic_error("GUI.Forms cannot mutate a disposed Timer");
    }
    Window* owner = bound_window();
    if (!owner) {
        throw std::logic_error("GUI.Forms cannot use a Timer after Window shutdown");
    }
    owner->verify_access(operation);
}

void Timer::set_interval(std::chrono::milliseconds interval) {
    require_mutable_timer("Timer interval mutation");
    if (interval < minimum_ui_timer_interval) {
        throw std::invalid_argument("GUI.Forms Timer interval must be at least 1 ms");
    }
    if (interval_ == interval) return;
    interval_ = interval;
    if (enabled_) {
        request_.disconnect();
        schedule(FrameClock::now() + interval_);
    }
}

bool Timer::enabled() const noexcept {
    return enabled_ && request_.connected();
}

void Timer::start() {
    require_mutable_timer("Timer start");
    if (enabled()) return;
    schedule(FrameClock::now() + interval_);
}

void Timer::start_at(FrameTime first_deadline) {
    require_mutable_timer("Timer deterministic start");
    request_.disconnect();
    schedule(first_deadline);
}

void Timer::schedule(FrameTime first_deadline) {
    Window* owner = bound_window();
    if (!owner) {
        throw std::logic_error("GUI.Forms cannot schedule after Window shutdown");
    }
    enabled_ = true;
    const std::weak_ptr<CallbackState> weak = callback_state_;
    request_ = owner->schedule_ui_timer(
        *this, interval_, first_deadline, [weak](FrameTime) {
            const auto state = weak.lock();
            Timer* timer = state ? state->owner : nullptr;
            if (!timer || !timer->is_alive() || !timer->enabled_) return;
            timer->tick_.emit();
        });
}

void Timer::stop() {
    require_mutable_timer("Timer stop");
    enabled_ = false;
    request_.disconnect();
}

void Timer::verify_dispose_thread() {
    if (Window* owner = bound_window()) {
        owner->verify_access("Timer disposal");
    }
}

void Timer::on_dispose() noexcept {
    enabled_ = false;
    request_.disconnect();
    if (callback_state_) callback_state_->owner = nullptr;
    window_lifetime_.reset();
}

} // namespace gui_forms
