#pragma once

#include "gui_forms/types/painter/painter.hpp"

#include <string>
#include <vector>

namespace gui_forms::test_support {

struct PaintedText final {
    std::string text;
    FontSpec font;
    Point origin;
};

class TypographyPainter final : public Painter {
public:
    void save() override {}
    void restore() override {}
    void translate(Point) override {}
    void clip_rect(Rect) override {}
    void fill_rect(Rect, Color) override {}
    void stroke_rect(Rect, Color, double) override {}
    void draw_line(Point, Point, Color, double) override {}
    void draw_image(ImageId, Rect, double) override {}
    void draw_text_utf8(Point origin, std::string_view text,
                        FontSpec font, Color) override {
        texts.push_back({std::string(text), font, origin});
    }
    [[nodiscard]] const PaintedText* find(std::string_view text) const {
        for (const PaintedText& painted : texts) {
            if (painted.text == text) return &painted;
        }
        return nullptr;
    }
    std::vector<PaintedText> texts;
};

} // namespace gui_forms::test_support
