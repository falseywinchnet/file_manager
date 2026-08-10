#include "tool_tip_bubble.hpp"

#include <algorithm>

namespace gui_forms {

ToolTipBubble::ToolTipBubble(StableId stable_id, std::string text)
    : Panel(std::move(stable_id)), text_(std::move(text)) {
    set_paint_plane(PaintPlane::overlay);
    set_focusable(false);
    set_border_style(BorderStyle::none);
    set_background(Color::rgba(255, 255, 225));
}

void ToolTipBubble::initialize_control_tree() {
    if (label_) return;
    label_ = make_control<Label>(
        StableId(std::string(stable_id().value()) + ".text"), text_);
    (*label_).set_paint_plane(PaintPlane::overlay);
    (*label_).set_font({FontRole::content, 12.0, 400, false});
    (*label_).set_foreground(Color::rgba(24, 31, 38));
    (*label_).set_text_wrapping(TextWrapping::word);
    (*label_).set_vertical_alignment(VerticalAlignment::center);
    add_child(label_);
}

void ToolTipBubble::set_content_size(Size size) {
    initialize_control_tree();
    (*label_).set_requested_bounds(
        {7.0, 4.0, std::max(0.0, size.width - 17.0),
         std::max(0.0, size.height - 11.0)});
}

void ToolTipBubble::on_paint(Painter& painter, Rect) {
    const Rect local{0.0, 0.0, committed_arranged_bounds().width,
                     committed_arranged_bounds().height};
    painter.fill_rect({3.0, 3.0, std::max(0.0, local.width - 3.0),
                       std::max(0.0, local.height - 3.0)},
                      Color::rgba(31, 42, 51, 66));
    const Rect body{0.0, 0.0, std::max(0.0, local.width - 3.0),
                    std::max(0.0, local.height - 3.0)};
    painter.fill_rect(body, Color::rgba(255, 255, 225));
    painter.stroke_rect({0.5, 0.5, std::max(0.0, body.width - 1.0),
                         std::max(0.0, body.height - 1.0)},
                        Color::rgba(104, 111, 118), 1.0);
}

bool ToolTipBubble::hit_test_local(Point) const {
    return false;
}

SemanticDescriptor ToolTipBubble::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::tool_tip;
    descriptor.name = text_;
    descriptor.exposed = true;
    descriptor.include_descendants = false;
    return descriptor;
}

} // namespace gui_forms
