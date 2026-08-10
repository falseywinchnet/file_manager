#include "../support/drawing_support.hpp"

ImageReference::ImageReference(std::uint64_t stable_id, std::uint32_t width,
                               std::uint32_t height, PixelFormat pixel_format,
                               std::uint64_t generation) {
    require_enum(pixel_format, 1U, "image pixel format");
    if (stable_id == 0U || generation == 0U || width == 0U || height == 0U ||
        width > 32768U || height > 32768U) {
        throw std::invalid_argument(
            "image reference requires nonzero identity, generation, and bounded dimensions");
    }
    value_.stable_id = stable_id;
    value_.width = width;
    value_.height = height;
    value_.pixel_format = pixel_format;
    value_.generation = generation;
}

ImageSnapshot ImageReference::snapshot() const {
    require_alive();
    return value_;
}

bool ImageSnapshot::has_pixels() const noexcept {
    return storage_ != nullptr;
}

std::size_t ImageSnapshot::row_bytes() const noexcept {
    return storage_ ? storage_->row_bytes : 0U;
}

std::span<const std::byte> ImageSnapshot::pixels() const noexcept {
    return storage_ ? std::span<const std::byte>(storage_->bytes) :
                      std::span<const std::byte>{};
}


} // namespace gui_drawing

