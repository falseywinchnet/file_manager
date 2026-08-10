#pragma once

#include "gui_forms/control.hpp"

#include <cmath>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace gui_forms::scaled_layout_detail {

struct SlotForRemovedChild final {
    const std::unordered_set<std::uint64_t>* retained{};

    [[nodiscard]] bool operator()(
        const std::pair<const std::uint64_t, Rect>& entry) const {
        return !(*retained).contains(entry.first);
    }
};

inline void validate_design_size(Size size) {
    if (!std::isfinite(size.width) || !std::isfinite(size.height) ||
        size.width <= 0.0 || size.height <= 0.0) {
        throw std::invalid_argument(
            "scaled layout design size must be finite and positive");
    }
}

inline void validate_design_bounds(Rect bounds) {
    if (!std::isfinite(bounds.x) || !std::isfinite(bounds.y) ||
        !std::isfinite(bounds.width) || !std::isfinite(bounds.height) ||
        bounds.width < 0.0 || bounds.height < 0.0) {
        throw std::invalid_argument(
            "scaled layout bounds must be finite and nonnegative");
    }
}

template <typename Parent>
void reconcile_slots(Parent& parent,
                     std::unordered_map<std::uint64_t, Rect>& slots) {
    std::unordered_set<std::uint64_t> retained;
    retained.reserve(parent.children().size());
    for (const Control::Ptr& child : parent.children()) {
        retained.insert((*child).runtime_id().value);
    }
    std::erase_if(slots, SlotForRemovedChild{&retained});
}

[[nodiscard]] inline Rect child_bounds(Size design_size, Rect bounds,
                                       Rect final_bounds) {
    const double scale_x = final_bounds.width / design_size.width;
    const double scale_y = final_bounds.height / design_size.height;
    return {bounds.x * scale_x, bounds.y * scale_y,
            bounds.width * scale_x, bounds.height * scale_y};
}

} // namespace gui_forms::scaled_layout_detail
