#include "text_mask_native.hpp"
#include <algorithm>
#include <cmath>

namespace gui_forms::detail::mask_native {
namespace {
struct BitmapBorrow final {
    const FT_Bitmap* bitmap{};
    std::int64_t left{0};
    std::int64_t top{0};
};

void blend_gray(const FT_Bitmap& bitmap, const std::size_t left, const std::size_t top, TextMaskStorage& target);
void blend_mono(const FT_Bitmap& bitmap, const std::size_t left, const std::size_t top, TextMaskStorage& target);

void validate_gray_bitmap(const FT_Bitmap& bitmap, const std::size_t pitch) {
    if (bitmap.pixel_mode != FT_PIXEL_MODE_GRAY || bitmap.num_grays != 256 || bitmap.width > pitch) {
        throw NativeFailure(TextMaskStatus::unsupported_profile);
    }
}
void validate_mono_bitmap(const FT_Bitmap& bitmap, const std::size_t pitch) {
    if (bitmap.pixel_mode != FT_PIXEL_MODE_MONO || (bitmap.width + 7U) / 8U > pitch) {
        throw NativeFailure(TextMaskStatus::unsupported_profile);
    }
}

struct RasterConfiguration final {
    double scale{1.0};
    FT_Int32 load_flags{FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT};
    FT_Render_Mode render_mode{FT_RENDER_MODE_NORMAL};
    void (*validate_bitmap)(const FT_Bitmap&, std::size_t){validate_gray_bitmap};
    void (*blend_bitmap)(const FT_Bitmap&, std::size_t, std::size_t, TextMaskStorage&){blend_gray};
};

RasterConfiguration make_raster_configuration(const MaskOptions& options) {
    RasterConfiguration result{};
    result.scale = options.scale;
    if (options.raster == TextMaskRaster::true_mono) {
        result.load_flags = FT_LOAD_NO_BITMAP | FT_LOAD_TARGET_MONO | FT_LOAD_MONOCHROME;
        result.render_mode = FT_RENDER_MODE_MONO;
        result.validate_bitmap = validate_mono_bitmap;
        result.blend_bitmap = blend_mono;
    }
    return result;
}

BitmapBorrow load_bitmap(NativeFonts& fonts, const RasterConfiguration& configuration, const NativeGlyph& glyph) {
    if (glyph.face >= fonts.count) throw NativeFailure(TextMaskStatus::native_failure);
    const FT_Face face = fonts.faces[glyph.face].value;
    if (glyph.glyph == 0 || glyph.glyph >= static_cast<std::uint64_t>((*face).num_glyphs)) {
        throw NativeFailure(TextMaskStatus::missing_font_coverage);
    }
    const double device_x = glyph.x * configuration.scale;
    const double device_y = glyph.y * configuration.scale;
    if (!std::isfinite(device_x) || !std::isfinite(device_y) || std::abs(device_x) > 16'000'000.0 || std::abs(device_y) > 16'000'000.0) {
        throw MaskLimitFailure(TextMaskLimit::mask_dimension);
    }
    const std::int64_t origin_x = std::llround(device_x);
    const std::int64_t origin_y = std::llround(device_y);
    FT_Set_Transform(face, nullptr, nullptr);
    const FT_Error loaded = FT_Load_Glyph(face, glyph.glyph, configuration.load_flags);
    if (loaded != 0) throw NativeFailure(TextMaskStatus::native_failure);
    const FT_GlyphSlot slot = (*face).glyph;
    if (slot == nullptr || (*slot).format != FT_GLYPH_FORMAT_OUTLINE) throw NativeFailure(TextMaskStatus::unsupported_profile);
    const FT_Error rendered = FT_Render_Glyph(slot, configuration.render_mode);
    if (rendered != 0) throw NativeFailure(TextMaskStatus::native_failure);
    const FT_Bitmap& bitmap = (*slot).bitmap;
    if (bitmap.width == 0 || bitmap.rows == 0) return {};
    if (bitmap.width > 4096 || bitmap.rows > 4096) throw MaskLimitFailure(TextMaskLimit::mask_dimension);
    if (bitmap.buffer == nullptr || bitmap.pitch <= 0 || bitmap.pitch > 16384) throw NativeFailure(TextMaskStatus::unsupported_profile);
    const std::size_t pitch = static_cast<std::size_t>(bitmap.pitch);
    configuration.validate_bitmap(bitmap, pitch);
    // Font bitmap memory is native/opaque and already allocated here. These
    // are indexing guards, not a claimed pre-allocation bound on FreeType.
    if (bitmap.rows > (64U * 1024U * 1024U) / pitch) throw NativeFailure(TextMaskStatus::unsupported_profile);
    const BitmapBorrow result{&bitmap, origin_x + (*slot).bitmap_left, origin_y - (*slot).bitmap_top};
    return result;
}

void blend_gray(const FT_Bitmap& bitmap, const std::size_t left, const std::size_t top, TextMaskStorage& target) {
    const std::size_t pitch = static_cast<std::size_t>(bitmap.pitch);
    const std::size_t stride = target.metrics.stride_bytes;
    for (std::size_t row = 0; row < bitmap.rows; ++row) {
        const std::size_t source_row = row * pitch;
        const std::size_t target_row = (top + row) * stride + left;
        for (std::size_t column = 0; column < bitmap.width; ++column) {
            const unsigned coverage = bitmap.buffer[source_row + column];
            const unsigned previous = target.pixels[target_row + column];
            const unsigned combined = coverage + previous * (255U - coverage) / 255U;
            target.pixels[target_row + column] = static_cast<std::uint8_t>(combined);
        }
    }
}
void blend_mono(const FT_Bitmap& bitmap, const std::size_t left, const std::size_t top, TextMaskStorage& target) {
    const std::size_t pitch = static_cast<std::size_t>(bitmap.pitch);
    const std::size_t stride = target.metrics.stride_bytes;
    for (std::size_t row = 0; row < bitmap.rows; ++row) {
        const std::size_t source_row = row * pitch;
        const std::size_t target_row = (top + row) * stride + left;
        for (std::size_t column = 0; column < bitmap.width; ++column) {
            const unsigned packed = bitmap.buffer[source_row + column / 8U];
            const unsigned bit = 0x80U >> (column % 8U);
            if ((packed & bit) != 0) target.pixels[target_row + column] = 255;
        }
    }
}
}

NativeInk measure_native_ink(NativeFonts& fonts, const MaskOptions& options, const std::span<const NativeGlyph> glyphs) {
    fonts.set_raster_size(options);
    const RasterConfiguration configuration = make_raster_configuration(options);
    NativeInk bounds{};
    for (std::size_t index = 0; index < glyphs.size(); ++index) {
        const BitmapBorrow placement = load_bitmap(fonts, configuration, glyphs[index]);
        if (placement.bitmap == nullptr) continue;
        const std::int64_t right = placement.left + (*placement.bitmap).width;
        const std::int64_t bottom = placement.top + (*placement.bitmap).rows;
        if (!bounds.present) bounds = {placement.left, placement.top, right, bottom, true};
        else {
            bounds.left = std::min(bounds.left, placement.left);
            bounds.top = std::min(bounds.top, placement.top);
            bounds.right = std::max(bounds.right, right);
            bounds.bottom = std::max(bounds.bottom, bottom);
        }
        if (bounds.right - bounds.left > 4096 || bounds.bottom - bounds.top > 4096) {
            throw MaskLimitFailure(TextMaskLimit::mask_dimension);
        }
    }
    return bounds;
}
void fill_native_mask(NativeFonts& fonts, const MaskOptions& options,
    const std::span<const NativeGlyph> glyphs, TextMaskStorage& storage) {
    const RasterConfiguration configuration = make_raster_configuration(options);
    for (std::size_t index = 0; index < glyphs.size(); ++index) {
        const BitmapBorrow placement = load_bitmap(fonts, configuration, glyphs[index]);
        if (placement.bitmap == nullptr) continue;
        const FT_Bitmap& bitmap = *placement.bitmap;
        const std::int64_t left = placement.left - storage.metrics.ink_left_px;
        const std::int64_t top = placement.top - storage.metrics.ink_top_px;
        if (left < 0 || top < 0 || left + bitmap.width > storage.metrics.width_px || top + bitmap.rows > storage.metrics.height_px) {
            throw NativeFailure(TextMaskStatus::native_failure);
        }
        const std::size_t target_left = static_cast<std::size_t>(left);
        const std::size_t target_top = static_cast<std::size_t>(top);
        configuration.blend_bitmap(bitmap, target_left, target_top, storage);
    }
}
} // namespace gui_forms::detail::mask_native
