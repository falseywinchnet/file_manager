#include "gui_forms/drawing_c_api.h"

#include "drawing_skia.hpp"

#include <cstring>
#include <limits>
#include <span>

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
    return api.raster_service_install(&service);
}
