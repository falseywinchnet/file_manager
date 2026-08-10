#pragma once

#include "gui_forms/types.hpp"

#include <cstdint>
#include <vector>

namespace gui_forms {

enum class MaterialFillKind : std::uint8_t {
    solid, linear_gradient, radial_gradient, image,
};
enum class MaterialImageMode : std::uint8_t {
    stretch, tile, nine_patch,
};
enum class MaterialCoordinateSpace : std::uint8_t {
    normalized, logical,
};

// One renderer-neutral material layer with explicit coordinate, spread,
// image sampling, scale, opacity, and nine-patch state.
struct MaterialFillLayer final {
    MaterialFillKind kind{MaterialFillKind::solid};
    MaterialCoordinateSpace coordinate_space{MaterialCoordinateSpace::normalized};
    Color color{Color::rgba(255, 255, 255)};
    Point start{};
    Point end{1.0, 0.0};
    Point center{0.5, 0.5};
    Size radii{0.5, 0.5};
    std::vector<GradientStop> stops;
    GradientSpreadMode spread{GradientSpreadMode::pad};
    ImageId image{};
    Size image_pixel_size{};
    Insets image_slice{};
    double image_scale{1.0};
    double opacity{1.0};
    MaterialImageMode image_mode{MaterialImageMode::stretch};

    [[nodiscard]] static MaterialFillLayer solid(Color color);
    [[nodiscard]] static MaterialFillLayer linear(
        Point start, Point end, std::vector<GradientStop> stops,
        MaterialCoordinateSpace space = MaterialCoordinateSpace::normalized,
        GradientSpreadMode spread = GradientSpreadMode::pad);
    [[nodiscard]] static MaterialFillLayer repeating_linear(
        Point start, Point end, std::vector<GradientStop> stops,
        MaterialCoordinateSpace space = MaterialCoordinateSpace::logical);
    [[nodiscard]] static MaterialFillLayer radial(
        Point center, Size radii, std::vector<GradientStop> stops,
        MaterialCoordinateSpace space = MaterialCoordinateSpace::normalized);
    [[nodiscard]] static MaterialFillLayer stretched_image(
        ImageId image, Size pixel_size, double opacity = 1.0);
    [[nodiscard]] static MaterialFillLayer tiled_image(
        ImageId image, Size pixel_size,
        double source_pixels_per_logical_pixel = 1.0,
        double opacity = 1.0);
    [[nodiscard]] static MaterialFillLayer nine_patch(
        ImageId image, Size pixel_size, Insets source_slice,
        double source_pixels_per_logical_pixel = 1.0,
        double opacity = 1.0);
    friend bool operator==(const MaterialFillLayer&,
                           const MaterialFillLayer&) = default;
};

} // namespace gui_forms
