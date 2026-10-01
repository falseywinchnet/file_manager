#pragma once

#include "../chunk/display_chunk.hpp"

#include <memory>
#include <optional>
#include <vector>

namespace gui_forms::detail {

class RecordingPainter final : public Painter {
public:
    explicit RecordingPainter(TextMetricsProvider* text_metrics = nullptr)
        : text_metrics_(text_metrics) {}
    void save() override;
    void restore() override;
    void translate(Point offset) override;
    void clip_rect(Rect rect) override;
    void clip_rounded_rect(Rect rect, double radius) override;
    void fill_rect(Rect rect, Color color) override;
    void fill_rounded_rect(Rect rect, double radius, Color color) override;
    void stroke_rect(Rect rect, Color color, double width) override;
    void stroke_rounded_rect(Rect rect, double radius, Color color,
                             double width) override;
    void fill_linear_gradient(Rect rect, Point start, Point end,
                              std::span<const GradientStop> stops) override;
    void fill_linear_gradient_spread(
        Rect rect, Point start, Point end,
        std::span<const GradientStop> stops,
        GradientSpreadMode spread) override;
    void fill_radial_gradient(Rect rect, Point center, Size radii,
                              std::span<const GradientStop> stops) override;
    void draw_box_shadow(Rect rect, double corner_radius, Point offset,
                         double blur_radius, double spread,
                         Color color) override;
    void draw_inset_box_shadow(Rect rect, double corner_radius, Point offset,
                               double blur_radius, double spread,
                               Color color) override;
    void draw_line(Point from, Point to, Color color, double width) override;
    void draw_text_utf8(Point origin, std::string_view text,
                        FontSpec font, Color color) override;
#if defined(GUI_FORMS_PREPARED_TEXT)
    [[nodiscard]] PreparedTextPaintResult draw_prepared_text(const PreparedTextLayout& layout,
        const LayoutAuthority expected, const Point baseline, const Color color) override;
#endif
    [[nodiscard]] Size measure_text_utf8(std::string_view text,
                                         FontSpec font) override;
    [[nodiscard]] ResolvedTextLayout resolve_text_layout_utf8(
        std::string_view text, FontSpec font) override;
    void draw_image(ImageId image, Rect destination, double opacity) override;
    void draw_live_surface(std::shared_ptr<LiveSurface> surface,
                           Rect destination, double opacity) override;
    void draw_image_region(ImageId image, Rect source, Rect destination,
                           double opacity) override;
    void draw_image_region_sampled(ImageId image, Rect source,
                                   Rect destination, ImageSampling sampling,
                                   double opacity) override;
    void fill_image_pattern(ImageId image, Size source_pixel_size,
                            Rect destination, Size logical_tile_size,
                            ImagePatternWrap wrap, double opacity) override;

    [[nodiscard]] std::shared_ptr<const DisplayChunk> finish(
        std::uint64_t generation, PaintPlane plane, Rect logical_bounds);

private:
    std::vector<DisplayCommand> commands_;
    std::uint64_t save_depth_{};
    TextMetricsProvider* text_metrics_{};
#if defined(GUI_FORMS_PREPARED_TEXT)
    std::optional<PreparedTextStatus> prepared_failure_{};
#endif
};

} // namespace gui_forms::detail
