#pragma once

#include "gui_forms/types.hpp"

#include <cstddef>
#include <cstdint>

namespace gui_forms {

enum class LiveSurfacePixelFormat : std::uint8_t {
    bgra32_premultiplied_srgb,
};

inline constexpr std::size_t default_live_surface_buffer_count = 3U;
inline constexpr std::size_t minimum_live_surface_buffer_count = 2U;
inline constexpr std::size_t maximum_live_surface_buffer_count = 8U;

struct LiveSurfaceDescription final {
    std::uint32_t width{};
    std::uint32_t height{};
    LiveSurfacePixelFormat pixel_format{
        LiveSurfacePixelFormat::bgra32_premultiplied_srgb};
    // A smaller pool favors latency and memory; a larger pool tolerates more
    // compositor-held read leases before a producer must drop a frame.
    std::size_t buffer_count{default_live_surface_buffer_count};
    // The producer promises every pixel has alpha 255; the presenter may copy
    // instead of blend, and need not paint what lies under it.
    bool opaque{};
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

} // namespace gui_forms
