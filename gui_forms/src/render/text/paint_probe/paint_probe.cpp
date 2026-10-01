#include "paint_probe.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <cmath>
#include <limits>
#include <new>
#include <stdexcept>
#include <utility>

namespace gui_forms::render::paint_probe {
namespace {
using namespace worker_probe;

struct Library final {
    FT_Library value{};
    Library() {
        const FT_Error error = FT_Init_FreeType(&value);
        if (error != 0) { throw std::runtime_error("Paint FreeType initialization"); }
    }
    ~Library() { if (value != nullptr) { FT_Done_FreeType(value); } }
    Library(const Library&) = delete;
    Library& operator=(const Library&) = delete;
};
struct Face final {
    FT_Face value{};
    Face() = default;
    ~Face() { if (value != nullptr) { FT_Done_Face(value); } }
    Face(const Face&) = delete;
    Face& operator=(const Face&) = delete;
};

Status validate_font_table(const Prepared& prepared, const FontSet& pinned) {
    if (!prepared.fonts) { return Status::missing_font; }
    const FontSet& supplied = *prepared.fonts;
    if (supplied.identity != pinned.identity || supplied.generation != pinned.generation ||
        prepared.identity.font_set != pinned.identity ||
        prepared.identity.font_generation != pinned.generation) { return Status::incompatible_face; }
    const std::size_t face_count = pinned.faces.size();
    for (std::size_t index = 0; index < face_count; ++index) {
        const FontBytes& expected = pinned.faces[index];
        const FontBytes& actual = supplied.faces[index];
        if (!actual.encoded || !expected.encoded || (*actual.encoded).empty()) { return Status::missing_font; }
        if (actual.role != expected.role || actual.weight != expected.weight ||
            actual.italic != expected.italic || actual.face_index != expected.face_index ||
            *actual.encoded != *expected.encoded) { return Status::incompatible_face; }
        if (!prepared.face_ids[index].valid()) { return Status::incompatible_face; }
        for (std::size_t prior = 0; prior < index; ++prior) {
            if (prepared.face_ids[prior] == prepared.face_ids[index]) { return Status::incompatible_face; }
        }
    }
    return Status::success;
}

std::size_t find_face(const Prepared& prepared, FontFaceId id) noexcept {
    const std::size_t count = prepared.face_ids.size();
    for (std::size_t index = 0; index < count; ++index) {
        if (prepared.face_ids[index] == id) { return index; }
    }
    return count;
}

Status rasterize(const Prepared& prepared, std::vector<std::uint8_t>& pixels) {
    const FontSet& fonts = *prepared.fonts;
    Library library{};
    // Faces are destroyed before their library, on this paint executor.
    std::array<Face, 4> faces{};
    const std::size_t face_count = faces.size();
    const double bounded_size = std::clamp(prepared.identity.font.size, 1.0, 4096.0);
    const long long rounded_size = std::llround(bounded_size * 64.0);
    const FT_F26Dot6 size = static_cast<FT_F26Dot6>(rounded_size);
    for (std::size_t index = 0; index < face_count; ++index) {
        const FontBytes& source = fonts.faces[index];
        const std::vector<std::byte>& bytes = *source.encoded;
        if (bytes.size() > 64U * 1024U * 1024U || source.face_index > 255U) { return Status::incompatible_face; }
        const FT_Long length = static_cast<FT_Long>(bytes.size());
        const FT_Long face_index = static_cast<FT_Long>(source.face_index);
        const FT_Byte* data = reinterpret_cast<const FT_Byte*>(bytes.data());
        const FT_Error opened = FT_New_Memory_Face(library.value, data, length, face_index, &faces[index].value);
        if (opened != 0 || faces[index].value == nullptr) { return Status::native_failure; }
        FT_Face face = faces[index].value;
        const FT_Error charmap = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
        const FT_Error sized = FT_Set_Char_Size(face, 0, size, 72U, 72U);
        if (charmap != 0 || sized != 0 || (*face).num_glyphs <= 0) { return Status::native_failure; }
    }
    // Validate every run and glyph before producing any staged pixel.
    const std::size_t source_bytes = prepared.display_utf8.size();
    for (const text::ShapedFontRun& run : prepared.geometry.runs) {
        const std::size_t index = find_face(prepared, run.face);
        if (index == face_count) { return Status::incompatible_face; }
        FT_Face face = faces[index].value;
        if (run.source_range.start.value() > run.source_range.end.value() ||
            run.source_range.end.value() > source_bytes) { return Status::invalid_geometry; }
        const std::uint64_t glyph_limit = static_cast<std::uint64_t>((*face).num_glyphs);
        for (const text::ShapedGlyph& glyph : run.glyphs) {
            if (glyph.glyph.value == 0 || glyph.glyph.value >= glyph_limit ||
                glyph.glyph.value > std::numeric_limits<FT_UInt>::max()) {
                return Status::incompatible_glyph;
            }
            if (!std::isfinite(glyph.x) || !std::isfinite(glyph.y) ||
                !std::isfinite(glyph.advance_x) || !std::isfinite(glyph.advance_y) ||
                std::abs(glyph.x) > 1'000'000.0F || std::abs(glyph.y) > 1'000'000.0F ||
                glyph.cluster.value() < run.source_range.start.value() ||
                glyph.cluster.value() >= run.source_range.end.value()) { return Status::invalid_geometry; }
        }
    }
    pixels.assign(Painter::width * Painter::height, 0);
    for (const text::ShapedFontRun& run : prepared.geometry.runs) {
        const std::size_t index = find_face(prepared, run.face);
        FT_Face face = faces[index].value;
        for (const text::ShapedGlyph& glyph : run.glyphs) {
            const double x = 8.0 + static_cast<double>(glyph.x);
            const double y = 64.0 + static_cast<double>(glyph.y);
            const double integral_x = std::floor(x);
            const double integral_y = std::floor(y);
            const long left_origin = static_cast<long>(integral_x);
            const long top_origin = static_cast<long>(integral_y);
            const long fractional_x = std::lround((x - integral_x) * 64.0);
            const long fractional_y = std::lround(-(y - integral_y) * 64.0);
            FT_Vector delta{fractional_x, fractional_y};
            FT_Set_Transform(face, nullptr, &delta);
            const FT_UInt id = static_cast<FT_UInt>(glyph.glyph.value);
            const FT_Error loaded = FT_Load_Glyph(face, id, FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP);
            if (loaded != 0) { return Status::native_failure; }
            FT_GlyphSlot slot = (*face).glyph;
            if (slot == nullptr || (*slot).format != FT_GLYPH_FORMAT_OUTLINE) { return Status::unsupported_raster; }
            const FT_Error rendered = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);
            if (rendered != 0) { return Status::native_failure; }
            const FT_Bitmap& bitmap = (*slot).bitmap;
            if (bitmap.width == 0 || bitmap.rows == 0) { continue; }
            if (bitmap.pixel_mode != FT_PIXEL_MODE_GRAY || bitmap.num_grays != 256 ||
                bitmap.pitch <= 0 || bitmap.buffer == nullptr ||
                bitmap.width > static_cast<unsigned>(bitmap.pitch)) { return Status::unsupported_raster; }
            // This fixture accepts only bounded positive-pitch gray bitmaps.
            // Native allocation already happened inside FT and remains opaque.
            constexpr std::size_t bitmap_byte_limit = 64U * 1024U * 1024U;
            if (bitmap.width > 8192U || bitmap.rows > 8192U || bitmap.pitch > 16384 ||
                (*slot).bitmap_left < -1'000'000 || (*slot).bitmap_left > 1'000'000 ||
                (*slot).bitmap_top < -1'000'000 || (*slot).bitmap_top > 1'000'000) {
                return Status::unsupported_raster;
            }
            const std::size_t pitch = static_cast<std::size_t>(bitmap.pitch);
            const std::size_t rows = static_cast<std::size_t>(bitmap.rows);
            if (rows > bitmap_byte_limit / pitch) { return Status::unsupported_raster; }
            const long bitmap_left = left_origin + (*slot).bitmap_left;
            const long bitmap_top = top_origin - (*slot).bitmap_top;
            for (unsigned row = 0; row < bitmap.rows; ++row) {
                const long target_y = bitmap_top + static_cast<long>(row);
                if (target_y < 0 || target_y >= static_cast<long>(Painter::height)) { continue; }
                const std::size_t source_row = static_cast<std::size_t>(row) * pitch;
                const std::size_t target_row = static_cast<std::size_t>(target_y) * Painter::width;
                for (unsigned column = 0; column < bitmap.width; ++column) {
                    const long target_x = bitmap_left + static_cast<long>(column);
                    if (target_x < 0 || target_x >= static_cast<long>(Painter::width)) { continue; }
                    const std::size_t destination = target_row + static_cast<std::size_t>(target_x);
                    const unsigned coverage = bitmap.buffer[source_row + column];
                    const unsigned prior = pixels[destination];
                    const unsigned composite = coverage + prior * (255U - coverage) / 255U;
                    pixels[destination] = static_cast<std::uint8_t>(composite);
                }
            }
        }
    }
    return Status::success;
}
} // namespace

Painter::Painter(std::shared_ptr<const worker_probe::FontSet> pinned_fonts)
    : pinned_fonts_(std::move(pinned_fonts)), executor_(std::this_thread::get_id()) {
    if (!pinned_fonts_) { throw std::invalid_argument("Paint requires pinned fonts"); }
}

Status Painter::paint(const worker_probe::Prepared& prepared, const worker_probe::Identity& expected,
                      std::uint64_t authority_epoch) {
    if (std::this_thread::get_id() != executor_) { return Status::stale; }
    if (!worker_probe::same_identity(prepared.identity, expected) ||
        authority_epoch == 0 || prepared.authority_epoch != authority_epoch) { return Status::stale; }
    if (!valid_font_spec(expected.font) || expected.scale != 1.0 || expected.wrap_width != 0.0 ||
        expected.tab_columns != 4 || prepared.display_utf8.size() > 16'384 ||
        !validate_utf8(prepared.display_utf8).valid()) { return Status::invalid_geometry; }
    const Status fonts = validate_font_table(prepared, *pinned_fonts_);
    if (fonts != Status::success) { return fonts; }
    if (prepared.geometry.missing_primary_face || prepared.geometry.missing_clusters != 0) {
        return Status::missing_font;
    }
    const text::ShapedText& geometry = prepared.geometry;
    if (!std::isfinite(geometry.width) || !std::isfinite(geometry.height) ||
        !std::isfinite(geometry.ascent) || !std::isfinite(geometry.descent) ||
        geometry.width < 0 || geometry.height < 0 || geometry.ascent < 0 || geometry.descent < 0 ||
        (!prepared.display_utf8.empty() && geometry.runs.empty())) { return Status::invalid_geometry; }
    if (geometry.runs.size() > 16'384) { return Status::invalid_geometry; }
    std::size_t glyph_count = 0;
    for (const text::ShapedFontRun& run : geometry.runs) {
        const std::size_t count = run.glyphs.size();
        if (count > 65'536 - glyph_count) { return Status::invalid_geometry; }
        glyph_count += count;
    }
    try {
        std::vector<std::uint8_t> staged{};
        const Status painted = rasterize(prepared, staged);
        if (painted != Status::success) { return painted; }
        pixels_ = std::move(staged);
        painted_identity_ = prepared.identity;
        return Status::success;
    } catch (const std::bad_alloc&) {
        return Status::resource_failure;
    } catch (const std::runtime_error&) {
        return Status::native_failure;
    }
}
const std::vector<std::uint8_t>& Painter::pixels() const noexcept { return pixels_; }
const worker_probe::Identity& Painter::painted_identity() const noexcept { return painted_identity_; }
} // namespace gui_forms::render::paint_probe
