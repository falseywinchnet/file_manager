#pragma once

#include "gui_forms/text.hpp"

#include <compare>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace gui_forms {

struct FontFaceId final {
    std::uint64_t value{};
    [[nodiscard]] constexpr bool valid() const noexcept { return value != 0; }
    friend constexpr auto operator<=>(const FontFaceId&,
                                      const FontFaceId&) = default;
};
struct GlyphId final {
    std::uint32_t value{};
    friend constexpr auto operator<=>(const GlyphId&, const GlyphId&) = default;
};
struct OpenTypeTag final {
    std::uint32_t value{};
    [[nodiscard]] constexpr bool valid() const noexcept { return value != 0; }
    friend constexpr auto operator<=>(const OpenTypeTag&,
                                      const OpenTypeTag&) = default;
};

[[nodiscard]] constexpr OpenTypeTag open_type_tag(
    char first, char second, char third, char fourth) noexcept {
    return {static_cast<std::uint32_t>(static_cast<unsigned char>(first)) << 24U |
            static_cast<std::uint32_t>(static_cast<unsigned char>(second)) << 16U |
            static_cast<std::uint32_t>(static_cast<unsigned char>(third)) << 8U |
            static_cast<std::uint32_t>(static_cast<unsigned char>(fourth))};
}

enum class TextDirection : std::uint8_t { left_to_right, right_to_left };

struct ShapingFeature final {
    OpenTypeTag tag{};
    std::uint32_t value{};
    Utf8Range range{};
    friend constexpr bool operator==(const ShapingFeature&,
                                     const ShapingFeature&) = default;
};
struct ShapingRequest final {
    std::string_view utf8_text;
    Utf8Range range{};
    FontFaceId face{};
    float font_size{};
    TextDirection direction{TextDirection::left_to_right};
    OpenTypeTag script{};
    std::string_view language;
    std::span<const ShapingFeature> features;
};
struct GlyphPlacement final {
    GlyphId glyph{};
    Utf8Offset cluster{};
    float advance_x{};
    float advance_y{};
    float offset_x{};
    float offset_y{};
    friend constexpr bool operator==(const GlyphPlacement&,
                                     const GlyphPlacement&) = default;
};
struct GlyphRun final {
    Utf8Range source_range{};
    FontFaceId face{};
    TextDirection direction{TextDirection::left_to_right};
    std::vector<GlyphPlacement> glyphs;
};

enum class ShapingRequestError : std::uint8_t {
    none, invalid_utf8, invalid_face, invalid_font_size, reversed_range,
    range_out_of_bounds, range_splits_scalar, range_splits_grapheme,
    invalid_feature_tag, feature_outside_range, feature_splits_scalar,
};
enum class GlyphRunError : std::uint8_t {
    none, invalid_request, mismatched_source_range, mismatched_face,
    mismatched_direction, cluster_out_of_range, cluster_splits_grapheme,
    nonmonotonic_clusters, nonfinite_placement,
};

[[nodiscard]] ShapingRequestError validate_shaping_request(
    const ShapingRequest& request);
[[nodiscard]] GlyphRunError validate_glyph_run(
    const ShapingRequest& request, const GlyphRun& run);
[[nodiscard]] std::string_view to_string(
    ShapingRequestError error) noexcept;
[[nodiscard]] std::string_view to_string(GlyphRunError error) noexcept;

struct FontFallbackRequest final {
    FontFaceId primary_face{};
    std::span<const FontFaceId> attempted_faces;
    std::span<const char32_t> cluster;
    std::string_view language;
    OpenTypeTag script{};
};
struct FontFallbackMatch final {
    FontFaceId face{};
    std::size_t covered_scalars{};
};

} // namespace gui_forms
