#pragma once

#include "gui_forms/types.hpp"

#include <cstddef>
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
enum class MaterialLinearGeometry : std::uint8_t {
    endpoints, css_angle,
};

// One renderer-neutral material layer with explicit coordinate, spread,
// image sampling, scale, opacity, and nine-patch state.
struct MaterialFillLayer final {
    MaterialFillKind kind{MaterialFillKind::solid};
    MaterialCoordinateSpace coordinate_space{MaterialCoordinateSpace::normalized};
    MaterialLinearGeometry linear_geometry{MaterialLinearGeometry::endpoints};
    double angle_degrees{180.0};
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
    [[nodiscard]] static MaterialFillLayer linear_css_angle(
        double angle_degrees, std::vector<GradientStop> stops,
        GradientSpreadMode spread = GradientSpreadMode::pad);
    [[nodiscard]] static MaterialFillLayer linear_css_angle(
        double angle_degrees, const GradientStop* stops,
        std::size_t stop_count,
        GradientSpreadMode spread = GradientSpreadMode::pad);
    [[nodiscard]] static MaterialFillLayer linear(
        Point start, Point end, const GradientStop* stops,
        std::size_t stop_count,
        MaterialCoordinateSpace space = MaterialCoordinateSpace::normalized,
        GradientSpreadMode spread = GradientSpreadMode::pad);
    [[nodiscard]] static MaterialFillLayer repeating_linear(
        Point start, Point end, std::vector<GradientStop> stops,
        MaterialCoordinateSpace space = MaterialCoordinateSpace::logical);
    [[nodiscard]] static MaterialFillLayer repeating_linear(
        Point start, Point end, const GradientStop* stops,
        std::size_t stop_count,
        MaterialCoordinateSpace space = MaterialCoordinateSpace::logical);
    [[nodiscard]] static MaterialFillLayer radial(
        Point center, Size radii, std::vector<GradientStop> stops,
        MaterialCoordinateSpace space = MaterialCoordinateSpace::normalized);
    [[nodiscard]] static MaterialFillLayer radial(
        Point center, Size radii, const GradientStop* stops,
        std::size_t stop_count,
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
    friend bool operator==(const MaterialFillLayer& left,
                           const MaterialFillLayer& right) noexcept(noexcept(
        left.kind == right.kind &&
        left.coordinate_space == right.coordinate_space &&
        left.linear_geometry == right.linear_geometry &&
        left.angle_degrees == right.angle_degrees &&
        left.color == right.color && left.start == right.start &&
        left.end == right.end && left.center == right.center &&
        left.radii == right.radii && left.stops == right.stops &&
        left.spread == right.spread && left.image == right.image &&
        left.image_pixel_size == right.image_pixel_size &&
        left.image_slice == right.image_slice &&
        left.image_scale == right.image_scale &&
        left.opacity == right.opacity && left.image_mode == right.image_mode)) {
        return left.kind == right.kind &&
               left.coordinate_space == right.coordinate_space &&
               left.linear_geometry == right.linear_geometry &&
               left.angle_degrees == right.angle_degrees &&
               left.color == right.color && left.start == right.start &&
               left.end == right.end && left.center == right.center &&
               left.radii == right.radii && left.stops == right.stops &&
               left.spread == right.spread && left.image == right.image &&
               left.image_pixel_size == right.image_pixel_size &&
               left.image_slice == right.image_slice &&
               left.image_scale == right.image_scale &&
               left.opacity == right.opacity &&
               left.image_mode == right.image_mode;
    }
};

} // namespace gui_forms
