#pragma once

#include "gui_forms/controls/scrollable_control/container_control/container_control.hpp"

namespace gui_forms {

class SplitterPanel final : public ContainerControl {
public:
    explicit SplitterPanel(StableId stable_id);

    // A SplitterPanel is the allocated pane surface, not merely an invisible
    // child owner. Painting the background from its committed bounds keeps the
    // visual allocation truthful while the splitter is moving.
    [[nodiscard]] Color background() const noexcept { return background_; }
    void set_background(Color color);
    void on_paint(Painter& painter, Rect local_damage) override;

private:
    Color background_{Color::rgba(0, 0, 0, 0)};
};

} // namespace gui_forms
