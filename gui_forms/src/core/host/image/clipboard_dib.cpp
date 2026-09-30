#include "clipboard_dib.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace gui_forms::detail {
namespace {
std::uint32_t u32(std::span<const std::byte> bytes, std::size_t offset) noexcept {
    std::uint32_t value = 0;
    for (unsigned index = 0; index < 4; ++index) value |= std::to_integer<std::uint32_t>(bytes[offset + index]) << (8U * index);
    return value;
}
std::uint16_t u16(std::span<const std::byte> bytes, std::size_t offset) noexcept {
    return static_cast<std::uint16_t>(std::to_integer<unsigned>(bytes[offset]) |
        (std::to_integer<unsigned>(bytes[offset + 1]) << 8U));
}
void put32(std::span<std::byte> bytes, std::size_t offset, std::uint32_t value) noexcept {
    for (unsigned index = 0; index < 4; ++index) bytes[offset + index] = static_cast<std::byte>(value >> (8U * index));
}
bool byte_mask(std::uint32_t mask) noexcept {
    return mask == 0x000000ffU || mask == 0x0000ff00U || mask == 0x00ff0000U || mask == 0xff000000U;
}
unsigned shift_for(std::uint32_t mask) noexcept {
    unsigned shift = 0;
    while ((mask & 255U) == 0U && shift < 24U) { mask >>= 8U; shift += 8U; }
    return shift;
}
}
std::vector<std::byte> encode_clipboard_dib(HostImageView image) {
    if (validate_host_image(image) != HostImageError::none) throw std::invalid_argument("invalid clipboard image");
    const std::size_t stride = static_cast<std::size_t>(image.width) * 4U;
    std::vector<std::byte> bytes(124U + stride * image.height);
    put32(bytes, 0, 124U); // BITMAPV5HEADER
    put32(bytes, 4, image.width);
    put32(bytes, 8, 0U - image.height); // top-down signed height
    bytes[12] = std::byte{1};
    bytes[14] = std::byte{32};
    put32(bytes, 16, 3U); // BI_BITFIELDS
    put32(bytes, 20, static_cast<std::uint32_t>(stride * image.height));
    put32(bytes, 40, 0x00ff0000U);
    put32(bytes, 44, 0x0000ff00U);
    put32(bytes, 48, 0x000000ffU);
    put32(bytes, 52, 0xff000000U);
    put32(bytes, 56, 0x73524742U); // LCS_sRGB
    put32(bytes, 108, 4U); // LCS_GM_IMAGES
    for (std::uint32_t y = 0; y < image.height; ++y) {
        const std::byte* source = image.pixels.data() + static_cast<std::size_t>(image.row_bytes) * y;
        std::byte* target = bytes.data() + 124U + stride * y;
        for (std::uint32_t x = 0; x < image.width; ++x) {
            const std::size_t offset = static_cast<std::size_t>(x) * 4U;
            target[offset] = source[offset + 2];
            target[offset + 1] = source[offset + 1];
            target[offset + 2] = source[offset];
            target[offset + 3] = source[offset + 3];
        }
    }
    return bytes;
}
HostClipboardImageResult decode_clipboard_dib(std::span<const std::byte> bytes) {
    HostClipboardImageResult result;
    result.status.error = HostServiceError::backend_failure;
    if (bytes.size() < 40U) return result;
    const std::uint32_t header = u32(bytes, 0);
    if ((header != 40U && header != 108U && header != 124U) || bytes.size() < header) return result;
    const std::uint32_t width = u32(bytes, 4);
    const std::uint32_t signed_height = u32(bytes, 8);
    const bool top_down = (signed_height & 0x80000000U) != 0U;
    const std::uint32_t height = top_down ? 0U - signed_height : signed_height;
    if (width == 0U || width > 0x7fffffffU || height == 0U || height > 0x7fffffffU || u16(bytes, 12) != 1U) return result;
    const std::uint64_t pixel_count = static_cast<std::uint64_t>(width) * height;
    if (pixel_count > HostImage::maximum_pixels) { result.status.error = HostServiceError::too_large; return result; }
    const std::uint16_t bits = u16(bytes, 14);
    const std::uint32_t compression = u32(bytes, 16);
    std::size_t start = header;
    std::uint32_t masks[4]{0x00ff0000U, 0x0000ff00U, 0x000000ffU, 0U};
    if ((bits != 24U && bits != 32U) || (compression != 0U && compression != 3U) ||
        (bits == 24U && compression != 0U)) {
        result.status.error = HostServiceError::unsupported; return result;
    }
    if (header >= 108U) {
        const std::uint32_t color_space = u32(bytes, 56);
        if (color_space != 0x73524742U && color_space != 0x57696e20U) {
            result.status.error = HostServiceError::unsupported; return result;
        }
    }
    if (compression == 3U) {
        if (header == 40U) {
            if (bytes.size() < 52U) return result;
            start += 12U;
        }
        for (std::size_t index = 0; index < 3U; ++index) masks[index] = u32(bytes, 40U + index * 4U);
        if (header >= 108U) masks[3] = u32(bytes, 52);
        std::uint32_t occupied = 0U;
        for (std::size_t index = 0; index < 4U; ++index) {
            if (index == 3U && masks[index] == 0U) continue;
            if (!byte_mask(masks[index]) || (occupied & masks[index]) != 0U) {
                result.status.error = HostServiceError::unsupported; return result;
            }
            occupied |= masks[index];
        }
    }
    const std::uint32_t palette = u32(bytes, 32);
    if (palette > 256U || palette * 4U > bytes.size() - start) return result;
    start += palette * 4U;
    const std::uint64_t stride = ((static_cast<std::uint64_t>(width) * bits + 31U) / 32U) * 4U;
    if (stride * height > bytes.size() - start) return result;
    result.image.width = width;
    result.image.height = height;
    result.image.row_bytes = static_cast<std::uint64_t>(width) * 4U;
    result.image.pixels.resize(static_cast<std::size_t>(pixel_count * 4U));
    for (std::uint32_t y = 0; y < height; ++y) {
        const std::uint32_t source_y = top_down ? y : height - 1U - y;
        const std::byte* source = bytes.data() + start + static_cast<std::size_t>(stride) * source_y;
        std::byte* target = result.image.pixels.data() + static_cast<std::size_t>(result.image.row_bytes) * y;
        for (std::uint32_t x = 0; x < width; ++x) {
            const std::size_t dest = static_cast<std::size_t>(x) * 4U;
            if (bits == 24U) {
                const std::size_t offset = static_cast<std::size_t>(x) * 3U;
                target[dest] = source[offset + 2U]; target[dest + 1U] = source[offset + 1U];
                target[dest + 2U] = source[offset]; target[dest + 3U] = std::byte{255};
            } else {
                const std::uint32_t pixel = u32({source + dest, 4U}, 0);
                for (std::size_t channel = 0; channel < 4U; ++channel) {
                    target[dest + channel] = channel == 3U && masks[3] == 0U ? std::byte{255} :
                        static_cast<std::byte>((pixel & masks[channel]) >> shift_for(masks[channel]));
                }
            }
        }
    }
    result.status = {};
    result.has_image = true;
    return result;
}
} // namespace gui_forms::detail
