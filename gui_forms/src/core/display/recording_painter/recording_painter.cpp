#include "recording_painter.hpp"

#include <stdexcept>
#include <utility>

namespace gui_forms::detail {

void RecordingPainter::save() {
    DisplayCommand command;
    command.operation = DisplayOperation::save;
    commands_.push_back(std::move(command));
    ++save_depth_;
}

void RecordingPainter::restore() {
    if (save_depth_ == 0U) {
        throw std::logic_error("GUI.Forms display chunk restored without a matching save");
    }
    DisplayCommand command;
    command.operation = DisplayOperation::restore;
    commands_.push_back(std::move(command));
    --save_depth_;
}

void RecordingPainter::translate(Point offset) {
    DisplayCommand command;
    command.operation = DisplayOperation::translate;
    command.first = offset;
    commands_.push_back(std::move(command));
}

void RecordingPainter::clip_rect(Rect rect) {
    DisplayCommand command;
    command.operation = DisplayOperation::clip_rect;
    command.rect = rect;
    commands_.push_back(std::move(command));
}

void RecordingPainter::clip_rounded_rect(Rect rect, double radius) {
    DisplayCommand command;
    command.operation = DisplayOperation::clip_rounded_rect;
    command.rect = rect;
    command.scalar = radius;
    commands_.push_back(std::move(command));
}

void RecordingPainter::fill_rect(Rect rect, Color color) {
    DisplayCommand command;
    command.operation = DisplayOperation::fill_rect;
    command.rect = rect;
    command.color = color;
    commands_.push_back(std::move(command));
}

void RecordingPainter::fill_rounded_rect(Rect rect, double radius, Color color) {
    DisplayCommand command;
    command.operation = DisplayOperation::fill_rounded_rect;
    command.rect = rect;
    command.color = color;
    command.scalar = radius;
    commands_.push_back(std::move(command));
}

void RecordingPainter::stroke_rect(Rect rect, Color color, double width) {
    DisplayCommand command;
    command.operation = DisplayOperation::stroke_rect;
    command.rect = rect;
    command.color = color;
    command.scalar = width;
    commands_.push_back(std::move(command));
}

void RecordingPainter::stroke_rounded_rect(Rect rect, double radius, Color color,
                                           double width) {
    DisplayCommand command;
    command.operation = DisplayOperation::stroke_rounded_rect;
    command.rect = rect;
    command.color = color;
    command.scalar = radius;
    command.secondary_scalar = width;
    commands_.push_back(std::move(command));
}

void RecordingPainter::fill_linear_gradient(
    Rect rect, Point start, Point end,
    std::span<const GradientStop> stops) {
    if (!valid_gradient_stops(stops)) {
        throw std::invalid_argument("display gradient stops are invalid");
    }
    DisplayCommand command;
    command.operation = DisplayOperation::fill_linear_gradient;
    command.rect = rect;
    command.first = start;
    command.second = end;
    command.gradient_stops.assign(stops.begin(), stops.end());
    commands_.push_back(std::move(command));
}

void RecordingPainter::fill_linear_gradient_spread(
    Rect rect, Point start, Point end,
    std::span<const GradientStop> stops, GradientSpreadMode spread) {
    if (!valid_gradient_stops(stops) ||
        (spread != GradientSpreadMode::pad &&
         spread != GradientSpreadMode::repeat &&
         spread != GradientSpreadMode::reflect)) {
        throw std::invalid_argument("display gradient spread is invalid");
    }
    DisplayCommand command;
    command.operation = DisplayOperation::fill_linear_gradient_spread;
    command.rect = rect;
    command.first = start;
    command.second = end;
    command.gradient_stops.assign(stops.begin(), stops.end());
    command.gradient_spread = spread;
    commands_.push_back(std::move(command));
}

void RecordingPainter::fill_radial_gradient(
    Rect rect, Point center, Size radii,
    std::span<const GradientStop> stops) {
    if (!valid_gradient_stops(stops)) {
        throw std::invalid_argument("display gradient stops are invalid");
    }
    DisplayCommand command;
    command.operation = DisplayOperation::fill_radial_gradient;
    command.rect = rect;
    command.first = center;
    command.second = {radii.width, radii.height};
    command.gradient_stops.assign(stops.begin(), stops.end());
    commands_.push_back(std::move(command));
}

void RecordingPainter::draw_box_shadow(Rect rect, double corner_radius,
                                       Point offset, double blur_radius,
                                       double spread, Color color) {
    DisplayCommand command;
    command.operation = DisplayOperation::draw_box_shadow;
    command.rect = rect;
    command.first = offset;
    command.color = color;
    command.scalar = corner_radius;
    command.secondary_scalar = blur_radius;
    command.tertiary_scalar = spread;
    commands_.push_back(std::move(command));
}

void RecordingPainter::draw_line(Point from, Point to, Color color, double width) {
    DisplayCommand command;
    command.operation = DisplayOperation::draw_line;
    command.first = from;
    command.second = to;
    command.color = color;
    command.scalar = width;
    commands_.push_back(std::move(command));
}

void RecordingPainter::draw_text_utf8(Point origin,
                                      std::string_view text,
                                      FontSpec font,
                                      Color color) {
    DisplayCommand command;
    command.operation = DisplayOperation::draw_text_utf8;
    command.first = origin;
    command.color = color;
    command.font = font;
    command.text.assign(text);
    commands_.push_back(std::move(command));
}

Size RecordingPainter::measure_text_utf8(std::string_view text,
                                         FontSpec font) {
    // Display chunks record drawing, not renderer-specific layout. Stock
    // controls which require exact text geometry are painted directly by the
    // terminal renderer; this deterministic fallback keeps headless recording
    // stable and deliberately exposes the unresolved shaping seam.
    std::size_t scalars{};
    for (const unsigned char byte : text) {
        if ((byte & 0xc0U) != 0x80U) ++scalars;
    }
    return {static_cast<double>(scalars) * font.size * 0.55,
            font.size * 1.2};
}

void RecordingPainter::draw_image(ImageId image, Rect destination, double opacity) {
    DisplayCommand command;
    command.operation = DisplayOperation::draw_image;
    command.rect = destination;
    command.image = image;
    command.scalar = opacity;
    commands_.push_back(std::move(command));
}

void RecordingPainter::draw_live_surface(std::shared_ptr<LiveSurface> surface,
                                         Rect destination, double opacity) {
    DisplayCommand command;
    command.operation = DisplayOperation::draw_live_surface;
    command.live_surface = std::move(surface);
    command.rect = destination;
    command.scalar = opacity;
    commands_.push_back(std::move(command));
}

void RecordingPainter::draw_image_region(ImageId image, Rect source,
                                          Rect destination, double opacity) {
    DisplayCommand command;
    command.operation = DisplayOperation::draw_image_region;
    command.first = {source.x, source.y};
    command.second = {source.width, source.height};
    command.rect = destination;
    command.image = image;
    command.scalar = opacity;
    commands_.push_back(std::move(command));
}

void RecordingPainter::draw_image_region_sampled(
    ImageId image, Rect source, Rect destination, ImageSampling sampling,
    double opacity) {
    DisplayCommand command;
    command.operation = DisplayOperation::draw_image_region_sampled;
    command.first = {source.x, source.y};
    command.second = {source.width, source.height};
    command.rect = destination;
    command.image = image;
    command.image_sampling = sampling;
    command.scalar = opacity;
    commands_.push_back(std::move(command));
}

void RecordingPainter::fill_image_pattern(
    ImageId image, Size source_pixel_size, Rect destination,
    Size logical_tile_size, ImagePatternWrap wrap, double opacity) {
    DisplayCommand command;
    command.operation = DisplayOperation::fill_image_pattern;
    command.first = {source_pixel_size.width, source_pixel_size.height};
    command.second = {logical_tile_size.width, logical_tile_size.height};
    command.rect = destination;
    command.image = image;
    command.scalar = opacity;
    command.image_pattern_wrap = wrap;
    commands_.push_back(std::move(command));
}

std::shared_ptr<const DisplayChunk> RecordingPainter::finish(
    std::uint64_t generation, PaintPlane plane, Rect logical_bounds) {
    if (save_depth_ != 0U) {
        throw std::logic_error("GUI.Forms display chunk has an unbalanced painter save");
    }
    return std::make_shared<const DisplayChunk>(generation, plane, logical_bounds,
                                                std::move(commands_));
}

} // namespace gui_forms::detail
