#pragma once

#include "gui_forms/types/geometry/geometry.hpp"
#include "gui_forms/types/paint_types/paint_types.hpp"
#include "gui_forms/typography.hpp"

#include <memory>
#include <span>
#include <string_view>

namespace gui_forms {

class LiveSurface;
class PaintFramebuffer;
#if defined(GUI_FORMS_PREPARED_TEXT)
class PreparedTextLayout;
struct LayoutAuthority;
struct PreparedTextPaintResult;
#endif

class Painter : public TextMetricsProvider {
public:
    virtual ~Painter() = default;
    // Native renderers preserve their fonts and drawing semantics offscreen.
    // Recording/headless painters explicitly report this facility unavailable.
    [[nodiscard]] virtual std::unique_ptr<PaintFramebuffer> create_framebuffer(
        Size logical_size, double scale);
#if defined(GUI_FORMS_PREPARED_TEXT)
    // Development surface: recording retains immutable storage; a compatible
    // backend stages pixels. Only the host's successful frame commit presents.
    [[nodiscard]] virtual PreparedTextPaintResult draw_prepared_text(
        const PreparedTextLayout& layout, const LayoutAuthority expected, const Point baseline, const Color color);
#endif

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
    // Drawn over the owning surface fill and clipped to its interior. The
    // source is the complement of a translated, spread-adjusted inner box, so
    // the recipe relaxes with live owner geometry rather than fixed traces.
    virtual void draw_inset_box_shadow(Rect rect, double corner_radius,
                                       Point offset, double blur_radius,
                                       double spread, Color color);
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
    [[nodiscard]] ResolvedTextLayout resolve_text_layout_utf8(
        std::string_view text, FontSpec font) override {
        ResolvedTextLayout result = estimate_text_layout_utf8(text, font);
        result.logical_size = measure_text_utf8(text, font);
        return result;
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

} // namespace gui_forms
