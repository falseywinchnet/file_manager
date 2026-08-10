#pragma once

#include "gui_forms/controls/scrollable_control/scrollable_control.hpp"
#include "gui_forms/theme.hpp"

#include <cstdint>
#include <optional>

namespace gui_forms {

enum class BorderStyle : std::uint8_t {
    none,
    line,
    sunken,
    raised,
};

class Panel : public ScrollableControl {
public:
    explicit Panel(StableId stable_id);

    [[nodiscard]] BorderStyle border_style() const noexcept { return border_style_; }
    void set_border_style(BorderStyle style);
    [[nodiscard]] Color background() const noexcept;
    [[nodiscard]] bool has_background_override() const noexcept {
        return background_override_.has_value();
    }
    void set_background(Color color);
    void clear_background();
    [[nodiscard]] const BasicControlStyle& style() const noexcept;
    [[nodiscard]] bool has_style_override() const noexcept {
        return style_override_.has_value();
    }
    void set_style(BasicControlStyle style);
    void clear_style();
    [[nodiscard]] ControlVisualRole visual_role() const noexcept {
        return visual_role_;
    }
    void set_visual_role(ControlVisualRole role);

    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] Insets visual_outsets() const noexcept override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

protected:
    [[nodiscard]] Rect local_bounds() const noexcept;
    void paint_panel(Painter& painter, Rect bounds) const;

private:
    std::optional<BasicControlStyle> style_override_;
    std::optional<Color> background_override_;
    ControlVisualRole visual_role_{ControlVisualRole::panel};
    BorderStyle border_style_{BorderStyle::none};
};

} // namespace gui_forms
