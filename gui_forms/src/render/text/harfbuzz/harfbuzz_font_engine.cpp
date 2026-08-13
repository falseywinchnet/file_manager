#include "harfbuzz_font_engine.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <hb-ft.h>
#include <hb.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace gui_forms::render::text {
namespace {

constexpr std::size_t maximum_face_bytes = 64U * 1024U * 1024U;
constexpr std::size_t maximum_faces = 64U;
constexpr FT_Long maximum_glyphs = 1'000'000L;

struct LibraryOwner final {
    FT_Library value{};
    LibraryOwner() {
        if (FT_Init_FreeType(&value) != 0 || value == nullptr) {
            throw std::runtime_error("FreeType initialization failed");
        }
    }
    ~LibraryOwner() {
        if (value != nullptr) FT_Done_FreeType(value);
    }
};

[[nodiscard]] std::vector<char32_t> decode_scalars(std::string_view utf8,
                                                    Utf8Range range) {
    std::vector<char32_t> scalars;
    std::size_t offset = range.start.value();
    while (offset < range.end.value()) {
        const unsigned char first = static_cast<unsigned char>(utf8[offset]);
        char32_t value{};
        std::size_t count{};
        if (first < 0x80U) {
            value = first;
            count = 1U;
        } else if ((first & 0xe0U) == 0xc0U) {
            value = first & 0x1fU;
            count = 2U;
        } else if ((first & 0xf0U) == 0xe0U) {
            value = first & 0x0fU;
            count = 3U;
        } else {
            value = first & 0x07U;
            count = 4U;
        }
        for (std::size_t index = 1U; index < count; ++index) {
            value = (value << 6U) |
                (static_cast<unsigned char>(utf8[offset + index]) & 0x3fU);
        }
        scalars.push_back(value);
        offset += count;
    }
    return scalars;
}

[[nodiscard]] bool ignorable_for_coverage(char32_t scalar) noexcept {
    return scalar == U'\u200c' || scalar == U'\u200d' ||
           (scalar >= U'\ufe00' && scalar <= U'\ufe0f') ||
           (scalar >= U'\U000e0100' && scalar <= U'\U000e01ef');
}

} // namespace

class HarfBuzzFontEngine::Impl final {
public:
    struct Face final {
        FontFaceId id{};
        std::optional<FontRole> role;
        std::uint16_t weight{};
        bool italic{};
        bool fallback{};
        std::string family;
        std::vector<std::byte> encoded;
        FT_Face face{};

        Face(FontFaceId id_value, std::optional<FontRole> role_value,
             std::uint16_t weight_value, bool italic_value,
             bool fallback_value, std::string family_value,
             std::vector<std::byte> bytes, FT_Face face_value)
            : id(id_value), role(role_value), weight(weight_value),
              italic(italic_value), fallback(fallback_value),
              family(std::move(family_value)), encoded(std::move(bytes)),
              face(face_value) {}
        ~Face() {
            if (face != nullptr) FT_Done_Face(face);
        }
        Face(const Face&) = delete;
        Face& operator=(const Face&) = delete;
        Face(Face&& other) noexcept
            : id(other.id), role(other.role), weight(other.weight),
              italic(other.italic), fallback(other.fallback),
              family(std::move(other.family)), encoded(std::move(other.encoded)),
              face(std::exchange(other.face, nullptr)) {}
        Face& operator=(Face&&) = delete;
    };

    struct FacePreference final {
        FontSpec font;

        [[nodiscard]] int tier(const Face& face) const noexcept {
            if (face.role && *face.role == font.role) return 0;
            if (face.role && *face.role == FontRole::content) return 1;
            return 2;
        }

        [[nodiscard]] bool operator()(const Face* left,
                                      const Face* right) const noexcept {
            const int left_tier = tier(*left);
            const int right_tier = tier(*right);
            if (left_tier != right_tier) return left_tier < right_tier;
            const int left_italic = (*left).italic == font.italic ? 0 : 1;
            const int right_italic = (*right).italic == font.italic ? 0 : 1;
            if (left_italic != right_italic) {
                return left_italic < right_italic;
            }
            return std::abs(static_cast<int>((*left).weight) - font.weight) <
                   std::abs(static_cast<int>((*right).weight) - font.weight);
        }
    };

    LibraryOwner library;
    std::vector<Face> faces;
    std::uint64_t next_face_id{1U};

    [[nodiscard]] std::vector<Face*> candidates(FontSpec font) {
        std::vector<Face*> result;
        for (Face& face : faces) {
            // The body face is the house text fallback for control and
            // monospace roles. This keeps Portsmouth as the preferred control
            // voice without turning punctuation, arrows, or localized text
            // it does not own into missing-glyph boxes. Global CJK/emoji faces
            // remain the last tier.
            if (!face.role || *face.role == font.role ||
                (font.role != FontRole::content &&
                 *face.role == FontRole::content)) {
                result.push_back(&face);
            }
        }
        std::stable_sort(result.begin(), result.end(), FacePreference{font});
        return result;
    }

    [[nodiscard]] bool has_primary(FontRole role) const noexcept {
        return std::any_of(faces.begin(), faces.end(), [role](const Face& face) {
            return face.role && *face.role == role;
        });
    }

    [[nodiscard]] static bool covers(Face& face,
                                     std::span<const char32_t> scalars) {
        for (const char32_t scalar : scalars) {
            if (ignorable_for_coverage(scalar)) continue;
            if (FT_Get_Char_Index(face.face, static_cast<FT_ULong>(scalar)) == 0U) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] std::optional<FontFaceId> register_face(
        std::optional<FontRole> role, std::uint16_t weight, bool italic,
        std::span<const std::byte> encoded, std::uint32_t face_index) {
        if (encoded.empty() || encoded.size() > maximum_face_bytes ||
            faces.size() >= maximum_faces || face_index > 255U) {
            return std::nullopt;
        }
        std::vector<std::byte> owned(encoded.begin(), encoded.end());
        FT_Face native{};
        const FT_Error opened = FT_New_Memory_Face(
            library.value, reinterpret_cast<const FT_Byte*>(owned.data()),
            static_cast<FT_Long>(owned.size()), static_cast<FT_Long>(face_index),
            &native);
        if (opened != 0 || native == nullptr || (*native).num_glyphs <= 0 ||
            (*native).num_glyphs > maximum_glyphs ||
            ((*native).face_flags & FT_FACE_FLAG_SCALABLE) == 0 ||
            FT_Select_Charmap(native, FT_ENCODING_UNICODE) != 0) {
            if (native != nullptr) FT_Done_Face(native);
            return std::nullopt;
        }
        const FontFaceId id{next_face_id++};
        std::string family = (*native).family_name != nullptr
            ? (*native).family_name : "unknown bundled face";
        faces.emplace_back(id, role, weight, italic, !role.has_value(),
                           std::move(family), std::move(owned), native);
        return id;
    }

    void append_run(ShapedText& result, Face& face, std::string_view utf8,
                    Utf8Range range, FontSpec font, double run_origin,
                    bool add_trailing_spacing) {
        const FT_F26Dot6 size = static_cast<FT_F26Dot6>(
            std::llround(std::clamp(font.size, 1.0, 4096.0) * 64.0));
        if (FT_Set_Char_Size(face.face, 0, size, 72U, 72U) != 0) return;
        hb_font_t* hb_font = hb_ft_font_create_referenced(face.face);
        if (hb_font == nullptr) return;
        hb_ft_font_set_load_flags(hb_font, FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP);
        hb_buffer_t* buffer = hb_buffer_create();
        if (buffer == nullptr) {
            hb_font_destroy(hb_font);
            return;
        }
        hb_buffer_set_cluster_level(buffer,
            HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
        hb_buffer_add_utf8(buffer, utf8.data(), static_cast<int>(utf8.size()),
                           static_cast<unsigned>(range.start.value()),
                           static_cast<int>(range.end.value() -
                                            range.start.value()));
        hb_buffer_guess_segment_properties(buffer);
        hb_shape(hb_font, buffer, nullptr, 0U);

        unsigned count{};
        const hb_glyph_info_t* infos = hb_buffer_get_glyph_infos(buffer, &count);
        const hb_glyph_position_t* positions =
            hb_buffer_get_glyph_positions(buffer, &count);
        ShapedFontRun run;
        run.face = face.id;
        run.source_range = range;
        run.glyphs.reserve(count);
        double pen_x = run_origin;
        double pen_y{};
        for (unsigned index = 0U; index < count; ++index) {
            const float offset_x = positions[index].x_offset / 64.0F;
            const float offset_y = -positions[index].y_offset / 64.0F;
            float advance_x = positions[index].x_advance / 64.0F;
            const float advance_y = -positions[index].y_advance / 64.0F;
            const bool cluster_end = index + 1U == count ||
                infos[index + 1U].cluster != infos[index].cluster;
            if (cluster_end && (index + 1U != count || add_trailing_spacing)) {
                advance_x += static_cast<float>(font.letter_spacing);
            }
            run.glyphs.push_back({GlyphId{infos[index].codepoint},
                                  Utf8Offset(infos[index].cluster),
                                  static_cast<float>(pen_x) + offset_x,
                                  static_cast<float>(pen_y) + offset_y,
                                  advance_x, advance_y});
            pen_x += advance_x;
            pen_y += advance_y;
        }
        result.width = std::max(result.width, pen_x);
        if ((*face.face).size != nullptr) {
            const double ascent = (*(*face.face).size).metrics.ascender / 64.0;
            const double descent = -(*(*face.face).size).metrics.descender / 64.0;
            result.ascent = std::max(result.ascent, ascent);
            result.descent = std::max(result.descent, descent);
            result.height = std::max(result.height, ascent + descent);
        }
        result.runs.push_back(std::move(run));
        hb_buffer_destroy(buffer);
        hb_font_destroy(hb_font);
    }
};

HarfBuzzFontEngine::HarfBuzzFontEngine() : impl_(std::make_unique<Impl>()) {}
HarfBuzzFontEngine::~HarfBuzzFontEngine() = default;

std::optional<FontFaceId> HarfBuzzFontEngine::register_typeface(
    FontRole role, std::uint16_t weight, bool italic,
    std::span<const std::byte> encoded, std::uint32_t face_index) {
    return (*impl_).register_face(role, weight, italic, encoded, face_index);
}

std::optional<FontFaceId> HarfBuzzFontEngine::register_fallback_typeface(
    std::uint16_t weight, bool italic, std::span<const std::byte> encoded,
    std::uint32_t face_index) {
    return (*impl_).register_face(
        std::nullopt, weight, italic, encoded, face_index);
}

ShapedText HarfBuzzFontEngine::shape(std::string_view utf8, FontSpec font) {
    ShapedText result;
    if (utf8.empty()) {
        result.height = font.size;
        return result;
    }
    if (!validate_utf8(utf8).valid() || !valid_font_spec(font)) {
        return result;
    }
    std::vector<Impl::Face*> candidates = (*impl_).candidates(font);
    if (!(*impl_).has_primary(font.role) || candidates.empty()) {
        result.missing_primary_face = true;
        return result;
    }

    TextStore store(utf8);
    struct Segment final { Impl::Face* face{}; Utf8Range range{}; };
    std::vector<Segment> segments;
    for (std::size_t index = 0U; index < store.grapheme_count().value(); ++index) {
        const Utf8Range range = store.grapheme_range(GraphemeIndex(index));
        const std::vector<char32_t> scalars = decode_scalars(utf8, range);
        Impl::Face* selected = nullptr;
        for (Impl::Face* candidate : candidates) {
            if (Impl::covers(*candidate, scalars)) {
                selected = candidate;
                break;
            }
        }
        if (selected == nullptr) {
            selected = candidates.front();
            ++result.missing_clusters;
        }
        if (!segments.empty() && segments.back().face == selected &&
            segments.back().range.end == range.start) {
            segments.back().range.end = range.end;
        } else {
            segments.push_back({selected, range});
        }
    }

    double origin{};
    for (std::size_t index = 0U; index < segments.size(); ++index) {
        const Segment& segment = segments[index];
        const double before = result.width;
        (*impl_).append_run(result, *segment.face, utf8, segment.range, font, origin,
                          index + 1U != segments.size());
        origin += std::max(0.0, result.width - before);
    }
    return result;
}

std::size_t HarfBuzzFontEngine::face_count() const noexcept {
    return (*impl_).faces.size();
}

ResolvedTextLayout HarfBuzzFontEngine::resolve(std::string_view utf8,
                                               FontSpec font) {
    ResolvedTextLayout result;
    result.effective_font = font;
    if (!validate_utf8(utf8).valid() || !valid_font_spec(font)) {
        result.status = TextResolutionStatus::invalid_request;
        return result;
    }
    const std::vector<Impl::Face*> candidates = (*impl_).candidates(font);
    if (!(*impl_).has_primary(font.role) || candidates.empty()) {
        result.status = TextResolutionStatus::missing_primary_face;
        return result;
    }
    result.primary_family = (*candidates.front()).family;
    const ShapedText shaped = shape(utf8, font);
    result.logical_size = {shaped.width, shaped.height};
    result.ascent = shaped.ascent;
    result.descent = shaped.descent;
    result.line_gap = std::max(
        0.0, shaped.height - shaped.ascent - shaped.descent);
    result.missing_clusters = shaped.missing_clusters;
    result.status = shaped.missing_primary_face
        ? TextResolutionStatus::missing_primary_face
        : shaped.missing_clusters != 0U
            ? TextResolutionStatus::missing_cluster_coverage
            : TextResolutionStatus::exact;
    result.runs.reserve(shaped.runs.size());
    for (const ShapedFontRun& run : shaped.runs) {
        const Impl::Face* resolved = nullptr;
        for (const Impl::Face& face : (*impl_).faces) {
            if (face.id == run.face) {
                resolved = &face;
                break;
            }
        }
        if (resolved == nullptr) continue;
        result.runs.push_back({
            run.source_range.start.value(),
            run.source_range.end.value() - run.source_range.start.value(),
            (*resolved).family, (*resolved).weight, (*resolved).italic,
            (*resolved).fallback || !(*resolved).role ||
                *(*resolved).role != font.role});
    }
    return result;
}

} // namespace gui_forms::render::text
