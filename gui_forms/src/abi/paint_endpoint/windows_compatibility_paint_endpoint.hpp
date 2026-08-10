#pragma once

#include "gui_forms/c_api.h"

#if defined(GF_C_API_HAS_WINDOWS_HOST)
#include "../control_adapters/abi_control_adapters.hpp"
#include "gui_forms/live_surface.hpp"
#include "windows_host.hpp"

#include <cstdint>
#include <memory>
#include <thread>

namespace gui_forms::abi::detail {

struct CompatibilityPaintBinding final {
    std::weak_ptr<RasterControl> target;
    std::thread::id owner_thread;
    std::shared_ptr<gui_forms::LiveSurface> surface;
};

struct CompatibilityPaintWrite final {
    std::shared_ptr<gui_forms::host::WindowsCompatibilityPaintEndpoint> endpoint;
    std::uintptr_t device_context{};
    std::thread::id owner_thread;
};

gf_result api_windows_paint_endpoint_acquire(
    gf_handle control, std::uint32_t width, std::uint32_t height,
    std::uint64_t* token, std::uintptr_t* compatibility_handle);
gf_result api_windows_paint_endpoint_configure(
    std::uint64_t token, std::uint32_t width, std::uint32_t height);
gf_result api_windows_paint_endpoint_touch(
    std::uint64_t token, std::uint32_t explicit_boundary);
gf_result api_windows_paint_endpoint_drain(std::uint64_t token);
gf_result api_windows_paint_endpoint_snapshot(
    std::uint64_t token, char* buffer, std::uint64_t capacity,
    std::uint64_t* required_size);
gf_result api_windows_paint_endpoint_submit_bgra(
    std::uintptr_t compatibility_handle, std::uint32_t width,
    std::uint32_t height, std::uint64_t row_bytes, const void* pixels);
gf_result api_windows_paint_endpoint_release(std::uint64_t token);
gf_result api_windows_paint_endpoint_get_dc(
    std::uintptr_t compatibility_handle, std::uintptr_t* device_context);
gf_result api_windows_paint_endpoint_release_dc(
    std::uintptr_t compatibility_handle, std::uintptr_t device_context);
gf_result api_windows_paint_endpoint_publish_dc(std::uintptr_t device_context);
gf_result api_windows_paint_endpoint_begin_write(
    std::uintptr_t device_context, std::uint64_t* write_lease);
gf_result api_windows_paint_endpoint_end_write(
    std::uint64_t write_lease, std::uint32_t publish);

} // namespace gui_forms::abi::detail
#endif
