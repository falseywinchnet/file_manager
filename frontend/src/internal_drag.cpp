#include "file_manager/internal_drag.hpp"

#include <cmath>

namespace file_manager {

std::optional<InternalDropIntent>
InternalDragController::observe(const DragPointerObservation& observation) {
    if (observation.phase == DragPointerPhase::cancel) {
        reset();
        return std::nullopt;
    }
    if (observation.phase == DragPointerPhase::down) {
        reset();
        if (!observation.item_id.empty()) {
            source_id_ = observation.item_id;
            origin_x_ = observation.x;
            origin_y_ = observation.y;
        }
        return std::nullopt;
    }
    if (source_id_.empty()) return std::nullopt;

    const auto distance = std::hypot(observation.x - origin_x_,
                                     observation.y - origin_y_);
    if (!active_ && distance >= activation_distance) active_ = true;
    if (observation.phase != DragPointerPhase::up) return std::nullopt;

    const auto source = source_id_;
    const bool activated = active_;
    reset();
    if (!activated || observation.item_id.empty() ||
        observation.item_id == source) {
        return std::nullopt;
    }
    return InternalDropIntent{source, std::string(observation.item_id),
                              observation.copy_modifier};
}

void InternalDragController::reset() noexcept {
    source_id_.clear();
    origin_x_ = 0.0;
    origin_y_ = 0.0;
    active_ = false;
}

bool InternalDragController::active() const noexcept {
    return active_;
}

std::string_view InternalDragController::source_id() const noexcept {
    return source_id_;
}

} // namespace file_manager
