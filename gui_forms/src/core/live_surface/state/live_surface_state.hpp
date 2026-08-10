#pragma once

#include "gui_forms/live_surface/types/live_surface_types.hpp"

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
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
    std::vector<std::shared_ptr<LiveSurfaceBuffer>> buffers;
    std::size_t published_slot{};
    std::size_t writing_slot{};
    std::uint64_t epoch{1};
    std::uint64_t generation{};
    std::uint64_t publishes{};
    std::uint64_t dropped_acquires{};
    std::uint64_t read_acquires{};
    std::uint64_t last_read_generation{};
    std::uint64_t next_wake_sequence{1};
    Rect damage{};
    std::vector<std::shared_ptr<LiveSurfaceWake>> wakes;
};

} // namespace gui_forms::detail
