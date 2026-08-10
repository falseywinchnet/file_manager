#include "gui_forms/theme.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace gui_forms {
namespace {

bool finite(Point point) noexcept {
    return std::isfinite(point.x) && std::isfinite(point.y);
}

bool finite(Size size) noexcept {
    return std::isfinite(size.width) && std::isfinite(size.height);
}

bool finite(Insets insets) noexcept {
    return std::isfinite(insets.left) && std::isfinite(insets.top) &&
           std::isfinite(insets.right) && std::isfinite(insets.bottom);
}

bool bounded_coordinate(Point point, MaterialCoordinateSpace space) noexcept {
    if (!finite(point)) return false;
    const double bound = space == MaterialCoordinateSpace::normalized
        ? 16.0 : 1'000'000.0;
    return std::abs(point.x) <= bound && std::abs(point.y) <= bound;
}

bool valid_material_border(const MaterialBorder& border) noexcept {
    return std::isfinite(border.width) && border.width > 0.0 &&
           border.width <= 64.0;
}

Point resolve_point(Point point, MaterialCoordinateSpace space,
                    Rect bounds) noexcept {
    if (space == MaterialCoordinateSpace::normalized) {
        return {bounds.x + point.x * bounds.width,
                bounds.y + point.y * bounds.height};
    }
    return {bounds.x + point.x, bounds.y + point.y};
}

Size resolve_radii(Size radii, MaterialCoordinateSpace space,
                   Rect bounds) noexcept {
    if (space == MaterialCoordinateSpace::normalized) {
        return {radii.width * bounds.width, radii.height * bounds.height};
    }
    return radii;
}

std::pair<Point, Point> resolve_css_linear_gradient(
    double angle_degrees, Rect bounds) noexcept {
    double angle = std::fmod(angle_degrees, 360.0);
    if (angle < 0.0) angle += 360.0;
    Point direction;
    constexpr double epsilon = 1.0e-12;
    if (std::abs(angle) < epsilon || std::abs(angle - 360.0) < epsilon) {
        direction = {0.0, -1.0};
    } else if (std::abs(angle - 90.0) < epsilon) {
        direction = {1.0, 0.0};
    } else if (std::abs(angle - 180.0) < epsilon) {
        direction = {0.0, 1.0};
    } else if (std::abs(angle - 270.0) < epsilon) {
        direction = {-1.0, 0.0};
    } else {
        constexpr double degrees_to_radians =
            3.14159265358979323846264338327950288 / 180.0;
        const double radians = angle * degrees_to_radians;
        direction = {std::sin(radians), -std::cos(radians)};
    }
    const Point center{bounds.x + bounds.width * 0.5,
                       bounds.y + bounds.height * 0.5};
    const double half_length =
        (std::abs(direction.x) * bounds.width +
         std::abs(direction.y) * bounds.height) * 0.5;
    return {
        {center.x - direction.x * half_length,
         center.y - direction.y * half_length},
        {center.x + direction.x * half_length,
         center.y + direction.y * half_length},
    };
}

void paint_stretched_image(Painter& painter, Rect bounds,
                           const MaterialFillLayer& fill) {
    painter.draw_image_region(
        fill.image, {0.0, 0.0, fill.image_pixel_size.width,
                     fill.image_pixel_size.height},
        bounds, fill.opacity);
}

void paint_tiled_image(Painter& painter, Rect bounds,
                       const MaterialFillLayer& fill) {
    const double tile_width = fill.image_pixel_size.width / fill.image_scale;
    const double tile_height = fill.image_pixel_size.height / fill.image_scale;
    painter.fill_image_pattern(
        fill.image, fill.image_pixel_size, bounds,
        {tile_width, tile_height}, ImagePatternWrap::tile, fill.opacity);
}

void paint_nine_patch(Painter& painter, Rect bounds,
                      const MaterialFillLayer& fill) {
    const Insets source = fill.image_slice;
    double destination_left = source.left / fill.image_scale;
    double destination_right = source.right / fill.image_scale;
    double destination_top = source.top / fill.image_scale;
    double destination_bottom = source.bottom / fill.image_scale;
    const double horizontal = destination_left + destination_right;
    if (horizontal > bounds.width && horizontal > 0.0) {
        const double factor = bounds.width / horizontal;
        destination_left *= factor;
        destination_right *= factor;
    }
    const double vertical = destination_top + destination_bottom;
    if (vertical > bounds.height && vertical > 0.0) {
        const double factor = bounds.height / vertical;
        destination_top *= factor;
        destination_bottom *= factor;
    }

    const std::array<double, 4> source_x{
        0.0, source.left,
        fill.image_pixel_size.width - source.right,
        fill.image_pixel_size.width};
    const std::array<double, 4> source_y{
        0.0, source.top,
        fill.image_pixel_size.height - source.bottom,
        fill.image_pixel_size.height};
    const std::array<double, 4> destination_x{
        bounds.x, bounds.x + destination_left,
        bounds.right() - destination_right, bounds.right()};
    const std::array<double, 4> destination_y{
        bounds.y, bounds.y + destination_top,
        bounds.bottom() - destination_bottom, bounds.bottom()};
    for (std::size_t row = 0; row < 3U; ++row) {
        for (std::size_t column = 0; column < 3U; ++column) {
            const Rect source_rect{
                source_x[column], source_y[row],
                source_x[column + 1U] - source_x[column],
                source_y[row + 1U] - source_y[row]};
            const Rect destination_rect{
                destination_x[column], destination_y[row],
                destination_x[column + 1U] - destination_x[column],
                destination_y[row + 1U] - destination_y[row]};
            if (!source_rect.empty() && !destination_rect.empty()) {
                painter.draw_image_region(fill.image, source_rect,
                                          destination_rect, fill.opacity);
            }
        }
    }
}

bool valid_spacing_value(double value) noexcept {
    return std::isfinite(value) && value >= 0.0 && value <= 4096.0;
}

bool valid_geometry_value(double value) noexcept {
    return std::isfinite(value) && value > 0.0 && value <= 16384.0;
}

bool valid_motion_duration(std::chrono::milliseconds duration) noexcept {
    return duration.count() > 0 && duration.count() <= 60'000;
}

bool valid_structure(const ThemeStructureTokens& structure) noexcept {
    const ThemeSpacingTokens& spacing = structure.spacing;
    const double spacing_values[] = {
        spacing.micro, spacing.xsmall, spacing.small, spacing.medium,
        spacing.large, spacing.xlarge, spacing.section};
    if (!std::all_of(std::begin(spacing_values), std::end(spacing_values),
                     &valid_spacing_value) ||
        !(spacing.micro <= spacing.xsmall &&
          spacing.xsmall <= spacing.small &&
          spacing.small <= spacing.medium &&
          spacing.medium <= spacing.large &&
          spacing.large <= spacing.xlarge &&
          spacing.xlarge <= spacing.section)) {
        return false;
    }

    const ThemeGeometryTokens& geometry = structure.geometry;
    const double geometry_values[] = {
        geometry.compact_control_height, geometry.control_height,
        geometry.large_control_height, geometry.minimum_touch_target,
        geometry.splitter_width, geometry.splitter_hit_width,
        geometry.navigation_extent, geometry.navigation_minimum,
        geometry.content_minimum, geometry.compact_breakpoint};
    if (!std::all_of(std::begin(geometry_values), std::end(geometry_values),
                     &valid_geometry_value) ||
        !(geometry.compact_control_height <= geometry.control_height &&
          geometry.control_height <= geometry.large_control_height &&
          geometry.splitter_width <= geometry.splitter_hit_width &&
          geometry.navigation_minimum <= geometry.navigation_extent)) {
        return false;
    }

    const ThemeTypographyTokens& typography = structure.typography;
    if (!valid_font_spec(typography.control) ||
        !valid_font_spec(typography.field) ||
        !valid_font_spec(typography.caption) ||
        !valid_font_spec(typography.heading) ||
        !valid_font_spec(typography.title) ||
        !valid_font_spec(typography.monospace)) {
        return false;
    }

    const ThemeMotionTokens& motion = structure.motion;
    return valid_motion_duration(motion.quick) &&
           valid_motion_duration(motion.standard) &&
           valid_motion_duration(motion.emphasized) &&
           valid_motion_duration(motion.busy_cycle) &&
           motion.quick <= motion.standard &&
           motion.standard <= motion.emphasized;
}

SurfaceMaterial surface(Color top, Color bottom, Color border,
                        double radius = 3.0) {
    SurfaceMaterial result;
    if (top == bottom) {
        result.fills = {MaterialFillLayer::solid(top)};
    } else {
        result.fills = {MaterialFillLayer::linear(
            {0.0, 0.0}, {0.0, 1.0},
            {{0.0, top}, {1.0, bottom}})};
    }
    result.border = MaterialBorder{border, 1.0};
    result.corner_radius = radius;
    return result;
}

ControlVisualRecipe recipe(Color top, Color bottom, Color border, Color text,
                           double radius = 3.0) {
    ControlVisualRecipe result;
    result.material = surface(top, bottom, border, radius);
    result.text = text;
    result.muted_text = Color::rgba(118, 128, 138);
    result.glyph = text;
    result.focus_ring = Color::rgba(31, 104, 185);
    result.default_ring = Color::rgba(20, 75, 145);
    return result;
}

ControlRoleRecipes professional_role(Color top, Color bottom, Color border,
                                     Color text, double radius = 3.0) {
    ControlRoleRecipes result;
    std::array<ControlVisualRecipe, control_surface_state_count>& values = result.ordinary;
    values[static_cast<std::size_t>(ControlSurfaceState::normal)] =
        recipe(top, bottom, border, text, radius);
    values[static_cast<std::size_t>(ControlSurfaceState::hot)] =
        recipe(Color::rgba(255, 255, 255), Color::rgba(213, 233, 250),
               Color::rgba(72, 139, 196), text, radius);
    values[static_cast<std::size_t>(ControlSurfaceState::pressed)] =
        recipe(Color::rgba(188, 218, 242), Color::rgba(225, 240, 251),
               Color::rgba(46, 109, 166), text, radius);
    values[static_cast<std::size_t>(ControlSurfaceState::pending)] =
        recipe(Color::rgba(255, 249, 218), Color::rgba(246, 229, 157),
               Color::rgba(173, 132, 31), text, radius);
    values[static_cast<std::size_t>(ControlSurfaceState::invalid)] =
        recipe(Color::rgba(255, 239, 239), Color::rgba(247, 207, 207),
               Color::rgba(178, 54, 54), text, radius);
    values[static_cast<std::size_t>(ControlSurfaceState::disabled)] =
        recipe(Color::rgba(240, 242, 244), Color::rgba(226, 229, 232),
               Color::rgba(184, 190, 196), Color::rgba(132, 143, 153), radius);
    values[static_cast<std::size_t>(ControlSurfaceState::deactivated)] =
        recipe(Color::rgba(244, 246, 248), Color::rgba(224, 228, 232),
               Color::rgba(161, 169, 176), Color::rgba(86, 96, 106), radius);

    result.selected = values;
    const std::array<Color, control_surface_state_count> selected_top{
        Color::rgba(225, 239, 255), Color::rgba(236, 247, 255),
        Color::rgba(166, 205, 242), Color::rgba(255, 242, 190),
        Color::rgba(255, 221, 221), Color::rgba(222, 226, 230),
        Color::rgba(218, 225, 232)};
    const std::array<Color, control_surface_state_count> selected_bottom{
        Color::rgba(188, 216, 247), Color::rgba(199, 226, 250),
        Color::rgba(205, 228, 247), Color::rgba(239, 214, 127),
        Color::rgba(244, 188, 188), Color::rgba(205, 211, 216),
        Color::rgba(198, 207, 216)};
    for (std::size_t index = 0; index < result.selected.size(); ++index) {
        ControlVisualRecipe& selected = result.selected[index];
        selected.material.fills = {MaterialFillLayer::linear(
            {0.0, 0.0}, {0.0, 1.0},
            {{0.0, selected_top[index]},
             {1.0, selected_bottom[index]}})};
        selected.material.border =
            MaterialBorder{Color::rgba(57, 117, 178), 1.0};
    }

    const ControlVisualRecipe hc_normal = recipe(
        Color::rgba(0, 0, 0), Color::rgba(0, 0, 0),
        Color::rgba(255, 255, 255), Color::rgba(255, 255, 255), radius);
    const ControlVisualRecipe hc_hot = recipe(
        Color::rgba(0, 0, 0), Color::rgba(0, 0, 0),
        Color::rgba(0, 255, 255), Color::rgba(255, 255, 255), radius);
    const ControlVisualRecipe hc_pressed = recipe(
        Color::rgba(255, 255, 255), Color::rgba(255, 255, 255),
        Color::rgba(255, 255, 0), Color::rgba(0, 0, 0), radius);
    result.high_contrast.fill(hc_normal);
    result.high_contrast[static_cast<std::size_t>(ControlSurfaceState::hot)] =
        hc_hot;
    result.high_contrast[static_cast<std::size_t>(ControlSurfaceState::pressed)] =
        hc_pressed;
    result.high_contrast[static_cast<std::size_t>(ControlSurfaceState::pending)] =
        hc_hot;
    result.high_contrast[static_cast<std::size_t>(ControlSurfaceState::invalid)] =
        recipe(Color::rgba(0, 0, 0), Color::rgba(0, 0, 0),
               Color::rgba(255, 96, 96), Color::rgba(255, 255, 255), radius);
    result.high_contrast[static_cast<std::size_t>(ControlSurfaceState::disabled)] =
        recipe(Color::rgba(0, 0, 0), Color::rgba(0, 0, 0),
               Color::rgba(128, 128, 128), Color::rgba(160, 160, 160), radius);
    result.high_contrast_selected = result.high_contrast;
    for (ControlVisualRecipe& selected : result.high_contrast_selected) {
        const Color state_border = selected.material.border
            ? (*selected.material.border).color : Color::rgba(255, 255, 0);
        selected.material = surface(Color::rgba(255, 255, 255),
                                    Color::rgba(255, 255, 255),
                                    state_border, radius);
        selected.text = Color::rgba(0, 0, 0);
        selected.glyph = Color::rgba(0, 0, 0);
    }
    return result;
}

bool valid_recipe(const ControlVisualRecipe& value) noexcept {
    return valid_surface_material(value.material) &&
           std::isfinite(value.focus_width) && value.focus_width >= 0.0 &&
           value.focus_width <= 16.0 && std::isfinite(value.focus_offset) &&
           value.focus_offset >= 0.0 && value.focus_offset <= 64.0 &&
           (value.focus_external || value.focus_offset == 0.0) &&
           std::isfinite(value.default_width) &&
           value.default_width >= 0.0 && value.default_width <= 16.0 &&
           finite(value.visual_offset) &&
           std::abs(value.visual_offset.x) <= 32.0 &&
           std::abs(value.visual_offset.y) <= 32.0 &&
           finite(value.pressed_content_offset) &&
           std::abs(value.pressed_content_offset.x) <= 32.0 &&
           std::abs(value.pressed_content_offset.y) <= 32.0;
}

} // namespace

bool valid_control_visual_recipe(
    const ControlVisualRecipe& value) noexcept {
    return valid_recipe(value);
}

bool valid_control_state_recipes(
    const ControlStateRecipes& value) noexcept {
    return std::all_of(value.values.begin(), value.values.end(),
                       &valid_recipe);
}

ControlStateRecipes ControlStateRecipes::from_parts(
    const ControlVisualRecipe* recipe_values, std::size_t value_count) {
    if (recipe_values == nullptr ||
        value_count != control_surface_state_count) {
        throw std::invalid_argument(
            "control state recipes require one nonnull recipe per retained state");
    }
    ControlStateRecipes result;
    std::copy_n(recipe_values, value_count, result.values.begin());
    if (!valid_control_state_recipes(result)) {
        throw std::invalid_argument("control state recipes contain an invalid recipe");
    }
    return result;
}

bool valid_surface_material(const SurfaceMaterial& material) noexcept {
    if (material.fills.empty() ||
        material.fills.size() > SurfaceMaterial::maximum_fill_layers ||
        material.shadows.size() > SurfaceMaterial::maximum_shadows ||
        !std::isfinite(material.corner_radius) ||
        material.corner_radius < 0.0 || material.corner_radius > 4096.0) {
        return false;
    }
    if (material.border && !valid_material_border(*material.border)) {
        return false;
    }
    if (material.border && !material.border_edges.empty()) return false;
    if (!material.border_edges.empty() && material.corner_radius != 0.0) {
        return false;
    }
    const std::optional<MaterialBorder>* edge_borders[]{
        &material.border_edges.top,
        &material.border_edges.right,
        &material.border_edges.bottom,
        &material.border_edges.left,
    };
    for (const std::optional<MaterialBorder>* edge : edge_borders) {
        if (*edge && !valid_material_border(**edge)) return false;
    }
    for (const MaterialShadow& shadow : material.shadows) {
        if (!finite(shadow.offset) || !std::isfinite(shadow.blur_radius) ||
            !std::isfinite(shadow.spread) || shadow.blur_radius < 0.0 ||
            shadow.blur_radius > 512.0 || std::abs(shadow.spread) > 4096.0 ||
            std::abs(shadow.offset.x) > 4096.0 ||
            std::abs(shadow.offset.y) > 4096.0) {
            return false;
        }
    }
    for (const MaterialFillLayer& fill : material.fills) {
        if (fill.coordinate_space != MaterialCoordinateSpace::normalized &&
            fill.coordinate_space != MaterialCoordinateSpace::logical) {
            return false;
        }
        switch (fill.kind) {
        case MaterialFillKind::solid:
            if (fill.linear_geometry != MaterialLinearGeometry::endpoints ||
                fill.spread != GradientSpreadMode::pad) return false;
            break;
        case MaterialFillKind::linear_gradient:
            if (fill.spread != GradientSpreadMode::pad &&
                fill.spread != GradientSpreadMode::repeat &&
                fill.spread != GradientSpreadMode::reflect) {
                return false;
            }
            if (fill.linear_geometry != MaterialLinearGeometry::endpoints &&
                fill.linear_geometry != MaterialLinearGeometry::css_angle) {
                return false;
            }
            if (fill.linear_geometry == MaterialLinearGeometry::css_angle) {
                if (fill.coordinate_space != MaterialCoordinateSpace::normalized ||
                    !std::isfinite(fill.angle_degrees) ||
                    std::abs(fill.angle_degrees) > 360'000.0 ||
                    !valid_gradient_stops(fill.stops)) return false;
            } else if (!bounded_coordinate(fill.start, fill.coordinate_space) ||
                       !bounded_coordinate(fill.end, fill.coordinate_space) ||
                       fill.start == fill.end ||
                       !valid_gradient_stops(fill.stops)) {
                return false;
            }
            break;
        case MaterialFillKind::radial_gradient: {
            if (fill.linear_geometry != MaterialLinearGeometry::endpoints ||
                fill.spread != GradientSpreadMode::pad) return false;
            if (!bounded_coordinate(fill.center, fill.coordinate_space) ||
                !finite(fill.radii) || fill.radii.width <= 0.0 ||
                fill.radii.height <= 0.0 ||
                !valid_gradient_stops(fill.stops)) {
                return false;
            }
            const double bound = fill.coordinate_space ==
                    MaterialCoordinateSpace::normalized
                ? 16.0 : 1'000'000.0;
            if (fill.radii.width > bound || fill.radii.height > bound) {
                return false;
            }
            break;
        }
        case MaterialFillKind::image:
            if (fill.linear_geometry != MaterialLinearGeometry::endpoints ||
                fill.image.value == 0U || !finite(fill.image_pixel_size) ||
                fill.image_pixel_size.width <= 0.0 ||
                fill.image_pixel_size.height <= 0.0 ||
                std::floor(fill.image_pixel_size.width) !=
                    fill.image_pixel_size.width ||
                std::floor(fill.image_pixel_size.height) !=
                    fill.image_pixel_size.height ||
                fill.image_pixel_size.width > 32768.0 ||
                fill.image_pixel_size.height > 32768.0 ||
                !finite(fill.image_slice) || fill.image_slice.left < 0.0 ||
                fill.image_slice.top < 0.0 || fill.image_slice.right < 0.0 ||
                fill.image_slice.bottom < 0.0 ||
                fill.image_slice.left + fill.image_slice.right >
                    fill.image_pixel_size.width ||
                fill.image_slice.top + fill.image_slice.bottom >
                    fill.image_pixel_size.height ||
                !std::isfinite(fill.image_scale) || fill.image_scale < 0.125 ||
                fill.image_scale > 8.0 || !std::isfinite(fill.opacity) ||
                fill.opacity < 0.0 || fill.opacity > 1.0) {
                return false;
            }
            if (fill.image_mode != MaterialImageMode::stretch &&
                fill.image_mode != MaterialImageMode::tile &&
                fill.image_mode != MaterialImageMode::nine_patch) {
                return false;
            }
            if (fill.image_mode != MaterialImageMode::nine_patch &&
                fill.image_slice != Insets{}) {
                return false;
            }
            break;
        default:
            return false;
        }
    }
    return true;
}

void paint_surface_material(Painter& painter, Rect bounds,
                            const SurfaceMaterial& material) {
    if (bounds.empty() || !valid_surface_material(material)) return;
    for (const MaterialShadow& shadow : material.shadows) {
        painter.draw_box_shadow(bounds, material.corner_radius, shadow.offset,
                                shadow.blur_radius, shadow.spread, shadow.color);
    }
    const bool clipped = material.corner_radius > 0.0;
    if (clipped) {
        painter.save();
        painter.clip_rounded_rect(bounds, material.corner_radius);
    }
    for (const MaterialFillLayer& fill : material.fills) {
        switch (fill.kind) {
        case MaterialFillKind::solid:
            if (clipped) {
                painter.fill_rounded_rect(bounds, material.corner_radius,
                                          fill.color);
            } else {
                painter.fill_rect(bounds, fill.color);
            }
            break;
        case MaterialFillKind::linear_gradient:
            if (fill.linear_geometry == MaterialLinearGeometry::css_angle) {
                const std::pair<Point, Point> endpoints =
                    resolve_css_linear_gradient(fill.angle_degrees, bounds);
                painter.fill_linear_gradient_spread(
                    bounds, endpoints.first, endpoints.second, fill.stops,
                    fill.spread);
                break;
            }
            painter.fill_linear_gradient_spread(
                bounds, resolve_point(fill.start, fill.coordinate_space, bounds),
                resolve_point(fill.end, fill.coordinate_space, bounds), fill.stops,
                fill.spread);
            break;
        case MaterialFillKind::radial_gradient:
            painter.fill_radial_gradient(
                bounds, resolve_point(fill.center, fill.coordinate_space, bounds),
                resolve_radii(fill.radii, fill.coordinate_space, bounds), fill.stops);
            break;
        case MaterialFillKind::image:
            switch (fill.image_mode) {
            case MaterialImageMode::stretch:
                paint_stretched_image(painter, bounds, fill);
                break;
            case MaterialImageMode::tile:
                paint_tiled_image(painter, bounds, fill);
                break;
            case MaterialImageMode::nine_patch:
                paint_nine_patch(painter, bounds, fill);
                break;
            }
            break;
        }
    }
    if (clipped) painter.restore();
    if (material.border) {
        const double inset = (*material.border).width * 0.5;
        painter.stroke_rounded_rect(
            {bounds.x + inset, bounds.y + inset,
             std::max(0.0, bounds.width - inset * 2.0),
             std::max(0.0, bounds.height - inset * 2.0)},
            std::max(0.0, material.corner_radius - inset),
            (*material.border).color, (*material.border).width);
    }
    if (material.border_edges.top) {
        const MaterialBorder& border = *material.border_edges.top;
        const double y = bounds.y + border.width * 0.5;
        painter.draw_line({bounds.x, y}, {bounds.x + bounds.width, y},
                          border.color, border.width);
    }
    if (material.border_edges.right) {
        const MaterialBorder& border = *material.border_edges.right;
        const double x = bounds.x + bounds.width - border.width * 0.5;
        painter.draw_line({x, bounds.y}, {x, bounds.y + bounds.height},
                          border.color, border.width);
    }
    if (material.border_edges.bottom) {
        const MaterialBorder& border = *material.border_edges.bottom;
        const double y = bounds.y + bounds.height - border.width * 0.5;
        painter.draw_line({bounds.x, y}, {bounds.x + bounds.width, y},
                          border.color, border.width);
    }
    if (material.border_edges.left) {
        const MaterialBorder& border = *material.border_edges.left;
        const double x = bounds.x + border.width * 0.5;
        painter.draw_line({x, bounds.y}, {x, bounds.y + bounds.height},
                          border.color, border.width);
    }
}

Insets surface_material_visual_outsets(const SurfaceMaterial& material) noexcept {
    Insets result{};
    for (const MaterialShadow& shadow : material.shadows) {
        const double extent = std::max(0.0, shadow.spread) +
                              shadow.blur_radius * 3.0;
        result.left = std::max(result.left,
                               std::max(0.0, extent - shadow.offset.x));
        result.top = std::max(result.top,
                              std::max(0.0, extent - shadow.offset.y));
        result.right = std::max(result.right,
                                std::max(0.0, extent + shadow.offset.x));
        result.bottom = std::max(result.bottom,
                                 std::max(0.0, extent + shadow.offset.y));
    }
    return result;
}

std::shared_ptr<const Theme> Theme::create(ThemeDefinition definition) {
    if (definition.id.empty() || definition.id.size() > 128U) {
        throw std::invalid_argument("theme id must contain 1 to 128 bytes");
    }
    for (const ControlRoleRecipes& role : definition.roles) {
        if (!std::all_of(role.ordinary.begin(), role.ordinary.end(),
                         &valid_recipe) ||
            !std::all_of(role.selected.begin(), role.selected.end(),
                         &valid_recipe) ||
            !std::all_of(role.high_contrast.begin(),
                         role.high_contrast.end(), &valid_recipe) ||
            !std::all_of(role.high_contrast_selected.begin(),
                         role.high_contrast_selected.end(), &valid_recipe)) {
            throw std::invalid_argument("theme contains an invalid visual recipe");
        }
    }
    if (!valid_structure(definition.structure)) {
        throw std::invalid_argument(
            "theme structural tokens must be finite, ordered, and bounded");
    }
    return std::shared_ptr<const Theme>(new Theme(std::move(definition)));
}

const ControlVisualRecipe& Theme::resolve(
    ControlVisualRole role, ControlVisualContext context) const noexcept {
    std::size_t role_index = static_cast<std::size_t>(role);
    std::size_t state_index = static_cast<std::size_t>(context.surface);
    if (role_index >= definition_.roles.size()) role_index = 0U;
    if (state_index >= control_surface_state_count) state_index = 0U;
    const ControlRoleRecipes& recipes = definition_.roles[role_index];
    if (context.high_contrast) {
        return context.selected ? recipes.high_contrast_selected[state_index]
                                : recipes.high_contrast[state_index];
    }
    return context.selected ? recipes.selected[state_index]
                            : recipes.ordinary[state_index];
}

ThemeDefinition windows_professional_theme_definition() {
    ThemeDefinition result;
    const Color ink = Color::rgba(27, 39, 51);
    for (ControlRoleRecipes& role : result.roles) {
        role = professional_role(Color::rgba(250, 252, 254),
                                 Color::rgba(225, 231, 237),
                                 Color::rgba(132, 148, 163), ink);
    }
    result.roles[static_cast<std::size_t>(ControlVisualRole::window)] =
        professional_role(Color::rgba(246, 248, 250),
                          Color::rgba(246, 248, 250),
                          Color::rgba(155, 166, 177), ink, 0.0);
    result.roles[static_cast<std::size_t>(ControlVisualRole::panel)] =
        professional_role(Color::rgba(240, 244, 248),
                          Color::rgba(232, 237, 242),
                          Color::rgba(148, 162, 175), ink, 0.0);
    result.roles[static_cast<std::size_t>(ControlVisualRole::card)] =
        professional_role(Color::rgba(255, 255, 255),
                          Color::rgba(244, 247, 250),
                          Color::rgba(151, 166, 179), ink, 6.0);
    result.roles[static_cast<std::size_t>(ControlVisualRole::accent_button)] =
        professional_role(Color::rgba(72, 145, 213),
                          Color::rgba(31, 100, 177),
                          Color::rgba(20, 75, 145), Color::rgba(255, 255, 255));
    result.roles[static_cast<std::size_t>(ControlVisualRole::command_button)] =
        professional_role(Color::rgba(252, 253, 255),
                          Color::rgba(229, 236, 244),
                          Color::rgba(126, 147, 166), ink, 2.0);
    result.roles[static_cast<std::size_t>(ControlVisualRole::editor)] =
        professional_role(Color::rgba(255, 255, 255),
                          Color::rgba(255, 255, 255),
                          Color::rgba(118, 139, 158), ink, 2.0);
    result.roles[static_cast<std::size_t>(ControlVisualRole::menu_item)] =
        professional_role(Color::rgba(255, 255, 255, 0),
                          Color::rgba(255, 255, 255, 0),
                          Color::rgba(148, 162, 175, 0), ink, 2.0);
    result.roles[static_cast<std::size_t>(ControlVisualRole::selection)] =
        professional_role(Color::rgba(255, 255, 255, 0),
                          Color::rgba(255, 255, 255, 0),
                          Color::rgba(148, 162, 175, 0), ink, 0.0);
    result.roles[static_cast<std::size_t>(ControlVisualRole::progress)] =
        professional_role(Color::rgba(238, 242, 246),
                          Color::rgba(220, 227, 234),
                          Color::rgba(118, 139, 158), ink, 2.0);
    ControlRoleRecipes& progress =
        result.roles[static_cast<std::size_t>(ControlVisualRole::progress)];
    for (ControlVisualRecipe& selected : progress.selected) {
        selected.text = Color::rgba(255, 255, 255);
        selected.glyph = Color::rgba(255, 255, 255);
        selected.muted_text = Color::rgba(255, 255, 255, 130);
    }
    return result;
}

std::shared_ptr<const Theme> default_theme() {
    static const std::shared_ptr<const Theme> value =
        Theme::create(windows_professional_theme_definition());
    return value;
}

} // namespace gui_forms
