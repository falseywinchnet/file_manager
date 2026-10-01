#include "harfbuzz_font_engine.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <hb-ft.h>
#include <hb.h>
#include <SheenBidi/SheenBidi.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace gui_forms::render::text {
namespace {

constexpr std::size_t maximum_face_bytes = 64U * 1024U * 1024U;
constexpr std::size_t maximum_faces = 64U;
constexpr FT_Long maximum_glyphs = 1'000'000L;

struct BidiOwner final {
    SBAlgorithmRef algorithm = nullptr;
    SBParagraphRef paragraph = nullptr;
    SBLineRef line = nullptr;
    BidiOwner() = default;
    BidiOwner(const BidiOwner&) = delete;
    BidiOwner& operator=(const BidiOwner&) = delete;
    ~BidiOwner() {
        if (line) { SBLineRelease(line); }
        if (paragraph) { SBParagraphRelease(paragraph); }
        if (algorithm) { SBAlgorithmRelease(algorithm); }
    }
    void clear_line() {
        if (line) { SBLineRelease(line); line = nullptr; }
        if (paragraph) { SBParagraphRelease(paragraph); paragraph = nullptr; }
    }
};
struct DirectionRun final { Utf8Range range{}; bool rtl{}; };
std::vector<DirectionRun> visual_direction_runs(std::string_view utf8) {
    std::vector<DirectionRun> runs{};
    const SBCodepointSequence sequence{SBStringEncodingUTF8, utf8.data(), utf8.size()};
    BidiOwner owner{};
    owner.algorithm = SBAlgorithmCreate(&sequence);
    if (!owner.algorithm) { throw std::bad_alloc(); }
    std::size_t offset = 0;
    while (offset < utf8.size()) {
        owner.paragraph = SBAlgorithmCreateParagraph(owner.algorithm, offset, utf8.size() - offset, SBLevelDefaultLTR);
        if (!owner.paragraph) { throw std::bad_alloc(); }
        const std::size_t length = SBParagraphGetLength(owner.paragraph);
        if (length == 0 || length > utf8.size() - offset) { throw std::runtime_error("Invalid bidi paragraph length"); }
        owner.line = SBParagraphCreateLine(owner.paragraph, offset, length);
        if (!owner.line) { throw std::bad_alloc(); }
        const SBRun* values = SBLineGetRunsPtr(owner.line);
        const std::size_t count = SBLineGetRunCount(owner.line);
        for (std::size_t i = 0; i < count; ++i) {
            runs.push_back({{Utf8Offset(values[i].offset), Utf8Offset(values[i].offset + values[i].length)}, (values[i].level & 1) != 0});
        }
        offset += length;
        owner.clear_line();
    }
    return runs;
}

struct LibraryOwner final {
    FT_Library value{};
    LibraryOwner() {
        const FT_Error initialized = FT_Init_FreeType(&value);
        if (initialized != 0 || value == nullptr) {
            throw std::runtime_error("FreeType initialization failed");
        }
    }
    LibraryOwner(const LibraryOwner&) = delete;
    LibraryOwner& operator=(const LibraryOwner&) = delete;
    ~LibraryOwner() {
        if (value != nullptr) FT_Done_FreeType(value);
    }
};

struct ReleaseFace final {
    void operator()(FT_Face face) const noexcept {
        if (face != nullptr) FT_Done_Face(face);
    }
};
using FreeTypeFaceOwner = std::unique_ptr<FT_FaceRec, ReleaseFace>;
using HarfBuzzFontOwner = std::unique_ptr<hb_font_t, void (*)(hb_font_t*)>;
using HarfBuzzBufferOwner = std::unique_ptr<hb_buffer_t, void (*)(hb_buffer_t*)>;

// utf8 is already validated; range contains complete scalars. The caller owns
// scratch storage and reserves at least range.length bytes as a scalar bound.
void decode_scalars(std::string_view utf8, Utf8Range range,
                    std::vector<char32_t>& scalars) {
    scalars.clear();
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
}

[[nodiscard]] bool ignorable_for_coverage(char32_t scalar) noexcept {
    const bool ignorable = scalar == U'\u200c' || scalar == U'\u200d' ||
           (scalar >= U'\ufe00' && scalar <= U'\ufe0f') ||
           (scalar >= U'\U000e0100' && scalar <= U'\U000e01ef');
    return ignorable;
}

} // namespace

class HarfBuzzFontEngine::Impl final {
public:
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
    TextLayoutDiagnostics diagnostics{};
    TextLayoutFailure failure{TextLayoutFailure::none};
    std::uint64_t failure_run{};
    [[nodiscard]] bool take_failure(TextLayoutFailure point) noexcept {
        if (failure != point || failure_run != diagnostics.append_calls) { return false; }
        failure = TextLayoutFailure::none;
        return true;
    }
#endif
    struct Face final {
        FontFaceId id{};
        std::optional<FontRole> role;
        std::uint16_t weight{};
        bool italic{};
        bool fallback{};
        std::string family;
        std::shared_ptr<const void> encoded;
        FT_Face face{};

        Face(FontFaceId id_value, std::optional<FontRole> role_value,
             std::uint16_t weight_value, bool italic_value,
             bool fallback_value, std::string family_value,
             std::shared_ptr<const void> bytes, FT_Face face_value)
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
        FontSpec font{};

        [[nodiscard]] int tier(const Face& face) const noexcept {
            if (face.role && *face.role == font.role) return 0;
            if (face.role && *face.role == FontRole::content) return 1;
            return 2;
        }

        [[nodiscard]] bool operator()(const Face* left,
                                      const Face* right) const noexcept {
            const int left_tier = tier(*left);
            const int right_tier = tier(*right);
            if (left_tier != right_tier) {
                const bool preferred = left_tier < right_tier;
                return preferred;
            }
            const int left_italic = (*left).italic == font.italic ? 0 : 1;
            const int right_italic = (*right).italic == font.italic ? 0 : 1;
            if (left_italic != right_italic) {
                const bool preferred = left_italic < right_italic;
                return preferred;
            }
            const int left_distance = std::abs(static_cast<int>((*left).weight) - font.weight);
            const int right_distance = std::abs(static_cast<int>((*right).weight) - font.weight);
            const bool preferred = left_distance < right_distance;
            return preferred;
        }
    };

    LibraryOwner library{};
    std::vector<Face> faces{};
    std::uint64_t next_face_id{1U};

    [[nodiscard]] std::vector<Face*> candidates(FontSpec font) {
        std::vector<Face*> result{};
        result.reserve(faces.size());
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
        for (const Face& face : faces) {
            if (face.role && *face.role == role) return true;
        }
        return false;
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
        std::span<const std::byte> bytes, std::shared_ptr<const void> owned,
        std::uint32_t face_index) {
        if (!owned || bytes.empty() || bytes.size() > maximum_face_bytes ||
            faces.size() >= maximum_faces || face_index > 255U) {
            return std::nullopt;
        }
        FT_Face native{};
        const FT_Error opened = FT_New_Memory_Face(
            library.value, reinterpret_cast<const FT_Byte*>(bytes.data()),
            static_cast<FT_Long>(bytes.size()), static_cast<FT_Long>(face_index),
            &native);
        FreeTypeFaceOwner pending_face{native};
        if (opened != 0 || native == nullptr || (*native).num_glyphs <= 0 ||
            (*native).num_glyphs > maximum_glyphs ||
            ((*native).face_flags & FT_FACE_FLAG_SCALABLE) == 0) {
            return std::nullopt;
        }
        const FT_Error charmap_selected = FT_Select_Charmap(native, FT_ENCODING_UNICODE);
        if (charmap_selected != 0) return std::nullopt;
        // Retain native cleanup until the owning Face has been constructed.
        // Family-name allocation and vector growth can both throw.
        const FontFaceId id{next_face_id++};
        std::string family = (*native).family_name != nullptr
            ? (*native).family_name : "unknown bundled face";
        faces.emplace_back(id, role, weight, italic, !role.has_value(),
                           std::move(family), std::move(owned), native);
        static_cast<void>(pending_face.release());
        return id;
    }

    struct CallFont final {
        Face* face{};
        HarfBuzzFontOwner owner{nullptr, hb_font_destroy};
    };
    // Native owners and face borrows live for one synchronous shape call.
    // Registration cannot run concurrently; the face vector stays unchanged.
    struct ShapeCall final {
        std::array<CallFont, maximum_faces> fonts{};
        std::size_t font_count{};
        HarfBuzzBufferOwner buffer{nullptr, hb_buffer_destroy};
    };

    void append_run(ShapeCall& call, ShapedText& result, Face& face, std::string_view utf8,
                    Utf8Range range, FontSpec font, double run_origin,
                    bool add_trailing_spacing, bool rtl) {
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
        ++diagnostics.append_calls;
        TextLayoutPhaseTimer append_timer(diagnostics, TextLayoutPhase::append_total);
#endif
        hb_font_t* hb_font = nullptr;
        const std::size_t font_count = call.font_count;
        for (std::size_t index = 0; index < font_count; ++index) {
            const CallFont& entry = call.fonts[index];
            if (entry.face == &face) {
                hb_font = entry.owner.get();
                break;
            }
        }
        if (hb_font == nullptr) {
            if (font_count >= maximum_faces) { throw std::runtime_error("Shape face bound exceeded"); }
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
            TextLayoutPhaseTimer size_timer(diagnostics, TextLayoutPhase::ft_size);
#endif
            const double bounded_size = std::clamp(font.size, 1.0, 4096.0);
            const long long fixed_size = std::llround(bounded_size * 64.0);
            const FT_F26Dot6 size = static_cast<FT_F26Dot6>(fixed_size);
            const FT_Error size_set = FT_Set_Char_Size(face.face, 0, size, 72U, 72U);
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
            size_timer.stop();
#endif
            if (size_set != 0) { throw std::runtime_error("FreeType size setup failed"); }
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
            TextLayoutPhaseTimer font_timer(diagnostics, TextLayoutPhase::hb_font);
#endif
            hb_font_t* acquired_font = nullptr;
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
            if (take_failure(TextLayoutFailure::empty_font)) {
                acquired_font = hb_font_get_empty();
            } else if (take_failure(TextLayoutFailure::unbound_font)) {
                acquired_font = hb_font_create(hb_face_get_empty());
            } else
#endif
            {
                acquired_font = hb_ft_font_create_referenced(face.face);
            }
            HarfBuzzFontOwner font_owner{acquired_font, hb_font_destroy};
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
            font_timer.stop();
#endif
            hb_font = font_owner.get();
            if (hb_font == nullptr || hb_font == hb_font_get_empty() ||
                hb_ft_font_get_ft_face(hb_font) != face.face) { throw std::bad_alloc(); }
            hb_ft_font_set_load_flags(hb_font, FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP);
            CallFont& entry = call.fonts[font_count];
            entry.face = &face;
            entry.owner = std::move(font_owner);
            ++call.font_count;
        }
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
        TextLayoutPhaseTimer buffer_timer(diagnostics, TextLayoutPhase::buffer_setup);
#endif
        if (!call.buffer) {
            hb_buffer_t* acquired = nullptr;
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
            if (take_failure(TextLayoutFailure::empty_buffer)) {
                acquired = hb_buffer_get_empty();
            } else
#endif
            {
                acquired = hb_buffer_create();
            }
            call.buffer.reset(acquired);
        }
        hb_buffer_t* buffer = call.buffer.get();
        if (buffer == nullptr || !hb_buffer_allocation_successful(buffer)) { throw std::bad_alloc(); }
        // Full reset restores flags, script/language, context and Unicode
        // functions to newly-created defaults before establishing this run.
        hb_buffer_reset(buffer);
        hb_buffer_set_cluster_level(buffer,
            HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
        hb_buffer_add_utf8(buffer, utf8.data(), static_cast<int>(utf8.size()),
                           static_cast<unsigned>(range.start.value()),
                           static_cast<int>(range.end.value() -
                                            range.start.value()));
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
        if (take_failure(TextLayoutFailure::after_add)) { throw std::bad_alloc(); }
#endif
        if (!hb_buffer_allocation_successful(buffer)) { throw std::bad_alloc(); }
        hb_buffer_set_direction(buffer, rtl ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
        hb_buffer_guess_segment_properties(buffer);
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
        buffer_timer.stop();
        TextLayoutPhaseTimer shape_timer(diagnostics, TextLayoutPhase::hb_shape);
#endif
        const char* const* shapers = nullptr;
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
        constexpr const char* unavailable_shapers[]{"diagnostic-unavailable-shaper", nullptr};
        if (take_failure(TextLayoutFailure::no_shaper)) { shapers = unavailable_shapers; }
#endif
        const hb_bool_t shaped = hb_shape_full(hb_font, buffer, nullptr, 0U, shapers);
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
        if (take_failure(TextLayoutFailure::after_shape)) { throw std::bad_alloc(); }
#endif
        if (!hb_buffer_allocation_successful(buffer)) { throw std::bad_alloc(); }
        if (!shaped) { throw std::runtime_error("HarfBuzz shaping failed"); }
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
        shape_timer.stop();
        TextLayoutPhaseTimer output_timer(diagnostics, TextLayoutPhase::glyph_output);
#endif

        unsigned count{};
        const hb_glyph_info_t* infos = hb_buffer_get_glyph_infos(buffer, &count);
        const hb_glyph_position_t* positions =
            hb_buffer_get_glyph_positions(buffer, &count);
        ShapedFontRun run{};
        run.face = face.id;
        run.source_range = range;
#if defined(GUI_FORMS_TEXT_LAYOUT_GEOMETRY_TRACE)
        run.diagnostic_rtl = rtl;
#endif
        run.glyphs.reserve(count);
        double pen_x = run_origin;
        double pen_y{};
        double ink_ascent = font.size * 0.8, ink_descent = font.size * 0.2;
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
            hb_glyph_extents_t extents{};
            if (!face.role && hb_font_get_glyph_extents(hb_font, infos[index].codepoint, &extents)) {
                const double top = pen_y + offset_y - extents.y_bearing / 64.0;
                const double bottom = top - extents.height / 64.0;
                ink_ascent = std::max(ink_ascent, -top);
                ink_descent = std::max(ink_descent, bottom);
            }
            pen_x += advance_x;
            pen_y += advance_y;
        }
        result.width = std::max(result.width, pen_x);
        if ((*face.face).size != nullptr) {
            // Fallback families reserve space for every mark in their repertoire.
            // Use the actual shaped ink (including positioned marks) so an Arabic
            // or CJK fallback does not double a compact control's line box.
            const double ascent = face.role ? (*(*face.face).size).metrics.ascender / 64.0 : ink_ascent;
            const double descent = face.role ? -(*(*face.face).size).metrics.descender / 64.0 : ink_descent;
            result.ascent = std::max(result.ascent, ascent);
            result.descent = std::max(result.descent, descent);
            result.height = result.ascent + result.descent;
        }
        result.runs.push_back(std::move(run));
    }
};

HarfBuzzFontEngine::HarfBuzzFontEngine() : impl_(std::make_unique<Impl>()) {}
HarfBuzzFontEngine::~HarfBuzzFontEngine() = default;

std::optional<FontFaceId> HarfBuzzFontEngine::register_shared_typeface(
    std::optional<FontRole> role, std::uint16_t weight, bool italic,
    std::shared_ptr<const std::vector<std::byte>> encoded, std::uint32_t face_index) {
    if (!encoded) return std::nullopt;
    const std::span<const std::byte> bytes(*encoded);
    const std::optional<FontFaceId> result =
        register_owned_typeface(role, weight, italic, bytes, std::move(encoded), face_index);
    return result;
}

std::optional<FontFaceId> HarfBuzzFontEngine::register_owned_typeface(
    std::optional<FontRole> role, std::uint16_t weight, bool italic,
    std::span<const std::byte> encoded, std::shared_ptr<const void> owner,
    std::uint32_t face_index) {
    const std::optional<FontFaceId> result =
        (*impl_).register_face(role, weight, italic, encoded, std::move(owner), face_index);
    return result;
}

std::optional<FontFaceId> HarfBuzzFontEngine::register_typeface(
    FontRole role, std::uint16_t weight, bool italic,
    std::span<const std::byte> encoded, std::uint32_t face_index) {
    if (encoded.empty() || encoded.size() > maximum_face_bytes ||
        (*impl_).faces.size() >= maximum_faces || face_index > 255U) return std::nullopt;
    std::shared_ptr<const std::vector<std::byte>> owner =
        std::make_shared<const std::vector<std::byte>>(encoded.begin(), encoded.end());
    const std::optional<FontFaceId> result =
        register_shared_typeface(role, weight, italic, std::move(owner), face_index);
    return result;
}

std::optional<FontFaceId> HarfBuzzFontEngine::register_fallback_typeface(
    std::uint16_t weight, bool italic, std::span<const std::byte> encoded,
    std::uint32_t face_index) {
    if (encoded.empty() || encoded.size() > maximum_face_bytes ||
        (*impl_).faces.size() >= maximum_faces || face_index > 255U) return std::nullopt;
    std::shared_ptr<const std::vector<std::byte>> owner =
        std::make_shared<const std::vector<std::byte>>(encoded.begin(), encoded.end());
    const std::optional<FontFaceId> result =
        register_shared_typeface(std::nullopt, weight, italic, std::move(owner), face_index);
    return result;
}

ShapedText HarfBuzzFontEngine::shape(std::string_view utf8, FontSpec font) {
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
    TextLayoutDiagnostics& counters = (*impl_).diagnostics;
    counters = TextLayoutDiagnostics{};
#endif
    ShapedText result{};
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

#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
    TextLayoutPhaseTimer store_timer(counters, TextLayoutPhase::store_graphemes);
#endif
    TextStore store(utf8);
    struct Segment final { Impl::Face* face{}; Utf8Range range{}; };
    const std::size_t graphemes = store.grapheme_count().value();
    std::size_t maximum_cluster_bytes = 0U;
    for (std::size_t index = 0U; index < graphemes; ++index) {
        const Utf8Range range = store.grapheme_range(GraphemeIndex(index));
        const std::size_t bytes = range.end.value() - range.start.value();
        maximum_cluster_bytes = std::max(maximum_cluster_bytes, bytes);
    }
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
    store_timer.stop();
    counters.graphemes = static_cast<std::uint64_t>(graphemes);
    TextLayoutPhaseTimer fallback_timer(counters, TextLayoutPhase::fallback);
#endif
    std::vector<char32_t> scalars{};
    scalars.reserve(maximum_cluster_bytes);
    std::vector<Segment> segments{};
    // Grow only when a distinct font run is published. Most text coalesces
    // into one run; reserving one Segment per grapheme wastes input-sized space.
    for (std::size_t index = 0U; index < graphemes; ++index) {
        const Utf8Range range = store.grapheme_range(GraphemeIndex(index));
        decode_scalars(utf8, range, scalars);
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

#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
    fallback_timer.stop();
    counters.segments = static_cast<std::uint64_t>(segments.size());
    TextLayoutPhaseTimer bidi_timer(counters, TextLayoutPhase::bidi);
#endif
    // Unicode bidi ordering is independent of font fallback. Reorder whole
    // directional runs, then shape font fragments with the resolved direction;
    // never reverse UTF-8 bytes or reorder stored application text.
    const std::vector<DirectionRun> directions = visual_direction_runs(utf8);
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
    bidi_timer.stop();
    counters.directions = static_cast<std::uint64_t>(directions.size());
    TextLayoutPhaseTimer intersection_timer(counters, TextLayoutPhase::intersections);
#endif
    struct VisualSegment final { Impl::Face* face{}; Utf8Range range{}; bool rtl{}; };
    std::vector<VisualSegment> visual{};
    std::vector<VisualSegment> parts{};
    parts.reserve(segments.size());
    for (const DirectionRun& direction : directions) {
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
        counters.intersection_pairs += counters.segments;
#endif
        parts.clear();
        for (const Segment& segment : segments) {
            const std::size_t start = std::max(segment.range.start.value(), direction.range.start.value());
            const std::size_t end = std::min(segment.range.end.value(), direction.range.end.value());
            if (start < end) { parts.push_back({segment.face, {Utf8Offset(start), Utf8Offset(end)}, direction.rtl}); }
        }
        if (direction.rtl) { std::reverse(parts.begin(), parts.end()); }
        visual.insert(visual.end(), parts.begin(), parts.end());
    }
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
    intersection_timer.stop();
    counters.visual_runs = static_cast<std::uint64_t>(visual.size());
#endif
    double origin{};
    Impl::ShapeCall call{};
    for (std::size_t index = 0U; index < visual.size(); ++index) {
        const VisualSegment& segment = visual[index];
        const double before = result.width;
        (*impl_).append_run(call, result, *segment.face, utf8, segment.range, font, origin,
                          index + 1U != visual.size(), segment.rtl);
        origin += std::max(0.0, result.width - before);
    }
    return result;
}

std::size_t HarfBuzzFontEngine::face_count() const noexcept {
    const std::size_t count = (*impl_).faces.size();
    return count;
}

#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
TextLayoutDiagnostics HarfBuzzFontEngine::diagnostics() const noexcept {
    const TextLayoutDiagnostics result = (*impl_).diagnostics;
    return result;
}
void HarfBuzzFontEngine::set_diagnostic_failure(TextLayoutFailure failure, std::uint64_t run) {
    if (failure != TextLayoutFailure::none && run == 0) {
        throw std::invalid_argument("Diagnostic failure run must be nonzero");
    }
    (*impl_).failure = failure;
    (*impl_).failure_run = run;
}
#endif

ResolvedTextLayout HarfBuzzFontEngine::resolve(std::string_view utf8,
                                               FontSpec font) {
    ResolvedTextLayout result{};
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
