#pragma once

#include "gui_forms/controls/range_control/range_control.hpp"

#include <cstdint>
#include <string_view>

namespace gui_forms {

enum class ScrollBarPart : std::uint8_t {
    none,
    decrement_button,
    increment_button,
    decrement_track,
    increment_track,
    thumb,
};

class ScrollBar : public RangeControl {
public:
    explicit ScrollBar(StableId stable_id,
                       Orientation orientation = Orientation::vertical);

    [[nodiscard]] double button_extent() const noexcept { return button_extent_; }
    void set_button_extent(double extent);
    [[nodiscard]] double minimum_thumb_extent() const noexcept {
        return minimum_thumb_extent_;
    }
    void set_minimum_thumb_extent(double extent);
    [[nodiscard]] FrameInterval initial_repeat_delay() const noexcept {
        return initial_repeat_delay_;
    }
    void set_initial_repeat_delay(FrameInterval delay);
    [[nodiscard]] FrameInterval repeat_interval() const noexcept {
        return repeat_interval_;
    }
    void set_repeat_interval(FrameInterval interval);

    [[nodiscard]] Rect decrement_button_bounds() const noexcept;
    [[nodiscard]] Rect increment_button_bounds() const noexcept;
    [[nodiscard]] Rect track_bounds() const noexcept;
    [[nodiscard]] Rect thumb_bounds() const noexcept;
    [[nodiscard]] ScrollBarPart part_at(Point local_point) const noexcept;

    [[nodiscard]] Size measure(Size available) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    void on_pointer(PointerEvent& event) override;
    void on_key(KeyEvent& event) override;
    void on_focus_changed(bool focused) override;
    void on_frame(FrameTime now) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    bool on_semantic_action(SemanticAction action,
                            std::string_view value) override;

protected:
    void on_detached_from_window() noexcept override;

private:
    [[nodiscard]] double axis_coordinate(Point local_point) const noexcept;
    [[nodiscard]] double value_from_thumb_coordinate(double coordinate) const noexcept;
    bool apply_part(ScrollBarPart part);
    void begin_repeat(ScrollBarPart part, Point pointer);
    void stop_interaction() noexcept;

    ScrollBarPart pressed_part_{ScrollBarPart::none};
    Point repeat_pointer_{};
    double thumb_drag_offset_{};
    double button_extent_{17.0};
    double minimum_thumb_extent_{18.0};
    FrameInterval initial_repeat_delay_{std::chrono::milliseconds(400)};
    FrameInterval repeat_interval_{std::chrono::milliseconds(60)};
    FrameRequestToken repeat_frames_;
    bool focused_{};
    bool tracking_thumb_{};
};

} // namespace gui_forms
