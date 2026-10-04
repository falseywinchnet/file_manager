#include "exif_orientation.hpp"

namespace jpeg_research {
namespace {

constexpr std::size_t encoded_limit = 16U * 1024U * 1024U;
constexpr std::size_t header_limit = 1024U * 1024U;
constexpr std::size_t marker_limit = 4096U;
constexpr std::size_t entry_limit = 4096U;

constexpr OrientationResult malformed_result{OrientationStatus::malformed, 1U};
constexpr OrientationResult limit_result{OrientationStatus::limit, 1U};

// Callers validate the complete field extent before these fixed-width reads.
template <bool Little>
std::uint16_t read16(const std::span<const unsigned char> bytes, const std::size_t offset) noexcept {
    const std::uint16_t first = bytes[offset];
    const std::uint16_t second = bytes[offset + 1U];
    if constexpr (Little) {
        const std::uint16_t value = static_cast<std::uint16_t>(first | (second << 8U));
        return value;
    } else {
        const std::uint16_t value = static_cast<std::uint16_t>((first << 8U) | second);
        return value;
    }
}

template <bool Little>
std::uint32_t read32(const std::span<const unsigned char> bytes, const std::size_t offset) noexcept {
    const std::uint32_t first = read16<Little>(bytes, offset);
    const std::uint32_t second = read16<Little>(bytes, offset + 2U);
    if constexpr (Little) {
        const std::uint32_t value = first | (second << 16U);
        return value;
    } else {
        const std::uint32_t value = (first << 16U) | second;
        return value;
    }
}

template <bool Little>
OrientationResult parse_ifd0(const std::span<const unsigned char> tiff) noexcept {
    if (read16<Little>(tiff, 2U) != 42U) return malformed_result;
    const std::uint32_t relative = read32<Little>(tiff, 4U);
    const std::size_t offset = static_cast<std::size_t>(relative);
    if (offset < 8U || offset > tiff.size() || tiff.size() - offset < 2U) {
        return malformed_result;
    }
    const std::size_t count = read16<Little>(tiff, offset);
    if (count > entry_limit) return limit_result;
    const std::size_t begin = offset + 2U;
    const std::size_t available = tiff.size() - begin;
    // Every entry is 12 bytes, followed by the four-byte next-IFD field.
    if (available < 4U || count > (available - 4U) / 12U) {
        return malformed_result;
    }
    OrientationResult result{};
    for (std::size_t index = 0U; index < count; ++index) {
        const std::size_t entry = begin + index * 12U;
        const std::uint16_t tag = read16<Little>(tiff, entry);
        if (tag != 0x0112U) continue;
        if (result.status == OrientationStatus::present) return malformed_result;
        const std::uint16_t type = read16<Little>(tiff, entry + 2U);
        const std::uint32_t elements = read32<Little>(tiff, entry + 4U);
        const std::uint16_t value = read16<Little>(tiff, entry + 8U);
        if (type != 3U || elements != 1U || value < 1U || value > 8U) {
            return malformed_result;
        }
        result.status = OrientationStatus::present;
        result.value = value;
    }
    return result;
}

OrientationResult parse_exif(const std::span<const unsigned char> tiff) noexcept {
    if (tiff.size() < 8U) return malformed_result;
    if (tiff[0] == 'I' && tiff[1] == 'I') {
        const OrientationResult result = parse_ifd0<true>(tiff);
        return result;
    }
    if (tiff[0] == 'M' && tiff[1] == 'M') {
        const OrientationResult result = parse_ifd0<false>(tiff);
        return result;
    }
    return malformed_result;
}

} // namespace

OrientationResult read_orientation(const std::span<const unsigned char> encoded) noexcept {
    if (encoded.size() > encoded_limit) return limit_result;
    if (encoded.size() < 2U || encoded[0] != 0xffU || encoded[1] != 0xd8U) {
        return malformed_result;
    }
    std::size_t offset = 2U;
    bool exif_seen = false;
    OrientationResult result{};
    for (std::size_t marker = 0U; marker < marker_limit; ++marker) {
        if (offset >= header_limit) return limit_result;
        if (offset >= encoded.size() || encoded[offset] != 0xffU) {
            return malformed_result;
        }
        while (offset < encoded.size() && encoded[offset] == 0xffU) {
            ++offset;
            if (offset >= header_limit) return limit_result;
        }
        if (offset == encoded.size()) return malformed_result;
        const unsigned char marker_code = encoded[offset];
        ++offset;
        if (marker_code == 0x01U) continue; // TEM has no length field.
        if (marker_code == 0U || marker_code == 0xd8U || marker_code == 0xd9U ||
            (marker_code >= 0xd0U && marker_code <= 0xd7U)) {
            return malformed_result;
        }
        if (encoded.size() - offset < 2U) return malformed_result;
        const std::size_t length = read16<false>(encoded, offset);
        if (length < 2U || length > encoded.size() - offset) return malformed_result;
        if (length > header_limit - offset) return limit_result;
        const std::span<const unsigned char> payload = encoded.subspan(offset + 2U, length - 2U);
        if (marker_code == 0xdaU) return result; // Entropy-coded data belongs to the codec.
        if (marker_code == 0xe1U && payload.size() >= 4U &&
            payload[0] == 'E' && payload[1] == 'x' && payload[2] == 'i' && payload[3] == 'f') {
            if (exif_seen || payload.size() < 6U || payload[4] != 0U || payload[5] != 0U) {
                return malformed_result;
            }
            exif_seen = true;
            result = parse_exif(payload.subspan(6U));
            if (result.status == OrientationStatus::malformed || result.status == OrientationStatus::limit) {
                return result;
            }
        }
        offset += length;
    }
    return limit_result;
}

} // namespace jpeg_research
