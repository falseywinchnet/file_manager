#pragma once

#include "gui_forms/resources.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>

namespace gui_forms::render {

class SkiaRaster final : public Painter {
public:
    SkiaRaster();
    ~SkiaRaster() override;
    SkiaRaster(const SkiaRaster&) = delete;
    SkiaRaster& operator=(const SkiaRaster&) = delete;

    bool resize(Size logical_size, double scale);
    void begin_frame(const DamageRegion& damage);
    void end_frame();

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

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace gui_forms::render
