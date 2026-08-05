#pragma once

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

    [[nodiscard]] Size measure(Size available) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    void on_pointer(PointerEvent& event) override;
    void on_key(KeyEvent& event) override;
    void on_focus_changed(bool focused) override;

private:
    [[nodiscard]] double value_from_window_point(Point point) const noexcept;

    double tick_frequency_{10.0};
    bool show_ticks_{true};
    bool tracking_{};
    bool focused_{};
};

class ProgressBar : public RangeControl {
public:
    explicit ProgressBar(StableId stable_id);

    [[nodiscard]] Size measure(Size available) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] bool hit_test_local(Point local_point) const override;
};

} // namespace gui_forms
