#pragma once

#include "gui_forms/resources/types/image_resource_types.hpp"

#include <optional>
#include <span>
#include <vector>

namespace gui_forms {

// Renderer-neutral ownership for validated PNG and premultiplied BGRA bytes.
// ImageId values are generational; views expire at the next registry mutation.
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
        std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes,
        std::span<const std::byte> pixels);
    [[nodiscard]] ImageLoadResult replace_png(
        ImageId image, std::span<const std::byte> encoded);
    [[nodiscard]] ImageLoadResult replace_bgra32_premultiplied(
        ImageId image, std::uint32_t width, std::uint32_t height,
        std::uint64_t row_bytes, std::span<const std::byte> pixels);
    [[nodiscard]] ImageLoadResult update_bgra32_premultiplied(
        ImageId image, std::uint32_t width, std::uint32_t height,
        std::uint64_t row_bytes, std::span<const std::byte> pixels);
    [[nodiscard]] ImageLoadResult patch_bgra32_premultiplied(
        ImageId image, std::uint32_t x, std::uint32_t y,
        std::uint32_t width, std::uint32_t height,
        std::uint64_t source_row_bytes, std::span<const std::byte> pixels);
    [[nodiscard]] bool remove(ImageId image) noexcept;
    void clear() noexcept;

    [[nodiscard]] std::optional<ImageResourceView> find(
        ImageId image) const noexcept;
    [[nodiscard]] std::vector<ImageId> image_ids() const;
    [[nodiscard]] ImageRegistrySnapshot snapshot() const noexcept;
    [[nodiscard]] const ImageRegistryLimits& limits() const noexcept;

private:
    struct Slot;
    [[nodiscard]] ImageLoadResult store_new(
        ImageResourceEncoding encoding, std::vector<std::byte> encoded,
        PngMetadata metadata, std::uint64_t row_bytes,
        std::uint64_t content_hash);
    [[nodiscard]] ImageResourceError registry_quota_error(
        std::uint64_t encoded_bytes, std::uint64_t decoded_bytes,
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
