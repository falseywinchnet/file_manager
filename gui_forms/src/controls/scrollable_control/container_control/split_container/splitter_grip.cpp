#include "splitter_grip.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

namespace gui_forms {

namespace {

constexpr double expanded_actuator_width = 12.0;
constexpr double expanded_actuator_extent = 42.0;
constexpr double rest_actuator_width = 7.0;
constexpr double rest_actuator_extent = 28.0;
constexpr Point engaged_shadow_offset{0.0, 2.0};
constexpr double engaged_shadow_blur = 3.0;
constexpr double engaged_shadow_reach = engaged_shadow_blur * 3.0;

} // namespace

SplitterGrip::SplitterGrip(StableId stable_id)
    : Control(std::move(stable_id)) {
    set_focusable(true);
}

void SplitterGrip::set_orientation(Orientation orientation) {
    if (orientation_ == orientation) return;
    orientation_ = orientation;
    set_cursor(orientation == Orientation::vertical
                   ? CursorKind::resize_horizontal
                   : CursorKind::resize_vertical);
    invalidate(Dirty::paint | Dirty::semantics | Dirty::accessibility);
}

void SplitterGrip::set_visible_width(double width) {
    if (visible_width_ == width) return;
    visible_width_ = width;
    invalidate(Dirty::paint);
}

void SplitterGrip::set_collapse_appearance(SplitFixedPanel panel,
                                            bool collapsed) {
    if (collapse_panel_ == panel && collapsed_ == collapsed) return;
    collapse_panel_ = panel;
    collapsed_ = collapsed;
    invalidate(Dirty::paint | Dirty::semantics | Dirty::accessibility);
}

void SplitterGrip::set_interaction_active(const bool active) {
    if (active_ == active) return;
    active_ = active;
    invalidate(Dirty::paint);
}

Rect SplitterGrip::actuator_bounds() const noexcept {
    return actuator_bounds(true);
}

Insets SplitterGrip::visual_outsets() const noexcept {
    // Theme and renderer damage law conservatively treats an outer shadow as
    // blur*3 plus its offset. Declare the union of that envelope and the
    // engaged actuator without widening layout or hit testing.
    const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                      committed_arranged_bounds().height};
    const Rect tab = actuator_bounds(true);
    const Rect shadow{
        tab.x + engaged_shadow_offset.x - engaged_shadow_reach,
        tab.y + engaged_shadow_offset.y - engaged_shadow_reach,
        tab.width + engaged_shadow_reach * 2.0,
        tab.height + engaged_shadow_reach * 2.0};
    const double left = std::min(tab.x, shadow.x);
    const double top = std::min(tab.y, shadow.y);
    const double right = std::max(tab.x + tab.width,
                                  shadow.x + shadow.width);
    const double bottom = std::max(tab.y + tab.height,
                                   shadow.y + shadow.height);
    return {std::max(0.0, -left), std::max(0.0, -top),
            std::max(0.0, right - bounds.width),
            std::max(0.0, bottom - bounds.height)};
}

void SplitterGrip::on_pointer(PointerEvent& event) {
    if (event.action != PointerAction::enter &&
        event.action != PointerAction::move &&
        event.action != PointerAction::leave) return;
    const bool hovered = event.action != PointerAction::leave &&
                         point_in_proximity(event.position);
    if (hovered_ == hovered) return;
    hovered_ = hovered;
    invalidate(Dirty::paint);
}

void SplitterGrip::on_focus_changed(bool focused) {
    focused_ = focused;
    invalidate(invalidation::focus);
}

bool SplitterGrip::engaged() const noexcept {
    return hovered_ || active_ ||
        (focused_ && window() != nullptr && (*window()).focus_cue_visible());
}

Rect SplitterGrip::actuator_bounds(const bool expanded) const noexcept {
    const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                      committed_arranged_bounds().height};
    if (orientation_ == Orientation::vertical) {
        const double width = expanded ? expanded_actuator_width
                                      : rest_actuator_width;
        const double height = std::min(
            expanded ? expanded_actuator_extent : rest_actuator_extent,
            bounds.height);
        return {(bounds.width - width) * 0.5,
                std::max(0.0, (bounds.height - height) * 0.5), width, height};
    }
    const double width = std::min(
        expanded ? expanded_actuator_extent : rest_actuator_extent,
        bounds.width);
    const double height = expanded ? expanded_actuator_width
                                   : rest_actuator_width;
    return {std::max(0.0, (bounds.width - width) * 0.5),
            (bounds.height - height) * 0.5, width, height};
}

bool SplitterGrip::point_in_proximity(const Point window_point) const noexcept {
    const Point local = point_from_window(window_point);
    return actuator_bounds(true).contains(local);
}

void SplitterGrip::on_paint(Painter& painter, Rect) {
    const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                      committed_arranged_bounds().height};
    const BasicControlStyle style;
    if (orientation_ == Orientation::vertical) {
        const double width = std::min(visible_width_, bounds.width);
        const double x = std::floor((bounds.width - width) * 0.5);
        painter.fill_rect({x, 0.0, width, bounds.height}, style.face);
        painter.draw_line({x, 0.0}, {x, bounds.height}, style.highlight, 1.0);
        painter.draw_line({x + std::max(0.0, width - 1.0), 0.0},
                          {x + std::max(0.0, width - 1.0), bounds.height},
                          style.dark_border, 1.0);
    } else {
        const double height = std::min(visible_width_, bounds.height);
        const double y = std::floor((bounds.height - height) * 0.5);
        painter.fill_rect({0.0, y, bounds.width, height}, style.face);
        painter.draw_line({0.0, y}, {bounds.width, y}, style.highlight, 1.0);
        painter.draw_line({0.0, y + std::max(0.0, height - 1.0)},
                          {bounds.width, y + std::max(0.0, height - 1.0)},
                          style.dark_border, 1.0);
    }
    if (collapse_panel_ == SplitFixedPanel::none) return;

    const bool expanded_grip = engaged();
    const bool first = collapse_panel_ == SplitFixedPanel::first;
    std::string arrow;
    const Rect tab = actuator_bounds(expanded_grip);
    Point text_origin;
    if (orientation_ == Orientation::vertical) {
        arrow = first ? (collapsed_ ? "▶" : "◀")
                      : (collapsed_ ? "◀" : "▶");
        text_origin = {tab.x + (tab.width - 7.0) * 0.5,
                       tab.y + tab.height * 0.5 + 4.0};
    } else {
        arrow = first ? (collapsed_ ? "▼" : "▲")
                      : (collapsed_ ? "▲" : "▼");
        text_origin = {tab.x + tab.width * 0.5 - 4.0,
                       tab.y + tab.height * 0.5 + 4.0};
    }
    if (expanded_grip) {
        painter.draw_box_shadow(
            tab, 0.0, engaged_shadow_offset, engaged_shadow_blur, 0.0,
            Color::rgba(style.dark_border.red, style.dark_border.green,
                        style.dark_border.blue, 102));
    }
    painter.fill_rect(tab, expanded_grip ? style.accent_light
                                         : style.face_light);
    painter.stroke_rect({tab.x + 0.5, tab.y + 0.5,
                         std::max(0.0, tab.width - 1.0),
                         std::max(0.0, tab.height - 1.0)},
                        expanded_grip ? style.accent : style.border, 1.0);
    if (expanded_grip) {
        painter.draw_line({tab.x + 1.0, tab.y + 1.0},
                          {tab.x + tab.width - 1.0, tab.y + 1.0},
                          style.highlight, 1.0);
    }
    painter.draw_text_utf8(text_origin, arrow,
                           effective_font(
                               {FontRole::control, 8.0, 700, false}),
                           style.text);
}

} // namespace gui_forms
