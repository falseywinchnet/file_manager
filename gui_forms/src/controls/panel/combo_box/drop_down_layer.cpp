#include "drop_down_layer.hpp"

#include <utility>
#include <algorithm>
#include <cmath>

namespace gui_forms {

DropDownList::DropDownList(StableId id) : ListBox(std::move(id)) {}

void DropDownList::initialize_control_tree() {
    scrollbar_ = make_control<ScrollBar>(StableId(std::string(stable_id().value()) + ".scrollbar"));
    (*scrollbar_).set_paint_plane(PaintPlane::overlay);
    (*scrollbar_).set_accessible_name("Scroll choices");
    scroll_subscription_ = (*scrollbar_).value_changed().subscribe(
        *this, Delegate<double>::bind<DropDownList, &DropDownList::scroll_changed>(*this));
    add_child(scrollbar_);
}

void DropDownList::synchronize_scroll() {
    const std::size_t visible = std::max<std::size_t>(1, static_cast<std::size_t>(
        std::floor(std::max(0.0, local_bounds().height - 4) / (item_height() * effective_text_scale()))));
    const std::size_t maximum = items().size() > visible ? items().size() - visible : 0;
    synchronizing_ = true;
    (*scrollbar_).set_visible(maximum != 0);
    (*scrollbar_).set_range(0, static_cast<double>(std::max<std::size_t>(1, maximum)));
    (*scrollbar_).set_small_change(1);
    (*scrollbar_).set_large_change(static_cast<double>(visible));
    const std::size_t top = std::min(top_index(), maximum);
    set_top_index(top);
    (*scrollbar_).set_value(static_cast<double>(top));
    synchronizing_ = false;
}

void DropDownList::arrange(Rect bounds) {
    ListBox::arrange(bounds);
    synchronize_scroll();
    set_child_layout(scrollbar_, {std::max(2.0, bounds.width - 20), 2, 18, std::max(0.0, bounds.height - 4)});
}

void DropDownList::scroll_changed(double value) {
    if (!synchronizing_) set_top_index(static_cast<std::size_t>(std::round(value)));
}

void DropDownList::on_pointer(PointerEvent& event) {
    if ((*scrollbar_).visible() && (*scrollbar_).absolute_bounds().contains(event.position) &&
        event.action != PointerAction::wheel) return;
    ListBox::on_pointer(event);
    synchronize_scroll();
}

void DropDownList::on_key(KeyEvent& event) {
    ListBox::on_key(event);
    synchronize_scroll();
}

bool DropDownList::on_semantic_child_action(std::string_view id, SemanticAction action,
                                           std::string_view value) {
    const bool result = ListBox::on_semantic_child_action(id, action, value);
    synchronize_scroll();
    return result;
}

DropDownLayer::DropDownLayer(StableId stable_id) : Panel(std::move(stable_id)) {
    set_paint_plane(PaintPlane::overlay);
    set_border_style(BorderStyle::none);
    set_background(Color::rgba(0, 0, 0, 0));
}

void DropDownLayer::on_pointer(PointerEvent& event) {
    if (event.action == PointerAction::down &&
        event.button == PointerButton::primary) {
        dismissed_.emit();
        event.handled = true;
    }
}

void DropDownLayer::on_key_preview(KeyEvent& event) {
    if (event.action == KeyAction::down &&
        event.physical_key == PhysicalKey::escape) {
        dismissed_.emit();
        event.handled = true;
    }
}

} // namespace gui_forms
