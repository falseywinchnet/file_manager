#include "harfbuzz_font_engine.hpp"
#include "bounded_shape_workspace.hpp"
#include "../../../core/text/unicode/unicode_grapheme.hpp"

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

void account_array(std::size_t count, std::size_t element_bytes,
                   std::size_t limit, std::size_t& used) {
    if (used > limit || count > (limit - used) / element_bytes) {
        throw std::length_error("Controlled shaping storage exceeds its admitted limit");
    }
    const std::size_t bytes = count * element_bytes;
    used += bytes;
}

std::size_t decode_scalars_to(std::string_view utf8, Utf8Range range,
                              std::span<char32_t> output) {
    std::size_t offset = range.start.value();
    std::size_t written = 0;
    while (offset < range.end.value()) {
        const unsigned char first = static_cast<unsigned char>(utf8[offset]);
        char32_t value{};
        std::size_t count{};
        if (first < 0x80U) { value = first; count = 1U; }
        else if ((first & 0xe0U) == 0xc0U) { value = first & 0x1fU; count = 2U; }
        else if ((first & 0xf0U) == 0xe0U) { value = first & 0x0fU; count = 3U; }
        else { value = first & 0x07U; count = 4U; }
        for (std::size_t index = 1U; index < count; ++index) {
            value = (value << 6U) | (static_cast<unsigned char>(utf8[offset + index]) & 0x3fU);
        }
        if (written == output.size()) throw std::length_error("Scalar scratch capacity exceeded");
        output[written] = value;
        ++written;
        offset += count;
    }
    return written;
}

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
    std::size_t bounded_face_limit{};

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
        bool nominal = true;
        for (const char32_t scalar : scalars) {
            if (ignorable_for_coverage(scalar)) continue;
            if (FT_Get_Char_Index(face.face, static_cast<FT_ULong>(scalar)) == 0U) {
                nominal = false;
                break;
            }
        }
        if (nominal) { return true; }
        // HarfBuzz can canonically compose a two-scalar cluster even when the
        // face has no nominal glyph for its combining mark. Keep original text
        // and offsets; this is only a bounded same-face coverage check.
        if (scalars.size() != 2) { return false; }
        const hb_codepoint_t first = static_cast<hb_codepoint_t>(scalars[0]);
        const hb_codepoint_t second = static_cast<hb_codepoint_t>(scalars[1]);
        hb_codepoint_t composed{};
        hb_unicode_funcs_t* unicode = hb_unicode_funcs_get_default();
        const hb_bool_t combined = hb_unicode_compose(unicode, first, second, &composed);
        if (!combined) { return false; }
        const FT_ULong scalar = static_cast<FT_ULong>(composed);
        const bool covered = FT_Get_Char_Index(face.face, scalar) != 0U;
        return covered;
    }

    [[nodiscard]] std::optional<FontFaceId> register_face(
        std::optional<FontRole> role, std::uint16_t weight, bool italic,
        std::span<const std::byte> bytes, std::shared_ptr<const void> owned,
        std::uint32_t face_index) {
        if (bounded_face_limit != 0 && faces.size() == bounded_face_limit) return std::nullopt;
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
        std::string family{};
        if (bounded_face_limit == 0) {
            family = (*native).family_name != nullptr ? (*native).family_name : "unknown bundled face";
        }
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

    template <bool Bounded = false>
    void append_run(ShapeCall& call, ShapedText& result, Face& face, std::string_view utf8,
                    Utf8Range range, FontSpec font, double run_origin,
                    bool add_trailing_spacing, bool rtl, BoundedShapedText* bounded = nullptr) {
        if constexpr (Bounded) {
            if ((*bounded).run_count == (*bounded).run_capacity) {
                throw std::length_error("Prepared run capacity exceeded");
            }
        }
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
        unsigned position_count{};
        const hb_glyph_position_t* positions =
            hb_buffer_get_glyph_positions(buffer, &position_count);
        if (position_count != count || (count != 0U && (infos == nullptr || positions == nullptr))) {
            throw std::runtime_error("Incomplete HarfBuzz glyph records");
        }
        ShapedFontRun run{};
        run.face = face.id;
        run.source_range = range;
#if defined(GUI_FORMS_TEXT_LAYOUT_GEOMETRY_TRACE)
        run.diagnostic_rtl = rtl;
#endif
        if constexpr (Bounded) {
            if (count > (*bounded).glyph_capacity - (*bounded).glyph_count) {
                throw std::length_error("Prepared glyph capacity exceeded");
            }
        } else {
            run.glyphs.reserve(count);
        }
        double pen_x = run_origin;
        double pen_y{};
        double ink_ascent = font.size * 0.8, ink_descent = font.size * 0.2;
        for (unsigned index = 0U; index < count; ++index) {
            const float offset_x = static_cast<float>(positions[index].x_offset) / 64.0F;
            const std::int64_t inverted_y_offset = -static_cast<std::int64_t>(positions[index].y_offset);
            const float offset_y = static_cast<float>(inverted_y_offset) / 64.0F;
            float advance_x = static_cast<float>(positions[index].x_advance) / 64.0F;
            const std::int64_t inverted_y_advance = -static_cast<std::int64_t>(positions[index].y_advance);
            const float advance_y = static_cast<float>(inverted_y_advance) / 64.0F;
            const bool cluster_end = index + 1U == count ||
                infos[index + 1U].cluster != infos[index].cluster;
            if (cluster_end && (index + 1U != count || add_trailing_spacing)) {
                advance_x += static_cast<float>(font.letter_spacing);
            }
            const ShapedGlyph glyph{GlyphId{infos[index].codepoint},
                                  Utf8Offset(infos[index].cluster),
                                  static_cast<float>(pen_x) + offset_x,
                                  static_cast<float>(pen_y) + offset_y,
                                  advance_x, advance_y};
            if constexpr (Bounded) {
                (*bounded).glyphs[(*bounded).glyph_count + index] = glyph;
            } else {
                run.glyphs.push_back(glyph);
            }
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
        if constexpr (Bounded) {
            (*bounded).runs[(*bounded).run_count] = BoundedFontRun{
                .face = face.id, .source_range = range,
                .glyph_begin = (*bounded).glyph_count, .glyph_count = count};
            ++(*bounded).run_count;
            (*bounded).glyph_count += count;
        } else {
            result.runs.push_back(std::move(run));
        }
    }
    std::size_t workspace_external_bytes(const std::size_t limit) const {
        if (bounded_face_limit == 0U) {
            throw std::invalid_argument("Reusable workspace requires bounded font registration");
        }
        std::size_t bytes = 0U;
        account_array(1U, sizeof(HarfBuzzFontEngine), limit, bytes);
        account_array(1U, sizeof(Impl), limit, bytes);
        account_array(faces.capacity(), sizeof(Face), limit, bytes);
        account_array(maximum_faces, sizeof(Face*), limit, bytes);
        account_array(1U, sizeof(ShapeCall), limit, bytes);
        account_array(1U, sizeof(BidiOwner), limit, bytes);
        account_array(1U, sizeof(ShapedText), limit, bytes);
        return bytes;
    }

    std::size_t bounded_candidates(const FontSpec font, const std::span<Face*> candidates) {
        std::size_t count = 0U;
        const FacePreference preference{font};
        for (Face& face : faces) {
            if (!face.role || *face.role == font.role ||
                (font.role != FontRole::content && *face.role == FontRole::content)) {
                if (count == candidates.size()) throw std::length_error("Face candidate capacity exceeded");
                std::size_t position = count;
                while (position != 0U && preference(&face, candidates[position - 1U])) {
                    candidates[position] = candidates[position - 1U];
                    --position;
                }
                candidates[position] = &face;
                ++count;
            }
        }
        return count;
    }

    // Validated input and complete grapheme edges; nonempty candidates all
    // borrow this engine's unchanged face table. Scratch arrays are disjoint,
    // scalar/segment/direction extents cover the validated scalar count and
    // visual covers twice that count. No array grows; native owners stay local.
    void fill_bounded_runs(const std::string_view utf8, const FontSpec font,
        const std::span<Face* const> candidates, const std::span<const std::size_t> edges,
        const std::span<char32_t> scalar_scratch, const std::span<WorkspaceFontSegment> segments,
        const std::span<WorkspaceDirectionRun> directions, const std::span<WorkspaceVisualSegment> visual,
        BoundedShapedText& output) {
        const std::size_t candidate_count = candidates.size();
        const std::size_t scalar_capacity = scalar_scratch.size();
        const std::size_t visual_capacity = visual.size();
        std::size_t segment_count = 0;
        for (std::size_t index = 1; index < edges.size(); ++index) {
            const Utf8Range range{Utf8Offset(edges[index - 1U]), Utf8Offset(edges[index])};
            const std::size_t scalar_count = decode_scalars_to(utf8, range, scalar_scratch);
            const std::span<const char32_t> cluster(scalar_scratch.data(), scalar_count);
            Impl::Face* selected = nullptr;
            for (std::size_t face_index = 0; face_index < candidate_count; ++face_index) {
                if (Impl::covers(*candidates[face_index], cluster)) { selected = candidates[face_index]; break; }
            }
            if (selected == nullptr) { selected = candidates[0]; ++output.missing_clusters; }
            const std::size_t selected_index = static_cast<std::size_t>(selected - faces.data());
            if (segment_count != 0 && segments[segment_count - 1U].face_index == selected_index) {
                segments[segment_count - 1U].range.end = range.end;
            } else {
                if (segment_count == scalar_capacity) throw std::length_error("Font segment capacity exceeded");
                segments[segment_count] = WorkspaceFontSegment{.face_index = selected_index, .range = range};
                ++segment_count;
            }
        }

        BidiOwner bidi{};
        const SBCodepointSequence sequence{SBStringEncodingUTF8, utf8.data(), utf8.size()};
        bidi.algorithm = SBAlgorithmCreate(&sequence);
        if (bidi.algorithm == nullptr) throw std::bad_alloc();
        std::size_t offset = 0;
        std::size_t direction_count = 0;
        while (offset < utf8.size()) {
            bidi.paragraph = SBAlgorithmCreateParagraph(bidi.algorithm, offset, utf8.size() - offset, SBLevelDefaultLTR);
            if (bidi.paragraph == nullptr) throw std::bad_alloc();
            const std::size_t length = SBParagraphGetLength(bidi.paragraph);
            if (length == 0 || length > utf8.size() - offset) throw std::runtime_error("Invalid bounded bidi paragraph");
            bidi.line = SBParagraphCreateLine(bidi.paragraph, offset, length);
            if (bidi.line == nullptr) throw std::bad_alloc();
            const std::size_t count = SBLineGetRunCount(bidi.line);
            if (count > scalar_capacity - direction_count) throw std::length_error("Direction capacity exceeded");
            const SBRun* runs = SBLineGetRunsPtr(bidi.line);
            for (std::size_t index = 0; index < count; ++index) {
                const SBRun& run = runs[index];
                if (run.offset > utf8.size() || run.length > utf8.size() - run.offset) {
                    throw std::runtime_error("Invalid bounded bidi run");
                }
                directions[direction_count] = WorkspaceDirectionRun{
                    .range = {Utf8Offset(run.offset), Utf8Offset(run.offset + run.length)}, .rtl = (run.level & 1U) != 0};
                ++direction_count;
            }
            offset += length;
            bidi.clear_line();
        }
        std::size_t visual_count = 0;
        for (std::size_t index = 0; index < direction_count; ++index) {
            const WorkspaceDirectionRun& direction = directions[index];
            const std::size_t begin = visual_count;
            for (std::size_t segment_index = 0; segment_index < segment_count; ++segment_index) {
                const WorkspaceFontSegment& segment = segments[segment_index];
                const std::size_t start = std::max(segment.range.start.value(), direction.range.start.value());
                const std::size_t end = std::min(segment.range.end.value(), direction.range.end.value());
                if (start < end) {
                    if (visual_count == visual_capacity) throw std::length_error("Visual run capacity exceeded");
                    visual[visual_count] = WorkspaceVisualSegment{.face_index = segment.face_index,
                        .range = {Utf8Offset(start), Utf8Offset(end)}, .rtl = direction.rtl};
                    ++visual_count;
                }
            }
            if (direction.rtl) std::reverse(visual.data() + begin, visual.data() + visual_count);
        }
        if (visual_count > output.run_capacity) throw std::length_error("Prepared run capacity exceeded");
        ShapedText metrics{};
        Impl::ShapeCall call{};
        double origin = 0;
        for (std::size_t index = 0; index < visual_count; ++index) {
            const WorkspaceVisualSegment& segment = visual[index];
            const double before = metrics.width;
            append_run<true>(call, metrics, faces[segment.face_index], utf8, segment.range, font,
                origin, index + 1U != visual_count, segment.rtl, &output);
            origin += std::max(0.0, metrics.width - before);
        }
        output.width = metrics.width;
        output.height = metrics.height;
        output.ascent = metrics.ascent;
        output.descent = metrics.descent;
    }
};

HarfBuzzFontEngine::HarfBuzzFontEngine() : impl_(std::make_unique<Impl>()) {}
HarfBuzzFontEngine::~HarfBuzzFontEngine() = default;

std::size_t HarfBuzzFontEngine::configure_bounded_registration(std::size_t face_limit, std::size_t byte_limit) {
    Impl& implementation = *impl_;
    if (!implementation.faces.empty() || implementation.bounded_face_limit != 0 ||
        face_limit == 0 || face_limit > maximum_faces) throw std::invalid_argument("Bounded face initialization order");
    std::size_t bytes = sizeof(HarfBuzzFontEngine) + sizeof(Impl);
    account_array(face_limit, sizeof(Impl::Face), byte_limit, bytes);
    implementation.faces.reserve(face_limit);
    implementation.bounded_face_limit = face_limit;
    // Count actual capacity too; no face insertion may grow this table.
    bytes = sizeof(HarfBuzzFontEngine) + sizeof(Impl);
    account_array(implementation.faces.capacity(), sizeof(Impl::Face), byte_limit, bytes);
    return bytes;
}

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

std::unique_ptr<BoundedShapedText> HarfBuzzFontEngine::shape_bounded(
    std::string_view utf8, FontSpec font, ShapeStorageLimits limits) {
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
    (*impl_).diagnostics = TextLayoutDiagnostics{};
#endif
    if (limits.input_bytes == 0 || limits.runs == 0 || limits.glyphs == 0 ||
        utf8.size() > limits.input_bytes || utf8.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        throw std::length_error("Bounded shaping input capacity exceeded");
    }
    if (!valid_font_spec(font)) throw std::invalid_argument("Invalid bounded shaping font");
    const gui_forms::detail::GraphemeStorageRequirement grapheme_requirement =
        gui_forms::detail::grapheme_storage_requirement(utf8);
    if (grapheme_requirement.status == gui_forms::detail::GraphemeStorageStatus::invalid_utf8) {
        throw std::invalid_argument("Invalid bounded shaping UTF-8");
    }
    if (grapheme_requirement.status != gui_forms::detail::GraphemeStorageStatus::success) {
        throw std::length_error("Grapheme storage arithmetic overflow");
    }
    using Segment = WorkspaceFontSegment;
    using VisualSegment = WorkspaceVisualSegment;
    using DirectionRun = WorkspaceDirectionRun;
    using FaceCandidates = std::array<Impl::Face*, maximum_faces>;
    const std::size_t scalar_capacity = grapheme_requirement.scalar_count;
    if (scalar_capacity > std::numeric_limits<std::size_t>::max() / 2U) {
        throw std::length_error("Visual intersection capacity overflow");
    }
    const std::size_t visual_capacity = scalar_capacity * 2U;
    std::size_t workspace = 0;
    account_array(1, sizeof(FaceCandidates), limits.workspace_bytes, workspace);
    account_array(1, sizeof(Impl::ShapeCall), limits.workspace_bytes, workspace);
    const std::size_t fixed_workspace = workspace;
    account_array(grapheme_requirement.boundary_capacity, sizeof(std::size_t), limits.workspace_bytes, workspace);
    account_array(scalar_capacity, sizeof(char32_t), limits.workspace_bytes, workspace);
    account_array(scalar_capacity, sizeof(Segment), limits.workspace_bytes, workspace);
    account_array(scalar_capacity, sizeof(DirectionRun), limits.workspace_bytes, workspace);
    account_array(visual_capacity, sizeof(VisualSegment), limits.workspace_bytes, workspace);
    std::size_t segmentation_peak = fixed_workspace;
    account_array(grapheme_requirement.peak_bytes, 1, limits.workspace_bytes, segmentation_peak);
    const std::size_t workspace_peak = std::max(workspace, segmentation_peak);
    std::size_t output_bytes = sizeof(BoundedShapedText);
    account_array(limits.runs, sizeof(BoundedFontRun), limits.output_bytes, output_bytes);
    account_array(limits.glyphs, sizeof(ShapedGlyph), limits.output_bytes, output_bytes);

    std::unique_ptr<BoundedShapedText> owner = std::make_unique<BoundedShapedText>();
    BoundedShapedText& output = *owner;
    output.runs = std::make_unique<BoundedFontRun[]>(limits.runs);
    output.glyphs = std::make_unique<ShapedGlyph[]>(limits.glyphs);
    output.run_capacity = limits.runs;
    output.glyph_capacity = limits.glyphs;
    output.controlled_output_bytes = output_bytes;
    output.controlled_workspace_peak = workspace_peak;
    if (utf8.empty()) { output.height = font.size; return owner; }

    FaceCandidates candidates{};
    const std::size_t candidate_count = (*impl_).bounded_candidates(font, candidates);
    if (!(*impl_).has_primary(font.role) || candidate_count == 0) {
        output.missing_primary_face = true;
        return owner;
    }
    gui_forms::detail::GraphemeBoundaryBuffer boundaries{};
    const gui_forms::detail::GraphemeStorageStatus segmented =
        gui_forms::detail::bounded_grapheme_boundaries(utf8,
            limits.workspace_bytes - fixed_workspace, boundaries);
    if (segmented == gui_forms::detail::GraphemeStorageStatus::resource_failure) throw std::bad_alloc();
    if (segmented != gui_forms::detail::GraphemeStorageStatus::success) {
        throw std::length_error("Bounded grapheme preparation failed");
    }
    std::unique_ptr<char32_t[]> scalars = std::make_unique<char32_t[]>(scalar_capacity);
    std::unique_ptr<Segment[]> segments = std::make_unique<Segment[]>(scalar_capacity);
    std::unique_ptr<DirectionRun[]> directions = std::make_unique<DirectionRun[]>(scalar_capacity);
    std::unique_ptr<VisualSegment[]> visual = std::make_unique<VisualSegment[]>(visual_capacity);
    const std::span<const std::size_t> edges = boundaries.boundaries();
    const std::span<char32_t> scalar_scratch(scalars.get(), scalar_capacity);
    const std::span<WorkspaceFontSegment> segment_scratch(segments.get(), scalar_capacity);
    const std::span<WorkspaceDirectionRun> direction_scratch(directions.get(), scalar_capacity);
    const std::span<WorkspaceVisualSegment> visual_scratch(visual.get(), visual_capacity);
    const std::span<Impl::Face* const> selected_faces(candidates.data(), candidate_count);
    (*impl_).fill_bounded_runs(utf8, font, selected_faces, edges, scalar_scratch,
        segment_scratch, direction_scratch, visual_scratch, output);
    return owner;
}

void HarfBuzzFontEngine::prepare_workspace(BoundedShapeWorkspace& workspace,
    const ShapeStorageLimits& limits) {
    const std::size_t external_bytes = (*impl_).workspace_external_bytes(limits.workspace_bytes);
    workspace.prepare(limits, external_bytes);
}

std::unique_ptr<BoundedShapedText> HarfBuzzFontEngine::shape_with_workspace(
    const std::string_view utf8, const FontSpec font, const ShapeStorageLimits& limits,
    BoundedShapeWorkspace& workspace) {
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
    (*impl_).diagnostics = TextLayoutDiagnostics{};
#endif
    const ShapeStorageLimits ceiling{};
    if (limits.input_bytes == 0U || limits.input_bytes > ceiling.input_bytes ||
        limits.runs == 0U || limits.runs > ceiling.runs ||
        limits.glyphs == 0U || limits.glyphs > ceiling.glyphs ||
        limits.output_bytes > ceiling.output_bytes || limits.workspace_bytes > ceiling.workspace_bytes ||
        utf8.size() > limits.input_bytes || limits.input_bytes > workspace.scalar_capacity_ ||
        limits.runs > workspace.staging_.run_capacity || limits.glyphs > workspace.staging_.glyph_capacity) {
        throw std::length_error("Reusable shaping input or prepared capacity exceeded");
    }
    if (!valid_font_spec(font)) throw std::invalid_argument("Invalid reusable shaping font");
    const Utf8ValidationResult validation = validate_utf8(utf8);
    if (!validation.valid()) throw std::invalid_argument("Invalid reusable shaping UTF-8");
    std::size_t peak = (*impl_).workspace_external_bytes(limits.workspace_bytes);
    const std::size_t retained = workspace.controlled_bytes();
    account_array(retained, 1U, limits.workspace_bytes, peak);
    workspace.clear_result();
    BoundedShapedText& staged = workspace.staging_;
    if (utf8.empty()) {
        staged.height = font.size;
    } else {
        std::array<Impl::Face*, maximum_faces> candidates{};
        const std::size_t candidate_count = (*impl_).bounded_candidates(font, candidates);
        if (!(*impl_).has_primary(font.role) || candidate_count == 0U) {
            staged.missing_primary_face = true;
        } else {
            const detail::GraphemeStorageStatus segmented = workspace.graphemes_.fill(utf8);
            if (segmented != detail::GraphemeStorageStatus::success) {
                throw std::length_error("Reusable grapheme capacity exceeded");
            }
            const std::span<const std::size_t> edges = workspace.graphemes_.boundaries();
            const std::span<char32_t> scalars(workspace.scalars_.get(), validation.scalar_count);
            const std::span<WorkspaceFontSegment> segments(workspace.segments_.get(), validation.scalar_count);
            const std::span<WorkspaceDirectionRun> directions(workspace.directions_.get(), validation.scalar_count);
            const std::size_t visual_count = validation.scalar_count * 2U;
            const std::span<WorkspaceVisualSegment> visual(workspace.visual_.get(), visual_count);
            const std::span<Impl::Face* const> selected(candidates.data(), candidate_count);
            // Per-call limits may be smaller than prepared capacities. Restore
            // capacities even when native shaping or a resource check throws.
            const std::size_t run_capacity = staged.run_capacity;
            const std::size_t glyph_capacity = staged.glyph_capacity;
            staged.run_capacity = limits.runs;
            staged.glyph_capacity = limits.glyphs;
            try {
                (*impl_).fill_bounded_runs(utf8, font, selected, edges, scalars, segments, directions, visual, staged);
            } catch (...) {
                staged.run_capacity = run_capacity;
                staged.glyph_capacity = glyph_capacity;
                workspace.clear_result();
                throw;
            }
            staged.run_capacity = run_capacity;
            staged.glyph_capacity = glyph_capacity;
        }
    }
    std::unique_ptr<BoundedShapedText> result = workspace.copy_result(limits.output_bytes, peak);
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
