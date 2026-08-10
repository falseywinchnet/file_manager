#include "gui_forms/controls/panel/material_panel/material_panel.hpp"
#include "gui_forms/window.hpp"

#include <stdexcept>
#include <utility>

namespace gui_forms {

MaterialPanel::MaterialPanel(StableId stable_id)
    : Panel(std::move(stable_id)) {
    set_background(Color::rgba(0, 0, 0, 0));
}

void MaterialPanel::set_material(SurfaceMaterial material) {
    if (!valid_surface_material(material)) {
        throw std::invalid_argument("material recipe is invalid or unbounded");
    }
    validate_window_images(material);
    if (material_ == material) return;
    material_ = std::move(material);
    invalidate(invalidation::paint_only);
    publish_change(material_changed_, material_);
}

void MaterialPanel::on_attached_to_window() {
    Panel::on_attached_to_window();
    validate_window_images(material_);
}

void MaterialPanel::validate_window_images(
    const SurfaceMaterial& material) const {
    if (window() == nullptr) return;
    for (const MaterialFillLayer& fill : material.fills) {
        if (fill.kind != MaterialFillKind::image) continue;
        const auto resource = window()->image_resources().find(fill.image);
        if (!resource ||
            static_cast<double>(resource->metadata.width) !=
                fill.image_pixel_size.width ||
            static_cast<double>(resource->metadata.height) !=
                fill.image_pixel_size.height) {
            throw std::invalid_argument(
                "material image is missing from the attached Window or its declared pixel size is stale");
        }
    }
}

void MaterialPanel::on_paint(Painter& painter, Rect) {
    const Rect arranged = committed_arranged_bounds();
    const Rect bounds{0.0, 0.0, arranged.width, arranged.height};
    if (bounds.empty()) return;

    paint_surface_material(painter, bounds, material_);
}

Insets MaterialPanel::visual_outsets() const noexcept {
    return surface_material_visual_outsets(material_);
}

} // namespace gui_forms
