#include "gui_forms/range_controls.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>
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

ProgressBar::ProgressBar(StableId stable_id)
    : RangeControl(std::move(stable_id)) {
    set_focusable(false);
}

void ProgressBar::set_visual_style(ProgressBarVisualStyle style_value) {
    require_mutable();
    if (visual_style_ == style_value) {
        return;
    }
    visual_style_ = style_value;
    animation_phase_ = 0.0;
    update_animation_registration();
    invalidate(Dirty::paint | Dirty::semantics);
}

void ProgressBar::set_overlay_style(ProgressBarOverlayStyle style_value) {
    require_mutable();
    if (overlay_style_ == style_value) return;
    overlay_style_ = style_value;
    animation_phase_ = 0.0;
    update_animation_registration();
    invalidate(Dirty::paint | Dirty::semantics);
}

void ProgressBar::set_stripe_width(double width) {
    require_mutable();
    require_finite(width, "progress stripe width must be finite");
    if (width < 2.0 || width > 32.0) {
        throw std::invalid_argument(
            "progress stripe width must be between 2 and 32 pixels");
    }
    if (stripe_width_ == width) return;
    stripe_width_ = width;
    invalidate(Dirty::paint);
}

void ProgressBar::set_animation_enabled(bool enabled_value) {
    require_mutable();
    if (animation_enabled_ == enabled_value) {
        return;
    }
    animation_enabled_ = enabled_value;
    if (!animation_enabled_) {
        animation_phase_ = 0.0;
    }
    update_animation_registration();
    invalidate(Dirty::paint | Dirty::semantics);
}

void ProgressBar::set_animation_paused(bool paused) {
    MotionPolicy policy = motion_policy_;
    policy.paused = paused;
    set_motion_policy(policy);
}

void ProgressBar::set_reduced_motion(bool reduced) {
    MotionPolicy policy = motion_policy_;
    policy.reduced = reduced;
    set_motion_policy(policy);
}

void ProgressBar::set_motion_policy(bool paused, bool reduced) {
    MotionPolicy policy = motion_policy_;
    policy.paused = paused;
    policy.reduced = reduced;
    set_motion_policy(policy);
}

void ProgressBar::set_motion_policy(MotionPolicy policy) {
    require_mutable();
    if (motion_policy_ == policy) return;
    motion_policy_ = policy;
    update_animation_registration();
    invalidate(Dirty::paint | Dirty::semantics);
}

MotionPolicy ProgressBar::effective_motion_policy() const noexcept {
    MotionPolicy policy = motion_policy_;
    if (window() != nullptr &&
        window()->presentation_settings().reduced_motion) {
        policy.reduced = true;
    }
    return policy;
}

void ProgressBar::set_animation_period(FrameInterval period) {
    require_mutable();
    if (period < std::chrono::milliseconds(100)) {
        throw std::invalid_argument(
            "progress animation period must be at least 100 milliseconds");
    }
    if (animation_period_ == period) {
        return;
    }
    animation_period_ = period;
    last_animation_frame_ = FrameClock::now();
    invalidate(Dirty::paint | Dirty::semantics);
}

bool ProgressBar::animated_style() const noexcept {
    return visual_style_ == ProgressBarVisualStyle::marquee ||
           visual_style_ == ProgressBarVisualStyle::pulse ||
           overlay_style_ == ProgressBarOverlayStyle::moving_stripes;
}

void ProgressBar::update_animation_registration() {
    animation_frames_.disconnect();
    const MotionPolicy policy = effective_motion_policy();
    if (window() == nullptr || !animation_enabled_ || !policy.active() ||
        !animated_style()) {
        return;
    }
    const FrameInterval interval = policy.frame_interval(
        std::chrono::milliseconds(16));
    last_animation_frame_ = FrameClock::now();
    animation_frames_ = window()->activate_surface(
        shared_from_this(), interval, last_animation_frame_ + interval);
}

void ProgressBar::on_attached_to_window() {
    RangeControl::on_attached_to_window();
    if (window() != nullptr) {
        presentation_subscription_ = window()->presentation_changed().subscribe(
            *this, [this](const PresentationSettings&) {
                update_animation_registration();
                invalidate(Dirty::paint | Dirty::semantics);
            });
    }
    update_animation_registration();
}

void ProgressBar::on_detached_from_window() noexcept {
    presentation_subscription_.disconnect();
    animation_frames_.disconnect();
    RangeControl::on_detached_from_window();
}

void ProgressBar::on_frame(FrameTime now) {
    const MotionPolicy policy = effective_motion_policy();
    if (!animation_enabled_ || !policy.active() ||
        !animated_style()) {
        return;
    }
    const double elapsed = std::max(
        0.0, std::chrono::duration<double>(now - last_animation_frame_).count());
    last_animation_frame_ = now;
    const double period =
        std::chrono::duration<double>(animation_period_).count();
    animation_phase_ = period <= 0.0
        ? 0.0
        : std::fmod(animation_phase_ +
                        elapsed * policy.speed_scale() / period,
                    1.0);
    if (animation_phase_ < 0.0) {
        animation_phase_ += 1.0;
    }
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
    const MotionPolicy policy = effective_motion_policy();
    const double presented_phase = policy.presentation_phase(animation_phase_);
    if (visual_style_ == ProgressBarVisualStyle::marquee) {
        if (orientation() == Orientation::horizontal) {
            const double band = std::max(18.0, fill.width * 0.28);
            fill.x += (fill.width + band) * presented_phase - band;
            fill.width = band;
            fill = Rect::intersection(fill, {2.0, 2.0,
                                             std::max(0.0, bounds.width - 4.0),
                                             std::max(0.0, bounds.height - 4.0)});
        } else {
            const double band = std::max(18.0, fill.height * 0.28);
            fill.y += (fill.height + band) * (1.0 - presented_phase) - band;
            fill.height = band;
            fill = Rect::intersection(fill, {2.0, 2.0,
                                             std::max(0.0, bounds.width - 4.0),
                                             std::max(0.0, bounds.height - 4.0)});
        }
    } else {
        if (orientation() == Orientation::horizontal) {
            fill.width *= ratio;
        } else {
            const double filled_height = fill.height * ratio;
            fill.y += fill.height - filled_height;
            fill.height = filled_height;
        }
    }

    const Color progress_color = enabled() ? style().accent : style().border;
    if (visual_style_ == ProgressBarVisualStyle::blocks) {
        constexpr double gap = 2.0;
        constexpr double block = 9.0;
        if (orientation() == Orientation::horizontal) {
            for (double x = fill.x; x + block <= fill.x + fill.width; x += block + gap) {
                painter.fill_rect({x, fill.y, block, fill.height}, progress_color);
            }
        } else {
            for (double y = fill.y + fill.height - block; y >= fill.y; y -= block + gap) {
                painter.fill_rect({fill.x, y, fill.width, block}, progress_color);
            }
        }
    } else {
        painter.fill_rect(fill, progress_color);
    }
    if (fill.width > 0.0 && fill.height > 3.0) {
        if (visual_style_ == ProgressBarVisualStyle::pulse) {
            const double highlight_width = std::max(8.0, fill.width * 0.18);
            const double x = fill.x +
                std::max(0.0, fill.width - highlight_width) * presented_phase;
            painter.fill_rect({x, fill.y, std::min(highlight_width, fill.width),
                               fill.height}, style().accent_light);
        } else {
            painter.fill_rect({fill.x, fill.y, fill.width, 3.0},
                              style().accent_light);
        }
    }
    if (overlay_style_ == ProgressBarOverlayStyle::moving_stripes &&
        fill.width > 0.0 && fill.height > 0.0) {
        // A retained clip makes the overlay reusable for both orientations and
        // for a marquee band without allowing a diagonal to escape the fill.
        const double pitch = stripe_width_ * 2.0;
        const double travel = animation_enabled_
            ? presented_phase * pitch : 0.0;
        painter.save();
        painter.clip_rect(fill);
        const double begin = fill.x - fill.height - pitch + travel;
        const double end = fill.x + fill.width + fill.height + pitch;
        for (double x = begin; x <= end; x += pitch) {
            painter.draw_line({x, fill.y + fill.height},
                              {x + fill.height, fill.y},
                              style().accent_light, stripe_width_);
        }
        painter.restore();
    }
}

bool ProgressBar::hit_test_local(Point) const {
    return false;
}

SemanticDescriptor ProgressBar::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::progress_bar;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    std::ostringstream value_text;
    value_text << value();
    descriptor.value = value_text.str();
    descriptor.numeric_value = value();
    descriptor.minimum_value = minimum();
    descriptor.maximum_value = maximum();
    if (animation_enabled_ && effective_motion_policy().active() &&
        animated_style()) {
        descriptor.states |= SemanticState::busy;
    }
    descriptor.exposed = true;
    return descriptor;
}

ScrollBar::ScrollBar(StableId stable_id, Orientation orientation)
    : RangeControl(std::move(stable_id)) {
    set_orientation(orientation);
    set_focusable(true);
    set_cursor(CursorKind::arrow);
}

HScrollBar::HScrollBar(StableId stable_id)
    : ScrollBar(std::move(stable_id), Orientation::horizontal) {}

VScrollBar::VScrollBar(StableId stable_id)
    : ScrollBar(std::move(stable_id), Orientation::vertical) {}

void ScrollBar::set_button_extent(double extent) {
    require_mutable();
    require_finite(extent, "scrollbar button extent must be finite");
    if (extent < 8.0 || extent > 64.0) {
        throw std::invalid_argument("scrollbar button extent must be between 8 and 64");
    }
    if (button_extent_ == extent) return;
    button_extent_ = extent;
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

void ScrollBar::set_minimum_thumb_extent(double extent) {
    require_mutable();
    require_finite(extent, "scrollbar thumb extent must be finite");
    if (extent < 8.0 || extent > 128.0) {
        throw std::invalid_argument("scrollbar thumb extent must be between 8 and 128");
    }
    if (minimum_thumb_extent_ == extent) return;
    minimum_thumb_extent_ = extent;
    invalidate(Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

void ScrollBar::set_initial_repeat_delay(FrameInterval delay) {
    require_mutable();
    if (delay < std::chrono::milliseconds(100) ||
        delay > std::chrono::seconds(2)) {
        throw std::invalid_argument(
            "scrollbar initial repeat delay must be between 100ms and 2s");
    }
    initial_repeat_delay_ = delay;
}

void ScrollBar::set_repeat_interval(FrameInterval interval) {
    require_mutable();
    if (interval < std::chrono::milliseconds(16) ||
        interval > std::chrono::milliseconds(500)) {
        throw std::invalid_argument(
            "scrollbar repeat interval must be between 16ms and 500ms");
    }
    repeat_interval_ = interval;
}

Rect ScrollBar::decrement_button_bounds() const noexcept {
    const Rect bounds = local_bounds();
    const double axis = orientation() == Orientation::horizontal
        ? bounds.width : bounds.height;
    const double extent = std::min(button_extent_, axis * 0.5);
    return orientation() == Orientation::horizontal
        ? Rect{0.0, 0.0, extent, bounds.height}
        : Rect{0.0, 0.0, bounds.width, extent};
}

Rect ScrollBar::increment_button_bounds() const noexcept {
    const Rect bounds = local_bounds();
    const double axis = orientation() == Orientation::horizontal
        ? bounds.width : bounds.height;
    const double extent = std::min(button_extent_, axis * 0.5);
    return orientation() == Orientation::horizontal
        ? Rect{std::max(0.0, bounds.width - extent), 0.0, extent, bounds.height}
        : Rect{0.0, std::max(0.0, bounds.height - extent), bounds.width, extent};
}

Rect ScrollBar::track_bounds() const noexcept {
    const Rect bounds = local_bounds();
    const Rect decrement = decrement_button_bounds();
    const Rect increment = increment_button_bounds();
    return orientation() == Orientation::horizontal
        ? Rect{decrement.width, 0.0,
               std::max(0.0, increment.x - decrement.width), bounds.height}
        : Rect{0.0, decrement.height, bounds.width,
               std::max(0.0, increment.y - decrement.height)};
}

Rect ScrollBar::thumb_bounds() const noexcept {
    const Rect track = track_bounds();
    const double track_extent = orientation() == Orientation::horizontal
        ? track.width : track.height;
    if (track_extent <= 0.0) return track;
    const double range = maximum() - minimum();
    const double proportion = large_change() / (range + large_change());
    const double thumb_extent = std::clamp(track_extent * proportion,
        std::min(minimum_thumb_extent_, track_extent), track_extent);
    const double movable = std::max(0.0, track_extent - thumb_extent);
    const double position = normalized_value() * movable;
    return orientation() == Orientation::horizontal
        ? Rect{track.x + position, track.y, thumb_extent, track.height}
        : Rect{track.x, track.y + position, track.width, thumb_extent};
}

double ScrollBar::axis_coordinate(Point local_point) const noexcept {
    return orientation() == Orientation::horizontal ? local_point.x : local_point.y;
}

ScrollBarPart ScrollBar::part_at(Point local_point) const noexcept {
    if (!local_bounds().contains(local_point)) return ScrollBarPart::none;
    if (decrement_button_bounds().contains(local_point)) {
        return ScrollBarPart::decrement_button;
    }
    if (increment_button_bounds().contains(local_point)) {
        return ScrollBarPart::increment_button;
    }
    const Rect thumb = thumb_bounds();
    if (thumb.contains(local_point)) return ScrollBarPart::thumb;
    const double coordinate = axis_coordinate(local_point);
    const double thumb_start = orientation() == Orientation::horizontal
        ? thumb.x : thumb.y;
    return coordinate < thumb_start ? ScrollBarPart::decrement_track
                                    : ScrollBarPart::increment_track;
}

double ScrollBar::value_from_thumb_coordinate(double coordinate) const noexcept {
    const Rect track = track_bounds();
    const Rect thumb = thumb_bounds();
    const double track_start = orientation() == Orientation::horizontal
        ? track.x : track.y;
    const double track_extent = orientation() == Orientation::horizontal
        ? track.width : track.height;
    const double thumb_extent = orientation() == Orientation::horizontal
        ? thumb.width : thumb.height;
    const double movable = std::max(0.0, track_extent - thumb_extent);
    if (movable <= 0.0) return minimum();
    const double ratio = std::clamp((coordinate - track_start) / movable, 0.0, 1.0);
    return minimum() + ratio * (maximum() - minimum());
}

bool ScrollBar::apply_part(ScrollBarPart part) {
    switch (part) {
    case ScrollBarPart::decrement_button:
        return set_value_from_input(value() - small_change(),
                                    RangeAction::small_decrement);
    case ScrollBarPart::increment_button:
        return set_value_from_input(value() + small_change(),
                                    RangeAction::small_increment);
    case ScrollBarPart::decrement_track:
        return set_value_from_input(value() - large_change(),
                                    RangeAction::large_decrement);
    case ScrollBarPart::increment_track:
        return set_value_from_input(value() + large_change(),
                                    RangeAction::large_increment);
    case ScrollBarPart::none:
    case ScrollBarPart::thumb:
        return false;
    }
    return false;
}

void ScrollBar::begin_repeat(ScrollBarPart part, Point pointer) {
    repeat_frames_.disconnect();
    repeat_pointer_ = pointer;
    if (window() == nullptr || part == ScrollBarPart::none ||
        part == ScrollBarPart::thumb) return;
    const FrameTime now = FrameClock::now();
    repeat_frames_ = window()->activate_surface(
        shared_from_this(), repeat_interval_, now + initial_repeat_delay_);
}

void ScrollBar::stop_interaction() noexcept {
    repeat_frames_.disconnect();
    pressed_part_ = ScrollBarPart::none;
    tracking_thumb_ = false;
    thumb_drag_offset_ = 0.0;
}

Size ScrollBar::measure(Size available) {
    const Rect requested = requested_bounds();
    const Size preferred = orientation() == Orientation::horizontal
        ? Size{requested.width > 0.0 ? requested.width : 180.0,
               requested.height > 0.0 ? requested.height : 18.0}
        : Size{requested.width > 0.0 ? requested.width : 18.0,
               requested.height > 0.0 ? requested.height : 180.0};
    return {std::min(available.width, preferred.width),
            std::min(available.height, preferred.height)};
}

void ScrollBar::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    const BasicControlStyle& colors = style();
    const Rect track = track_bounds();
    painter.fill_rect(bounds, colors.face);
    paint_sunken(painter, track, colors, colors.face_light);

    const auto paint_button = [&](Rect button, ScrollBarPart part, bool incrementing) {
        paint_thumb(painter, button, colors, pressed_part_ == part);
        const double cx = button.x + button.width * 0.5;
        const double cy = button.y + button.height * 0.5;
        const Color arrow = enabled() ? colors.dark_border : colors.disabled_text;
        if (orientation() == Orientation::horizontal) {
            const double direction = incrementing ? 1.0 : -1.0;
            painter.draw_line({cx - 2.0 * direction, cy - 4.0},
                              {cx + 2.0 * direction, cy}, arrow, 1.5);
            painter.draw_line({cx + 2.0 * direction, cy},
                              {cx - 2.0 * direction, cy + 4.0}, arrow, 1.5);
        } else {
            const double direction = incrementing ? 1.0 : -1.0;
            painter.draw_line({cx - 4.0, cy - 2.0 * direction},
                              {cx, cy + 2.0 * direction}, arrow, 1.5);
            painter.draw_line({cx, cy + 2.0 * direction},
                              {cx + 4.0, cy - 2.0 * direction}, arrow, 1.5);
        }
    };
    paint_button(decrement_button_bounds(), ScrollBarPart::decrement_button, false);
    paint_button(increment_button_bounds(), ScrollBarPart::increment_button, true);
    paint_thumb(painter, thumb_bounds(), colors, focused_ || tracking_thumb_);
}

void ScrollBar::on_pointer(PointerEvent& event) {
    if (!eligible_for_input()) return;
    const Rect absolute = absolute_bounds();
    const Point local{event.position.x - absolute.x, event.position.y - absolute.y};
    if (event.action == PointerAction::wheel && event.wheel_delta.y != 0.0) {
        const bool decrementing = event.wheel_delta.y > 0.0;
        set_value_from_input(value() + (decrementing ? -small_change() : small_change()),
                             decrementing ? RangeAction::small_decrement
                                          : RangeAction::small_increment);
        event.handled = true;
        return;
    }
    if (event.action == PointerAction::down &&
        event.button == PointerButton::primary) {
        if (window() != nullptr) static_cast<void>(window()->request_focus(shared_from_this()));
        pressed_part_ = part_at(local);
        if (pressed_part_ == ScrollBarPart::none) return;
        set_pointer_capture(true);
        if (pressed_part_ == ScrollBarPart::thumb) {
            const Rect thumb = thumb_bounds();
            const double thumb_start = orientation() == Orientation::horizontal
                ? thumb.x : thumb.y;
            thumb_drag_offset_ = axis_coordinate(local) - thumb_start;
            tracking_thumb_ = true;
        } else {
            static_cast<void>(apply_part(pressed_part_));
            begin_repeat(pressed_part_, local);
        }
        invalidate(Dirty::paint);
        event.handled = true;
        return;
    }
    if (event.action == PointerAction::move && tracking_thumb_) {
        const double coordinate = axis_coordinate(local) - thumb_drag_offset_;
        set_value_from_input(value_from_thumb_coordinate(coordinate),
                             RangeAction::thumb_track);
        event.handled = true;
        return;
    }
    if (event.action == PointerAction::move && pressed_part_ != ScrollBarPart::none) {
        repeat_pointer_ = local;
        event.handled = true;
        return;
    }
    if (event.action == PointerAction::up &&
        event.button == PointerButton::primary &&
        pressed_part_ != ScrollBarPart::none) {
        const bool was_thumb = tracking_thumb_;
        if (has_pointer_capture()) set_pointer_capture(false);
        stop_interaction();
        if (was_thumb) invalidate(Dirty::paint | Dirty::semantics);
        else invalidate(Dirty::paint);
        event.handled = true;
    }
}

void ScrollBar::on_key(KeyEvent& event) {
    if (!focused_ || event.action != KeyAction::down) return;
    double next = value();
    RangeAction action = RangeAction::thumb_position;
    bool recognized = true;
    switch (event.physical_key) {
    case PhysicalKey::left:
    case PhysicalKey::up:
        next -= small_change(); action = RangeAction::small_decrement; break;
    case PhysicalKey::right:
    case PhysicalKey::down:
        next += small_change(); action = RangeAction::small_increment; break;
    case PhysicalKey::page_up:
        next -= large_change(); action = RangeAction::large_decrement; break;
    case PhysicalKey::page_down:
        next += large_change(); action = RangeAction::large_increment; break;
    case PhysicalKey::home:
        next = minimum(); action = RangeAction::first; break;
    case PhysicalKey::end:
        next = maximum(); action = RangeAction::last; break;
    default:
        recognized = false; break;
    }
    if (recognized) {
        static_cast<void>(set_value_from_input(next, action));
        event.handled = true;
    }
}

void ScrollBar::on_focus_changed(bool focused) {
    focused_ = focused;
    if (!focused) {
        if (has_pointer_capture()) set_pointer_capture(false);
        stop_interaction();
    }
    invalidate(Dirty::paint | Dirty::semantics);
}

void ScrollBar::on_frame(FrameTime) {
    if (pressed_part_ == ScrollBarPart::decrement_track ||
        pressed_part_ == ScrollBarPart::increment_track) {
        if (part_at(repeat_pointer_) != pressed_part_) {
            repeat_frames_.disconnect();
            return;
        }
    }
    if (!apply_part(pressed_part_)) repeat_frames_.disconnect();
}

void ScrollBar::on_detached_from_window() noexcept {
    stop_interaction();
    RangeControl::on_detached_from_window();
}

SemanticDescriptor ScrollBar::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::scroll_bar;
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

bool ScrollBar::on_semantic_action(SemanticAction action,
                                   std::string_view value_text) {
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
