#include "gui_forms/drawing.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace gui_drawing {

struct PixelStorage final {
    std::uint32_t width{};
    std::uint32_t height{};
    PixelFormat pixel_format{PixelFormat::bgra32_premultiplied};
    std::size_t row_bytes{};
    std::vector<std::byte> bytes;
};

namespace {

std::atomic_uint64_t next_bitmap_id{1};

[[nodiscard]] std::int32_t clamp_i32(std::int64_t value) noexcept {
    return static_cast<std::int32_t>(std::clamp(
        value,
        static_cast<std::int64_t>(std::numeric_limits<std::int32_t>::min()),
        static_cast<std::int64_t>(std::numeric_limits<std::int32_t>::max())));
}

void require_finite(double value, std::string_view field) {
    if (!std::isfinite(value)) {
        throw std::invalid_argument(std::string(field) + " must be finite");
    }
}

void require_finite(PointF point, std::string_view field) {
    if (!std::isfinite(point.x) || !std::isfinite(point.y)) {
        throw std::invalid_argument(std::string(field) + " must be finite");
    }
}

void require_finite(RectF rect, std::string_view field) {
    if (!rect.finite()) {
        throw std::invalid_argument(std::string(field) + " must be finite");
    }
}

[[nodiscard]] std::string ascii_lower(std::string_view input) {
    std::string result;
    result.reserve(input.size());
    for (const unsigned char value : input) {
        result.push_back(value >= 'A' && value <= 'Z'
            ? static_cast<char>(value + ('a' - 'A'))
            : static_cast<char>(value));
    }
    return result;
}

[[nodiscard]] int hex_digit(char value) noexcept {
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    return -1;
}

[[nodiscard]] std::uint32_t parse_hex(std::string_view value) {
    std::uint32_t result{};
    for (const char character : value) {
        const int digit = hex_digit(character);
        if (digit < 0) {
            throw std::invalid_argument("HTML color contains a non-hexadecimal digit");
        }
        result = (result << 4U) | static_cast<std::uint32_t>(digit);
    }
    return result;
}

[[nodiscard]] bool valid_utf8(std::string_view input) noexcept {
    std::size_t index{};
    while (index < input.size()) {
        const auto lead = static_cast<unsigned char>(input[index]);
        if (lead <= 0x7fU) {
            ++index;
            continue;
        }
        std::size_t count{};
        std::uint32_t scalar{};
        std::uint32_t minimum{};
        if ((lead & 0xe0U) == 0xc0U) {
            count = 2;
            scalar = lead & 0x1fU;
            minimum = 0x80U;
        } else if ((lead & 0xf0U) == 0xe0U) {
            count = 3;
            scalar = lead & 0x0fU;
            minimum = 0x800U;
        } else if ((lead & 0xf8U) == 0xf0U) {
            count = 4;
            scalar = lead & 0x07U;
            minimum = 0x10000U;
        } else {
            return false;
        }
        if (index + count > input.size()) return false;
        for (std::size_t offset = 1; offset < count; ++offset) {
            const auto continuation = static_cast<unsigned char>(input[index + offset]);
            if ((continuation & 0xc0U) != 0x80U) return false;
            scalar = (scalar << 6U) | (continuation & 0x3fU);
        }
        if (scalar < minimum || scalar > 0x10ffffU ||
            (scalar >= 0xd800U && scalar <= 0xdfffU)) {
            return false;
        }
        index += count;
    }
    return true;
}

[[nodiscard]] std::string number(double value) {
    if (value == 0.0) value = 0.0;
    std::array<char, 64> buffer{};
    const auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(),
                                      value, std::chars_format::general,
                                      std::numeric_limits<double>::max_digits10);
    if (result.ec != std::errc{}) {
        throw std::runtime_error("GUI.Drawing could not format a finite number");
    }
    return std::string(buffer.data(), result.ptr);
}

[[nodiscard]] std::string color_text(Color color) {
    if (color.is_empty()) return "empty";
    std::ostringstream output;
    output.imbue(std::locale::classic());
    output << std::hex << std::nouppercase << std::setfill('0') << std::setw(8)
           << color.argb();
    return output.str();
}

[[nodiscard]] std::string point_text(PointF point) {
    return number(point.x) + "," + number(point.y);
}

[[nodiscard]] std::string rect_text(RectF rect) {
    return number(rect.x) + "," + number(rect.y) + "," +
           number(rect.width) + "," + number(rect.height);
}

[[nodiscard]] std::string matrix_text(const Matrix& value) {
    return number(value.m11()) + "," + number(value.m12()) + "," +
           number(value.m21()) + "," + number(value.m22()) + "," +
           number(value.dx()) + "," + number(value.dy());
}

[[nodiscard]] const char* command_name(CommandKind kind) noexcept {
    switch (kind) {
    case CommandKind::save: return "save";
    case CommandKind::restore: return "restore";
    case CommandKind::translate: return "translate";
    case CommandKind::set_transform: return "set_transform";
    case CommandKind::set_clip: return "set_clip";
    case CommandKind::reset_clip: return "reset_clip";
    case CommandKind::set_quality: return "set_quality";
    case CommandKind::clear: return "clear";
    case CommandKind::fill_rectangle: return "fill_rectangle";
    case CommandKind::draw_rectangle: return "draw_rectangle";
    case CommandKind::draw_line: return "draw_line";
    case CommandKind::draw_string: return "draw_string";
    case CommandKind::draw_ellipse: return "draw_ellipse";
    case CommandKind::fill_ellipse: return "fill_ellipse";
    case CommandKind::fill_polygon: return "fill_polygon";
    case CommandKind::draw_path: return "draw_path";
    case CommandKind::fill_path: return "fill_path";
    case CommandKind::draw_image: return "draw_image";
    }
    return "unknown";
}

[[nodiscard]] std::string escaped(std::string_view text) {
    std::string result;
    result.reserve(text.size());
    for (const unsigned char value : text) {
        switch (value) {
        case '\\': result += "\\\\"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        case '|': result += "\\|"; break;
        default: result.push_back(static_cast<char>(value)); break;
        }
    }
    return result;
}

void require_pen_width(double width) {
    require_finite(width, "pen width");
    if (width <= 0.0 || width > 1'000'000.0) {
        throw std::invalid_argument("pen width must be positive and bounded");
    }
}

void require_blend_positions(std::span<const double> positions,
                             std::size_t color_count) {
    if (positions.size() != color_count || positions.size() < 2U ||
        positions.size() > 4096U) {
        throw std::invalid_argument(
            "gradient colors and positions require equal bounded counts");
    }
    double previous = -1.0;
    for (std::size_t index = 0; index < positions.size(); ++index) {
        const double position = positions[index];
        require_finite(position, "gradient position");
        if (position < 0.0 || position > 1.0 ||
            (index != 0U && position <= previous)) {
            throw std::invalid_argument(
                "gradient positions must be strictly ordered in the unit interval");
        }
        previous = position;
    }
    if (positions.front() != 0.0 || positions.back() != 1.0) {
        throw std::invalid_argument("gradient positions must begin at zero and end at one");
    }
}

[[nodiscard]] Color interpolate_color(Color first, Color second,
                                      double amount) noexcept {
    const auto channel = [amount](std::uint8_t left, std::uint8_t right) {
        return static_cast<std::uint8_t>(std::lround(
            static_cast<double>(left) +
            (static_cast<double>(right) - left) * amount));
    };
    return Color::from_argb(channel(first.alpha(), second.alpha()),
                            channel(first.red(), second.red()),
                            channel(first.green(), second.green()),
                            channel(first.blue(), second.blue()));
}

[[nodiscard]] std::uint8_t premultiply(std::uint8_t channel,
                                       std::uint8_t alpha) noexcept {
    return static_cast<std::uint8_t>(
        (static_cast<unsigned>(channel) * alpha + 127U) / 255U);
}

[[nodiscard]] std::uint8_t unpremultiply(std::uint8_t channel,
                                         std::uint8_t alpha) noexcept {
    if (alpha == 0U) return 0U;
    return static_cast<std::uint8_t>(std::min(
        255U, (static_cast<unsigned>(channel) * 255U + alpha / 2U) / alpha));
}

[[nodiscard]] std::uint8_t normalized_channel(double value) noexcept {
    if (value <= 0.0) return 0U;
    if (value >= 1.0) return 255U;
    return static_cast<std::uint8_t>(std::lround(value * 255.0));
}

[[nodiscard]] std::size_t pixel_offset(const PixelStorage& storage,
                                       std::uint32_t x,
                                       std::uint32_t y) noexcept {
    return static_cast<std::size_t>(y) * storage.row_bytes +
           static_cast<std::size_t>(x) * 4U;
}

void store_color(PixelStorage& storage, std::uint32_t x, std::uint32_t y,
                 Color color) noexcept {
    const std::size_t offset = pixel_offset(storage, x, y);
    const std::uint8_t alpha = color.is_empty() ? 0U : color.alpha();
    const std::uint8_t red = premultiply(color.red(), alpha);
    const std::uint8_t green = premultiply(color.green(), alpha);
    const std::uint8_t blue = premultiply(color.blue(), alpha);
    if (storage.pixel_format == PixelFormat::bgra32_premultiplied) {
        storage.bytes[offset] = static_cast<std::byte>(blue);
        storage.bytes[offset + 1U] = static_cast<std::byte>(green);
        storage.bytes[offset + 2U] = static_cast<std::byte>(red);
    } else {
        storage.bytes[offset] = static_cast<std::byte>(red);
        storage.bytes[offset + 1U] = static_cast<std::byte>(green);
        storage.bytes[offset + 2U] = static_cast<std::byte>(blue);
    }
    storage.bytes[offset + 3U] = static_cast<std::byte>(alpha);
}

[[nodiscard]] Color load_color(const PixelStorage& storage,
                               std::uint32_t x,
                               std::uint32_t y) noexcept {
    const std::size_t offset = pixel_offset(storage, x, y);
    const std::uint8_t first = std::to_integer<std::uint8_t>(storage.bytes[offset]);
    const std::uint8_t green =
        std::to_integer<std::uint8_t>(storage.bytes[offset + 1U]);
    const std::uint8_t third =
        std::to_integer<std::uint8_t>(storage.bytes[offset + 2U]);
    const std::uint8_t alpha =
        std::to_integer<std::uint8_t>(storage.bytes[offset + 3U]);
    const std::uint8_t red = storage.pixel_format == PixelFormat::bgra32_premultiplied
        ? third : first;
    const std::uint8_t blue = storage.pixel_format == PixelFormat::bgra32_premultiplied
        ? first : third;
    return Color::from_argb(alpha, unpremultiply(red, alpha),
                            unpremultiply(green, alpha), unpremultiply(blue, alpha));
}

[[nodiscard]] bool stored_color_equals(const PixelStorage& storage,
                                       std::uint32_t x, std::uint32_t y,
                                       Color color) noexcept {
    const std::size_t offset = pixel_offset(storage, x, y);
    const std::uint8_t alpha = color.is_empty() ? 0U : color.alpha();
    const std::uint8_t red = premultiply(color.red(), alpha);
    const std::uint8_t green = premultiply(color.green(), alpha);
    const std::uint8_t blue = premultiply(color.blue(), alpha);
    const std::uint8_t first = storage.pixel_format ==
            PixelFormat::bgra32_premultiplied ? blue : red;
    const std::uint8_t third = storage.pixel_format ==
            PixelFormat::bgra32_premultiplied ? red : blue;
    return std::to_integer<std::uint8_t>(storage.bytes[offset]) == first &&
        std::to_integer<std::uint8_t>(storage.bytes[offset + 1U]) == green &&
        std::to_integer<std::uint8_t>(storage.bytes[offset + 2U]) == third &&
        std::to_integer<std::uint8_t>(storage.bytes[offset + 3U]) == alpha;
}

template <typename Enum>
void require_enum(Enum value, unsigned maximum, std::string_view field) {
    using Underlying = std::underlying_type_t<Enum>;
    if (static_cast<unsigned>(static_cast<Underlying>(value)) > maximum) {
        throw std::invalid_argument(std::string(field) + " is outside the declared enum");
    }
}

[[nodiscard]] std::string pen_text(const PenSnapshot& pen) {
    std::string result = color_text(pen.color) + "," + number(pen.width) + "," +
                         std::to_string(static_cast<unsigned>(pen.dash_style));
    if (pen.dash_style == DashStyle::custom) {
        result += ",pattern=";
        for (std::size_t index = 0; index < pen.dash_pattern.size(); ++index) {
            if (index != 0U) result += ",";
            result += number(pen.dash_pattern[index]);
        }
    }
    return result;
}

[[nodiscard]] std::string brush_text(const BrushSnapshot& brush) {
    if (brush.kind == BrushKind::solid) return color_text(brush.primary);
    std::string result = std::to_string(static_cast<unsigned>(brush.kind)) + "," +
        color_text(brush.primary) + "," + color_text(brush.secondary) + "," +
        std::to_string(static_cast<unsigned>(brush.wrap_mode));
    if (brush.kind == BrushKind::hatch) {
        return result + ",hatch=" +
            std::to_string(static_cast<unsigned>(brush.hatch_style));
    }
    if (brush.kind == BrushKind::texture) {
        return result + ",image=" + std::to_string(brush.image.stable_id) +
            "@" + std::to_string(brush.image.generation) + ",size=" +
            std::to_string(brush.image.width) + "x" +
            std::to_string(brush.image.height) + ",transform=" +
            matrix_text(brush.transform);
    }
    result += ",bounds=" + rect_text(brush.bounds) +
              ",center=" + point_text(brush.center) +
              ",angle=" + number(brush.angle) + ",stops=";
    for (std::size_t index = 0; index < brush.colors.size(); ++index) {
        if (index != 0U) result += ";";
        result += color_text(brush.colors[index]) + "@" +
                  number(brush.positions[index]);
    }
    return result;
}

[[nodiscard]] std::string points_text(std::span<const PointF> points) {
    std::string result;
    for (std::size_t index = 0; index < points.size(); ++index) {
        if (index != 0U) result += ";";
        result += point_text(points[index]);
    }
    return result;
}

[[nodiscard]] std::string path_text(const PathSnapshot& path) {
    std::string result = std::to_string(static_cast<unsigned>(path.fill_mode));
    for (const PathElement& element : path.elements) {
        result += ";" + std::to_string(static_cast<unsigned>(element.verb));
        switch (element.verb) {
        case PathVerb::line:
            result += "," + point_text(element.first) + "," + point_text(element.second);
            break;
        case PathVerb::quadratic:
            result += "," + point_text(element.first) + "," +
                      point_text(element.second) + "," + point_text(element.third);
            break;
        case PathVerb::bezier:
            result += "," + point_text(element.first) + "," +
                      point_text(element.second) + "," + point_text(element.third) +
                      "," + point_text(element.fourth);
            break;
        case PathVerb::rectangle:
        case PathVerb::ellipse:
            result += "," + rect_text(element.rect);
            break;
        case PathVerb::arc:
            result += "," + rect_text(element.rect) + "," +
                      number(element.start_angle) + "," +
                      number(element.sweep_angle);
            break;
        case PathVerb::start_figure:
        case PathVerb::close_figure:
            break;
        }
    }
    return result;
}

[[nodiscard]] std::string image_attributes_text(
    const ImageAttributesSnapshot& attributes) {
    std::string result;
    if (attributes.has_color_matrix) {
        result = "matrix:";
        for (std::size_t index = 0; index < attributes.color_matrix.size(); ++index) {
            if (index != 0U) result += ",";
            result += number(attributes.color_matrix[index]);
        }
    }
    if (!attributes.remap_table.empty()) {
        if (!result.empty()) result += ";";
        result += "remap:";
        for (std::size_t index = 0; index < attributes.remap_table.size(); ++index) {
            if (index != 0U) result += ",";
            result += color_text(attributes.remap_table[index].old_color) + ">" +
                      color_text(attributes.remap_table[index].new_color);
        }
    }
    if (result.empty()) return "identity";
    return result;
}

[[nodiscard]] bool path_snapshot_visible(const PathSnapshot& path, PointF point) {
    bool inside = false;
    std::vector<PointF> figure;
    const auto include_polygon = [&] {
        if (figure.size() < 3U) {
            figure.clear();
            return;
        }
        bool hit = false;
        for (std::size_t i = 0, j = figure.size() - 1U; i < figure.size(); j = i++) {
            const PointF a = figure[i];
            const PointF b = figure[j];
            const bool crosses = ((a.y > point.y) != (b.y > point.y)) &&
                (point.x < (b.x - a.x) * (point.y - a.y) /
                               (b.y - a.y) + a.x);
            if (crosses) hit = !hit;
        }
        inside = path.fill_mode == FillMode::alternate ? inside != hit : inside || hit;
        figure.clear();
    };
    for (const PathElement& element : path.elements) {
        bool primitive_hit = false;
        switch (element.verb) {
        case PathVerb::start_figure:
            include_polygon();
            break;
        case PathVerb::line:
            if (figure.empty()) figure.push_back(element.first);
            figure.push_back(element.second);
            break;
        case PathVerb::quadratic: {
            if (figure.empty() || figure.back() != element.first) {
                figure.push_back(element.first);
            }
            constexpr int segments = 24;
            for (int index = 1; index <= segments; ++index) {
                const double t = static_cast<double>(index) / segments;
                const double u = 1.0 - t;
                figure.push_back({u * u * element.first.x +
                                      2.0 * u * t * element.second.x +
                                      t * t * element.third.x,
                                  u * u * element.first.y +
                                      2.0 * u * t * element.second.y +
                                      t * t * element.third.y});
            }
            break;
        }
        case PathVerb::bezier: {
            if (figure.empty() || figure.back() != element.first) {
                figure.push_back(element.first);
            }
            constexpr int segments = 32;
            for (int index = 1; index <= segments; ++index) {
                const double t = static_cast<double>(index) / segments;
                const double u = 1.0 - t;
                figure.push_back({u * u * u * element.first.x +
                                      3.0 * u * u * t * element.second.x +
                                      3.0 * u * t * t * element.third.x +
                                      t * t * t * element.fourth.x,
                                  u * u * u * element.first.y +
                                      3.0 * u * u * t * element.second.y +
                                      3.0 * u * t * t * element.third.y +
                                      t * t * t * element.fourth.y});
            }
            break;
        }
        case PathVerb::rectangle:
            include_polygon();
            primitive_hit = element.rect.contains(point);
            inside = path.fill_mode == FillMode::alternate ?
                inside != primitive_hit : inside || primitive_hit;
            break;
        case PathVerb::ellipse: {
            include_polygon();
            const double rx = element.rect.width / 2.0;
            const double ry = element.rect.height / 2.0;
            if (rx > 0.0 && ry > 0.0) {
                const double dx = (point.x - (element.rect.x + rx)) / rx;
                const double dy = (point.y - (element.rect.y + ry)) / ry;
                primitive_hit = dx * dx + dy * dy <= 1.0;
            }
            inside = path.fill_mode == FillMode::alternate ?
                inside != primitive_hit : inside || primitive_hit;
            break;
        }
        case PathVerb::arc: {
            constexpr double degrees_to_radians =
                0.01745329251994329576923690768489;
            const double center_x = element.rect.x + element.rect.width * 0.5;
            const double center_y = element.rect.y + element.rect.height * 0.5;
            const double radius_x = element.rect.width * 0.5;
            const double radius_y = element.rect.height * 0.5;
            const auto point_at = [&](double degrees) {
                const double radians = degrees * degrees_to_radians;
                return PointF{center_x + radius_x * std::cos(radians),
                              center_y + radius_y * std::sin(radians)};
            };
            const PointF start = point_at(element.start_angle);
            if (figure.empty() || figure.back() != start) figure.push_back(start);
            const int segments = std::max(
                1, static_cast<int>(std::ceil(std::abs(element.sweep_angle) / 12.0)));
            for (int index = 1; index <= segments; ++index) {
                const double ratio = static_cast<double>(index) / segments;
                figure.push_back(point_at(
                    element.start_angle + element.sweep_angle * ratio));
            }
            break;
        }
        case PathVerb::close_figure:
            include_polygon();
            break;
        }
    }
    include_polygon();
    return inside;
}

} // namespace

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

Color Color::from_name(std::string_view name) {
    struct Entry { std::string_view name; std::uint32_t argb; };
    static constexpr Entry entries[] = {
        {"aqua", 0xff00ffffU}, {"aquamarine", 0xff7fffd4U},
        {"azure", 0xfff0ffffU}, {"black", 0xff000000U},
        {"blue", 0xff0000ffU}, {"darkblue", 0xff00008bU},
        {"darkgray", 0xffa9a9a9U}, {"dodgerblue", 0xff1e90ffU},
        {"gainsboro", 0xffdcdcdcU}, {"gray", 0xff808080U},
        {"green", 0xff008000U}, {"khaki", 0xfff0e68cU},
        {"lightblue", 0xffadd8e6U}, {"lightcoral", 0xfff08080U},
        {"lightgreen", 0xff90ee90U}, {"lightpink", 0xffffb6c1U},
        {"lime", 0xff00ff00U}, {"limegreen", 0xff32cd32U},
        {"magenta", 0xffff00ffU}, {"maroon", 0xff800000U},
        {"orange", 0xffffa500U}, {"orangered", 0xffff4500U},
        {"orchid", 0xffda70d6U}, {"purple", 0xff800080U},
        {"red", 0xffff0000U}, {"salmon", 0xfffa8072U},
        {"silver", 0xffc0c0c0U}, {"springgreen", 0xff00ff7fU},
        {"transparent", 0x00ffffffU}, {"white", 0xffffffffU},
        {"yellow", 0xffffff00U}, {"yellowgreen", 0xff9acd32U},
    };
    const std::string key = ascii_lower(name);
    const auto found = std::find_if(std::begin(entries), std::end(entries),
                                    [&key](const Entry& entry) {
                                        return entry.name == key;
                                    });
    return found == std::end(entries) ? Color::empty() :
        Color(found->argb, false, true);
}

Color Color::from_html(std::string_view value) {
    if (value.empty()) return Color::empty();
    if (value.front() != '#') return from_name(value);
    const std::string_view digits = value.substr(1);
    if (digits.size() == 3) {
        const std::uint32_t compact = parse_hex(digits);
        const auto red = static_cast<std::uint8_t>(((compact >> 8U) & 0xfU) * 17U);
        const auto green = static_cast<std::uint8_t>(((compact >> 4U) & 0xfU) * 17U);
        const auto blue = static_cast<std::uint8_t>((compact & 0xfU) * 17U);
        return from_rgb(red, green, blue);
    }
    if (digits.size() == 6) return from_argb(UINT32_C(0xff000000) | parse_hex(digits));
    if (digits.size() == 8) return from_argb(parse_hex(digits));
    throw std::invalid_argument("HTML color must have 3, 6, or 8 hexadecimal digits");
}

double Color::brightness() const noexcept {
    if (empty_) return 0.0;
    const auto minimum = std::min({red(), green(), blue()});
    const auto maximum = std::max({red(), green(), blue()});
    return (static_cast<double>(minimum) + maximum) / 510.0;
}

namespace {

void require_alpha(double alpha, std::string_view field) {
    require_finite(alpha, field);
    if (alpha < 0.0 || alpha > 1.0) {
        throw std::invalid_argument(std::string(field) +
                                    " must be in the unit interval");
    }
}

void require_linear(LinearSrgb color) {
    require_finite(color.red, "linear sRGB red");
    require_finite(color.green, "linear sRGB green");
    require_finite(color.blue, "linear sRGB blue");
    require_alpha(color.alpha, "linear sRGB alpha");
}

void require_xyz(XyzD65 color) {
    require_finite(color.x, "XYZ X");
    require_finite(color.y, "XYZ Y");
    require_finite(color.z, "XYZ Z");
    require_alpha(color.alpha, "XYZ alpha");
}

void require_oklab(Oklab color) {
    require_finite(color.lightness, "OKLab lightness");
    require_finite(color.a, "OKLab a");
    require_finite(color.b, "OKLab b");
    require_alpha(color.alpha, "OKLab alpha");
}

[[nodiscard]] double normalized_hue(double degrees) noexcept {
    double result = std::fmod(degrees, 360.0);
    if (result < 0.0) result += 360.0;
    return result == 360.0 ? 0.0 : result;
}

void require_oklch(Oklch color) {
    require_finite(color.lightness, "OKLCH lightness");
    require_finite(color.chroma, "OKLCH chroma");
    require_finite(color.hue_degrees, "OKLCH hue");
    require_alpha(color.alpha, "OKLCH alpha");
    if (color.chroma < 0.0) {
        throw std::invalid_argument("OKLCH chroma must be nonnegative");
    }
}

[[nodiscard]] double decode_srgb(double value) noexcept {
    return value <= 0.04045 ? value / 12.92 :
        std::pow((value + 0.055) / 1.055, 2.4);
}

[[nodiscard]] double encode_srgb(double value) noexcept {
    return value <= 0.0031308 ? value * 12.92 :
        1.055 * std::pow(value, 1.0 / 2.4) - 0.055;
}

} // namespace

LinearSrgb srgb_to_linear(Color color) {
    if (color.is_empty()) return {};
    return {decode_srgb(color.red() / 255.0),
            decode_srgb(color.green() / 255.0),
            decode_srgb(color.blue() / 255.0),
            color.alpha() / 255.0};
}

SrgbConversion linear_to_srgb(LinearSrgb color) {
    require_linear(color);
    constexpr double epsilon = 5e-7;
    const bool in_gamut = color.red >= -epsilon && color.red <= 1.0 + epsilon &&
        color.green >= -epsilon && color.green <= 1.0 + epsilon &&
        color.blue >= -epsilon && color.blue <= 1.0 + epsilon;
    const double red = std::clamp(encode_srgb(color.red), 0.0, 1.0);
    const double green = std::clamp(encode_srgb(color.green), 0.0, 1.0);
    const double blue = std::clamp(encode_srgb(color.blue), 0.0, 1.0);
    return {Color::from_argb(normalized_channel(color.alpha),
                             normalized_channel(red),
                             normalized_channel(green),
                             normalized_channel(blue)),
            color, in_gamut, !in_gamut};
}

XyzD65 linear_srgb_to_xyz_d65(LinearSrgb color) {
    require_linear(color);
    return {
        0.4124564 * color.red + 0.3575761 * color.green +
            0.1804375 * color.blue,
        0.2126729 * color.red + 0.7151522 * color.green +
            0.0721750 * color.blue,
        0.0193339 * color.red + 0.1191920 * color.green +
            0.9503041 * color.blue,
        color.alpha};
}

LinearSrgb xyz_d65_to_linear_srgb(XyzD65 color) {
    require_xyz(color);
    return {
         3.2404542 * color.x - 1.5371385 * color.y - 0.4985314 * color.z,
        -0.9692660 * color.x + 1.8760108 * color.y + 0.0415560 * color.z,
         0.0556434 * color.x - 0.2040259 * color.y + 1.0572252 * color.z,
         color.alpha};
}

Oklab linear_srgb_to_oklab(LinearSrgb color) {
    require_linear(color);
    const double l = 0.4122214708 * color.red +
                     0.5363325363 * color.green +
                     0.0514459929 * color.blue;
    const double m = 0.2119034982 * color.red +
                     0.6806995451 * color.green +
                     0.1073969566 * color.blue;
    const double s = 0.0883024619 * color.red +
                     0.2817188376 * color.green +
                     0.6299787005 * color.blue;
    const double l_root = std::cbrt(l);
    const double m_root = std::cbrt(m);
    const double s_root = std::cbrt(s);
    return {
        0.2104542553 * l_root + 0.7936177850 * m_root -
            0.0040720468 * s_root,
        1.9779984951 * l_root - 2.4285922050 * m_root +
            0.4505937099 * s_root,
        0.0259040371 * l_root + 0.7827717662 * m_root -
            0.8086757660 * s_root,
        color.alpha};
}

LinearSrgb oklab_to_linear_srgb(Oklab color) {
    require_oklab(color);
    const double l_root = color.lightness + 0.3963377774 * color.a +
                          0.2158037573 * color.b;
    const double m_root = color.lightness - 0.1055613458 * color.a -
                          0.0638541728 * color.b;
    const double s_root = color.lightness - 0.0894841775 * color.a -
                          1.2914855480 * color.b;
    const double l = l_root * l_root * l_root;
    const double m = m_root * m_root * m_root;
    const double s = s_root * s_root * s_root;
    return {
         4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s,
        -1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s,
        -0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s,
        color.alpha};
}

Oklab xyz_d65_to_oklab(XyzD65 color) {
    return linear_srgb_to_oklab(xyz_d65_to_linear_srgb(color));
}

XyzD65 oklab_to_xyz_d65(Oklab color) {
    return linear_srgb_to_xyz_d65(oklab_to_linear_srgb(color));
}

Oklch oklab_to_oklch(Oklab color) {
    require_oklab(color);
    const double chroma = std::hypot(color.a, color.b);
    const double hue = chroma <= 1e-15 ? 0.0 : normalized_hue(
        std::atan2(color.b, color.a) * 180.0 / std::acos(-1.0));
    return {color.lightness, chroma, hue, color.alpha};
}

Oklab oklch_to_oklab(Oklch color) {
    require_oklch(color);
    const double radians = normalized_hue(color.hue_degrees) *
                           std::acos(-1.0) / 180.0;
    return {color.lightness, color.chroma * std::cos(radians),
            color.chroma * std::sin(radians), color.alpha};
}

SrgbConversion oklch_to_srgb(Oklch color) {
    return linear_to_srgb(oklab_to_linear_srgb(oklch_to_oklab(color)));
}

OklchGamutMapping map_oklch_to_srgb_gamut(Oklch color) {
    require_oklch(color);
    color.hue_degrees = normalized_hue(color.hue_degrees);
    SrgbConversion requested = oklch_to_srgb(color);
    if (requested.in_gamut) return {color, color, requested};

    Oklch neutral = color;
    neutral.chroma = 0.0;
    SrgbConversion neutral_srgb = oklch_to_srgb(neutral);
    if (!neutral_srgb.in_gamut) {
        return {color, neutral, neutral_srgb};
    }

    double low = 0.0;
    double high = color.chroma;
    Oklch mapped = neutral;
    SrgbConversion mapped_srgb = neutral_srgb;
    for (std::size_t iteration = 0; iteration < 24U; ++iteration) {
        const double candidate_chroma = (low + high) * 0.5;
        Oklch candidate = color;
        candidate.chroma = candidate_chroma;
        SrgbConversion converted = oklch_to_srgb(candidate);
        if (converted.in_gamut) {
            low = candidate_chroma;
            mapped = candidate;
            mapped_srgb = converted;
        } else {
            high = candidate_chroma;
        }
    }
    return {color, mapped, mapped_srgb};
}

const SystemPalette& default_system_palette() noexcept {
    static const SystemPalette palette{};
    return palette;
}

Matrix Matrix::translation(double x, double y) {
    require_finite(x, "translation x");
    require_finite(y, "translation y");
    return {1.0, 0.0, 0.0, 1.0, x, y};
}

Matrix Matrix::rotation_at(double degrees, PointF center) {
    require_finite(degrees, "rotation degrees");
    require_finite(center, "rotation center");
    const double radians = degrees * (std::acos(-1.0) / 180.0);
    const double cosine = std::cos(radians);
    const double sine = std::sin(radians);
    const Matrix rotation{cosine, sine, -sine, cosine, 0.0, 0.0};
    return Matrix::translation(-center.x, -center.y)
        .followed_by(rotation)
        .followed_by(Matrix::translation(center.x, center.y));
}

bool Matrix::finite() const noexcept {
    return std::isfinite(m11_) && std::isfinite(m12_) &&
           std::isfinite(m21_) && std::isfinite(m22_) &&
           std::isfinite(dx_) && std::isfinite(dy_);
}

PointF Matrix::transform(PointF point) const {
    if (!finite()) throw std::logic_error("matrix is not finite");
    require_finite(point, "transformed point");
    const PointF result{point.x * m11_ + point.y * m21_ + dx_,
                        point.x * m12_ + point.y * m22_ + dy_};
    require_finite(result, "matrix result");
    return result;
}

RectF Matrix::transform_bounds(RectF rect) const {
    require_finite(rect, "transformed rectangle");
    const PointF points[] = {
        transform({rect.left(), rect.top()}), transform({rect.right(), rect.top()}),
        transform({rect.right(), rect.bottom()}), transform({rect.left(), rect.bottom()}),
    };
    double left = points[0].x;
    double top = points[0].y;
    double right = points[0].x;
    double bottom = points[0].y;
    for (const PointF point : points) {
        left = std::min(left, point.x);
        top = std::min(top, point.y);
        right = std::max(right, point.x);
        bottom = std::max(bottom, point.y);
    }
    return {left, top, right - left, bottom - top};
}

Matrix Matrix::followed_by(const Matrix& next) const {
    if (!finite() || !next.finite()) {
        throw std::invalid_argument("matrix composition requires finite matrices");
    }
    const Matrix result{
        m11_ * next.m11_ + m12_ * next.m21_,
        m11_ * next.m12_ + m12_ * next.m22_,
        m21_ * next.m11_ + m22_ * next.m21_,
        m21_ * next.m12_ + m22_ * next.m22_,
        dx_ * next.m11_ + dy_ * next.m21_ + next.dx_,
        dx_ * next.m12_ + dy_ * next.m22_ + next.dy_,
    };
    if (!result.finite()) throw std::overflow_error("matrix composition overflowed");
    return result;
}

DrawingObject::DrawingObject() : owner_thread_(std::this_thread::get_id()) {}
DrawingObject::~DrawingObject() = default;

void DrawingObject::dispose() {
    verify_access();
    if (state_ == ObjectState::disposed) return;
    state_ = ObjectState::disposing;
    on_dispose();
    state_ = ObjectState::disposed;
}

ObjectState DrawingObject::state() const {
    verify_access();
    return state_;
}

bool DrawingObject::is_disposed() const {
    verify_access();
    return state_ == ObjectState::disposed;
}

void DrawingObject::verify_access() const {
    if (std::this_thread::get_id() != owner_thread_) {
        throw std::logic_error("GUI.Drawing object accessed from a non-owner thread");
    }
}

void DrawingObject::require_alive() const {
    verify_access();
    if (state_ != ObjectState::alive) {
        throw std::logic_error("GUI.Drawing object is disposed");
    }
}

void DrawingObject::handoff_to_current_thread() {
    owner_thread_ = std::this_thread::get_id();
}

void DrawingObject::on_dispose() noexcept {}

SolidBrush::SolidBrush(Color color) : color_(color) {}

Color SolidBrush::color() const {
    require_alive();
    return color_;
}

BrushSnapshot SolidBrush::snapshot() const {
    require_alive();
    BrushSnapshot result;
    result.kind = BrushKind::solid;
    result.primary = color_;
    return result;
}

HatchBrush::HatchBrush(HatchStyle style, Color foreground, Color background)
    : style_(style), foreground_(foreground), background_(background) {
    require_enum(style, 5U, "hatch style");
}

BrushSnapshot HatchBrush::snapshot() const {
    require_alive();
    BrushSnapshot result;
    result.kind = BrushKind::hatch;
    result.primary = foreground_;
    result.secondary = background_;
    result.hatch_style = style_;
    return result;
}

LinearGradientBrush::LinearGradientBrush(RectF bounds, Color first, Color second,
                                         double angle, WrapMode wrap_mode)
    : bounds_(bounds), first_(first), second_(second), angle_(angle),
      wrap_mode_(wrap_mode) {
    require_finite(bounds, "linear gradient bounds");
    require_finite(angle, "linear gradient angle");
    require_enum(wrap_mode, 4U, "linear gradient wrap mode");
    if (bounds.empty() || std::abs(angle) > 1'000'000.0) {
        throw std::invalid_argument("linear gradient geometry must be nonempty and bounded");
    }
}

void LinearGradientBrush::set_blend(std::span<const double> factors,
                                    std::span<const double> positions) {
    require_alive();
    require_blend_positions(positions, factors.size());
    std::vector<Color> colors;
    colors.reserve(factors.size());
    for (const double factor : factors) {
        require_finite(factor, "gradient blend factor");
        if (factor < 0.0 || factor > 1.0) {
            throw std::invalid_argument("gradient blend factors must be in the unit interval");
        }
        colors.push_back(interpolate_color(first_, second_, factor));
    }
    colors_ = std::move(colors);
    positions_.assign(positions.begin(), positions.end());
}

void LinearGradientBrush::set_interpolation_colors(const ColorBlend& blend) {
    require_alive();
    require_blend_positions(blend.positions, blend.colors.size());
    colors_ = blend.colors;
    positions_ = blend.positions;
}

void LinearGradientBrush::set_wrap_mode(WrapMode mode) {
    require_alive();
    require_enum(mode, 4U, "linear gradient wrap mode");
    wrap_mode_ = mode;
}

BrushSnapshot LinearGradientBrush::snapshot() const {
    require_alive();
    BrushSnapshot result;
    result.kind = BrushKind::linear_gradient;
    result.primary = first_;
    result.secondary = second_;
    result.wrap_mode = wrap_mode_;
    result.bounds = bounds_;
    result.angle = angle_;
    result.colors = colors_.empty() ? std::vector<Color>{first_, second_} : colors_;
    result.positions = positions_.empty() ? std::vector<double>{0.0, 1.0} : positions_;
    return result;
}

PathGradientBrush::PathGradientBrush(std::span<const PointF> points,
                                     WrapMode wrap_mode)
    : points_(points.begin(), points.end()), wrap_mode_(wrap_mode) {
    require_enum(wrap_mode, 4U, "path gradient wrap mode");
    if (points.size() < 3U || points.size() > GraphicsPath::maximum_elements) {
        throw std::invalid_argument("path gradient requires 3 through 1000000 points");
    }
    for (const PointF point : points_) {
        require_finite(point, "path gradient point");
        center_.x += point.x / static_cast<double>(points_.size());
        center_.y += point.y / static_cast<double>(points_.size());
    }
    require_finite(center_, "path gradient center");
}

void PathGradientBrush::set_center_color(Color color) {
    require_alive();
    center_color_ = color;
}

void PathGradientBrush::set_center_point(PointF point) {
    require_alive();
    require_finite(point, "path gradient center");
    center_ = point;
}

void PathGradientBrush::set_surround_colors(std::span<const Color> colors) {
    require_alive();
    if (colors.empty() || colors.size() > points_.size()) {
        throw std::invalid_argument(
            "path gradient surround colors require 1 through point-count entries");
    }
    surround_colors_.assign(colors.begin(), colors.end());
}

void PathGradientBrush::set_interpolation_colors(const ColorBlend& blend) {
    require_alive();
    require_blend_positions(blend.positions, blend.colors.size());
    colors_ = blend.colors;
    positions_ = blend.positions;
}

BrushSnapshot PathGradientBrush::snapshot() const {
    require_alive();
    BrushSnapshot result;
    result.kind = BrushKind::path_gradient;
    result.primary = center_color_;
    result.secondary = surround_colors_.front();
    result.wrap_mode = wrap_mode_;
    result.center = center_;
    result.points = points_;
    result.colors = colors_.empty() ?
        std::vector<Color>{center_color_, surround_colors_.front()} : colors_;
    result.positions = positions_.empty() ?
        std::vector<double>{0.0, 1.0} : positions_;
    return result;
}

Pen::Pen(Color color, double width) : color_(color), width_(width) {
    require_pen_width(width);
}

Pen::Pen(const Brush& brush, double width)
    : color_(brush.snapshot().primary), width_(width) {
    require_pen_width(width);
}

void Pen::set_width(double width) {
    require_alive();
    require_pen_width(width);
    width_ = width;
}

void Pen::set_dash_style(DashStyle style) {
    require_alive();
    require_enum(style, 5U, "dash style");
    dash_style_ = style;
    if (style != DashStyle::custom) dash_pattern_.clear();
}

void Pen::set_dash_pattern(std::span<const double> pattern) {
    require_alive();
    if (pattern.empty() || pattern.size() > 256) {
        throw std::invalid_argument("dash pattern must contain 1 through 256 entries");
    }
    std::vector<double> copy;
    copy.reserve(pattern.size());
    for (const double entry : pattern) {
        require_finite(entry, "dash pattern entry");
        if (entry <= 0.0) {
            throw std::invalid_argument("dash pattern entries must be positive");
        }
        copy.push_back(entry);
    }
    dash_pattern_ = std::move(copy);
    dash_style_ = DashStyle::custom;
}

PenSnapshot Pen::snapshot() const {
    require_alive();
    return {color_, width_, dash_style_, dash_pattern_};
}

Font::Font(std::string family, double size, FontStyle style,
           GraphicsUnit unit, std::uint8_t charset) {
    require_finite(size, "font size");
    if (family.empty() || family.find('\0') != std::string::npos ||
        family.size() > 4096 || !valid_utf8(family)) {
        throw std::invalid_argument("font family must be bounded valid UTF-8 without NUL");
    }
    if (size <= 0.0 || size > 1'000'000.0) {
        throw std::invalid_argument("font size must be positive and bounded");
    }
    if ((static_cast<std::uint32_t>(style) & ~UINT32_C(0x0f)) != 0U) {
        throw std::invalid_argument("font style contains undeclared bits");
    }
    require_enum(unit, 6U, "graphics unit");
    value_ = {std::move(family), size, static_cast<std::uint32_t>(style), unit, charset};
}

Font::Font(const Font& source, FontStyle style) : value_(source.snapshot()) {
    value_.style = static_cast<std::uint32_t>(style);
}

FontSnapshot Font::snapshot() const {
    require_alive();
    return value_;
}

std::int32_t Font::deterministic_height() const {
    require_alive();
    return clamp_i32(static_cast<std::int64_t>(std::ceil(value_.size * 1.2)));
}

StringFormat::StringFormat(std::uint32_t flags) { value_.flags = flags; }
StringFormat::StringFormat(const StringFormat& source) : value_(source.snapshot()) {}

void StringFormat::set_alignment(StringAlignment alignment) {
    require_alive();
    require_enum(alignment, 2U, "string alignment");
    value_.alignment = alignment;
}

void StringFormat::set_line_alignment(StringAlignment alignment) {
    require_alive();
    require_enum(alignment, 2U, "line alignment");
    value_.line_alignment = alignment;
}

void StringFormat::set_trimming(StringTrimming trimming) {
    require_alive();
    require_enum(trimming, 5U, "string trimming");
    value_.trimming = trimming;
}

void StringFormat::set_flags(std::uint32_t flags) {
    require_alive();
    value_.flags = flags;
}

StringFormatSnapshot StringFormat::snapshot() const {
    require_alive();
    return value_;
}

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

ImageReference::ImageReference(std::uint64_t stable_id, std::uint32_t width,
                               std::uint32_t height, PixelFormat pixel_format,
                               std::uint64_t generation) {
    require_enum(pixel_format, 1U, "image pixel format");
    if (stable_id == 0U || generation == 0U || width == 0U || height == 0U ||
        width > 32768U || height > 32768U) {
        throw std::invalid_argument(
            "image reference requires nonzero identity, generation, and bounded dimensions");
    }
    value_.stable_id = stable_id;
    value_.width = width;
    value_.height = height;
    value_.pixel_format = pixel_format;
    value_.generation = generation;
}

ImageSnapshot ImageReference::snapshot() const {
    require_alive();
    return value_;
}

bool ImageSnapshot::has_pixels() const noexcept {
    return storage_ != nullptr;
}

std::size_t ImageSnapshot::row_bytes() const noexcept {
    return storage_ ? storage_->row_bytes : 0U;
}

std::span<const std::byte> ImageSnapshot::pixels() const noexcept {
    return storage_ ? std::span<const std::byte>(storage_->bytes) :
                      std::span<const std::byte>{};
}

void ImageAttributes::set_color_matrix(std::span<const double, 25> matrix) {
    require_alive();
    for (const double entry : matrix) require_finite(entry, "color matrix entry");
    std::copy(matrix.begin(), matrix.end(), value_.color_matrix.begin());
    value_.has_color_matrix = true;
}

void ImageAttributes::reset_color_matrix() {
    require_alive();
    value_.has_color_matrix = false;
    value_.color_matrix = {};
}

void ImageAttributes::set_remap_table(
    std::span<const ImageAttributesSnapshot::ColorRemap> table) {
    require_alive();
    if (table.size() > 4096U) {
        throw std::length_error("image color remap table limit exceeded");
    }
    std::vector<ImageAttributesSnapshot::ColorRemap> copy;
    copy.reserve(table.size());
    for (const auto& entry : table) {
        if (entry.old_color.is_empty() || entry.new_color.is_empty()) {
            throw std::invalid_argument("image color remaps require concrete colors");
        }
        const auto duplicate = std::find_if(copy.begin(), copy.end(),
            [&](const auto& existing) {
                return existing.old_color.argb() == entry.old_color.argb();
            });
        if (duplicate != copy.end()) {
            throw std::invalid_argument("image color remap source colors must be unique");
        }
        copy.push_back(entry);
    }
    value_.remap_table = std::move(copy);
}

void ImageAttributes::reset_remap_table() {
    require_alive();
    value_.remap_table.clear();
}

std::unique_ptr<ImageAttributes> ImageAttributes::clone() const {
    require_alive();
    auto result = std::make_unique<ImageAttributes>();
    result->value_ = value_;
    return result;
}

ImageAttributesSnapshot ImageAttributes::snapshot() const {
    require_alive();
    return value_;
}

Bitmap::Bitmap(std::uint32_t width, std::uint32_t height,
               PixelFormat pixel_format) {
    require_enum(pixel_format, 1U, "bitmap pixel format");
    if (width == 0U || height == 0U || width > maximum_dimension ||
        height > maximum_dimension) {
        throw std::invalid_argument("bitmap dimensions must be nonzero and bounded");
    }
    const std::uint64_t row_bytes = static_cast<std::uint64_t>(width) * 4ULL;
    const std::uint64_t byte_count = row_bytes * height;
    if (byte_count > maximum_bytes ||
        byte_count > std::numeric_limits<std::size_t>::max()) {
        throw std::length_error("bitmap byte limit exceeded");
    }
    storage_ = std::make_shared<PixelStorage>();
    storage_->width = width;
    storage_->height = height;
    storage_->pixel_format = pixel_format;
    storage_->row_bytes = static_cast<std::size_t>(row_bytes);
    storage_->bytes.resize(static_cast<std::size_t>(byte_count));
    damage_history_.reserve(maximum_damage_history);
    stable_id_ = next_bitmap_id.fetch_add(1U, std::memory_order_relaxed);
    if (stable_id_ == 0U) {
        throw std::overflow_error("bitmap identity space exhausted");
    }
}

std::uint32_t Bitmap::width() const {
    require_alive();
    return storage_->width;
}

std::uint32_t Bitmap::height() const {
    require_alive();
    return storage_->height;
}

PixelFormat Bitmap::pixel_format() const {
    require_alive();
    return storage_->pixel_format;
}

std::uint64_t Bitmap::generation() const {
    require_alive();
    return generation_;
}

Color Bitmap::get_pixel(std::uint32_t x, std::uint32_t y) const {
    require_alive();
    require_unlocked();
    require_coordinate(x, y);
    return load_color(*storage_, x, y);
}

void Bitmap::set_pixel(std::uint32_t x, std::uint32_t y, Color color) {
    require_alive();
    require_unlocked();
    require_coordinate(x, y);
    if (stored_color_equals(*storage_, x, y, color)) return;
    prepare_write();
    store_color(*storage_, x, y, color);
    publish_mutation({RectI{static_cast<std::int32_t>(x),
                            static_cast<std::int32_t>(y), 1, 1}});
}

void Bitmap::make_transparent(Color key) {
    require_alive();
    require_unlocked();
    bool found = false;
    for (std::uint32_t y = 0; y < storage_->height && !found; ++y) {
        for (std::uint32_t x = 0; x < storage_->width; ++x) {
            const Color value = load_color(*storage_, x, y);
            if (value.alpha() != 0U && value.red() == key.red() &&
                value.green() == key.green() &&
                value.blue() == key.blue()) {
                found = true;
                break;
            }
        }
    }
    if (!found) return;

    prepare_write();
    std::vector<RectI> damage;
    for (std::uint32_t y = 0; y < storage_->height; ++y) {
        std::optional<std::uint32_t> run_start;
        for (std::uint32_t x = 0; x < storage_->width; ++x) {
            const Color value = load_color(*storage_, x, y);
            if (value.alpha() != 0U && value.red() == key.red() &&
                value.green() == key.green() &&
                value.blue() == key.blue()) {
                store_color(*storage_, x, y, Color::from_argb(0U, 0U, 0U, 0U));
                if (!run_start) run_start = x;
            } else if (run_start) {
                damage.push_back({static_cast<std::int32_t>(*run_start),
                                  static_cast<std::int32_t>(y),
                                  static_cast<std::int32_t>(x - *run_start), 1});
                run_start.reset();
            }
        }
        if (run_start) {
            damage.push_back({static_cast<std::int32_t>(*run_start),
                              static_cast<std::int32_t>(y),
                              static_cast<std::int32_t>(storage_->width - *run_start),
                              1});
        }
        if (damage.size() > maximum_damage_rectangles) {
            damage.assign(1U, RectI{0, 0,
                static_cast<std::int32_t>(storage_->width),
                static_cast<std::int32_t>(storage_->height)});
            for (++y; y < storage_->height; ++y) {
                for (std::uint32_t x = 0; x < storage_->width; ++x) {
                    const Color value = load_color(*storage_, x, y);
                    if (value.alpha() != 0U && value.red() == key.red() &&
                        value.green() == key.green() &&
                        value.blue() == key.blue()) {
                        store_color(*storage_, x, y,
                                    Color::from_argb(0U, 0U, 0U, 0U));
                    }
                }
            }
            break;
        }
    }
    publish_mutation(std::move(damage));
}

std::unique_ptr<Bitmap> Bitmap::clone(RectI source) const {
    require_alive();
    require_unlocked();
    if (source.width <= 0 || source.height <= 0 || source.x < 0 || source.y < 0 ||
        source.right() > storage_->width || source.bottom() > storage_->height) {
        throw std::invalid_argument("bitmap clone rectangle is outside the image");
    }
    auto result = std::make_unique<Bitmap>(static_cast<std::uint32_t>(source.width),
                                           static_cast<std::uint32_t>(source.height),
                                           storage_->pixel_format);
    for (std::int32_t y = 0; y < source.height; ++y) {
        const std::size_t input = pixel_offset(
            *storage_, static_cast<std::uint32_t>(source.x),
            static_cast<std::uint32_t>(source.y + y));
        const std::size_t output = static_cast<std::size_t>(y) * result->storage_->row_bytes;
        std::copy_n(storage_->bytes.begin() + static_cast<std::ptrdiff_t>(input),
                    result->storage_->row_bytes,
                    result->storage_->bytes.begin() + static_cast<std::ptrdiff_t>(output));
    }
    return result;
}

std::unique_ptr<Bitmap> Bitmap::thumbnail(std::uint32_t width,
                                          std::uint32_t height) const {
    require_alive();
    require_unlocked();
    auto result = std::make_unique<Bitmap>(width, height, storage_->pixel_format);
    for (std::uint32_t y = 0; y < height; ++y) {
        const std::uint32_t source_y = std::min(
            storage_->height - 1U,
            static_cast<std::uint32_t>((static_cast<std::uint64_t>(y) *
                                        storage_->height) / height));
        for (std::uint32_t x = 0; x < width; ++x) {
            const std::uint32_t source_x = std::min(
                storage_->width - 1U,
                static_cast<std::uint32_t>((static_cast<std::uint64_t>(x) *
                                            storage_->width) / width));
            store_color(*result->storage_, x, y,
                        load_color(*storage_, source_x, source_y));
        }
    }
    return result;
}

std::unique_ptr<Bitmap> Bitmap::adjusted(
    const ImageAttributes& attributes) const {
    require_alive();
    require_unlocked();
    const ImageAttributesSnapshot adjustment = attributes.snapshot();
    auto result = std::make_unique<Bitmap>(storage_->width, storage_->height,
                                           storage_->pixel_format);
    for (std::uint32_t y = 0; y < storage_->height; ++y) {
        for (std::uint32_t x = 0; x < storage_->width; ++x) {
            Color input = load_color(*storage_, x, y);
            const auto remap = std::find_if(
                adjustment.remap_table.begin(), adjustment.remap_table.end(),
                [&](const auto& entry) {
                    return entry.old_color.argb() == input.argb();
                });
            if (remap != adjustment.remap_table.end()) input = remap->new_color;
            if (!adjustment.has_color_matrix) {
                store_color(*result->storage_, x, y, input);
                continue;
            }
            const auto& matrix = adjustment.color_matrix;
            const double values[5] = {
                input.red() / 255.0, input.green() / 255.0,
                input.blue() / 255.0, input.alpha() / 255.0, 1.0};
            double output[4]{};
            for (std::size_t column = 0; column < 4U; ++column) {
                for (std::size_t row = 0; row < 5U; ++row) {
                    output[column] += values[row] * matrix[row * 5U + column];
                }
            }
            store_color(*result->storage_, x, y,
                        Color::from_argb(normalized_channel(output[3]),
                                         normalized_channel(output[0]),
                                         normalized_channel(output[1]),
                                         normalized_channel(output[2])));
        }
    }
    return result;
}

BitmapLockView Bitmap::lock(BitmapLockMode mode) {
    require_alive();
    require_enum(mode, 2U, "bitmap lock mode");
    require_unlocked();
    if (mode != BitmapLockMode::read) prepare_write();
    active_lock_token_ = next_lock_token_++;
    if (active_lock_token_ == 0U) {
        throw std::overflow_error("bitmap lock token space exhausted");
    }
    lock_mode_ = mode;
    return {storage_->bytes.data(),
            mode == BitmapLockMode::read ? nullptr : storage_->bytes.data(),
            storage_->row_bytes, storage_->width, storage_->height,
            storage_->pixel_format, active_lock_token_};
}

void Bitmap::unlock(std::uint64_t token) {
    require_alive();
    if (active_lock_token_ == 0U || token == 0U || token != active_lock_token_) {
        throw std::invalid_argument("bitmap lock token is invalid or already released");
    }
    if (active_edit_) {
        throw std::logic_error(
            "a bounded bitmap edit must be committed or cancelled explicitly");
    }
    const bool wrote = lock_mode_ != BitmapLockMode::read;
    active_lock_token_ = 0U;
    lock_mode_ = BitmapLockMode::read;
    if (wrote) {
        publish_mutation({RectI{0, 0,
            static_cast<std::int32_t>(storage_->width),
            static_cast<std::int32_t>(storage_->height)}});
    }
}

bool Bitmap::locked() const {
    require_alive();
    return active_lock_token_ != 0U;
}

BitmapEditView Bitmap::begin_edit(RectI bounds) {
    require_alive();
    require_unlocked();
    if (bounds.x < 0 || bounds.y < 0 || bounds.width <= 0 ||
        bounds.height <= 0 || bounds.right() > storage_->width ||
        bounds.bottom() > storage_->height) {
        throw std::invalid_argument(
            "bitmap edit rectangle must be nonempty and inside the image");
    }
    prepare_write();

    const std::size_t edit_row_bytes =
        static_cast<std::size_t>(bounds.width) * 4U;
    active_edit_backup_.resize(
        edit_row_bytes * static_cast<std::size_t>(bounds.height));
    for (std::int32_t row = 0; row < bounds.height; ++row) {
        const std::size_t source = pixel_offset(
            *storage_, static_cast<std::uint32_t>(bounds.x),
            static_cast<std::uint32_t>(bounds.y + row));
        std::copy_n(storage_->bytes.data() + source, edit_row_bytes,
                    active_edit_backup_.data() +
                        static_cast<std::size_t>(row) * edit_row_bytes);
    }

    active_lock_token_ = next_lock_token_++;
    if (active_lock_token_ == 0U) {
        active_edit_backup_.clear();
        throw std::overflow_error("bitmap lock token space exhausted");
    }
    active_edit_ = true;
    active_edit_bounds_ = bounds;
    lock_mode_ = BitmapLockMode::read_write;
    const std::size_t origin = pixel_offset(
        *storage_, static_cast<std::uint32_t>(bounds.x),
        static_cast<std::uint32_t>(bounds.y));
    return {storage_->bytes.data() + origin, storage_->bytes.data() + origin,
            storage_->row_bytes, bounds, storage_->pixel_format,
            active_lock_token_};
}

std::uint64_t Bitmap::commit_edit(std::uint64_t token) {
    require_alive();
    require_edit_token(token);
    const std::size_t edit_row_bytes =
        static_cast<std::size_t>(active_edit_bounds_.width) * 4U;
    std::vector<RectI> damage;
    bool compacted = false;

    for (std::int32_t row = 0; row < active_edit_bounds_.height; ++row) {
        const std::size_t storage_row = pixel_offset(
            *storage_, static_cast<std::uint32_t>(active_edit_bounds_.x),
            static_cast<std::uint32_t>(active_edit_bounds_.y + row));
        const std::byte* current = storage_->bytes.data() + storage_row;
        const std::byte* original = active_edit_backup_.data() +
            static_cast<std::size_t>(row) * edit_row_bytes;
        std::optional<std::int32_t> run_start;
        for (std::int32_t column = 0; column < active_edit_bounds_.width;
             ++column) {
            const std::size_t pixel = static_cast<std::size_t>(column) * 4U;
            const bool changed = !std::equal(current + pixel,
                                             current + pixel + 4U,
                                             original + pixel);
            if (changed && !run_start) {
                run_start = column;
            } else if (!changed && run_start) {
                const RectI span{active_edit_bounds_.x + *run_start,
                                 active_edit_bounds_.y + row,
                                 column - *run_start, 1};
                if (!damage.empty() && damage.back().x == span.x &&
                    damage.back().width == span.width &&
                    damage.back().bottom() == span.y) {
                    ++damage.back().height;
                } else {
                    damage.push_back(span);
                }
                run_start.reset();
            }
        }
        if (run_start) {
            const RectI span{active_edit_bounds_.x + *run_start,
                             active_edit_bounds_.y + row,
                             active_edit_bounds_.width - *run_start, 1};
            if (!damage.empty() && damage.back().x == span.x &&
                damage.back().width == span.width &&
                damage.back().bottom() == span.y) {
                ++damage.back().height;
            } else {
                damage.push_back(span);
            }
        }
        if (damage.size() > maximum_damage_rectangles) {
            compacted = true;
            break;
        }
    }

    if (compacted) {
        damage.assign(1U, active_edit_bounds_);
    }
    finish_edit();
    if (!damage.empty()) publish_mutation(std::move(damage));
    return generation_;
}

void Bitmap::cancel_edit(std::uint64_t token) {
    require_alive();
    require_edit_token(token);
    const std::size_t edit_row_bytes =
        static_cast<std::size_t>(active_edit_bounds_.width) * 4U;
    for (std::int32_t row = 0; row < active_edit_bounds_.height; ++row) {
        const std::size_t destination = pixel_offset(
            *storage_, static_cast<std::uint32_t>(active_edit_bounds_.x),
            static_cast<std::uint32_t>(active_edit_bounds_.y + row));
        std::copy_n(active_edit_backup_.data() +
                        static_cast<std::size_t>(row) * edit_row_bytes,
                    edit_row_bytes, storage_->bytes.data() + destination);
    }
    finish_edit();
}

BitmapDamageSnapshot Bitmap::changes_since(std::uint64_t generation) const {
    require_alive();
    require_unlocked();
    if (generation > generation_) {
        throw std::invalid_argument(
            "bitmap damage generation is newer than the bitmap");
    }
    BitmapDamageSnapshot result{generation, generation_, true, {}};
    if (generation == generation_) return result;

    if (damage_history_.empty() ||
        generation < damage_history_.front().generation - 1U) {
        result.history_complete = false;
        result.rectangles.push_back({0, 0,
            static_cast<std::int32_t>(storage_->width),
            static_cast<std::int32_t>(storage_->height)});
        return result;
    }

    for (const DamageRecord& record : damage_history_) {
        if (record.generation <= generation) continue;
        if (result.rectangles.size() + record.rectangles.size() >
            maximum_damage_rectangles) {
            result.rectangles.assign(1U, RectI{0, 0,
                static_cast<std::int32_t>(storage_->width),
                static_cast<std::int32_t>(storage_->height)});
            return result;
        }
        result.rectangles.insert(result.rectangles.end(),
                                 record.rectangles.begin(),
                                 record.rectangles.end());
    }
    return result;
}

ImageSnapshot Bitmap::snapshot() const {
    require_alive();
    require_unlocked();
    ImageSnapshot result;
    result.stable_id = stable_id_;
    result.width = storage_->width;
    result.height = storage_->height;
    result.pixel_format = storage_->pixel_format;
    result.generation = generation_;
    result.storage_ = storage_;
    return result;
}

TextureBrush::TextureBrush(const Bitmap& image, WrapMode wrap_mode) {
    require_enum(wrap_mode, 4U, "texture wrap mode");
    value_.kind = BrushKind::texture;
    value_.wrap_mode = wrap_mode;
    value_.image = image.snapshot();
}

TextureBrush::TextureBrush(const ImageReference& image, WrapMode wrap_mode) {
    require_enum(wrap_mode, 4U, "texture wrap mode");
    value_.kind = BrushKind::texture;
    value_.wrap_mode = wrap_mode;
    value_.image = image.snapshot();
}

TextureBrush::TextureBrush(BrushSnapshot value) : value_(std::move(value)) {}

void TextureBrush::set_wrap_mode(WrapMode mode) {
    require_alive();
    require_enum(mode, 4U, "texture wrap mode");
    value_.wrap_mode = mode;
}

void TextureBrush::set_transform(Matrix transform) {
    require_alive();
    if (!transform.finite()) {
        throw std::invalid_argument("texture transform must be finite");
    }
    value_.transform = transform;
}

void TextureBrush::reset_transform() {
    require_alive();
    value_.transform = Matrix{};
}

void TextureBrush::translate_transform(double x, double y) {
    require_alive();
    value_.transform = value_.transform.followed_by(Matrix::translation(x, y));
}

void TextureBrush::scale_transform(double x, double y) {
    require_alive();
    require_finite(x, "texture scale x");
    require_finite(y, "texture scale y");
    if (x == 0.0 || y == 0.0) {
        throw std::invalid_argument("texture scale must be nonzero");
    }
    value_.transform = value_.transform.followed_by(
        Matrix{x, 0.0, 0.0, y, 0.0, 0.0});
}

void TextureBrush::rotate_transform(double degrees) {
    require_alive();
    value_.transform = value_.transform.followed_by(
        Matrix::rotation_at(degrees, {0.0, 0.0}));
}

std::unique_ptr<TextureBrush> TextureBrush::clone() const {
    require_alive();
    return std::unique_ptr<TextureBrush>(new TextureBrush(value_));
}

BrushSnapshot TextureBrush::snapshot() const {
    require_alive();
    return value_;
}

void Bitmap::on_dispose() noexcept {
    active_lock_token_ = 0U;
    active_edit_ = false;
    active_edit_backup_.clear();
    damage_history_.clear();
    storage_.reset();
}

void Bitmap::require_unlocked() const {
    if (active_lock_token_ != 0U) {
        throw std::logic_error("bitmap operation is unavailable during a pixel lock");
    }
}

void Bitmap::require_coordinate(std::uint32_t x, std::uint32_t y) const {
    if (x >= storage_->width || y >= storage_->height) {
        throw std::out_of_range("bitmap pixel coordinate is outside the image");
    }
}

void Bitmap::require_edit_token(std::uint64_t token) const {
    if (!active_edit_ || active_lock_token_ == 0U || token == 0U ||
        token != active_lock_token_) {
        throw std::invalid_argument(
            "bitmap edit token is invalid or already released");
    }
}

void Bitmap::prepare_write() {
    if (generation_ == std::numeric_limits<std::uint64_t>::max()) {
        throw std::overflow_error("bitmap generation space exhausted");
    }
    if (storage_.use_count() != 1) {
        storage_ = std::make_shared<PixelStorage>(*storage_);
    }
}

void Bitmap::publish_mutation(std::vector<RectI> damage) {
    if (damage.empty()) return;
    if (damage_history_.size() == maximum_damage_history) {
        damage_history_.erase(damage_history_.begin());
    }
    const std::uint64_t next_generation = generation_ + 1U;
    damage_history_.push_back({next_generation, std::move(damage)});
    generation_ = next_generation;
}

void Bitmap::finish_edit() noexcept {
    active_lock_token_ = 0U;
    lock_mode_ = BitmapLockMode::read;
    active_edit_ = false;
    active_edit_bounds_ = {};
    active_edit_backup_.clear();
}

GraphicsStateToken GraphicsRecorder::save() {
    require_recordable();
    if (saved_.size() >= maximum_state_depth) {
        throw std::length_error("GUI.Drawing state stack limit exceeded");
    }
    const GraphicsStateToken token{next_token_++};
    saved_.push_back({token, state_});
    DrawingCommand command;
    command.kind = CommandKind::save;
    command.state = state_;
    command.token = token;
    append(std::move(command));
    return token;
}

void GraphicsRecorder::restore(GraphicsStateToken token) {
    require_recordable();
    const auto found = std::find_if(saved_.rbegin(), saved_.rend(),
                                    [token](const SavedState& value) {
                                        return value.token == token;
                                    });
    if (token.value == 0 || found == saved_.rend()) {
        throw std::invalid_argument("graphics state token is invalid or already restored");
    }
    state_ = found->state;
    saved_.erase(found.base() - 1, saved_.end());
    DrawingCommand command;
    command.kind = CommandKind::restore;
    command.state = state_;
    command.token = token;
    append(std::move(command));
}

void GraphicsRecorder::translate(double x, double y) {
    require_recordable();
    state_.transform = state_.transform.followed_by(Matrix::translation(x, y));
    DrawingCommand command;
    command.kind = CommandKind::translate;
    command.state = state_;
    command.first = {x, y};
    append(std::move(command));
}

void GraphicsRecorder::set_transform(Matrix transform) {
    require_recordable();
    if (!transform.finite()) throw std::invalid_argument("graphics transform must be finite");
    state_.transform = transform;
    DrawingCommand command;
    command.kind = CommandKind::set_transform;
    command.state = state_;
    append(std::move(command));
}

void GraphicsRecorder::set_clip(RectF clip) {
    require_recordable();
    require_finite(clip, "graphics clip");
    state_.clip = state_.transform.transform_bounds(clip);
    DrawingCommand command;
    command.kind = CommandKind::set_clip;
    command.state = state_;
    command.rect = clip;
    append(std::move(command));
}

void GraphicsRecorder::reset_clip() {
    require_recordable();
    state_.clip.reset();
    DrawingCommand command;
    command.kind = CommandKind::reset_clip;
    command.state = state_;
    append(std::move(command));
}

void GraphicsRecorder::set_quality(SmoothingMode smoothing,
                                   InterpolationMode interpolation,
                                   PixelOffsetMode pixel_offset,
                                   CompositingMode compositing,
                                   CompositingQuality compositing_quality) {
    require_recordable();
    require_enum(smoothing, 4U, "smoothing mode");
    require_enum(interpolation, 7U, "interpolation mode");
    require_enum(pixel_offset, 4U, "pixel offset mode");
    require_enum(compositing, 1U, "compositing mode");
    require_enum(compositing_quality, 4U, "compositing quality");
    state_.smoothing = smoothing;
    state_.interpolation = interpolation;
    state_.pixel_offset = pixel_offset;
    state_.compositing = compositing;
    state_.compositing_quality = compositing_quality;
    DrawingCommand command;
    command.kind = CommandKind::set_quality;
    command.state = state_;
    append(std::move(command));
}

GraphicsState GraphicsRecorder::current_state() const {
    require_alive();
    return state_;
}

bool GraphicsRecorder::is_visible(PointF point) const {
    require_alive();
    require_finite(point, "visibility point");
    return !state_.clip || state_.clip->contains(state_.transform.transform(point));
}

SizeF GraphicsRecorder::measure_string(std::string_view utf8, const Font& font,
                                       const StringFormat& format,
                                       const TextMetricsProvider& provider) const {
    require_alive();
    if (utf8.size() > maximum_text_bytes || !valid_utf8(utf8)) {
        throw std::invalid_argument("measured text must be bounded valid UTF-8");
    }
    const SizeF result = provider.measure(utf8, font.snapshot(), format.snapshot());
    if (!std::isfinite(result.width) || !std::isfinite(result.height) ||
        result.width < 0.0 || result.height < 0.0) {
        throw std::logic_error("text metrics provider returned invalid dimensions");
    }
    return result;
}

void GraphicsRecorder::clear(Color color) {
    require_recordable();
    DrawingCommand command;
    command.kind = CommandKind::clear;
    command.state = state_;
    command.color = color;
    append(std::move(command));
}

void GraphicsRecorder::fill_rectangle(const Brush& brush, RectF rect) {
    require_recordable();
    require_finite(rect, "fill rectangle");
    DrawingCommand command;
    command.kind = CommandKind::fill_rectangle;
    command.state = state_;
    command.rect = rect;
    command.brush = brush.snapshot();
    command.color = command.brush.primary;
    append(std::move(command));
}

void GraphicsRecorder::draw_rectangle(const Pen& pen, RectF rect) {
    require_recordable();
    require_finite(rect, "stroke rectangle");
    DrawingCommand command;
    command.kind = CommandKind::draw_rectangle;
    command.state = state_;
    command.rect = rect;
    command.pen = pen.snapshot();
    append(std::move(command));
}

void GraphicsRecorder::draw_line(const Pen& pen, PointF from, PointF to) {
    require_recordable();
    require_finite(from, "line start");
    require_finite(to, "line end");
    DrawingCommand command;
    command.kind = CommandKind::draw_line;
    command.state = state_;
    command.first = from;
    command.second = to;
    command.pen = pen.snapshot();
    append(std::move(command));
}

void GraphicsRecorder::draw_string(std::string_view utf8, const Font& font,
                                   const SolidBrush& brush, PointF origin,
                                   const StringFormat& format) {
    require_recordable();
    require_finite(origin, "text origin");
    if (utf8.size() > maximum_text_bytes || !valid_utf8(utf8)) {
        throw std::invalid_argument("drawn text must be bounded valid UTF-8");
    }
    DrawingCommand command;
    command.kind = CommandKind::draw_string;
    command.state = state_;
    command.first = origin;
    command.color = brush.color();
    command.font = font.snapshot();
    command.format = format.snapshot();
    command.text.assign(utf8);
    append(std::move(command));
}

void GraphicsRecorder::draw_ellipse(const Pen& pen, RectF bounds) {
    require_recordable();
    require_finite(bounds, "stroke ellipse");
    DrawingCommand command;
    command.kind = CommandKind::draw_ellipse;
    command.state = state_;
    command.rect = bounds;
    command.pen = pen.snapshot();
    append(std::move(command));
}

void GraphicsRecorder::fill_ellipse(const Brush& brush, RectF bounds) {
    require_recordable();
    require_finite(bounds, "fill ellipse");
    DrawingCommand command;
    command.kind = CommandKind::fill_ellipse;
    command.state = state_;
    command.rect = bounds;
    command.brush = brush.snapshot();
    command.color = command.brush.primary;
    append(std::move(command));
}

void GraphicsRecorder::fill_polygon(const Brush& brush,
                                    std::span<const PointF> points,
                                    FillMode fill_mode) {
    require_recordable();
    require_enum(fill_mode, 1U, "polygon fill mode");
    if (points.size() < 3U || points.size() > GraphicsPath::maximum_elements) {
        throw std::invalid_argument("polygon must contain 3 through 1000000 points");
    }
    for (const PointF point : points) require_finite(point, "polygon point");
    DrawingCommand command;
    command.kind = CommandKind::fill_polygon;
    command.state = state_;
    command.brush = brush.snapshot();
    command.color = command.brush.primary;
    command.path.fill_mode = fill_mode;
    command.points.assign(points.begin(), points.end());
    append(std::move(command));
}

void GraphicsRecorder::draw_path(const Pen& pen, const GraphicsPath& path) {
    require_recordable();
    DrawingCommand command;
    command.kind = CommandKind::draw_path;
    command.state = state_;
    command.pen = pen.snapshot();
    command.path = path.snapshot();
    append(std::move(command));
}

void GraphicsRecorder::fill_path(const Brush& brush,
                                 const GraphicsPath& path) {
    require_recordable();
    DrawingCommand command;
    command.kind = CommandKind::fill_path;
    command.state = state_;
    command.brush = brush.snapshot();
    command.color = command.brush.primary;
    command.path = path.snapshot();
    append(std::move(command));
}

void GraphicsRecorder::draw_image(const ImageReference& image,
                                  RectF destination, RectF source,
                                  const ImageAttributes& attributes) {
    require_recordable();
    require_finite(destination, "image destination");
    require_finite(source, "image source");
    DrawingCommand command;
    command.kind = CommandKind::draw_image;
    command.state = state_;
    command.rect = destination;
    command.first = {source.x, source.y};
    command.second = {source.width, source.height};
    command.image = image.snapshot();
    command.image_attributes = attributes.snapshot();
    append(std::move(command));
}

void GraphicsRecorder::draw_image(const Bitmap& image, RectF destination,
                                  RectF source,
                                  const ImageAttributes& attributes) {
    require_recordable();
    require_finite(destination, "bitmap destination");
    require_finite(source, "bitmap source");
    DrawingCommand command;
    command.kind = CommandKind::draw_image;
    command.state = state_;
    command.rect = destination;
    command.first = {source.x, source.y};
    command.second = {source.width, source.height};
    command.image = image.snapshot();
    command.image_attributes = attributes.snapshot();
    append(std::move(command));
}

void GraphicsRecorder::close() {
    require_alive();
    closed_ = true;
    saved_.clear();
}

bool GraphicsRecorder::closed() const {
    require_alive();
    return closed_;
}

std::span<const DrawingCommand> GraphicsRecorder::commands() const {
    require_alive();
    return commands_;
}

std::string GraphicsRecorder::deterministic_trace() const {
    require_alive();
    std::string trace = "gui.drawing.trace/v1\n";
    for (std::size_t index = 0; index < commands_.size(); ++index) {
        const DrawingCommand& command = commands_[index];
        trace += std::to_string(index) + "|" + command_name(command.kind);
        switch (command.kind) {
        case CommandKind::save:
        case CommandKind::restore:
            trace += "|token=" + std::to_string(command.token.value);
            break;
        case CommandKind::translate:
            trace += "|offset=" + point_text(command.first);
            break;
        case CommandKind::set_clip:
            trace += "|rect=" + rect_text(command.rect);
            break;
        case CommandKind::clear:
            trace += "|color=" + color_text(command.color);
            break;
        case CommandKind::fill_rectangle:
            trace += "|rect=" + rect_text(command.rect) +
                     "|brush=" + brush_text(command.brush);
            break;
        case CommandKind::draw_rectangle:
            trace += "|rect=" + rect_text(command.rect) +
                     "|pen=" + pen_text(command.pen);
            break;
        case CommandKind::draw_line:
            trace += "|from=" + point_text(command.first) +
                     "|to=" + point_text(command.second) +
                     "|pen=" + pen_text(command.pen);
            break;
        case CommandKind::draw_string:
            trace += "|at=" + point_text(command.first) +
                     "|font=" + escaped(command.font.family) + "," +
                     number(command.font.size) + "," +
                     std::to_string(command.font.style) +
                     "|brush=" + color_text(command.color) +
                     "|format=" + std::to_string(static_cast<unsigned>(command.format.alignment)) +
                     "," + std::to_string(static_cast<unsigned>(command.format.line_alignment)) +
                     "," + std::to_string(static_cast<unsigned>(command.format.trimming)) +
                     "," + std::to_string(command.format.flags) +
                     "|text=" + escaped(command.text);
            break;
        case CommandKind::draw_ellipse:
            trace += "|bounds=" + rect_text(command.rect) +
                     "|pen=" + pen_text(command.pen);
            break;
        case CommandKind::fill_ellipse:
            trace += "|bounds=" + rect_text(command.rect) +
                     "|brush=" + brush_text(command.brush);
            break;
        case CommandKind::fill_polygon:
            trace += "|fill=" +
                     std::to_string(static_cast<unsigned>(command.path.fill_mode)) +
                     "|points=" + points_text(command.points) +
                     "|brush=" + brush_text(command.brush);
            break;
        case CommandKind::draw_path:
            trace += "|path=" + path_text(command.path) +
                     "|pen=" + pen_text(command.pen);
            break;
        case CommandKind::fill_path:
            trace += "|path=" + path_text(command.path) +
                     "|brush=" + brush_text(command.brush);
            break;
        case CommandKind::draw_image:
            trace += "|image=" + std::to_string(command.image.stable_id) + "," +
                     std::to_string(command.image.width) + "," +
                     std::to_string(command.image.height) + "," +
                     std::to_string(static_cast<unsigned>(command.image.pixel_format)) +
                     "," + std::to_string(command.image.generation) +
                     "|destination=" + rect_text(command.rect) +
                     "|source=" + point_text(command.first) + "," +
                     point_text(command.second) +
                     "|attributes=" + image_attributes_text(command.image_attributes);
            break;
        case CommandKind::set_transform:
        case CommandKind::reset_clip:
        case CommandKind::set_quality:
            break;
        }
        trace += "|transform=" + matrix_text(command.state.transform);
        trace += command.state.clip ? "|clip=" + rect_text(*command.state.clip) : "|clip=none";
        trace += "|quality=" +
                 std::to_string(static_cast<unsigned>(command.state.smoothing)) + "," +
                 std::to_string(static_cast<unsigned>(command.state.interpolation)) + "," +
                 std::to_string(static_cast<unsigned>(command.state.pixel_offset)) + "," +
                 std::to_string(static_cast<unsigned>(command.state.compositing)) + "," +
                 std::to_string(static_cast<unsigned>(command.state.compositing_quality));
        trace += "\n";
    }
    trace += "end|commands=" + std::to_string(commands_.size()) +
             "|closed=" + std::string(closed_ ? "1" : "0") + "\n";
    return trace;
}

void GraphicsRecorder::on_dispose() noexcept {
    closed_ = true;
    saved_.clear();
    commands_.clear();
}

void GraphicsRecorder::require_recordable() const {
    require_alive();
    if (closed_) throw std::logic_error("GUI.Drawing recorder is closed");
    if (commands_.size() >= maximum_commands) {
        throw std::length_error("GUI.Drawing command limit exceeded");
    }
}

void GraphicsRecorder::append(DrawingCommand command) {
    if (commands_.size() >= maximum_commands) {
        throw std::length_error("GUI.Drawing command limit exceeded");
    }
    commands_.push_back(std::move(command));
}

} // namespace gui_drawing
