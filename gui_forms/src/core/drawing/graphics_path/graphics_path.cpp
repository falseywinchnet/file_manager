#include "../support/drawing_support.hpp"

GraphicsPath::GraphicsPath(FillMode fill_mode) : fill_mode_(fill_mode) {
    require_enum(fill_mode, 1U, "path fill mode");
}

FillMode GraphicsPath::fill_mode() const {
    require_alive();
    return fill_mode_;
}

void GraphicsPath::set_fill_mode(FillMode fill_mode) {
    require_alive();
    require_enum(fill_mode, 1U, "path fill mode");
    fill_mode_ = fill_mode;
}

void GraphicsPath::reset() {
    require_alive();
    elements_.clear();
}

void GraphicsPath::start_figure() {
    require_alive();
    append({PathVerb::start_figure});
}

void GraphicsPath::close_figure() {
    require_alive();
    append({PathVerb::close_figure});
}

void GraphicsPath::add_line(PointF from, PointF to) {
    require_alive();
    require_finite(from, "path line start");
    require_finite(to, "path line end");
    PathElement element;
    element.verb = PathVerb::line;
    element.first = from;
    element.second = to;
    append(std::move(element));
}

void GraphicsPath::add_quadratic(PointF from, PointF control, PointF to) {
    require_alive();
    require_finite(from, "path quadratic start");
    require_finite(control, "path quadratic control");
    require_finite(to, "path quadratic end");
    PathElement element;
    element.verb = PathVerb::quadratic;
    element.first = from;
    element.second = control;
    element.third = to;
    append(std::move(element));
}

void GraphicsPath::add_bezier(PointF from, PointF control1,
                              PointF control2, PointF to) {
    require_alive();
    require_finite(from, "path bezier start");
    require_finite(control1, "path bezier first control");
    require_finite(control2, "path bezier second control");
    require_finite(to, "path bezier end");
    PathElement element;
    element.verb = PathVerb::bezier;
    element.first = from;
    element.second = control1;
    element.third = control2;
    element.fourth = to;
    append(std::move(element));
}

void GraphicsPath::add_beziers(std::span<const PointF> points) {
    require_alive();
    if (points.size() < 4U || (points.size() - 1U) % 3U != 0U) {
        throw std::invalid_argument(
            "path bezier sequence requires 4 + 3n points");
    }
    const std::size_t count = (points.size() - 1U) / 3U;
    if (count > maximum_elements - elements_.size()) {
        throw std::length_error("GUI.Drawing path element limit exceeded");
    }
    for (const PointF point : points) require_finite(point, "path bezier point");
    std::vector<PathElement> appended;
    appended.reserve(count);
    for (std::size_t index = 0U; index < count; ++index) {
        PathElement element;
        element.verb = PathVerb::bezier;
        element.first = points[index * 3U];
        element.second = points[index * 3U + 1U];
        element.third = points[index * 3U + 2U];
        element.fourth = points[index * 3U + 3U];
        appended.push_back(element);
    }
    elements_.insert(elements_.end(), appended.begin(), appended.end());
}

void GraphicsPath::add_polygon(std::span<const PointF> points) {
    require_alive();
    if (points.size() < 3U) {
        throw std::invalid_argument("path polygon requires at least three points");
    }
    // start + (n - 1) edges + closing edge + close verb = n + 2.
    if (points.size() + 2U > maximum_elements - elements_.size()) {
        throw std::length_error("GUI.Drawing path element limit exceeded");
    }
    for (const PointF point : points) require_finite(point, "path polygon point");
    std::vector<PathElement> appended;
    appended.reserve(points.size() + 2U);
    appended.push_back({PathVerb::start_figure});
    for (std::size_t index = 1U; index < points.size(); ++index) {
        PathElement line;
        line.verb = PathVerb::line;
        line.first = points[index - 1U];
        line.second = points[index];
        appended.push_back(line);
    }
    PathElement closing;
    closing.verb = PathVerb::line;
    closing.first = points.back();
    closing.second = points.front();
    appended.push_back(closing);
    appended.push_back({PathVerb::close_figure});
    elements_.insert(elements_.end(), appended.begin(), appended.end());
}

void GraphicsPath::add_rectangle(RectF rectangle) {
    require_alive();
    require_finite(rectangle, "path rectangle");
    PathElement element;
    element.verb = PathVerb::rectangle;
    element.rect = rectangle;
    append(std::move(element));
}

void GraphicsPath::add_ellipse(RectF bounds) {
    require_alive();
    require_finite(bounds, "path ellipse");
    PathElement element;
    element.verb = PathVerb::ellipse;
    element.rect = bounds;
    append(std::move(element));
}

void GraphicsPath::add_arc(RectF bounds, double start_angle,
                           double sweep_angle) {
    require_alive();
    require_finite(bounds, "path arc bounds");
    require_finite(start_angle, "path arc start angle");
    require_finite(sweep_angle, "path arc sweep angle");
    if (bounds.empty() || sweep_angle == 0.0 ||
        std::abs(start_angle) > 1'000'000.0 ||
        std::abs(sweep_angle) > 1'000'000.0) {
        throw std::invalid_argument("path arc requires bounded nonempty geometry");
    }
    PathElement element;
    element.verb = PathVerb::arc;
    element.rect = bounds;
    element.start_angle = start_angle;
    element.sweep_angle = sweep_angle;
    append(std::move(element));
}

void GraphicsPath::add_pie(RectF bounds, double start_angle,
                           double sweep_angle) {
    require_alive();
    // Validate atomically before appending the five-element closed figure.
    require_finite(bounds, "path pie bounds");
    require_finite(start_angle, "path pie start angle");
    require_finite(sweep_angle, "path pie sweep angle");
    if (bounds.empty() || sweep_angle == 0.0 ||
        std::abs(start_angle) > 1'000'000.0 ||
        std::abs(sweep_angle) > 1'000'000.0) {
        throw std::invalid_argument("path pie requires bounded nonempty geometry");
    }
    if (5U > maximum_elements - elements_.size()) {
        throw std::length_error("GUI.Drawing path element limit exceeded");
    }
    constexpr double degrees_to_radians = 0.01745329251994329576923690768489;
    const PointF center{bounds.x + bounds.width * 0.5,
                        bounds.y + bounds.height * 0.5};
    const auto at = [&](double angle) {
        const double radians = angle * degrees_to_radians;
        return PointF{center.x + bounds.width * 0.5 * std::cos(radians),
                      center.y + bounds.height * 0.5 * std::sin(radians)};
    };
    const PointF start = at(start_angle);
    const PointF end = at(start_angle + sweep_angle);
    PathElement radial_start{PathVerb::line};
    radial_start.first = center;
    radial_start.second = start;
    PathElement arc{PathVerb::arc};
    arc.rect = bounds;
    arc.start_angle = start_angle;
    arc.sweep_angle = sweep_angle;
    PathElement radial_end{PathVerb::line};
    radial_end.first = end;
    radial_end.second = center;
    elements_.push_back({PathVerb::start_figure});
    elements_.push_back(radial_start);
    elements_.push_back(arc);
    elements_.push_back(radial_end);
    elements_.push_back({PathVerb::close_figure});
}

void GraphicsPath::add_path(const GraphicsPath& path, bool connect) {
    require_alive();
    const PathSnapshot source = path.snapshot();
    if (source.elements.empty()) return;
    const std::vector<PointF> left = connect && !elements_.empty() ?
        path_points() : std::vector<PointF>{};
    const std::vector<PointF> right = connect && !elements_.empty() ?
        path.path_points() : std::vector<PointF>{};
    const bool add_connector = !left.empty() && !right.empty();
    const std::size_t extra = source.elements.size() + (add_connector ? 1U : 0U);
    if (extra > maximum_elements - elements_.size()) {
        throw std::length_error("GUI.Drawing path element limit exceeded");
    }
    std::vector<PathElement> replacement = elements_;
    replacement.reserve(elements_.size() + extra);
    if (add_connector) {
        PathElement connector;
        connector.verb = PathVerb::line;
        connector.first = left.back();
        connector.second = right.front();
        replacement.push_back(connector);
    }
    replacement.insert(replacement.end(), source.elements.begin(), source.elements.end());
    elements_.swap(replacement);
}

void GraphicsPath::transform(const Matrix& matrix) {
    require_alive();
    if (!matrix.finite()) {
        throw std::invalid_argument("path transform must be finite");
    }
    std::vector<PathElement> transformed = elements_;
    for (PathElement& element : transformed) {
        switch (element.verb) {
        case PathVerb::line:
            element.first = matrix.transform(element.first);
            element.second = matrix.transform(element.second);
            break;
        case PathVerb::quadratic:
            element.first = matrix.transform(element.first);
            element.second = matrix.transform(element.second);
            element.third = matrix.transform(element.third);
            break;
        case PathVerb::bezier:
            element.first = matrix.transform(element.first);
            element.second = matrix.transform(element.second);
            element.third = matrix.transform(element.third);
            element.fourth = matrix.transform(element.fourth);
            break;
        case PathVerb::rectangle:
        case PathVerb::ellipse:
        case PathVerb::arc:
            element.rect = matrix.transform_bounds(element.rect);
            break;
        case PathVerb::start_figure:
        case PathVerb::close_figure:
            break;
        }
    }
    elements_ = std::move(transformed);
}

bool GraphicsPath::is_visible(PointF point) const {
    require_alive();
    require_finite(point, "path visibility point");
    return path_snapshot_visible(snapshot(), point);
}

std::vector<PointF> GraphicsPath::path_points() const {
    require_alive();
    std::vector<PointF> result;
    result.reserve(elements_.size() * 2U);
    constexpr double degrees_to_radians = 0.017453292519943295769;
    for (const PathElement& element : elements_) {
        switch (element.verb) {
        case PathVerb::line:
            if (result.empty() || result.back() != element.first) {
                result.push_back(element.first);
            }
            result.push_back(element.second);
            break;
        case PathVerb::quadratic:
            if (result.empty() || result.back() != element.first) {
                result.push_back(element.first);
            }
            result.push_back(element.second);
            result.push_back(element.third);
            break;
        case PathVerb::bezier:
            if (result.empty() || result.back() != element.first) {
                result.push_back(element.first);
            }
            result.push_back(element.second);
            result.push_back(element.third);
            result.push_back(element.fourth);
            break;
        case PathVerb::rectangle:
            result.insert(result.end(), {{element.rect.left(), element.rect.top()},
                {element.rect.right(), element.rect.top()},
                {element.rect.right(), element.rect.bottom()},
                {element.rect.left(), element.rect.bottom()}});
            break;
        case PathVerb::ellipse: {
            // The compatibility API exposes four cubic ellipse segments through
            // PathPoints:
            // the right-middle start, three points per quadrant, and the
            // repeated closing point. Keep that public geometry rather than a
            // four-cardinal approximation.
            constexpr double kappa = 0.55228474983079339840;
            const double center_x = element.rect.x + element.rect.width / 2.0;
            const double center_y = element.rect.y + element.rect.height / 2.0;
            const double radius_x = element.rect.width / 2.0;
            const double radius_y = element.rect.height / 2.0;
            const double control_x = radius_x * kappa;
            const double control_y = radius_y * kappa;
            result.insert(result.end(), {
                {center_x + radius_x, center_y},
                {center_x + radius_x, center_y + control_y},
                {center_x + control_x, center_y + radius_y},
                {center_x, center_y + radius_y},
                {center_x - control_x, center_y + radius_y},
                {center_x - radius_x, center_y + control_y},
                {center_x - radius_x, center_y},
                {center_x - radius_x, center_y - control_y},
                {center_x - control_x, center_y - radius_y},
                {center_x, center_y - radius_y},
                {center_x + control_x, center_y - radius_y},
                {center_x + radius_x, center_y - control_y},
                {center_x + radius_x, center_y},
            });
            break;
        }
        case PathVerb::arc: {
            const double center_x = element.rect.x + element.rect.width / 2.0;
            const double center_y = element.rect.y + element.rect.height / 2.0;
            const double radius_x = element.rect.width / 2.0;
            const double radius_y = element.rect.height / 2.0;
            const auto parameter_angle = [&](double geometric_degrees) {
                const double geometric = geometric_degrees * degrees_to_radians;
                return std::atan2(radius_x * std::sin(geometric),
                                  radius_y * std::cos(geometric));
            };
            constexpr double two_pi = 6.283185307179586476925286766559;
            constexpr double half_pi = 1.5707963267948966192313216916398;
            const double sweep = std::clamp(element.sweep_angle, -360.0, 360.0);
            double start = parameter_angle(element.start_angle);
            double finish = parameter_angle(element.start_angle + sweep);
            if (sweep > 0.0) {
                while (finish <= start) finish += two_pi;
                if (sweep == 360.0) finish = start + two_pi;
            } else {
                while (finish >= start) finish -= two_pi;
                if (sweep == -360.0) finish = start - two_pi;
            }
            const auto point_at = [&](double parameter) {
                return PointF{center_x + radius_x * std::cos(parameter),
                              center_y + radius_y * std::sin(parameter)};
            };
            result.push_back(point_at(start));
            while ((sweep > 0.0 && start < finish) ||
                   (sweep < 0.0 && start > finish)) {
                const double delta = sweep > 0.0
                    ? std::min(half_pi, finish - start)
                    : std::max(-half_pi, finish - start);
                const double end = start + delta;
                const double alpha = 4.0 / 3.0 * std::tan(delta / 4.0);
                const PointF first = point_at(start);
                const PointF last = point_at(end);
                result.push_back({first.x - alpha * radius_x * std::sin(start),
                                  first.y + alpha * radius_y * std::cos(start)});
                result.push_back({last.x + alpha * radius_x * std::sin(end),
                                  last.y - alpha * radius_y * std::cos(end)});
                result.push_back(last);
                start = end;
            }
            break;
        }
        case PathVerb::start_figure:
        case PathVerb::close_figure:
            break;
        }
    }
    return result;
}

std::unique_ptr<GraphicsPath> GraphicsPath::clone() const {
    require_alive();
    auto result = std::make_unique<GraphicsPath>(fill_mode_);
    result->elements_ = elements_;
    return result;
}

RectF GraphicsPath::bounds() const {
    require_alive();
    bool any = false;
    double left{};
    double top{};
    double right{};
    double bottom{};
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
    const auto include_rect = [&](RectF value) {
        include_point({value.left(), value.top()});
        include_point({value.right(), value.bottom()});
    };
    for (const PathElement& element : elements_) {
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
            include_rect(element.rect);
            break;
        case PathVerb::start_figure:
        case PathVerb::close_figure:
            break;
        }
    }
    return any ? RectF{left, top, right - left, bottom - top} : RectF{};
}

PathSnapshot GraphicsPath::snapshot() const {
    require_alive();
    return {fill_mode_, elements_};
}

void GraphicsPath::append(PathElement element) {
    if (elements_.size() >= maximum_elements) {
        throw std::length_error("GUI.Drawing path element limit exceeded");
    }
    elements_.push_back(std::move(element));
}


} // namespace gui_drawing

