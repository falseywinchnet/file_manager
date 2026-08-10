#pragma once

#include "gui_forms/drawing/object/drawing_object.hpp"
#include "gui_forms/drawing/types/drawing_types.hpp"

#include <array>
#include <memory>
#include <span>
#include <vector>

namespace gui_drawing {

struct ImageAttributesSnapshot final {
    bool has_color_matrix{};
    std::array<double, 25> color_matrix{};
    struct ColorRemap final {
        Color old_color;
        Color new_color;
        friend constexpr bool operator==(const ColorRemap& left,
                                         const ColorRemap& right) noexcept {
            return left.old_color == right.old_color &&
                   left.new_color == right.new_color;
        }
    };
    std::vector<ColorRemap> remap_table;
};

class ImageAttributes final : public DrawingObject {
public:
    void set_color_matrix(std::span<const double, 25> matrix);
    void reset_color_matrix();
    void set_remap_table(std::span<const ImageAttributesSnapshot::ColorRemap> table);
    void reset_remap_table();
    [[nodiscard]] std::unique_ptr<ImageAttributes> clone() const;
    [[nodiscard]] ImageAttributesSnapshot snapshot() const;

private:
    ImageAttributesSnapshot value_;
};

} // namespace gui_drawing
