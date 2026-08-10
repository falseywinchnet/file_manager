#include "gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp"
#include "splitter_grip.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>

namespace gui_forms {

namespace {

void require_finite_nonnegative(double value, const char* message) {
    if (!std::isfinite(value) || value < 0.0) throw std::invalid_argument(message);
}

} // namespace

SplitContainer::SplitContainer(StableId stable_id)
    : ContainerControl(std::move(stable_id)) {
    const std::string prefix(this->stable_id().value());
    first_panel_ = make_control<SplitterPanel>(StableId(prefix + ".panel1"));
    second_panel_ = make_control<SplitterPanel>(StableId(prefix + ".panel2"));
    splitter_ = make_control<SplitterGrip>(StableId(prefix + ".splitter"));
    update_splitter_cursor();
}

void SplitContainer::initialize_control_tree() {
    require_mutable();
    if (tree_initialized_) return;
    add_child(first_panel_);
    add_child(second_panel_);
    add_child(splitter_);
    tree_initialized_ = true;
}

void SplitContainer::set_orientation(Orientation orientation) {
    require_mutable();
    if (orientation_ == orientation) return;
    orientation_ = orientation;
    previous_axis_extent_ = 0.0;
    previous_second_extent_ = 0.0;
    update_splitter_cursor();
    invalidate(invalidation::bounds);
}

void SplitContainer::set_splitter_distance(double distance) {
    require_finite_nonnegative(distance,
                               "splitter distance must be finite and nonnegative");
    set_distance(distance, SplitChangeReason::programmatic);
}

void SplitContainer::set_splitter_width(double width) {
    require_mutable();
    if (!std::isfinite(width) || width <= 0.0) {
        throw std::invalid_argument("splitter width must be finite and positive");
    }
    if (splitter_width_ == width) return;
    splitter_width_ = width;
    if (splitter_hit_width_ < width) splitter_hit_width_ = width;
    static_cast<SplitterGrip&>(*splitter_).set_visible_width(width);
    invalidate(invalidation::bounds);
}

void SplitContainer::set_splitter_hit_width(double width) {
    require_mutable();
    if (!std::isfinite(width) || width <= 0.0) {
        throw std::invalid_argument(
            "splitter hit width must be finite and positive");
    }
    width = std::max(width, splitter_width_);
    if (splitter_hit_width_ == width) return;
    splitter_hit_width_ = width;
    invalidate(Dirty::arrange | Dirty::hit_test | Dirty::semantics |
               Dirty::accessibility);
}

void SplitContainer::set_first_minimum(double extent) {
    require_mutable();
    require_finite_nonnegative(extent,
                               "first panel minimum must be finite and nonnegative");
    if (first_maximum_ && extent > *first_maximum_) {
        throw std::invalid_argument(
            "first panel minimum may not exceed its maximum");
    }
    if (first_minimum_ == extent) return;
    first_minimum_ = extent;
    invalidate(invalidation::bounds);
}

void SplitContainer::set_second_minimum(double extent) {
    require_mutable();
    require_finite_nonnegative(extent,
                               "second panel minimum must be finite and nonnegative");
    if (second_maximum_ && extent > *second_maximum_) {
        throw std::invalid_argument(
            "second panel minimum may not exceed its maximum");
    }
    if (second_minimum_ == extent) return;
    second_minimum_ = extent;
    invalidate(invalidation::bounds);
}

void SplitContainer::set_first_maximum(std::optional<double> extent) {
    require_mutable();
    if (extent) {
        require_finite_nonnegative(*extent,
                                   "first panel maximum must be finite and nonnegative");
        if (*extent < first_minimum_) {
            throw std::invalid_argument(
                "first panel maximum may not be below its minimum");
        }
    }
    if (first_maximum_ == extent) return;
    first_maximum_ = extent;
    invalidate(invalidation::bounds);
}

void SplitContainer::set_second_maximum(std::optional<double> extent) {
    require_mutable();
    if (extent) {
        require_finite_nonnegative(*extent,
                                   "second panel maximum must be finite and nonnegative");
        if (*extent < second_minimum_) {
            throw std::invalid_argument(
                "second panel maximum may not be below its minimum");
        }
    }
    if (second_maximum_ == extent) return;
    second_maximum_ = extent;
    invalidate(invalidation::bounds);
}

void SplitContainer::set_first_collapsed(bool collapsed,
                                         SplitCollapseOrigin origin) {
    require_mutable();
    if (first_collapsed_ == collapsed) return;
    if (collapsed && second_collapsed_) {
        throw std::logic_error("both split panels may not be collapsed");
    }
    const double old = effective_distance_;
    const SplitCollapseOrigin previous_origin = first_collapse_origin_;
    if (collapsed) {
        if (effective_distance_ > 0.0) remembered_distance_ = effective_distance_;
        transfer_focus_from(first_panel_);
    } else if (remembered_distance_ >= 0.0) {
        requested_distance_ = remembered_distance_;
    }
    first_collapsed_ = collapsed;
    first_collapse_origin_ = collapsed ? origin : SplitCollapseOrigin::none;
    if (!collapsed && previous_origin == SplitCollapseOrigin::automatic_accommodation &&
        origin == SplitCollapseOrigin::user &&
        axis_extent(committed_arranged_bounds()) <
            automatic_collapse_threshold_) {
        automatic_collapse_suppressed_ = true;
    }
    first_panel_->set_visible(!collapsed);
    static_cast<SplitterGrip&>(*splitter_).set_collapse_appearance(
        collapse_panel_, collapse_target_is_collapsed());
    const double total = axis_extent(committed_arranged_bounds());
    effective_distance_ = collapsed ? 0.0
                                    : constrained_distance(requested_distance_, total);
    invalidate(invalidation::bounds);
    const SplitChangeEvent change{old, effective_distance_,
                                  SplitChangeReason::collapse, origin};
    publish_change(splitter_changed_, change);
}

void SplitContainer::set_second_collapsed(bool collapsed,
                                          SplitCollapseOrigin origin) {
    require_mutable();
    if (second_collapsed_ == collapsed) return;
    if (collapsed && first_collapsed_) {
        throw std::logic_error("both split panels may not be collapsed");
    }
    const double old = effective_distance_;
    const SplitCollapseOrigin previous_origin = second_collapse_origin_;
    if (collapsed) {
        if (effective_distance_ > 0.0) remembered_distance_ = effective_distance_;
        transfer_focus_from(second_panel_);
    } else if (remembered_distance_ >= 0.0) {
        requested_distance_ = remembered_distance_;
    }
    second_collapsed_ = collapsed;
    second_collapse_origin_ = collapsed ? origin : SplitCollapseOrigin::none;
    if (!collapsed && previous_origin == SplitCollapseOrigin::automatic_accommodation &&
        origin == SplitCollapseOrigin::user &&
        axis_extent(committed_arranged_bounds()) <
            automatic_collapse_threshold_) {
        automatic_collapse_suppressed_ = true;
    }
    second_panel_->set_visible(!collapsed);
    static_cast<SplitterGrip&>(*splitter_).set_collapse_appearance(
        collapse_panel_, collapse_target_is_collapsed());
    const double total = axis_extent(committed_arranged_bounds());
    effective_distance_ = collapsed
        ? std::max(0.0, total - splitter_width_)
        : constrained_distance(requested_distance_, total);
    invalidate(invalidation::bounds);
    const SplitChangeEvent change{old, effective_distance_,
                                  SplitChangeReason::collapse, origin};
    publish_change(splitter_changed_, change);
}

void SplitContainer::set_splitter_fixed(bool fixed) {
    require_mutable();
    if (splitter_fixed_ == fixed) return;
    splitter_fixed_ = fixed;
    invalidate(Dirty::semantics | Dirty::accessibility);
}

void SplitContainer::set_fixed_panel(SplitFixedPanel panel) {
    require_mutable();
    if (fixed_panel_ == panel) return;
    fixed_panel_ = panel;
    invalidate(Dirty::semantics | Dirty::accessibility);
}

void SplitContainer::set_collapse_panel(SplitFixedPanel panel) {
    require_mutable();
    if (collapse_panel_ == panel) return;
    collapse_panel_ = panel;
    automatic_collapse_suppressed_ = false;
    static_cast<SplitterGrip&>(*splitter_).set_collapse_appearance(
        collapse_panel_, collapse_target_is_collapsed());
    invalidate(Dirty::paint | Dirty::hit_test | Dirty::semantics |
               Dirty::accessibility);
}

void SplitContainer::set_automatic_collapse_threshold(double extent) {
    require_mutable();
    require_finite_nonnegative(
        extent, "automatic collapse threshold must be finite and nonnegative");
    if (automatic_collapse_threshold_ == extent) return;
    automatic_collapse_threshold_ = extent;
    automatic_collapse_suppressed_ = false;
    if (extent == 0.0 && collapse_target_origin() ==
                             SplitCollapseOrigin::automatic_accommodation) {
        if (collapse_panel_ == SplitFixedPanel::first) {
            set_first_collapsed(false,
                                SplitCollapseOrigin::automatic_accommodation);
        } else if (collapse_panel_ == SplitFixedPanel::second) {
            set_second_collapsed(false,
                                 SplitCollapseOrigin::automatic_accommodation);
        }
    }
    invalidate(invalidation::bounds);
}

void SplitContainer::set_keyboard_increment(double increment) {
    require_mutable();
    if (!std::isfinite(increment) || increment <= 0.0) {
        throw std::invalid_argument(
            "splitter keyboard increment must be finite and positive");
    }
    keyboard_increment_ = increment;
}

Size SplitContainer::measure(Size available) {
    const Rect requested = requested_bounds();
    return {std::min(available.width,
                     requested.width > 0.0 ? requested.width : available.width),
            std::min(available.height,
                     requested.height > 0.0 ? requested.height : available.height)};
}

void SplitContainer::arrange(Rect final_bounds) {
    arrange_self(final_bounds);
    const double total = axis_extent(final_bounds);
    reconcile_automatic_collapse(total);
    const double available = std::max(0.0, total - splitter_width_);
    double desired = requested_distance_ < 0.0 ? available * 0.5
                                               : requested_distance_;
    if (previous_axis_extent_ > 0.0 && total != previous_axis_extent_ &&
        fixed_panel_ == SplitFixedPanel::second && !second_collapsed_) {
        desired = std::max(0.0, available - previous_second_extent_);
    }
    const double old = effective_distance_;
    effective_distance_ = constrained_distance(desired, total);
    if (requested_distance_ >= 0.0 || fixed_panel_ == SplitFixedPanel::second) {
        requested_distance_ = effective_distance_;
    }
    const double second_extent = std::max(0.0, available - effective_distance_);
    const double hit_width = std::min(total, std::max(splitter_width_,
                                                      splitter_hit_width_));
    const double hit_origin = std::clamp(
        effective_distance_ + (splitter_width_ - hit_width) * 0.5,
        0.0, std::max(0.0, total - hit_width));
    if (orientation_ == Orientation::vertical) {
        set_child_layout(first_panel_,
            {0.0, 0.0, effective_distance_, final_bounds.height});
        set_child_layout(second_panel_,
            {effective_distance_ + splitter_width_, 0.0, second_extent,
             final_bounds.height});
        set_child_layout(splitter_,
            {hit_origin, 0.0, hit_width, final_bounds.height});
    } else {
        set_child_layout(first_panel_,
            {0.0, 0.0, final_bounds.width, effective_distance_});
        set_child_layout(second_panel_,
            {0.0, effective_distance_ + splitter_width_, final_bounds.width,
             second_extent});
        set_child_layout(splitter_,
            {0.0, hit_origin, final_bounds.width, hit_width});
    }
    previous_axis_extent_ = total;
    previous_second_extent_ = second_extent;
    if (old != effective_distance_) {
        const SplitChangeEvent change{old, effective_distance_,
                                      SplitChangeReason::container_resize};
        publish_change(splitter_changed_, change);
    }
}

void SplitContainer::on_pointer_preview(PointerEvent& event) {
    const bool on_collapse_tab = collapse_panel_ != SplitFixedPanel::none &&
                                 collapse_tab_bounds().contains(event.position);
    if (event.action == PointerAction::down &&
        event.button == PointerButton::primary && on_collapse_tab) {
        collapse_tab_tracking_ = true;
        pointer_tracking_ = false;
        if (window() != nullptr) window()->request_focus(splitter_);
        splitter_->set_pointer_capture(true);
        event.handled = true;
        return;
    }
    if (collapse_tab_tracking_) {
        if (event.action == PointerAction::up) {
            collapse_tab_tracking_ = false;
            splitter_->set_pointer_capture(false);
            if (on_collapse_tab) {
                toggle_collapse_target(SplitCollapseOrigin::user);
            }
        }
        event.handled = true;
        return;
    }
    if (splitter_fixed_ || first_collapsed_ || second_collapsed_) return;
    if (event.action == PointerAction::down &&
        event.button == PointerButton::primary &&
        splitter_->absolute_bounds().contains(event.position)) {
        pointer_tracking_ = true;
        pointer_offset_ = pointer_axis(event.position) - effective_distance_;
        if (window() != nullptr) window()->request_focus(splitter_);
        splitter_->set_pointer_capture(true);
        event.handled = true;
    } else if (event.action == PointerAction::move && pointer_tracking_) {
        set_distance(pointer_axis(event.position) - pointer_offset_,
                     SplitChangeReason::pointer);
        event.handled = true;
    } else if (event.action == PointerAction::up && pointer_tracking_) {
        pointer_tracking_ = false;
        set_distance(pointer_axis(event.position) - pointer_offset_,
                     SplitChangeReason::pointer);
        event.handled = true;
    }
}

void SplitContainer::on_key_preview(KeyEvent& event) {
    if (event.action != KeyAction::down || window() == nullptr ||
        window()->focused_control() != splitter_) return;
    if (collapse_panel_ != SplitFixedPanel::none &&
        (event.physical_key == PhysicalKey::enter ||
         event.physical_key == PhysicalKey::space)) {
        toggle_collapse_target(SplitCollapseOrigin::user);
        event.handled = true;
        return;
    }
    if (splitter_fixed_ || first_collapsed_ || second_collapsed_) return;
    double delta{};
    if (orientation_ == Orientation::vertical) {
        if (event.physical_key == PhysicalKey::left) delta = -keyboard_increment_;
        else if (event.physical_key == PhysicalKey::right) delta = keyboard_increment_;
        else return;
    } else {
        if (event.physical_key == PhysicalKey::up) delta = -keyboard_increment_;
        else if (event.physical_key == PhysicalKey::down) delta = keyboard_increment_;
        else return;
    }
    if ((static_cast<std::uint8_t>(event.modifiers) &
         static_cast<std::uint8_t>(Modifier::shift)) != 0U) {
        delta *= 10.0;
    }
    set_distance(effective_distance_ + delta, SplitChangeReason::keyboard);
    event.handled = true;
}

SemanticDescriptor SplitContainer::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::split_pane;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    descriptor.value = std::to_string(effective_distance_);
    if (collapse_panel_ != SplitFixedPanel::none) {
        const bool collapsed = collapse_target_is_collapsed();
        descriptor.description = collapsed
            ? "Split pane collapsed; activate to restore its remembered extent"
            : "Split pane expanded; activate to collapse it";
        descriptor.actions = {SemanticAction::focus,
            collapsed ? SemanticAction::expand : SemanticAction::collapse};
        if (!collapsed) descriptor.states |= SemanticState::expanded;
    }
    descriptor.exposed = true;
    return descriptor;
}

bool SplitContainer::on_semantic_action(SemanticAction action,
                                        std::string_view value) {
    if (action == SemanticAction::expand &&
        collapse_panel_ != SplitFixedPanel::none &&
        collapse_target_is_collapsed()) {
        toggle_collapse_target(SplitCollapseOrigin::user);
        return true;
    }
    if (action == SemanticAction::collapse &&
        collapse_panel_ != SplitFixedPanel::none &&
        !collapse_target_is_collapsed()) {
        toggle_collapse_target(SplitCollapseOrigin::user);
        return true;
    }
    return ContainerControl::on_semantic_action(action, value);
}

double SplitContainer::axis_extent(Rect bounds) const noexcept {
    return orientation_ == Orientation::vertical ? bounds.width : bounds.height;
}

double SplitContainer::pointer_axis(Point point) const noexcept {
    const Rect absolute = absolute_bounds();
    return orientation_ == Orientation::vertical ? point.x - absolute.x
                                                  : point.y - absolute.y;
}

double SplitContainer::constrained_distance(double requested,
                                            double total_extent) const noexcept {
    const double available = std::max(0.0, total_extent - splitter_width_);
    if (first_collapsed_) return 0.0;
    if (second_collapsed_) return available;
    if (!std::isfinite(requested) || requested < 0.0) requested = available * 0.5;
    double lower = std::min(first_minimum_, available);
    double upper = std::max(0.0, available - second_minimum_);
    if (lower > upper) {
        const double requested_minimum = first_minimum_ + second_minimum_;
        const double compromise = requested_minimum > 0.0
            ? available * first_minimum_ / requested_minimum
            : available * 0.5;
        lower = upper = compromise;
    }
    if (first_maximum_) upper = std::min(upper, *first_maximum_);
    if (second_maximum_) {
        lower = std::max(lower, std::max(0.0, available - *second_maximum_));
    }
    if (lower > upper) {
        double compromise = available * 0.5;
        if (first_maximum_ && second_maximum_ &&
            *first_maximum_ + *second_maximum_ > 0.0) {
            compromise = available * *first_maximum_ /
                         (*first_maximum_ + *second_maximum_);
        }
        lower = upper = std::clamp(compromise, 0.0, available);
    }
    return std::clamp(requested, lower, upper);
}

Rect SplitContainer::collapse_tab_bounds() const noexcept {
    const Rect bounds = splitter_->absolute_bounds();
    if (orientation_ == Orientation::vertical) {
        const double height = std::min(34.0, bounds.height);
        return {bounds.x, bounds.y + std::max(0.0, (bounds.height - height) * 0.5),
                bounds.width, height};
    }
    const double width = std::min(34.0, bounds.width);
    return {bounds.x + std::max(0.0, (bounds.width - width) * 0.5), bounds.y,
            width, bounds.height};
}

bool SplitContainer::collapse_target_is_collapsed() const noexcept {
    if (collapse_panel_ == SplitFixedPanel::first) return first_collapsed_;
    if (collapse_panel_ == SplitFixedPanel::second) return second_collapsed_;
    return false;
}

SplitCollapseOrigin SplitContainer::collapse_target_origin() const noexcept {
    if (collapse_panel_ == SplitFixedPanel::first) return first_collapse_origin_;
    if (collapse_panel_ == SplitFixedPanel::second) return second_collapse_origin_;
    return SplitCollapseOrigin::none;
}

void SplitContainer::toggle_collapse_target(SplitCollapseOrigin origin) {
    if (collapse_panel_ == SplitFixedPanel::first) {
        set_first_collapsed(!first_collapsed_, origin);
    } else if (collapse_panel_ == SplitFixedPanel::second) {
        set_second_collapsed(!second_collapsed_, origin);
    }
}

void SplitContainer::reconcile_automatic_collapse(double total_extent) {
    if (collapse_panel_ == SplitFixedPanel::none ||
        automatic_collapse_threshold_ <= 0.0) return;
    const bool constrained = total_extent < automatic_collapse_threshold_;
    if (!constrained) {
        automatic_collapse_suppressed_ = false;
        if (collapse_target_origin() ==
            SplitCollapseOrigin::automatic_accommodation) {
            if (collapse_panel_ == SplitFixedPanel::first) {
                set_first_collapsed(
                    false, SplitCollapseOrigin::automatic_accommodation);
            } else {
                set_second_collapsed(
                    false, SplitCollapseOrigin::automatic_accommodation);
            }
        }
        return;
    }
    if (!automatic_collapse_suppressed_ && !collapse_target_is_collapsed()) {
        if (collapse_panel_ == SplitFixedPanel::first) {
            set_first_collapsed(true,
                SplitCollapseOrigin::automatic_accommodation);
        } else {
            set_second_collapsed(true,
                SplitCollapseOrigin::automatic_accommodation);
        }
    }
}

void SplitContainer::set_distance(double distance, SplitChangeReason reason) {
    require_mutable();
    require_finite_nonnegative(distance,
                               "splitter distance must be finite and nonnegative");
    const double old = effective_distance_;
    requested_distance_ = distance;
    const double total = axis_extent(committed_arranged_bounds());
    effective_distance_ = total > 0.0 ? constrained_distance(distance, total)
                                      : distance;
    if (old == effective_distance_) return;
    invalidate(invalidation::bounds);
    const SplitChangeEvent change{old, effective_distance_, reason};
    publish_change(splitter_changed_, change);
}

void SplitContainer::transfer_focus_from(
    const std::shared_ptr<SplitterPanel>& panel) {
    if (window() == nullptr) return;
    const Control::Ptr focused = window()->focused_control();
    if (focused == panel || panel->contains_descendant(focused)) {
        window()->request_focus(splitter_);
    }
}

void SplitContainer::update_splitter_cursor() {
    auto& splitter = static_cast<SplitterGrip&>(*splitter_);
    splitter.set_orientation(orientation_);
    splitter.set_visible_width(splitter_width_);
}

} // namespace gui_forms
