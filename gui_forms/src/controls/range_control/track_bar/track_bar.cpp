#include "gui_forms/controls/range_control/track_bar/track_bar.hpp"

#include "../range_control_rendering.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace gui_forms {
using namespace range_control_detail;

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

void TrackBar::set_visual_style(TrackBarVisualStyle style_value) {
    require_mutable();
    if (visual_style_ == style_value) {
        return;
    }
    visual_style_ = style_value;
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
    if (visual_style_ == TrackBarVisualStyle::classic) {
        paint_sunken(painter, track, style(), style().face_light);
    } else {
        const Rect modern_track = horizontal
            ? Rect{track.x, track.y + 1.5, track.width, 2.0}
            : Rect{track.x + 1.5, track.y, 2.0, track.height};
        painter.fill_rect(modern_track, style().border);
        Rect filled = modern_track;
        if (horizontal) {
            filled.width *= normalized_value();
        } else {
            const double height = filled.height * normalized_value();
            filled.y += filled.height - height;
            filled.height = height;
        }
        painter.fill_rect(filled, style().accent);
    }

    if (show_ticks_ && visual_style_ != TrackBarVisualStyle::compact) {
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
    const double thumb_extent = visual_style_ == TrackBarVisualStyle::compact
        ? 8.0 : visual_style_ == TrackBarVisualStyle::filled ? 12.0 : 14.0;
    const Rect thumb = horizontal
        ? Rect{track.x + ratio * track.width - thumb_extent * 0.5,
               std::max(2.0, (bounds.height - thumb_extent) * 0.5),
               thumb_extent, thumb_extent}
        : Rect{std::max(2.0, (bounds.width - thumb_extent) * 0.5),
               track.y + (1.0 - ratio) * track.height - thumb_extent * 0.5,
               thumb_extent, thumb_extent};
    if (visual_style_ == TrackBarVisualStyle::classic) {
        paint_thumb(painter, thumb, style(), focused_);
    } else {
        painter.fill_rect(thumb, focused_ ? style().accent : style().face_light);
        painter.stroke_rect({thumb.x + 0.5, thumb.y + 0.5,
                             std::max(0.0, thumb.width - 1.0),
                             std::max(0.0, thumb.height - 1.0)},
                            style().dark_border, 1.0);
    }
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

SemanticDescriptor TrackBar::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::slider;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    std::ostringstream value_text;
    value_text << value();
    descriptor.value = value_text.str();
    descriptor.numeric_value = value();
    descriptor.minimum_value = minimum();
    descriptor.maximum_value = maximum();
    descriptor.actions = {SemanticAction::focus, SemanticAction::increment,
                          SemanticAction::decrement, SemanticAction::set_value};
    descriptor.exposed = true;
    return descriptor;
}

bool TrackBar::on_semantic_action(SemanticAction action, std::string_view value_text) {
    if (action == SemanticAction::increment || action == SemanticAction::decrement) {
        const bool increasing = action == SemanticAction::increment;
        set_value_from_input(value() + (increasing ? small_change() : -small_change()),
                             increasing ? RangeAction::small_increment
                                        : RangeAction::small_decrement);
        return true;
    }
    if (action == SemanticAction::set_value) {
        try {
            std::size_t consumed{};
            const double parsed = std::stod(std::string(value_text), &consumed);
            if (consumed != value_text.size() || !std::isfinite(parsed)) return false;
            return set_value_from_input(parsed, RangeAction::thumb_position) ||
                   parsed == value();
        } catch (const std::exception&) {
            return false;
        }
    }
    return RangeControl::on_semantic_action(action, value_text);
}

} // namespace gui_forms
