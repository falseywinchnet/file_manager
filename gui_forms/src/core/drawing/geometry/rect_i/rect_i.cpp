#include "gui_forms/drawing/geometry/rect_i/rect_i.hpp"

#include "../../support/drawing_support.hpp"

#include <algorithm>

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

} // namespace gui_drawing
