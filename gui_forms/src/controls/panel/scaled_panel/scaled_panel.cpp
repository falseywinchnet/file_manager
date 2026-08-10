#include "gui_forms/controls/panel/scaled_panel/scaled_panel.hpp"
#include "../../scrollable_control/container_control/scaled_layout_utilities.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>

namespace gui_forms {

ScaledPanel::ScaledPanel(StableId stable_id, Size design_size)
    : Panel(std::move(stable_id)), design_size_(design_size) {
    scaled_layout_detail::validate_design_size(design_size_);
}

void ScaledPanel::set_design_size(Size size) {
    require_mutable();
    scaled_layout_detail::validate_design_size(size);
    if (design_size_ == size) return;
    design_size_ = size;
    invalidate(Dirty::layout);
}

void ScaledPanel::add_at(Control::Ptr child, Rect design_bounds) {
    require_mutable();
    if (!child) throw std::invalid_argument("scaled layout child may not be null");
    scaled_layout_detail::validate_design_bounds(design_bounds);
    const RuntimeId id = child->runtime_id();
    add_child(std::move(child));
    slots_[id.value] = design_bounds;
    invalidate(Dirty::layout);
}

void ScaledPanel::set_design_bounds(const Control& child, Rect design_bounds) {
    require_mutable();
    scaled_layout_detail::validate_design_bounds(design_bounds);
    if (child.parent().get() != this) {
        throw std::logic_error("scaled layout child must belong to the panel");
    }
    slots_[child.runtime_id().value] = design_bounds;
    invalidate(Dirty::layout);
}

std::optional<Rect> ScaledPanel::design_bounds(const Control& child) const {
    const auto found = slots_.find(child.runtime_id().value);
    return found == slots_.end() ? std::nullopt
                                : std::optional<Rect>(found->second);
}

void ScaledPanel::reconcile_slots() {
    scaled_layout_detail::reconcile_slots(*this, slots_);
}

void ScaledPanel::arrange(Rect final_bounds) {
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
