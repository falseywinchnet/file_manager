#include "windows_compatibility_paint_endpoint.hpp"

#if defined(GF_C_API_HAS_WINDOWS_HOST)
#include "../error/abi_error.hpp"
#include "../registry/registry.hpp"

#include <cstring>
#include <limits>
#include <mutex>
#include <span>
#include <unordered_map>
#include <utility>
#include <vector>

namespace gui_forms::abi::detail {

std::mutex compatibility_paint_endpoints_mutex;
std::unordered_map<std::uint64_t,
    std::shared_ptr<gui_forms::host::WindowsCompatibilityPaintEndpoint>>
    compatibility_paint_endpoints;
std::unordered_map<std::uint64_t, CompatibilityPaintBinding>
    compatibility_paint_bindings;
std::uint64_t next_compatibility_paint_endpoint{1};
std::unordered_map<std::uint64_t, CompatibilityPaintWrite>
    compatibility_paint_writes;
std::uint64_t next_compatibility_paint_write{1};

std::shared_ptr<gui_forms::host::WindowsCompatibilityPaintEndpoint>
compatibility_paint_endpoint(std::uint64_t token) {
    std::scoped_lock lock(compatibility_paint_endpoints_mutex);
    const auto found = compatibility_paint_endpoints.find(token);
    return found == compatibility_paint_endpoints.end() ? nullptr : found->second;
}

std::vector<std::shared_ptr<
    gui_forms::host::WindowsCompatibilityPaintEndpoint>>
compatibility_paint_endpoint_snapshot() {
    std::vector<std::shared_ptr<
        gui_forms::host::WindowsCompatibilityPaintEndpoint>> result;
    std::scoped_lock lock(compatibility_paint_endpoints_mutex);
    result.reserve(compatibility_paint_endpoints.size());
    for (const auto& [token, endpoint] : compatibility_paint_endpoints) {
        static_cast<void>(token);
        if (endpoint) result.push_back(endpoint);
    }
    return result;
}

std::shared_ptr<gui_forms::host::WindowsCompatibilityPaintEndpoint>
compatibility_paint_endpoint_for_handle(std::uintptr_t handle) {
    for (const auto& endpoint : compatibility_paint_endpoint_snapshot()) {
        if (endpoint->compatibility_handle() == handle) return endpoint;
    }
    return {};
}

std::shared_ptr<gui_forms::host::WindowsCompatibilityPaintEndpoint>
compatibility_paint_endpoint_for_dc(std::uintptr_t device_context) {
    for (const auto& endpoint : compatibility_paint_endpoint_snapshot()) {
        if (endpoint->device_context() == device_context) return endpoint;
    }
    return {};
}

gf_result api_windows_paint_endpoint_acquire(
    gf_handle control, std::uint32_t width, std::uint32_t height,
    std::uint64_t* token, std::uintptr_t* compatibility_handle) {
    if (token == nullptr || compatibility_handle == nullptr || width == 0U || height == 0U ||
        width > 32768U || height > 32768U ||
        static_cast<std::uint64_t>(width) * height > 268435456ULL) {
        return fail(GF_ERROR_INVALID_ARGUMENT,
                    "paint endpoint requires bounded dimensions and outputs");
    }
    std::weak_ptr<RasterControl> target;
    std::thread::id owner_thread;
    if (const gf_result result = registry().compatibility_paint_target(
            control, &target, &owner_thread); result != GF_OK) {
        return result;
    }
    if (std::this_thread::get_id() != owner_thread) {
        return fail(GF_ERROR_WRONG_THREAD,
                    "paint endpoint must be acquired on its control owner thread");
    }
    auto endpoint = gui_forms::host::WindowsCompatibilityPaintEndpoint::acquire(
        width, height);
    if (!endpoint || endpoint->compatibility_handle() == 0U) {
        return fail(GF_ERROR_INTERNAL,
                    "virtual Windows paint endpoint could not be created");
    }
    const std::uintptr_t virtual_handle = endpoint->compatibility_handle();
    const auto live_surface = endpoint->live_surface();
    if (const auto raster = target.lock()) {
        raster->set_live_surface(live_surface);
    }
    std::scoped_lock lock(compatibility_paint_endpoints_mutex);
    const std::uint64_t allocated = next_compatibility_paint_endpoint++;
    compatibility_paint_endpoints.emplace(allocated, endpoint);
    compatibility_paint_bindings.emplace(
        allocated, CompatibilityPaintBinding{target, owner_thread, live_surface});
    *token = allocated;
    *compatibility_handle = virtual_handle;
    return GF_OK;
}

gf_result api_windows_paint_endpoint_configure(
    std::uint64_t token, std::uint32_t width, std::uint32_t height) {
    if (width == 0U || height == 0U || width > 32768U || height > 32768U) {
        return fail(GF_ERROR_INVALID_ARGUMENT,
                    "paint endpoint configure requires bounded dimensions");
    }
    const auto endpoint = compatibility_paint_endpoint(token);
    if (!endpoint) {
        return fail(GF_ERROR_STALE_HANDLE,
                    "paint endpoint token is not active");
    }
    if (!endpoint->configure(width, height)) {
        return fail(GF_ERROR_WRONG_THREAD,
                    "paint endpoint configure requires its owner thread");
    }
    CompatibilityPaintBinding binding;
    {
        std::scoped_lock lock(compatibility_paint_endpoints_mutex);
        const auto found = compatibility_paint_bindings.find(token);
        if (found != compatibility_paint_bindings.end()) binding = found->second;
    }
    if (const auto raster = binding.target.lock()) {
        raster->set_live_surface(endpoint->live_surface());
    }
    return GF_OK;
}

gf_result api_windows_paint_endpoint_touch(std::uint64_t token,
                                           std::uint32_t explicit_boundary) {
    if (explicit_boundary > 1U) {
        return fail(GF_ERROR_INVALID_ARGUMENT,
                    "paint endpoint explicit flag must be zero or one");
    }
    const auto endpoint = compatibility_paint_endpoint(token);
    if (!endpoint) {
        return fail(GF_ERROR_STALE_HANDLE,
                    "paint endpoint token is not active");
    }
    endpoint->touch(explicit_boundary != 0U);
    return GF_OK;
}

gf_result api_windows_paint_endpoint_drain(std::uint64_t token) {
    const auto endpoint = compatibility_paint_endpoint(token);
    if (!endpoint) {
        return fail(GF_ERROR_STALE_HANDLE,
                    "paint endpoint token is not active");
    }
    return endpoint->drain_now() ? GF_OK :
        fail(GF_ERROR_WRONG_THREAD,
             "paint endpoint drain requires its owner thread");
}

gf_result api_windows_paint_endpoint_snapshot(
    std::uint64_t token, char* buffer, std::uint64_t capacity,
    std::uint64_t* required_size) {
    if (required_size == nullptr) {
        return fail(GF_ERROR_INVALID_ARGUMENT,
                    "paint endpoint snapshot requires a size output");
    }
    const auto endpoint = compatibility_paint_endpoint(token);
    if (!endpoint) {
        return fail(GF_ERROR_STALE_HANDLE,
                    "paint endpoint token is not active");
    }
    const std::string value = endpoint->snapshot();
    *required_size = value.size();
    if (capacity < value.size() || (!value.empty() && buffer == nullptr)) {
        return fail(GF_ERROR_BUFFER_TOO_SMALL,
                    "paint endpoint snapshot buffer is too small");
    }
    if (!value.empty()) std::memcpy(buffer, value.data(), value.size());
    return GF_OK;
}

gf_result api_windows_paint_endpoint_submit_bgra(
    std::uintptr_t compatibility_handle, std::uint32_t width, std::uint32_t height,
    std::uint64_t row_bytes, const void* pixels) {
    if (compatibility_handle == 0U || pixels == nullptr || width == 0U || height == 0U ||
        row_bytes < static_cast<std::uint64_t>(width) * 4U ||
        height > std::numeric_limits<std::size_t>::max() / row_bytes) {
        return fail(GF_ERROR_INVALID_ARGUMENT,
                    "paint endpoint submission requires bounded BGRA pixels");
    }
    const auto endpoint = compatibility_paint_endpoint_for_handle(compatibility_handle);
    if (!endpoint) {
        return fail(GF_ERROR_STALE_HANDLE,
                    "handle is not a compatibility paint endpoint");
    }
    const auto bytes = std::span<const std::byte>(
        static_cast<const std::byte*>(pixels),
        static_cast<std::size_t>(row_bytes) * height);
    return endpoint->submit_bgra32_premultiplied(
        width, height, row_bytes, bytes) ? GF_OK :
        fail(GF_ERROR_INVALID_ARGUMENT,
             "paint endpoint rejected the submitted frame");
}

gf_result api_windows_paint_endpoint_release(std::uint64_t token) {
    std::shared_ptr<gui_forms::host::WindowsCompatibilityPaintEndpoint> endpoint;
    CompatibilityPaintBinding binding;
    {
        std::scoped_lock lock(compatibility_paint_endpoints_mutex);
        const auto found = compatibility_paint_endpoints.find(token);
        if (found == compatibility_paint_endpoints.end()) {
            return fail(GF_ERROR_STALE_HANDLE,
                        "paint endpoint token is not active");
        }
        endpoint = std::move(found->second);
        compatibility_paint_endpoints.erase(found);
        const auto bound = compatibility_paint_bindings.find(token);
        if (bound != compatibility_paint_bindings.end()) {
            binding = bound->second;
            compatibility_paint_bindings.erase(bound);
        }
    }
    // Managed finalizers may release from a worker. Native retained state is
    // never mutated from that thread; an invalid HWND becomes an inert draw
    // source and normal control disposal revokes its frame lease. The ordinary
    // owner-thread path clears the binding immediately.
    if (std::this_thread::get_id() == binding.owner_thread) {
        if (const auto raster = binding.target.lock()) {
            raster->clear_live_surface(binding.surface);
        }
    }
    endpoint->release();
    return GF_OK;
}

gf_result api_windows_paint_endpoint_get_dc(
    std::uintptr_t compatibility_handle, std::uintptr_t* device_context) {
    if (compatibility_handle == 0U || device_context == nullptr) {
        return fail(GF_ERROR_INVALID_ARGUMENT,
                    "paint endpoint GetDC requires a handle and output");
    }
    const auto endpoint = compatibility_paint_endpoint_for_handle(compatibility_handle);
    if (!endpoint) return fail(
        GF_ERROR_STALE_HANDLE,
        "handle is not a compatibility paint endpoint");
    *device_context = endpoint->device_context();
    return *device_context != 0U ? GF_OK :
        fail(GF_ERROR_STALE_HANDLE,
             "paint endpoint has no active memory DC");
}

gf_result api_windows_paint_endpoint_release_dc(
    std::uintptr_t compatibility_handle, std::uintptr_t device_context) {
    if (compatibility_handle == 0U || device_context == 0U) {
        return fail(GF_ERROR_INVALID_ARGUMENT,
                    "paint endpoint ReleaseDC requires both handles");
    }
    const auto endpoint = compatibility_paint_endpoint_for_handle(compatibility_handle);
    if (endpoint && endpoint->device_context() == device_context) return GF_OK;
    return fail(GF_ERROR_STALE_HANDLE,
                "DC is not owned by the compatibility paint endpoint");
}

gf_result api_windows_paint_endpoint_publish_dc(
    std::uintptr_t device_context) {
    if (device_context == 0U) {
        return fail(GF_ERROR_INVALID_ARGUMENT,
                    "paint endpoint publish requires a DC");
    }
    const auto endpoint = compatibility_paint_endpoint_for_dc(device_context);
    if (!endpoint) {
        return fail(GF_ERROR_STALE_HANDLE,
                    "DC is not a compatibility paint endpoint");
    }
    return endpoint->publish_device_context(device_context) ? GF_OK :
        fail(GF_ERROR_INTERNAL, "paint endpoint frame could not publish");
}

gf_result api_windows_paint_endpoint_begin_write(
    std::uintptr_t device_context, std::uint64_t* write_lease) {
    if (device_context == 0U || write_lease == nullptr) {
        return fail(GF_ERROR_INVALID_ARGUMENT,
                    "paint endpoint write requires a DC and lease output");
    }
    const auto endpoint = compatibility_paint_endpoint_for_dc(device_context);
    if (!endpoint) {
        return fail(GF_ERROR_STALE_HANDLE,
                    "DC is not a compatibility paint endpoint");
    }
    std::uint64_t allocated{};
    {
        std::scoped_lock lock(compatibility_paint_endpoints_mutex);
        allocated = next_compatibility_paint_write++;
        compatibility_paint_writes.emplace(
            allocated, CompatibilityPaintWrite{
                endpoint, device_context, std::this_thread::get_id()});
    }
    if (!endpoint->begin_device_context_write(device_context)) {
        std::scoped_lock lock(compatibility_paint_endpoints_mutex);
        compatibility_paint_writes.erase(allocated);
        return fail(GF_ERROR_STALE_HANDLE,
                    "paint endpoint retired before its write began");
    }
    *write_lease = allocated;
    return GF_OK;
}

gf_result api_windows_paint_endpoint_end_write(
    std::uint64_t write_lease, std::uint32_t publish) {
    if (write_lease == 0U || publish > 1U) {
        return fail(GF_ERROR_INVALID_ARGUMENT,
                    "paint endpoint end-write requires a lease and boolean");
    }
    CompatibilityPaintWrite operation;
    {
        std::scoped_lock lock(compatibility_paint_endpoints_mutex);
        const auto found = compatibility_paint_writes.find(write_lease);
        if (found == compatibility_paint_writes.end()) {
            return fail(GF_ERROR_STALE_HANDLE,
                        "paint endpoint write lease is not active");
        }
        if (found->second.owner_thread != std::this_thread::get_id()) {
            return fail(GF_ERROR_WRONG_THREAD,
                        "paint endpoint write must end on its acquiring thread");
        }
        operation = std::move(found->second);
        compatibility_paint_writes.erase(found);
    }
    return operation.endpoint->end_device_context_write(
        operation.device_context, publish != 0U) ? GF_OK :
        fail(GF_ERROR_INTERNAL,
             "paint endpoint write could not finish or publish");
}

} // namespace gui_forms::abi::detail
#endif
