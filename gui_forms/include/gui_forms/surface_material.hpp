#pragma once

#include "gui_forms/types.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace gui_forms {

enum class MaterialFillKind : std::uint8_t {
    solid,
    linear_gradient,
    radial_gradient,
    image,
};

enum class MaterialImageMode : std::uint8_t {
    stretch,
    tile,
    nine_patch,
};

enum class MaterialCoordinateSpace : std::uint8_t {
    normalized,
    logical,
};

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
        ImageId image, Size pixel_size, double source_pixels_per_logical_pixel = 1.0,
        double opacity = 1.0);
    [[nodiscard]] static MaterialFillLayer nine_patch(
        ImageId image, Size pixel_size, Insets source_slice,
        double source_pixels_per_logical_pixel = 1.0,
        double opacity = 1.0);

    friend bool operator==(const MaterialFillLayer&,
                           const MaterialFillLayer&) = default;
};

struct MaterialBorder final {
    Color color{Color::rgba(76, 94, 111)};
    double width{1.0};
    friend constexpr bool operator==(const MaterialBorder&,
                                     const MaterialBorder&) = default;
};

struct MaterialShadow final {
    Point offset{0.0, 2.0};
    double blur_radius{4.0};
    double spread{};
    Color color{Color::rgba(20, 30, 42, 72)};
    friend constexpr bool operator==(const MaterialShadow&,
                                     const MaterialShadow&) = default;
};

struct SurfaceMaterial final {
    static constexpr std::size_t maximum_fill_layers = 8U;
    static constexpr std::size_t maximum_shadows = 4U;
    std::vector<MaterialFillLayer> fills{
        MaterialFillLayer::solid(Color::rgba(255, 255, 255))};
    std::vector<MaterialShadow> shadows;
    std::optional<MaterialBorder> border;
    double corner_radius{};

    friend bool operator==(const SurfaceMaterial&,
                           const SurfaceMaterial&) = default;
};

[[nodiscard]] bool valid_surface_material(
    const SurfaceMaterial& material) noexcept;

// Shared renderer-neutral realization used by MaterialPanel and themed stock
// controls. Geometry is resolved against `bounds`; no platform object escapes.
void paint_surface_material(Painter& painter, Rect bounds,
                            const SurfaceMaterial& material);
[[nodiscard]] Insets surface_material_visual_outsets(
    const SurfaceMaterial& material) noexcept;

} // namespace gui_forms
