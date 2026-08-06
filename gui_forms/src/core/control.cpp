#include "gui_forms/control.hpp"

#include "gui_forms/window.hpp"
#include "display_chunk.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <stdexcept>

namespace gui_forms {

std::atomic<std::uint64_t> Control::next_runtime_id_{1};

StableId::StableId(std::string value) : value_(std::move(value)) {
    if (value_.empty()) {
        throw std::invalid_argument("GUI.Forms stable IDs may not be empty");
    }
}

Control::Control(StableId stable_id)
    : runtime_id_{next_runtime_id_.fetch_add(1, std::memory_order_relaxed)},
      stable_id_(std::move(stable_id)) {}

Control::~Control() = default;

void Control::set_tag(std::any tag) {
    require_mutable();
    tag_ = std::move(tag);
}

namespace {

void validate_insets(Insets value, const char* message) {
    if (!std::isfinite(value.left) || !std::isfinite(value.top) ||
        !std::isfinite(value.right) || !std::isfinite(value.bottom) ||
        value.left < 0.0 || value.top < 0.0 || value.right < 0.0 ||
        value.bottom < 0.0) {
        throw std::invalid_argument(message);
    }
}

[[nodiscard]] Rect client_rect(Size size, Insets padding) noexcept {
    return {padding.left, padding.top,
            std::max(0.0, size.width - padding.left - padding.right),
            std::max(0.0, size.height - padding.top - padding.bottom)};
}

[[nodiscard]] bool valid_dock(DockStyle dock) noexcept {
    switch (dock) {
    case DockStyle::none:
    case DockStyle::top:
    case DockStyle::bottom:
    case DockStyle::left:
    case DockStyle::right:
    case DockStyle::fill:
        return true;
    }
    return false;
}

[[nodiscard]] bool valid_anchor(AnchorStyles anchor) noexcept {
    constexpr std::uint8_t valid =
        static_cast<std::uint8_t>(AnchorStyles::top) |
        static_cast<std::uint8_t>(AnchorStyles::bottom) |
        static_cast<std::uint8_t>(AnchorStyles::left) |
        static_cast<std::uint8_t>(AnchorStyles::right);
    return (static_cast<std::uint8_t>(anchor) & ~valid) == 0U;
}

[[nodiscard]] Rect anchored_bounds(Rect reference, Rect old_client,
                                   Rect new_client, AnchorStyles anchor) noexcept {
    const double left = reference.x - old_client.x;
    const double right = old_client.x + old_client.width -
                         (reference.x + reference.width);
    const double top = reference.y - old_client.y;
    const double bottom = old_client.y + old_client.height -
                          (reference.y + reference.height);
    const bool anchored_left = has_anchor(anchor, AnchorStyles::left);
    const bool anchored_right = has_anchor(anchor, AnchorStyles::right);
    const bool anchored_top = has_anchor(anchor, AnchorStyles::top);
    const bool anchored_bottom = has_anchor(anchor, AnchorStyles::bottom);

    Rect result = reference;
    if (anchored_left && anchored_right) {
        result.x = new_client.x + left;
        result.width = std::max(0.0, new_client.width - left - right);
    } else if (anchored_right) {
        result.x = new_client.x + new_client.width - right - reference.width;
    } else if (anchored_left) {
        result.x = new_client.x + left;
    } else {
        const double offset = reference.x + reference.width * 0.5 -
                              (old_client.x + old_client.width * 0.5);
        result.x = new_client.x + new_client.width * 0.5 + offset -
                   reference.width * 0.5;
    }

    if (anchored_top && anchored_bottom) {
        result.y = new_client.y + top;
        result.height = std::max(0.0, new_client.height - top - bottom);
    } else if (anchored_bottom) {
        result.y = new_client.y + new_client.height - bottom - reference.height;
    } else if (anchored_top) {
        result.y = new_client.y + top;
    } else {
        const double offset = reference.y + reference.height * 0.5 -
                              (old_client.y + old_client.height * 0.5);
        result.y = new_client.y + new_client.height * 0.5 + offset -
                   reference.height * 0.5;
    }
    return result;
}

} // namespace

void Control::add_child(Ptr child) {
    require_mutable();
    if (lifecycle_notification_ ||
        (window_ != nullptr && window_->in_lifecycle_notification_)) {
        throw std::logic_error("GUI.Forms cannot mutate the visual tree during lifecycle notification");
    }
    if (!child) {
        throw std::invalid_argument("GUI.Forms cannot add a null child");
    }
    if (child.get() == this) {
        throw std::logic_error("GUI.Forms control cannot parent itself");
    }
    if (!child->is_alive()) {
        throw std::logic_error("GUI.Forms cannot attach a disposed control");
    }
    if (child->window_) {
        child->window_->require_ui_thread("visual-tree mutation");
        if (child->window_->in_lifecycle_notification_) {
            throw std::logic_error("GUI.Forms cannot mutate the visual tree during lifecycle notification");
        }
    }
    for (auto ancestor = shared_from_this(); ancestor; ancestor = ancestor->parent()) {
        if (ancestor == child) {
            throw std::logic_error("GUI.Forms control tree cannot contain a cycle");
        }
    }
    if (child->parent().get() == this) {
        return;
    }
    if (child->window_ && child->window_ != window_) {
        throw std::logic_error("GUI.Forms control belongs to another window");
    }
    if (auto previous_parent = child->parent()) {
        static_cast<void>(previous_parent->remove_child(child->runtime_id()));
    } else if (child->window_) {
        throw std::logic_error("GUI.Forms cannot reparent another window root");
    }
    if (!child->is_alive()) {
        throw std::logic_error("GUI.Forms callback disposed control during reparenting");
    }

    child->parent_ = weak_from_this();
    child->layout_slot_.reset();
    child->anchor_reference_.reset();
    children_.push_back(child);
    if (window_) {
        try {
            window_->attach_subtree(child, weak_from_this());
        } catch (...) {
            children_.pop_back();
            child->parent_.reset();
            throw;
        }
    }
    invalidate_declared(invalidation::visual_tree);
}

Control::Ptr Control::remove_child(RuntimeId child_id) {
    require_mutable();
    if (lifecycle_notification_ ||
        (window_ != nullptr && window_->in_lifecycle_notification_)) {
        throw std::logic_error("GUI.Forms cannot mutate the visual tree during lifecycle notification");
    }
    const auto found = std::find_if(children_.begin(), children_.end(),
                                    [child_id](const Ptr& candidate) {
                                        return candidate->runtime_id() == child_id;
                                    });
    if (found == children_.end()) {
        return {};
    }
    Ptr removed = *found;
    if (window_) {
        window_->detach_subtree(removed);
    }
    const auto current = std::find_if(children_.begin(), children_.end(),
                                      [child_id](const Ptr& candidate) {
                                          return candidate->runtime_id() == child_id;
                                      });
    if (current != children_.end()) {
        children_.erase(current);
    }
    if (removed->parent_.lock().get() == this) {
        removed->parent_.reset();
    }
    removed->layout_slot_.reset();
    removed->anchor_reference_.reset();
    if (window_) {
        static_cast<void>(window_->recompute_subtree_dirty(window_->root_));
    }
    invalidate_declared(invalidation::visual_tree);
    return removed;
}

bool Control::set_child_index(RuntimeId child_id, std::size_t index) {
    require_mutable();
    const auto found = std::find_if(children_.begin(), children_.end(),
                                    [child_id](const Ptr& candidate) {
                                        return candidate->runtime_id() == child_id;
                                    });
    if (found == children_.end()) {
        return false;
    }
    Ptr child = *found;
    children_.erase(found);
    // WinForms index zero is topmost. GUI.Forms paints from front to back, so
    // the topmost retained child is the final vector element.
    const std::size_t bounded = std::min(index, children_.size());
    children_.insert(children_.end() - static_cast<std::ptrdiff_t>(bounded),
                     std::move(child));
    invalidate_declared(invalidation::visual_tree);
    return true;
}

void Control::clear_children() {
    require_mutable();
    if (lifecycle_notification_ ||
        (window_ != nullptr && window_->in_lifecycle_notification_)) {
        throw std::logic_error("GUI.Forms cannot mutate the visual tree during lifecycle notification");
    }
    while (!children_.empty()) {
        static_cast<void>(remove_child(children_.back()->runtime_id()));
    }
}

void Control::set_requested_bounds(Rect bounds) {
    require_mutable();
    if (requested_bounds_ == bounds) {
        return;
    }
    requested_bounds_ = bounds;
    anchor_reference_.reset();
    invalidate_declared(invalidation::bounds);
}

void Control::set_child_layout(const Ptr& child, Rect bounds) {
    require_mutable();
    if (!child || !child->is_alive() || child->parent().get() != this) {
        throw std::invalid_argument(
            "GUI.Forms child layout requires a live direct child");
    }
    if (!std::isfinite(bounds.x) || !std::isfinite(bounds.y) ||
        !std::isfinite(bounds.width) || !std::isfinite(bounds.height) ||
        bounds.width < 0.0 || bounds.height < 0.0) {
        throw std::invalid_argument(
            "GUI.Forms child layout bounds must be finite and nonnegative");
    }
    if (child->layout_slot_ == bounds) return;
    child->layout_slot_ = bounds;
    if (window_ != nullptr) {
        window_->mark_child_layout_slot(*child);
    } else {
        constexpr Dirty effects = Dirty::arrange | Dirty::hit_test |
                                  Dirty::semantics | Dirty::accessibility;
        child->dirty_ |= effects;
        child->subtree_dirty_ |= effects;
        for (Ptr ancestor = child->parent(); ancestor;
             ancestor = ancestor->parent()) {
            ancestor->subtree_dirty_ |= effects;
        }
    }
}

void Control::arrange_self(Rect final_bounds) noexcept {
    arranged_bounds_ = final_bounds;
}

void Control::set_margin(Insets margin) {
    require_mutable();
    validate_insets(margin, "GUI.Forms margin must be finite and nonnegative");
    if (margin_ == margin) return;
    margin_ = margin;
    invalidate(Dirty::measure | Dirty::arrange | Dirty::hit_test |
               Dirty::semantics | Dirty::accessibility);
}

void Control::set_padding(Insets padding) {
    require_mutable();
    validate_insets(padding, "GUI.Forms padding must be finite and nonnegative");
    if (padding_ == padding) return;
    padding_ = padding;
    invalidate(invalidation::bounds);
}

void Control::set_dock(DockStyle dock) {
    require_mutable();
    if (!valid_dock(dock)) {
        throw std::invalid_argument("GUI.Forms DockStyle value is invalid");
    }
    if (dock_ == dock) return;
    dock_ = dock;
    anchor_reference_.reset();
    if (dock_ == DockStyle::none) {
        if (const Ptr owner = parent(); owner && owner->window_ != nullptr &&
            (owner->arranged_bounds_.width > 0.0 ||
             owner->arranged_bounds_.height > 0.0) &&
            (arranged_bounds_.width > 0.0 || arranged_bounds_.height > 0.0)) {
            anchor_reference_ = AnchorReference{
                arranged_bounds_,
                client_rect({owner->arranged_bounds_.width,
                             owner->arranged_bounds_.height}, owner->padding_)};
        }
    }
    invalidate(invalidation::bounds);
}

void Control::set_anchor(AnchorStyles anchor) {
    require_mutable();
    if (!valid_anchor(anchor)) {
        throw std::invalid_argument("GUI.Forms AnchorStyles contains unknown flags");
    }
    if (anchor_ == anchor) return;
    anchor_ = anchor;
    anchor_reference_.reset();
    if (dock_ == DockStyle::none) {
        if (const Ptr owner = parent(); owner && owner->window_ != nullptr &&
            (owner->arranged_bounds_.width > 0.0 ||
             owner->arranged_bounds_.height > 0.0) &&
            (arranged_bounds_.width > 0.0 || arranged_bounds_.height > 0.0)) {
            anchor_reference_ = AnchorReference{
                arranged_bounds_,
                client_rect({owner->arranged_bounds_.width,
                             owner->arranged_bounds_.height}, owner->padding_)};
        }
    }
    invalidate(invalidation::bounds);
}

Rect Control::arranged_bounds() const {
    if (window_) {
        window_->ensure_layout(true);
    }
    return arranged_bounds_;
}

Rect Control::absolute_bounds() const {
    if (window_) {
        window_->ensure_layout(true);
        return window_->absolute_bounds_of(*this);
    }
    Rect result = arranged_bounds_;
    for (auto ancestor = parent(); ancestor; ancestor = ancestor->parent()) {
        result.x += ancestor->arranged_bounds_.x;
        result.y += ancestor->arranged_bounds_.y;
    }
    return result;
}

void Control::set_visible(bool visible) {
    require_mutable();
    if (visible_ == visible) {
        return;
    }
    visible_ = visible;
    if (window_ && !visible) {
        window_->on_eligibility_changed(shared_from_this());
    }
    if (!is_alive()) {
        return;
    }
    invalidate_subtree(invalidation::visibility);
}

void Control::set_enabled(bool enabled) {
    require_mutable();
    if (enabled_ == enabled) {
        return;
    }
    enabled_ = enabled;
    if (window_ && !enabled) {
        window_->on_eligibility_changed(shared_from_this());
    }
    if (!is_alive()) {
        return;
    }
    invalidate_declared(invalidation::enabled);
}

void Control::set_focusable(bool focusable) {
    require_mutable();
    if (focusable_ == focusable) {
        return;
    }
    focusable_ = focusable;
    if (window_ && !focusable) {
        window_->on_eligibility_changed(shared_from_this());
    }
    if (!is_alive()) {
        return;
    }
    invalidate_declared(invalidation::focusability);
}

void Control::set_allow_drop(bool allow_drop) {
    require_mutable();
    if (allow_drop_ == allow_drop) {
        return;
    }
    allow_drop_ = allow_drop;
    if (window_ && !allow_drop) {
        window_->on_eligibility_changed(shared_from_this());
    }
    if (!is_alive()) {
        return;
    }
    invalidate(Dirty::semantics | Dirty::hit_test);
}

void Control::set_cursor(std::optional<CursorKind> cursor) {
    require_mutable();
    if (cursor_ == cursor) {
        return;
    }
    cursor_ = cursor;
    invalidate(Dirty::semantics);
}

void Control::set_accessible_name(std::string name) {
    require_mutable();
    if (accessible_name_ == name) return;
    accessible_name_ = std::move(name);
    invalidate(Dirty::semantics);
}

void Control::set_accessible_description(std::string description) {
    require_mutable();
    if (accessible_description_ == description) return;
    accessible_description_ = std::move(description);
    invalidate(Dirty::semantics);
}

CursorKind Control::effective_cursor() const noexcept {
    const Control* current = this;
    Ptr owner;
    while (current != nullptr) {
        if (current->cursor_.has_value()) {
            return *current->cursor_;
        }
        owner = current->parent_.lock();
        current = owner.get();
    }
    return CursorKind::arrow;
}

bool Control::effectively_visible() const noexcept {
    if (!visible_ || !is_alive()) {
        return false;
    }
    for (auto ancestor = parent(); ancestor; ancestor = ancestor->parent()) {
        if (!ancestor->visible_ || !ancestor->is_alive()) {
            return false;
        }
    }
    return true;
}

bool Control::effectively_enabled() const noexcept {
    if (!enabled_ || !is_alive()) {
        return false;
    }
    for (auto ancestor = parent(); ancestor; ancestor = ancestor->parent()) {
        if (!ancestor->enabled_ || !ancestor->is_alive()) {
            return false;
        }
    }
    return true;
}

bool Control::eligible_for_input() const noexcept {
    return window_ != nullptr && effectively_visible() && effectively_enabled();
}

void Control::set_pointer_capture(bool captured) {
    require_mutable();
    if (window_ == nullptr) {
        if (!captured) return;
        throw std::logic_error(
            "GUI.Forms pointer capture requires an attached control");
    }
    if (captured) {
        window_->capture_pointer(shared_from_this());
    } else if (window_->captured_control().get() == this) {
        window_->release_pointer();
    }
}

bool Control::has_pointer_capture() const noexcept {
    return window_ != nullptr && window_->captured_control().get() == this;
}

void Control::set_paint_plane(PaintPlane plane) {
    require_mutable();
    if (!is_valid_paint_plane(plane)) {
        throw std::invalid_argument("invalid paint plane");
    }
    if (paint_plane_ == plane) {
        return;
    }
    if (window_) {
        window_->change_paint_plane(*this, plane);
        return;
    }
    paint_plane_ = plane;
    display_chunk_.reset();
    invalidate(invalidation::paint_only);
}

std::optional<DisplayChunkInfo> Control::display_chunk_info() const noexcept {
    return display_chunk_ ? std::optional(display_chunk_->info()) : std::nullopt;
}

void Control::invalidate(Dirty requested_dirty) {
    require_mutable();
    if (requested_dirty == Dirty::none) {
        return;
    }
    if (has_dirty(requested_dirty, Dirty::measure)) {
        requested_dirty |= Dirty::arrange;
    }
    if (initialization_depth_ != 0) {
        pending_initialization_dirty_ |= requested_dirty;
        return;
    }
    if (window_) {
        window_->mark_dirty(*this, requested_dirty);
        return;
    }
    dirty_ |= requested_dirty;
    for (Control* current = this; current != nullptr;) {
        current->subtree_dirty_ |= requested_dirty;
        const auto visual_parent = current->parent_.lock();
        current = visual_parent.get();
    }
}

void Control::invalidate_subtree(Dirty requested_dirty) {
    require_mutable();
    if (requested_dirty == Dirty::none) {
        return;
    }
    if (has_dirty(requested_dirty, Dirty::measure)) {
        requested_dirty |= Dirty::arrange;
    }
    if (initialization_depth_ != 0) {
        pending_initialization_dirty_ |= requested_dirty;
        pending_initialization_subtree_ = true;
        return;
    }
    if (window_) {
        window_->mark_subtree_dirty(*this, requested_dirty);
        return;
    }
    std::function<void(Control&)> apply = [&](Control& control) {
        control.dirty_ |= requested_dirty;
        control.subtree_dirty_ |= requested_dirty;
        for (const auto& child : control.children_) {
            apply(*child);
        }
    };
    apply(*this);
    for (auto visual_parent = parent(); visual_parent;
         visual_parent = visual_parent->parent()) {
        visual_parent->subtree_dirty_ |= requested_dirty;
    }
}

void Control::invalidate_declared(Dirty declared_effects) {
    require_mutable();
    if (declared_effects != Dirty::none) {
        invalidate(declared_effects);
        return;
    }
    if (window_) {
        window_->metrics_.record_undeclared_mutation();
    }
#ifndef NDEBUG
    throw std::logic_error("GUI.Forms mutation has no declared invalidation effects");
#else
    invalidate_subtree(invalidation::conservative_subtree);
#endif
}

void Control::begin_init() {
    require_mutable();
    ++initialization_depth_;
}

void Control::end_init() {
    require_mutable();
    if (initialization_depth_ == 0) {
        throw std::logic_error("GUI.Forms EndInit has no matching BeginInit");
    }
    --initialization_depth_;
    if (initialization_depth_ != 0) {
        return;
    }

    const Dirty pending = std::exchange(pending_initialization_dirty_, Dirty::none);
    const bool subtree = std::exchange(pending_initialization_subtree_, false);
    if (pending != Dirty::none) {
        if (subtree) {
            invalidate_subtree(pending);
        } else {
            invalidate(pending);
        }
    }
    initialization_completed_.emit(pending, subtree);
}

Size Control::measure(Size available) {
    return {std::min(requested_bounds_.width, available.width),
            std::min(requested_bounds_.height, available.height)};
}

void Control::arrange(Rect final_bounds) {
    arrange_self(final_bounds);
    if (children_.empty()) return;

    const Rect client = client_rect({final_bounds.width, final_bounds.height},
                                    padding_);
    Rect remaining = client;
    // Child index zero is topmost. The vector stores that control last, so
    // reverse traversal gives WinForms-style z-order-sensitive docking.
    for (auto iterator = children_.rbegin(); iterator != children_.rend();
         ++iterator) {
        const Ptr& child = *iterator;
        if (!child || !child->is_alive() || !child->visible_ ||
            child->dock_ == DockStyle::none) {
            continue;
        }
        const Size desired = child->measure({remaining.width, remaining.height});
        const double width = child->requested_bounds_.width > 0.0
            ? child->requested_bounds_.width : desired.width;
        const double height = child->requested_bounds_.height > 0.0
            ? child->requested_bounds_.height : desired.height;
        Rect slot = remaining;
        switch (child->dock_) {
        case DockStyle::top: {
            const double extent = std::clamp(height, 0.0, remaining.height);
            slot.height = extent;
            remaining.y += extent;
            remaining.height -= extent;
            break;
        }
        case DockStyle::bottom: {
            const double extent = std::clamp(height, 0.0, remaining.height);
            slot.y = remaining.y + remaining.height - extent;
            slot.height = extent;
            remaining.height -= extent;
            break;
        }
        case DockStyle::left: {
            const double extent = std::clamp(width, 0.0, remaining.width);
            slot.width = extent;
            remaining.x += extent;
            remaining.width -= extent;
            break;
        }
        case DockStyle::right: {
            const double extent = std::clamp(width, 0.0, remaining.width);
            slot.x = remaining.x + remaining.width - extent;
            slot.width = extent;
            remaining.width -= extent;
            break;
        }
        case DockStyle::fill:
            remaining = {remaining.x, remaining.y, 0.0, 0.0};
            break;
        case DockStyle::none:
            break;
        }
        child->anchor_reference_.reset();
        set_child_layout(child, slot);
    }

    for (const Ptr& child : children_) {
        if (!child || !child->is_alive() || !child->visible_ ||
            child->dock_ != DockStyle::none) {
            continue;
        }
        if (!child->anchor_reference_) {
            child->anchor_reference_ = AnchorReference{
                child->requested_bounds_, client};
        }
        set_child_layout(
            child, anchored_bounds(child->anchor_reference_->bounds,
                                   child->anchor_reference_->client,
                                   client, child->anchor_));
    }
}

void Control::on_paint(Painter&, Rect) {}

bool Control::hit_test_local(Point local_point) const {
    return Rect{0.0, 0.0, arranged_bounds_.width, arranged_bounds_.height}.contains(local_point);
}

void Control::on_pointer_preview(PointerEvent&) {}
void Control::on_pointer(PointerEvent&) {}
void Control::on_pointer_bubble(PointerEvent&) {}
void Control::on_key_preview(KeyEvent&) {}
void Control::on_key(KeyEvent&) {}
void Control::on_key_bubble(KeyEvent&) {}
void Control::on_text_input(TextInputEvent&) {}
void Control::on_frame(FrameTime) {}
SemanticDescriptor Control::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.name = accessible_name_;
    descriptor.description = accessible_description_;
    return descriptor;
}

std::vector<SemanticNode> Control::semantic_virtual_children() const {
    return {};
}

bool Control::on_semantic_action(SemanticAction action, std::string_view) {
    if (action == SemanticAction::focus && window_ != nullptr && focusable_) {
        return window_->request_focus(shared_from_this());
    }
    return false;
}
bool Control::on_semantic_child_action(std::string_view, SemanticAction,
                                       std::string_view) {
    return false;
}
void Control::on_drag_preview(DragEvent&) {}
void Control::on_drag(DragEvent&) {}
void Control::on_drag_bubble(DragEvent&) {}
void Control::on_focus_changed(bool) {}
void Control::on_activate() {}
void Control::on_attached_to_window() {}
void Control::on_attachment_committed() noexcept {}
void Control::on_detached_from_window() noexcept {}

void Control::clear_dirty(Dirty cleared) noexcept {
    dirty_ = without_dirty(dirty_, cleared);
}

void Control::clear_subtree_dirty(Dirty cleared) noexcept {
    subtree_dirty_ = without_dirty(subtree_dirty_, cleared);
}

std::uint64_t Control::subtree_size() const noexcept {
    std::uint64_t result = 1;
    for (const Ptr& child : children_) {
        result += child->subtree_size();
    }
    return result;
}

void Control::require_mutable() const {
    if (!is_alive()) {
        throw std::logic_error("GUI.Forms cannot mutate a disposed control");
    }
    if (window_) {
        window_->require_ui_thread("control mutation");
    }
}

void Control::verify_dispose_thread() {
    if (lifecycle_notification_ ||
        (window_ != nullptr && window_->in_lifecycle_notification_)) {
        throw std::logic_error("GUI.Forms cannot dispose a control during lifecycle notification");
    }
    if (window_) {
        window_->require_ui_thread("control disposal");
    }
}

void Control::on_dispose() noexcept {
    initialization_depth_ = 0;
    pending_initialization_dirty_ = Dirty::none;
    pending_initialization_subtree_ = false;
    tag_.reset();
    Ptr self = weak_from_this().lock();
    if (window_ && self) {
        window_->dispose_subtree(self);
    } else if (auto visual_parent = parent_.lock()) {
        const auto found = std::find_if(visual_parent->children_.begin(),
                                        visual_parent->children_.end(),
                                        [this](const Ptr& candidate) {
                                            return candidate.get() == this;
                                        });
        if (found != visual_parent->children_.end()) {
            visual_parent->children_.erase(found);
        }
        parent_.reset();
    }

    auto visual_children = std::move(children_);
    children_.clear();
    display_chunk_.reset();
    for (const auto& child : visual_children) {
        child->parent_.reset();
        child->dispose();
    }
}

} // namespace gui_forms
