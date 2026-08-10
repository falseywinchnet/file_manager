#include "splitter_grip.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

namespace gui_forms {

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

void SplitterGrip::on_focus_changed(bool focused) {
    focused_ = focused;
    invalidate(invalidation::focus);
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
    if (focused_) {
        painter.stroke_rect({1.0, 1.0, std::max(0.0, bounds.width - 2.0),
                             std::max(0.0, bounds.height - 2.0)},
                            style.accent, 1.0);
    }
    if (collapse_panel_ == SplitFixedPanel::none) return;

    const bool first = collapse_panel_ == SplitFixedPanel::first;
    std::string arrow;
    Rect tab;
    Point text_origin;
    if (orientation_ == Orientation::vertical) {
        arrow = first ? (collapsed_ ? "▶" : "◀")
                      : (collapsed_ ? "◀" : "▶");
        tab = {0.0, std::max(0.0, (bounds.height - 34.0) * 0.5),
               bounds.width, std::min(34.0, bounds.height)};
        text_origin = {std::max(0.0, (bounds.width - 7.0) * 0.5),
                       tab.y + tab.height * 0.5 + 4.0};
    } else {
        arrow = first ? (collapsed_ ? "▼" : "▲")
                      : (collapsed_ ? "▲" : "▼");
        tab = {std::max(0.0, (bounds.width - 34.0) * 0.5), 0.0,
               std::min(34.0, bounds.width), bounds.height};
        text_origin = {tab.x + tab.width * 0.5 - 4.0,
                       std::max(7.0, bounds.height * 0.5 + 4.0)};
    }
    painter.fill_rect(tab, focused_ ? style.accent_light : style.face_light);
    painter.stroke_rect({tab.x + 0.5, tab.y + 0.5,
                         std::max(0.0, tab.width - 1.0),
                         std::max(0.0, tab.height - 1.0)},
                        focused_ ? style.accent : style.border, 1.0);
    painter.draw_text_utf8(text_origin, arrow,
                           effective_font(
                               {FontRole::control, 8.0, 700, false}),
                           style.text);
}

} // namespace gui_forms
