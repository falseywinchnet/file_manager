#include "../support/drawing_support.hpp"

void PointI::offset(std::int32_t dx, std::int32_t dy) noexcept {
    x = clamp_i32(static_cast<std::int64_t>(x) + dx);
    y = clamp_i32(static_cast<std::int64_t>(y) + dy);
}

void PointF::offset(double dx, double dy) {
    require_finite(dx, "point offset x");
    require_finite(dy, "point offset y");
    require_finite(*this, "point");
    const PointF result{x + dx, y + dy};
    require_finite(result, "offset point");
    *this = result;
}

bool RectI::contains(PointI point) const noexcept {
    return !empty() && point.x >= left() && point.x < right() &&
           point.y >= top() && point.y < bottom();
}

bool RectI::contains(RectI rect) const noexcept {
    return !empty() && !rect.empty() && rect.left() >= left() &&
           rect.top() >= top() && rect.right() <= right() &&
           rect.bottom() <= bottom();
}

bool RectI::intersects(RectI rect) const noexcept {
    return !empty() && !rect.empty() && left() < rect.right() &&
           rect.left() < right() && top() < rect.bottom() &&
           rect.top() < bottom();
}

void RectI::offset(std::int32_t dx, std::int32_t dy) noexcept {
    x = clamp_i32(static_cast<std::int64_t>(x) + dx);
    y = clamp_i32(static_cast<std::int64_t>(y) + dy);
}

void RectI::inflate(std::int32_t dx, std::int32_t dy) noexcept {
    x = clamp_i32(static_cast<std::int64_t>(x) - dx);
    y = clamp_i32(static_cast<std::int64_t>(y) - dy);
    width = clamp_i32(static_cast<std::int64_t>(width) +
                      static_cast<std::int64_t>(dx) * 2);
    height = clamp_i32(static_cast<std::int64_t>(height) +
                       static_cast<std::int64_t>(dy) * 2);
}

void RectI::intersect(RectI rect) noexcept { *this = intersection(*this, rect); }

RectI RectI::intersection(RectI left, RectI right) noexcept {
    const std::int64_t x1 = std::max(left.left(), right.left());
    const std::int64_t y1 = std::max(left.top(), right.top());
    const std::int64_t x2 = std::min(left.right(), right.right());
    const std::int64_t y2 = std::min(left.bottom(), right.bottom());
    if (x2 <= x1 || y2 <= y1) return {};
    return {clamp_i32(x1), clamp_i32(y1), clamp_i32(x2 - x1), clamp_i32(y2 - y1)};
}

RectI RectI::united(RectI left, RectI right) noexcept {
    if (left.empty()) return right;
    if (right.empty()) return left;
    const std::int64_t x1 = std::min(left.left(), right.left());
    const std::int64_t y1 = std::min(left.top(), right.top());
    const std::int64_t x2 = std::max(left.right(), right.right());
    const std::int64_t y2 = std::max(left.bottom(), right.bottom());
    return {clamp_i32(x1), clamp_i32(y1), clamp_i32(x2 - x1), clamp_i32(y2 - y1)};
}

bool RectF::finite() const noexcept {
    return std::isfinite(x) && std::isfinite(y) &&
           std::isfinite(width) && std::isfinite(height);
}

bool RectF::contains(PointF point) const noexcept {
    return finite() && std::isfinite(point.x) && std::isfinite(point.y) &&
           !empty() && point.x >= left() && point.x < right() &&
           point.y >= top() && point.y < bottom();
}

bool RectF::contains(RectF rect) const noexcept {
    return finite() && rect.finite() && !empty() && !rect.empty() &&
           rect.left() >= left() && rect.top() >= top() &&
           rect.right() <= right() && rect.bottom() <= bottom();
}

bool RectF::intersects(RectF rect) const noexcept {
    return finite() && rect.finite() && !empty() && !rect.empty() &&
           left() < rect.right() && rect.left() < right() &&
           top() < rect.bottom() && rect.top() < bottom();
}

void RectF::offset(double dx, double dy) {
    require_finite(*this, "rectangle");
    require_finite(dx, "rectangle offset x");
    require_finite(dy, "rectangle offset y");
    const RectF result{x + dx, y + dy, width, height};
    require_finite(result, "offset rectangle");
    *this = result;
}

void RectF::inflate(double dx, double dy) {
    require_finite(*this, "rectangle");
    require_finite(dx, "rectangle inflation x");
    require_finite(dy, "rectangle inflation y");
    const RectF result{x - dx, y - dy, width + dx * 2.0,
                       height + dy * 2.0};
    require_finite(result, "inflated rectangle");
    *this = result;
}

void RectF::intersect(RectF rect) noexcept { *this = intersection(*this, rect); }

RectF RectF::intersection(RectF left, RectF right) noexcept {
    if (!left.finite() || !right.finite()) return {};
    const double x1 = std::max(left.left(), right.left());
    const double y1 = std::max(left.top(), right.top());
    const double x2 = std::min(left.right(), right.right());
    const double y2 = std::min(left.bottom(), right.bottom());
    if (x2 <= x1 || y2 <= y1) return {};
    return {x1, y1, x2 - x1, y2 - y1};
}

RectF RectF::united(RectF left, RectF right) noexcept {
    if (!left.finite() || !right.finite()) return {};
    if (left.empty()) return right;
    if (right.empty()) return left;
    const double x1 = std::min(left.left(), right.left());
    const double y1 = std::min(left.top(), right.top());
    const double x2 = std::max(left.right(), right.right());
    const double y2 = std::max(left.bottom(), right.bottom());
    return {x1, y1, x2 - x1, y2 - y1};
}


} // namespace gui_drawing

