#include "gui_forms/window.hpp"
#include "display_chunk.hpp"
#include "frame_scheduler.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <stdexcept>
#include <thread>
#include <unordered_set>

namespace gui_forms {

namespace {

constexpr std::uint32_t maximum_layout_passes = 4;

bool contains_control(const Control::Ptr& root, const Control::Ptr& candidate) {
    if (!root || !candidate) {
        return false;
    }
    if (root == candidate) {
        return true;
    }
    for (const auto& child : root->children()) {
        if (contains_control(child, candidate)) {
            return true;
        }
    }
    return false;
}

bool single_allowed_effect(DragEffect effect, DragEffect allowed) noexcept {
    return effect != DragEffect::none &&
           (effect == DragEffect::copy || effect == DragEffect::move ||
            effect == DragEffect::link) &&
           has_drag_effect(allowed, effect);
}

} // namespace

Window::Window(Control::Ptr root, Size client_size)
    : root_(std::move(root)), client_size_(client_size),
      ui_thread_(std::this_thread::get_id()) {
    if (!root_) {
        throw std::invalid_argument("GUI.Forms window requires a retained root control");
    }
    if (root_->parent()) {
        throw std::logic_error("GUI.Forms window root may not already have a parent");
    }
    if (!root_->is_alive()) {
        throw std::logic_error("GUI.Forms window root may not be disposed");
    }
    attach_subtree(root_, {});
    const Rect initial_damage{0.0, 0.0, client_size_.width, client_size_.height};
    add_damage_all_planes(initial_damage);
    metrics_.record_dirty_mark(initial_damage.area());
}

ImageLoadResult Window::load_png(std::span<const std::byte> encoded) {
    require_ui_thread("PNG resource load");
    return image_resources_.load_png(encoded);
}

ImageLoadResult Window::replace_png(ImageId image,
                                    std::span<const std::byte> encoded) {
    require_ui_thread("PNG resource replacement");
    ImageLoadResult result = image_resources_.replace_png(image, encoded);
    if (result) {
        add_damage_all_planes({0.0, 0.0, client_size_.width, client_size_.height});
        paint_dirty_ = true;
    }
    return result;
}

bool Window::remove_image(ImageId image) {
    require_ui_thread("PNG resource removal");
    if (!image_resources_.remove(image)) {
        return false;
    }
    add_damage_all_planes({0.0, 0.0, client_size_.width, client_size_.height});
    paint_dirty_ = true;
    return true;
}

Window::~Window() {
    for (const auto& request : frame_requests_) {
        request->disconnect();
    }
    frame_requests_.clear();
    update_frame_schedule_metrics();
    if (root_) {
        detach_subtree(root_);
    }
}

void Window::resize(Size client_size) {
    require_ui_thread("window resize");
    if (client_size.width < 0.0 || client_size.height < 0.0) {
        throw std::invalid_argument("GUI.Forms client size may not be negative");
    }
    if (client_size_ == client_size) {
        return;
    }
    const Rect old_bounds{0.0, 0.0, client_size_.width, client_size_.height};
    client_size_ = client_size;
    add_damage_all_planes(old_bounds);
    mark_subtree_dirty(*root_, invalidation::bounds);
}

void Window::set_scale(double scale) {
    require_ui_thread("scale mutation");
    if (!std::isfinite(scale) || scale <= 0.0) {
        throw std::invalid_argument("GUI.Forms scale must be finite and positive");
    }
    if (scale_ == scale) {
        return;
    }
    scale_ = scale;
    mark_subtree_dirty(*root_, invalidation::conservative_subtree);
}

UpdateScope Window::begin_update() {
    require_ui_thread("update scope");
    ++update_depth_;
    metrics_.enter_update_scope();
    return UpdateScope(*this);
}

void Window::perform_layout() {
    require_ui_thread("layout");
    ensure_layout(false);
}

void Window::flush() {
    require_ui_thread("flush");
    ensure_layout(false);
}

void Window::paint(Painter& painter, Rect requested_damage) {
    require_ui_thread("paint");
    ensure_layout(true);

    if (!root_->is_alive()) {
        return;
    }

    Rect paint_bounds = requested_damage;
    if (paint_bounds.empty()) {
        DamageRegion aggregate;
        for (const DamageRegion& plane : plane_damage_) {
            for (const Rect rect : plane.rectangles()) {
                aggregate.add(rect);
            }
        }
        paint_bounds = aggregate.bounds();
    }
    const Rect window_bounds{0.0, 0.0, client_size_.width, client_size_.height};
    paint_bounds = Rect::intersection(paint_bounds, window_bounds);
    if (paint_bounds.empty()) {
        return;
    }

    in_paint_ = true;
    std::uint64_t visited_nodes = 0;
    std::uint64_t painted_controls = 0;
    std::uint64_t consumed_invalidations = 0;
    std::uint64_t chunks_rebuilt = 0;
    std::uint64_t chunks_reused = 0;
    std::uint64_t commands_replayed = 0;
    for (std::size_t index = 0; index < paint_plane_count; ++index) {
        const PaintPlane plane = static_cast<PaintPlane>(index);
        Rect plane_bounds = paint_bounds;
        if (requested_damage.empty()) {
            plane_bounds = Rect::intersection(plane_damage_[index].bounds(), window_bounds);
        }
        if (plane_bounds.empty()) {
            continue;
        }
        paint_recursive(root_, painter, plane_bounds, plane, visited_nodes,
                        painted_controls, consumed_invalidations, chunks_rebuilt,
                        chunks_reused, commands_replayed);
        const Rect pending_bounds = plane_damage_[index].bounds();
        if (pending_bounds.empty() ||
            Rect::intersection(plane_bounds, pending_bounds) == pending_bounds) {
            plane_damage_[index].clear();
        }
    }
    in_paint_ = false;

    const bool full_window = paint_bounds.area() >= window_bounds.area();
    metrics_.record_paint(visited_nodes, painted_controls, consumed_invalidations,
                          chunks_rebuilt, chunks_reused, commands_replayed,
                          paint_bounds.area(), full_window);
    update_display_cache_metrics();

    paint_dirty_ = has_dirty(root_->subtree_dirty_, Dirty::paint) ||
                   std::any_of(plane_damage_.begin(), plane_damage_.end(),
                               [](const DamageRegion& damage) { return !damage.empty(); });
}

DamageRegion Window::take_damage() {
    require_ui_thread("damage mutation");
    DamageRegion result;
    for (DamageRegion& plane : plane_damage_) {
        for (const Rect rect : plane.rectangles()) {
            result.add(rect);
        }
        plane = {};
    }
    return result;
}

DamageRegion Window::take_damage(PaintPlane plane) {
    require_ui_thread("plane damage mutation");
    if (!is_valid_paint_plane(plane)) {
        throw std::invalid_argument("invalid paint plane");
    }
    const std::size_t index = paint_plane_index(plane);
    DamageRegion result = std::move(plane_damage_[index]);
    plane_damage_[index] = {};
    paint_dirty_ = has_dirty(root_->subtree_dirty_, Dirty::paint) ||
                   std::any_of(plane_damage_.begin(), plane_damage_.end(),
                               [](const DamageRegion& damage) { return !damage.empty(); });
    return result;
}

bool Window::needs_frame() const noexcept {
    return paint_dirty_;
}

std::optional<FrameTime> Window::next_wake() const noexcept {
    if (occluded_) {
        return std::nullopt;
    }
    std::optional<FrameTime> result;
    for (const auto& request : frame_requests_) {
        if (!request->connected()) {
            continue;
        }
        const auto target = request->target.lock();
        if (!target || !target->is_alive() || target->window_ != this) {
            continue;
        }
        if (!result || request->deadline < *result) {
            result = request->deadline;
        }
    }
    return result;
}

FrameRequestToken Window::schedule_paint(const Control::Ptr& control,
                                         FrameTime deadline) {
    require_ui_thread("frame deadline scheduling");
    if (!control || !control->is_alive() || control->window_ != this) {
        throw std::logic_error("GUI.Forms frame deadline requires an attached control");
    }
    compact_frame_requests();
    if (frame_requests_.size() >= maximum_scheduled_frame_requests) {
        throw std::length_error("GUI.Forms scheduled frame request limit reached");
    }
    auto request = std::make_shared<detail::ScheduledFrameRequest>(
        detail::FrameRequestKind::deadline, control, deadline);
    frame_requests_.push_back(request);
    control->own_revocable(request);
    metrics_.record_frame_request();
    update_frame_schedule_metrics();
    return FrameRequestToken(request);
}

FrameRequestToken Window::activate_surface(const Control::Ptr& control,
                                           FrameInterval interval,
                                           FrameTime first_deadline) {
    require_ui_thread("active surface scheduling");
    if (!control || !control->is_alive() || control->window_ != this) {
        throw std::logic_error("GUI.Forms active surface requires an attached control");
    }
    if (interval < minimum_active_surface_interval) {
        throw std::invalid_argument("GUI.Forms active surface interval is below the bound");
    }
    compact_frame_requests();
    if (frame_requests_.size() >= maximum_scheduled_frame_requests ||
        active_surface_count() >= maximum_active_surfaces) {
        throw std::length_error("GUI.Forms active surface limit reached");
    }
    auto request = std::make_shared<detail::ScheduledFrameRequest>(
        detail::FrameRequestKind::active_surface, control, first_deadline, interval);
    frame_requests_.push_back(request);
    control->own_revocable(request);
    metrics_.record_frame_request();
    update_frame_schedule_metrics();
    return FrameRequestToken(request);
}

FramePollResult Window::poll_frame_schedule(FrameTime now) {
    require_ui_thread("frame schedule polling");
    compact_frame_requests();

    FramePollResult result;
    if (occluded_) {
        result.damage_pending = needs_frame();
        result.suppressed_by_occlusion = true;
        metrics_.record_occluded_frame_poll();
        update_frame_schedule_metrics();
        return result;
    }
    std::unordered_set<std::uint64_t> invalidated_controls;
    for (const auto& request : frame_requests_) {
        if (!request->connected() || request->deadline > now) {
            continue;
        }
        const auto target = request->target.lock();
        if (!target || !target->is_alive() || target->window_ != this) {
            request->disconnect();
            continue;
        }

        if (request->kind == detail::FrameRequestKind::deadline) {
            ++result.deadlines_fired;
            request->disconnect();
        } else {
            ++result.active_surface_ticks;
            const FrameInterval lateness = now - request->deadline;
            const auto skipped = lateness / request->interval;
            result.coalesced_requests += static_cast<std::uint64_t>(skipped);
            // Never issue a burst of catch-up frames. A late active surface
            // emits one invalidation and starts its next interval from now.
            request->deadline = now + request->interval;
        }

        const bool already_requested = has_dirty(target->dirty_, Dirty::paint) ||
            !invalidated_controls.insert(target->runtime_id().value).second;
        if (already_requested) {
            ++result.coalesced_requests;
        } else {
            target->invalidate(invalidation::paint_only);
        }
    }

    compact_frame_requests();
    metrics_.record_frame_poll(result.deadlines_fired, result.active_surface_ticks,
                               result.coalesced_requests);
    update_frame_schedule_metrics();
    result.damage_pending = needs_frame();
    result.next_wake = next_wake();
    return result;
}

void Window::cancel_frame_requests() {
    require_ui_thread("frame schedule cancellation");
    for (const auto& request : frame_requests_) {
        request->disconnect();
    }
    frame_requests_.clear();
    update_frame_schedule_metrics();
}

void Window::set_occluded(bool occluded, FrameTime transition_time) {
    require_ui_thread("occlusion transition");
    if (occluded_ == occluded) {
        return;
    }
    occluded_ = occluded;
    metrics_.record_occlusion_transition(occluded);
    if (!occluded) {
        for (const auto& request : frame_requests_) {
            if (request->connected() &&
                request->kind == detail::FrameRequestKind::active_surface &&
                request->deadline < transition_time) {
                request->deadline = transition_time;
            }
        }
    }
    update_frame_schedule_metrics();
}

Control::Ptr Window::find(std::string_view stable_id) const {
    const auto found = stable_ids_.find(std::string(stable_id));
    return found == stable_ids_.end() ? Control::Ptr{} : found->second.lock();
}

Control::Ptr Window::hit_test(Point position) {
    require_ui_thread("hit test");
    ensure_layout(true);
    return hit_test_recursive(root_, position);
}

bool Window::request_focus(const Control::Ptr& control) {
    require_ui_thread("focus mutation");
    if (control && (!eligible(control) || !control->focusable_)) {
        return false;
    }
    Control::Ptr previous = focused_.lock();
    if (previous == control) {
        return true;
    }
    if (previous) {
        focused_.reset();
        metrics_.record_callback_emitted();
        previous->on_focus_changed(false);
        if (previous->is_alive()) {
            previous->invalidate(invalidation::focus);
        }
    }
    if (control && eligible(control) && control->focusable_) {
        focused_ = control;
        metrics_.record_callback_emitted();
        control->on_focus_changed(true);
        if (eligible(control)) {
            control->invalidate(invalidation::focus);
        } else {
            focused_.reset();
        }
    }
    metrics_.record_focus_transition();
    return true;
}

void Window::capture_pointer(const Control::Ptr& control, std::uint64_t pointer_id) {
    require_ui_thread("pointer capture");
    if (control && !eligible(control)) {
        throw std::logic_error("GUI.Forms cannot capture pointer to a foreign control");
    }
    change_pointer_capture(control, pointer_id == 0 ? 1 : pointer_id, false);
}

void Window::release_pointer() {
    require_ui_thread("pointer capture release");
    change_pointer_capture({}, 0, false);
}

void Window::change_pointer_capture(const Control::Ptr& control,
                                    std::uint64_t pointer_id,
                                    bool revoked) {
    const Control::Ptr previous = captured_.lock();
    if (previous == control && (!control || captured_pointer_id_ == pointer_id)) {
        return;
    }
    if (control) {
        captured_ = control;
        captured_pointer_id_ = pointer_id;
    } else {
        captured_.reset();
        captured_pointer_id_ = 0;
    }
    if (revoked && previous) {
        metrics_.record_capture_revocation();
    }
    PointerCaptureChange change;
    change.captured = static_cast<bool>(control);
    change.control_id = control ? control->runtime_id() : RuntimeId{};
    change.stable_id = control ? std::string(control->stable_id().value()) : std::string{};
    change.pointer_id = control ? pointer_id : 0;
    pointer_capture_changed_.emit(change);
}

bool Window::dispatch_pointer(PointerEvent event) {
    require_ui_thread("pointer dispatch");
    metrics_.record_input();
    Control::Ptr target = captured_.lock();
    if (!target) {
        target = hit_test(event.position);
    }
    if (!target) {
        return false;
    }

    const auto route = route_to(target);
    event.phase = EventPhase::preview;
    for (const auto& control : route) {
        if (!eligible(control)) {
            continue;
        }
        metrics_.record_callback_emitted();
        control->on_pointer_preview(event);
        if (event.handled) {
            if (event.action == PointerAction::up &&
                event.button == PointerButton::primary) {
                pressed_.reset();
                release_pointer();
            }
            return true;
        }
    }

    if (!eligible(target)) {
        return event.handled;
    }

    if (event.action == PointerAction::down &&
        event.button == PointerButton::primary) {
        request_focus(target);
        if (eligible(target)) {
            pressed_ = target;
            capture_pointer(target, event.pointer_id == 0 ? 1 : event.pointer_id);
        }
    }

    if (eligible(target)) {
        event.phase = EventPhase::target;
        metrics_.record_callback_emitted();
        target->on_pointer(event);
    }
    if (!event.handled) {
        event.phase = EventPhase::bubble;
        for (auto iterator = route.rbegin(); iterator != route.rend(); ++iterator) {
            if (*iterator == target || !eligible(*iterator)) {
                continue;
            }
            metrics_.record_callback_emitted();
            (*iterator)->on_pointer_bubble(event);
            if (event.handled) {
                break;
            }
        }
    }

    if (event.action == PointerAction::up &&
        event.button == PointerButton::primary) {
        Control::Ptr pressed = pressed_.lock();
        pressed_.reset();
        release_pointer();
        Control::Ptr released_over = hit_test(event.position);
        if (pressed && pressed == released_over && eligible(pressed)) {
            metrics_.record_callback_emitted();
            pressed->on_activate();
            metrics_.record_activation();
        }
    }
    return event.handled;
}

bool Window::dispatch_key(KeyEvent event) {
    require_ui_thread("key dispatch");
    metrics_.record_input();
    Control::Ptr target = focused_.lock();
    if (!target || !eligible(target)) {
        return false;
    }
    const auto route = route_to(target);
    event.phase = EventPhase::preview;
    for (const auto& control : route) {
        if (!eligible(control)) {
            continue;
        }
        metrics_.record_callback_emitted();
        control->on_key_preview(event);
        if (event.handled) {
            return true;
        }
    }
    if (!eligible(target)) {
        return event.handled;
    }
    event.phase = EventPhase::target;
    metrics_.record_callback_emitted();
    target->on_key(event);
    if (!event.handled) {
        event.phase = EventPhase::bubble;
        for (auto iterator = route.rbegin(); iterator != route.rend(); ++iterator) {
            if (*iterator == target || !eligible(*iterator)) {
                continue;
            }
            metrics_.record_callback_emitted();
            (*iterator)->on_key_bubble(event);
            if (event.handled) {
                break;
            }
        }
    }
    return event.handled;
}

bool Window::dispatch_text(TextInputEvent event) {
    require_ui_thread("text dispatch");
    metrics_.record_input();
    Control::Ptr target = focused_.lock();
    if (!target || !eligible(target)) {
        return false;
    }
    metrics_.record_callback_emitted();
    target->on_text_input(event);
    return event.handled;
}

Control::Ptr Window::drop_target_at(Point position) {
    for (Control::Ptr target = hit_test(position); target; target = target->parent()) {
        if (eligible(target) && target->allow_drop()) {
            return target;
        }
    }
    return {};
}

DragDispatchResult Window::route_drag(const Control::Ptr& target, DragEvent event) {
    DragDispatchResult result;
    if (!target || !eligible(target) || !target->allow_drop()) {
        return result;
    }
    const auto route = route_to(target);
    event.phase = EventPhase::preview;
    for (const auto& control : route) {
        if (!eligible(control)) {
            continue;
        }
        metrics_.record_callback_emitted();
        control->on_drag_preview(event);
        if (event.handled) {
            break;
        }
    }
    if (!event.handled && eligible(target) && target->allow_drop()) {
        event.phase = EventPhase::target;
        metrics_.record_callback_emitted();
        target->on_drag(event);
    }
    if (!event.handled) {
        event.phase = EventPhase::bubble;
        for (auto iterator = route.rbegin(); iterator != route.rend(); ++iterator) {
            if (*iterator == target || !eligible(*iterator)) {
                continue;
            }
            metrics_.record_callback_emitted();
            (*iterator)->on_drag_bubble(event);
            if (event.handled) {
                break;
            }
        }
    }
    result.handled = event.handled;
    if (single_allowed_effect(event.accepted_effect, event.allowed_effects)) {
        result.accepted_effect = event.accepted_effect;
    }
    return result;
}

DragDispatchResult Window::dispatch_drag(DragEvent event) {
    require_ui_thread("drag dispatch");
    metrics_.record_input();

    DragDispatchResult aggregate;
    const auto route_and_merge = [this, &aggregate](const Control::Ptr& target,
                                                     DragEvent routed) {
        const DragDispatchResult current = route_drag(target, std::move(routed));
        aggregate.handled = aggregate.handled || current.handled;
        if (current.accepted_effect != DragEffect::none) {
            aggregate.accepted_effect = current.accepted_effect;
        }
    };
    const auto leave_current = [this, &event, &route_and_merge]() {
        Control::Ptr previous = drag_target_.lock();
        if (previous) {
            DragEvent leave = event;
            leave.action = DragAction::leave;
            leave.accepted_effect = DragEffect::none;
            leave.handled = false;
            route_and_merge(previous, std::move(leave));
        }
        drag_target_.reset();
    };

    if (event.action == DragAction::leave) {
        if (drag_session_id_ == event.session_id) {
            leave_current();
            drag_session_id_ = 0;
        }
        return aggregate;
    }

    if (drag_session_id_ != 0 && drag_session_id_ != event.session_id) {
        leave_current();
        drag_session_id_ = 0;
    }
    if (drag_session_id_ == 0) {
        drag_session_id_ = event.session_id;
    }

    Control::Ptr candidate = drop_target_at(event.position);
    Control::Ptr previous = drag_target_.lock();
    if (candidate != previous) {
        leave_current();
        if (candidate) {
            drag_target_ = candidate;
            DragEvent enter = event;
            enter.action = DragAction::enter;
            enter.accepted_effect = DragEffect::none;
            enter.handled = false;
            route_and_merge(candidate, std::move(enter));
        }
    }

    if (event.action == DragAction::over && candidate) {
        event.accepted_effect = DragEffect::none;
        event.handled = false;
        route_and_merge(candidate, std::move(event));
    } else if (event.action == DragAction::drop && candidate) {
        event.accepted_effect = DragEffect::none;
        event.handled = false;
        route_and_merge(candidate, std::move(event));
        drag_target_.reset();
        drag_session_id_ = 0;
    } else if (event.action == DragAction::drop) {
        drag_target_.reset();
        drag_session_id_ = 0;
    }
    return aggregate;
}

void Window::cancel_drag() noexcept {
    drag_target_.reset();
    drag_session_id_ = 0;
}

void Window::attach_subtree(const Control::Ptr& control, const Control::WeakPtr& parent) {
    require_ui_thread("visual-tree attachment");
    register_subtree(control);
    std::function<void(const Control::Ptr&, const Control::WeakPtr&)> attach =
        [&](const Control::Ptr& current, const Control::WeakPtr& current_parent) {
            current->window_ = this;
            current->parent_ = current_parent;
            for (const auto& child : current->children_) {
                attach(child, current);
            }
        };
    attach(control, parent);
    metrics_.set_population(root_ ? root_->subtree_size() : control->subtree_size(),
                            stable_ids_.size());
    static_cast<void>(recompute_subtree_dirty(root_));
    mark_subtree_dirty(*control, invalidation::visual_tree);
}

void Window::detach_subtree(const Control::Ptr& control) {
    require_ui_thread("visual-tree detachment");
    revoke_interaction_for_subtree(control, true);
    if (control->window_ != this) {
        return;
    }
    const Rect old_bounds = absolute_bounds_of(*control);
    add_subtree_damage(control);
    unregister_subtree(control);
    std::function<void(const Control::Ptr&)> detach = [&](const Control::Ptr& current) {
        current->window_ = nullptr;
        for (const auto& child : current->children_) {
            detach(child);
        }
    };
    detach(control);
    paint_dirty_ = true;
    metrics_.record_dirty_mark(old_bounds.area());
    metrics_.set_population(stable_ids_.size(), stable_ids_.size());
    update_display_cache_metrics();
}

void Window::dispose_subtree(const Control::Ptr& control) noexcept {
    const std::uint64_t disposal_count = control->subtree_size();
    revoke_interaction_for_subtree(control, false);
    const Rect old_bounds = absolute_bounds_of(*control);
    add_subtree_damage(control);
    unregister_subtree(control);
    std::function<void(const Control::Ptr&)> detach = [&](const Control::Ptr& current) {
        current->window_ = nullptr;
        for (const auto& child : current->children_) {
            detach(child);
        }
    };
    detach(control);
    if (auto visual_parent = control->parent_.lock()) {
        const auto found = std::find(visual_parent->children_.begin(),
                                    visual_parent->children_.end(), control);
        if (found != visual_parent->children_.end()) {
            visual_parent->children_.erase(found);
        }
    }
    control->parent_.reset();
    paint_dirty_ = true;
    layout_dirty_ = true;
    hit_test_dirty_ = true;
    metrics_.record_dirty_mark(old_bounds.area());
    metrics_.record_disposal(disposal_count);
    metrics_.set_population(stable_ids_.size(), stable_ids_.size());
    static_cast<void>(recompute_subtree_dirty(root_));
    update_display_cache_metrics();
}

void Window::revoke_interaction_for_subtree(const Control::Ptr& control,
                                            bool notify_focus) {
    Control::Ptr focused = focused_.lock();
    if (contains_control(control, focused)) {
        focused_.reset();
        metrics_.record_focus_transition();
        metrics_.record_focus_revocation();
        if (notify_focus && focused && focused->is_alive()) {
            metrics_.record_callback_emitted();
            focused->on_focus_changed(false);
            if (focused->is_alive()) {
                focused->invalidate(invalidation::focus);
            }
        }
    }
    if (contains_control(control, captured_.lock())) {
        change_pointer_capture({}, 0, true);
    }
    if (contains_control(control, pressed_.lock())) {
        pressed_.reset();
        metrics_.record_press_revocation();
    }
    if (contains_control(control, drag_target_.lock())) {
        cancel_drag();
    }
}

void Window::on_eligibility_changed(const Control::Ptr& control) {
    require_ui_thread("eligibility mutation");
    revoke_interaction_for_subtree(control, true);
}

void Window::register_subtree(const Control::Ptr& control) {
    std::vector<Control::Ptr> controls;
    std::unordered_set<std::string> local_ids;
    std::function<void(const Control::Ptr&)> collect = [&](const Control::Ptr& current) {
        if (!current->is_alive()) {
            throw std::logic_error("GUI.Forms cannot attach a disposed control");
        }
        const std::string id(current->stable_id_.value());
        if (!local_ids.insert(id).second || stable_ids_.contains(id)) {
            throw std::logic_error("GUI.Forms duplicate stable ID: " + id);
        }
        controls.push_back(current);
        for (const auto& child : current->children_) {
            collect(child);
        }
    };
    collect(control);
    for (const auto& current : controls) {
        stable_ids_.emplace(std::string(current->stable_id_.value()), current);
    }
}

void Window::unregister_subtree(const Control::Ptr& control) {
    stable_ids_.erase(std::string(control->stable_id_.value()));
    for (const auto& child : control->children_) {
        unregister_subtree(child);
    }
}

void Window::mark_dirty(Control& control, Dirty requested_dirty) {
    require_ui_thread("dirty mutation");
    if (has_dirty(requested_dirty, Dirty::measure)) {
        requested_dirty |= Dirty::arrange;
    }
    control.dirty_ |= requested_dirty;
    control.subtree_dirty_ |= requested_dirty;
    metrics_.record_mutation();

    if (has_dirty(requested_dirty, Dirty::layout)) {
        for (auto ancestor = control.parent(); ancestor; ancestor = ancestor->parent()) {
            ancestor->dirty_ |= Dirty::measure | Dirty::arrange;
            ancestor->subtree_dirty_ |= Dirty::measure | Dirty::arrange;
        }
        layout_dirty_ = true;
        if (in_layout_) {
            second_layout_pass_requested_ = true;
        }
    }
    for (auto ancestor = control.parent(); ancestor; ancestor = ancestor->parent()) {
        ancestor->subtree_dirty_ |= requested_dirty;
    }
    if (has_dirty(requested_dirty, Dirty::hit_test)) {
        hit_test_dirty_ = true;
    }
    if (has_dirty(requested_dirty, Dirty::paint)) {
        paint_dirty_ = true;
        Rect bounds = absolute_bounds_of(control);
        if (bounds.empty()) {
            bounds = control.requested_bounds_;
        }
        add_damage(bounds, control.paint_plane_);
        metrics_.record_dirty_mark(bounds.area());
    } else {
        metrics_.record_dirty_mark(0.0);
    }
}

void Window::mark_subtree_dirty(Control& control, Dirty requested_dirty) {
    require_ui_thread("subtree dirty mutation");
    if (has_dirty(requested_dirty, Dirty::measure)) {
        requested_dirty |= Dirty::arrange;
    }
    double requested_damage_area = 0.0;
    std::function<void(Control&)> apply = [&](Control& current) {
        current.dirty_ |= requested_dirty;
        current.subtree_dirty_ |= requested_dirty;
        if (has_dirty(requested_dirty, Dirty::paint)) {
            Rect bounds = absolute_bounds_of(current);
            if (bounds.empty()) {
                bounds = current.requested_bounds_;
            }
            add_damage(bounds, current.paint_plane_);
            requested_damage_area += bounds.area();
        }
        for (const auto& child : current.children_) {
            apply(*child);
        }
    };
    apply(control);
    for (auto ancestor = control.parent(); ancestor; ancestor = ancestor->parent()) {
        ancestor->subtree_dirty_ |= requested_dirty;
        if (has_dirty(requested_dirty, Dirty::layout)) {
            ancestor->dirty_ |= Dirty::measure | Dirty::arrange;
        }
    }
    metrics_.record_mutation();
    if (has_dirty(requested_dirty, Dirty::layout)) {
        layout_dirty_ = true;
        if (in_layout_) {
            second_layout_pass_requested_ = true;
        }
    }
    if (has_dirty(requested_dirty, Dirty::hit_test)) {
        hit_test_dirty_ = true;
    }
    if (has_dirty(requested_dirty, Dirty::paint)) {
        paint_dirty_ = true;
        metrics_.record_dirty_mark(requested_damage_area);
    } else {
        metrics_.record_dirty_mark(0.0);
    }
}

void Window::change_paint_plane(Control& control, PaintPlane plane) {
    require_ui_thread("paint-plane mutation");
    const Rect bounds = absolute_bounds_of(control);
    add_damage(bounds, control.paint_plane_);
    control.paint_plane_ = plane;
    control.display_chunk_.reset();
    mark_dirty(control, invalidation::paint_only);
    update_display_cache_metrics();
}

void Window::add_damage(Rect damage, PaintPlane plane) {
    DamageRegion& region = plane_damage_[paint_plane_index(plane)];
    const std::uint64_t compactions = region.compaction_count();
    const std::uint64_t collapses = region.collapse_count();
    region.add(damage);
    metrics_.record_damage_region(
        region.rectangle_count(), region.compaction_count() - compactions,
        region.collapse_count() - collapses);
}

void Window::add_damage_all_planes(Rect damage) {
    for (std::size_t index = 0; index < paint_plane_count; ++index) {
        add_damage(damage, static_cast<PaintPlane>(index));
    }
}

void Window::add_subtree_damage(const Control::Ptr& control) {
    add_damage(absolute_bounds_of(*control), control->paint_plane_);
    for (const auto& child : control->children_) {
        add_subtree_damage(child);
    }
}

void Window::ensure_layout(bool read_barrier) {
    require_ui_thread("layout");
    if (!root_->is_alive()) {
        layout_dirty_ = false;
        hit_test_dirty_ = false;
        return;
    }
    if (!layout_dirty_) {
        return;
    }
    if (read_barrier && update_depth_ > 0) {
        return;
    }
    if (in_layout_) {
        second_layout_pass_requested_ = true;
        return;
    }

    metrics_.record_flush(read_barrier);
    in_layout_ = true;
    std::uint32_t pass = 0;
    do {
        second_layout_pass_requested_ = false;
        std::uint64_t measure_visited = 0;
        std::uint64_t measured = 0;
        measure_dirty_recursive(root_, client_size_, measure_visited, measured);
        metrics_.record_measure(measure_visited, measured);

        std::uint64_t arrange_visited = 0;
        std::uint64_t arranged = 0;
        arrange_dirty_recursive(root_,
                                {0.0, 0.0, client_size_.width, client_size_.height},
                                arrange_visited, arranged);
        metrics_.record_arrange(arrange_visited, arranged);

        static_cast<void>(recompute_subtree_dirty(root_));
        ++pass;
    } while (has_dirty(root_->subtree_dirty_, Dirty::layout) &&
             pass < maximum_layout_passes);

    if (has_dirty(root_->subtree_dirty_, Dirty::layout)) {
        metrics_.record_pass_limit_hit();
    }
    layout_dirty_ = has_dirty(root_->subtree_dirty_, Dirty::layout);
    hit_test_dirty_ = layout_dirty_;
    in_layout_ = false;
}

void Window::leave_update_scope() {
    require_ui_thread("update scope close");
    if (update_depth_ == 0) {
        throw std::logic_error("GUI.Forms update scope underflow");
    }
    --update_depth_;
    metrics_.leave_update_scope();
    flush_if_outermost();
}

void Window::flush_if_outermost() {
    if (update_depth_ == 0) {
        ensure_layout(false);
    }
}

std::vector<Control::Ptr> Window::route_to(const Control::Ptr& target) const {
    std::vector<Control::Ptr> route;
    for (auto current = target; current; current = current->parent()) {
        route.push_back(current);
    }
    std::reverse(route.begin(), route.end());
    return route;
}

Control::Ptr Window::hit_test_recursive(const Control::Ptr& control,
                                        Point window_position) const {
    if (!eligible(control)) {
        return {};
    }
    const Rect bounds = absolute_bounds_of(*control);
    if (!bounds.contains(window_position)) {
        return {};
    }
    for (auto child = control->children_.rbegin(); child != control->children_.rend(); ++child) {
        if (auto target = hit_test_recursive(*child, window_position)) {
            return target;
        }
    }
    const Point local{window_position.x - bounds.x, window_position.y - bounds.y};
    return control->hit_test_local(local) ? control : Control::Ptr{};
}

void Window::measure_dirty_recursive(const Control::Ptr& control,
                                     Size available,
                                     std::uint64_t& visited_nodes,
                                     std::uint64_t& callbacks) {
    if (!has_dirty(control->subtree_dirty_, Dirty::measure)) {
        return;
    }
    ++visited_nodes;
    if (!control->is_alive() || !control->visible_) {
        clear_layout_dirty_subtree(control);
        return;
    }

    const Size child_available{
        control->requested_bounds_.width > 0.0 ? control->requested_bounds_.width
                                               : available.width,
        control->requested_bounds_.height > 0.0 ? control->requested_bounds_.height
                                                : available.height};
    for (const auto& child : control->children_) {
        measure_dirty_recursive(child, child_available, visited_nodes, callbacks);
    }
    if (has_dirty(control->dirty_, Dirty::measure)) {
        control->clear_dirty(Dirty::measure);
        static_cast<void>(control->measure(available));
        ++callbacks;
    }
}

void Window::arrange_dirty_recursive(const Control::Ptr& control,
                                     Rect final_bounds,
                                     std::uint64_t& visited_nodes,
                                     std::uint64_t& callbacks) {
    if (!has_dirty(control->subtree_dirty_, Dirty::arrange)) {
        return;
    }
    ++visited_nodes;
    if (!control->is_alive() || !control->visible_) {
        clear_layout_dirty_subtree(control);
        return;
    }

    if (has_dirty(control->dirty_, Dirty::arrange)) {
        const Rect old_bounds = absolute_bounds_of(*control);
        control->clear_dirty(Dirty::arrange | Dirty::hit_test);
        control->arrange(final_bounds);
        ++callbacks;
        const Rect new_bounds = absolute_bounds_of(*control);
        if (old_bounds != new_bounds) {
            add_damage(old_bounds, control->paint_plane_);
            add_damage(new_bounds, control->paint_plane_);
            paint_dirty_ = true;
        }
    }
    for (const auto& child : control->children_) {
        arrange_dirty_recursive(child, child->requested_bounds_, visited_nodes, callbacks);
    }
}

Dirty Window::recompute_subtree_dirty(const Control::Ptr& control) noexcept {
    Dirty summary = control->dirty_;
    for (const auto& child : control->children_) {
        summary |= recompute_subtree_dirty(child);
    }
    control->subtree_dirty_ = summary;
    return summary;
}

void Window::clear_layout_dirty_subtree(const Control::Ptr& control) noexcept {
    control->clear_dirty(Dirty::layout | Dirty::hit_test);
    control->clear_subtree_dirty(Dirty::layout | Dirty::hit_test);
    for (const auto& child : control->children_) {
        clear_layout_dirty_subtree(child);
    }
}

void Window::clear_paint_dirty_subtree(const Control::Ptr& control) noexcept {
    control->clear_dirty(Dirty::paint);
    control->clear_subtree_dirty(Dirty::paint);
    for (const auto& child : control->children_) {
        clear_paint_dirty_subtree(child);
    }
}

void Window::paint_recursive(const Control::Ptr& control,
                             Painter& painter,
                             Rect window_damage,
                             PaintPlane plane,
                             std::uint64_t& visited_nodes,
                             std::uint64_t& painted_controls,
                             std::uint64_t& consumed_invalidations,
                             std::uint64_t& chunks_rebuilt,
                             std::uint64_t& chunks_reused,
                             std::uint64_t& commands_replayed) {
    ++visited_nodes;
    if (!control->is_alive() || !control->visible_) {
        clear_paint_dirty_subtree(control);
        return;
    }
    const Rect bounds = absolute_bounds_of(*control);
    const Rect intersection = Rect::intersection(bounds, window_damage);
    if (intersection.empty()) {
        return;
    }

    if (control->paint_plane_ == plane) {
        const Rect logical_bounds{0.0, 0.0, bounds.width, bounds.height};
        const bool invalidated = has_dirty(control->dirty_, Dirty::paint);
        const bool rebuild = invalidated || !control->display_chunk_ ||
                             control->display_chunk_->plane() != plane ||
                             control->display_chunk_->logical_bounds() != logical_bounds;
        if (rebuild) {
            control->clear_dirty(Dirty::paint);
            detail::RecordingPainter recorder;
            control->on_paint(recorder, logical_bounds);
            control->display_chunk_ = recorder.finish(++display_generation_, plane,
                                                      logical_bounds);
            ++chunks_rebuilt;
            consumed_invalidations += invalidated ? 1U : 0U;
        } else {
            ++chunks_reused;
        }

        painter.save();
        painter.translate({bounds.x, bounds.y});
        const Rect local_damage{intersection.x - bounds.x, intersection.y - bounds.y,
                                intersection.width, intersection.height};
        painter.clip_rect(local_damage);
        commands_replayed +=
            detail::replay_display_chunk(*control->display_chunk_, painter);
        painter.restore();
        ++painted_controls;
    }

    for (const auto& child : control->children_) {
        paint_recursive(child, painter, intersection, plane, visited_nodes,
                        painted_controls, consumed_invalidations, chunks_rebuilt,
                        chunks_reused, commands_replayed);
    }

    Dirty paint_summary = control->dirty_ & Dirty::paint;
    for (const auto& child : control->children_) {
        paint_summary |= child->subtree_dirty_ & Dirty::paint;
    }
    control->subtree_dirty_ = without_dirty(control->subtree_dirty_, Dirty::paint) |
                              paint_summary;
}

std::uint64_t Window::display_cache_entries(const Control::Ptr& control) const noexcept {
    std::uint64_t entries = control->display_chunk_ ? 1U : 0U;
    for (const auto& child : control->children_) {
        entries += display_cache_entries(child);
    }
    return entries;
}

void Window::update_display_cache_metrics() noexcept {
    metrics_.set_display_cache(root_ ? display_cache_entries(root_) : 0U,
                               display_generation_);
}

void Window::compact_frame_requests() noexcept {
    for (const auto& request : frame_requests_) {
        const auto target = request->target.lock();
        if (!target || !target->is_alive() || target->window_ != this) {
            request->disconnect();
        }
    }
    std::erase_if(frame_requests_, [](const auto& request) {
        return !request->connected();
    });
}

std::size_t Window::active_surface_count() const noexcept {
    return static_cast<std::size_t>(std::count_if(
        frame_requests_.begin(), frame_requests_.end(), [this](const auto& request) {
            if (!request->connected() ||
                request->kind != detail::FrameRequestKind::active_surface) {
                return false;
            }
            const auto target = request->target.lock();
            return target && target->is_alive() && target->window_ == this;
        }));
}

void Window::update_frame_schedule_metrics() noexcept {
    metrics_.set_active_surface_count(active_surface_count());
}

Rect Window::absolute_bounds_of(const Control& control) const {
    Rect result = control.arranged_bounds_;
    for (auto ancestor = control.parent(); ancestor; ancestor = ancestor->parent()) {
        result.x += ancestor->arranged_bounds_.x;
        result.y += ancestor->arranged_bounds_.y;
    }
    return result;
}

bool Window::eligible(const Control::Ptr& control) const noexcept {
    return control && control->window_ == this && control->eligible_for_input();
}

void Window::require_ui_thread(std::string_view operation) {
    if (std::this_thread::get_id() == ui_thread_) {
        return;
    }
    metrics_.record_wrong_thread_rejection();
    throw std::logic_error("GUI.Forms rejected wrong-thread " + std::string(operation));
}

UpdateScope::~UpdateScope() {
    close();
}

UpdateScope::UpdateScope(UpdateScope&& other) noexcept
    : window_(std::exchange(other.window_, nullptr)) {}

UpdateScope& UpdateScope::operator=(UpdateScope&& other) noexcept {
    if (this != &other) {
        close();
        window_ = std::exchange(other.window_, nullptr);
    }
    return *this;
}

void UpdateScope::perform_layout() {
    if (window_) {
        window_->perform_layout();
    }
}

void UpdateScope::close() {
    if (window_) {
        Window* closing = std::exchange(window_, nullptr);
        closing->leave_update_scope();
    }
}

} // namespace gui_forms
