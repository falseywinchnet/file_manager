#pragma once

#include <compare>
#include <cstdint>

namespace gui_drawing {

struct PointI final {
    std::int32_t x{};
    std::int32_t y{};
    void offset(std::int32_t dx, std::int32_t dy) noexcept;
    friend constexpr bool operator==(const PointI&, const PointI&) = default;
};

struct PointF final {
    double x{};
    double y{};
    void offset(double dx, double dy);
    friend constexpr bool operator==(const PointF&, const PointF&) = default;
};

struct SizeI final {
    std::int32_t width{};
    std::int32_t height{};
    friend constexpr bool operator==(const SizeI&, const SizeI&) = default;
};

struct SizeF final {
    double width{};
    double height{};
    friend constexpr bool operator==(const SizeF&, const SizeF&) = default;
};

struct RectI final {
    std::int32_t x{};
    std::int32_t y{};
    std::int32_t width{};
    std::int32_t height{};

    [[nodiscard]] constexpr std::int64_t left() const noexcept { return x; }
    [[nodiscard]] constexpr std::int64_t top() const noexcept { return y; }
    [[nodiscard]] constexpr std::int64_t right() const noexcept {
        return static_cast<std::int64_t>(x) + width;
    }
    [[nodiscard]] constexpr std::int64_t bottom() const noexcept {
        return static_cast<std::int64_t>(y) + height;
    }
    [[nodiscard]] constexpr bool empty() const noexcept {
        return width <= 0 || height <= 0;
    }
    [[nodiscard]] bool contains(PointI point) const noexcept;
    [[nodiscard]] bool contains(RectI rect) const noexcept;
    [[nodiscard]] bool intersects(RectI rect) const noexcept;
    void offset(std::int32_t dx, std::int32_t dy) noexcept;
    void inflate(std::int32_t dx, std::int32_t dy) noexcept;
    void intersect(RectI rect) noexcept;
    [[nodiscard]] static RectI intersection(RectI left, RectI right) noexcept;
    [[nodiscard]] static RectI united(RectI left, RectI right) noexcept;
    friend constexpr bool operator==(const RectI&, const RectI&) = default;
};

struct RectF final {
    double x{};
    double y{};
    double width{};
    double height{};

    [[nodiscard]] constexpr double left() const noexcept { return x; }
    [[nodiscard]] constexpr double top() const noexcept { return y; }
    [[nodiscard]] constexpr double right() const noexcept { return x + width; }
    [[nodiscard]] constexpr double bottom() const noexcept { return y + height; }
    [[nodiscard]] constexpr bool empty() const noexcept {
        return width <= 0.0 || height <= 0.0;
    }
    [[nodiscard]] bool finite() const noexcept;
    [[nodiscard]] bool contains(PointF point) const noexcept;
    [[nodiscard]] bool contains(RectF rect) const noexcept;
    [[nodiscard]] bool intersects(RectF rect) const noexcept;
    void offset(double dx, double dy);
    void inflate(double dx, double dy);
    void intersect(RectF rect) noexcept;
    [[nodiscard]] static RectF intersection(RectF left, RectF right) noexcept;
    [[nodiscard]] static RectF united(RectF left, RectF right) noexcept;
    friend constexpr bool operator==(const RectF&, const RectF&) = default;
};

} // namespace gui_drawing
