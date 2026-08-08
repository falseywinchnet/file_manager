#include "gui_forms/live_surface.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <limits>
#include <mutex>
#include <new>
#include <utility>
#include <vector>

namespace gui_forms::detail {

struct LiveSurfaceBuffer final {
    LiveSurfaceDescription description{};
    std::uint64_t row_bytes{};
    std::vector<std::byte> pixels;
};

struct LiveSurfaceWake final {
    std::uint64_t sequence{};
    std::atomic<bool> connected{true};
    std::function<void()> callback;
};

struct LiveSurfaceState final {
    mutable std::mutex mutex;
    LiveSurfaceDescription description{};
    std::array<std::shared_ptr<LiveSurfaceBuffer>, 3> buffers;
    std::size_t published_slot{buffers.size()};
    std::size_t writing_slot{buffers.size()};
    std::uint64_t epoch{1};
    std::uint64_t generation{};
    std::uint64_t publishes{};
    std::uint64_t dropped_acquires{};
    std::uint64_t read_acquires{};
    std::uint64_t next_wake_sequence{1};
    Rect damage{};
    std::vector<std::shared_ptr<LiveSurfaceWake>> wakes;
};

} // namespace gui_forms::detail

namespace gui_forms {
namespace {

constexpr std::uint32_t maximum_dimension = 32768U;
constexpr std::uint64_t maximum_pixels = 268435456ULL;

[[nodiscard]] bool valid_description(
    LiveSurfaceDescription description) noexcept {
    if (description.width == 0U || description.height == 0U ||
        description.width > maximum_dimension ||
        description.height > maximum_dimension ||
        description.pixel_format !=
            LiveSurfacePixelFormat::bgra32_premultiplied_srgb) {
        return false;
    }
    return static_cast<std::uint64_t>(description.width) *
        description.height <= maximum_pixels;
}

[[nodiscard]] std::shared_ptr<detail::LiveSurfaceBuffer> make_buffer(
    LiveSurfaceDescription description) {
    if (!valid_description(description)) return {};
    const std::uint64_t row_bytes =
        static_cast<std::uint64_t>(description.width) * 4U;
    const std::uint64_t byte_count = row_bytes * description.height;
    if (byte_count > std::numeric_limits<std::size_t>::max()) return {};
    try {
        auto result = std::make_shared<detail::LiveSurfaceBuffer>();
        result->description = description;
        result->row_bytes = row_bytes;
        result->pixels.resize(static_cast<std::size_t>(byte_count));
        return result;
    } catch (const std::bad_alloc&) {
        return {};
    }
}

[[nodiscard]] Rect full_damage(
    LiveSurfaceDescription description) noexcept {
    return {0.0, 0.0, static_cast<double>(description.width),
            static_cast<double>(description.height)};
}

} // namespace

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

LiveSurfaceFrame::LiveSurfaceFrame(
    std::shared_ptr<const detail::LiveSurfaceBuffer> buffer,
    std::uint64_t epoch, std::uint64_t generation, Rect damage) noexcept
    : buffer_(std::move(buffer)), epoch_(epoch), generation_(generation),
      damage_(damage) {}

std::uint32_t LiveSurfaceFrame::width() const noexcept {
    return buffer_ ? buffer_->description.width : 0U;
}

std::uint32_t LiveSurfaceFrame::height() const noexcept {
    return buffer_ ? buffer_->description.height : 0U;
}

std::uint64_t LiveSurfaceFrame::row_bytes() const noexcept {
    return buffer_ ? buffer_->row_bytes : 0U;
}

LiveSurfacePixelFormat LiveSurfaceFrame::pixel_format() const noexcept {
    return buffer_ ? buffer_->description.pixel_format
                   : LiveSurfacePixelFormat::bgra32_premultiplied_srgb;
}

std::span<const std::byte> LiveSurfaceFrame::pixels() const noexcept {
    return buffer_ ? std::span<const std::byte>(buffer_->pixels)
                   : std::span<const std::byte>{};
}

LiveSurfaceWriteLease::LiveSurfaceWriteLease(
    std::shared_ptr<detail::LiveSurfaceState> state,
    std::shared_ptr<detail::LiveSurfaceBuffer> buffer, std::size_t slot,
    std::uint64_t epoch) noexcept
    : state_(std::move(state)), buffer_(std::move(buffer)), slot_(slot),
      epoch_(epoch) {}

LiveSurfaceWriteLease::~LiveSurfaceWriteLease() { abandon(); }

LiveSurfaceWriteLease::LiveSurfaceWriteLease(
    LiveSurfaceWriteLease&& other) noexcept
    : state_(std::move(other.state_)), buffer_(std::move(other.buffer_)),
      slot_(other.slot_), epoch_(other.epoch_) {}

LiveSurfaceWriteLease& LiveSurfaceWriteLease::operator=(
    LiveSurfaceWriteLease&& other) noexcept {
    if (this != &other) {
        abandon();
        state_ = std::move(other.state_);
        buffer_ = std::move(other.buffer_);
        slot_ = other.slot_;
        epoch_ = other.epoch_;
    }
    return *this;
}

std::uint32_t LiveSurfaceWriteLease::width() const noexcept {
    return buffer_ ? buffer_->description.width : 0U;
}

std::uint32_t LiveSurfaceWriteLease::height() const noexcept {
    return buffer_ ? buffer_->description.height : 0U;
}

std::uint64_t LiveSurfaceWriteLease::row_bytes() const noexcept {
    return buffer_ ? buffer_->row_bytes : 0U;
}

std::span<std::byte> LiveSurfaceWriteLease::pixels() noexcept {
    return buffer_ ? std::span<std::byte>(buffer_->pixels)
                   : std::span<std::byte>{};
}

std::uint64_t LiveSurfaceWriteLease::publish(Rect damage) {
    if (!state_ || !buffer_) return 0U;
    std::vector<std::shared_ptr<detail::LiveSurfaceWake>> wakes;
    std::uint64_t published{};
    {
        std::scoped_lock lock(state_->mutex);
        if (state_->epoch != epoch_ || state_->writing_slot != slot_ ||
            slot_ >= state_->buffers.size() ||
            state_->buffers[slot_].get() != buffer_.get()) {
            state_.reset();
            buffer_.reset();
            return 0U;
        }
        const Rect bounds = full_damage(buffer_->description);
        if (damage.empty()) damage = bounds;
        damage = Rect::intersection(damage, bounds);
        state_->published_slot = slot_;
        state_->writing_slot = state_->buffers.size();
        state_->damage = damage.empty() ? bounds : damage;
        ++state_->generation;
        ++state_->publishes;
        published = state_->generation;
        wakes = state_->wakes;
    }
    state_.reset();
    buffer_.reset();
    for (const auto& wake : wakes) {
        if (!wake || !wake->connected.load(std::memory_order_acquire) ||
            !wake->callback) {
            continue;
        }
        try {
            wake->callback();
        } catch (...) {
            // A presentation wake is a best-effort scheduler signal. Producer
            // publication remains committed even if a consumer is retiring.
        }
    }
    return published;
}

void LiveSurfaceWriteLease::abandon() noexcept {
    if (state_) {
        std::scoped_lock lock(state_->mutex);
        if (state_->epoch == epoch_ && state_->writing_slot == slot_) {
            state_->writing_slot = state_->buffers.size();
        }
    }
    state_.reset();
    buffer_.reset();
}

LiveSurface::LiveSurface(
    std::shared_ptr<detail::LiveSurfaceState> state) noexcept
    : state_(std::move(state)) {}

LiveSurface::~LiveSurface() = default;

std::shared_ptr<LiveSurface> LiveSurface::create(
    LiveSurfaceDescription description) {
    if (!valid_description(description)) return {};
    auto state = std::make_shared<detail::LiveSurfaceState>();
    state->description = description;
    for (auto& buffer : state->buffers) {
        buffer = make_buffer(description);
        if (!buffer) return {};
    }
    return std::shared_ptr<LiveSurface>(new LiveSurface(std::move(state)));
}

bool LiveSurface::reconfigure(LiveSurfaceDescription description) {
    if (!state_ || !valid_description(description)) return false;
    std::array<std::shared_ptr<detail::LiveSurfaceBuffer>, 3> replacement;
    for (auto& buffer : replacement) {
        buffer = make_buffer(description);
        if (!buffer) return false;
    }
    std::vector<std::shared_ptr<detail::LiveSurfaceWake>> wakes;
    {
        std::scoped_lock lock(state_->mutex);
        if (state_->writing_slot != state_->buffers.size()) return false;
        state_->description = description;
        state_->buffers = std::move(replacement);
        state_->published_slot = state_->buffers.size();
        state_->writing_slot = state_->buffers.size();
        state_->generation = 0U;
        state_->damage = {};
        ++state_->epoch;
        wakes = state_->wakes;
    }
    for (const auto& wake : wakes) {
        if (!wake || !wake->connected.load(std::memory_order_acquire) ||
            !wake->callback) {
            continue;
        }
        try {
            wake->callback();
        } catch (...) {
        }
    }
    return true;
}

LiveSurfaceWriteLease LiveSurface::try_acquire_write(
    bool preserve_published_contents) noexcept {
    if (!state_) return {};
    std::scoped_lock lock(state_->mutex);
    if (state_->writing_slot != state_->buffers.size()) {
        ++state_->dropped_acquires;
        return {};
    }
    for (std::size_t slot = 0; slot < state_->buffers.size(); ++slot) {
        if (slot == state_->published_slot ||
            state_->buffers[slot].use_count() != 1) {
            continue;
        }
        if (preserve_published_contents &&
            state_->published_slot < state_->buffers.size()) {
            state_->buffers[slot]->pixels =
                state_->buffers[state_->published_slot]->pixels;
        }
        state_->writing_slot = slot;
        return {state_, state_->buffers[slot], slot, state_->epoch};
    }
    ++state_->dropped_acquires;
    return {};
}

LiveSurfaceFrame LiveSurface::acquire_latest() const noexcept {
    if (!state_) return {};
    std::scoped_lock lock(state_->mutex);
    if (state_->published_slot >= state_->buffers.size()) return {};
    ++state_->read_acquires;
    return {state_->buffers[state_->published_slot], state_->epoch,
            state_->generation, state_->damage};
}

LiveSurfaceSnapshot LiveSurface::snapshot() const noexcept {
    if (!state_) return {};
    std::scoped_lock lock(state_->mutex);
    return {state_->description, state_->epoch, state_->generation,
            state_->publishes, state_->dropped_acquires,
            state_->read_acquires,
            state_->writing_slot < state_->buffers.size(),
            state_->published_slot < state_->buffers.size()};
}

LiveSurfaceWakeConnection LiveSurface::connect_presentation_wake(
    std::function<void()> wake) {
    if (!state_ || !wake) return {};
    auto connection = std::make_shared<detail::LiveSurfaceWake>();
    connection->callback = std::move(wake);
    {
        std::scoped_lock lock(state_->mutex);
        connection->sequence = state_->next_wake_sequence++;
        if (state_->next_wake_sequence == 0U) state_->next_wake_sequence = 1U;
        state_->wakes.push_back(connection);
    }
    return {state_, std::move(connection)};
}

} // namespace gui_forms
