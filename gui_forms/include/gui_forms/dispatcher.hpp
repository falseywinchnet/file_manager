#pragma once

#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <stdexcept>
#include <utility>

namespace gui_forms {

namespace detail {
struct DispatchWork;
struct DispatcherState;
}

inline constexpr std::size_t maximum_posted_callbacks = 4096U;
inline constexpr std::size_t maximum_callbacks_per_dispatch_turn = 1024U;

enum class DispatchOperationState : std::uint8_t {
    invalid,
    pending,
    running,
    completed,
    cancelled,
    faulted,
};

class DispatchCancelledError final : public std::runtime_error {
public:
    DispatchCancelledError()
        : std::runtime_error(
              "GUI.Forms synchronous Invoke was cancelled before execution") {}
};

// A posted operation remains queued when this observation handle is dropped.
// Cancellation is explicit, matching BeginInvoke-style fire-and-forget use
// while still allowing deterministic revocation when a caller needs it.
class DispatchOperation final {
public:
    DispatchOperation() = default;
    // Internal construction seam; DispatchWork is intentionally incomplete to
    // consumers, so only the core dispatcher can produce a meaningful value.
    explicit DispatchOperation(std::shared_ptr<detail::DispatchWork> work)
        : work_(std::move(work)) {}

    [[nodiscard]] std::uint64_t sequence() const noexcept;
    [[nodiscard]] DispatchOperationState state() const noexcept;
    [[nodiscard]] bool pending() const noexcept;
    [[nodiscard]] bool cancel() noexcept;
    [[nodiscard]] std::exception_ptr exception() const noexcept;

private:
    friend class Window;
    friend class Control;
    void wait_and_rethrow() const;
    std::shared_ptr<detail::DispatchWork> work_;
};

struct DispatchDrainResult final {
    std::uint64_t invoked{};
    std::uint64_t cancelled{};
    std::uint64_t faulted{};
    std::size_t remaining{};
    bool wake_requested{};
};

struct DispatcherSnapshot final {
    std::uint64_t posted{};
    std::uint64_t invoked{};
    std::uint64_t cancelled{};
    std::uint64_t faulted{};
    std::uint64_t wake_requests{};
    std::uint64_t coalesced_wakes{};
    std::uint64_t synchronous_invocations{};
    std::uint64_t inline_invocations{};
    std::uint64_t marshalled_invocations{};
    std::size_t pending{};
    std::size_t maximum_pending{};
    bool accepting{};
    bool wake_pending{};
};

} // namespace gui_forms
