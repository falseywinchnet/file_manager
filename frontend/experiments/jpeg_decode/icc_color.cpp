#include "icc_color.hpp"

#include <lcms2.h>

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace jpeg_research {
namespace {

void require(const bool condition, const char* const message) {
    if (!condition) throw std::runtime_error(message);
}

struct ErrorState final { cmsUInt32Number code{}; };

// C callback cannot allocate or throw. The context borrows this invocation's
// state until its destruction; no process-global error handler is changed.
void report_error(const cmsContext context, const cmsUInt32Number code, const char*) noexcept {
    ErrorState* const state = static_cast<ErrorState*>(cmsGetContextUserData(context));
    if (state != nullptr) (*state).code = code;
}

struct ColorContext final {
    ErrorState error{};
    cmsContext handle{};
    ColorContext() : handle(cmsCreateContext(nullptr, &error)) {
        require(handle != nullptr, "color context allocation failed");
        cmsSetLogErrorHandlerTHR(handle, report_error);
    }
    ~ColorContext() { cmsDeleteContext(handle); }
    ColorContext(const ColorContext&) = delete;
    ColorContext& operator=(const ColorContext&) = delete;
    void check() const { require(error.code == 0U, "color profile or transform failed"); }
};

struct Profile final {
    cmsHPROFILE handle{};
    explicit Profile(const cmsHPROFILE acquired) : handle(acquired) {
        require(handle != nullptr, "color profile acquisition failed");
    }
    ~Profile() { cmsCloseProfile(handle); }
    Profile(const Profile&) = delete;
    Profile& operator=(const Profile&) = delete;
};

struct Transform final {
    cmsHTRANSFORM handle{};
    explicit Transform(const cmsHTRANSFORM acquired) : handle(acquired) {
        require(handle != nullptr, "color transform acquisition failed");
    }
    ~Transform() { cmsDeleteTransform(handle); }
    Transform(const Transform&) = delete;
    Transform& operator=(const Transform&) = delete;
};

void validate_profile_extent(const std::span<const unsigned char> bytes) {
    require(bytes.size() >= 132U && bytes.size() <= icc_profile_limit, "ICC byte extent");
    const std::uint32_t declared = (static_cast<std::uint32_t>(bytes[0]) << 24U) |
        (static_cast<std::uint32_t>(bytes[1]) << 16U) |
        (static_cast<std::uint32_t>(bytes[2]) << 8U) | static_cast<std::uint32_t>(bytes[3]);
    require(declared == bytes.size(), "ICC declared extent differs from supplied bytes");
}

void validate_raster(const Pixels& source) {
    require(source.width > 0 && source.height > 0 && source.width <= 1024 && source.height <= 1024,
        "ICC raster dimensions");
    require(source.row_bytes == source.width * 4, "ICC raster stride");
    const std::size_t extent = static_cast<std::size_t>(source.row_bytes) *
        static_cast<std::size_t>(source.height);
    require(source.bytes.size() == extent, "ICC raster byte extent");
    for (std::size_t offset = 3U; offset < extent; offset += 4U) {
        require(source.bytes[offset] == 255U, "ICC source must be opaque");
    }
}

// Caller establishes BGRA shape. Allocate the bounded gray plane once before
// the kernel; it remains alive through cmsDoTransform and has disjoint storage.
[[nodiscard]] std::vector<unsigned char> extract_gray(const Pixels& source, const std::size_t count) {
    std::vector<unsigned char> gray(count, 0U);
    for (std::size_t pixel = 0U; pixel < count; ++pixel) {
        const std::size_t offset = pixel * 4U;
        const unsigned char value = source.bytes[offset];
        require(value == source.bytes[offset + 1U] && value == source.bytes[offset + 2U],
            "gray source channels differ");
        gray[pixel] = value;
    }
    return gray;
}

} // namespace

Pixels convert_icc_to_srgb(const Pixels& source, const std::span<const unsigned char> profile,
    const IccSource source_space) {
    validate_profile_extent(profile);
    validate_raster(source);
    require(source_space == IccSource::rgb || source_space == IccSource::gray, "ICC source space");
    ColorContext context{};
    const cmsUInt32Number profile_size = static_cast<cmsUInt32Number>(profile.size());
    Profile input{cmsOpenProfileFromMemTHR(context.handle, profile.data(), profile_size)};
    context.check();
    const cmsProfileClassSignature profile_class = cmsGetDeviceClass(input.handle);
    require(profile_class == cmsSigInputClass || profile_class == cmsSigDisplayClass,
        "ICC profile class needs a separate policy");
    const cmsColorSpaceSignature expected = source_space == IccSource::rgb ? cmsSigRgbData : cmsSigGrayData;
    const cmsColorSpaceSignature actual = cmsGetColorSpace(input.handle);
    require(actual == expected, "ICC profile does not match JPEG color space");
    const cmsBool supported = cmsIsIntentSupported(input.handle, INTENT_RELATIVE_COLORIMETRIC, LCMS_USED_AS_INPUT);
    require(supported != 0, "ICC relative colorimetric input unavailable");
    Profile output{cmsCreate_sRGBProfileTHR(context.handle)};
    context.check();
    const std::size_t pixel_count = source.bytes.size() / 4U;
    const cmsUInt32Number count = static_cast<cmsUInt32Number>(pixel_count);
    Pixels result{};
    result.width = source.width;
    result.height = source.height;
    result.row_bytes = source.row_bytes;
    // Initialize alpha too: gray has no source extra channel to copy. Transform
    // writes B/G/R; RGB uses COPY_ALPHA from the validated opaque source.
    result.bytes.resize(source.bytes.size(), 255U);
    if (source_space == IccSource::gray) {
        const std::vector<unsigned char> gray = extract_gray(source, pixel_count);
        // The default optimizer failed the independent dark gray ramp oracle
        // on the pinned build (input 1: expected 13, observed 6). Retain the
        // non-optimized transform until a measured alternative passes it.
        Transform transform{cmsCreateTransformTHR(context.handle, input.handle, TYPE_GRAY_8,
            output.handle, TYPE_BGRA_8, INTENT_RELATIVE_COLORIMETRIC,
            cmsFLAGS_NOCACHE | cmsFLAGS_NOOPTIMIZE)};
        context.check();
        cmsDoTransform(transform.handle, gray.data(), result.bytes.data(), count);
        context.check();
    } else {
        Transform transform{cmsCreateTransformTHR(context.handle, input.handle, TYPE_BGRA_8,
            output.handle, TYPE_BGRA_8, INTENT_RELATIVE_COLORIMETRIC, cmsFLAGS_NOCACHE | cmsFLAGS_COPY_ALPHA)};
        context.check();
        cmsDoTransform(transform.handle, source.bytes.data(), result.bytes.data(), count);
        context.check();
    }
    return result;
}

} // namespace jpeg_research
