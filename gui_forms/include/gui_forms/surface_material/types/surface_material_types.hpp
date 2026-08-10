#pragma once

#include "gui_forms/surface_material/material_fill_layer/material_fill_layer.hpp"

#include <cstddef>
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
struct MaterialBorderEdges final {
    std::optional<MaterialBorder> top;
    std::optional<MaterialBorder> right;
    std::optional<MaterialBorder> bottom;
    std::optional<MaterialBorder> left;
    [[nodiscard]] static MaterialBorderEdges from_parts(
        const MaterialBorder* top, const MaterialBorder* right,
        const MaterialBorder* bottom, const MaterialBorder* left);
    [[nodiscard]] bool empty() const noexcept {
        return !top && !right && !bottom && !left;
    }
    friend bool operator==(const MaterialBorderEdges& left_value,
                           const MaterialBorderEdges& right_value) noexcept {
        return left_value.top == right_value.top &&
               left_value.right == right_value.right &&
               left_value.bottom == right_value.bottom &&
               left_value.left == right_value.left;
    }
};
struct SurfaceMaterial final {
    static constexpr std::size_t maximum_fill_layers = 8U;
    static constexpr std::size_t maximum_shadows = 4U;
    std::vector<MaterialFillLayer> fills{
        MaterialFillLayer::solid(Color::rgba(255, 255, 255))};
    std::vector<MaterialShadow> shadows;
    std::optional<MaterialBorder> border;
    MaterialBorderEdges border_edges;
    double corner_radius{};
    [[nodiscard]] static SurfaceMaterial from_parts(
        const MaterialFillLayer* fills, std::size_t fill_count,
        const MaterialShadow* shadows, std::size_t shadow_count,
        const MaterialBorder* border, double corner_radius);
    [[nodiscard]] static SurfaceMaterial from_parts(
        const MaterialFillLayer* fills, std::size_t fill_count,
        const MaterialShadow* shadows, std::size_t shadow_count,
        const MaterialBorder* border, const MaterialBorderEdges* border_edges,
        double corner_radius);
    friend bool operator==(const SurfaceMaterial& left,
                           const SurfaceMaterial& right) noexcept(noexcept(
        left.fills == right.fills && left.shadows == right.shadows &&
        left.border == right.border && left.border_edges == right.border_edges &&
        left.corner_radius == right.corner_radius)) {
        return left.fills == right.fills && left.shadows == right.shadows &&
               left.border == right.border &&
               left.border_edges == right.border_edges &&
               left.corner_radius == right.corner_radius;
    }
};

} // namespace gui_forms
