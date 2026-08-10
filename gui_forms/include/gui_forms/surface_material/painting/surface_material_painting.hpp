#pragma once

#include "gui_forms/surface_material/types/surface_material_types.hpp"

namespace gui_forms {

[[nodiscard]] bool valid_surface_material(
    const SurfaceMaterial& material) noexcept;
void paint_surface_material(Painter& painter, Rect bounds,
                            const SurfaceMaterial& material);
[[nodiscard]] Insets surface_material_visual_outsets(
    const SurfaceMaterial& material) noexcept;

} // namespace gui_forms
