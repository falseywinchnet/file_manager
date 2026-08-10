#pragma once

#include "gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp"

namespace gui_forms {

// Source-private retained control for the visible splitter seam, enlarged hit
// target, keyboard focus cue, and optional collapse tab. SplitContainer owns
// its policy; SplitterGrip owns only the seam's visual state machine.
class SplitterGrip final : public Control {
public:
    explicit SplitterGrip(StableId stable_id);

    void set_orientation(Orientation orientation);
    void set_visible_width(double width);
    void set_collapse_appearance(SplitFixedPanel panel, bool collapsed);
    void on_focus_changed(bool focused) override;
    void on_paint(Painter& painter, Rect local_damage) override;

private:
    Orientation orientation_{Orientation::vertical};
    SplitFixedPanel collapse_panel_{SplitFixedPanel::none};
    double visible_width_{3.0};
    bool collapsed_{};
    bool focused_{};
};

} // namespace gui_forms
