#include "gui_forms/controls/drawing_surface/drawing_surface.hpp"

#include <utility>

namespace gui_forms {

DrawingSurface::DrawingSurface(StableId stable_id)
    : Control(std::move(stable_id)) {}

void DrawingSurface::set_paint_callback(PaintCallback callback) {
    require_mutable();
    paint_callback_ = std::move(callback);
    invalidate(Dirty::paint | Dirty::semantics);
}

void DrawingSurface::set_hit_test_visible(bool visible) {
    require_mutable();
    if (hit_test_visible_ == visible) return;
    hit_test_visible_ = visible;
    invalidate(Dirty::hit_test | Dirty::semantics);
}

void DrawingSurface::set_semantic_role(SemanticRole role) {
    require_mutable();
    if (semantic_role_ == role) return;
    semantic_role_ = role;
    invalidate(Dirty::semantics);
}

void DrawingSurface::set_background(Color color) {
    require_mutable();
    if (background_ == color) return;
    background_ = color;
    invalidate(Dirty::paint);
}

void DrawingSurface::on_paint(Painter& painter, Rect local_damage) {
    const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                      committed_arranged_bounds().height};
    painter.fill_rect(bounds, background_);
    if (paint_callback_) paint_callback_(painter, bounds, local_damage);
}

bool DrawingSurface::hit_test_local(Point local_point) const {
    return hit_test_visible_ && Control::hit_test_local(local_point);
}

SemanticDescriptor DrawingSurface::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = semantic_role_;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    descriptor.exposed = !descriptor.name.empty() || !descriptor.description.empty();
    return descriptor;
}

} // namespace gui_forms
