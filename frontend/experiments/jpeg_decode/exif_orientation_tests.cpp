#include "exif_orientation.hpp"

#include <array>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

using jpeg_research::OrientationStatus;

void require(const bool accepted, const char* const message) {
    if (!accepted) throw std::runtime_error(message);
}

// Independent literal TIFF/JPEG layout. Orientation's SHORT is inline in IFD0.
constexpr std::array<unsigned char, 42U> little_fixture{
    0xff, 0xd8, 0xff, 0xe1, 0x00, 0x22, 'E', 'x', 'i', 'f', 0, 0,
    'I', 'I', 42, 0, 8, 0, 0, 0, 1, 0,
    0x12, 0x01, 3, 0, 1, 0, 0, 0, 6, 0, 0, 0,
    0, 0, 0, 0, 0xff, 0xda, 0, 2,
};
constexpr std::array<unsigned char, 42U> big_fixture{
    0xff, 0xd8, 0xff, 0xe1, 0x00, 0x22, 'E', 'x', 'i', 'f', 0, 0,
    'M', 'M', 0, 42, 0, 0, 0, 8, 0, 1,
    0x01, 0x12, 0, 3, 0, 0, 0, 1, 0, 6, 0, 0,
    0, 0, 0, 0, 0xff, 0xda, 0, 2,
};

void expect(const std::span<const unsigned char> input, const OrientationStatus status,
            const std::uint16_t orientation, const char* const message) {
    const jpeg_research::OrientationResult result = jpeg_research::read_orientation(input);
    require(result.status == status && result.value == orientation, message);
}

void verify_endian_and_absence() {
    for (unsigned char orientation = 1U; orientation <= 8U; ++orientation) {
        std::array<unsigned char, 42U> little = little_fixture;
        std::array<unsigned char, 42U> big = big_fixture;
        little[30U] = orientation;
        big[31U] = orientation;
        expect(little, OrientationStatus::present, orientation, "little-endian orientation");
        expect(big, OrientationStatus::present, orientation, "big-endian orientation");
    }
    const std::array<unsigned char, 6U> no_exif{0xff, 0xd8, 0xff, 0xda, 0, 2};
    expect(no_exif, OrientationStatus::absent, 1U, "missing EXIF defaults to one");
    std::array<unsigned char, 42U> other_tag = little_fixture;
    other_tag[22U] = 0x13U;
    expect(other_tag, OrientationStatus::absent, 1U, "unrelated IFD tag is not orientation");
    other_tag[34U] = 8U; // Deliberate next-IFD cycle: the primary-only parser never follows it.
    expect(other_tag, OrientationStatus::absent, 1U, "next IFD is outside primary-orientation scope");
    std::array<unsigned char, 42U> xmp = little_fixture;
    xmp[6U] = 'X';
    expect(xmp, OrientationStatus::absent, 1U, "non-EXIF APP1 is skipped");
}

void verify_malformed() {
    for (std::size_t size = 0U; size < little_fixture.size(); ++size) {
        const std::span<const unsigned char> prefix(little_fixture.data(), size);
        expect(prefix, OrientationStatus::malformed, 1U, "every truncated header prefix refuses");
    }
    const std::array<std::size_t, 8U> positions{0U, 4U, 10U, 12U, 14U, 16U, 24U, 26U};
    const std::array<unsigned char, 8U> corruptions{0U, 0xffU, 1U, 'Z', 43U, 0xffU, 4U, 2U};
    for (std::size_t index = 0U; index < positions.size(); ++index) {
        std::array<unsigned char, 42U> damaged = little_fixture;
        damaged[positions[index]] = corruptions[index];
        expect(damaged, OrientationStatus::malformed, 1U, "malformed EXIF structure refuses");
    }
    constexpr std::array<unsigned char, 3U> invalid_values{0U, 9U, 255U};
    for (const unsigned char value : invalid_values) {
        std::array<unsigned char, 42U> damaged = little_fixture;
        damaged[30U] = value;
        expect(damaged, OrientationStatus::malformed, 1U, "invalid orientation refuses");
    }
    std::vector<unsigned char> duplicate(little_fixture.begin(), little_fixture.begin() + 38);
    duplicate.insert(duplicate.end(), little_fixture.begin() + 2, little_fixture.end());
    expect(duplicate, OrientationStatus::malformed, 1U, "duplicate EXIF marker refuses");
    std::vector<unsigned char> duplicate_tag(little_fixture.begin(), little_fixture.end());
    duplicate_tag.insert(duplicate_tag.begin() + 34, little_fixture.begin() + 22, little_fixture.begin() + 34);
    duplicate_tag[5U] = 0x2eU;
    duplicate_tag[20U] = 2U;
    expect(duplicate_tag, OrientationStatus::malformed, 1U, "duplicate orientation tag refuses");
}

void verify_bounds_and_separators() {
    std::vector<unsigned char> filled{0xff, 0xd8, 0xff, 0xff, 0x01};
    filled.insert(filled.end(), little_fixture.begin() + 2, little_fixture.end());
    expect(filled, OrientationStatus::present, 6U, "fill bytes and standalone TEM");
    std::array<unsigned char, 42U> excessive = little_fixture;
    excessive[20U] = 1U;
    excessive[21U] = 16U;
    expect(excessive, OrientationStatus::limit, 1U, "IFD-entry work bound");
    std::vector<unsigned char> markers(8194U, 0U);
    markers[0U] = 0xffU;
    markers[1U] = 0xd8U;
    for (std::size_t index = 0U; index < 4096U; ++index) {
        const std::size_t offset = 2U + index * 2U;
        markers[offset] = 0xffU;
        markers[offset + 1U] = 0x01U;
    }
    expect(markers, OrientationStatus::limit, 1U, "marker-count work bound");
    std::vector<unsigned char> large(1024U * 1024U + 1U, 0xffU);
    large[1U] = 0xd8U;
    expect(large, OrientationStatus::limit, 1U, "header fill-byte work bound");
    large.resize(16U * 1024U * 1024U + 1U);
    expect(large, OrientationStatus::limit, 1U, "complete encoded-byte bound");
    const std::array<unsigned char, 8U> stuffed{0xff, 0xd8, 0xff, 0x00, 0xff, 0xda, 0, 2};
    expect(stuffed, OrientationStatus::malformed, 1U, "stuffed bytes are not header markers");
}

} // namespace

int main() {
    try {
        verify_endian_and_absence();
        verify_malformed();
        verify_bounds_and_separators();
        std::cout << "PASS EXIF IFD0 orientation in both byte orders; absent, malformed, ambiguous and bounded states\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
