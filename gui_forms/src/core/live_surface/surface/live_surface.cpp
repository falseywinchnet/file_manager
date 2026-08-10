#include "gui_forms/live_surface/surface/live_surface.hpp"

#include "../buffer/live_surface_buffer.hpp"

#include <atomic>
#include <mutex>
#include <utility>
#include <vector>

namespace gui_forms {

LiveSurface::LiveSurface(
    std::shared_ptr<detail::LiveSurfaceState> state) noexcept
    : state_(std::move(state)) {}

LiveSurface::~LiveSurface() = default;

std::shared_ptr<LiveSurface> LiveSurface::create(
    LiveSurfaceDescription description) {
    if (!detail::valid_live_surface_description(description)) return {};
    std::shared_ptr<gui_forms::detail::LiveSurfaceState> state = std::make_shared<detail::LiveSurfaceState>();
    (*state).description = description;
    (*state).buffers.resize(description.buffer_count);
    (*state).published_slot = (*state).buffers.size();
    (*state).writing_slot = (*state).buffers.size();
    for (std::shared_ptr<gui_forms::detail::LiveSurfaceBuffer>& buffer : (*state).buffers) {
        buffer = detail::make_live_surface_buffer(description);
        if (!buffer) return {};
    }
    return std::shared_ptr<LiveSurface>(new LiveSurface(std::move(state)));
}

bool LiveSurface::reconfigure(LiveSurfaceDescription description) {
    if (!state_ || !detail::valid_live_surface_description(description)) {
        return false;
    }
    std::vector<std::shared_ptr<detail::LiveSurfaceBuffer>> replacement(
        description.buffer_count);
    for (std::shared_ptr<gui_forms::detail::LiveSurfaceBuffer>& buffer : replacement) {
        buffer = detail::make_live_surface_buffer(description);
        if (!buffer) return false;
    }
    std::vector<std::shared_ptr<detail::LiveSurfaceWake>> wakes;
    {
        std::scoped_lock lock((*state_).mutex);
        if ((*state_).writing_slot != (*state_).buffers.size()) return false;
        (*state_).description = description;
        (*state_).buffers = std::move(replacement);
        (*state_).published_slot = (*state_).buffers.size();
        (*state_).writing_slot = (*state_).buffers.size();
        (*state_).generation = 0U;
        (*state_).damage = {};
        ++(*state_).epoch;
        wakes = (*state_).wakes;
    }
    for (const std::shared_ptr<gui_forms::detail::LiveSurfaceWake>& wake : wakes) {
        if (!wake || !(*wake).connected.load(std::memory_order_acquire) ||
            !(*wake).callback) {
            continue;
        }
        try {
            (*wake).callback();
        } catch (...) {
        }
    }
    return true;
}

LiveSurfaceWriteLease LiveSurface::try_acquire_write(
    bool preserve_published_contents) noexcept {
    if (!state_) return {};
    std::scoped_lock lock((*state_).mutex);
    if ((*state_).writing_slot != (*state_).buffers.size()) {
        ++(*state_).dropped_acquires;
        return {};
    }
    for (std::size_t slot = 0; slot < (*state_).buffers.size(); ++slot) {
        if (slot == (*state_).published_slot ||
            (*state_).buffers[slot].use_count() != 1) {
            continue;
        }
        if (preserve_published_contents &&
            (*state_).published_slot < (*state_).buffers.size()) {
            (*(*state_).buffers[slot]).pixels =
                (*(*state_).buffers[(*state_).published_slot]).pixels;
        }
        (*state_).writing_slot = slot;
        return {state_, (*state_).buffers[slot], slot, (*state_).epoch};
    }
    ++(*state_).dropped_acquires;
    return {};
}

LiveSurfaceFrame LiveSurface::acquire_latest() const noexcept {
    if (!state_) return {};
    std::scoped_lock lock((*state_).mutex);
    if ((*state_).published_slot >= (*state_).buffers.size()) return {};
    ++(*state_).read_acquires;
    (*state_).last_read_generation = (*state_).generation;
    return {(*state_).buffers[(*state_).published_slot], (*state_).epoch,
            (*state_).generation, (*state_).damage};
}

LiveSurfaceSnapshot LiveSurface::snapshot() const noexcept {
    if (!state_) return {};
    std::scoped_lock lock((*state_).mutex);
    return {(*state_).description, (*state_).epoch, (*state_).generation,
            (*state_).publishes, (*state_).dropped_acquires,
            (*state_).read_acquires, (*state_).last_read_generation,
            (*state_).writing_slot < (*state_).buffers.size(),
            (*state_).published_slot < (*state_).buffers.size()};
}

LiveSurfaceWakeConnection LiveSurface::connect_presentation_wake(
    std::function<void()> wake) {
    if (!state_ || !wake) return {};
    std::shared_ptr<gui_forms::detail::LiveSurfaceWake> connection = std::make_shared<detail::LiveSurfaceWake>();
    (*connection).callback = std::move(wake);
    {
        std::scoped_lock lock((*state_).mutex);
        (*connection).sequence = (*state_).next_wake_sequence++;
        if ((*state_).next_wake_sequence == 0U) (*state_).next_wake_sequence = 1U;
        (*state_).wakes.push_back(connection);
    }
    return {state_, std::move(connection)};
}

} // namespace gui_forms
