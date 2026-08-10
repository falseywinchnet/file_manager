#include "gui_forms/types.hpp"

#include <algorithm>

namespace gui_forms {

Rect Rect::intersection(Rect left, Rect right) noexcept {
    const double x0 = std::max(left.x, right.x);
    const double y0 = std::max(left.y, right.y);
    const double x1 = std::min(left.x + left.width, right.x + right.width);
    const double y1 = std::min(left.y + left.height, right.y + right.height);
    return {x0, y0, std::max(0.0, x1 - x0), std::max(0.0, y1 - y0)};
}

Rect Rect::united(Rect left, Rect right) noexcept {
    if (left.empty()) {
        return right;
    }
    if (right.empty()) {
        return left;
    }
    const double x0 = std::min(left.x, right.x);
    const double y0 = std::min(left.y, right.y);
    const double x1 = std::max(left.x + left.width, right.x + right.width);
    const double y1 = std::max(left.y + left.height, right.y + right.height);
    return {x0, y0, x1 - x0, y1 - y0};
}

} // namespace gui_forms
