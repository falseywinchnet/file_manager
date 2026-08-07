#pragma once

#include "gui_forms/animation.hpp"
#include "gui_forms/basic_controls.hpp"

#include <cstdint>

namespace gui_forms {

enum class Orientation : std::uint8_t {
    horizontal,
    vertical,
};

enum class RangeAction : std::uint8_t {
    small_decrement,
    small_increment,
    large_decrement,
    large_increment,
    first,
    last,
    thumb_track,
    thumb_position,
};

enum class TrackBarVisualStyle : std::uint8_t {
    classic,
    filled,
    compact,
};

enum class ProgressBarVisualStyle : std::uint8_t {
    blocks,
    continuous,
    marquee,
    // Source-compatible spelling retained for the original proving slice.
    pulse,
    luminance_pulse = pulse,
    marching_stripes,
    laser_etch,
};

// Orthogonal to ProgressBarVisualStyle: stripes can decorate a determinate
// fill, while marquee remains the WinForms-compatible indeterminate mode.
enum class ProgressBarOverlayStyle : std::uint8_t {
    none,
    moving_stripes,
    marching_stripes = moving_stripes,
};

// Renderer-neutral effect parameters shared by the animated determinate
// styles. Applications may replace the complete record atomically; invalid
// geometry is rejected without partially changing the active appearance.
struct ProgressBarAnimationAppearance final {
    Color luminance_color{Color::rgba(255, 255, 255, 92)};
    Color stripe_color{Color::rgba(224, 241, 252, 116)};
    Color laser_phase_color{Color::rgba(130, 222, 247, 108)};
    Color laser_edge_color{Color::rgba(196, 246, 255, 226)};
    Color laser_spark_color{Color::rgba(238, 253, 255, 236)};
    double pulse_extent{0.32};
    double laser_edge_extent{13.0};
    double laser_phase_pitch{10.0};

    friend constexpr bool operator==(const ProgressBarAnimationAppearance&,
                                     const ProgressBarAnimationAppearance&) = default;
};

struct RangeScrollEvent final {
    double old_value{};
    double new_value{};
    RangeAction action{RangeAction::thumb_position};
};

// Renderer-neutral range substrate for GUI.Forms-native sliders and the retired compatibility specimen
// ColorSlider compatibility family. Programmatic mutations publish only
// value_changed; user input publishes scroll first and value_changed second.
class RangeControl : public Control {
public:
    explicit RangeControl(StableId stable_id);

    [[nodiscard]] double minimum() const noexcept { return minimum_; }
    [[nodiscard]] double maximum() const noexcept { return maximum_; }
    [[nodiscard]] double value() const noexcept { return value_; }
    [[nodiscard]] double small_change() const noexcept { return small_change_; }
    [[nodiscard]] double large_change() const noexcept { return large_change_; }
    [[nodiscard]] Orientation orientation() const noexcept { return orientation_; }
    [[nodiscard]] double normalized_value() const noexcept;
    [[nodiscard]] const BasicControlStyle& style() const noexcept { return style_; }

    void set_range(double minimum, double maximum);
    void set_minimum(double minimum);
    void set_maximum(double maximum);
    virtual void set_value(double value);
    void set_small_change(double change);
    void set_large_change(double change);
    void set_orientation(Orientation orientation);
    void set_style(BasicControlStyle style);
    void increment(double delta);

    [[nodiscard]] Event<double, double>& range_changed() noexcept {
        return range_changed_;
    }
    [[nodiscard]] Event<double>& value_changed() noexcept {
        return value_changed_;
    }
    [[nodiscard]] Event<const RangeScrollEvent&>& scroll() noexcept {
        return scroll_;
    }

protected:
    [[nodiscard]] Rect local_bounds() const noexcept;
    bool set_value_from_input(double value, RangeAction action);

private:
    double minimum_{};
    double maximum_{100.0};
    double value_{};
    double small_change_{1.0};
    double large_change_{10.0};
    Orientation orientation_{Orientation::horizontal};
    BasicControlStyle style_;
    Event<double, double> range_changed_;
    Event<double> value_changed_;
    Event<const RangeScrollEvent&> scroll_;
};

class TrackBar : public RangeControl {
public:
    explicit TrackBar(StableId stable_id);

    [[nodiscard]] double tick_frequency() const noexcept { return tick_frequency_; }
    void set_tick_frequency(double frequency);
    [[nodiscard]] bool show_ticks() const noexcept { return show_ticks_; }
    void set_show_ticks(bool show);
    [[nodiscard]] TrackBarVisualStyle visual_style() const noexcept {
        return visual_style_;
    }
    void set_visual_style(TrackBarVisualStyle style);

    [[nodiscard]] Size measure(Size available) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    void on_pointer(PointerEvent& event) override;
    void on_key(KeyEvent& event) override;
    void on_focus_changed(bool focused) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    bool on_semantic_action(SemanticAction action,
                            std::string_view value) override;

private:
    [[nodiscard]] double value_from_window_point(Point point) const noexcept;

    double tick_frequency_{10.0};
    bool show_ticks_{true};
    TrackBarVisualStyle visual_style_{TrackBarVisualStyle::classic};
    bool tracking_{};
    bool focused_{};
};

class ProgressBar : public RangeControl {
public:
    explicit ProgressBar(StableId stable_id);

    [[nodiscard]] ProgressBarVisualStyle visual_style() const noexcept {
        return visual_style_;
    }
    void set_visual_style(ProgressBarVisualStyle style);
    [[nodiscard]] ProgressBarOverlayStyle overlay_style() const noexcept {
        return overlay_style_;
    }
    void set_overlay_style(ProgressBarOverlayStyle style);
    [[nodiscard]] double stripe_width() const noexcept { return stripe_width_; }
    void set_stripe_width(double width);
    [[nodiscard]] const ProgressBarAnimationAppearance& animation_appearance()
        const noexcept {
        return animation_appearance_;
    }
    void set_animation_appearance(ProgressBarAnimationAppearance appearance);
    [[nodiscard]] bool animation_enabled() const noexcept {
        return animation_enabled_;
    }
    void set_animation_enabled(bool enabled);
    [[nodiscard]] bool animation_paused() const noexcept {
        return motion_policy_.paused;
    }
    // Pause is distinct from disabling the animation feature: it revokes the
    // frame lease while retaining the exact phase for a deterministic resume.
    void set_animation_paused(bool paused);
    [[nodiscard]] bool reduced_motion() const noexcept {
        return motion_policy_.reduced;
    }
    // Reduced motion remains animated with a lower cadence, slower phase, and
    // limited visual excursion. It is distinct from pause and disable.
    void set_reduced_motion(bool reduced);
    // Atomically applies the two orthogonal playback policies and performs at
    // most one frame-lease transition. This avoids transient scheduling when
    // an application changes pause and reduced-motion state together.
    void set_motion_policy(bool paused, bool reduced_motion);
    // Applies the complete application policy in one transaction. The
    // enabled bit is a master playback gate and, unlike
    // set_animation_enabled(), retains the current phase and visual feature.
    void set_motion_policy(MotionPolicy policy);
    [[nodiscard]] MotionPolicy motion_policy() const noexcept {
        return motion_policy_;
    }
    [[nodiscard]] MotionPolicy effective_motion_policy() const noexcept;
    [[nodiscard]] FrameInterval animation_period() const noexcept {
        return animation_period_;
    }
    void set_animation_period(FrameInterval period);
    [[nodiscard]] double animation_phase() const noexcept {
        return animation_phase_;
    }

    [[nodiscard]] Size measure(Size available) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    void on_frame(FrameTime now) override;
    [[nodiscard]] bool hit_test_local(Point local_point) const override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

protected:
    void on_attached_to_window() override;
    void on_detached_from_window() noexcept override;

private:
    void update_animation_registration();
    [[nodiscard]] bool animated_style() const noexcept;

    ProgressBarVisualStyle visual_style_{ProgressBarVisualStyle::continuous};
    ProgressBarOverlayStyle overlay_style_{ProgressBarOverlayStyle::none};
    ProgressBarAnimationAppearance animation_appearance_{};
    double stripe_width_{7.0};
    FrameRequestToken animation_frames_;
    SubscriptionToken presentation_subscription_;
    FrameInterval animation_period_{std::chrono::milliseconds(1400)};
    FrameTime last_animation_frame_{};
    double animation_phase_{};
    bool animation_enabled_{true};
    MotionPolicy motion_policy_{};
};

enum class ScrollBarPart : std::uint8_t {
    none,
    decrement_button,
    increment_button,
    decrement_track,
    increment_track,
    thumb,
};

// A renderer-neutral desktop scrollbar. The retained control owns hit regions,
// captured thumb tracking, ordered range events, and deadline-driven repeat;
// hosts only normalize pointer/key/wheel input.
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

class HScrollBar final : public ScrollBar {
public:
    explicit HScrollBar(StableId stable_id);
};

class VScrollBar final : public ScrollBar {
public:
    explicit VScrollBar(StableId stable_id);
};

} // namespace gui_forms
