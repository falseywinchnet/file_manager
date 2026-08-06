#pragma once

#include "gui_forms/types.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace gui_forms {

enum class PngColorType : std::uint8_t {
    grayscale = 0,
    truecolor = 2,
    indexed = 3,
    grayscale_alpha = 4,
    truecolor_alpha = 6,
};

enum class ImageResourceEncoding : std::uint8_t {
    png,
    bgra32_premultiplied,
};

struct ImageRegistryLimits final {
    std::size_t maximum_encoded_bytes_per_image{32U * 1024U * 1024U};
    std::uint32_t maximum_width{4096};
    std::uint32_t maximum_height{4096};
    std::uint64_t maximum_pixels{4096ULL * 4096ULL};
    std::uint64_t maximum_decoded_bytes_per_image{64ULL * 1024ULL * 1024ULL};
    std::size_t maximum_chunks_per_image{4096};
    std::size_t maximum_resources{1024};
    std::uint64_t maximum_total_encoded_bytes{64ULL * 1024ULL * 1024ULL};
    std::uint64_t maximum_total_decoded_bytes{256ULL * 1024ULL * 1024ULL};
};

enum class ImageResourceError : std::uint8_t {
    none,
    empty_input,
    encoded_limit_exceeded,
    invalid_signature,
    truncated_chunk,
    invalid_chunk_type,
    chunk_limit_exceeded,
    crc_mismatch,
    ihdr_not_first,
    duplicate_ihdr,
    invalid_ihdr,
    dimension_limit_exceeded,
    unsupported_color_format,
    unsupported_color_profile,
    invalid_palette,
    invalid_transparency,
    invalid_color_metadata,
    noncontiguous_idat,
    missing_idat,
    missing_iend,
    trailing_data,
    unknown_critical_chunk,
    resource_count_exceeded,
    registry_encoded_limit_exceeded,
    registry_decoded_limit_exceeded,
    stale_image_id,
    allocation_failed,
};

[[nodiscard]] std::string_view image_resource_error_name(
    ImageResourceError error) noexcept;

struct PngMetadata final {
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint8_t bit_depth{};
    PngColorType color_type{PngColorType::truecolor_alpha};
    bool interlaced{};
    std::uint64_t source_row_bytes{};
    std::uint64_t decoded_byte_count{};
};

struct PngValidationResult final {
    ImageResourceError error{ImageResourceError::none};
    PngMetadata metadata{};

    [[nodiscard]] explicit operator bool() const noexcept {
        return error == ImageResourceError::none;
    }
};

[[nodiscard]] PngValidationResult validate_png(
    std::span<const std::byte> encoded,
    const ImageRegistryLimits& limits = {}) noexcept;

struct ImageLoadResult final {
    ImageId image{};
    ImageResourceError error{ImageResourceError::none};

    [[nodiscard]] explicit operator bool() const noexcept {
        return error == ImageResourceError::none && image.value != 0;
    }
};

struct ImageResourceView final {
    ImageId image{};
    ImageResourceEncoding encoding{ImageResourceEncoding::png};
    PngMetadata metadata{};
    std::span<const std::byte> encoded{};
    std::uint64_t row_bytes{};
    std::uint64_t content_hash{};
};

struct ImageRegistrySnapshot final {
    std::uint64_t revision{};
    std::uint64_t resource_count{};
    std::uint64_t encoded_bytes{};
    std::uint64_t decoded_bytes{};
};

// Renderer-neutral ownership for validated PNG bytes. ImageId values are
// generational: successful replacement and slot reuse invalidate older IDs.
// Views remain valid only until the next registry mutation.
class ImageRegistry final {
public:
    explicit ImageRegistry(ImageRegistryLimits limits = {});
    ~ImageRegistry();
    ImageRegistry(ImageRegistry&&) noexcept;
    ImageRegistry& operator=(ImageRegistry&&) noexcept;
    ImageRegistry(const ImageRegistry&) = delete;
    ImageRegistry& operator=(const ImageRegistry&) = delete;

    [[nodiscard]] ImageLoadResult load_png(std::span<const std::byte> encoded);
    [[nodiscard]] ImageLoadResult load_bgra32_premultiplied(
        std::uint32_t width,
        std::uint32_t height,
        std::uint64_t row_bytes,
        std::span<const std::byte> pixels);
    [[nodiscard]] ImageLoadResult replace_png(ImageId image,
                                               std::span<const std::byte> encoded);
    [[nodiscard]] ImageLoadResult replace_bgra32_premultiplied(
        ImageId image,
        std::uint32_t width,
        std::uint32_t height,
        std::uint64_t row_bytes,
        std::span<const std::byte> pixels);
    [[nodiscard]] bool remove(ImageId image) noexcept;
    void clear() noexcept;

    [[nodiscard]] std::optional<ImageResourceView> find(ImageId image) const noexcept;
    [[nodiscard]] std::vector<ImageId> image_ids() const;
    [[nodiscard]] ImageRegistrySnapshot snapshot() const noexcept;
    [[nodiscard]] const ImageRegistryLimits& limits() const noexcept;

private:
    struct Slot;

    [[nodiscard]] ImageLoadResult store_new(ImageResourceEncoding encoding,
                                             std::vector<std::byte> encoded,
                                             PngMetadata metadata,
                                             std::uint64_t row_bytes,
                                             std::uint64_t content_hash);
    [[nodiscard]] ImageResourceError registry_quota_error(
        std::uint64_t encoded_bytes,
        std::uint64_t decoded_bytes,
        std::uint64_t replaced_encoded = 0,
        std::uint64_t replaced_decoded = 0) const noexcept;

    ImageRegistryLimits limits_;
    std::vector<Slot> slots_;
    std::uint64_t revision_{};
    std::uint64_t resource_count_{};
    std::uint64_t encoded_bytes_{};
    std::uint64_t decoded_bytes_{};
};

} // namespace gui_forms
