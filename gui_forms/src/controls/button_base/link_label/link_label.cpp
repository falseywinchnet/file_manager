#include "gui_forms/controls/button_base/link_label/link_label.hpp"
#include "../../basic/basic_control_rendering.hpp"
#include "gui_forms/text.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

namespace gui_forms {

LinkLabel::LinkLabel(StableId stable_id, std::string text)
    : ButtonBase(std::move(stable_id), std::move(text)) {}

void LinkLabel::set_visited(bool visited_value) {
    require_mutable();
    if (visited_ == visited_value) {
        return;
    }
    visited_ = visited_value;
    invalidate(Dirty::paint | Dirty::semantics);
}

void LinkLabel::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    const BasicControlStyle& colors = style();
    const Color foreground = enabled()
        ? (visited_ ? colors.visited_link : colors.link) : colors.disabled_text;
    const double baseline = std::max(font().size,
        (bounds.height + font().size) * 0.5 - 1.0);
    const double width = std::min(bounds.width - 4.0,
                                  estimated_text_width(display_text(), font()));
    painter.draw_text_utf8({2.0, baseline}, display_text(), font(), foreground);
    painter.draw_line({2.0, baseline + 2.0}, {2.0 + std::max(0.0, width), baseline + 2.0},
                      foreground, 1.0);
    if (focus_cue_visible()) {
        paint_focus(painter, bounds, foreground);
    }
}

void LinkLabel::on_activate() {
    set_visited(true);
    if (is_alive()) {
        ButtonBase::on_activate();
    }
}

SemanticDescriptor LinkLabel::semantic_descriptor() const {
    SemanticDescriptor descriptor = ButtonBase::semantic_descriptor();
    descriptor.role = SemanticRole::link;
    descriptor.value = visited_ ? "visited" : "unvisited";
    return descriptor;
}

} // namespace gui_forms
