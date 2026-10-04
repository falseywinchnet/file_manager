#include <turbojpeg.h>
#include "exif_orientation.hpp"
#include "jpeg_pixels.hpp"
#include "icc_color.hpp"
#include "icc_fixtures.hpp"
#include "preview_frame.hpp"
#include "jpeg_decode.hpp"
#include "codec_owner.hpp"
#include "encoded_stream.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using jpeg_research::encoded_limit;

void require(const bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

using Codec = jpeg_research::Codec;
using CompressedBytes = jpeg_research::CompressedBytes;
using jpeg_research::decode;
using jpeg_research::decode_exif;
using jpeg_research::orient;

using Pixels = jpeg_research::Pixels;

[[nodiscard]] std::vector<unsigned char> make_fixture(const int width, const int height,
    const bool progressive, const bool grayscale, const std::span<const unsigned char> profile = {}) {
    require(width > 0 && height > 0 && width <= 6000 && height <= 4000,
            "fixture dimensions exceed the declared generator extent");
    const std::size_t row_bytes = static_cast<std::size_t>(width) * 3U;
    const std::size_t extent = row_bytes * static_cast<std::size_t>(height);
    std::vector<unsigned char> rgb(extent, 0U);
    // Four distinct constant quadrants give independent orientation anchors.
    constexpr std::array<std::array<unsigned char, 3>, 4> colors{{
        {{240U, 24U, 24U}}, {{24U, 232U, 24U}},
        {{24U, 24U, 224U}}, {{216U, 216U, 24U}}
    }};
    for (int y = 0; y < height; ++y) {
        const std::size_t row = static_cast<std::size_t>(y) * row_bytes;
        const std::size_t lower = y >= height / 2 ? 2U : 0U;
        for (int x = 0; x < width; ++x) {
            const std::size_t quadrant = lower + (x >= width / 2 ? 1U : 0U);
            const std::size_t offset = row + static_cast<std::size_t>(x) * 3U;
            rgb[offset] = colors[quadrant][0];
            rgb[offset + 1U] = colors[quadrant][1];
            rgb[offset + 2U] = colors[quadrant][2];
        }
    }
    Codec encoder{TJINIT_COMPRESS};
    encoder.set(TJPARAM_QUALITY, 95);
    encoder.set(TJPARAM_SUBSAMP, grayscale ? TJSAMP_GRAY : TJSAMP_444);
    encoder.set(TJPARAM_PROGRESSIVE, progressive ? 1 : 0);
    encoder.set(TJPARAM_MAXMEMORY, 256);
    // TurboJPEG's setter copies bytes but exposes a mutable pointer parameter.
    // Supply separately owned mutable fixture bytes instead of casting const.
    std::vector<unsigned char> profile_copy(profile.begin(), profile.end());
    if (!profile_copy.empty()) {
        const int profile_status = tj3SetICCProfile(encoder.handle, profile_copy.data(), profile_copy.size());
        encoder.check(profile_status);
    }
    CompressedBytes encoded{};
    const int status = tj3Compress8(encoder.handle, rgb.data(), width,
        static_cast<int>(row_bytes), height, TJPF_RGB, &encoded.data, &encoded.size);
    encoder.check(status);
    require(encoded.size != 0U && encoded.size <= encoded_limit, "fixture encoded extent invalid");
    std::vector<unsigned char> owned(encoded.data, encoded.data + encoded.size);
    return owned;
}

// Table oracle: quadrant labels in output TL, TR, BL, BR for each orientation.
constexpr std::array<std::array<unsigned, 4>, 8> expected_quadrants{{
    {{0U, 1U, 2U, 3U}}, {{1U, 0U, 3U, 2U}}, {{3U, 2U, 1U, 0U}}, {{2U, 3U, 0U, 1U}},
    {{0U, 2U, 1U, 3U}}, {{2U, 0U, 3U, 1U}}, {{3U, 1U, 2U, 0U}}, {{1U, 3U, 0U, 2U}}
}};

void check_quadrants(const Pixels& raster, const unsigned orientation) {
    constexpr std::array<std::array<int, 3>, 4> colors{{
        {{24, 24, 240}}, {{24, 232, 24}}, {{224, 24, 24}}, {{24, 216, 216}}
    }};
    for (std::size_t corner = 0U; corner < 4U; ++corner) {
        const int x = (corner % 2U == 0U) ? raster.width / 4 : 3 * raster.width / 4;
        const int y = corner < 2U ? raster.height / 4 : 3 * raster.height / 4;
        const std::size_t offset = static_cast<std::size_t>(y) * static_cast<std::size_t>(raster.row_bytes) +
            static_cast<std::size_t>(x) * 4U;
        const unsigned expected = expected_quadrants[orientation - 1U][corner];
        for (std::size_t channel = 0U; channel < 3U; ++channel) {
            const int difference = static_cast<int>(raster.bytes[offset + channel]) - colors[expected][channel];
            require(difference >= -8 && difference <= 8, "decoded orientation/color anchor differs from oracle");
        }
    }
    for (std::size_t offset = 3U; offset < raster.bytes.size(); offset += 4U) {
        require(raster.bytes[offset] == 255U, "BGRA alpha must be initialized opaque");
    }
}

void require_decode_rejection(const std::span<const unsigned char> bytes, const unsigned orientation) {
    bool rejected{};
    try {
        const Pixels raster = decode(bytes, orientation);
        static_cast<void>(raster);
    } catch (const std::runtime_error&) { rejected = true; }
    require(rejected, "invalid specimen input was not rejected");
}

[[nodiscard]] std::vector<unsigned char> with_orientation(
    const std::vector<unsigned char>& encoded, const unsigned char orientation) {
    require(encoded.size() >= 2U && encoded.size() <= encoded_limit - 36U,
            "orientation fixture source extent");
    // APP1 alone, with little-endian TIFF IFD0. Inject after SOI without changing
    // the generated pixel encoding; the corner oracle remains independent.
    constexpr std::array<unsigned char, 36U> app1{
        0xff, 0xe1, 0, 0x22, 'E', 'x', 'i', 'f', 0, 0,
        'I', 'I', 42, 0, 8, 0, 0, 0, 1, 0,
        0x12, 1, 3, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    };
    std::vector<unsigned char> tagged{};
    tagged.reserve(encoded.size() + app1.size());
    tagged.insert(tagged.end(), encoded.begin(), encoded.begin() + 2);
    tagged.insert(tagged.end(), app1.begin(), app1.end());
    tagged.insert(tagged.end(), encoded.begin() + 2, encoded.end());
    tagged[30U] = orientation;
    return tagged;
}

void verify_embedded_orientation(const std::vector<unsigned char>& encoded) {
    std::vector<unsigned char> tagged = with_orientation(encoded, 1U);
    for (unsigned char orientation = 1U; orientation <= 8U; ++orientation) {
        tagged[30U] = orientation;
        const Pixels raster = decode_exif(tagged);
        const int width = orientation < 5U ? 320 : 192;
        const int height = orientation < 5U ? 192 : 320;
        require(raster.width == width && raster.height == height, "embedded EXIF dimensions");
        check_quadrants(raster, orientation);
    }
    const Pixels no_metadata = decode_exif(encoded);
    check_quadrants(no_metadata, 1U);
    tagged[30U] = 9U;
    bool refused = false;
    try { const Pixels invalid = decode_exif(tagged); static_cast<void>(invalid); }
    catch (const std::runtime_error&) { refused = true; }
    require(refused, "invalid EXIF must refuse before pixel publication");
}

void emit_generated_frame(const unsigned char orientation, const bool linear_color,
                          const bool encoded_only) {
    require(orientation >= 1U && orientation <= 8U, "frame fixture orientation");
    std::vector<unsigned char> profile{};
    if (linear_color) {
        profile = jpeg_research::make_icc_fixture(jpeg_research::IccFixture::linear_rgb);
    }
    // Baseline/untagged and progressive/linear RGB both traverse real JPEG decode,
    // embedded EXIF and output scaling. The latter combines EXIF and ICC markers.
    const std::vector<unsigned char> encoded = make_fixture(2048, 1536, linear_color, false, profile);
    const std::vector<unsigned char> tagged = with_orientation(encoded, orientation);
    if (encoded_only) {
        jpeg_research::write_encoded_stream(tagged);
        return;
    }
    const Pixels raster = decode_exif(tagged);
    jpeg_research::write_preview_frame(raster, 7, 11);
}

void verify_embedded_color() {
    const std::vector<unsigned char> srgb = jpeg_research::make_icc_fixture(jpeg_research::IccFixture::srgb);
    const std::vector<unsigned char> linear_rgb = jpeg_research::make_icc_fixture(jpeg_research::IccFixture::linear_rgb);
    const std::vector<unsigned char> linear_gray = jpeg_research::make_icc_fixture(jpeg_research::IccFixture::linear_gray);
    for (const bool progressive : {false, true}) {
        for (const bool gray : {false, true}) {
            const std::vector<unsigned char>& profile = gray ? linear_gray : linear_rgb;
            const std::vector<unsigned char> plain = make_fixture(320, 192, progressive, gray);
            const std::vector<unsigned char> tagged = make_fixture(320, 192, progressive, gray, profile);
            for (unsigned orientation = 1U; orientation <= 8U; ++orientation) {
                const Pixels reference = decode(plain, orientation);
                const Pixels converted = decode(tagged, orientation);
                require(converted.width == reference.width && converted.height == reference.height,
                    "ICC must preserve oriented geometry");
                // Same generated JPEG samples, with only the attached profile
                // differing. The scalar transfer equation is independent of LCMS.
                for (std::size_t pixel = 0U; pixel < reference.bytes.size(); pixel += 4U) {
                    for (std::size_t channel = 0U; channel < 3U; ++channel) {
                        const int expected = jpeg_research::encode_linear_srgb(reference.bytes[pixel + channel]);
                        const int difference = static_cast<int>(converted.bytes[pixel + channel]) - expected;
                        require(difference >= -3 && difference <= 3, "embedded ICC colors differ from scalar oracle");
                    }
                    require(converted.bytes[pixel + 3U] == 255U, "embedded ICC alpha");
                }
            }
        }
        const std::vector<unsigned char> identity = make_fixture(320, 192, progressive, false, srgb);
        const Pixels identity_raster = decode(identity, 1U);
        check_quadrants(identity_raster, 1U);
    }
    const std::vector<unsigned char> wrong_space = make_fixture(320, 192, false, false, linear_gray);
    require_decode_rejection(wrong_space, 1U);
    const std::vector<unsigned char> truncated(srgb.begin(), srgb.end() - 1);
    const std::vector<unsigned char> malformed = make_fixture(320, 192, false, false, truncated);
    require_decode_rejection(malformed, 1U);
    const std::vector<unsigned char> oversized(jpeg_research::icc_profile_limit + 1U, 0U);
    const std::vector<unsigned char> excessive = make_fixture(320, 192, false, false, oversized);
    require_decode_rejection(excessive, 1U);
    std::vector<unsigned char> marker_error = make_fixture(320, 192, false, false, srgb);
    constexpr std::array<unsigned char, 12U> signature{'I', 'C', 'C', '_', 'P', 'R', 'O', 'F', 'I', 'L', 'E', 0U};
    const std::vector<unsigned char>::iterator marker = std::search(marker_error.begin(), marker_error.end(),
        signature.begin(), signature.end());
    require(marker != marker_error.end(), "generated ICC marker must be present");
    const std::size_t signature_offset = static_cast<std::size_t>(marker - marker_error.begin());
    require(signature_offset + 14U <= marker_error.size(), "generated ICC sequence extent");
    marker_error[signature_offset + 13U] = 2U; // Declares a missing second APP2 segment.
    require_decode_rejection(marker_error, 1U);
    marker_error[signature_offset + 13U] = 1U;
    marker_error[signature_offset + 12U] = 0U; // Sequence numbers start at one.
    require_decode_rejection(marker_error, 1U);
    std::cerr << "PASS ICC RGB/gray scalar oracles, baseline/progressive, all orientations, malformed/oversize/mismatch refusal\n";
}

void verify() {
    jpeg_research::verify_icc_conversion();
    verify_embedded_color();
    const std::vector<unsigned char> encoded = make_fixture(320, 192, false, false);
    verify_embedded_orientation(encoded);
    for (unsigned orientation = 1U; orientation <= 8U; ++orientation) {
        const Pixels raster = decode(encoded, orientation);
        const int expected_width = orientation < 5U ? 320 : 192;
        const int expected_height = orientation < 5U ? 192 : 320;
        require(raster.width == expected_width && raster.height == expected_height,
                "orientation must preserve or swap non-square dimensions");
        check_quadrants(raster, orientation);
    }
    const std::vector<unsigned char> gray = make_fixture(320, 192, false, true);
    const Pixels grayscale = decode(gray, 1U);
    for (std::size_t offset = 0U; offset < grayscale.bytes.size(); offset += 4U) {
        require(grayscale.bytes[offset] == grayscale.bytes[offset + 1U] &&
                grayscale.bytes[offset] == grayscale.bytes[offset + 2U] &&
                grayscale.bytes[offset + 3U] == 255U, "grayscale output contract");
    }
    require_decode_rejection({}, 1U);
    require_decode_rejection(encoded, 0U);
    require_decode_rejection(std::span<const unsigned char>(encoded.data(), encoded.size() / 2U), 1U);
    std::cerr << "PASS embedded EXIF and supplied orientations, geometry, colors, opaque alpha, grayscale, empty/truncated/range refusal\n";
}

#ifdef _WIN32
void verify_wic_control() {
    // Both decoders receive identical generated bytes and output geometry.
    // Corner colors/opaque alpha are independent oracles, not byte equality:
    // IDCT and resampling implementations may legitimately differ.
    for (const bool progressive : {false, true}) {
        const std::vector<unsigned char> encoded = make_fixture(2048, 1536, progressive, false);
        const Pixels portable = decode(encoded, 1U);
        const Pixels native = jpeg_research::decode_wic(encoded, portable.width, portable.height);
        require(native.width == portable.width && native.height == portable.height &&
                native.bytes.size() == portable.bytes.size(), "WIC comparison geometry");
        check_quadrants(native, 1U);
        for (unsigned orientation = 2U; orientation <= 8U; ++orientation) {
            // Fresh native decode avoids copying a large raster just to test
            // the shared separately allocated transform.
            Pixels unrotated = jpeg_research::decode_wic(encoded, portable.width, portable.height);
            const Pixels rotated = orient(std::move(unrotated), orientation);
            check_quadrants(rotated, orientation);
        }
    }
    const std::vector<unsigned char> gray = make_fixture(320, 192, false, true);
    const Pixels grayscale = jpeg_research::decode_wic(gray, 320, 192);
    for (std::size_t offset = 0U; offset < grayscale.bytes.size(); offset += 4U) {
        require(grayscale.bytes[offset] == grayscale.bytes[offset + 1U] &&
                grayscale.bytes[offset] == grayscale.bytes[offset + 2U] &&
                grayscale.bytes[offset + 3U] == 255U, "WIC grayscale output");
    }
    bool refused = false;
    try { const Pixels invalid = jpeg_research::decode_wic({}, 320, 192); static_cast<void>(invalid); }
    catch (const std::runtime_error&) { refused = true; }
    require(refused, "WIC empty input must refuse");
    refused = false;
    try { const Pixels invalid = jpeg_research::decode_wic(gray, 1025, 192); static_cast<void>(invalid); }
    catch (const std::runtime_error&) { refused = true; }
    require(refused, "WIC oversized output must refuse");
    constexpr std::array<unsigned char, 4U> invalid_header{1U, 2U, 3U, 4U};
    refused = false;
    try { const Pixels invalid = jpeg_research::decode_wic(invalid_header, 320, 192); static_cast<void>(invalid); }
    catch (const std::runtime_error&) { refused = true; }
    require(refused, "WIC invalid JPEG must refuse after COM acquisition");
    refused = false;
    try { const Pixels invalid = jpeg_research::decode_wic(gray, 321, 192); static_cast<void>(invalid); }
    catch (const std::runtime_error&) { refused = true; }
    require(refused, "WIC enlargement must refuse after frame acquisition");
    const Pixels recovered = jpeg_research::decode_wic(gray, 320, 192);
    require(recovered.bytes == grayscale.bytes, "WIC failure cleanup must permit the next valid decode");
    std::cerr << "PASS WIC baseline/progressive shared geometry, orientation, alpha, grayscale and input/output refusal\n";
}
#endif

void measure(const char* name, const int width, const int height, const bool progressive) {
    const std::vector<unsigned char> encoded = make_fixture(width, height, progressive, false);
    for (unsigned orientation : {1U, 6U}) {
        for (unsigned repetition = 0U; repetition < 31U; ++repetition) {
            const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            const Pixels raster = decode(encoded, orientation);
            const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            check_quadrants(raster, orientation);
            const std::chrono::duration<double, std::milli> elapsed = end - begin;
            const double milliseconds = elapsed.count();
            std::cout << name << ',' << orientation << ',' << repetition << ',' << encoded.size() << ','
                << raster.width << ',' << raster.height << ',' << raster.bytes.size() << ',' << milliseconds << '\n';
        }
    }
}

#ifdef _WIN32
void measure_wic(const char* const name, const int width, const int height, const bool progressive) {
    const std::vector<unsigned char> encoded = make_fixture(width, height, progressive, false);
    // Resolve comparison dimensions once, outside WIC timing. This makes output
    // extents identical without making the portable codec part of the WIC call.
    const Pixels geometry = decode(encoded, 1U);
    for (unsigned orientation : {1U, 6U}) {
        for (unsigned repetition = 0U; repetition < 31U; ++repetition) {
            const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            Pixels unrotated = jpeg_research::decode_wic(encoded, geometry.width, geometry.height);
            const Pixels raster = orient(std::move(unrotated), orientation);
            const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            check_quadrants(raster, orientation);
            const std::chrono::duration<double, std::milli> elapsed = end - begin;
            const double milliseconds = elapsed.count();
            std::cout << name << ',' << orientation << ',' << repetition << ',' << encoded.size() << ','
                << raster.width << ',' << raster.height << ',' << raster.bytes.size() << ',' << milliseconds << '\n';
        }
    }
}
#endif

} // namespace

int main(const int argc, char** const argv) {
    try {
        if (argc == 3) {
            const std::string_view mode(argv[1]);
            const std::string_view selected(argv[2]);
            const bool plain_frame = mode == "--emit-frame";
            const bool color_frame = mode == "--emit-color-frame";
            const bool plain_encoded = mode == "--emit-encoded";
            const bool color_encoded = mode == "--emit-color-encoded";
            require((plain_frame || color_frame || plain_encoded || color_encoded) && selected.size() == 1 &&
                    selected[0] >= '1' && selected[0] <= '8', "closed frame fixture arguments");
            const unsigned char orientation = static_cast<unsigned char>(selected[0] - '0');
            const bool linear = color_frame || color_encoded;
            const bool encoded_only = plain_encoded || color_encoded;
            emit_generated_frame(orientation, linear, encoded_only);
            return 0;
        }
        const bool measure_requested = argc == 2 && std::string_view(argv[1]) == "--measure";
        const bool verify_requested = argc == 1 ||
            (argc == 2 && std::string_view(argv[1]) == "--verify-only");
#ifdef _WIN32
        if (argc == 2 && std::string_view(argv[1]) == "--measure-wic") {
            verify_wic_control();
            std::cout << "fixture,orientation,repetition,encoded_bytes,width,height,output_bytes,decode_ms\n";
            measure_wic("baseline-24mp", 6000, 4000, false);
            measure_wic("progressive-3mp", 2048, 1536, true);
            return 0;
        }
        if (argc == 2 && std::string_view(argv[1]) == "--verify-wic") {
            verify_wic_control();
            return 0;
        }
#endif
        require(measure_requested || verify_requested, "use --verify-only or --measure");
        verify();
        if (!measure_requested) return 0;
        std::cout << "fixture,orientation,repetition,encoded_bytes,width,height,output_bytes,decode_ms\n";
        measure("baseline-24mp", 6000, 4000, false);
        measure("progressive-3mp", 2048, 1536, true);
    } catch (const std::exception& error) {
        std::cerr << "FAIL " << error.what() << '\n';
        return 1;
    }
    return 0;
}
