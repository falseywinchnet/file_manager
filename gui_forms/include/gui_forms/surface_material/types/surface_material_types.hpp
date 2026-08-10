#pragma once

#include "gui_forms/surface_material/material_fill_layer/material_fill_layer.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace gui_forms {

struct MaterialBorder final {
    Color color{Color::rgba(76, 94, 111)};
    double width{1.0};
    friend constexpr bool operator==(const MaterialBorder& left,
                                     const MaterialBorder& right) noexcept {
        return left.color == right.color && left.width == right.width;
    }
};
struct MaterialShadow final {
    Point offset{0.0, 2.0};
    double blur_radius{4.0};
    double spread{};
    Color color{Color::rgba(20, 30, 42, 72)};
    friend constexpr bool operator==(const MaterialShadow& left,
                                     const MaterialShadow& right) noexcept {
        return left.offset == right.offset &&
               left.blur_radius == right.blur_radius &&
               left.spread == right.spread && left.color == right.color;
    }
};
struct SurfaceMaterial final {
    static constexpr std::size_t maximum_fill_layers = 8U;
    static constexpr std::size_t maximum_shadows = 4U;
    std::vector<MaterialFillLayer> fills{
        MaterialFillLayer::solid(Color::rgba(255, 255, 255))};
    std::vector<MaterialShadow> shadows;
    std::optional<MaterialBorder> border;
    double corner_radius{};
    friend bool operator==(const SurfaceMaterial& left,
                           const SurfaceMaterial& right) noexcept(noexcept(
        left.fills == right.fills && left.shadows == right.shadows &&
        left.border == right.border &&
        left.corner_radius == right.corner_radius)) {
        return left.fills == right.fills && left.shadows == right.shadows &&
               left.border == right.border &&
               left.corner_radius == right.corner_radius;
    }
};

} // namespace gui_forms
