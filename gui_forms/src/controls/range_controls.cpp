#include "gui_forms/range_controls.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace gui_forms {
namespace {

constexpr double track_inset = 10.0;

void require_finite(double value, const char* message) {
    if (!std::isfinite(value)) {
        throw std::invalid_argument(message);
    }
}

void paint_sunken(Painter& painter, Rect bounds,
                  const BasicControlStyle& style, Color fill) {
    painter.fill_rect(bounds, fill);
    painter.draw_line({bounds.x, bounds.y},
                      {bounds.x + bounds.width, bounds.y}, style.dark_border, 1.0);
    painter.draw_line({bounds.x, bounds.y},
                      {bounds.x, bounds.y + bounds.height}, style.dark_border, 1.0);
    painter.draw_line({bounds.x, bounds.y + bounds.height - 1.0},
                      {bounds.x + bounds.width, bounds.y + bounds.height - 1.0},
                      style.highlight, 1.0);
    painter.draw_line({bounds.x + bounds.width - 1.0, bounds.y},
                      {bounds.x + bounds.width - 1.0, bounds.y + bounds.height},
                      style.highlight, 1.0);
}

void paint_thumb(Painter& painter, Rect bounds,
                 const BasicControlStyle& style, bool focused) {
    painter.fill_rect(bounds, style.face);
    painter.fill_rect({bounds.x + 1.0, bounds.y + 1.0,
                       std::max(0.0, bounds.width - 2.0),
                       std::max(0.0, (bounds.height - 2.0) * 0.42)},
                      style.face_light);
    painter.draw_line({bounds.x, bounds.y},
                      {bounds.x + bounds.width - 1.0, bounds.y},
                      style.highlight, 1.0);
    painter.draw_line({bounds.x, bounds.y},
                      {bounds.x, bounds.y + bounds.height - 1.0},
                      style.highlight, 1.0);
    painter.draw_line({bounds.x, bounds.y + bounds.height - 1.0},
                      {bounds.x + bounds.width - 1.0,
                       bounds.y + bounds.height - 1.0}, style.dark_border, 1.0);
    painter.draw_line({bounds.x + bounds.width - 1.0, bounds.y},
                      {bounds.x + bounds.width - 1.0,
                       bounds.y + bounds.height - 1.0}, style.dark_border, 1.0);
    if (focused && bounds.width > 4.0 && bounds.height > 4.0) {
        painter.stroke_rect({bounds.x + 2.0, bounds.y + 2.0,
                             bounds.width - 4.0, bounds.height - 4.0},
                            style.accent, 1.0);
    }
}

} // namespace

RangeControl::RangeControl(StableId stable_id)
    : Control(std::move(stable_id)) {}

double RangeControl::normalized_value() const noexcept {
    return (value_ - minimum_) / (maximum_ - minimum_);
}

void RangeControl::set_range(double minimum_value, double maximum_value) {
    require_mutable();
    require_finite(minimum_value, "range minimum must be finite");
    require_finite(maximum_value, "range maximum must be finite");
    if (minimum_value >= maximum_value) {
        throw std::invalid_argument("range maximum must be greater than minimum");
    }
    if (minimum_ == minimum_value && maximum_ == maximum_value) {
        return;
    }
    const double old_value = value_;
    minimum_ = minimum_value;
    maximum_ = maximum_value;
    value_ = std::clamp(value_, minimum_, maximum_);
    invalidate(Dirty::paint | Dirty::semantics);
    range_changed_.emit(minimum_, maximum_);
    if (is_alive() && old_value != value_) {
        value_changed_.emit(value_);
    }
}

void RangeControl::set_minimum(double minimum_value) {
    set_range(minimum_value, maximum_);
}

void RangeControl::set_maximum(double maximum_value) {
    set_range(minimum_, maximum_value);
}

void RangeControl::set_value(double value) {
    require_mutable();
    require_finite(value, "range value must be finite");
    if (value < minimum_ || value > maximum_) {
        throw std::out_of_range("range value is outside minimum and maximum");
    }
    if (value_ == value) {
        return;
    }
    value_ = value;
    invalidate(Dirty::paint | Dirty::semantics);
    value_changed_.emit(value_);
}

void RangeControl::set_small_change(double change) {
    require_mutable();
    require_finite(change, "small change must be finite");
    if (change <= 0.0) {
        throw std::invalid_argument("small change must be positive");
    }
    if (small_change_ == change) {
        return;
    }
    small_change_ = change;
    invalidate(Dirty::semantics);
}

void RangeControl::set_large_change(double change) {
    require_mutable();
    require_finite(change, "large change must be finite");
    if (change <= 0.0) {
        throw std::invalid_argument("large change must be positive");
    }
    if (large_change_ == change) {
        return;
    }
    large_change_ = change;
    invalidate(Dirty::semantics);
}

void RangeControl::set_orientation(Orientation orientation) {
    require_mutable();
    if (orientation_ == orientation) {
        return;
    }
    orientation_ = orientation;
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

void RangeControl::set_style(BasicControlStyle style) {
    require_mutable();
    if (style_ == style) {
        return;
    }
    style_ = std::move(style);
    invalidate(Dirty::paint | Dirty::semantics);
}

void RangeControl::increment(double delta) {
    require_mutable();
    require_finite(delta, "range increment must be finite");
    set_value(std::clamp(value_ + delta, minimum_, maximum_));
}

Rect RangeControl::local_bounds() const noexcept {
    const Rect arranged = committed_arranged_bounds();
    return {0.0, 0.0, arranged.width, arranged.height};
}

bool RangeControl::set_value_from_input(double value, RangeAction action) {
    require_mutable();
    require_finite(value, "input range value must be finite");
    const double next = std::clamp(value, minimum_, maximum_);
    if (next == value_) {
        return false;
    }
    const RangeScrollEvent event{value_, next, action};
    value_ = next;
    invalidate(Dirty::paint | Dirty::semantics);
    scroll_.emit(event);
    if (is_alive()) {
        value_changed_.emit(value_);
    }
    return true;
}

TrackBar::TrackBar(StableId stable_id)
    : RangeControl(std::move(stable_id)) {
    set_focusable(true);
    set_cursor(CursorKind::hand);
}

void TrackBar::set_tick_frequency(double frequency) {
    require_mutable();
    require_finite(frequency, "tick frequency must be finite");
    if (frequency <= 0.0) {
        throw std::invalid_argument("tick frequency must be positive");
    }
    if (tick_frequency_ == frequency) {
        return;
    }
    tick_frequency_ = frequency;
    invalidate(Dirty::paint | Dirty::semantics);
}

void TrackBar::set_show_ticks(bool show) {
    require_mutable();
    if (show_ticks_ == show) {
        return;
    }
    show_ticks_ = show;
    invalidate(Dirty::paint | Dirty::semantics);
}

Size TrackBar::measure(Size available) {
    const Rect requested = requested_bounds();
    const Size preferred = orientation() == Orientation::horizontal
        ? Size{requested.width > 0.0 ? requested.width : 160.0,
               requested.height > 0.0 ? requested.height : 28.0}
        : Size{requested.width > 0.0 ? requested.width : 28.0,
               requested.height > 0.0 ? requested.height : 160.0};
    return {std::min(available.width, preferred.width),
            std::min(available.height, preferred.height)};
}

void TrackBar::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    const bool horizontal = orientation() == Orientation::horizontal;
    const Rect track = horizontal
        ? Rect{track_inset, bounds.height * 0.5 - 2.5,
               std::max(0.0, bounds.width - track_inset * 2.0), 5.0}
        : Rect{bounds.width * 0.5 - 2.5, track_inset, 5.0,
               std::max(0.0, bounds.height - track_inset * 2.0)};
    paint_sunken(painter, track, style(), style().face_light);

    if (show_ticks_) {
        const double span = maximum() - minimum();
        const double requested_count = std::floor(span / tick_frequency_);
        const std::size_t bounded_count = static_cast<std::size_t>(
            std::min(requested_count, 256.0));
        for (std::size_t index = 0; index <= bounded_count; ++index) {
            const double ratio = bounded_count == 0U
                ? 0.0 : static_cast<double>(index) /
                    static_cast<double>(bounded_count);
            if (horizontal) {
                const double x = track.x + ratio * track.width;
                painter.draw_line({x, track.y + track.height + 2.0},
                                  {x, track.y + track.height + 5.0},
                                  style().dark_border, 1.0);
            } else {
                const double y = track.y + (1.0 - ratio) * track.height;
                painter.draw_line({track.x + track.width + 2.0, y},
                                  {track.x + track.width + 5.0, y},
                                  style().dark_border, 1.0);
            }
        }
    }

    const double ratio = normalized_value();
    const Rect thumb = horizontal
        ? Rect{track.x + ratio * track.width - 7.0, 2.0, 14.0,
               std::max(0.0, bounds.height - 8.0)}
        : Rect{2.0, track.y + (1.0 - ratio) * track.height - 7.0,
               std::max(0.0, bounds.width - 8.0), 14.0};
    paint_thumb(painter, thumb, style(), focused_);
}

double TrackBar::value_from_window_point(Point point) const noexcept {
    const Rect absolute = absolute_bounds();
    double ratio = 0.0;
    if (orientation() == Orientation::horizontal) {
        const double span = std::max(1.0, absolute.width - track_inset * 2.0);
        ratio = (point.x - absolute.x - track_inset) / span;
    } else {
        const double span = std::max(1.0, absolute.height - track_inset * 2.0);
        ratio = 1.0 - (point.y - absolute.y - track_inset) / span;
    }
    ratio = std::clamp(ratio, 0.0, 1.0);
    return minimum() + ratio * (maximum() - minimum());
}

void TrackBar::on_pointer(PointerEvent& event) {
    if (event.action == PointerAction::wheel) {
        if (event.wheel_delta.y != 0.0) {
            const bool incrementing = event.wheel_delta.y > 0.0;
            set_value_from_input(value() + (incrementing ? small_change() : -small_change()),
                                 incrementing ? RangeAction::small_increment
                                              : RangeAction::small_decrement);
            event.handled = true;
        }
        return;
    }
    if (event.button == PointerButton::primary &&
        event.action == PointerAction::down) {
        tracking_ = true;
        set_value_from_input(value_from_window_point(event.position),
                             RangeAction::thumb_track);
        event.handled = true;
    } else if (event.action == PointerAction::move && tracking_) {
        set_value_from_input(value_from_window_point(event.position),
                             RangeAction::thumb_track);
        event.handled = true;
    } else if (event.button == PointerButton::primary &&
               event.action == PointerAction::up && tracking_) {
        tracking_ = false;
        set_value_from_input(value_from_window_point(event.position),
                             RangeAction::thumb_position);
        event.handled = true;
    }
}

void TrackBar::on_key(KeyEvent& event) {
    if (event.action != KeyAction::down) {
        return;
    }
    double next = value();
    RangeAction action = RangeAction::thumb_position;
    bool recognized = true;
    switch (event.physical_key) {
    case PhysicalKey::left:
    case PhysicalKey::down:
        next -= small_change();
        action = RangeAction::small_decrement;
        break;
    case PhysicalKey::right:
    case PhysicalKey::up:
        next += small_change();
        action = RangeAction::small_increment;
        break;
    case PhysicalKey::page_down:
        next -= large_change();
        action = RangeAction::large_decrement;
        break;
    case PhysicalKey::page_up:
        next += large_change();
        action = RangeAction::large_increment;
        break;
    case PhysicalKey::home:
        next = minimum();
        action = RangeAction::first;
        break;
    case PhysicalKey::end:
        next = maximum();
        action = RangeAction::last;
        break;
    default:
        recognized = false;
        break;
    }
    if (recognized) {
        set_value_from_input(next, action);
        event.handled = true;
    }
}

void TrackBar::on_focus_changed(bool focused) {
    focused_ = focused;
    if (!focused) {
        tracking_ = false;
    }
    invalidate(Dirty::paint | Dirty::semantics);
}

ProgressBar::ProgressBar(StableId stable_id)
    : RangeControl(std::move(stable_id)) {
    set_focusable(false);
}

Size ProgressBar::measure(Size available) {
    const Rect requested = requested_bounds();
    const Size preferred = orientation() == Orientation::horizontal
        ? Size{requested.width > 0.0 ? requested.width : 160.0,
               requested.height > 0.0 ? requested.height : 20.0}
        : Size{requested.width > 0.0 ? requested.width : 20.0,
               requested.height > 0.0 ? requested.height : 160.0};
    return {std::min(available.width, preferred.width),
            std::min(available.height, preferred.height)};
}

void ProgressBar::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    paint_sunken(painter, bounds, style(), style().paper);
    const double ratio = normalized_value();
    Rect fill{2.0, 2.0, std::max(0.0, bounds.width - 4.0),
              std::max(0.0, bounds.height - 4.0)};
    if (orientation() == Orientation::horizontal) {
        fill.width *= ratio;
    } else {
        const double filled_height = fill.height * ratio;
        fill.y += fill.height - filled_height;
        fill.height = filled_height;
    }
    painter.fill_rect(fill, enabled() ? style().accent : style().border);
    if (fill.width > 0.0 && fill.height > 3.0) {
        painter.fill_rect({fill.x, fill.y, fill.width, 3.0}, style().accent_light);
    }
}

bool ProgressBar::hit_test_local(Point) const {
    return false;
}

} // namespace gui_forms
