#include "jpeg_decode.hpp"
#include "codec_owner.hpp"
#include "exif_orientation.hpp"
#include "icc_color.hpp"
#include <turbojpeg.h>
#include <cstdint>
#include <stdexcept>
#include <utility>

namespace jpeg_research {
namespace {
constexpr int source_pixel_limit{64 * 1024 * 1024};
constexpr int preview_side_limit{1024};
constexpr int intermediate_memory_megabytes{64};

void require(const bool condition, const char* const message) {
    if (!condition) { throw std::runtime_error(message); }
}

// Input has already passed the 1024-square extent check. Output is separately
// allocated; source/output do not alias. Choice of transform precedes the loop.
template<unsigned Orientation>
void orient_kernel(const Pixels& source, Pixels& destination) {
    static_assert(Orientation >= 2U && Orientation <= 8U);
    for (int y = 0; y < source.height; ++y) {
        const std::size_t row = static_cast<std::size_t>(y) * static_cast<std::size_t>(source.row_bytes);
        for (int x = 0; x < source.width; ++x) {
            int dx{};
            int dy{};
            if constexpr (Orientation == 2U) { dx = source.width - 1 - x; dy = y; }
            if constexpr (Orientation == 3U) { dx = source.width - 1 - x; dy = source.height - 1 - y; }
            if constexpr (Orientation == 4U) { dx = x; dy = source.height - 1 - y; }
            if constexpr (Orientation == 5U) { dx = y; dy = x; }
            if constexpr (Orientation == 6U) { dx = source.height - 1 - y; dy = x; }
            if constexpr (Orientation == 7U) { dx = source.height - 1 - y; dy = source.width - 1 - x; }
            if constexpr (Orientation == 8U) { dx = y; dy = source.width - 1 - x; }
            const std::size_t input = row + static_cast<std::size_t>(x) * 4U;
            const std::size_t output = static_cast<std::size_t>(dy) *
                static_cast<std::size_t>(destination.row_bytes) + static_cast<std::size_t>(dx) * 4U;
            for (std::size_t channel = 0U; channel < 4U; ++channel) {
                destination.bytes[output + channel] = source.bytes[input + channel];
            }
        }
    }
}

} // namespace

[[nodiscard]] Pixels orient(Pixels source, const unsigned orientation) {
    if (orientation == 1U) return source;
    Pixels output{};
    output.width = orientation >= 5U ? source.height : source.width;
    output.height = orientation >= 5U ? source.width : source.height;
    output.row_bytes = output.width * 4;
    output.bytes.resize(source.bytes.size());
    switch (orientation) {
        case 2U: orient_kernel<2U>(source, output); break;
        case 3U: orient_kernel<3U>(source, output); break;
        case 4U: orient_kernel<4U>(source, output); break;
        case 5U: orient_kernel<5U>(source, output); break;
        case 6U: orient_kernel<6U>(source, output); break;
        case 7U: orient_kernel<7U>(source, output); break;
        case 8U: orient_kernel<8U>(source, output); break;
        default: throw std::runtime_error("invalid orientation");
    }
    return output;
}

[[nodiscard]] Pixels decode(const std::span<const unsigned char> encoded, const unsigned orientation) {
    require(!encoded.empty() && encoded.size() <= encoded_limit, "encoded input limit");
    require(orientation >= 1U && orientation <= 8U, "orientation range");
    Codec decoder{TJINIT_DECOMPRESS};
    decoder.set(TJPARAM_STOPONWARNING, 1);
    decoder.set(TJPARAM_MAXMEMORY, intermediate_memory_megabytes);
    decoder.set(TJPARAM_MAXPIXELS, source_pixel_limit);
    decoder.set(TJPARAM_SCANLIMIT, 100);
    decoder.set(TJPARAM_SAVEMARKERS, 4);
    const int header_status = tj3DecompressHeader(decoder.handle, encoded.data(), encoded.size());
    decoder.check(header_status);
    const int width = tj3Get(decoder.handle, TJPARAM_JPEGWIDTH);
    const int height = tj3Get(decoder.handle, TJPARAM_JPEGHEIGHT);
    const int precision = tj3Get(decoder.handle, TJPARAM_PRECISION);
    const int colorspace = tj3Get(decoder.handle, TJPARAM_COLORSPACE);
    const int lossless = tj3Get(decoder.handle, TJPARAM_LOSSLESS);
    require(width > 0 && height > 0 && width <= 16384 && height <= 16384, "source dimensions");
    const std::uint64_t source_pixels = static_cast<std::uint64_t>(width) * static_cast<std::uint64_t>(height);
    require(source_pixels <= static_cast<std::uint64_t>(source_pixel_limit), "source pixels");
    require(precision == 8, "only 8-bit JPEG admitted by specimen");
    // The codec ignores the chosen scale for lossless JPEG, so admitting it
    // would invalidate the destination extent established below.
    require(lossless == 0, "lossless JPEG needs a distinct extent contract");
    require(colorspace == TJCS_YCbCr || colorspace == TJCS_RGB || colorspace == TJCS_GRAY,
            "CMYK/YCCK not admitted by specimen");
    std::size_t profile_size{};
    const int profile_status = tj3GetICCProfile(decoder.handle, nullptr, &profile_size);
    // In the pinned 3.2.0 implementation, absence returns -1/TJERR_WARNING and
    // zero size. Other errors remain failures; no error-string matching.
    const int profile_error = tj3GetErrorCode(decoder.handle);
    const bool absent_profile = profile_status == -1 && profile_error == TJERR_WARNING && profile_size == 0U;
    if (!absent_profile) decoder.check(profile_status);
    require(profile_size <= jpeg_research::icc_profile_limit, "ICC profile byte limit");
    CompressedBytes profile{};
    if (profile_size != 0U) {
        const int fetch_status = tj3GetICCProfile(decoder.handle, &profile.data, &profile.size);
        decoder.check(fetch_status);
        require(profile.data != nullptr && profile.size == profile_size, "ICC profile extraction extent");
    }
    int factor_count{};
    const tjscalingfactor* factors = tj3GetScalingFactors(&factor_count);
    require(factors != nullptr && factor_count > 0 && factor_count <= 64, "scaling-factor list");
    tjscalingfactor selected{0, 1};
    int output_width{};
    int output_height{};
    for (int index = 0; index < factor_count; ++index) {
        const tjscalingfactor factor = factors[index];
        require(factor.num > 0 && factor.num <= 64 && factor.denom > 0 && factor.denom <= 64,
                "scaling-factor extent");
        if (factor.num > factor.denom) continue;
        const int scaled_width = (width * factor.num + factor.denom - 1) / factor.denom;
        const int scaled_height = (height * factor.num + factor.denom - 1) / factor.denom;
        if (scaled_width > preview_side_limit || scaled_height > preview_side_limit) continue;
        if (factor.num * selected.denom <= selected.num * factor.denom) continue;
        selected = factor;
        output_width = scaled_width;
        output_height = scaled_height;
    }
    require(selected.num != 0, "no admitted decoder scale fits the output bound");
    const int scale_status = tj3SetScalingFactor(decoder.handle, selected);
    decoder.check(scale_status);
    Pixels raster{};
    raster.width = output_width;
    raster.height = output_height;
    raster.row_bytes = output_width * 4;
    const std::size_t extent = static_cast<std::size_t>(raster.row_bytes) * static_cast<std::size_t>(output_height);
    raster.bytes.resize(extent);
    const int decode_status = tj3Decompress8(decoder.handle, encoded.data(), encoded.size(),
        raster.bytes.data(), raster.row_bytes, TJPF_BGRA);
    decoder.check(decode_status);
    if (profile_size != 0U) {
        const jpeg_research::IccSource space = colorspace == TJCS_GRAY ?
            jpeg_research::IccSource::gray : jpeg_research::IccSource::rgb;
        const std::span<const unsigned char> bytes(profile.data, profile.size);
        Pixels converted = jpeg_research::convert_icc_to_srgb(raster, bytes, space);
        raster = std::move(converted);
    }
    Pixels result = orient(std::move(raster), orientation);
    return result;
}

[[nodiscard]] Pixels decode_exif(const std::span<const unsigned char> encoded) {
    const jpeg_research::OrientationResult metadata = jpeg_research::read_orientation(encoded);
    require(metadata.status == jpeg_research::OrientationStatus::absent ||
            metadata.status == jpeg_research::OrientationStatus::present,
            "EXIF orientation malformed, ambiguous or outside metadata bounds");
    Pixels result = decode(encoded, metadata.value);
    return result;
}

} // namespace jpeg_research
