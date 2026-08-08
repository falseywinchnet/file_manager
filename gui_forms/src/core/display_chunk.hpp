#pragma once

#include "gui_forms/control.hpp"
#include "gui_forms/display.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace gui_forms::detail {

enum class DisplayOperation : std::uint8_t {
    save = 0,
    restore = 1,
    translate = 2,
    clip_rect = 3,
    fill_rect = 4,
    stroke_rect = 5,
    draw_line = 6,
    draw_text_utf8 = 7,
    draw_image = 8,
    clip_rounded_rect = 9,
    fill_rounded_rect = 10,
    stroke_rounded_rect = 11,
    fill_linear_gradient = 12,
    fill_radial_gradient = 13,
    draw_box_shadow = 14,
    fill_linear_gradient_spread = 15,
    draw_image_region = 16,
    fill_image_pattern = 17,
    draw_image_region_sampled = 18,
    draw_live_surface = 19,
};

struct DisplayCommand final {
    DisplayOperation operation{};
    Point first{};
    Point second{};
    Rect rect{};
    Color color{};
    FontSpec font{};
    ImageId image{};
    std::shared_ptr<LiveSurface> live_surface;
    double scalar{};
    double secondary_scalar{};
    double tertiary_scalar{};
    std::vector<GradientStop> gradient_stops;
    GradientSpreadMode gradient_spread{GradientSpreadMode::pad};
    ImagePatternWrap image_pattern_wrap{ImagePatternWrap::tile};
    ImageSampling image_sampling{ImageSampling::linear};
    std::string text;
};

class DisplayChunk final {
public:
    DisplayChunk(std::uint64_t generation,
                 PaintPlane plane,
                 Rect logical_bounds,
                 std::vector<DisplayCommand> commands);

    [[nodiscard]] DisplayChunkInfo info() const noexcept;
    [[nodiscard]] std::uint64_t generation() const noexcept { return generation_; }
    [[nodiscard]] PaintPlane plane() const noexcept { return plane_; }
    [[nodiscard]] Rect logical_bounds() const noexcept { return logical_bounds_; }
    [[nodiscard]] const std::vector<DisplayCommand>& commands() const noexcept {
        return commands_;
    }

private:
    std::uint64_t generation_{};
    PaintPlane plane_{PaintPlane::control};
    Rect logical_bounds_{};
    std::vector<DisplayCommand> commands_;
};

class RecordingPainter final : public Painter {
public:
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
    void draw_line(Point from, Point to, Color color, double width) override;
    void draw_text_utf8(Point origin,
                        std::string_view text,
                        FontSpec font,
                        Color color) override;
    [[nodiscard]] Size measure_text_utf8(std::string_view text,
                                         FontSpec font) override;
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
};

[[nodiscard]] std::uint64_t replay_display_chunk(const DisplayChunk& chunk,
                                                 Painter& painter);

} // namespace gui_forms::detail
