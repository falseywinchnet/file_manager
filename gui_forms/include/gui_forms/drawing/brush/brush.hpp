#pragma once

#include "gui_forms/drawing/object/drawing_object.hpp"
#include "gui_forms/drawing/types/drawing_types.hpp"

namespace gui_drawing {

class Brush : public DrawingObject {
public:
    [[nodiscard]] virtual BrushSnapshot snapshot() const = 0;
};

class SolidBrush final : public Brush {
public:
    explicit SolidBrush(Color color);
    [[nodiscard]] Color color() const;
    [[nodiscard]] BrushSnapshot snapshot() const override;

private:
    Color color_;
};

} // namespace gui_drawing
