#include "gui_forms/host/image/host_image.hpp"

#include <limits>

namespace gui_forms {
HostImageError validate_host_image(HostImageView image) noexcept {
    if (image.width == 0U || image.height == 0U) {
        return HostImageError::invalid_dimensions;
    }
    const std::uint64_t pixels = static_cast<std::uint64_t>(image.width) * image.height;
    if (pixels > HostImage::maximum_pixels) return HostImageError::too_large;
    const std::uint64_t row = static_cast<std::uint64_t>(image.width) * 4U;
    if (image.row_bytes < row ||
        image.row_bytes > std::numeric_limits<std::size_t>::max()) {
        return HostImageError::invalid_stride;
    }
    if (image.pixels.size() < row ||
        image.height - 1U > (image.pixels.size() - row) / image.row_bytes) {
        return HostImageError::short_buffer;
    }
    return HostImageError::none;
}
} // namespace gui_forms
