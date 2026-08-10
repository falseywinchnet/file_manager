#include "gui_forms/controls/scrollable_control/container_control/flow_layout_panel/flow_layout_panel.hpp"
#include "../container_layout_utilities.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>

namespace gui_forms {

using namespace container_layout_detail;

FlowLayoutPanel::FlowLayoutPanel(StableId stable_id)
    : ContainerControl(std::move(stable_id)) {}

void FlowLayoutPanel::set_flow_direction(FlowDirection direction) {
    require_mutable();
    if (flow_direction_ == direction) return;
    flow_direction_ = direction;
    invalidate(invalidation::bounds);
}

void FlowLayoutPanel::set_wrap_contents(bool wrap) {
    require_mutable();
    if (wrap_contents_ == wrap) return;
    wrap_contents_ = wrap;
    invalidate(invalidation::bounds);
}

void FlowLayoutPanel::set_item_spacing(Size spacing) {
    require_mutable();
    if (!std::isfinite(spacing.width) || !std::isfinite(spacing.height) ||
        spacing.width < 0.0 || spacing.height < 0.0 ||
        spacing.width > 256.0 || spacing.height > 256.0) {
        throw std::invalid_argument(
            "FlowLayoutPanel item spacing must be finite and between zero and 256");
    }
    if (item_spacing_ == spacing) return;
    item_spacing_ = spacing;
    invalidate(invalidation::bounds);
}

void FlowLayoutPanel::set_auto_size(bool auto_size) {
    Control::set_auto_size(auto_size);
}

void FlowLayoutPanel::set_flow_break(const Control& child, bool flow_break) {
    require_mutable();
    if (child.parent().get() != this || !child.is_alive()) {
        throw std::invalid_argument(
            "FlowLayoutPanel flow break requires a live direct child");
    }
    const std::uint64_t id = child.runtime_id().value;
    const bool previous = flow_breaks_.contains(id) && flow_breaks_.at(id);
    if (previous == flow_break) return;
    if (flow_break) flow_breaks_[id] = true;
    else flow_breaks_.erase(id);
    invalidate(invalidation::bounds);
}

bool FlowLayoutPanel::flow_break(const Control& child) const {
    if (child.parent().get() != this || !child.is_alive()) return false;
    const FlowBreakMap::const_iterator found =
        flow_breaks_.find(child.runtime_id().value);
    return found != flow_breaks_.end() && (*found).second;
}

void FlowLayoutPanel::reconcile_flow_breaks() {
    std::unordered_set<std::uint64_t> live;
    for (const Control::Ptr& child : children()) {
        if (child && (*child).is_alive() && (*child).parent().get() == this) {
            live.insert((*child).runtime_id().value);
        }
    }
    FlowBreakMap::iterator entry = flow_breaks_.begin();
    while (entry != flow_breaks_.end()) {
        if (!live.contains((*entry).first)) {
            entry = flow_breaks_.erase(entry);
        } else {
            ++entry;
        }
    }
}

Size FlowLayoutPanel::layout_children(Size available, bool assign) {
    reconcile_flow_breaks();
    const Insets inset = padding();
    const Size inner{std::max(0.0, available.width - horizontal_extent(inset)),
                     std::max(0.0, available.height - vertical_extent(inset))};
    const bool horizontal = flow_direction_ == FlowDirection::left_to_right ||
                            flow_direction_ == FlowDirection::right_to_left;
    const double main_limit = horizontal ? inner.width : inner.height;
    const double main_spacing = horizontal ? item_spacing_.width
                                           : item_spacing_.height;
    const double cross_spacing = horizontal ? item_spacing_.height
                                            : item_spacing_.width;

    struct Item final {
        Control::Ptr control;
        Size desired;
        Insets margin;
        bool break_after{};
    };
    struct Line final {
        std::vector<Item> items;
        double main{};
        double cross{};
    };

    std::vector<Line> lines;
    Line line;
    const std::vector<Control::Ptr> retained = snapshot_layout_children();
    for (const Control::Ptr& child : retained) {
        if (!is_current_layout_child(child) || !(*child).visible()) continue;
        const Size desired = preferred_child_size(child, inner);
        if (!is_alive()) return {};
        if (!is_current_layout_child(child) || !(*child).visible()) continue;
        Item item{child, desired, (*child).margin(), flow_break(*child)};
        const double item_main = horizontal
            ? horizontal_extent(item.margin) + item.desired.width
            : vertical_extent(item.margin) + item.desired.height;
        const double item_cross = horizontal
            ? vertical_extent(item.margin) + item.desired.height
            : horizontal_extent(item.margin) + item.desired.width;
        const double preceding_spacing = line.items.empty() ? 0.0 : main_spacing;
        if (!line.items.empty() && wrap_contents_ &&
            line.main + preceding_spacing + item_main > main_limit) {
            lines.push_back(std::move(line));
            line = {};
        }
        if (!line.items.empty()) line.main += main_spacing;
        line.main += item_main;
        line.cross = std::max(line.cross, item_cross);
        line.items.push_back(std::move(item));
        if (!line.items.empty() && line.items.back().break_after) {
            lines.push_back(std::move(line));
            line = {};
        }
    }
    if (!line.items.empty()) {
        lines.push_back(std::move(line));
        line = {};
    }

    double cross_origin = 0.0;
    double content_main = 0.0;
    for (std::size_t line_index = 0U; line_index < lines.size(); ++line_index) {
        const Line& current = lines[line_index];
        double main_origin = 0.0;
        for (std::size_t item_index = 0U; item_index < current.items.size();
             ++item_index) {
            const Item& item = current.items[item_index];
            if (item_index != 0U) main_origin += main_spacing;
            Rect slot;
            if (horizontal) {
                slot.width = item.desired.width;
                slot.height = item.desired.height;
                slot.y = inset.top + cross_origin + item.margin.top;
                if (flow_direction_ == FlowDirection::left_to_right) {
                    slot.x = inset.left + main_origin + item.margin.left;
                } else {
                    slot.x = inset.left + inner.width - main_origin -
                             item.margin.right - item.desired.width;
                }
                main_origin += horizontal_extent(item.margin) + item.desired.width;
            } else {
                slot.width = item.desired.width;
                slot.height = item.desired.height;
                slot.x = inset.left + cross_origin + item.margin.left;
                if (flow_direction_ == FlowDirection::top_down) {
                    slot.y = inset.top + main_origin + item.margin.top;
                } else {
                    slot.y = inset.top + inner.height - main_origin -
                             item.margin.bottom - item.desired.height;
                }
                main_origin += vertical_extent(item.margin) + item.desired.height;
            }
            if (assign && is_current_layout_child(item.control)) {
                set_child_layout(item.control, slot);
            }
        }
        content_main = std::max(content_main, current.main);
        cross_origin += current.cross;
        if (line_index + 1U < lines.size()) cross_origin += cross_spacing;
    }
    const Size content = horizontal ? Size{content_main, cross_origin}
                                    : Size{cross_origin, content_main};
    return {content.width + horizontal_extent(inset),
            content.height + vertical_extent(inset)};
}

Size FlowLayoutPanel::measure(Size available) {
    available = {std::max(0.0, available.width),
                 std::max(0.0, available.height)};
    if (auto_size()) {
        const Size desired = layout_children(available, false);
        return {std::min(available.width, desired.width),
                std::min(available.height, desired.height)};
    }
    const Rect requested = requested_bounds();
    return {std::min(available.width,
                     requested.width > 0.0 ? requested.width : available.width),
            std::min(available.height,
                     requested.height > 0.0 ? requested.height : available.height)};
}

void FlowLayoutPanel::arrange(Rect final_bounds) {
    arrange_self(final_bounds);
    static_cast<void>(layout_children({final_bounds.width, final_bounds.height}, true));
}

SemanticDescriptor FlowLayoutPanel::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::group;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    descriptor.exposed = !descriptor.name.empty() || !descriptor.description.empty();
    return descriptor;
}

} // namespace gui_forms
