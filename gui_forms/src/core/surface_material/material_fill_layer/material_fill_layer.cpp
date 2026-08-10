#include "gui_forms/surface_material/material_fill_layer/material_fill_layer.hpp"

#include <utility>

namespace gui_forms {

MaterialFillLayer MaterialFillLayer::solid(Color value) {
    MaterialFillLayer result;
    result.kind = MaterialFillKind::solid;
    result.color = value;
    return result;
}

MaterialFillLayer MaterialFillLayer::linear(
    Point from, Point to, std::vector<GradientStop> values,
    MaterialCoordinateSpace space, GradientSpreadMode spread_mode) {
    MaterialFillLayer result;
    result.kind = MaterialFillKind::linear_gradient;
    result.coordinate_space = space;
    result.start = from;
    result.end = to;
    result.stops = std::move(values);
    result.spread = spread_mode;
    return result;
}

MaterialFillLayer MaterialFillLayer::repeating_linear(
    Point from, Point to, std::vector<GradientStop> values,
    MaterialCoordinateSpace space) {
    return linear(from, to, std::move(values), space,
                  GradientSpreadMode::repeat);
}

MaterialFillLayer MaterialFillLayer::radial(
    Point origin, Size radius, std::vector<GradientStop> values,
    MaterialCoordinateSpace space) {
    MaterialFillLayer result;
    result.kind = MaterialFillKind::radial_gradient;
    result.coordinate_space = space;
    result.center = origin;
    result.radii = radius;
    result.stops = std::move(values);
    result.spread = GradientSpreadMode::pad;
    return result;
}

MaterialFillLayer MaterialFillLayer::stretched_image(
    ImageId value, Size pixel_size, double value_opacity) {
    MaterialFillLayer result;
    result.kind = MaterialFillKind::image;
    result.image = value;
    result.image_pixel_size = pixel_size;
    result.opacity = value_opacity;
    result.image_mode = MaterialImageMode::stretch;
    return result;
}

MaterialFillLayer MaterialFillLayer::tiled_image(
    ImageId value, Size pixel_size, double source_pixels_per_logical_pixel,
    double value_opacity) {
    MaterialFillLayer result = stretched_image(value, pixel_size, value_opacity);
    result.image_scale = source_pixels_per_logical_pixel;
    result.image_mode = MaterialImageMode::tile;
    return result;
}

MaterialFillLayer MaterialFillLayer::nine_patch(
    ImageId value, Size pixel_size, Insets source_slice,
    double source_pixels_per_logical_pixel, double value_opacity) {
    MaterialFillLayer result = stretched_image(value, pixel_size, value_opacity);
    result.image_slice = source_slice;
    result.image_scale = source_pixels_per_logical_pixel;
    result.image_mode = MaterialImageMode::nine_patch;
    return result;
}

} // namespace gui_forms
