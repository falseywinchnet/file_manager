#pragma once

#include <algorithm>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

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
    [[nodiscard]] constexpr double area() const noexcept {
        return empty() ? 0.0 : width * height;
    }
    [[nodiscard]] constexpr bool contains(Point point) const noexcept {
        return !empty() && point.x >= x && point.y >= y &&
               point.x < x + width && point.y < y + height;
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

struct Color {
    std::uint8_t red{};
    std::uint8_t green{};
    std::uint8_t blue{};
    std::uint8_t alpha{255};
    friend constexpr bool operator==(const Color&, const Color&) = default;

    [[nodiscard]] static constexpr Color rgba(std::uint8_t red_value,
                                               std::uint8_t green_value,
                                               std::uint8_t blue_value,
                                               std::uint8_t alpha_value = 255) noexcept {
        return {red_value, green_value, blue_value, alpha_value};
    }
};

enum class FontRole : std::uint8_t {
    control,
    content,
    monospace,
};

enum class CursorKind : std::uint8_t {
    arrow,
    text,
    hand,
    crosshair,
    resize_horizontal,
    resize_vertical,
    wait,
    forbidden,
};

struct FontSpec {
    FontRole role{FontRole::control};
    double size{13.0};
    std::uint16_t weight{400};
    bool italic{};
};

struct ImageId {
    std::uint64_t value{};
    friend constexpr auto operator<=>(const ImageId&, const ImageId&) = default;
};

class Painter {
public:
    virtual ~Painter() = default;

    virtual void save() = 0;
    virtual void restore() = 0;
    virtual void translate(Point offset) = 0;
    virtual void clip_rect(Rect rect) = 0;
    virtual void fill_rect(Rect rect, Color color) = 0;
    virtual void stroke_rect(Rect rect, Color color, double width) = 0;
    virtual void draw_line(Point from, Point to, Color color, double width) = 0;
    virtual void draw_text_utf8(Point origin,
                                std::string_view text,
                                FontSpec font,
                                Color color) = 0;
    virtual void draw_image(ImageId image,
                            Rect destination,
                            double opacity = 1.0) = 0;
};

class DamageRegion {
public:
    static constexpr std::size_t maximum_rectangles = 64;

    void add(Rect rect);
    void clear() noexcept;

    [[nodiscard]] bool empty() const noexcept { return rectangles_.empty(); }
    [[nodiscard]] std::span<const Rect> rectangles() const noexcept { return rectangles_; }
    [[nodiscard]] std::size_t rectangle_count() const noexcept { return rectangles_.size(); }
    [[nodiscard]] std::uint64_t compaction_count() const noexcept { return compaction_count_; }
    [[nodiscard]] std::uint64_t collapse_count() const noexcept { return collapse_count_; }
    [[nodiscard]] Rect bounds() const noexcept;
    [[nodiscard]] double area() const noexcept;

private:
    std::vector<Rect> rectangles_;
    std::uint64_t compaction_count_{};
    std::uint64_t collapse_count_{};
};

} // namespace gui_forms
