#pragma once

#include "gui_forms/types.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>

namespace gui_forms {

enum class LiveSurfacePixelFormat : std::uint8_t {
    bgra32_premultiplied_srgb,
};

struct LiveSurfaceDescription final {
    std::uint32_t width{};
    std::uint32_t height{};
    LiveSurfacePixelFormat pixel_format{
        LiveSurfacePixelFormat::bgra32_premultiplied_srgb};
};

struct LiveSurfaceSnapshot final {
    LiveSurfaceDescription description{};
    std::uint64_t epoch{};
    std::uint64_t published_generation{};
    std::uint64_t publishes{};
    std::uint64_t dropped_acquires{};
    std::uint64_t read_acquires{};
    // Newest generation ever sampled by a terminal renderer. This is a
    // sampling diagnostic, not a compositor-present acknowledgement.
    std::uint64_t last_read_generation{};
    bool write_active{};
    bool has_frame{};
};

namespace detail {
struct LiveSurfaceState;
struct LiveSurfaceBuffer;
struct LiveSurfaceWake;
}

// Revocable connection for a retained consumer's presentation wake. The wake
// may run on the producer thread and therefore must only signal/marshal work;
// it must never paint, mutate controls, or call application code directly.
// Disconnect is idempotent. A wake already copied by an in-flight publish may
// finish, so consumers must also use weak/revocable ownership in the callback.
class LiveSurfaceWakeConnection final {
public:
    LiveSurfaceWakeConnection() = default;
    ~LiveSurfaceWakeConnection();
    LiveSurfaceWakeConnection(LiveSurfaceWakeConnection&&) noexcept;
    LiveSurfaceWakeConnection& operator=(LiveSurfaceWakeConnection&&) noexcept;
    LiveSurfaceWakeConnection(const LiveSurfaceWakeConnection&) = delete;
    LiveSurfaceWakeConnection& operator=(const LiveSurfaceWakeConnection&) = delete;

    [[nodiscard]] bool connected() const noexcept;
    void disconnect() noexcept;

private:
    friend class LiveSurface;
    LiveSurfaceWakeConnection(
        std::weak_ptr<detail::LiveSurfaceState> state,
        std::shared_ptr<detail::LiveSurfaceWake> wake) noexcept;

    std::weak_ptr<detail::LiveSurfaceState> state_;
    std::shared_ptr<detail::LiveSurfaceWake> wake_;
};

// Immutable read lease over one completely published frame. The producer can
// continue rendering into another buffer while this lease is held.
class LiveSurfaceFrame final {
public:
    LiveSurfaceFrame() = default;
    ~LiveSurfaceFrame() = default;
    LiveSurfaceFrame(LiveSurfaceFrame&&) noexcept = default;
    LiveSurfaceFrame& operator=(LiveSurfaceFrame&&) noexcept = default;
    LiveSurfaceFrame(const LiveSurfaceFrame&) = delete;
    LiveSurfaceFrame& operator=(const LiveSurfaceFrame&) = delete;

    [[nodiscard]] explicit operator bool() const noexcept {
        return buffer_ != nullptr;
    }
    [[nodiscard]] std::uint32_t width() const noexcept;
    [[nodiscard]] std::uint32_t height() const noexcept;
    [[nodiscard]] std::uint64_t row_bytes() const noexcept;
    [[nodiscard]] LiveSurfacePixelFormat pixel_format() const noexcept;
    [[nodiscard]] std::uint64_t epoch() const noexcept { return epoch_; }
    [[nodiscard]] std::uint64_t generation() const noexcept {
        return generation_;
    }
    [[nodiscard]] Rect damage() const noexcept { return damage_; }
    [[nodiscard]] std::span<const std::byte> pixels() const noexcept;

private:
    friend class LiveSurface;
    LiveSurfaceFrame(std::shared_ptr<const detail::LiveSurfaceBuffer> buffer,
                     std::uint64_t epoch, std::uint64_t generation,
                     Rect damage) noexcept;

    std::shared_ptr<const detail::LiveSurfaceBuffer> buffer_;
    std::uint64_t epoch_{};
    std::uint64_t generation_{};
    Rect damage_{};
};

// Exclusive producer lease. Destruction without publish abandons the candidate
// frame. Acquiring may fail instead of blocking when all three buffers are
// still owned by the compositor; high-rate producers should drop that frame.
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

// Renderer-neutral latest-frame surface. It retains no window, input target,
// application callback, or presentation queue. A producer publishes complete
// candidates; terminal painters acquire only the newest immutable candidate.
// Retained consumers may connect a wake primitive so publication touches the
// render graph without polling. The consumer owns coalescing and UI marshalling.
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
