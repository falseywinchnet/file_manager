#include "gui_forms/controls/panel/panel.hpp"
#include "../basic/basic_control_rendering.hpp"
#include "gui_forms/text.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

namespace gui_forms {

Panel::Panel(StableId stable_id) : ScrollableControl(std::move(stable_id)) {
    set_paint_plane(PaintPlane::backplane);
}

void Panel::set_border_style(BorderStyle style) {
    require_mutable();
    switch (style) {
    case BorderStyle::none:
    case BorderStyle::line:
    case BorderStyle::sunken:
    case BorderStyle::raised:
        break;
    default:
        throw std::invalid_argument("Panel border style is invalid");
    }
    if (border_style_ == style) {
        return;
    }
    border_style_ = style;
    invalidate(Dirty::paint | Dirty::semantics);
}

void Panel::set_background(Color color) {
    require_mutable();
    if (background_override_ && *background_override_ == color) return;
    background_override_ = color;
    invalidate(Dirty::paint);
}

Color Panel::background() const noexcept {
    if (background_override_) return *background_override_;
    const ControlVisualRecipe& recipe = effective_theme().resolve(
        visual_role_, visual_context());
    const MaterialFillLayer& fill = recipe.material.fills.front();
    return fill.kind == MaterialFillKind::solid ? fill.color
                                                : fill.stops.front().color;
}

void Panel::clear_background() {
    require_mutable();
    if (!background_override_) return;
    background_override_.reset();
    invalidate(Dirty::style | Dirty::paint);
}

void Panel::set_style(BasicControlStyle style) {
    require_mutable();
    if (style_override_ && *style_override_ == style) return;
    style_override_ = std::move(style);
    invalidate(Dirty::paint | Dirty::semantics);
}

const BasicControlStyle& Panel::style() const noexcept {
    return style_override_ ? *style_override_ : effective_theme().basic_style();
}

void Panel::clear_style() {
    require_mutable();
    if (!style_override_) return;
    style_override_.reset();
    invalidate(Dirty::style | Dirty::paint | Dirty::semantics);
}

void Panel::set_visual_role(ControlVisualRole role) {
    require_mutable();
    if (role != ControlVisualRole::window && role != ControlVisualRole::panel &&
        role != ControlVisualRole::card) {
        throw std::invalid_argument(
            "panel visual role must be window, panel, or card");
    }
    if (visual_role_ == role) return;
    visual_role_ = role;
    invalidate(Dirty::style | Dirty::paint | Dirty::semantics);
}

Rect Panel::local_bounds() const noexcept {
    const Rect arranged = committed_arranged_bounds();
    return {0.0, 0.0, arranged.width, arranged.height};
}

void Panel::paint_panel(Painter& painter, Rect bounds) const {
    const BasicControlStyle& colors = style();
    if (!background_override_ && !style_override_) {
        paint_surface_material(
            painter, bounds,
            effective_theme().resolve(visual_role_, visual_context()).material);
        if (border_style_ == BorderStyle::none) return;
    } else {
        painter.fill_rect(bounds, background());
    }
    switch (border_style_) {
    case BorderStyle::none:
        break;
    case BorderStyle::line:
        painter.stroke_rect({0.5, 0.5, std::max(0.0, bounds.width - 1.0),
                             std::max(0.0, bounds.height - 1.0)},
                            colors.border, 1.0);
        break;
    case BorderStyle::sunken:
        painter.draw_line({0.0, 0.0}, {bounds.width, 0.0}, colors.dark_border, 1.0);
        painter.draw_line({0.0, 0.0}, {0.0, bounds.height}, colors.dark_border, 1.0);
        painter.draw_line({0.0, bounds.height - 1.0},
                          {bounds.width, bounds.height - 1.0}, colors.highlight, 1.0);
        painter.draw_line({bounds.width - 1.0, 0.0},
                          {bounds.width - 1.0, bounds.height}, colors.highlight, 1.0);
        break;
    case BorderStyle::raised:
        painter.draw_line({0.0, 0.0}, {bounds.width, 0.0}, colors.highlight, 1.0);
        painter.draw_line({0.0, 0.0}, {0.0, bounds.height}, colors.highlight, 1.0);
        painter.draw_line({0.0, bounds.height - 1.0},
                          {bounds.width, bounds.height - 1.0}, colors.dark_border, 1.0);
        painter.draw_line({bounds.width - 1.0, 0.0},
                          {bounds.width - 1.0, bounds.height}, colors.dark_border, 1.0);
        break;
    }
}

Insets Panel::visual_outsets() const noexcept {
    if (background_override_ || style_override_) return {};
    return surface_material_visual_outsets(
        effective_theme().resolve(visual_role_, visual_context()).material);
}

void Panel::on_paint(Painter& painter, Rect) {
    paint_panel(painter, local_bounds());
}

SemanticDescriptor Panel::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::group;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    descriptor.exposed = !descriptor.name.empty() ||
                         !descriptor.description.empty();
    return descriptor;
}

} // namespace gui_forms
