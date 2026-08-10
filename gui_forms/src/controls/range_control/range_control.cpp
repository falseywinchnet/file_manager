#include "gui_forms/controls/range_control/range_control.hpp"

#include "range_control_rendering.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace gui_forms {
using namespace range_control_detail;

RangeControl::RangeControl(StableId stable_id)
    : Control(std::move(stable_id)) {
    define_bindable_property({
        {"Value", BindingValueKind::number, "Behavior",
         "Current value within the retained range.", BindingValue{0.0},
         Dirty::paint | Dirty::semantics},
        [this] { return BindingValue{value_}; },
        [this](const BindingValue& value) {
            const auto converted = convert_binding_value(value, BindingValueKind::number);
            if (!converted) throw std::invalid_argument("RangeControl.Value binding requires a number");
            set_value(std::get<double>(*converted));
        },
        [this](Component& owner, std::function<void()> changed) {
            return value_changed_.subscribe(owner,
                [changed = std::move(changed)](double) { changed(); });
        }, {}, {}});
}

double RangeControl::normalized_value() const noexcept {
    return (value_ - minimum_) / (maximum_ - minimum_);
}

void RangeControl::set_range(double minimum_value, double maximum_value) {
    require_mutable();
    require_finite(minimum_value, "range minimum must be finite");
    require_finite(maximum_value, "range maximum must be finite");
    if (minimum_value >= maximum_value) {
        throw std::invalid_argument("range maximum must be greater than minimum");
    }
    if (minimum_ == minimum_value && maximum_ == maximum_value) {
        return;
    }
    const double old_value = value_;
    minimum_ = minimum_value;
    maximum_ = maximum_value;
    value_ = std::clamp(value_, minimum_, maximum_);
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(range_changed_, minimum_, maximum_);
    if (is_alive() && old_value != value_) {
        publish_change(value_changed_, value_);
    }
}

void RangeControl::set_minimum(double minimum_value) {
    set_range(minimum_value, maximum_);
}

void RangeControl::set_maximum(double maximum_value) {
    set_range(minimum_, maximum_value);
}

void RangeControl::set_value(double value) {
    require_mutable();
    require_finite(value, "range value must be finite");
    if (value < minimum_ || value > maximum_) {
        throw std::out_of_range("range value is outside minimum and maximum");
    }
    if (value_ == value) {
        return;
    }
    value_ = value;
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(value_changed_, value_);
}

void RangeControl::set_small_change(double change) {
    require_mutable();
    require_finite(change, "small change must be finite");
    if (change <= 0.0) {
        throw std::invalid_argument("small change must be positive");
    }
    if (small_change_ == change) {
        return;
    }
    small_change_ = change;
    invalidate(Dirty::semantics);
}

void RangeControl::set_large_change(double change) {
    require_mutable();
    require_finite(change, "large change must be finite");
    if (change <= 0.0) {
        throw std::invalid_argument("large change must be positive");
    }
    if (large_change_ == change) {
        return;
    }
    large_change_ = change;
    invalidate(Dirty::semantics);
}

void RangeControl::set_orientation(Orientation orientation) {
    require_mutable();
    if (orientation_ == orientation) {
        return;
    }
    orientation_ = orientation;
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

void RangeControl::set_style(BasicControlStyle style) {
    require_mutable();
    if (style_ == style) {
        return;
    }
    style_ = std::move(style);
    invalidate(Dirty::paint | Dirty::semantics);
}

void RangeControl::increment(double delta) {
    require_mutable();
    require_finite(delta, "range increment must be finite");
    set_value(std::clamp(value_ + delta, minimum_, maximum_));
}

Rect RangeControl::local_bounds() const noexcept {
    const Rect arranged = committed_arranged_bounds();
    return {0.0, 0.0, arranged.width, arranged.height};
}

bool RangeControl::set_value_from_input(double value, RangeAction action) {
    require_mutable();
    require_finite(value, "input range value must be finite");
    const double next = std::clamp(value, minimum_, maximum_);
    if (next == value_) {
        return false;
    }
    const RangeScrollEvent event{value_, next, action};
    value_ = next;
    invalidate(Dirty::paint | Dirty::semantics);
    scroll_.emit(event);
    if (is_alive()) {
        publish_change(value_changed_, value_);
    }
    return true;
}

} // namespace gui_forms
