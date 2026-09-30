#include "clipboard_image_wire.hpp"

#include <algorithm>
#include <array>
#include <stdexcept>

namespace gui_forms::detail {
namespace {
constexpr std::array<std::byte, 4> magic{std::byte{'G'}, std::byte{'F'}, std::byte{'I'}, std::byte{'M'}};
void write_u32(std::span<std::byte> bytes, std::size_t offset, std::uint32_t value) noexcept {
    for (unsigned index = 0; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>((value >> (8U * index)) & 255U);
    }
}
std::uint32_t read_u32(std::span<const std::byte> bytes, std::size_t offset) noexcept {
    std::uint32_t value = 0U;
    for (unsigned index = 0; index < 4U; ++index) {
        value |= std::to_integer<std::uint32_t>(bytes[offset + index]) << (8U * index);
    }
    return value;
}
void copy_rows(HostImageView image, std::span<std::byte> destination) {
    const std::size_t row_bytes = static_cast<std::size_t>(image.width) * 4U;
    for (std::uint32_t row = 0; row < image.height; ++row) {
        std::copy_n(image.pixels.data() + static_cast<std::size_t>(image.row_bytes) * row,
                    row_bytes, destination.data() + row_bytes * row);
    }
}
} // namespace
HostImage copy_host_image(HostImageView image) {
    if (validate_host_image(image) != HostImageError::none) {
        throw std::invalid_argument("invalid host image view");
    }
    HostImage result;
    result.width = image.width;
    result.height = image.height;
    result.row_bytes = static_cast<std::uint64_t>(image.width) * 4U;
    result.pixels.resize(static_cast<std::size_t>(result.row_bytes) * image.height);
    copy_rows(image, result.pixels);
    return result;
}
std::vector<std::byte> encode_clipboard_image(HostImageView image) {
    if (validate_host_image(image) != HostImageError::none) {
        throw std::invalid_argument("invalid clipboard image view");
    }
    std::vector<std::byte> bytes(16U + static_cast<std::size_t>(image.width) * image.height * 4U);
    std::copy(magic.begin(), magic.end(), bytes.begin());
    write_u32(bytes, 4, 1U);
    write_u32(bytes, 8, image.width);
    write_u32(bytes, 12, image.height);
    copy_rows(image, std::span<std::byte>(bytes).subspan(16));
    return bytes;
}
HostClipboardImageResult decode_clipboard_image(
    std::span<const std::byte> bytes, bool allow_allocation_padding) {
    HostClipboardImageResult result;
    result.status.error = HostServiceError::backend_failure;
    if (bytes.size() < 16U || !std::equal(magic.begin(), magic.end(), bytes.begin()) ||
        read_u32(bytes, 4) != 1U) return result;
    const std::uint32_t width = read_u32(bytes, 8);
    const std::uint32_t height = read_u32(bytes, 12);
    const std::uint64_t pixel_count = static_cast<std::uint64_t>(width) * height;
    if (pixel_count > HostImage::maximum_pixels) {
        result.status.error = HostServiceError::too_large;
        return result;
    }
    const std::uint64_t expected = pixel_count * 4U;
    if (bytes.size() - 16U < expected ||
        (!allow_allocation_padding && bytes.size() - 16U != expected)) return result;
    const HostImageView view{width, height, static_cast<std::uint64_t>(width) * 4U,
                             bytes.subspan(16, static_cast<std::size_t>(expected))};
    if (validate_host_image(view) != HostImageError::none) return result;
    result.image = copy_host_image(view);
    result.status = {};
    result.has_image = true;
    return result;
}
} // namespace gui_forms::detail
