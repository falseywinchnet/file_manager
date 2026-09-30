#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace gui_forms {

/// A borrowed, top-to-bottom, straight-alpha RGBA8 sRGB image.
/// Row padding is permitted; the final row does not require trailing padding.
/// The producer retains ownership and keeps the bytes alive for the whole call.
struct HostImageView final {
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint64_t row_bytes{};
    std::span<const std::byte> pixels;
};

/// An owned host-service image. Color channels are not premultiplied.
/// Clipboard reads return independent storage, including RGB under zero alpha
/// when the source representation preserves those channels.
struct HostImage final {
    static constexpr std::uint64_t maximum_pixels = 100'000'000U;
    static constexpr std::uint64_t maximum_bytes = maximum_pixels * 4U;
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint64_t row_bytes{};
    std::vector<std::byte> pixels;

    [[nodiscard]] HostImageView view() const noexcept {
        return {width, height, row_bytes, pixels};
    }
};

enum class HostImageError : std::uint8_t {
    none,
    invalid_dimensions,
    too_large,
    invalid_stride,
    short_buffer,
};

/// Validates dimensions, the 100-megapixel bound and the readable row span.
/// Performs no allocation and never multiplies an unchecked source stride.
[[nodiscard]] HostImageError validate_host_image(HostImageView image) noexcept;

} // namespace gui_forms
