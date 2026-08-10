#include "gui_forms/live_surface/wake_connection/live_surface_wake_connection.hpp"

#include "../state/live_surface_state.hpp"

#include <algorithm>
#include <mutex>
#include <utility>

namespace gui_forms {

LiveSurfaceWakeConnection::LiveSurfaceWakeConnection(
    std::weak_ptr<detail::LiveSurfaceState> state,
    std::shared_ptr<detail::LiveSurfaceWake> wake) noexcept
    : state_(std::move(state)), wake_(std::move(wake)) {}

LiveSurfaceWakeConnection::~LiveSurfaceWakeConnection() { disconnect(); }

LiveSurfaceWakeConnection::LiveSurfaceWakeConnection(
    LiveSurfaceWakeConnection&& other) noexcept
    : state_(std::move(other.state_)), wake_(std::move(other.wake_)) {}

LiveSurfaceWakeConnection& LiveSurfaceWakeConnection::operator=(
    LiveSurfaceWakeConnection&& other) noexcept {
    if (this != &other) {
        disconnect();
        state_ = std::move(other.state_);
        wake_ = std::move(other.wake_);
    }
    return *this;
}

bool LiveSurfaceWakeConnection::connected() const noexcept {
    return wake_ && wake_->connected.load(std::memory_order_acquire);
}

void LiveSurfaceWakeConnection::disconnect() noexcept {
    if (!wake_) return;
    wake_->connected.store(false, std::memory_order_release);
    if (const auto state = state_.lock()) {
        std::scoped_lock lock(state->mutex);
        std::erase_if(state->wakes, [sequence = wake_->sequence](const auto& wake) {
            return !wake || wake->sequence == sequence;
        });
    }
    state_.reset();
    wake_.reset();
}

} // namespace gui_forms
