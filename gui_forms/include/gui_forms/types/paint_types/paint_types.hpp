#pragma once

#include <cmath>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <span>

namespace gui_forms {

struct Color {
    std::uint8_t red{};
    std::uint8_t green{};
    std::uint8_t blue{};
    std::uint8_t alpha{255};
    friend constexpr bool operator==(const Color&, const Color&) = default;

    [[nodiscard]] static constexpr Color rgba(std::uint8_t red_value,
                                               std::uint8_t green_value,
                                               std::uint8_t blue_value,
                                               std::uint8_t alpha_value = 255) noexcept {
        return {red_value, green_value, blue_value, alpha_value};
    }
};

// Renderer-neutral retained paint vocabulary.  Stops are deliberately copied
// into display chunks; callers may therefore supply stack-backed spans without
// extending their lifetime through presentation.
struct GradientStop final {
    double offset{};
    Color color{};
    friend constexpr bool operator==(const GradientStop&,
                                     const GradientStop&) = default;
};

// Controls how a linear gradient behaves outside its authored start/end
// interval. `repeat` is the retained material primitive behind pinstripes,
// grooves, scanlines, and other scale-independent surface texture; `reflect`
// mirrors alternate intervals so the seam remains continuous.
enum class GradientSpreadMode : std::uint8_t {
    pad,
    repeat,
    reflect,
};

inline constexpr std::size_t maximum_gradient_stops = 32U;

[[nodiscard]] bool valid_gradient_stops(
    std::span<const GradientStop> stops) noexcept;

enum class FontRole : std::uint8_t {
    control,
    content,
    monospace,
};

enum class CursorKind : std::uint8_t {
    arrow,
    text,
    hand,
    crosshair,
    resize_horizontal,
    resize_vertical,
    wait,
    forbidden,
};

struct FontSpec {
    FontRole role{FontRole::control};
    double size{13.0};
    std::uint16_t weight{400};
    bool italic{};
    // Additional logical pixels inserted between shaped grapheme clusters.
    // This is a layout input: painters and measurement must apply the same
    // value. Zero preserves the typeface's native spacing.
    double letter_spacing{};
    friend constexpr bool operator==(const FontSpec&, const FontSpec&) = default;
};

[[nodiscard]] inline bool valid_font_spec(FontSpec font) noexcept {
    return std::isfinite(font.size) && font.size > 0.0 &&
           std::isfinite(font.letter_spacing) &&
           font.letter_spacing >= -font.size * 0.25 &&
           font.letter_spacing <= font.size;
}

struct ImageId {
    std::uint64_t value{};
    friend constexpr auto operator<=>(const ImageId&, const ImageId&) = default;
};

enum class ImagePatternWrap : std::uint8_t {
    tile,
};

enum class ImageSampling : std::uint8_t {
    nearest,
    linear,
};

} // namespace gui_forms
