#include "gui_forms/scrolling.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>

namespace gui_forms {
namespace {

constexpr double comparison_epsilon = 0.0001;

[[nodiscard]] std::uint64_t virtual_runtime_id(
    std::string_view stable_id) noexcept {
    std::uint64_t value = 1469598103934665603ULL;
    for (const unsigned char byte : stable_id) {
        value ^= byte;
        value *= 1099511628211ULL;
    }
    return value | (std::uint64_t{1} << 63U);
}

[[nodiscard]] bool approximately_greater(double left, double right) noexcept {
    return left > right + comparison_epsilon;
}

[[nodiscard]] bool parse_finite_number(std::string_view text,
                                       double& value) noexcept {
    try {
        std::size_t consumed{};
        const std::string copy(text);
        const double parsed = std::stod(copy, &consumed);
        if (consumed != copy.size() || !std::isfinite(parsed)) return false;
        value = parsed;
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace

void ScrollProperties::set_enabled(bool enabled) {
    if (owner_ == nullptr || owner_->auto_scroll()) return;
    if (enabled_ == enabled) return;
    enabled_ = enabled;
    owner_->axis_properties_changed(orientation_, false);
}

void ScrollProperties::set_visible(bool visible) {
    if (owner_ == nullptr || owner_->auto_scroll()) return;
    if (visible_ == visible) return;
    visible_ = visible;
    owner_->axis_properties_changed(orientation_, false);
}

void ScrollProperties::set_minimum(double minimum) {
    ScrollableControl::validate_axis_value(
        minimum, "scroll minimum must be finite and nonnegative");
    if (minimum < 0.0) {
        throw std::invalid_argument(
            "scroll minimum must be finite and nonnegative");
    }
    if (owner_ != nullptr && owner_->auto_scroll()) return;
    if (minimum_ == minimum) return;
    minimum_ = minimum;
    maximum_ = std::max(maximum_, minimum_);
    value_ = std::clamp(value_, minimum_, maximum_);
    owner_->axis_properties_changed(orientation_, true);
}

void ScrollProperties::set_maximum(double maximum) {
    ScrollableControl::validate_axis_value(
        maximum, "scroll maximum must be finite");
    if (owner_ != nullptr && owner_->auto_scroll()) return;
    if (maximum_ == maximum) return;
    maximum_ = maximum;
    if (minimum_ > maximum_) minimum_ = maximum_;
    value_ = std::clamp(value_, minimum_, maximum_);
    owner_->axis_properties_changed(orientation_, true);
}

double ScrollProperties::large_change() const noexcept {
    return std::min(large_change_, std::max(0.0, maximum_ - minimum_ + 1.0));
}

void ScrollProperties::set_large_change(double value) {
    ScrollableControl::validate_axis_value(
        value, "scroll large change must be finite and nonnegative");
    if (value < 0.0) {
        throw std::invalid_argument(
            "scroll large change must be finite and nonnegative");
    }
    if (large_change_ == value) return;
    large_change_ = value;
    large_change_authored_ = true;
    owner_->axis_properties_changed(orientation_, true);
}

double ScrollProperties::small_change() const noexcept {
    return std::min(small_change_, large_change());
}

void ScrollProperties::set_small_change(double value) {
    ScrollableControl::validate_axis_value(
        value, "scroll small change must be finite and nonnegative");
    if (value < 0.0) {
        throw std::invalid_argument(
            "scroll small change must be finite and nonnegative");
    }
    if (small_change_ == value) return;
    small_change_ = value;
    small_change_authored_ = true;
    owner_->axis_properties_changed(orientation_, false);
}

void ScrollProperties::set_value(double value) {
    ScrollableControl::validate_axis_value(value,
                                            "scroll value must be finite");
    if (value < minimum_ || value > maximum_) {
        throw std::out_of_range("scroll value is outside Minimum/Maximum");
    }
    if (owner_ == nullptr) {
        value_ = value;
        return;
    }
    static_cast<void>(owner_->apply_axis_value(
        orientation_, value, ScrollEventType::thumb_position, false));
}

ScrollAxisSnapshot ScrollProperties::snapshot() const noexcept {
    return {enabled_, visible_, minimum_, maximum_, large_change(),
            small_change(), value_};
}

double ScrollProperties::maximum_position() const noexcept {
    return std::max(minimum_, maximum_ - large_change() + 1.0);
}

void ScrollProperties::set_automatic(double viewport_extent,
                                     double content_extent,
                                     double value) noexcept {
    minimum_ = 0.0;
    maximum_ = std::max(0.0, std::ceil(content_extent) - 1.0);
    if (!large_change_authored_) large_change_ = std::max(0.0, viewport_extent);
    if (!small_change_authored_) {
        small_change_ = std::min(48.0, std::max(1.0, viewport_extent * 0.1));
    }
    visible_ = approximately_greater(content_extent, viewport_extent);
    enabled_ = true;
    value_ = std::clamp(value, minimum_, maximum_position());
}

ScrollableControl::ScrollableControl(StableId stable_id)
    : Control(std::move(stable_id)),
      horizontal_scroll_(*this, ScrollOrientation::horizontal),
      vertical_scroll_(*this, ScrollOrientation::vertical) {}

void ScrollableControl::validate_size(Size size, const char* message) {
    if (!std::isfinite(size.width) || !std::isfinite(size.height) ||
        size.width < 0.0 || size.height < 0.0) {
        throw std::invalid_argument(message);
    }
}

void ScrollableControl::validate_axis_value(double value,
                                            const char* message) {
    if (!std::isfinite(value)) throw std::invalid_argument(message);
}

void ScrollableControl::set_auto_scroll(bool enabled) {
    require_mutable();
    if (auto_scroll_ == enabled) return;
    auto_scroll_ = enabled;
    if (enabled) scroll_state_ |= scroll_state_auto_scrolling;
    else {
        scroll_state_ &= ~scroll_state_auto_scrolling;
        scroll_position_ = {};
        horizontal_scroll_.value_ = horizontal_scroll_.minimum_;
        vertical_scroll_.value_ = vertical_scroll_.minimum_;
    }
    invalidate(Dirty::arrange | Dirty::paint | Dirty::hit_test |
               Dirty::semantics | Dirty::accessibility);
}

void ScrollableControl::set_auto_scroll_margin(Size margin) {
    require_mutable();
    validate_size(margin,
                  "AutoScrollMargin must be finite and nonnegative");
    if (auto_scroll_margin_ == margin) return;
    auto_scroll_margin_ = margin;
    if (auto_scroll_) {
        invalidate(Dirty::arrange | Dirty::paint | Dirty::hit_test |
                   Dirty::semantics | Dirty::accessibility);
    }
}

void ScrollableControl::set_auto_scroll_margin(double x, double y) {
    if (!std::isfinite(x) || !std::isfinite(y)) {
        throw std::invalid_argument("AutoScrollMargin must be finite");
    }
    set_auto_scroll_margin(Size{std::max(0.0, x), std::max(0.0, y)});
}

void ScrollableControl::set_auto_scroll_min_size(Size size) {
    require_mutable();
    validate_size(size,
                  "AutoScrollMinSize must be finite and nonnegative");
    if (auto_scroll_min_size_ == size) return;
    auto_scroll_min_size_ = size;
    if (!auto_scroll_) {
        auto_scroll_ = true;
        scroll_state_ |= scroll_state_auto_scrolling;
    }
    invalidate(Dirty::arrange | Dirty::paint | Dirty::hit_test |
               Dirty::semantics | Dirty::accessibility);
}

void ScrollableControl::set_auto_scroll_position(Point position) {
    require_mutable();
    if (!std::isfinite(position.x) || !std::isfinite(position.y)) {
        throw std::invalid_argument("AutoScrollPosition must be finite");
    }
    static_cast<void>(scroll_to(
        {std::max(0.0, position.x), std::max(0.0, position.y)}));
}

Rect ScrollableControl::display_rectangle() const noexcept {
    const Rect viewport = effective_viewport_rectangle();
    return {viewport.x - scroll_position_.x,
            viewport.y - scroll_position_.y,
            std::max(viewport.width, content_extent_.width),
            std::max(viewport.height, content_extent_.height)};
}

Rect ScrollableControl::effective_viewport_rectangle() const noexcept {
    // Composite controls commonly own their child arrangement while retaining
    // Panel/ContainerControl behavior. With no scrolling requested, their
    // viewport is always the live client rectangle even when they deliberately
    // bypass ScrollableControl's child-layout pass.
    if (!auto_scroll_ && !horizontal_scroll_.visible_ &&
        !vertical_scroll_.visible_) {
        return client_rectangle();
    }
    return viewport_rectangle_;
}

void ScrollableControl::set_hscroll(bool visible) {
    require_mutable();
    if (auto_scroll_) return;
    horizontal_scroll_.set_visible(visible);
}

void ScrollableControl::set_vscroll(bool visible) {
    require_mutable();
    if (auto_scroll_) return;
    vertical_scroll_.set_visible(visible);
}

bool ScrollableControl::get_scroll_state(std::uint32_t bit) const noexcept {
    return (scroll_state_ & bit) != 0U;
}

void ScrollableControl::set_scroll_state(std::uint32_t bit, bool value) {
    require_mutable();
    constexpr std::uint32_t known = scroll_state_auto_scrolling |
        scroll_state_hscroll_visible | scroll_state_vscroll_visible |
        scroll_state_user_has_scrolled | scroll_state_full_drag;
    if (bit == 0U || (bit & ~known) != 0U) {
        throw std::invalid_argument("unknown ScrollableControl state bit");
    }
    if ((bit & scroll_state_auto_scrolling) != 0U) set_auto_scroll(value);
    if ((bit & scroll_state_hscroll_visible) != 0U) set_hscroll(value);
    if ((bit & scroll_state_vscroll_visible) != 0U) set_vscroll(value);
    const std::uint32_t passive = bit &
        (scroll_state_user_has_scrolled | scroll_state_full_drag);
    if (value) scroll_state_ |= passive;
    else scroll_state_ &= ~passive;
}

ScrollSnapshot ScrollableControl::scroll_snapshot() const noexcept {
    return {auto_scroll_, scroll_position_, auto_scroll_margin_,
            auto_scroll_min_size_, display_rectangle(),
            effective_viewport_rectangle(),
            horizontal_scroll_.snapshot(), vertical_scroll_.snapshot()};
}

Size ScrollableControl::content_extent() {
    Size extent{
        std::max(0.0, auto_scroll_min_size_.width),
        std::max(0.0, auto_scroll_min_size_.height)};
    const Insets own_padding = padding();
    const std::vector<Control::Ptr> retained = snapshot_layout_children();
    for (const Control::Ptr& child : retained) {
        if (!is_current_layout_child(child) || !child->visible() ||
            child->dock() != DockStyle::none) continue;
        Rect bounds = child->requested_bounds();
        if (child->auto_size()) {
            const Size preferred = child->get_preferred_size({0.0, 0.0});
            if (!is_alive()) return extent;
            if (!is_current_layout_child(child) || !child->visible() ||
                child->dock() != DockStyle::none) {
                continue;
            }
            bounds.width = child->auto_size_mode() == AutoSizeMode::grow_only
                ? std::max(bounds.width, preferred.width) : preferred.width;
            bounds.height = child->auto_size_mode() == AutoSizeMode::grow_only
                ? std::max(bounds.height, preferred.height) : preferred.height;
        }
        const Insets child_margin = child->margin();
        extent.width = std::max(
            extent.width,
            std::max(0.0, bounds.x) + bounds.width + child_margin.right +
                auto_scroll_margin_.width + own_padding.right);
        extent.height = std::max(
            extent.height,
            std::max(0.0, bounds.y) + bounds.height + child_margin.bottom +
                auto_scroll_margin_.height + own_padding.bottom);
    }
    return extent;
}

void ScrollableControl::recompute_scroll_layout(Size client_size) {
    const Rect previous_viewport = viewport_rectangle_;
    const Size previous_content = content_extent_;
    const bool previous_h = horizontal_scroll_.visible_;
    const bool previous_v = vertical_scroll_.visible_;
    const Point previous_position = scroll_position_;

    content_extent_ = content_extent();
    if (!is_alive()) return;
    const double width = std::max(0.0, client_size.width);
    const double height = std::max(0.0, client_size.height);
    bool h = auto_scroll_ ? false : horizontal_scroll_.visible_;
    bool v = auto_scroll_ ? false : vertical_scroll_.visible_;
    if (auto_scroll_) {
        for (int pass = 0; pass != 3; ++pass) {
            const double available_width = std::max(
                0.0, width - (v ? scrollbar_thickness_ : 0.0));
            const double available_height = std::max(
                0.0, height - (h ? scrollbar_thickness_ : 0.0));
            const bool next_h = approximately_greater(
                content_extent_.width, available_width);
            const bool next_v = approximately_greater(
                content_extent_.height, available_height);
            if (h == next_h && v == next_v) break;
            h = next_h;
            v = next_v;
        }
    }
    viewport_rectangle_ = {
        0.0, 0.0,
        std::max(0.0, width - (v ? scrollbar_thickness_ : 0.0)),
        std::max(0.0, height - (h ? scrollbar_thickness_ : 0.0))};
    content_extent_.width = std::max(content_extent_.width,
                                     viewport_rectangle_.width);
    content_extent_.height = std::max(content_extent_.height,
                                      viewport_rectangle_.height);

    if (auto_scroll_) {
        horizontal_scroll_.set_automatic(viewport_rectangle_.width,
                                         content_extent_.width,
                                         scroll_position_.x);
        vertical_scroll_.set_automatic(viewport_rectangle_.height,
                                       content_extent_.height,
                                       scroll_position_.y);
        h = horizontal_scroll_.visible_;
        v = vertical_scroll_.visible_;
    } else {
        horizontal_scroll_.value_ = std::clamp(
            horizontal_scroll_.value_, horizontal_scroll_.minimum_,
            horizontal_scroll_.maximum_position());
        vertical_scroll_.value_ = std::clamp(
            vertical_scroll_.value_, vertical_scroll_.minimum_,
            vertical_scroll_.maximum_position());
    }
    scroll_position_ = {horizontal_scroll_.visible_
                            ? horizontal_scroll_.value_ : 0.0,
                        vertical_scroll_.visible_
                            ? vertical_scroll_.value_ : 0.0};
    if (h) scroll_state_ |= scroll_state_hscroll_visible;
    else scroll_state_ &= ~scroll_state_hscroll_visible;
    if (v) scroll_state_ |= scroll_state_vscroll_visible;
    else scroll_state_ &= ~scroll_state_vscroll_visible;

    if (previous_viewport != viewport_rectangle_ ||
        previous_content != content_extent_ || previous_h != h ||
        previous_v != v || previous_position != scroll_position_) {
        invalidate(Dirty::paint | Dirty::hit_test | Dirty::semantics |
                   Dirty::accessibility);
    }
}

void ScrollableControl::adjust_scrollbars(bool display_scrollbars) {
    if (!display_scrollbars) {
        horizontal_scroll_.visible_ = false;
        vertical_scroll_.visible_ = false;
        scroll_position_ = {};
        scroll_state_ &= ~(scroll_state_hscroll_visible |
                           scroll_state_vscroll_visible);
    }
}

void ScrollableControl::arrange(Rect final_bounds) {
    if (arranging_scroll_) {
        Control::arrange(final_bounds);
        return;
    }
    arranging_scroll_ = true;
    struct Reset final {
        bool& value;
        ~Reset() { value = false; }
    } reset{arranging_scroll_};
    arrange_self(final_bounds);
    recompute_scroll_layout({final_bounds.width, final_bounds.height});
    if (!is_alive()) return;
    if (auto_scroll_) adjust_scrollbars(true);
    Control::arrange(final_bounds);
    if (scroll_position_ == Point{}) return;
    for (const Control::Ptr& child : children()) {
        if (!child || !child->is_alive() || !child->visible() ||
            child->dock() != DockStyle::none) continue;
        Rect bounds = child->committed_arranged_bounds();
        bounds.x -= scroll_position_.x;
        bounds.y -= scroll_position_.y;
        set_child_layout(child, bounds);
    }
}

double ScrollableControl::maximum_offset(
    ScrollOrientation orientation) const noexcept {
    const ScrollProperties& axis = orientation == ScrollOrientation::horizontal
        ? horizontal_scroll_ : vertical_scroll_;
    return axis.visible_ ? axis.maximum_position() : 0.0;
}

bool ScrollableControl::apply_axis_value(ScrollOrientation orientation,
                                         double value,
                                         ScrollEventType type,
                                         bool notify) {
    require_mutable();
    validate_axis_value(value, "scroll position must be finite");
    ScrollProperties& axis = orientation == ScrollOrientation::horizontal
        ? horizontal_scroll_ : vertical_scroll_;
    const double old_value = axis.value_;
    const double next = std::clamp(
        value, axis.minimum_, maximum_offset(orientation));
    if (std::abs(next - old_value) <= comparison_epsilon) return false;
    axis.value_ = next;
    if (orientation == ScrollOrientation::horizontal) scroll_position_.x = next;
    else scroll_position_.y = next;
    if (notify) scroll_state_ |= scroll_state_user_has_scrolled;
    invalidate(Dirty::arrange | Dirty::paint | Dirty::hit_test |
               Dirty::semantics | Dirty::accessibility);
    if (notify) notify_scroll(orientation, type, old_value, next);
    return true;
}

bool ScrollableControl::scroll_to(Point position, ScrollEventType type,
                                  bool notify) {
    if (!std::isfinite(position.x) || !std::isfinite(position.y)) {
        throw std::invalid_argument("scroll position must be finite");
    }
    const bool horizontal = apply_axis_value(
        ScrollOrientation::horizontal, position.x, type, notify);
    const bool vertical = apply_axis_value(
        ScrollOrientation::vertical, position.y, type, notify);
    return horizontal || vertical;
}

bool ScrollableControl::scroll_by(Point delta,
                                  ScrollEventType horizontal_type,
                                  ScrollEventType vertical_type,
                                  bool notify) {
    if (!std::isfinite(delta.x) || !std::isfinite(delta.y)) {
        throw std::invalid_argument("scroll delta must be finite");
    }
    const bool horizontal = apply_axis_value(
        ScrollOrientation::horizontal, scroll_position_.x + delta.x,
        horizontal_type, notify);
    const bool vertical = apply_axis_value(
        ScrollOrientation::vertical, scroll_position_.y + delta.y,
        vertical_type, notify);
    return horizontal || vertical;
}

void ScrollableControl::set_display_rect_location(Point location) {
    if (!std::isfinite(location.x) || !std::isfinite(location.y)) {
        throw std::invalid_argument("display rectangle location must be finite");
    }
    static_cast<void>(scroll_to({std::max(0.0, -location.x),
                                 std::max(0.0, -location.y)}));
}

Point ScrollableControl::scroll_to_control(const Control& control) const {
    Rect bounds;
    if (attached() && control.attached()) {
        bounds = rectangle_from_window(control.rectangle_to_window(
            {0.0, 0.0, control.committed_arranged_bounds().width,
             control.committed_arranged_bounds().height}));
        bounds.x += scroll_position_.x;
        bounds.y += scroll_position_.y;
    } else {
        // Detached controls still have a complete authored retained tree even
        // though no Window has committed descendant layout slots. Walk that
        // logical tree so designer/tests and pre-show initialization can reveal
        // a control without inventing zero-sized arranged geometry.
        bounds = control.requested_bounds();
        for (Control::Ptr ancestor = control.parent(); ancestor;
             ancestor = ancestor->parent()) {
            if (ancestor.get() == this) break;
            const Rect parent_bounds = ancestor->requested_bounds();
            bounds.x += parent_bounds.x;
            bounds.y += parent_bounds.y;
        }
    }
    Point next = scroll_position_;
    if (bounds.x - auto_scroll_margin_.width < next.x) {
        next.x = bounds.x - auto_scroll_margin_.width;
    } else if (bounds.right() + auto_scroll_margin_.width >
               next.x + viewport_rectangle_.width) {
        next.x = bounds.right() + auto_scroll_margin_.width -
                 viewport_rectangle_.width;
    }
    if (bounds.y - auto_scroll_margin_.height < next.y) {
        next.y = bounds.y - auto_scroll_margin_.height;
    } else if (bounds.bottom() + auto_scroll_margin_.height >
               next.y + viewport_rectangle_.height) {
        next.y = bounds.bottom() + auto_scroll_margin_.height -
                 viewport_rectangle_.height;
    }
    const Point offset = control.auto_scroll_offset();
    next.x -= offset.x;
    next.y -= offset.y;
    return {std::max(0.0, next.x), std::max(0.0, next.y)};
}

void ScrollableControl::scroll_control_into_view(
    const Control::Ptr& control) {
    require_mutable();
    if (!control || !contains(*control) || !auto_scroll_ ||
        (!hscroll() && !vscroll()) || viewport_rectangle_.empty()) return;
    static_cast<void>(scroll_to(scroll_to_control(*control)));
    scroll_state_ &= ~scroll_state_user_has_scrolled;
}

void ScrollableControl::notify_scroll(ScrollOrientation orientation,
                                      ScrollEventType type,
                                      double old_value,
                                      double new_value) {
    ScrollEvent event{type, old_value, new_value, orientation};
    last_scroll_event_ = event;
    ++scroll_event_revision_;
    scroll_event_.emit(event);
}

ScrollableControl::AxisGeometry ScrollableControl::axis_geometry(
    ScrollOrientation orientation) const noexcept {
    AxisGeometry geometry;
    if (orientation == ScrollOrientation::horizontal) {
        if (!horizontal_scroll_.visible_) return geometry;
        geometry.bar = {0.0, viewport_rectangle_.height,
                        viewport_rectangle_.width, scrollbar_thickness_};
        const double button = std::min(scrollbar_button_extent_,
                                       geometry.bar.width * 0.5);
        geometry.first_button = {geometry.bar.x, geometry.bar.y, button,
                                 geometry.bar.height};
        geometry.last_button = {geometry.bar.right() - button, geometry.bar.y,
                                button, geometry.bar.height};
        geometry.track = {geometry.first_button.right(), geometry.bar.y,
                          std::max(0.0, geometry.bar.width - button * 2.0),
                          geometry.bar.height};
    } else {
        if (!vertical_scroll_.visible_) return geometry;
        geometry.bar = {viewport_rectangle_.width, 0.0,
                        scrollbar_thickness_, viewport_rectangle_.height};
        const double button = std::min(scrollbar_button_extent_,
                                       geometry.bar.height * 0.5);
        geometry.first_button = {geometry.bar.x, geometry.bar.y,
                                 geometry.bar.width, button};
        geometry.last_button = {geometry.bar.x,
                                geometry.bar.bottom() - button,
                                geometry.bar.width, button};
        geometry.track = {geometry.bar.x, geometry.first_button.bottom(),
                          geometry.bar.width,
                          std::max(0.0, geometry.bar.height - button * 2.0)};
    }
    const ScrollProperties& axis = orientation == ScrollOrientation::horizontal
        ? horizontal_scroll_ : vertical_scroll_;
    const double track_extent = orientation == ScrollOrientation::horizontal
        ? geometry.track.width : geometry.track.height;
    const double viewport_extent = orientation == ScrollOrientation::horizontal
        ? viewport_rectangle_.width : viewport_rectangle_.height;
    const double content = orientation == ScrollOrientation::horizontal
        ? content_extent_.width : content_extent_.height;
    const double thumb_extent = std::min(
        track_extent, std::max(minimum_thumb_extent_,
            content <= 0.0 ? track_extent : track_extent * viewport_extent / content));
    const double travel = std::max(0.0, track_extent - thumb_extent);
    const double maximum = maximum_offset(orientation);
    const double offset = maximum <= 0.0 ? 0.0 : travel * axis.value_ / maximum;
    geometry.thumb = orientation == ScrollOrientation::horizontal
        ? Rect{geometry.track.x + offset, geometry.track.y,
               thumb_extent, geometry.track.height}
        : Rect{geometry.track.x, geometry.track.y + offset,
               geometry.track.width, thumb_extent};
    return geometry;
}

ScrollableControl::ScrollPart ScrollableControl::part_at(
    Point local) const noexcept {
    const AxisGeometry horizontal = axis_geometry(ScrollOrientation::horizontal);
    if (horizontal.bar.contains(local)) {
        if (horizontal.first_button.contains(local)) return ScrollPart::horizontal_first;
        if (horizontal.last_button.contains(local)) return ScrollPart::horizontal_last;
        if (horizontal.thumb.contains(local)) return ScrollPart::horizontal_thumb;
        return local.x < horizontal.thumb.x
            ? ScrollPart::horizontal_track_before
            : ScrollPart::horizontal_track_after;
    }
    const AxisGeometry vertical = axis_geometry(ScrollOrientation::vertical);
    if (vertical.bar.contains(local)) {
        if (vertical.first_button.contains(local)) return ScrollPart::vertical_first;
        if (vertical.last_button.contains(local)) return ScrollPart::vertical_last;
        if (vertical.thumb.contains(local)) return ScrollPart::vertical_thumb;
        return local.y < vertical.thumb.y
            ? ScrollPart::vertical_track_before
            : ScrollPart::vertical_track_after;
    }
    return ScrollPart::none;
}

bool ScrollableControl::handle_scroll_pointer(PointerEvent& event) {
    const Point local = point_from_window(event.position);
    if (dragging_orientation_) {
        if (event.pointer_id != 0U && dragging_pointer_id_ != 0U &&
            event.pointer_id != dragging_pointer_id_) return false;
        const AxisGeometry geometry = axis_geometry(*dragging_orientation_);
        const double track_extent = *dragging_orientation_ ==
                ScrollOrientation::horizontal
            ? geometry.track.width : geometry.track.height;
        const double thumb_extent = *dragging_orientation_ ==
                ScrollOrientation::horizontal
            ? geometry.thumb.width : geometry.thumb.height;
        const double travel = std::max(0.0, track_extent - thumb_extent);
        if (event.action == PointerAction::move && travel > 0.0) {
            const double movement = *dragging_orientation_ ==
                    ScrollOrientation::horizontal
                ? local.x - dragging_origin_.x
                : local.y - dragging_origin_.y;
            const double next = dragging_start_value_ + movement *
                maximum_offset(*dragging_orientation_) / travel;
            static_cast<void>(apply_axis_value(*dragging_orientation_, next,
                                               ScrollEventType::thumb_track,
                                               true));
            event.handled = true;
            return true;
        }
        if (event.action == PointerAction::up) {
            const ScrollOrientation orientation = *dragging_orientation_;
            const double value = orientation == ScrollOrientation::horizontal
                ? scroll_position_.x : scroll_position_.y;
            dragging_orientation_.reset();
            dragging_pointer_id_ = 0U;
            set_pointer_capture(false);
            notify_scroll(orientation, ScrollEventType::thumb_position,
                          value, value);
            event.handled = true;
            return true;
        }
        event.handled = true;
        return true;
    }
    if (event.action != PointerAction::down ||
        event.button != PointerButton::primary) return false;
    const ScrollPart part = part_at(local);
    if (part == ScrollPart::none) return false;
    const bool horizontal = part >= ScrollPart::horizontal_first &&
        part <= ScrollPart::horizontal_last;
    const ScrollOrientation orientation = horizontal
        ? ScrollOrientation::horizontal : ScrollOrientation::vertical;
    ScrollProperties& axis = horizontal ? horizontal_scroll_ : vertical_scroll_;
    if (!axis.enabled_) {
        event.handled = true;
        return true;
    }
    if (part == ScrollPart::horizontal_thumb ||
        part == ScrollPart::vertical_thumb) {
        dragging_orientation_ = orientation;
        dragging_pointer_id_ = event.pointer_id == 0U ? 1U : event.pointer_id;
        dragging_origin_ = local;
        dragging_start_value_ = axis.value_;
        set_pointer_capture(true);
    } else {
        double next = axis.value_;
        ScrollEventType type = ScrollEventType::small_increment;
        if (part == ScrollPart::horizontal_first ||
            part == ScrollPart::vertical_first) {
            next -= axis.small_change();
            type = ScrollEventType::small_decrement;
        } else if (part == ScrollPart::horizontal_last ||
                   part == ScrollPart::vertical_last) {
            next += axis.small_change();
            type = ScrollEventType::small_increment;
        } else if (part == ScrollPart::horizontal_track_before ||
                   part == ScrollPart::vertical_track_before) {
            next -= axis.large_change();
            type = ScrollEventType::large_decrement;
        } else {
            next += axis.large_change();
            type = ScrollEventType::large_increment;
        }
        static_cast<void>(apply_axis_value(orientation, next, type, true));
    }
    event.handled = true;
    return true;
}

bool ScrollableControl::handle_wheel(PointerEvent& event) {
    if (event.action != PointerAction::wheel || event.handled) return false;
    auto normalized = [](double delta) {
        if (delta == 0.0) return 0.0;
        return std::abs(delta) <= 8.0 ? delta * 48.0 : delta;
    };
    const double vertical_delta = -normalized(event.wheel_delta.y);
    const double horizontal_delta = -normalized(event.wheel_delta.x);
    bool changed{};
    if (vertical_scroll_.visible_ && vertical_scroll_.enabled_ &&
        vertical_delta != 0.0) {
        changed = apply_axis_value(
            ScrollOrientation::vertical,
            scroll_position_.y + vertical_delta,
            vertical_delta < 0.0 ? ScrollEventType::small_decrement
                                 : ScrollEventType::small_increment,
            false);
    }
    if (!changed && horizontal_scroll_.visible_ &&
        horizontal_scroll_.enabled_) {
        const double delta = horizontal_delta != 0.0
            ? horizontal_delta : vertical_delta;
        if (delta != 0.0) {
            changed = apply_axis_value(
                ScrollOrientation::horizontal,
                scroll_position_.x + delta,
                delta < 0.0 ? ScrollEventType::small_decrement
                            : ScrollEventType::small_increment,
                false);
        }
    }
    if (changed) event.handled = true;
    return changed;
}

void ScrollableControl::on_pointer(PointerEvent& event) {
    if (handle_scroll_pointer(event)) return;
    static_cast<void>(handle_wheel(event));
}

void ScrollableControl::on_pointer_bubble(PointerEvent& event) {
    static_cast<void>(handle_wheel(event));
}

void ScrollableControl::axis_properties_changed(
    ScrollOrientation orientation, bool position_changed) {
    require_mutable();
    ScrollProperties& axis = orientation == ScrollOrientation::horizontal
        ? horizontal_scroll_ : vertical_scroll_;
    if (position_changed) {
        axis.value_ = std::clamp(axis.value_, axis.minimum_,
                                 axis.maximum_position());
        if (orientation == ScrollOrientation::horizontal) {
            scroll_position_.x = axis.visible_ ? axis.value_ : 0.0;
        } else {
            scroll_position_.y = axis.visible_ ? axis.value_ : 0.0;
        }
    }
    invalidate(Dirty::arrange | Dirty::paint | Dirty::hit_test |
               Dirty::semantics | Dirty::accessibility);
}

void ScrollableControl::paint_axis(Painter& painter,
                                   ScrollOrientation orientation,
                                   const AxisGeometry& geometry) const {
    if (geometry.bar.empty()) return;
    const BasicControlStyle& style = effective_theme().basic_style();
    const ScrollProperties& axis = orientation == ScrollOrientation::horizontal
        ? horizontal_scroll_ : vertical_scroll_;
    painter.fill_rect(geometry.bar, style.face);
    painter.stroke_rect(geometry.bar, style.border, 1.0);
    painter.fill_rect(geometry.first_button, style.face_light);
    painter.fill_rect(geometry.last_button, style.face_light);
    painter.stroke_rect(geometry.first_button, style.border, 1.0);
    painter.stroke_rect(geometry.last_button, style.border, 1.0);
    painter.fill_rect(geometry.thumb,
                      axis.enabled_ ? style.accent_light : style.face);
    painter.stroke_rect(geometry.thumb,
                        axis.enabled_ ? style.dark_border : style.border, 1.0);
    const FontSpec font = effective_font(
        {FontRole::control, 8.0, 700, false});
    const Color glyph = axis.enabled_ ? style.text : style.disabled_text;
    if (orientation == ScrollOrientation::horizontal) {
        painter.draw_text_utf8(
            {geometry.first_button.x + 5.0,
             geometry.first_button.y + geometry.first_button.height * 0.5 + 3.0},
            "‹", font, glyph);
        painter.draw_text_utf8(
            {geometry.last_button.x + 5.0,
             geometry.last_button.y + geometry.last_button.height * 0.5 + 3.0},
            "›", font, glyph);
    } else {
        painter.draw_text_utf8(
            {geometry.first_button.x + 4.0,
             geometry.first_button.y + geometry.first_button.height * 0.5 + 3.0},
            "▲", font, glyph);
        painter.draw_text_utf8(
            {geometry.last_button.x + 4.0,
             geometry.last_button.y + geometry.last_button.height * 0.5 + 3.0},
            "▼", font, glyph);
    }
}

void ScrollableControl::on_paint_overlay(Painter& painter, Rect) {
    paint_axis(painter, ScrollOrientation::horizontal,
               axis_geometry(ScrollOrientation::horizontal));
    paint_axis(painter, ScrollOrientation::vertical,
               axis_geometry(ScrollOrientation::vertical));
    if (hscroll() && vscroll()) {
        const BasicControlStyle& style = effective_theme().basic_style();
        const Rect corner{viewport_rectangle_.width,
                          viewport_rectangle_.height,
                          scrollbar_thickness_, scrollbar_thickness_};
        painter.fill_rect(corner, style.face);
        painter.stroke_rect(corner, style.border, 1.0);
    }
}

std::vector<SemanticNode>
ScrollableControl::semantic_virtual_children() const {
    std::vector<SemanticNode> result = Control::semantic_virtual_children();
    const Rect absolute = absolute_bounds();
    auto append = [&](ScrollOrientation orientation,
                      const ScrollProperties& axis) {
        if (!axis.visible_) return;
        SemanticNode node;
        node.stable_id = std::string(stable_id().value()) +
            (orientation == ScrollOrientation::horizontal
                 ? ".horizontal-scroll" : ".vertical-scroll");
        node.runtime_id = virtual_runtime_id(node.stable_id);
        node.role = SemanticRole::scroll_bar;
        node.name = orientation == ScrollOrientation::horizontal
            ? "Horizontal scroll bar" : "Vertical scroll bar";
        node.numeric_value = axis.value_;
        node.minimum_value = axis.minimum_;
        node.maximum_value = axis.maximum_position();
        const Rect local = axis_geometry(orientation).bar;
        node.bounds = {absolute.x + local.x, absolute.y + local.y,
                       local.width, local.height};
        node.states = SemanticState::visible;
        if (axis.enabled_ && effectively_enabled()) {
            node.states |= SemanticState::enabled;
            node.actions = {SemanticAction::increment,
                            SemanticAction::decrement,
                            SemanticAction::set_value};
        }
        result.push_back(std::move(node));
    };
    append(ScrollOrientation::horizontal, horizontal_scroll_);
    append(ScrollOrientation::vertical, vertical_scroll_);
    return result;
}

bool ScrollableControl::on_semantic_child_action(
    std::string_view child_stable_id, SemanticAction action,
    std::string_view value) {
    const std::string prefix(stable_id().value());
    std::optional<ScrollOrientation> orientation;
    if (child_stable_id == prefix + ".horizontal-scroll") {
        orientation = ScrollOrientation::horizontal;
    } else if (child_stable_id == prefix + ".vertical-scroll") {
        orientation = ScrollOrientation::vertical;
    } else {
        return Control::on_semantic_child_action(
            child_stable_id, action, value);
    }
    ScrollProperties& axis = *orientation == ScrollOrientation::horizontal
        ? horizontal_scroll_ : vertical_scroll_;
    if (!axis.visible_ || !axis.enabled_ || !effectively_enabled()) return false;
    double next = axis.value_;
    ScrollEventType type = ScrollEventType::thumb_position;
    if (action == SemanticAction::increment) {
        next += axis.small_change();
        type = ScrollEventType::small_increment;
    } else if (action == SemanticAction::decrement) {
        next -= axis.small_change();
        type = ScrollEventType::small_decrement;
    } else if (action == SemanticAction::set_value) {
        if (!parse_finite_number(value, next)) return false;
    } else {
        return false;
    }
    static_cast<void>(apply_axis_value(*orientation, next, type, true));
    return true;
}

} // namespace gui_forms
