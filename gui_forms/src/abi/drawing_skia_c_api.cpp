#include "gui_forms/drawing_c_api.h"

#include "drawing_skia.hpp"

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <span>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#if defined(_WIN32)
#define GDR_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define GDR_EXPORT __attribute__((visibility("default")))
#else
#define GDR_EXPORT
#endif

namespace {

using gui_drawing::Bitmap;
using gui_drawing::GraphicsRecorder;
using gui_drawing::render::DecodeResult;
using gui_drawing::render::RasterError;
using gui_drawing::render::RasterResult;
using gui_drawing::render::SkiaExecutor;

SkiaExecutor& executor() {
    thread_local SkiaExecutor value;
    return value;
}

std::filesystem::path module_directory() {
#if defined(_WIN32)
    HMODULE module{};
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                              GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          reinterpret_cast<LPCWSTR>(&module_directory), &module) == FALSE) {
        return {};
    }
    std::vector<wchar_t> path(32768);
    const DWORD count = GetModuleFileNameW(module, path.data(),
                                           static_cast<DWORD>(path.size()));
    if (count == 0 || count >= path.size()) return {};
    return std::filesystem::path(std::wstring(path.data(), count)).parent_path();
#else
    Dl_info info{};
    if (dladdr(reinterpret_cast<const void*>(&module_directory), &info) == 0 ||
        info.dli_fname == nullptr) {
        return {};
    }
    return std::filesystem::path(info.dli_fname).parent_path();
#endif
}

std::vector<std::byte> read_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input) return {};
    const std::streamoff length = input.tellg();
    if (length <= 0 || length > static_cast<std::streamoff>(64U * 1024U * 1024U)) {
        return {};
    }
    std::vector<std::byte> bytes(static_cast<std::size_t>(length));
    input.seekg(0);
    input.read(reinterpret_cast<char*>(bytes.data()), length);
    return input ? std::move(bytes) : std::vector<std::byte>{};
}

void register_packaged_fonts() {
    // The raster executor is thread-local. Native initialization can happen while
    // a resource is decoded on a loader thread, before the UI thread performs its
    // first paint, so font registration must have the same thread-local lifetime.
    thread_local bool attempted = false;
    if (attempted) return;
    attempted = true;
    const auto path = module_directory() / "fonts" / "PortsmouthRapids.ttf";
    const auto bytes = read_file(path);
    const bool registered = !bytes.empty() &&
        executor().register_typeface("Portsmouth Rapids", bytes);
    if (const char* trace = std::getenv("GUI_DRAWING_TRACE_FONTS");
        trace != nullptr && std::strcmp(trace, "1") == 0) {
        std::fprintf(stderr, "gui-drawing-font=path:%s|bytes:%zu|registered:%s\n",
                     path.string().c_str(), bytes.size(), registered ? "true" : "false");
        std::fflush(stderr);
    }
}

gd_result translate_error(RasterError error) noexcept {
    switch (error) {
    case RasterError::none: return GD_OK;
    case RasterError::wrong_thread: return GD_ERROR_WRONG_THREAD;
    case RasterError::invalid_argument:
    case RasterError::missing_image_pixels:
    case RasterError::decode_failed: return GD_ERROR_INVALID_ARGUMENT;
    case RasterError::encoded_limit_exceeded:
    case RasterError::dimension_limit_exceeded: return GD_ERROR_LIMIT_EXCEEDED;
    case RasterError::target_unavailable:
    case RasterError::encode_failed:
    case RasterError::internal_error: return GD_ERROR_INTERNAL;
    }
    return GD_ERROR_INTERNAL;
}

gd_result execute(const void* recorder, void* bitmap,
                  std::uint64_t* commands_executed) {
    if (recorder == nullptr || bitmap == nullptr || commands_executed == nullptr) {
        return GD_ERROR_INVALID_ARGUMENT;
    }
    try {
        register_packaged_fonts();
        const RasterResult result = executor().execute(
            *static_cast<const GraphicsRecorder*>(recorder),
            *static_cast<Bitmap*>(bitmap));
        *commands_executed = result.commands_executed;
        return translate_error(result.error);
    } catch (...) {
        return GD_ERROR_INTERNAL;
    }
}

gd_result encode_png(const void* bitmap, void* buffer, std::uint64_t capacity,
                     std::uint64_t* required_size) {
    if (bitmap == nullptr || required_size == nullptr) {
        return GD_ERROR_INVALID_ARGUMENT;
    }
    try {
        RasterError error = RasterError::internal_error;
        const std::vector<std::byte> png = executor().encode_png(
            *static_cast<const Bitmap*>(bitmap), &error);
        if (error != RasterError::none) return translate_error(error);
        *required_size = png.size();
        if (capacity < png.size() || (!png.empty() && buffer == nullptr)) {
            return GD_ERROR_BUFFER_TOO_SMALL;
        }
        if (!png.empty()) std::memcpy(buffer, png.data(), png.size());
        return GD_OK;
    } catch (...) {
        return GD_ERROR_INTERNAL;
    }
}

gd_result decode_png(const void* data, std::uint64_t size, void** bitmap) {
    if (data == nullptr || size == 0U || bitmap == nullptr ||
        size > std::numeric_limits<std::size_t>::max()) {
        return GD_ERROR_INVALID_ARGUMENT;
    }
    *bitmap = nullptr;
    try {
        DecodeResult result = executor().decode_png(
            {static_cast<const std::byte*>(data), static_cast<std::size_t>(size)});
        if (!result) return translate_error(result.error);
        *bitmap = result.bitmap.release();
        return GD_OK;
    } catch (...) {
        return GD_ERROR_INTERNAL;
    }
}

} // namespace

extern "C" GDR_EXPORT gd_result gdr_initialize(void) {
    gd_api_v0 api{};
    api.struct_size = sizeof(api);
    const gd_result loaded = gd_get_api_v0(GD_ABI_VERSION_0_1, &api);
    if (loaded != GD_OK) return loaded;
    if (api.raster_service_install == nullptr) return GD_ERROR_UNSUPPORTED_VERSION;
    const gd_raster_service_v0 service{
        sizeof(gd_raster_service_v0), GD_ABI_VERSION_0_1,
        &execute, &encode_png, &decode_png,
    };
    const gd_result installed = api.raster_service_install(&service);
    if (installed == GD_OK) register_packaged_fonts();
    return installed;
}
