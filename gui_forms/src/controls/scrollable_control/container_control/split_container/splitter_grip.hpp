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
    void set_visible_geometry(double origin, double width);
    void set_collapse_appearance(SplitFixedPanel panel, bool collapsed);
    void set_dragging(bool dragging);
    void set_actuator_pressed(bool pressed);
    void reset_interaction() noexcept;
    [[nodiscard]] SplitSeamState seam_state() const noexcept;
    [[nodiscard]] Rect actuator_bounds() const noexcept;
    [[nodiscard]] Insets visual_outsets() const noexcept override;
    void on_pointer(PointerEvent& event) override;
    void on_focus_changed(bool focused) override;
    void on_paint(Painter& painter, Rect local_damage) override;

private:
    [[nodiscard]] bool engaged() const noexcept;
    [[nodiscard]] Rect actuator_bounds(double cross_extent,
                                       double axis_extent) const noexcept;
    [[nodiscard]] bool point_in_proximity(Point window_point) const noexcept;

    Orientation orientation_{Orientation::vertical};
    SplitFixedPanel collapse_panel_{SplitFixedPanel::none};
    double visible_origin_{3.0};
    double visible_width_{3.0};
    bool collapsed_{};
    bool near_{};
    bool hot_{};
    bool dragging_{};
    bool actuator_pressed_{};
    bool focused_{};
};

} // namespace gui_forms
