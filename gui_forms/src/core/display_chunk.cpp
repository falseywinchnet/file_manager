#include "display_chunk.hpp"

#include <stdexcept>
#include <utility>

namespace gui_forms::detail {

DisplayChunk::DisplayChunk(std::uint64_t generation,
                           PaintPlane plane,
                           Rect logical_bounds,
                           std::vector<DisplayCommand> commands)
    : generation_(generation), plane_(plane), logical_bounds_(logical_bounds),
      commands_(std::move(commands)) {}

DisplayChunkInfo DisplayChunk::info() const noexcept {
    return {generation_, plane_, logical_bounds_, commands_.size()};
}

void RecordingPainter::save() {
    commands_.push_back({.operation = DisplayOperation::save});
    ++save_depth_;
}

void RecordingPainter::restore() {
    if (save_depth_ == 0U) {
        throw std::logic_error("GUI.Forms display chunk restored without a matching save");
    }
    commands_.push_back({.operation = DisplayOperation::restore});
    --save_depth_;
}

void RecordingPainter::translate(Point offset) {
    commands_.push_back({.operation = DisplayOperation::translate, .first = offset});
}

void RecordingPainter::clip_rect(Rect rect) {
    commands_.push_back({.operation = DisplayOperation::clip_rect, .rect = rect});
}

void RecordingPainter::fill_rect(Rect rect, Color color) {
    commands_.push_back(
        {.operation = DisplayOperation::fill_rect, .rect = rect, .color = color});
}

void RecordingPainter::stroke_rect(Rect rect, Color color, double width) {
    commands_.push_back({.operation = DisplayOperation::stroke_rect,
                         .rect = rect,
                         .color = color,
                         .scalar = width});
}

void RecordingPainter::draw_line(Point from, Point to, Color color, double width) {
    commands_.push_back({.operation = DisplayOperation::draw_line,
                         .first = from,
                         .second = to,
                         .color = color,
                         .scalar = width});
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
    commands_.push_back({.operation = DisplayOperation::draw_image,
                         .rect = destination,
                         .image = image,
                         .scalar = opacity});
}

std::shared_ptr<const DisplayChunk> RecordingPainter::finish(
    std::uint64_t generation, PaintPlane plane, Rect logical_bounds) {
    if (save_depth_ != 0U) {
        throw std::logic_error("GUI.Forms display chunk has an unbalanced painter save");
    }
    return std::make_shared<const DisplayChunk>(generation, plane, logical_bounds,
                                                std::move(commands_));
}

std::uint64_t replay_display_chunk(const DisplayChunk& chunk, Painter& painter) {
    for (const DisplayCommand& command : chunk.commands()) {
        switch (command.operation) {
        case DisplayOperation::save: painter.save(); break;
        case DisplayOperation::restore: painter.restore(); break;
        case DisplayOperation::translate: painter.translate(command.first); break;
        case DisplayOperation::clip_rect: painter.clip_rect(command.rect); break;
        case DisplayOperation::fill_rect:
            painter.fill_rect(command.rect, command.color);
            break;
        case DisplayOperation::stroke_rect:
            painter.stroke_rect(command.rect, command.color, command.scalar);
            break;
        case DisplayOperation::draw_line:
            painter.draw_line(command.first, command.second, command.color,
                              command.scalar);
            break;
        case DisplayOperation::draw_text_utf8:
            painter.draw_text_utf8(command.first, command.text, command.font,
                                   command.color);
            break;
        case DisplayOperation::draw_image:
            painter.draw_image(command.image, command.rect, command.scalar);
            break;
        }
    }
    return chunk.commands().size();
}

} // namespace gui_forms::detail
