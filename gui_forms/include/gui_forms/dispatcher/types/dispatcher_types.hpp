#pragma once

#include <cstddef>
#include <cstdint>

namespace gui_forms {

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
