#include "icc_fixtures.hpp"
#include "icc_color.hpp"

#include <lcms2.h>

#include <cmath>
#include <cstddef>
#include <iostream>
#include <stdexcept>

namespace jpeg_research {
namespace {

void require(const bool condition, const char* const message) {
    if (!condition) throw std::runtime_error(message);
}

// Fixture owners deliberately acquire after construction: failures later in a
// sequence still release every earlier resource. No vendor handle escapes.
struct FixtureResources final {
    cmsContext context{};
    cmsToneCurve* curve{};
    cmsHPROFILE profile{};
    FixtureResources() = default;
    ~FixtureResources() {
        if (profile != nullptr) cmsCloseProfile(profile);
        if (curve != nullptr) cmsFreeToneCurve(curve);
        if (context != nullptr) cmsDeleteContext(context);
    }
    FixtureResources(const FixtureResources&) = delete;
    FixtureResources& operator=(const FixtureResources&) = delete;
};

void require_refusal(const Pixels& source, const std::span<const unsigned char> profile,
    const IccSource space) {
    bool refused{};
    try {
        const Pixels result = convert_icc_to_srgb(source, profile, space);
        static_cast<void>(result);
    } catch (const std::runtime_error&) { refused = true; }
    require(refused, "ICC invalid input must refuse");
}

void check_linear_ramp(const Pixels& result, const char* const label) {
    for (std::size_t index = 0U; index < 256U; ++index) {
        const int expected = encode_linear_srgb(static_cast<unsigned char>(index));
        for (std::size_t channel = 0U; channel < 3U; ++channel) {
            const int actual = static_cast<int>(result.bytes[index * 4U + channel]);
            const int difference = actual - expected;
            if (difference < -2 || difference > 2) {
                std::cerr << label << " sample=" << index << " channel=" << channel <<
                    " expected=" << expected << " actual=" << actual << '\n';
            }
            require(difference >= -2 && difference <= 2, "ICC linear ramp differs from sRGB equation");
        }
        require(result.bytes[index * 4U + 3U] == 255U, "ICC conversion must preserve opaque alpha");
    }
}

} // namespace

std::vector<unsigned char> make_icc_fixture(const IccFixture kind) {
    FixtureResources resources{};
    resources.context = cmsCreateContext(nullptr, nullptr);
    require(resources.context != nullptr, "fixture color context");
    if (kind == IccFixture::srgb) {
        resources.profile = cmsCreate_sRGBProfileTHR(resources.context);
    } else {
        resources.curve = cmsBuildGamma(resources.context, 1.0);
        require(resources.curve != nullptr, "fixture linear curve");
        if (kind == IccFixture::linear_gray) {
            const cmsCIExyY* const white = cmsD50_xyY();
            resources.profile = cmsCreateGrayProfileTHR(resources.context, white, resources.curve);
        } else {
            require(kind == IccFixture::linear_rgb, "fixture profile kind");
            const cmsCIExyY white{0.3127, 0.3290, 1.0};
            const cmsCIExyYTRIPLE primaries{{0.64, 0.33, 1.0}, {0.30, 0.60, 1.0}, {0.15, 0.06, 1.0}};
            cmsToneCurve* curves[3]{resources.curve, resources.curve, resources.curve};
            resources.profile = cmsCreateRGBProfileTHR(resources.context, &white, &primaries, curves);
        }
    }
    require(resources.profile != nullptr, "fixture color profile");
    cmsUInt32Number size{};
    const cmsBool sized = cmsSaveProfileToMem(resources.profile, nullptr, &size);
    require(sized != 0 && size >= 132U && size <= icc_profile_limit, "fixture profile size");
    std::vector<unsigned char> bytes(size, 0U);
    const cmsBool saved = cmsSaveProfileToMem(resources.profile, bytes.data(), &size);
    require(saved != 0 && size == bytes.size(), "fixture profile serialization");
    return bytes;
}

int encode_linear_srgb(const unsigned char sample) {
    const double linear = static_cast<double>(sample) / 255.0;
    double encoded{};
    if (linear <= 0.0031308) {
        encoded = linear * 12.92;
    } else {
        encoded = 1.055 * std::pow(linear, 1.0 / 2.4) - 0.055;
    }
    const double rounded = std::floor(encoded * 255.0 + 0.5);
    const int result = static_cast<int>(rounded);
    return result;
}

void verify_icc_conversion() {
    Pixels source{};
    source.width = 256;
    source.height = 1;
    source.row_bytes = 1024;
    source.bytes.resize(1024U, 255U);
    for (std::size_t index = 0U; index < 256U; ++index) {
        const unsigned char value = static_cast<unsigned char>(index);
        source.bytes[index * 4U] = value;
        source.bytes[index * 4U + 1U] = value;
        source.bytes[index * 4U + 2U] = value;
    }
    const std::vector<unsigned char> srgb = make_icc_fixture(IccFixture::srgb);
    const std::vector<unsigned char> linear_rgb = make_icc_fixture(IccFixture::linear_rgb);
    const std::vector<unsigned char> linear_gray = make_icc_fixture(IccFixture::linear_gray);
    const Pixels identity = convert_icc_to_srgb(source, srgb, IccSource::rgb);
    require(identity.bytes == source.bytes, "sRGB profile should preserve the 8-bit neutral ramp");
    const Pixels converted_rgb = convert_icc_to_srgb(source, linear_rgb, IccSource::rgb);
    check_linear_ramp(converted_rgb, "linear RGB");
    const Pixels converted_gray = convert_icc_to_srgb(source, linear_gray, IccSource::gray);
    check_linear_ramp(converted_gray, "linear gray");
    require(converted_rgb.bytes[128U * 4U] > source.bytes[128U * 4U] + 50U,
        "linear profile must not silently be ignored");
    require_refusal(source, {}, IccSource::rgb);
    require_refusal(source, std::span<const unsigned char>(srgb.data(), srgb.size() - 1U), IccSource::rgb);
    const std::vector<unsigned char> oversized(icc_profile_limit + 1U, 0U);
    require_refusal(source, oversized, IccSource::rgb);
    require_refusal(source, linear_gray, IccSource::rgb);
    require_refusal(source, srgb, IccSource::gray);
    std::vector<unsigned char> malformed = srgb;
    malformed[36U] = 0U; // ICC magic, independent of its still-correct extent.
    require_refusal(source, malformed, IccSource::rgb);
    std::vector<unsigned char> device_link = srgb;
    device_link[12U] = 'l'; device_link[13U] = 'i'; device_link[14U] = 'n'; device_link[15U] = 'k';
    require_refusal(source, device_link, IccSource::rgb);
    Pixels invalid = source; // Explicit small fixture copy; original stays valid.
    invalid.row_bytes = 1023;
    require_refusal(invalid, srgb, IccSource::rgb);
    invalid.row_bytes = source.row_bytes;
    invalid.bytes[3U] = 0U;
    require_refusal(invalid, srgb, IccSource::rgb);
    invalid.bytes[3U] = 255U;
    invalid.bytes[1U] = 1U;
    require_refusal(invalid, linear_gray, IccSource::gray);
    const Pixels recovered = convert_icc_to_srgb(source, srgb, IccSource::rgb);
    require(recovered.bytes == source.bytes, "color failures must not affect the next invocation");
}

} // namespace jpeg_research
