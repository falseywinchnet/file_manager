#pragma once

#include <algorithm>
#include <cmath>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

namespace gui_forms {

class LiveSurface;

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

// Renderer-neutral retained paint vocabulary.  Stops are deliberately copied
// into display chunks; callers may therefore supply stack-backed spans without
// extending their lifetime through presentation.
struct GradientStop final {
    double offset{};
    Color color{};
    friend constexpr bool operator==(const GradientStop&,
                                     const GradientStop&) = default;
};

// Controls how a linear gradient behaves outside its authored start/end
// interval. `repeat` is the retained material primitive behind pinstripes,
// grooves, scanlines, and other scale-independent surface texture; `reflect`
// mirrors alternate intervals so the seam remains continuous.
enum class GradientSpreadMode : std::uint8_t {
    pad,
    repeat,
    reflect,
};

inline constexpr std::size_t maximum_gradient_stops = 32U;

[[nodiscard]] bool valid_gradient_stops(
    std::span<const GradientStop> stops) noexcept;

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
    // Additional logical pixels inserted between shaped grapheme clusters.
    // This is a layout input: painters and measurement must apply the same
    // value. Zero preserves the typeface's native spacing.
    double letter_spacing{};
    friend constexpr bool operator==(const FontSpec&, const FontSpec&) = default;
};

[[nodiscard]] inline bool valid_font_spec(FontSpec font) noexcept {
    return std::isfinite(font.size) && font.size > 0.0 &&
           std::isfinite(font.letter_spacing) &&
           font.letter_spacing >= -font.size * 0.25 &&
           font.letter_spacing <= font.size;
}

struct ImageId {
    std::uint64_t value{};
    friend constexpr auto operator<=>(const ImageId&, const ImageId&) = default;
};

enum class ImagePatternWrap : std::uint8_t {
    tile,
};

enum class ImageSampling : std::uint8_t {
    nearest,
    linear,
};

class Painter {
public:
    virtual ~Painter() = default;

    virtual void save() = 0;
    virtual void restore() = 0;
    virtual void translate(Point offset) = 0;
    virtual void clip_rect(Rect rect) = 0;
    // Rich geometry has deterministic renderer-neutral fallbacks so a minimal
    // host stays coherent.  Recording and production raster painters override
    // these operations to preserve the authored material exactly.
    virtual void clip_rounded_rect(Rect rect, double radius);
    virtual void fill_rect(Rect rect, Color color) = 0;
    virtual void fill_rounded_rect(Rect rect, double radius, Color color);
    virtual void stroke_rect(Rect rect, Color color, double width) = 0;
    virtual void stroke_rounded_rect(Rect rect, double radius, Color color,
                                     double width);
    virtual void fill_linear_gradient(
        Rect rect, Point start, Point end,
        std::span<const GradientStop> stops);
    virtual void fill_linear_gradient_spread(
        Rect rect, Point start, Point end,
        std::span<const GradientStop> stops, GradientSpreadMode spread);
    virtual void fill_radial_gradient(
        Rect rect, Point center, Size radii,
        std::span<const GradientStop> stops);
    // Drawn before the owning surface fill.  The shape body may therefore be
    // included by a fallback without changing the final composited result.
    virtual void draw_box_shadow(Rect rect, double corner_radius, Point offset,
                                 double blur_radius, double spread,
                                 Color color);
    virtual void draw_line(Point from, Point to, Color color, double width) = 0;
    virtual void draw_text_utf8(Point origin,
                                std::string_view text,
                                FontSpec font,
                                Color color) = 0;
    // Text controls need the same metrics used by the painter for caret hit
    // testing, selection geometry, and horizontal viewport maintenance.  The
    // renderer owns those metrics; controls must not guess a fixed glyph width.
    [[nodiscard]] virtual Size measure_text_utf8(std::string_view text,
                                                 FontSpec font) {
        std::size_t scalars{};
        for (const unsigned char byte : text) {
            if ((byte & 0xc0U) != 0x80U) ++scalars;
        }
        const double tracking = scalars > 1U
            ? static_cast<double>(scalars - 1U) * font.letter_spacing : 0.0;
        return {static_cast<double>(scalars) * font.size * 0.55 + tracking,
                font.size * 1.2};
    }
    virtual void draw_image(ImageId image,
                            Rect destination,
                            double opacity = 1.0) = 0;
    // A live raster is a retained renderer-neutral resource whose producer may
    // publish independently of the UI dispatcher. Display lists retain the
    // resource and terminal renderers sample only its newest complete frame.
    virtual void draw_live_surface(std::shared_ptr<LiveSurface> surface,
                                   Rect destination,
                                   double opacity = 1.0);
    // Source-region replay is the renderer-neutral primitive behind image
    // strips, texture fills, and nine-patch materials. `source` is expressed
    // in source-image pixels. Minimal painters may preserve coherence by
    // falling back to whole-image scaling; production painters override it.
    virtual void draw_image_region(ImageId image, Rect source,
                                   Rect destination,
                                   double opacity = 1.0);
    virtual void draw_image_region_sampled(
        ImageId image, Rect source, Rect destination, ImageSampling sampling,
        double opacity = 1.0);
    // One retained command regardless of repetition count. Production
    // painters realize the pattern exactly; the base implementation is a
    // bounded compatibility fallback for deliberately minimal hosts.
    virtual void fill_image_pattern(ImageId image, Size source_pixel_size,
                                    Rect destination, Size logical_tile_size,
                                    ImagePatternWrap wrap = ImagePatternWrap::tile,
                                    double opacity = 1.0);
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
