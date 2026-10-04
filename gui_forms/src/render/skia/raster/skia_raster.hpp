#pragma once

#include "gui_forms/resources.hpp"
#include "gui_forms/live_surface/frame/live_surface_frame.hpp"
#if defined(GUI_FORMS_PREPARED_TEXT)
#include "gui_forms/prepared_text.hpp"
#include "gui_forms/control.hpp"
#include "gui_forms/window/presentation/presentation_types.hpp"
#endif

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>

namespace gui_forms::render {

class SkiaRaster final : public Painter {
public:
    SkiaRaster();
    std::unique_ptr<PaintFramebuffer> create_framebuffer(Size logical_size, double scale) override;
    ~SkiaRaster() override;
    SkiaRaster(const SkiaRaster&) = delete;
    SkiaRaster& operator=(const SkiaRaster&) = delete;

    bool resize(Size logical_size, double scale);
    void begin_frame(const DamageRegion& damage);
    void end_frame();
#if defined(GUI_FORMS_PREPARED_TEXT)
    // Guarded host transaction. begin expands damage when a full repaint is
    // required; caller paints that updated region. Null receipts and exceptions
    // must abort. end_frame only restores canvas state, never publishes pixels.
    [[nodiscard]] PreparedTextStatus begin_prepared_frame(const Size logical_size,
        const double scale, DamageRegion& damage);
    [[nodiscard]] PreparedTextStatus commit_prepared_frame(const PaintReceipt receipt);
    void abort_prepared_frame() noexcept;
    [[nodiscard]] PaintReceipt prepared_front_receipt() const noexcept;
    [[nodiscard]] double prepared_front_scale() const noexcept;
    [[nodiscard]] bool prepared_front_matches(const Size logical_size, const double scale) const noexcept;
    [[nodiscard]] PreparedTextPaintResult draw_prepared_text(const PreparedTextLayout& layout,
        const LayoutAuthority expected, const Point baseline, const Color color) override;
#endif

    [[nodiscard]] bool register_typeface(FontRole role,
                                         std::uint16_t weight,
                                         bool italic,
                                         std::span<const std::byte> encoded);
    [[nodiscard]] bool register_fallback_typeface(
        std::uint16_t weight, bool italic,
        std::span<const std::byte> encoded);
    // Bundled files remain read-only mappings shared with the text shaper.
    [[nodiscard]] bool register_typeface_file(
        FontRole role, std::uint16_t weight, bool italic, const char* path);
    [[nodiscard]] bool register_fallback_typeface_file(
        std::uint16_t weight, bool italic, const char* path);
    [[nodiscard]] bool synchronize_images(const ImageRegistry& registry);
    [[nodiscard]] const void* pixels() const noexcept;
    [[nodiscard]] std::size_t row_bytes() const noexcept;
    [[nodiscard]] std::uint32_t pixel_width() const noexcept;
    [[nodiscard]] std::uint32_t pixel_height() const noexcept;
    [[nodiscard]] std::size_t byte_size() const noexcept;

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
    void fill_linear_gradient(
        Rect rect, Point start, Point end,
        std::span<const GradientStop> stops) override;
    void fill_linear_gradient_spread(
        Rect rect, Point start, Point end,
        std::span<const GradientStop> stops,
        GradientSpreadMode spread) override;
    void fill_radial_gradient(
        Rect rect, Point center, Size radii,
        std::span<const GradientStop> stops) override;
    void draw_box_shadow(Rect rect, double corner_radius, Point offset,
                         double blur_radius, double spread,
                         Color color) override;
    void draw_inset_box_shadow(Rect rect, double corner_radius, Point offset,
                               double blur_radius, double spread,
                               Color color) override;
    void draw_line(Point from, Point to, Color color, double width) override;
    void draw_text_utf8(Point origin,
                        std::string_view text,
                        FontSpec font,
                        Color color) override;
    [[nodiscard]] Size measure_text_utf8(std::string_view text,
                                         FontSpec font) override;
    [[nodiscard]] ResolvedTextLayout resolve_text_layout_utf8(
        std::string_view text, FontSpec font) override;
    void draw_image(ImageId image, Rect destination, double opacity) override;
    void draw_live_surface(const std::shared_ptr<LiveSurface> surface,
                           const Rect destination, const double opacity) override;
    // Internal host seam: keep the coverage promise and sampled pixels in the
    // same read lease through rasterization, even during producer reconfigure.
    [[nodiscard]] bool draw_live_surface_frame(const LiveSurfaceFrame& frame,
                                               const Rect destination, const double opacity);
    void draw_image_region(ImageId image, Rect source, Rect destination,
                           double opacity) override;
    void draw_image_region_sampled(ImageId image, Rect source,
                                   Rect destination, ImageSampling sampling,
                                   double opacity) override;
    void fill_image_pattern(ImageId image, Size source_pixel_size,
                            Rect destination, Size logical_tile_size,
                            ImagePatternWrap wrap, double opacity) override;

private:
    friend struct SkiaLiveSurfaceTestAccess;
    class Impl;
    std::unique_ptr<Impl> impl_;
    bool fail_next_live_image_{};
};

} // namespace gui_forms::render
