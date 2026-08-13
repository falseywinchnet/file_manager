#pragma once

#include "gui_forms/text_shaping.hpp"
#include "gui_forms/typography.hpp"
#include "gui_forms/types.hpp"

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
        return left.glyph == right.glyph && left.cluster == right.cluster &&
               left.x == right.x && left.y == right.y &&
               left.advance_x == right.advance_x &&
               left.advance_y == right.advance_y;
    }
};

struct ShapedFontRun final {
    FontFaceId face{};
    Utf8Range source_range{};
    std::vector<ShapedGlyph> glyphs;
};

struct ShapedText final {
    std::vector<ShapedFontRun> runs;
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
    [[nodiscard]] ShapedText shape(std::string_view utf8, FontSpec font);
    [[nodiscard]] ResolvedTextLayout resolve(std::string_view utf8,
                                             FontSpec font);
    [[nodiscard]] std::size_t face_count() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace gui_forms::render::text
