#pragma once

#include "gui_forms/drawing/color/color.hpp"
#include "gui_forms/drawing/matrix/matrix.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace gui_drawing {

enum class DashStyle : std::uint8_t { solid, dash, dot, dash_dot, dash_dot_dot, custom };
enum class FontStyle : std::uint32_t {
    regular = 0,
    bold = 1U << 0U,
    italic = 1U << 1U,
    underline = 1U << 2U,
    strikeout = 1U << 3U,
};

enum class GraphicsUnit : std::uint8_t { world, display, pixel, point, inch, document, millimeter };
enum class StringAlignment : std::uint8_t { near, center, far };
enum class StringTrimming : std::uint8_t { none, character, word, ellipsis_character, ellipsis_word, ellipsis_path };
enum class SmoothingMode : std::uint8_t { default_mode, high_speed, high_quality, none, anti_alias };
enum class InterpolationMode : std::uint8_t { default_mode, low, high, nearest, bilinear, bicubic, high_quality_bilinear, high_quality_bicubic };
enum class PixelOffsetMode : std::uint8_t { default_mode, high_speed, high_quality, none, half };
enum class CompositingMode : std::uint8_t { source_over, source_copy };
enum class CompositingQuality : std::uint8_t { default_mode, high_speed, high_quality, gamma_corrected, assume_linear };
enum class FillMode : std::uint8_t { alternate, winding };
enum class PixelFormat : std::uint8_t { bgra32_premultiplied, rgba32_premultiplied };
enum class WrapMode : std::uint8_t { tile, tile_flip_x, tile_flip_y, tile_flip_xy, clamp };
enum class HatchStyle : std::uint8_t {
    horizontal, vertical, forward_diagonal, backward_diagonal,
    cross, diagonal_cross,
};
struct PixelStorage;

struct ImageSnapshot final {
    std::uint64_t stable_id{};
    std::uint32_t width{};
    std::uint32_t height{};
    PixelFormat pixel_format{PixelFormat::bgra32_premultiplied};
    std::uint64_t generation{};

    [[nodiscard]] bool has_pixels() const noexcept;
    [[nodiscard]] std::size_t row_bytes() const noexcept;
    [[nodiscard]] std::span<const std::byte> pixels() const noexcept;

private:
    std::shared_ptr<const PixelStorage> storage_;
    friend class Bitmap;
};

enum class BrushKind : std::uint8_t {
    solid,
    hatch,
    linear_gradient,
    path_gradient,
    texture,
};

struct ColorBlend final {
    std::vector<Color> colors;
    std::vector<double> positions;
};

struct BrushSnapshot final {
    BrushKind kind{BrushKind::solid};
    Color primary;
    Color secondary;
    HatchStyle hatch_style{HatchStyle::horizontal};
    WrapMode wrap_mode{WrapMode::tile};
    RectF bounds;
    PointF center;
    double angle{};
    std::vector<PointF> points;
    std::vector<Color> colors;
    std::vector<double> positions;
    ImageSnapshot image;
    Matrix transform;
};

} // namespace gui_drawing
