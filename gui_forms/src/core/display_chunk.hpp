#pragma once

#include "gui_forms/control.hpp"
#include "gui_forms/display.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace gui_forms::detail {

enum class DisplayOperation : std::uint8_t {
    save,
    restore,
    translate,
    clip_rect,
    fill_rect,
    stroke_rect,
    draw_line,
    draw_text_utf8,
    draw_image,
};

struct DisplayCommand final {
    DisplayOperation operation{};
    Point first{};
    Point second{};
    Rect rect{};
    Color color{};
    FontSpec font{};
    ImageId image{};
    double scalar{};
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
    void fill_rect(Rect rect, Color color) override;
    void stroke_rect(Rect rect, Color color, double width) override;
    void draw_line(Point from, Point to, Color color, double width) override;
    void draw_text_utf8(Point origin,
                        std::string_view text,
                        FontSpec font,
                        Color color) override;
    void draw_image(ImageId image, Rect destination, double opacity) override;

    [[nodiscard]] std::shared_ptr<const DisplayChunk> finish(
        std::uint64_t generation, PaintPlane plane, Rect logical_bounds);

private:
    std::vector<DisplayCommand> commands_;
    std::uint64_t save_depth_{};
};

[[nodiscard]] std::uint64_t replay_display_chunk(const DisplayChunk& chunk,
                                                 Painter& painter);

} // namespace gui_forms::detail
