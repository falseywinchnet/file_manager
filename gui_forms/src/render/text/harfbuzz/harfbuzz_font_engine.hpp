#pragma once

#include "gui_forms/text_shaping.hpp"
#include "gui_forms/typography.hpp"
#include "gui_forms/types.hpp"

#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
#include "text_layout_diagnostics.hpp"
#endif

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace gui_forms::render::text {

struct ShapedGlyph final {
    GlyphId glyph{};
    Utf8Offset cluster{};
    float x{};
    float y{};
    float advance_x{};
    float advance_y{};
    friend constexpr bool operator==(const ShapedGlyph& left,
                                     const ShapedGlyph& right) noexcept {
        const bool equal = left.glyph == right.glyph && left.cluster == right.cluster &&
               left.x == right.x && left.y == right.y &&
               left.advance_x == right.advance_x &&
               left.advance_y == right.advance_y;
        return equal;
    }
};

struct ShapedFontRun final {
    FontFaceId face{};
    Utf8Range source_range{};
#if defined(GUI_FORMS_TEXT_LAYOUT_GEOMETRY_TRACE)
    bool diagnostic_rtl{};
#endif
    std::vector<ShapedGlyph> glyphs{};
};

struct ShapedText final {
    std::vector<ShapedFontRun> runs{};
    double width{};
    double height{};
    double ascent{};
    double descent{};
    std::size_t missing_clusters{};
    bool missing_primary_face{};
};

struct BoundedFontRun final {
    FontFaceId face{};
    Utf8Range source_range{};
    std::size_t glyph_begin{};
    std::size_t glyph_count{};
};

struct ShapeStorageLimits final {
    std::size_t input_bytes{16'384};
    std::size_t runs{16'384};
    std::size_t glyphs{65'536};
    std::size_t output_bytes{8U * 1024U * 1024U};
    std::size_t workspace_bytes{16U * 1024U * 1024U};
};

// Private complete geometry owner. Arrays have fixed allocated capacity and
// explicit live counts. Native allocator payload is not included in these
// controlled byte reports. The enclosing unique owner transfers as one unit.
struct BoundedShapedText final {
    std::unique_ptr<BoundedFontRun[]> runs{};
    std::unique_ptr<ShapedGlyph[]> glyphs{};
    std::size_t run_count{};
    std::size_t glyph_count{};
    std::size_t run_capacity{};
    std::size_t glyph_capacity{};
    std::size_t controlled_output_bytes{};
    std::size_t controlled_workspace_peak{};
    double width{};
    double height{};
    double ascent{};
    double descent{};
    std::size_t missing_clusters{};
    bool missing_primary_face{};
};

// Private portable text engine. It owns every encoded face and never consults
// a host font catalog. The renderer maps returned face IDs to its own private
// raster objects; HarfBuzz/FreeType objects never cross this boundary.
class HarfBuzzFontEngine final {
public:
    HarfBuzzFontEngine();
    ~HarfBuzzFontEngine();
    HarfBuzzFontEngine(const HarfBuzzFontEngine&) = delete;
    HarfBuzzFontEngine& operator=(const HarfBuzzFontEngine&) = delete;

    [[nodiscard]] std::optional<FontFaceId> register_typeface(
        FontRole role, std::uint16_t weight, bool italic,
        std::span<const std::byte> encoded, std::uint32_t face_index = 0U);
    // A fallback face participates after all faces registered for the
    // requested role, without duplicating its encoded bytes per role.
    [[nodiscard]] std::optional<FontFaceId> register_fallback_typeface(
        std::uint16_t weight, bool italic,
        std::span<const std::byte> encoded, std::uint32_t face_index = 0U);
    // Shared immutable bytes let the rasterizer and shaper retain one font
    // allocation. Span overloads continue to take an independent owned copy.
    [[nodiscard]] std::optional<FontFaceId> register_shared_typeface(
        std::optional<FontRole> role, std::uint16_t weight, bool italic,
        std::shared_ptr<const std::vector<std::byte>> encoded,
        std::uint32_t face_index = 0U);
    // The owner keeps the immutable span alive, including file mappings. This
    // avoids a private heap copy of every bundled font in each native window.
    [[nodiscard]] std::optional<FontFaceId> register_owned_typeface(
        std::optional<FontRole> role, std::uint16_t weight, bool italic,
        std::span<const std::byte> encoded, std::shared_ptr<const void> owner,
        std::uint32_t face_index = 0U);
    // Native setup/allocation failures throw; a call-owned partial result never
    // becomes a successful return. Callers retain old state until return succeeds.
    [[nodiscard]] ShapedText shape(std::string_view utf8, FontSpec font);
    // Throws length_error before a controlled allocation would exceed limits;
    // invalid input and native/resource failures also throw. The caller's old
    // owner survives until it explicitly adopts the complete returned owner.
    [[nodiscard]] std::unique_ptr<BoundedShapedText> shape_bounded(
        std::string_view utf8, FontSpec font, ShapeStorageLimits limits);
    [[nodiscard]] ResolvedTextLayout resolve(std::string_view utf8,
                                             FontSpec font);
    [[nodiscard]] std::size_t face_count() const noexcept;
#if defined(GUI_FORMS_TEXT_LAYOUT_DIAGNOSTICS)
    [[nodiscard]] TextLayoutDiagnostics diagnostics() const noexcept;
    // Executor-confined, consumed at the named point/run. An unreached point
    // remains armed until replaced; none explicitly disarms it.
    void set_diagnostic_failure(TextLayoutFailure failure, std::uint64_t run);
#endif

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace gui_forms::render::text
