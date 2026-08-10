#include "../support/drawing_support.hpp"

Region::Region(RectF rectangle) {
    require_finite(rectangle, "region rectangle");
    if (!rectangle.empty()) value_.rectangles.push_back(rectangle);
}

Region::Region(const GraphicsPath& path) {
    value_.paths.push_back(path.snapshot());
}

void Region::unite(RectF rectangle) {
    require_alive();
    require_finite(rectangle, "region union rectangle");
    if (!rectangle.empty()) value_.rectangles.push_back(rectangle);
}

void Region::unite(const GraphicsPath& path) {
    require_alive();
    value_.paths.push_back(path.snapshot());
}

void Region::exclude(RectF rectangle) {
    require_alive();
    require_finite(rectangle, "region exclusion rectangle");
    if (!rectangle.empty()) value_.exclusions.push_back(rectangle);
}

bool Region::is_visible(PointF point) const {
    require_alive();
    require_finite(point, "region visibility point");
    bool included = false;
    for (const RectF rectangle : value_.rectangles) {
        if (rectangle.contains(point)) {
            included = true;
            break;
        }
    }
    for (const PathSnapshot& path : value_.paths) {
        if (path_snapshot_visible(path, point)) {
            included = true;
            break;
        }
    }
    if (!included) return false;
    for (const RectF rectangle : value_.exclusions) {
        if (rectangle.contains(point)) return false;
    }
    return true;
}

RectF Region::bounds() const {
    require_alive();
    RectBoundsAccumulator accumulator;
    for (const RectF rectangle : value_.rectangles) {
        accumulator.include(rectangle);
    }
    for (const PathSnapshot& path : value_.paths) {
        for (const PathElement& element : path.elements) {
            switch (element.verb) {
            case PathVerb::line:
                accumulator.include(element.first);
                accumulator.include(element.second);
                break;
            case PathVerb::quadratic:
                accumulator.include(element.first);
                accumulator.include(element.second);
                accumulator.include(element.third);
                break;
            case PathVerb::bezier:
                accumulator.include(element.first);
                accumulator.include(element.second);
                accumulator.include(element.third);
                accumulator.include(element.fourth);
                break;
            case PathVerb::rectangle:
            case PathVerb::ellipse:
            case PathVerb::arc:
                accumulator.include(element.rect);
                break;
            case PathVerb::start_figure:
            case PathVerb::close_figure:
                break;
            }
        }
    }
    return accumulator.bounds();
}

RegionSnapshot Region::snapshot() const {
    require_alive();
    return value_;
}


} // namespace gui_drawing
