#include "gui_forms/controls/panel/group_box/scaled_group_box/scaled_group_box.hpp"
#include "../../../scrollable_control/container_control/scaled_layout_utilities.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>

namespace gui_forms {

ScaledGroupBox::ScaledGroupBox(StableId stable_id, std::string text,
                               Size design_size)
    : GroupBox(std::move(stable_id), std::move(text)),
      design_size_(design_size) {
    scaled_layout_detail::validate_design_size(design_size_);
}

void ScaledGroupBox::set_design_size(Size size) {
    require_mutable();
    scaled_layout_detail::validate_design_size(size);
    if (design_size_ == size) return;
    design_size_ = size;
    invalidate(Dirty::layout);
}

void ScaledGroupBox::add_at(Control::Ptr child, Rect design_bounds) {
    require_mutable();
    if (!child) throw std::invalid_argument("scaled layout child may not be null");
    scaled_layout_detail::validate_design_bounds(design_bounds);
    const RuntimeId id = child->runtime_id();
    add_child(std::move(child));
    slots_[id.value] = design_bounds;
    invalidate(Dirty::layout);
}

void ScaledGroupBox::set_design_bounds(const Control& child, Rect design_bounds) {
    require_mutable();
    scaled_layout_detail::validate_design_bounds(design_bounds);
    if (child.parent().get() != this) {
        throw std::logic_error("scaled layout child must belong to the group");
    }
    slots_[child.runtime_id().value] = design_bounds;
    invalidate(Dirty::layout);
}

std::optional<Rect> ScaledGroupBox::design_bounds(const Control& child) const {
    const auto found = slots_.find(child.runtime_id().value);
    return found == slots_.end() ? std::nullopt
                                : std::optional<Rect>(found->second);
}

void ScaledGroupBox::reconcile_slots() {
    scaled_layout_detail::reconcile_slots(*this, slots_);
}

void ScaledGroupBox::arrange(Rect final_bounds) {
    reconcile_slots();
    arrange_self(final_bounds);
    for (const Control::Ptr& child : children()) {
        const auto slot = slots_.find(child->runtime_id().value);
        if (slot == slots_.end()) continue;
        set_child_layout(child,
                         scaled_layout_detail::child_bounds(design_size_, slot->second,
                                             final_bounds));
    }
}

} // namespace gui_forms
