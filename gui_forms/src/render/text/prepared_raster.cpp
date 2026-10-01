#include "../../core/text/prepared/prepared_storage.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace gui_forms {
namespace {

struct RasterLibrary final {
    FT_Library value{};
    RasterLibrary() {
        const FT_Error status = FT_Init_FreeType(&value);
        if (status != 0) throw std::runtime_error("Prepared raster initialization");
    }
    ~RasterLibrary() { if (value != nullptr) FT_Done_FreeType(value); }
    RasterLibrary(const RasterLibrary&) = delete;
    RasterLibrary& operator=(const RasterLibrary&) = delete;
};

struct RasterFace final {
    FT_Face value{};
    RasterFace() = default;
    ~RasterFace() { if (value != nullptr) FT_Done_Face(value); }
    RasterFace(const RasterFace&) = delete;
    RasterFace& operator=(const RasterFace&) = delete;
};

struct InkBounds final {
    std::int64_t left{};
    std::int64_t top{};
    std::int64_t right{};
    std::int64_t bottom{};
    bool present{};
};

struct GlyphBitmap final {
    // Borrow ends on the next load on this face or face destruction.
    const FT_Bitmap* bitmap{};
    std::int64_t left{};
    std::int64_t top{};
};

std::size_t raster_face_index(const detail::PreparedTextStorage& storage, const FontFaceId id) noexcept {
    const std::size_t count = (*storage.fonts).face_count;
    for (std::size_t index = 0; index < count; ++index) {
        if (storage.face_ids[index] == id) return index;
    }
    return count;
}

PreparedTextStatus load_bitmap(const FT_Face face, const render::text::ShapedGlyph& glyph, GlyphBitmap& output) {
    const double x = static_cast<double>(glyph.x);
    const double y = static_cast<double>(glyph.y);
    const double integral_x = std::floor(x);
    const double integral_y = std::floor(y);
    const long fractional_x = std::lround((x - integral_x) * 64.0);
    const long fractional_y = std::lround(-(y - integral_y) * 64.0);
    FT_Vector delta{fractional_x, fractional_y};
    FT_Set_Transform(face, nullptr, &delta);
    const FT_UInt id = static_cast<FT_UInt>(glyph.glyph.value);
    const FT_Error loaded = FT_Load_Glyph(face, id, FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP);
    if (loaded != 0) return PreparedTextStatus::native_failure;
    FT_GlyphSlot slot = (*face).glyph;
    if (slot == nullptr || (*slot).format != FT_GLYPH_FORMAT_OUTLINE) return PreparedTextStatus::unsupported_profile;
    const FT_Error rendered = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);
    if (rendered != 0) return PreparedTextStatus::native_failure;
    const FT_Bitmap& bitmap = (*slot).bitmap;
    if (bitmap.width == 0 || bitmap.rows == 0) { output = {}; return PreparedTextStatus::success; }
    if (bitmap.pixel_mode != FT_PIXEL_MODE_GRAY || bitmap.num_grays != 256 || bitmap.pitch <= 0 ||
        bitmap.buffer == nullptr || bitmap.width > static_cast<unsigned>(bitmap.pitch) ||
        bitmap.width > 8192U || bitmap.rows > 8192U || bitmap.pitch > 16384 ||
        (*slot).bitmap_left < -1'000'000 || (*slot).bitmap_left > 1'000'000 ||
        (*slot).bitmap_top < -1'000'000 || (*slot).bitmap_top > 1'000'000) return PreparedTextStatus::unsupported_profile;
    // FT allocation has already occurred. This is an indexing guard, not a
    // pre-allocation bound on native memory. All arithmetic below is widened.
    const std::size_t pitch = static_cast<std::size_t>(bitmap.pitch);
    if (bitmap.rows > (64U * 1024U * 1024U) / pitch) return PreparedTextStatus::unsupported_profile;
    output.bitmap = &bitmap;
    output.left = static_cast<std::int64_t>(integral_x) + (*slot).bitmap_left;
    output.top = static_cast<std::int64_t>(integral_y) - (*slot).bitmap_top;
    return PreparedTextStatus::success;
}

PreparedTextStatus open_faces(const detail::PreparedTextStorage& storage, RasterLibrary& library,
    std::array<RasterFace, PreparedTextLimits::font_faces>& faces) {
    const detail::PreparedFontBank& bank = *storage.fonts;
    for (std::size_t index = 0; index < bank.face_count; ++index) {
        if (!storage.face_ids[index].valid()) return PreparedTextStatus::incompatible_font;
        for (std::size_t prior = 0; prior < index; ++prior) {
            if (storage.face_ids[prior] == storage.face_ids[index]) return PreparedTextStatus::incompatible_font;
        }
        const detail::PreparedFontFace& source = bank.faces[index];
        const FT_Byte* bytes = reinterpret_cast<const FT_Byte*>(source.bytes.get());
        const FT_Long length = static_cast<FT_Long>(source.size);
        const FT_Long face_index = static_cast<FT_Long>(source.index);
        const FT_Error opened = FT_New_Memory_Face(library.value, bytes, length, face_index, &faces[index].value);
        if (opened != 0) return PreparedTextStatus::incompatible_font;
        FT_Face face = faces[index].value;
        if (face == nullptr || !FT_IS_SCALABLE(face) || (*face).num_glyphs <= 0) return PreparedTextStatus::incompatible_font;
        const FT_Error charmap = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
        const FT_F26Dot6 size = static_cast<FT_F26Dot6>(storage.metrics.device_size_26_6);
        const FT_Error sized = FT_Set_Char_Size(face, 0, size, 72U, 72U);
        if (charmap != 0 || sized != 0) return PreparedTextStatus::incompatible_font;
    }
    return PreparedTextStatus::success;
}

PreparedTextStatus validate_geometry(const detail::PreparedTextStorage& storage,
    const std::array<RasterFace, PreparedTextLimits::font_faces>& faces) {
    const render::text::BoundedShapedText& geometry = *storage.geometry;
    if (geometry.run_count > geometry.run_capacity || geometry.glyph_count > geometry.glyph_capacity ||
        geometry.run_count > 16'384 || geometry.glyph_count > 65'536 ||
        (geometry.run_count != 0 && !geometry.runs) || (geometry.glyph_count != 0 && !geometry.glyphs) ||
        !std::isfinite(geometry.width) || !std::isfinite(geometry.height) || !std::isfinite(geometry.ascent) ||
        !std::isfinite(geometry.descent) || geometry.width < 0 || geometry.height < 0 ||
        geometry.ascent < 0 || geometry.descent < 0) return PreparedTextStatus::invalid_geometry;
    std::size_t consumed = 0;
    for (std::size_t run_index = 0; run_index < geometry.run_count; ++run_index) {
        const render::text::BoundedFontRun& run = geometry.runs[run_index];
        const std::size_t face_index = raster_face_index(storage, run.face);
        if (face_index == (*storage.fonts).face_count) return PreparedTextStatus::incompatible_font;
        if (run.glyph_begin != consumed || run.glyph_count > geometry.glyph_count - consumed ||
            run.source_range.start.value() >= run.source_range.end.value() ||
            run.source_range.end.value() > (*storage.input).text_bytes) return PreparedTextStatus::invalid_geometry;
        const std::uint64_t glyph_limit = static_cast<std::uint64_t>((*faces[face_index].value).num_glyphs);
        for (std::size_t offset = 0; offset < run.glyph_count; ++offset) {
            const render::text::ShapedGlyph& glyph = geometry.glyphs[consumed + offset];
            if (glyph.glyph.value == 0 || glyph.glyph.value >= glyph_limit ||
                glyph.glyph.value > std::numeric_limits<FT_UInt>::max()) return PreparedTextStatus::incompatible_font;
            if (!std::isfinite(glyph.x) || !std::isfinite(glyph.y) || !std::isfinite(glyph.advance_x) ||
                !std::isfinite(glyph.advance_y) || std::abs(glyph.x) > 1'000'000.0F || std::abs(glyph.y) > 1'000'000.0F ||
                glyph.cluster.value() < run.source_range.start.value() ||
                glyph.cluster.value() >= run.source_range.end.value()) return PreparedTextStatus::invalid_geometry;
        }
        consumed += run.glyph_count;
    }
    if (consumed != geometry.glyph_count) return PreparedTextStatus::invalid_geometry;
    return PreparedTextStatus::success;
}

PreparedTextStatus measure_ink(const detail::PreparedTextStorage& storage,
    std::array<RasterFace, PreparedTextLimits::font_faces>& faces, InkBounds& bounds) {
    const render::text::BoundedShapedText& geometry = *storage.geometry;
    for (std::size_t run_index = 0; run_index < geometry.run_count; ++run_index) {
        const render::text::BoundedFontRun& run = geometry.runs[run_index];
        const std::size_t face_index = raster_face_index(storage, run.face);
        FT_Face face = faces[face_index].value;
        for (std::size_t offset = 0; offset < run.glyph_count; ++offset) {
            const render::text::ShapedGlyph& glyph = geometry.glyphs[run.glyph_begin + offset];
            GlyphBitmap bitmap{};
            const PreparedTextStatus loaded = load_bitmap(face, glyph, bitmap);
            if (loaded != PreparedTextStatus::success) return loaded;
            if (bitmap.bitmap == nullptr) continue;
            const std::int64_t right = bitmap.left + (*bitmap.bitmap).width;
            const std::int64_t bottom = bitmap.top + (*bitmap.bitmap).rows;
            if (!bounds.present) bounds = {bitmap.left, bitmap.top, right, bottom, true};
            else {
                bounds.left = std::min(bounds.left, bitmap.left);
                bounds.top = std::min(bounds.top, bitmap.top);
                bounds.right = std::max(bounds.right, right);
                bounds.bottom = std::max(bounds.bottom, bottom);
            }
            if (bounds.right - bounds.left > static_cast<std::int64_t>(PreparedTextLimits::mask_dimension) ||
                bounds.bottom - bounds.top > static_cast<std::int64_t>(PreparedTextLimits::mask_dimension)) {
                return PreparedTextStatus::budget_exceeded;
            }
        }
    }
    return PreparedTextStatus::success;
}

PreparedTextStatus fill_mask(const detail::PreparedTextStorage& storage,
    std::array<RasterFace, PreparedTextLimits::font_faces>& faces, detail::PreparedMaskStorage& mask) {
    const render::text::BoundedShapedText& geometry = *storage.geometry;
    for (std::size_t run_index = 0; run_index < geometry.run_count; ++run_index) {
        const render::text::BoundedFontRun& run = geometry.runs[run_index];
        const std::size_t face_index = raster_face_index(storage, run.face);
        FT_Face face = faces[face_index].value;
        for (std::size_t offset = 0; offset < run.glyph_count; ++offset) {
            GlyphBitmap placement{};
            const render::text::ShapedGlyph& glyph = geometry.glyphs[run.glyph_begin + offset];
            const PreparedTextStatus loaded = load_bitmap(face, glyph, placement);
            if (loaded != PreparedTextStatus::success) return loaded;
            if (placement.bitmap == nullptr) continue;
            const FT_Bitmap& bitmap = *placement.bitmap;
            const std::int64_t left = placement.left - mask.left;
            const std::int64_t top = placement.top - mask.top;
            if (left < 0 || top < 0 || left + bitmap.width > mask.width || top + bitmap.rows > mask.height) {
                return PreparedTextStatus::invalid_geometry;
            }
            const std::size_t pitch = static_cast<std::size_t>(bitmap.pitch);
            const std::size_t target_left = static_cast<std::size_t>(left);
            const std::size_t target_top = static_cast<std::size_t>(top);
            for (std::size_t row = 0; row < bitmap.rows; ++row) {
                const std::size_t source_row = row * pitch;
                const std::size_t destination_row = (target_top + row) * mask.width + target_left;
                for (std::size_t column = 0; column < bitmap.width; ++column) {
                    const unsigned coverage = bitmap.buffer[source_row + column];
                    const unsigned previous = mask.pixels[destination_row + column];
                    const unsigned combined = coverage + previous * (255U - coverage) / 255U;
                    mask.pixels[destination_row + column] = static_cast<std::uint8_t>(combined);
                }
            }
        }
    }
    return PreparedTextStatus::success;
}
} // namespace

PreparedTextStatus detail::validate_prepared_fonts(const PreparedFontBank& bank) {
    try {
        RasterLibrary library{};
        std::array<RasterFace, PreparedTextLimits::font_faces> faces{};
        for (std::size_t index = 0; index < bank.face_count; ++index) {
            const PreparedFontFace& source = bank.faces[index];
            const FT_Byte* bytes = reinterpret_cast<const FT_Byte*>(source.bytes.get());
            const FT_Long length = static_cast<FT_Long>(source.size);
            const FT_Long face_index = static_cast<FT_Long>(source.index);
            const FT_Error opened = FT_New_Memory_Face(library.value, bytes, length, face_index, &faces[index].value);
            if (opened != 0) return PreparedTextStatus::incompatible_font;
            FT_Face face = faces[index].value;
            if (face == nullptr || !FT_IS_SCALABLE(face) || (*face).num_glyphs <= 0 || (*face).num_glyphs > 1'000'000) {
                return PreparedTextStatus::incompatible_font;
            }
            const FT_Error charmap = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
            if (charmap != 0) return PreparedTextStatus::incompatible_font;
        }
        return PreparedTextStatus::success;
    } catch (const std::bad_alloc&) { return PreparedTextStatus::resource_failure; }
    catch (const std::runtime_error&) { return PreparedTextStatus::native_failure; }
}

PreparedTextStatus rasterize_prepared_text(const PreparedTextLayout& layout, const LayoutAuthority expected, GrayTextMask& output) {
    const std::shared_ptr<const detail::PreparedTextStorage> storage = detail::PreparedTextAccess::layout(layout);
    if (!storage) return PreparedTextStatus::invalid_input;
    if (!detail::prepared_authority_current(*storage, expected)) return PreparedTextStatus::stale;
    try {
        RasterLibrary library{};
        // Faces die before library and before the retained encoded-byte owner.
        std::array<RasterFace, PreparedTextLimits::font_faces> faces{};
        const PreparedTextStatus opened = open_faces(*storage, library, faces);
        if (opened != PreparedTextStatus::success) return opened;
        const PreparedTextStatus validated = validate_geometry(*storage, faces);
        if (validated != PreparedTextStatus::success) return validated;
        InkBounds bounds{};
        const PreparedTextStatus measured = measure_ink(*storage, faces, bounds);
        if (measured != PreparedTextStatus::success) return measured;
        const std::size_t width = static_cast<std::size_t>(bounds.right - bounds.left);
        const std::size_t height = static_cast<std::size_t>(bounds.bottom - bounds.top);
        if (width != 0 && height > PreparedTextLimits::mask_bytes / width) return PreparedTextStatus::budget_exceeded;
        const std::size_t bytes = width * height;
        detail::PreparedReservation reservation{};
        const PreparedTextStatus admitted = reservation.acquire((*storage).ledger, detail::PreparedResource::mask, bytes);
        if (admitted != PreparedTextStatus::success) return admitted;
        std::unique_ptr<detail::PreparedMaskStorage> candidate = std::make_unique<detail::PreparedMaskStorage>();
        detail::PreparedMaskStorage& mask = *candidate;
        mask.reservation = std::move(reservation);
        mask.metrics = (*storage).metrics;
        mask.width = static_cast<std::uint32_t>(width);
        mask.height = static_cast<std::uint32_t>(height);
        mask.left = static_cast<std::int32_t>(bounds.left);
        mask.top = static_cast<std::int32_t>(bounds.top);
        mask.pixel_count = bytes;
        if (bytes != 0) mask.pixels = std::make_unique<std::uint8_t[]>(bytes);
        const PreparedTextStatus painted = fill_mask(*storage, faces, mask);
        if (painted != PreparedTextStatus::success) return painted;
        detail::PreparedAuthorityState& authority = *(*storage).authority_state;
        std::lock_guard<std::mutex> publication_lock(authority.mutex);
        if (!detail::prepared_authority_current_locked(*storage, expected)) return PreparedTextStatus::stale;
        detail::PreparedTextAccess::mask(output) = std::move(candidate);
        return PreparedTextStatus::success;
    } catch (const std::bad_alloc&) { return PreparedTextStatus::resource_failure; }
    catch (const std::runtime_error&) { return PreparedTextStatus::native_failure; }
}
} // namespace gui_forms
