#pragma once

#include <cmath>

namespace gui_forms {

struct Point {
    double x{};
    double y{};
    friend constexpr bool operator==(const Point&, const Point&) = default;
};

struct Size {
    double width{};
    double height{};
    friend constexpr bool operator==(const Size&, const Size&) = default;
};

struct Rect {
    double x{};
    double y{};
    double width{};
    double height{};
    friend constexpr bool operator==(const Rect&, const Rect&) = default;

    [[nodiscard]] constexpr bool empty() const noexcept {
        return width <= 0.0 || height <= 0.0;
    }
    [[nodiscard]] bool finite() const noexcept {
        return std::isfinite(x) && std::isfinite(y) &&
               std::isfinite(width) && std::isfinite(height);
    }
    [[nodiscard]] constexpr double left() const noexcept { return x; }
    [[nodiscard]] constexpr double top() const noexcept { return y; }
    [[nodiscard]] constexpr double right() const noexcept { return x + width; }
    [[nodiscard]] constexpr double bottom() const noexcept { return y + height; }
    [[nodiscard]] constexpr double area() const noexcept {
        return empty() ? 0.0 : width * height;
    }
    [[nodiscard]] constexpr bool contains(Point point) const noexcept {
        return !empty() && point.x >= x && point.y >= y &&
               point.x < x + width && point.y < y + height;
    }
    [[nodiscard]] constexpr bool contains(Rect rect) const noexcept {
        return !empty() && !rect.empty() && rect.x >= x && rect.y >= y &&
               rect.x + rect.width <= x + width &&
               rect.y + rect.height <= y + height;
    }
    [[nodiscard]] static Rect intersection(Rect left, Rect right) noexcept;
    [[nodiscard]] static Rect united(Rect left, Rect right) noexcept;
};

struct Insets {
    double left{};
    double top{};
    double right{};
    double bottom{};
    friend constexpr bool operator==(const Insets&, const Insets&) = default;
};

} // namespace gui_forms
