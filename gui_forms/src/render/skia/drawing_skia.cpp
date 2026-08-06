#include "drawing_skia.hpp"

#include "include/codec/SkCodec.h"
#include "include/codec/SkPngDecoder.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurface.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkColorMatrixFilter.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkGradient.h"
#include "include/encode/SkPngEncoder.h"
#if defined(__APPLE__)
#include "include/ports/SkFontMgr_mac_ct.h"
#else
#include "include/ports/SkFontMgr_empty.h"
#endif

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace gui_drawing::render {
namespace {

[[nodiscard]] double pixel_font_size(const FontSnapshot& font) noexcept {
    constexpr double dpi = 96.0;
    switch (font.unit) {
    case GraphicsUnit::display: return font.size * dpi / 75.0;
    case GraphicsUnit::point: return font.size * dpi / 72.0;
    case GraphicsUnit::inch: return font.size * dpi;
    case GraphicsUnit::document: return font.size * dpi / 300.0;
    case GraphicsUnit::millimeter: return font.size * dpi / 25.4;
    case GraphicsUnit::world:
    case GraphicsUnit::pixel: return font.size;
    }
    return font.size;
}

[[nodiscard]] std::string typeface_key(std::string_view family,
                                       std::uint32_t style) {
    return std::string(family) + '\x1f' + std::to_string(style & 1U);
}

[[nodiscard]] std::string ascii_lower(std::string_view value) {
    std::string result(value);
    std::transform(result.begin(), result.end(), result.begin(), [](char entry) {
        return entry >= 'A' && entry <= 'Z' ? static_cast<char>(entry + ('a' - 'A'))
                                            : entry;
    });
    return result;
}

[[nodiscard]] std::size_t first_utf8_codepoint_size(std::string_view text) noexcept {
    if (text.empty()) return 0U;
    const auto first = static_cast<unsigned char>(text.front());
    if ((first & 0x80U) == 0U) return 1U;
    if ((first & 0xe0U) == 0xc0U) return std::min<std::size_t>(2U, text.size());
    if ((first & 0xf0U) == 0xe0U) return std::min<std::size_t>(3U, text.size());
    return std::min<std::size_t>(4U, text.size());
}

[[nodiscard]] SkColor to_sk_color(Color color) noexcept {
    return color.is_empty() ? SK_ColorTRANSPARENT :
        SkColorSetARGB(color.alpha(), color.red(), color.green(), color.blue());
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

[[nodiscard]] std::vector<std::byte> remapped_pixels(
    const ImageSnapshot& image, const ImageAttributesSnapshot& attributes) {
    std::vector<std::byte> result(image.pixels().begin(), image.pixels().end());
    if (attributes.remap_table.empty()) return result;
    for (std::size_t offset = 0; offset + 3U < result.size(); offset += 4U) {
        const std::uint8_t first = std::to_integer<std::uint8_t>(result[offset]);
        const std::uint8_t green = std::to_integer<std::uint8_t>(result[offset + 1U]);
        const std::uint8_t third = std::to_integer<std::uint8_t>(result[offset + 2U]);
        const std::uint8_t alpha = std::to_integer<std::uint8_t>(result[offset + 3U]);
        const std::uint8_t red = image.pixel_format == PixelFormat::bgra32_premultiplied
            ? third : first;
        const std::uint8_t blue = image.pixel_format == PixelFormat::bgra32_premultiplied
            ? first : third;
        const Color source = Color::from_argb(
            alpha, unpremultiply(red, alpha), unpremultiply(green, alpha),
            unpremultiply(blue, alpha));
        const auto found = std::find_if(
            attributes.remap_table.begin(), attributes.remap_table.end(),
            [&](const auto& entry) { return entry.old_color.argb() == source.argb(); });
        if (found == attributes.remap_table.end()) continue;
        const Color replacement = found->new_color;
        const std::uint8_t out_alpha = replacement.alpha();
        const std::uint8_t out_red = premultiply(replacement.red(), out_alpha);
        const std::uint8_t out_green = premultiply(replacement.green(), out_alpha);
        const std::uint8_t out_blue = premultiply(replacement.blue(), out_alpha);
        if (image.pixel_format == PixelFormat::bgra32_premultiplied) {
            result[offset] = static_cast<std::byte>(out_blue);
            result[offset + 2U] = static_cast<std::byte>(out_red);
        } else {
            result[offset] = static_cast<std::byte>(out_red);
            result[offset + 2U] = static_cast<std::byte>(out_blue);
        }
        result[offset + 1U] = static_cast<std::byte>(out_green);
        result[offset + 3U] = static_cast<std::byte>(out_alpha);
    }
    return result;
}

[[nodiscard]] SkRect to_sk_rect(RectF rectangle) noexcept {
    return SkRect::MakeXYWH(static_cast<SkScalar>(rectangle.x),
                            static_cast<SkScalar>(rectangle.y),
                            static_cast<SkScalar>(rectangle.width),
                            static_cast<SkScalar>(rectangle.height));
}

[[nodiscard]] SkImageInfo image_info(std::uint32_t width, std::uint32_t height,
                                     PixelFormat format) {
    return SkImageInfo::Make(
        static_cast<int>(width), static_cast<int>(height),
        format == PixelFormat::bgra32_premultiplied ?
            kBGRA_8888_SkColorType : kRGBA_8888_SkColorType,
        kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
}

[[nodiscard]] SkMatrix to_sk_matrix(const Matrix& matrix) {
    SkMatrix result;
    result.setAll(static_cast<SkScalar>(matrix.m11()),
                  static_cast<SkScalar>(matrix.m21()),
                  static_cast<SkScalar>(matrix.dx()),
                  static_cast<SkScalar>(matrix.m12()),
                  static_cast<SkScalar>(matrix.m22()),
                  static_cast<SkScalar>(matrix.dy()),
                  0.0F, 0.0F, 1.0F);
    return result;
}

[[nodiscard]] SkPath to_sk_path(const PathSnapshot& source) {
    SkPathBuilder builder(source.fill_mode == FillMode::alternate ?
        SkPathFillType::kEvenOdd : SkPathFillType::kWinding);
    bool has_current_point = false;
    PointF current_point{};
    for (const PathElement& element : source.elements) {
        switch (element.verb) {
        case PathVerb::start_figure:
            has_current_point = false;
            break;
        case PathVerb::line:
            if (!has_current_point || current_point != element.first) {
                builder.moveTo(static_cast<SkScalar>(element.first.x),
                               static_cast<SkScalar>(element.first.y));
            }
            builder.lineTo(static_cast<SkScalar>(element.second.x),
                           static_cast<SkScalar>(element.second.y));
            current_point = element.second;
            has_current_point = true;
            break;
        case PathVerb::rectangle:
            builder.addRect(to_sk_rect(element.rect));
            has_current_point = false;
            break;
        case PathVerb::ellipse:
            builder.addOval(to_sk_rect(element.rect));
            has_current_point = false;
            break;
        case PathVerb::arc:
            builder.addArc(to_sk_rect(element.rect),
                           static_cast<SkScalar>(element.start_angle),
                           static_cast<SkScalar>(element.sweep_angle));
            has_current_point = false;
            break;
        case PathVerb::close_figure:
            builder.close();
            has_current_point = false;
            break;
        }
    }
    return builder.detach();
}

void apply_image_attributes(SkPaint& paint,
                            const ImageAttributesSnapshot& attributes) {
    if (!attributes.has_color_matrix) return;
    float matrix[20]{};
    for (std::size_t output = 0; output < 4U; ++output) {
        for (std::size_t input = 0; input < 4U; ++input) {
            matrix[output * 5U + input] = static_cast<float>(
                attributes.color_matrix[input * 5U + output]);
        }
        matrix[output * 5U + 4U] = static_cast<float>(
            attributes.color_matrix[20U + output] * 255.0);
    }
    paint.setColorFilter(SkColorFilters::Matrix(matrix));
}

[[nodiscard]] SkSamplingOptions sampling_for(const GraphicsState& state) {
    return state.interpolation == InterpolationMode::nearest ?
        SkSamplingOptions(SkFilterMode::kNearest) :
        SkSamplingOptions(SkFilterMode::kLinear);
}

void configure_paint(SkPaint& paint, const GraphicsState& state) {
    paint.setAntiAlias(state.smoothing != SmoothingMode::none &&
                       state.smoothing != SmoothingMode::high_speed);
    paint.setBlendMode(state.compositing == CompositingMode::source_copy ?
        SkBlendMode::kSrc : SkBlendMode::kSrcOver);
}

void configure_pen(SkPaint& paint, const PenSnapshot& pen,
                   const GraphicsState& state) {
    configure_paint(paint, state);
    paint.setColor(to_sk_color(pen.color));
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(static_cast<SkScalar>(pen.width));
    std::vector<SkScalar> pattern;
    if (pen.dash_style == DashStyle::custom) {
        pattern.reserve(pen.dash_pattern.size());
        for (const double entry : pen.dash_pattern) {
            pattern.push_back(static_cast<SkScalar>(entry));
        }
    } else if (pen.dash_style != DashStyle::solid) {
        const SkScalar width = static_cast<SkScalar>(pen.width);
        switch (pen.dash_style) {
        case DashStyle::dash: pattern = {3.0F * width, width}; break;
        case DashStyle::dot: pattern = {width, width}; break;
        case DashStyle::dash_dot: pattern = {3.0F * width, width, width, width}; break;
        case DashStyle::dash_dot_dot:
            pattern = {3.0F * width, width, width, width, width, width};
            break;
        case DashStyle::solid:
        case DashStyle::custom:
            break;
        }
    }
    if (pattern.size() >= 2U && pattern.size() % 2U == 0U) {
        paint.setPathEffect(SkDashPathEffect::Make(
            SkSpan<const SkScalar>(pattern.data(), pattern.size()), 0.0F));
    }
}

[[nodiscard]] SkTileMode tile_mode(WrapMode mode) noexcept {
    switch (mode) {
    case WrapMode::tile: return SkTileMode::kRepeat;
    case WrapMode::tile_flip_x:
    case WrapMode::tile_flip_y:
    case WrapMode::tile_flip_xy: return SkTileMode::kMirror;
    case WrapMode::clamp: return SkTileMode::kClamp;
    }
    return SkTileMode::kClamp;
}

void configure_brush(SkPaint& paint, const BrushSnapshot& brush,
                     const GraphicsState& state) {
    configure_paint(paint, state);
    paint.setStyle(SkPaint::kFill_Style);
    if (brush.kind == BrushKind::solid) {
        paint.setColor(to_sk_color(brush.primary));
        return;
    }
    if (brush.kind == BrushKind::hatch) {
        constexpr int extent = 8;
        std::array<SkColor, extent * extent> pixels;
        pixels.fill(to_sk_color(brush.secondary));
        const auto mark = [&](int x, int y) {
            pixels[static_cast<std::size_t>(y * extent + x)] =
                to_sk_color(brush.primary);
        };
        for (int coordinate = 0; coordinate < extent; ++coordinate) {
            switch (brush.hatch_style) {
            case HatchStyle::horizontal: mark(coordinate, 3); break;
            case HatchStyle::vertical: mark(3, coordinate); break;
            case HatchStyle::forward_diagonal:
                mark(coordinate, extent - 1 - coordinate); break;
            case HatchStyle::backward_diagonal: mark(coordinate, coordinate); break;
            case HatchStyle::cross:
                mark(coordinate, 3); mark(3, coordinate); break;
            case HatchStyle::diagonal_cross:
                mark(coordinate, coordinate);
                mark(coordinate, extent - 1 - coordinate);
                break;
            }
        }
        const SkImageInfo info = SkImageInfo::MakeN32Premul(extent, extent);
        sk_sp<SkData> data = SkData::MakeWithCopy(pixels.data(), sizeof(pixels));
        sk_sp<SkImage> image = SkImages::RasterFromData(
            info, std::move(data), extent * sizeof(SkColor));
        if (image) {
            paint.setShader(image->makeShader(
                SkTileMode::kRepeat, SkTileMode::kRepeat,
                SkSamplingOptions(SkFilterMode::kNearest)));
        }
        return;
    }

    std::vector<SkColor4f> colors;
    std::vector<float> positions;
    colors.reserve(brush.colors.size());
    positions.reserve(brush.positions.size());
    for (const Color color : brush.colors) {
        colors.push_back(SkColor4f::FromColor(to_sk_color(color)));
    }
    for (const double position : brush.positions) {
        positions.push_back(static_cast<SkScalar>(position));
    }
    if (colors.size() < 2U || colors.size() != positions.size()) {
        paint.setColor(to_sk_color(brush.primary));
        return;
    }
    const SkGradient::Colors gradient_colors(
        SkSpan<const SkColor4f>(colors.data(), colors.size()),
        SkSpan<const float>(positions.data(), positions.size()),
        tile_mode(brush.wrap_mode), SkColorSpace::MakeSRGB());
    const SkGradient gradient(gradient_colors, SkGradient::Interpolation{});
    if (brush.kind == BrushKind::linear_gradient) {
        constexpr double degrees_to_radians = 0.017453292519943295769;
        const double radians = brush.angle * degrees_to_radians;
        const double half_length = std::abs(std::cos(radians)) * brush.bounds.width / 2.0 +
            std::abs(std::sin(radians)) * brush.bounds.height / 2.0;
        const PointF center{brush.bounds.x + brush.bounds.width / 2.0,
                            brush.bounds.y + brush.bounds.height / 2.0};
        const SkPoint points[] = {
            {static_cast<SkScalar>(center.x - std::cos(radians) * half_length),
             static_cast<SkScalar>(center.y - std::sin(radians) * half_length)},
            {static_cast<SkScalar>(center.x + std::cos(radians) * half_length),
             static_cast<SkScalar>(center.y + std::sin(radians) * half_length)},
        };
        paint.setShader(SkShaders::LinearGradient(points, gradient));
        return;
    }
    double radius{};
    for (const PointF point : brush.points) {
        radius = std::max(radius, std::hypot(point.x - brush.center.x,
                                             point.y - brush.center.y));
    }
    if (radius <= 0.0) {
        paint.setColor(to_sk_color(brush.primary));
        return;
    }
    paint.setShader(SkShaders::RadialGradient(
        {static_cast<SkScalar>(brush.center.x),
         static_cast<SkScalar>(brush.center.y)},
        static_cast<SkScalar>(radius), gradient));
}

class BitmapUnlock final {
public:
    BitmapUnlock(Bitmap& bitmap, std::uint64_t token)
        : bitmap_(&bitmap), token_(token) {}
    ~BitmapUnlock() {
        if (bitmap_ != nullptr) {
            try { bitmap_->unlock(token_); } catch (...) {}
        }
    }
    void release() {
        bitmap_->unlock(token_);
        bitmap_ = nullptr;
    }

private:
    Bitmap* bitmap_;
    std::uint64_t token_;
};

} // namespace

class SkiaExecutor::Impl final {
public:
    sk_sp<SkFontMgr> font_manager{
#if defined(__APPLE__)
        SkFontMgr_New_CoreText(nullptr)
#else
        SkFontMgr_New_Custom_Empty()
#endif
    };
    std::unordered_map<std::string, sk_sp<SkTypeface>> typefaces;

    [[nodiscard]] sk_sp<SkTypeface> typeface(std::string_view family,
                                             std::uint32_t style) const {
        const auto found = typefaces.find(typeface_key(family, style));
        if (found != typefaces.end()) return found->second;
        const auto regular = typefaces.find(typeface_key(family, 0U));
        if (regular != typefaces.end()) return regular->second;
        const SkFontStyle requested(
            (style & 1U) != 0U ? SkFontStyle::kBold_Weight
                               : SkFontStyle::kNormal_Weight,
            SkFontStyle::kNormal_Width,
            (style & 2U) != 0U ? SkFontStyle::kItalic_Slant
                               : SkFontStyle::kUpright_Slant);
        if (!family.empty()) {
            if (sk_sp<SkTypeface> platform = font_manager->matchFamilyStyle(
                    std::string(family).c_str(), requested)) {
                return platform;
            }
        }
        const auto fallback = typefaces.find(typeface_key("Portsmouth Rapids", style));
        if (fallback != typefaces.end()) return fallback->second;
        const auto regular_fallback = typefaces.find(typeface_key("Portsmouth Rapids", 0U));
        if (regular_fallback != typefaces.end()) return regular_fallback->second;
        if (sk_sp<SkTypeface> platform = font_manager->legacyMakeTypeface(
                nullptr, requested)) {
            return platform;
        }
        return SkTypeface::MakeEmpty();
    }

    [[nodiscard]] SkFont font(const FontSnapshot& spec) const {
        SkFont result(typeface(spec.family, spec.style),
                      static_cast<SkScalar>(pixel_font_size(spec)));
        result.setEdging(SkFont::Edging::kAntiAlias);
        if ((spec.style & 1U) != 0U &&
            typefaces.find(typeface_key(spec.family, 1U)) == typefaces.end()) {
            result.setEmbolden(true);
        }
        if ((spec.style & 2U) != 0U) result.setSkewX(-0.25F);
        return result;
    }
};

SkiaExecutor::SkiaExecutor()
    : impl_(std::make_unique<Impl>()), owner_thread_(std::this_thread::get_id()) {}
SkiaExecutor::~SkiaExecutor() = default;

bool SkiaExecutor::register_typeface(std::string_view family,
                                     std::span<const std::byte> encoded,
                                     std::uint32_t style) {
    if (!owner_thread() || family.empty() || family.size() > 4096U || encoded.empty()) {
        return false;
    }
    sk_sp<SkData> data = SkData::MakeWithCopy(encoded.data(), encoded.size());
    const std::string requested_family = ascii_lower(family);
    const int requested_weight = (style & 1U) != 0U
        ? SkFontStyle::kBold_Weight : SkFontStyle::kNormal_Weight;
    sk_sp<SkTypeface> best;
    int best_score = std::numeric_limits<int>::min();
    // makeFromData's collection index is observable for TTC/OTC files. Scan a
    // bounded number of faces and choose the requested family/style instead of
    // silently using collection face zero.
    for (int index = 0; index < 64; ++index) {
        sk_sp<SkTypeface> candidate = impl_->font_manager->makeFromData(data, index);
        if (!candidate) {
            if (index == 0) return false;
            break;
        }
        SkString candidate_family;
        candidate->getFamilyName(&candidate_family);
        const bool exact_family = ascii_lower(candidate_family.c_str()) == requested_family;
        const int weight_distance = std::abs(candidate->fontStyle().weight() -
                                             requested_weight);
        const int score = (exact_family ? 100000 : 0) - weight_distance;
        if (score > best_score) {
            best_score = score;
            best = std::move(candidate);
        }
        if (exact_family && weight_distance == 0) break;
    }
    if (!best) return false;
    impl_->typefaces[typeface_key(family, style)] = std::move(best);
    return true;
}

SizeF SkiaExecutor::measure_string(std::string_view utf8,
                                  const FontSnapshot& font_spec,
                                  const StringFormatSnapshot& format,
                                  double layout_width) {
    if (!owner_thread() || !std::isfinite(layout_width) || layout_width < 0.0) {
        throw std::invalid_argument("text measurement arguments are invalid");
    }
    if (utf8.empty()) return {};
    const SkFont font = impl_->font(font_spec);
    SkFontMetrics metrics{};
    font.getMetrics(&metrics);
    const bool fit_black_box = (format.flags & UINT32_C(0x0004)) != 0U;
    const double overhang = fit_black_box ? 0.0 : pixel_font_size(font_spec) / 6.0;
    const double line_height = std::max(0.0,
        static_cast<double>(metrics.fDescent - metrics.fAscent + metrics.fLeading) +
        overhang);
    const bool wrap = layout_width > 0.0 && (format.flags & UINT32_C(0x1000)) == 0U;
    const double content_width = wrap ? std::max(0.0, layout_width - overhang * 2.0) : 0.0;
    double maximum_width{};
    std::size_t line_count{};
    std::size_t offset{};
    do {
        const std::size_t newline = utf8.find('\n', offset);
        const std::size_t end = newline == std::string_view::npos ? utf8.size() : newline;
        std::string_view line = utf8.substr(offset, end - offset);
        if (!line.empty() && line.back() == '\r') line.remove_suffix(1U);
        if (!wrap || line.empty()) {
            const SkScalar width = font.measureText(line.data(), line.size(),
                                                    SkTextEncoding::kUTF8);
            maximum_width = std::max(maximum_width, static_cast<double>(width));
            ++line_count;
        } else {
            while (!line.empty()) {
                std::vector<std::size_t> byte_ends;
                for (std::string_view rest = line; !rest.empty();) {
                    const std::size_t count = first_utf8_codepoint_size(rest);
                    byte_ends.push_back(line.size() - rest.size() + count);
                    rest.remove_prefix(count);
                }
                std::vector<SkGlyphID> glyphs(font.countText(
                    line.data(), line.size(), SkTextEncoding::kUTF8));
                font.textToGlyphs(line.data(), line.size(), SkTextEncoding::kUTF8,
                                  SkSpan<SkGlyphID>(glyphs));
                std::vector<SkScalar> widths(glyphs.size());
                font.getWidths(SkSpan<const SkGlyphID>(glyphs), SkSpan<SkScalar>(widths));
                SkScalar width{};
                std::size_t fitted{};
                while (fitted < widths.size() &&
                       width + widths[fitted] <= static_cast<SkScalar>(content_width)) {
                    width += widths[fitted++];
                }
                if (fitted == 0U) {
                    fitted = 1U;
                    width = widths.empty() ? 0.0F : widths.front();
                }
                std::size_t consumed = byte_ends[std::min(fitted, byte_ends.size()) - 1U];
                if (consumed < line.size()) {
                    const std::size_t space = line.substr(0U, consumed).find_last_of(" \t");
                    if (space != std::string_view::npos && space != 0U) {
                        consumed = space;
                        width = font.measureText(line.data(), consumed,
                                                 SkTextEncoding::kUTF8);
                    }
                }
                maximum_width = std::max(maximum_width,
                                         std::min(content_width, static_cast<double>(width)));
                ++line_count;
                line.remove_prefix(consumed);
                while (!line.empty() && (line.front() == ' ' || line.front() == '\t')) {
                    line.remove_prefix(1U);
                }
            }
        }
        if (newline == std::string_view::npos) break;
        offset = newline + 1U;
    } while (offset <= utf8.size());
    return {wrap ? std::min(layout_width, maximum_width + overhang * 2.0) :
                   maximum_width + overhang * 2.0,
            line_height * static_cast<double>(line_count)};
}

RasterResult SkiaExecutor::execute(const GraphicsRecorder& recorder,
                                   Bitmap& target,
                                   std::size_t first_command) {
    if (!owner_thread()) return {RasterError::wrong_thread};
    try {
        const std::span<const DrawingCommand> commands = recorder.commands();
        if (first_command > commands.size()) return {RasterError::invalid_argument};
        if (first_command == commands.size()) return {};
        auto scratch = target.clone({0, 0, static_cast<std::int32_t>(target.width()),
                                      static_cast<std::int32_t>(target.height())});
        const BitmapLockView scratch_lock = scratch->lock(BitmapLockMode::write);
        BitmapUnlock unlock_scratch(*scratch, scratch_lock.token);
        const SkImageInfo info = image_info(scratch_lock.width, scratch_lock.height,
                                            scratch_lock.pixel_format);
        sk_sp<SkSurface> surface = SkSurfaces::WrapPixels(
            info, scratch_lock.writable_data, scratch_lock.row_bytes);
        if (!surface) return {RasterError::target_unavailable};
        SkCanvas* canvas = surface->getCanvas();
        std::size_t executed{};
        for (const DrawingCommand& command : commands.subspan(first_command)) {
            canvas->restoreToCount(1);
            canvas->resetMatrix();
            if (command.state.clip) {
                canvas->clipRect(to_sk_rect(*command.state.clip),
                                 SkClipOp::kIntersect, false);
            }
            canvas->setMatrix(to_sk_matrix(command.state.transform));

            SkPaint paint;
            configure_paint(paint, command.state);
            switch (command.kind) {
            case CommandKind::save:
            case CommandKind::restore:
            case CommandKind::translate:
            case CommandKind::set_transform:
            case CommandKind::set_clip:
            case CommandKind::reset_clip:
            case CommandKind::set_quality:
                break;
            case CommandKind::clear:
                canvas->clear(to_sk_color(command.color));
                break;
            case CommandKind::fill_rectangle:
                configure_brush(paint, command.brush, command.state);
                canvas->drawRect(to_sk_rect(command.rect), paint);
                break;
            case CommandKind::draw_rectangle:
                configure_pen(paint, command.pen, command.state);
                canvas->drawRect(to_sk_rect(command.rect), paint);
                break;
            case CommandKind::draw_line:
                configure_pen(paint, command.pen, command.state);
                canvas->drawLine(static_cast<SkScalar>(command.first.x),
                                 static_cast<SkScalar>(command.first.y),
                                 static_cast<SkScalar>(command.second.x),
                                 static_cast<SkScalar>(command.second.y), paint);
                break;
            case CommandKind::draw_string: {
                paint.setColor(to_sk_color(command.color));
                SkFont font = impl_->font(command.font);
                const SkScalar overhang = (command.format.flags & UINT32_C(0x0004)) == 0U ?
                    static_cast<SkScalar>(pixel_font_size(command.font) / 6.0) : 0.0F;
                SkScalar x = static_cast<SkScalar>(command.first.x) + overhang;
                SkFontMetrics metrics;
                font.getMetrics(&metrics);
                const SkScalar baseline = static_cast<SkScalar>(command.first.y) -
                    metrics.fAscent + overhang;
                if (command.format.alignment != StringAlignment::near) {
                    const SkScalar width = font.measureText(
                        command.text.data(), command.text.size(),
                        SkTextEncoding::kUTF8, nullptr, &paint);
                    x -= command.format.alignment == StringAlignment::center ?
                        width / 2.0F : width;
                }
                canvas->drawSimpleText(command.text.data(), command.text.size(),
                                       SkTextEncoding::kUTF8, x,
                                       baseline,
                                       font, paint);
                break;
            }
            case CommandKind::draw_ellipse:
                configure_pen(paint, command.pen, command.state);
                canvas->drawOval(to_sk_rect(command.rect), paint);
                break;
            case CommandKind::fill_ellipse:
                configure_brush(paint, command.brush, command.state);
                canvas->drawOval(to_sk_rect(command.rect), paint);
                break;
            case CommandKind::fill_polygon: {
                SkPathBuilder builder(command.path.fill_mode == FillMode::alternate ?
                    SkPathFillType::kEvenOdd : SkPathFillType::kWinding);
                builder.moveTo(static_cast<SkScalar>(command.points.front().x),
                               static_cast<SkScalar>(command.points.front().y));
                for (std::size_t index = 1; index < command.points.size(); ++index) {
                    builder.lineTo(static_cast<SkScalar>(command.points[index].x),
                                   static_cast<SkScalar>(command.points[index].y));
                }
                builder.close();
                configure_brush(paint, command.brush, command.state);
                canvas->drawPath(builder.detach(), paint);
                break;
            }
            case CommandKind::draw_path:
                configure_pen(paint, command.pen, command.state);
                canvas->drawPath(to_sk_path(command.path), paint);
                break;
            case CommandKind::fill_path:
                configure_brush(paint, command.brush, command.state);
                canvas->drawPath(to_sk_path(command.path), paint);
                break;
            case CommandKind::draw_image: {
                if (!command.image.has_pixels()) {
                    return {RasterError::missing_image_pixels, executed};
                }
                const SkImageInfo source_info = image_info(
                    command.image.width, command.image.height,
                    command.image.pixel_format);
                const std::vector<std::byte> pixels = remapped_pixels(
                    command.image, command.image_attributes);
                sk_sp<SkData> data = SkData::MakeWithCopy(
                    pixels.data(), pixels.size());
                sk_sp<SkImage> image = SkImages::RasterFromData(
                    source_info, std::move(data), command.image.row_bytes());
                if (!image) return {RasterError::missing_image_pixels, executed};
                apply_image_attributes(paint, command.image_attributes);
                const RectF source{command.first.x, command.first.y,
                                   command.second.x, command.second.y};
                canvas->drawImageRect(image.get(), to_sk_rect(source),
                                      to_sk_rect(command.rect),
                                      sampling_for(command.state), &paint,
                                      SkCanvas::kStrict_SrcRectConstraint);
                break;
            }
            }
            ++executed;
        }
        unlock_scratch.release();
        const ImageSnapshot rendered = scratch->snapshot();
        const BitmapLockView target_lock = target.lock(BitmapLockMode::write);
        BitmapUnlock unlock_target(target, target_lock.token);
        if (rendered.pixels().size() !=
            target_lock.row_bytes * static_cast<std::size_t>(target_lock.height)) {
            return {RasterError::target_unavailable};
        }
        std::memcpy(target_lock.writable_data, rendered.pixels().data(),
                    rendered.pixels().size());
        unlock_target.release();
        return {RasterError::none, executed};
    } catch (const std::invalid_argument&) {
        return {RasterError::invalid_argument};
    } catch (const std::logic_error&) {
        return {RasterError::invalid_argument};
    } catch (...) {
        return {RasterError::internal_error};
    }
}

DecodeResult SkiaExecutor::decode_png(std::span<const std::byte> encoded,
                                      const PngCodecLimits& limits) {
    if (!owner_thread()) return {.error = RasterError::wrong_thread};
    if (encoded.empty()) return {.error = RasterError::decode_failed};
    if (encoded.size() > limits.maximum_encoded_bytes) {
        return {.error = RasterError::encoded_limit_exceeded};
    }
    try {
        sk_sp<SkData> data = SkData::MakeWithCopy(encoded.data(), encoded.size());
        SkCodec::Result codec_result = SkCodec::kInternalError;
        std::unique_ptr<SkCodec> codec =
            SkPngDecoder::Decode(std::move(data), &codec_result);
        if (!codec || codec_result != SkCodec::kSuccess) {
            return {.error = RasterError::decode_failed};
        }
        const SkImageInfo source = codec->getInfo();
        if (source.width() <= 0 || source.height() <= 0 ||
            static_cast<std::uint32_t>(source.width()) > limits.maximum_width ||
            static_cast<std::uint32_t>(source.height()) > limits.maximum_height ||
            static_cast<std::uint64_t>(source.width()) * source.height() >
                limits.maximum_pixels) {
            return {.error = RasterError::dimension_limit_exceeded};
        }
        auto bitmap = std::make_unique<Bitmap>(
            static_cast<std::uint32_t>(source.width()),
            static_cast<std::uint32_t>(source.height()),
            PixelFormat::bgra32_premultiplied);
        const BitmapLockView lock = bitmap->lock(BitmapLockMode::write);
        BitmapUnlock unlock(*bitmap, lock.token);
        const SkImageInfo output = image_info(lock.width, lock.height,
                                              lock.pixel_format);
        if (codec->getPixels(output, lock.writable_data, lock.row_bytes) !=
            SkCodec::kSuccess) {
            return {.error = RasterError::decode_failed};
        }
        // A few palette/tRNS PNGs are returned by SkCodec with straight RGB
        // channels even though the requested destination is tagged premultiplied.
        // Transparent WinForms theme glyphs then carry non-zero color at A=0,
        // which becomes an opaque pale square when that raster is composited by
        // another retained surface. Detect the impossible premultiplied values
        // and normalize the complete decoded image exactly once.
        auto* pixels = reinterpret_cast<std::uint8_t*>(lock.writable_data);
        bool straight_alpha{};
        for (std::uint32_t y = 0; y < lock.height && !straight_alpha; ++y) {
            const auto* row = pixels + static_cast<std::size_t>(y) * lock.row_bytes;
            for (std::uint32_t x = 0; x < lock.width; ++x) {
                const auto* pixel = row + static_cast<std::size_t>(x) * 4U;
                const std::uint8_t alpha = pixel[3];
                if (pixel[0] > alpha || pixel[1] > alpha || pixel[2] > alpha) {
                    straight_alpha = true;
                    break;
                }
            }
        }
        if (straight_alpha) {
            for (std::uint32_t y = 0; y < lock.height; ++y) {
                auto* row = pixels + static_cast<std::size_t>(y) * lock.row_bytes;
                for (std::uint32_t x = 0; x < lock.width; ++x) {
                    auto* pixel = row + static_cast<std::size_t>(x) * 4U;
                    const std::uint8_t alpha = pixel[3];
                    pixel[0] = premultiply(pixel[0], alpha);
                    pixel[1] = premultiply(pixel[1], alpha);
                    pixel[2] = premultiply(pixel[2], alpha);
                }
            }
        }
        unlock.release();
        return {std::move(bitmap), RasterError::none};
    } catch (const std::length_error&) {
        return {.error = RasterError::dimension_limit_exceeded};
    } catch (...) {
        return {.error = RasterError::internal_error};
    }
}

std::vector<std::byte> SkiaExecutor::encode_png(
    const Bitmap& bitmap, RasterError* error, const PngCodecLimits& limits) {
    const auto set_error = [error](RasterError value) {
        if (error != nullptr) *error = value;
    };
    if (!owner_thread()) {
        set_error(RasterError::wrong_thread);
        return {};
    }
    try {
        const ImageSnapshot snapshot = bitmap.snapshot();
        if (snapshot.width > limits.maximum_width ||
            snapshot.height > limits.maximum_height ||
            static_cast<std::uint64_t>(snapshot.width) * snapshot.height >
                limits.maximum_pixels) {
            set_error(RasterError::dimension_limit_exceeded);
            return {};
        }
        const SkPixmap pixmap(image_info(snapshot.width, snapshot.height,
                                         snapshot.pixel_format),
                              snapshot.pixels().data(), snapshot.row_bytes());
        SkPngEncoder::Options options;
        options.fFilterFlags = SkPngEncoder::FilterFlag::kAll;
        options.fZLibLevel = 6;
        sk_sp<SkData> encoded = SkPngEncoder::Encode(pixmap, options);
        if (!encoded || encoded->size() == 0U) {
            set_error(RasterError::encode_failed);
            return {};
        }
        if (encoded->size() > limits.maximum_encoded_bytes) {
            set_error(RasterError::encoded_limit_exceeded);
            return {};
        }
        std::vector<std::byte> result(encoded->size());
        std::memcpy(result.data(), encoded->data(), encoded->size());
        set_error(RasterError::none);
        return result;
    } catch (const std::logic_error&) {
        set_error(RasterError::invalid_argument);
        return {};
    } catch (...) {
        set_error(RasterError::internal_error);
        return {};
    }
}

bool SkiaExecutor::owner_thread() const noexcept {
    return std::this_thread::get_id() == owner_thread_;
}

} // namespace gui_drawing::render
