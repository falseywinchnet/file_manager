#pragma once

#include <compare>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

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

class Color final {
public:
    constexpr Color() noexcept = default;

    [[nodiscard]] static constexpr Color empty() noexcept { return {}; }
    [[nodiscard]] static constexpr Color from_argb(std::uint32_t argb) noexcept {
        return Color(argb, false, false);
    }
    [[nodiscard]] static constexpr Color from_argb(std::uint8_t alpha,
                                                    std::uint8_t red,
                                                    std::uint8_t green,
                                                    std::uint8_t blue) noexcept {
        return from_argb((static_cast<std::uint32_t>(alpha) << 24U) |
                         (static_cast<std::uint32_t>(red) << 16U) |
                         (static_cast<std::uint32_t>(green) << 8U) | blue);
    }
    [[nodiscard]] static constexpr Color from_rgb(std::uint8_t red,
                                                   std::uint8_t green,
                                                   std::uint8_t blue) noexcept {
        return from_argb(255U, red, green, blue);
    }
    [[nodiscard]] static constexpr Color with_alpha(std::uint8_t alpha,
                                                     Color color) noexcept {
        return color.empty_ ? Color::empty() :
            Color((static_cast<std::uint32_t>(alpha) << 24U) |
                  (color.argb_ & UINT32_C(0x00ffffff)), false, color.known_);
    }

    [[nodiscard]] static Color from_name(std::string_view name);
    [[nodiscard]] static Color from_html(std::string_view value);

    [[nodiscard]] constexpr bool is_empty() const noexcept { return empty_; }
    [[nodiscard]] constexpr bool is_known() const noexcept { return known_; }
    [[nodiscard]] constexpr std::uint32_t argb() const noexcept { return argb_; }
    [[nodiscard]] constexpr std::uint8_t alpha() const noexcept {
        return static_cast<std::uint8_t>(argb_ >> 24U);
    }
    [[nodiscard]] constexpr std::uint8_t red() const noexcept {
        return static_cast<std::uint8_t>(argb_ >> 16U);
    }
    [[nodiscard]] constexpr std::uint8_t green() const noexcept {
        return static_cast<std::uint8_t>(argb_ >> 8U);
    }
    [[nodiscard]] constexpr std::uint8_t blue() const noexcept {
        return static_cast<std::uint8_t>(argb_);
    }
    [[nodiscard]] double brightness() const noexcept;

    friend constexpr bool operator==(const Color&, const Color&) = default;

private:
    constexpr Color(std::uint32_t argb, bool empty, bool known) noexcept
        : argb_(argb), empty_(empty), known_(known) {}

    [[nodiscard]] static constexpr Color known(std::uint32_t argb) noexcept {
        return Color(argb, false, true);
    }

    std::uint32_t argb_{};
    bool empty_{true};
    bool known_{};
};

struct SystemPalette final {
    Color control{Color::from_rgb(240, 240, 240)};
    Color control_light{Color::from_rgb(255, 255, 255)};
    Color control_text{Color::from_rgb(0, 0, 0)};
};

[[nodiscard]] const SystemPalette& default_system_palette() noexcept;

class Matrix final {
public:
    constexpr Matrix() noexcept = default;
    constexpr Matrix(double m11, double m12, double m21, double m22,
                     double dx, double dy) noexcept
        : m11_(m11), m12_(m12), m21_(m21), m22_(m22), dx_(dx), dy_(dy) {}

    [[nodiscard]] static Matrix translation(double x, double y);
    [[nodiscard]] static Matrix rotation_at(double degrees, PointF center);
    [[nodiscard]] bool finite() const noexcept;
    [[nodiscard]] PointF transform(PointF point) const;
    [[nodiscard]] RectF transform_bounds(RectF rect) const;
    [[nodiscard]] Matrix followed_by(const Matrix& next) const;

    [[nodiscard]] constexpr double m11() const noexcept { return m11_; }
    [[nodiscard]] constexpr double m12() const noexcept { return m12_; }
    [[nodiscard]] constexpr double m21() const noexcept { return m21_; }
    [[nodiscard]] constexpr double m22() const noexcept { return m22_; }
    [[nodiscard]] constexpr double dx() const noexcept { return dx_; }
    [[nodiscard]] constexpr double dy() const noexcept { return dy_; }
    friend constexpr bool operator==(const Matrix&, const Matrix&) = default;

private:
    double m11_{1.0};
    double m12_{};
    double m21_{};
    double m22_{1.0};
    double dx_{};
    double dy_{};
};

enum class ObjectState : std::uint8_t { alive, disposing, disposed };

class DrawingObject {
public:
    DrawingObject();
    virtual ~DrawingObject();
    DrawingObject(const DrawingObject&) = delete;
    DrawingObject& operator=(const DrawingObject&) = delete;

    void dispose();
    [[nodiscard]] ObjectState state() const;
    [[nodiscard]] bool is_disposed() const;
    [[nodiscard]] std::thread::id owner_thread() const noexcept { return owner_thread_; }
    void verify_access() const;

protected:
    void require_alive() const;
    virtual void on_dispose() noexcept;

private:
    std::thread::id owner_thread_;
    ObjectState state_{ObjectState::alive};
};

enum class DashStyle : std::uint8_t { solid, dash, dot, dash_dot, dash_dot_dot, custom };
enum class FontStyle : std::uint32_t {
    regular = 0,
    bold = 1U << 0U,
    italic = 1U << 1U,
    underline = 1U << 2U,
    strikeout = 1U << 3U,
};
enum class GraphicsUnit : std::uint8_t { world, display, pixel, point, inch, document, millimeter };
enum class StringAlignment : std::uint8_t { near, center, far };
enum class StringTrimming : std::uint8_t { none, character, word, ellipsis_character, ellipsis_word, ellipsis_path };
enum class SmoothingMode : std::uint8_t { default_mode, high_speed, high_quality, none, anti_alias };
enum class InterpolationMode : std::uint8_t { default_mode, low, high, nearest, bilinear, bicubic, high_quality_bilinear, high_quality_bicubic };
enum class PixelOffsetMode : std::uint8_t { default_mode, high_speed, high_quality, none, half };
enum class CompositingMode : std::uint8_t { source_over, source_copy };
enum class CompositingQuality : std::uint8_t { default_mode, high_speed, high_quality, gamma_corrected, assume_linear };
enum class FillMode : std::uint8_t { alternate, winding };
enum class PixelFormat : std::uint8_t { bgra32_premultiplied, rgba32_premultiplied };
enum class WrapMode : std::uint8_t { tile, tile_flip_x, tile_flip_y, tile_flip_xy, clamp };
enum class HatchStyle : std::uint8_t {
    horizontal, vertical, forward_diagonal, backward_diagonal,
    cross, diagonal_cross,
};
enum class BrushKind : std::uint8_t { solid, hatch, linear_gradient, path_gradient };

struct ColorBlend final {
    std::vector<Color> colors;
    std::vector<double> positions;
};

struct BrushSnapshot final {
    BrushKind kind{BrushKind::solid};
    Color primary;
    Color secondary;
    HatchStyle hatch_style{HatchStyle::horizontal};
    WrapMode wrap_mode{WrapMode::tile};
    RectF bounds;
    PointF center;
    double angle{};
    std::vector<PointF> points;
    std::vector<Color> colors;
    std::vector<double> positions;
};

class Brush : public DrawingObject {
public:
    [[nodiscard]] virtual BrushSnapshot snapshot() const = 0;
};

class SolidBrush final : public Brush {
public:
    explicit SolidBrush(Color color);
    [[nodiscard]] Color color() const;
    [[nodiscard]] BrushSnapshot snapshot() const override;

private:
    Color color_;
};

class HatchBrush final : public Brush {
public:
    HatchBrush(HatchStyle style, Color foreground,
               Color background = Color::from_argb(0U, 0U, 0U, 0U));
    [[nodiscard]] BrushSnapshot snapshot() const override;

private:
    HatchStyle style_;
    Color foreground_;
    Color background_;
};

class LinearGradientBrush final : public Brush {
public:
    LinearGradientBrush(RectF bounds, Color first, Color second,
                        double angle = 0.0,
                        WrapMode wrap_mode = WrapMode::tile);
    void set_blend(std::span<const double> factors,
                   std::span<const double> positions);
    void set_interpolation_colors(const ColorBlend& blend);
    void set_wrap_mode(WrapMode mode);
    [[nodiscard]] BrushSnapshot snapshot() const override;

private:
    RectF bounds_;
    Color first_;
    Color second_;
    double angle_{};
    WrapMode wrap_mode_{WrapMode::tile};
    std::vector<Color> colors_;
    std::vector<double> positions_;
};

class PathGradientBrush final : public Brush {
public:
    explicit PathGradientBrush(std::span<const PointF> points,
                               WrapMode wrap_mode = WrapMode::clamp);
    void set_center_color(Color color);
    void set_center_point(PointF point);
    void set_surround_colors(std::span<const Color> colors);
    void set_interpolation_colors(const ColorBlend& blend);
    [[nodiscard]] BrushSnapshot snapshot() const override;

private:
    std::vector<PointF> points_;
    PointF center_;
    Color center_color_{Color::from_rgb(255, 255, 255)};
    std::vector<Color> surround_colors_{Color::from_rgb(0, 0, 0)};
    WrapMode wrap_mode_{WrapMode::clamp};
    std::vector<Color> colors_;
    std::vector<double> positions_;
};

struct PenSnapshot final {
    Color color;
    double width{1.0};
    DashStyle dash_style{DashStyle::solid};
    std::vector<double> dash_pattern;
};

class Pen final : public DrawingObject {
public:
    explicit Pen(Color color, double width = 1.0);
    explicit Pen(const Brush& brush, double width = 1.0);

    void set_width(double width);
    void set_dash_style(DashStyle style);
    void set_dash_pattern(std::span<const double> pattern);
    [[nodiscard]] PenSnapshot snapshot() const;

private:
    Color color_;
    double width_{1.0};
    DashStyle dash_style_{DashStyle::solid};
    std::vector<double> dash_pattern_;
};

struct FontSnapshot final {
    std::string family;
    double size{12.0};
    std::uint32_t style{};
    GraphicsUnit unit{GraphicsUnit::point};
    std::uint8_t charset{1};
};

class Font final : public DrawingObject {
public:
    Font(std::string family, double size,
         FontStyle style = FontStyle::regular,
         GraphicsUnit unit = GraphicsUnit::point,
         std::uint8_t charset = 1);
    Font(const Font& source, FontStyle style);

    [[nodiscard]] FontSnapshot snapshot() const;
    [[nodiscard]] std::int32_t deterministic_height() const;

private:
    FontSnapshot value_;
};

struct StringFormatSnapshot final {
    StringAlignment alignment{StringAlignment::near};
    StringAlignment line_alignment{StringAlignment::near};
    StringTrimming trimming{StringTrimming::none};
    std::uint32_t flags{};
};

class StringFormat final : public DrawingObject {
public:
    StringFormat() = default;
    explicit StringFormat(std::uint32_t flags);
    explicit StringFormat(const StringFormat& source);

    void set_alignment(StringAlignment alignment);
    void set_line_alignment(StringAlignment alignment);
    void set_trimming(StringTrimming trimming);
    void set_flags(std::uint32_t flags);
    [[nodiscard]] StringFormatSnapshot snapshot() const;

private:
    StringFormatSnapshot value_;
};

enum class PathVerb : std::uint8_t {
    start_figure,
    line,
    rectangle,
    ellipse,
    arc,
    close_figure,
};

struct PathElement final {
    PathVerb verb{};
    PointF first{};
    PointF second{};
    RectF rect{};
    double start_angle{};
    double sweep_angle{};
};

struct PathSnapshot final {
    FillMode fill_mode{FillMode::alternate};
    std::vector<PathElement> elements;
};

struct PixelStorage;

class GraphicsPath final : public DrawingObject {
public:
    static constexpr std::size_t maximum_elements = 1'000'000;

    explicit GraphicsPath(FillMode fill_mode = FillMode::alternate);
    void reset();
    void start_figure();
    void close_figure();
    void add_line(PointF from, PointF to);
    void add_rectangle(RectF rectangle);
    void add_ellipse(RectF bounds);
    void add_arc(RectF bounds, double start_angle, double sweep_angle);
    void add_path(const GraphicsPath& path, bool connect);
    void transform(const Matrix& matrix);
    [[nodiscard]] bool is_visible(PointF point) const;
    [[nodiscard]] std::vector<PointF> path_points() const;
    [[nodiscard]] std::unique_ptr<GraphicsPath> clone() const;
    [[nodiscard]] RectF bounds() const;
    [[nodiscard]] PathSnapshot snapshot() const;

private:
    void append(PathElement element);

    FillMode fill_mode_;
    std::vector<PathElement> elements_;
};

struct RegionSnapshot final {
    std::vector<RectF> rectangles;
    std::vector<PathSnapshot> paths;
    std::vector<RectF> exclusions;
};

class Region final : public DrawingObject {
public:
    explicit Region(RectF rectangle);
    explicit Region(const GraphicsPath& path);
    void unite(RectF rectangle);
    void unite(const GraphicsPath& path);
    void exclude(RectF rectangle);
    [[nodiscard]] bool is_visible(PointF point) const;
    [[nodiscard]] RectF bounds() const;
    [[nodiscard]] RegionSnapshot snapshot() const;

private:
    RegionSnapshot value_;
};

struct ImageSnapshot final {
    std::uint64_t stable_id{};
    std::uint32_t width{};
    std::uint32_t height{};
    PixelFormat pixel_format{PixelFormat::bgra32_premultiplied};
    std::uint64_t generation{};

    [[nodiscard]] bool has_pixels() const noexcept;
    [[nodiscard]] std::size_t row_bytes() const noexcept;
    [[nodiscard]] std::span<const std::byte> pixels() const noexcept;

private:
    std::shared_ptr<const PixelStorage> storage_;
    friend class Bitmap;
};

/*
 * M11e image identity only. Pixel storage, locking, codecs, and mutation are
 * intentionally M11f work; this object lets commands and ABI ownership be
 * proven without smuggling a backend surface into the portable contract.
 */
class ImageReference final : public DrawingObject {
public:
    ImageReference(std::uint64_t stable_id, std::uint32_t width,
                   std::uint32_t height, PixelFormat pixel_format,
                   std::uint64_t generation = 1);
    [[nodiscard]] ImageSnapshot snapshot() const;

private:
    ImageSnapshot value_;
};

struct ImageAttributesSnapshot final {
    bool has_color_matrix{};
    std::array<double, 25> color_matrix{};
    struct ColorRemap final {
        Color old_color;
        Color new_color;
        friend constexpr bool operator==(const ColorRemap&,
                                         const ColorRemap&) = default;
    };
    std::vector<ColorRemap> remap_table;
};

class ImageAttributes final : public DrawingObject {
public:
    void set_color_matrix(std::span<const double, 25> matrix);
    void reset_color_matrix();
    void set_remap_table(std::span<const ImageAttributesSnapshot::ColorRemap> table);
    void reset_remap_table();
    [[nodiscard]] std::unique_ptr<ImageAttributes> clone() const;
    [[nodiscard]] ImageAttributesSnapshot snapshot() const;

private:
    ImageAttributesSnapshot value_;
};

enum class BitmapLockMode : std::uint8_t { read, write, read_write };

struct BitmapLockView final {
    const std::byte* data{};
    std::byte* writable_data{};
    std::size_t row_bytes{};
    std::uint32_t width{};
    std::uint32_t height{};
    PixelFormat pixel_format{PixelFormat::bgra32_premultiplied};
    std::uint64_t token{};
};

class Bitmap final : public DrawingObject {
public:
    static constexpr std::uint32_t maximum_dimension = 32768;
    static constexpr std::uint64_t maximum_bytes = 256ULL * 1024ULL * 1024ULL;

    Bitmap(std::uint32_t width, std::uint32_t height,
           PixelFormat pixel_format = PixelFormat::bgra32_premultiplied);

    [[nodiscard]] std::uint32_t width() const;
    [[nodiscard]] std::uint32_t height() const;
    [[nodiscard]] PixelFormat pixel_format() const;
    [[nodiscard]] std::uint64_t generation() const;
    [[nodiscard]] Color get_pixel(std::uint32_t x, std::uint32_t y) const;
    void set_pixel(std::uint32_t x, std::uint32_t y, Color color);
    void make_transparent(Color key);
    [[nodiscard]] std::unique_ptr<Bitmap> clone(RectI source) const;
    [[nodiscard]] std::unique_ptr<Bitmap> thumbnail(std::uint32_t width,
                                                    std::uint32_t height) const;
    [[nodiscard]] std::unique_ptr<Bitmap> adjusted(
        const ImageAttributes& attributes) const;

    [[nodiscard]] BitmapLockView lock(BitmapLockMode mode);
    void unlock(std::uint64_t token);
    [[nodiscard]] bool locked() const;
    [[nodiscard]] ImageSnapshot snapshot() const;

protected:
    void on_dispose() noexcept override;

private:
    void require_unlocked() const;
    void require_coordinate(std::uint32_t x, std::uint32_t y) const;
    void prepare_write();
    void publish_mutation();

    std::shared_ptr<PixelStorage> storage_;
    std::uint64_t stable_id_{};
    std::uint64_t generation_{1};
    std::uint64_t next_lock_token_{1};
    std::uint64_t active_lock_token_{};
    BitmapLockMode lock_mode_{BitmapLockMode::read};
};

struct GraphicsState final {
    Matrix transform;
    std::optional<RectF> clip;
    SmoothingMode smoothing{SmoothingMode::default_mode};
    InterpolationMode interpolation{InterpolationMode::default_mode};
    PixelOffsetMode pixel_offset{PixelOffsetMode::default_mode};
    CompositingMode compositing{CompositingMode::source_over};
    CompositingQuality compositing_quality{CompositingQuality::default_mode};
};

struct GraphicsStateToken final {
    std::uint64_t value{};
    friend constexpr bool operator==(const GraphicsStateToken&,
                                     const GraphicsStateToken&) = default;
};

enum class CommandKind : std::uint8_t {
    save,
    restore,
    translate,
    set_transform,
    set_clip,
    reset_clip,
    set_quality,
    clear,
    fill_rectangle,
    draw_rectangle,
    draw_line,
    draw_string,
    draw_ellipse,
    fill_ellipse,
    fill_polygon,
    draw_path,
    fill_path,
    draw_image,
};

struct DrawingCommand final {
    CommandKind kind{};
    GraphicsState state;
    GraphicsStateToken token;
    PointF first{};
    PointF second{};
    RectF rect{};
    Color color;
    PenSnapshot pen;
    FontSnapshot font;
    StringFormatSnapshot format;
    PathSnapshot path;
    ImageSnapshot image;
    ImageAttributesSnapshot image_attributes;
    BrushSnapshot brush;
    std::vector<PointF> points;
    std::string text;
};

class TextMetricsProvider {
public:
    virtual ~TextMetricsProvider() = default;
    [[nodiscard]] virtual SizeF measure(std::string_view utf8,
                                        const FontSnapshot& font,
                                        const StringFormatSnapshot& format) const = 0;
};

class GraphicsRecorder final : public DrawingObject {
public:
    static constexpr std::size_t maximum_commands = 1'000'000;
    static constexpr std::size_t maximum_state_depth = 256;
    static constexpr std::size_t maximum_text_bytes = 1U << 20U;

    GraphicsRecorder() = default;

    [[nodiscard]] GraphicsStateToken save();
    void restore(GraphicsStateToken token);
    void translate(double x, double y);
    void set_transform(Matrix transform);
    void set_clip(RectF clip);
    void reset_clip();
    void set_quality(SmoothingMode smoothing,
                     InterpolationMode interpolation,
                     PixelOffsetMode pixel_offset,
                     CompositingMode compositing,
                     CompositingQuality compositing_quality);

    [[nodiscard]] GraphicsState current_state() const;
    [[nodiscard]] bool is_visible(PointF point) const;
    [[nodiscard]] SizeF measure_string(std::string_view utf8, const Font& font,
                                       const StringFormat& format,
                                       const TextMetricsProvider& provider) const;

    void clear(Color color);
    void fill_rectangle(const Brush& brush, RectF rect);
    void draw_rectangle(const Pen& pen, RectF rect);
    void draw_line(const Pen& pen, PointF from, PointF to);
    void draw_string(std::string_view utf8, const Font& font,
                     const SolidBrush& brush, PointF origin,
                     const StringFormat& format);
    void draw_ellipse(const Pen& pen, RectF bounds);
    void fill_ellipse(const Brush& brush, RectF bounds);
    void fill_polygon(const Brush& brush, std::span<const PointF> points,
                      FillMode fill_mode = FillMode::alternate);
    void draw_path(const Pen& pen, const GraphicsPath& path);
    void fill_path(const Brush& brush, const GraphicsPath& path);
    void draw_image(const ImageReference& image, RectF destination,
                    RectF source, const ImageAttributes& attributes);
    void draw_image(const Bitmap& image, RectF destination,
                    RectF source, const ImageAttributes& attributes);

    void close();
    [[nodiscard]] bool closed() const;
    [[nodiscard]] std::span<const DrawingCommand> commands() const;
    [[nodiscard]] std::string deterministic_trace() const;

protected:
    void on_dispose() noexcept override;

private:
    struct SavedState final {
        GraphicsStateToken token;
        GraphicsState state;
    };

    void require_recordable() const;
    void append(DrawingCommand command);

    GraphicsState state_;
    std::vector<SavedState> saved_;
    std::vector<DrawingCommand> commands_;
    std::uint64_t next_token_{1};
    bool closed_{};
};

constexpr FontStyle operator|(FontStyle left, FontStyle right) noexcept {
    return static_cast<FontStyle>(static_cast<std::uint32_t>(left) |
                                  static_cast<std::uint32_t>(right));
}

} // namespace gui_drawing
