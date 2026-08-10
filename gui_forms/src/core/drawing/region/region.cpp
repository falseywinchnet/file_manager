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
    const bool included = std::any_of(
        value_.rectangles.begin(), value_.rectangles.end(),
        [point](RectF rectangle) { return rectangle.contains(point); }) ||
        std::any_of(value_.paths.begin(), value_.paths.end(),
                    [point](const PathSnapshot& path) {
                        return path_snapshot_visible(path, point);
                    });
    if (!included) return false;
    return std::none_of(value_.exclusions.begin(), value_.exclusions.end(),
                        [point](RectF rectangle) {
                            return rectangle.contains(point);
                        });
}

RectF Region::bounds() const {
    require_alive();
    bool any = false;
    double left{}, top{}, right{}, bottom{};
    const auto include_point = [&](PointF point) {
        if (!any) {
            left = right = point.x;
            top = bottom = point.y;
            any = true;
            return;
        }
        left = std::min(left, point.x);
        top = std::min(top, point.y);
        right = std::max(right, point.x);
        bottom = std::max(bottom, point.y);
    };
    const auto include = [&](RectF rectangle) {
        if (rectangle.empty()) return;
        include_point({rectangle.left(), rectangle.top()});
        include_point({rectangle.right(), rectangle.bottom()});
    };
    for (const RectF rectangle : value_.rectangles) include(rectangle);
    for (const PathSnapshot& path : value_.paths) {
        for (const PathElement& element : path.elements) {
            switch (element.verb) {
            case PathVerb::line:
                include_point(element.first);
                include_point(element.second);
                break;
            case PathVerb::quadratic:
                include_point(element.first);
                include_point(element.second);
                include_point(element.third);
                break;
            case PathVerb::bezier:
                include_point(element.first);
                include_point(element.second);
                include_point(element.third);
                include_point(element.fourth);
                break;
            case PathVerb::rectangle:
            case PathVerb::ellipse:
            case PathVerb::arc:
                include(element.rect);
                break;
            case PathVerb::start_figure:
            case PathVerb::close_figure:
                break;
            }
        }
    }
    return any ? RectF{left, top, right - left, bottom - top} : RectF{};
}

RegionSnapshot Region::snapshot() const {
    require_alive();
    return value_;
}


} // namespace gui_drawing

