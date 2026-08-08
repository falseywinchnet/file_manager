#include "gui_forms/window.hpp"
#include "gui_forms/live_surface.hpp"
#include "dispatcher_state.hpp"
#include "display_chunk.hpp"
#include "frame_scheduler.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <functional>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <unordered_set>

namespace gui_forms {

namespace detail {

class PopupAttachment final : public Revocable {
public:
    PopupAttachment(Window& window, Control::Ptr owner, Control::Ptr popup)
        : window_(&window), owner_(std::move(owner)), popup_(std::move(popup)) {}

    void disconnect() noexcept override {
        if (connected_ && window_ != nullptr) {
            window_->close_popup(*this);
        }
    }
    [[nodiscard]] bool connected() const noexcept override { return connected_; }
    [[nodiscard]] Control::Ptr owner() const noexcept { return owner_.lock(); }
    [[nodiscard]] Control::Ptr popup() const noexcept { return popup_.lock(); }
    [[nodiscard]] Event<>& closed() noexcept { return closed_; }
    void revoke(bool publish_closed = true) noexcept {
        connected_ = false;
        window_ = nullptr;
        if (publish_closed) closed_.emit();
        owner_.reset();
        popup_.reset();
    }
    void publish_closed() { closed_.emit(); }

private:
    Window* window_{};
    Control::WeakPtr owner_;
    Control::WeakPtr popup_;
    bool connected_{true};
    Event<> closed_;
};

class AcceleratorAttachment final : public Revocable {
public:
    AcceleratorAttachment(Window& window, Component& owner, KeyGesture gesture,
                          std::function<bool()> callback,
                          AcceleratorOptions options)
        : window_(&window), owner_(&owner), gesture_(gesture),
          callback_(std::move(callback)), options_(options) {}

    void disconnect() noexcept override {
        if (connected_ && window_) window_->close_accelerator(*this);
    }
    [[nodiscard]] bool connected() const noexcept override { return connected_; }
    [[nodiscard]] Component* owner() const noexcept { return owner_; }
    [[nodiscard]] KeyGesture gesture() const noexcept { return gesture_; }
    [[nodiscard]] AcceleratorOptions options() const noexcept { return options_; }
    bool invoke() { return callback_ && callback_(); }
    void revoke() noexcept {
        connected_ = false;
        window_ = nullptr;
        owner_ = nullptr;
        callback_ = {};
    }

private:
    Window* window_{};
    Component* owner_{};
    KeyGesture gesture_{};
    std::function<bool()> callback_;
    AcceleratorOptions options_{};
    bool connected_{true};
};

} // namespace detail

void PopupToken::disconnect() noexcept {
    if (attachment_) {
        attachment_->disconnect();
        attachment_.reset();
    }
}

bool PopupToken::connected() const noexcept {
    return attachment_ && attachment_->connected();
}

Event<>* PopupToken::closed_event() noexcept {
    return attachment_ ? &attachment_->closed() : nullptr;
}

void AcceleratorToken::disconnect() noexcept {
    if (attachment_) {
        attachment_->disconnect();
        attachment_.reset();
    }
}

bool AcceleratorToken::connected() const noexcept {
    return attachment_ && attachment_->connected();
}

namespace {

constexpr std::uint32_t maximum_layout_passes = 4;
constexpr std::uint32_t maximum_semantic_snapshot_passes = 4;

[[nodiscard]] std::optional<char32_t> physical_mnemonic(
    std::uint32_t physical_key) noexcept {
    // USB HID keyboard usages are contiguous for A-Z and for 1-9,0.
    if (physical_key >= 0x04U && physical_key <= 0x1DU) {
        return U'a' + static_cast<char32_t>(physical_key - 0x04U);
    }
    if (physical_key >= 0x1EU && physical_key <= 0x26U) {
        return U'1' + static_cast<char32_t>(physical_key - 0x1EU);
    }
    if (physical_key == 0x27U) return U'0';
    return std::nullopt;
}

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

[[nodiscard]] bool current_semantic_identity(
    const Control::Ptr& control, const Control* expected_parent,
    const Window* owner) noexcept {
    return control && control->is_alive() &&
           control->attached_window() == owner &&
           control->parent().get() == expected_parent;
}

void append_semantic_nodes(const Control::Ptr& control,
                           const Control* expected_parent,
                           const Window* owner,
                           const Control::Ptr& focused,
                           std::vector<SemanticNode>& destination,
                           std::size_t& count) {
    if (!current_semantic_identity(control, expected_parent, owner) ||
        !control->effectively_visible()) return;
    SemanticDescriptor descriptor = control->semantic_descriptor();
    if (!current_semantic_identity(control, expected_parent, owner)) return;
    control->apply_provider_semantics(descriptor);
    std::vector<SemanticNode> descendants;
    if (!descriptor.exposed || descriptor.include_descendants) {
        const std::vector<Control::Ptr> children(control->children().begin(),
                                                 control->children().end());
        for (const Control::Ptr& child : children) {
            if (!current_semantic_identity(child, control.get(), owner)) continue;
            append_semantic_nodes(child, control.get(), owner, focused,
                                  descendants, count);
        }
        if (!current_semantic_identity(control, expected_parent, owner)) return;
        std::vector<SemanticNode> virtual_children =
            control->semantic_virtual_children();
        if (!current_semantic_identity(control, expected_parent, owner)) return;
        count += virtual_children.size();
        descendants.insert(descendants.end(),
                            std::make_move_iterator(virtual_children.begin()),
                            std::make_move_iterator(virtual_children.end()));
    }
    if (!descriptor.exposed) {
        destination.insert(destination.end(),
                           std::make_move_iterator(descendants.begin()),
                           std::make_move_iterator(descendants.end()));
        return;
    }
    SemanticNode node;
    node.runtime_id = control->runtime_id().value;
    node.stable_id = std::string(control->stable_id().value());
    node.role = descriptor.role;
    node.name = descriptor.name;
    node.value = descriptor.value;
    node.description = descriptor.description;
    node.numeric_value = descriptor.numeric_value;
    node.minimum_value = descriptor.minimum_value;
    node.maximum_value = descriptor.maximum_value;
    node.bounds = control->absolute_bounds();
    if (!current_semantic_identity(control, expected_parent, owner)) return;
    node.states |= descriptor.states;
    if (control->effectively_enabled()) node.states |= SemanticState::enabled;
    node.states |= SemanticState::visible;
    if (control->focusable()) node.states |= SemanticState::focusable;
    if (control == focused) node.states |= SemanticState::focused;
    node.actions = descriptor.actions;
    node.children = std::move(descendants);
    destination.push_back(std::move(node));
    ++count;
}

bool dispatch_semantic_child_action(Control::Ptr control,
                                    std::string_view stable_id,
                                    SemanticAction action,
                                    std::string_view value) {
    if (!control || !control->effectively_visible() ||
        !control->effectively_enabled()) return false;
    Window* const owner = control->attached_window();
    if (control->on_semantic_child_action(stable_id, action, value)) return true;
    if (!control->is_alive() || control->attached_window() != owner) return false;
    const std::vector<Control::Ptr> children(control->children().begin(),
                                             control->children().end());
    for (const Control::Ptr& child : children) {
        if (!child || !child->is_alive() || child->parent() != control ||
            child->attached_window() != owner) {
            continue;
        }
        if (dispatch_semantic_child_action(child, stable_id, action, value)) return true;
    }
    return false;
}

} // namespace

Window::Window(Control::Ptr root, Size client_size)
    : root_(std::move(root)), client_size_(client_size),
      theme_(default_theme()),
      lifetime_(std::make_shared<detail::WindowLifetime>()),
      ui_thread_(std::this_thread::get_id()),
      dispatcher_state_(std::make_shared<detail::DispatcherState>(ui_thread_)) {
    lifetime_->window = this;
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

ImageLoadResult Window::load_bgra32_premultiplied(
    std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes,
    std::span<const std::byte> pixels) {
    require_ui_thread("BGRA resource load");
    return image_resources_.load_bgra32_premultiplied(
        width, height, row_bytes, pixels);
}

ImageLoadResult Window::replace_png(ImageId image,
                                    std::span<const std::byte> encoded) {
    require_ui_thread("PNG resource replacement");
    ImageLoadResult result = image_resources_.replace_png(image, encoded);
    if (result) {
        add_damage_all_planes({0.0, 0.0, client_size_.width, client_size_.height});
        touch_paint();
        paint_dirty_ = true;
    }
    return result;
}

ImageLoadResult Window::replace_bgra32_premultiplied(
    ImageId image, std::uint32_t width, std::uint32_t height,
    std::uint64_t row_bytes, std::span<const std::byte> pixels,
    Control& consumer) {
    require_ui_thread("scoped BGRA resource replacement");
    if (consumer.window_ != this) {
        throw std::invalid_argument(
            "scoped BGRA replacement requires an attached consumer");
    }
    ImageLoadResult result = image_resources_.replace_bgra32_premultiplied(
        image, width, height, row_bytes, pixels);
    if (result) {
        mark_dirty(consumer, Dirty::paint | Dirty::semantics);
    }
    return result;
}

ImageLoadResult Window::update_bgra32_premultiplied(
    ImageId image, std::uint32_t width, std::uint32_t height,
    std::uint64_t row_bytes, std::span<const std::byte> pixels,
    Control& consumer) {
    require_ui_thread("scoped live BGRA resource update");
    if (consumer.window_ != this) {
        throw std::invalid_argument(
            "scoped live BGRA update requires an attached consumer");
    }
    ImageLoadResult result = image_resources_.update_bgra32_premultiplied(
        image, width, height, row_bytes, pixels);
    if (result) {
        mark_dirty(consumer, Dirty::paint | Dirty::semantics);
    }
    return result;
}

ImageLoadResult Window::patch_bgra32_premultiplied(
    ImageId image, std::uint32_t x, std::uint32_t y,
    std::uint32_t width, std::uint32_t height,
    std::uint64_t source_row_bytes, std::span<const std::byte> pixels,
    Control& consumer, Rect local_damage) {
    require_ui_thread("scoped BGRA resource patch");
    if (consumer.window_ != this) {
        throw std::invalid_argument(
            "scoped BGRA patch requires an attached consumer");
    }
    ImageLoadResult result = image_resources_.patch_bgra32_premultiplied(
        image, x, y, width, height, source_row_bytes, pixels);
    if (result) {
        mark_paint_dirty(consumer, local_damage);
    }
    return result;
}

ImageLoadResult Window::replace_png(ImageId image,
                                    std::span<const std::byte> encoded,
                                    Control& consumer) {
    require_ui_thread("scoped PNG resource replacement");
    if (consumer.window_ != this) {
        throw std::invalid_argument(
            "scoped PNG replacement requires an attached consumer");
    }
    ImageLoadResult result = image_resources_.replace_png(image, encoded);
    if (result) {
        mark_dirty(consumer, Dirty::paint | Dirty::semantics);
    }
    return result;
}

bool Window::remove_image(ImageId image) {
    require_ui_thread("PNG resource removal");
    if (!image_resources_.remove(image)) {
        return false;
    }
    // The unscoped removal API cannot know which retained display chunks
    // reference this generational ID. Conservatively retire all cached paint
    // chunks so a dead ImageId is never replayed into any renderer. Scoped
    // replacement APIs remain available when the caller can name a consumer.
    mark_subtree_dirty(*root_, Dirty::paint | Dirty::semantics);
    return true;
}

Window::~Window() {
    paint_wake_handler_ = {};
    paint_wake_pending_ = false;
    abandon_deferred_input();
    shutdown_dispatcher();
    while (!accelerators_.empty()) {
        accelerators_.back()->disconnect();
    }
    while (!popups_.empty()) {
        popups_.back()->disconnect();
    }
    focus_scopes_.clear();
    for (const auto& request : frame_requests_) {
        request->disconnect();
    }
    frame_requests_.clear();
    update_frame_schedule_metrics();
    if (root_) {
        detach_subtree(root_);
    }
    lifetime_->window = nullptr;
}

AcceleratorToken Window::register_accelerator(
    Component& owner, KeyGesture gesture, std::function<bool()> callback,
    AcceleratorOptions options) {
    require_ui_thread("accelerator registration");
    if (!owner.is_alive() || gesture.physical_key == 0U || !callback) {
        throw std::invalid_argument(
            "accelerator requires a live owner, physical key, and callback");
    }
    auto attachment = std::make_shared<detail::AcceleratorAttachment>(
        *this, owner, gesture, std::move(callback), options);
    accelerators_.push_back(attachment);
    owner.own_revocable(attachment);
    return AcceleratorToken(std::move(attachment));
}

void Window::close_accelerator(
    detail::AcceleratorAttachment& accelerator) noexcept {
    const auto found = std::find_if(accelerators_.begin(), accelerators_.end(),
        [&accelerator](const auto& candidate) {
            return candidate.get() == &accelerator;
        });
    if (found == accelerators_.end()) return;
    (*found)->revoke();
    accelerators_.erase(found);
}

bool Window::dispatch_accelerator(const KeyEvent& event, bool preemptive) {
    if (event.action != KeyAction::down) return false;
    const auto snapshot = accelerators_;
    for (auto iterator = snapshot.rbegin(); iterator != snapshot.rend(); ++iterator) {
        const auto& accelerator = *iterator;
        if (!accelerator || !accelerator->connected() ||
            accelerator->options().before_focused_route != preemptive ||
            accelerator->gesture().physical_key != event.physical_key ||
            accelerator->gesture().modifiers != event.modifiers) continue;
        Component* owner = accelerator->owner();
        if (!owner || !owner->is_alive()) continue;
        if (accelerator->invoke()) return true;
    }
    return false;
}

PopupToken Window::open_popup(const Control::Ptr& owner,
                              const Control::Ptr& popup,
                              PopupOptions options) {
    require_ui_thread("popup attachment");
    const bool owner_available = owner && owner->window_ == this &&
        owner->is_alive() && owner->effectively_visible() &&
        (!options.require_enabled_owner || owner->effectively_enabled());
    if (!owner_available) {
        throw std::logic_error(
            options.require_enabled_owner
                ? "GUI.Forms popup requires an enabled, visible attached owner"
                : "GUI.Forms passive popup requires a visible attached owner");
    }
    if (!popup || popup->window_ != nullptr || popup->parent() || !popup->is_alive()) {
        throw std::logic_error("GUI.Forms popup must be a live detached root");
    }
    // Popups are window overlays, not children in the consumer's layout
    // vocabulary. Attaching one to root_ lets a TableLayoutPanel, Flow panel,
    // or docking root assign it an application cell and silently collapse the
    // overlay. Keep it as a separate retained root while still registering the
    // subtree with this Window for lifetime, focus, semantics, and dispatch.
    attach_subtree(popup, {});
    auto attachment = std::make_shared<detail::PopupAttachment>(*this, owner, popup);
    popups_.push_back(attachment);
    owner->own_revocable(attachment);
    return PopupToken(std::move(attachment));
}

void Window::close_popup(detail::PopupAttachment& popup) noexcept {
    const auto found = std::find_if(popups_.begin(), popups_.end(),
        [&popup](const auto& candidate) { return candidate.get() == &popup; });
    if (found == popups_.end()) {
        popup.revoke();
        return;
    }
    const auto attachment = *found;
    const Control::Ptr owner = attachment->owner();
    const Control::Ptr overlay = attachment->popup();
    attachment->revoke(false);
    popups_.erase(found);
    if (overlay && !overlay->parent() && overlay->window_ == this &&
        overlay != root_ && !in_lifecycle_notification_) {
        try {
            detach_subtree(overlay);
        } catch (...) {
        }
    }
    const auto publish_closed = [attachment] {
        attachment->publish_closed();
    };
    if (owner && owner->is_alive() && owner->window_ == this) {
        owner->publish_change(
            static_cast<const void*>(&attachment->closed()), publish_closed);
    } else {
        publish_closed();
    }
}

void Window::close_popups_for_subtree(const Control::Ptr& control) noexcept {
    std::vector<std::shared_ptr<detail::PopupAttachment>> closing;
    for (const auto& popup : popups_) {
        if (contains_control(control, popup->owner())) {
            closing.push_back(popup);
        }
    }
    for (const auto& popup : closing) {
        popup->disconnect();
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
    ++surface_epoch_;
    if (surface_epoch_ == 0U) ++surface_epoch_;
    add_damage_all_planes(old_bounds);
    mark_subtree_dirty(*root_, invalidation::bounds);
    for (const auto& popup : popups_) {
        if (const Control::Ptr overlay = popup->popup()) {
            mark_subtree_dirty(*overlay, invalidation::bounds);
        }
    }
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
    ++surface_epoch_;
    if (surface_epoch_ == 0U) ++surface_epoch_;
    mark_subtree_dirty(*root_, invalidation::conservative_subtree);
    for (const auto& popup : popups_) {
        if (const Control::Ptr overlay = popup->popup()) {
            mark_subtree_dirty(*overlay, invalidation::conservative_subtree);
        }
    }
}

void Window::set_presentation_settings(PresentationSettings settings) {
    require_ui_thread("presentation settings mutation");
    if (!std::isfinite(settings.text_scale) ||
        settings.text_scale < 0.5 || settings.text_scale > 4.0) {
        throw std::invalid_argument(
            "GUI.Forms text scale must be finite and between 0.5 and 4.0");
    }
    if (presentation_settings_ == settings) {
        return;
    }
    const bool text_scale_changed =
        presentation_settings_.text_scale != settings.text_scale;
    presentation_settings_ = settings;
    // Transient overlays own geometry resolved for the opening text metrics.
    // A text-scale transition dismisses them deterministically; reopening
    // rebuilds rows, hit regions, focus scope, and screen-edge placement from
    // the new logical metrics.
    if (text_scale_changed) {
        while (!popups_.empty()) {
            popups_.back()->disconnect();
        }
    }
    mark_subtree_dirty(*root_, invalidation::conservative_subtree);
    for (const auto& popup : popups_) {
        if (const Control::Ptr overlay = popup->popup()) {
            mark_subtree_dirty(*overlay, invalidation::conservative_subtree);
        }
    }
    presentation_changed_.emit(presentation_settings_);
}

void Window::set_theme(std::shared_ptr<const Theme> theme) {
    require_ui_thread("theme mutation");
    if (!theme) throw std::invalid_argument("window theme may not be null");
    if (theme_ == theme) return;
    theme_ = std::move(theme);
    // Themes include structural spacing/geometry/type tokens as well as paint
    // recipes, so replacement must remeasure and re-hit-test inherited content.
    mark_subtree_dirty(*root_, invalidation::conservative_subtree);
    for (const auto& popup : popups_) {
        if (const Control::Ptr overlay = popup->popup()) {
            mark_subtree_dirty(*overlay,
                               Dirty::style | Dirty::paint | Dirty::semantics);
        }
    }
    theme_changed_.emit(*theme_);
}

void Window::set_active(bool active) {
    require_ui_thread("activation mutation");
    if (active_ == active) return;
    active_ = active;
    mark_subtree_dirty(*root_, Dirty::style | Dirty::paint | Dirty::semantics);
    for (const auto& popup : popups_) {
        if (const Control::Ptr overlay = popup->popup()) {
            mark_subtree_dirty(*overlay,
                               Dirty::style | Dirty::paint | Dirty::semantics);
        }
    }
    active_changed_.emit(active_);
}

void Window::set_text_scale(double text_scale) {
    PresentationSettings settings = presentation_settings_;
    settings.text_scale = text_scale;
    set_presentation_settings(settings);
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

std::optional<PaintReceipt> Window::paint(Painter& painter,
                                          Rect requested_damage) {
    require_ui_thread("paint");
    if (in_paint_) {
        // Refresh/Update/native paint recursion requests a later pass. It must
        // never re-enter application painting while the exclusive lease is held.
        ++reentrant_paint_requests_deferred_;
        dirty_after_render_ = true;
        paint_dirty_ = true;
        paint_lease_state_ = PaintLeaseState::rendering_dirty;
        add_damage_all_planes({0.0, 0.0, client_size_.width, client_size_.height});
        request_paint_wake();
        return std::nullopt;
    }
    if (occluded_) {
        paint_lease_state_ = PaintLeaseState::occluded_dirty;
        return std::nullopt;
    }
    ensure_layout(true);

    if (!root_->is_alive()) {
        return std::nullopt;
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
        return std::nullopt;
    }

    struct ControlCheckpoint final {
        Control::Ptr control;
        std::shared_ptr<const detail::DisplayChunk> chunk;
    };
    std::vector<ControlCheckpoint> checkpoints;
    const auto checkpoint_tree = [&checkpoints](const Control::Ptr& root) {
        const auto visit = [&checkpoints](const auto& self,
                                         const Control::Ptr& control) -> void {
            if (!control) return;
            checkpoints.push_back({control, control->display_chunk_});
            for (const Control::Ptr& child : control->children_) self(self, child);
        };
        visit(visit, root);
    };
    checkpoint_tree(root_);
    for (const auto& popup : popups_) checkpoint_tree(popup->popup());

    const std::uint64_t lease_revision = content_revision_;
    const std::uint64_t lease_epoch = surface_epoch_;
    const std::uint64_t generation_before = display_generation_;
    ++paint_leases_started_;
    in_paint_ = true;
    dirty_after_render_ = false;
    paint_lease_state_ = PaintLeaseState::rendering;
    std::uint64_t visited_nodes = 0;
    std::uint64_t painted_controls = 0;
    std::uint64_t consumed_invalidations = 0;
    std::uint64_t chunks_rebuilt = 0;
    std::uint64_t chunks_reused = 0;
    std::uint64_t commands_replayed = 0;
    const auto abandon = [&]() {
        for (const ControlCheckpoint& checkpoint : checkpoints) {
            if (!checkpoint.control || !checkpoint.control->is_alive()) continue;
            checkpoint.control->display_chunk_ = checkpoint.chunk;
            checkpoint.control->dirty_ |= Dirty::paint;
        }
        if (root_ && root_->is_alive()) {
            static_cast<void>(recompute_subtree_dirty(root_));
        }
        display_generation_ = generation_before;
        add_damage_all_planes(paint_bounds);
        paint_dirty_ = true;
        dirty_after_render_ = true;
        in_paint_ = false;
        ++paint_leases_abandoned_;
        update_paint_lease_state();
        update_display_cache_metrics();
        if (!root_ || !root_->is_alive()) {
            abandon_deferred_input();
        } else {
            schedule_deferred_input_drain();
        }
    };

    try {
        // Application callbacks and retained chunk rebuilding record into a
        // complete candidate command list. No candidate command reaches the
        // host raster until every callback has returned successfully.
        detail::RecordingPainter candidate;
        // A top-level retained surface owns an explicit backplane even when
        // its application root is a transparent layout container. Replaying
        // only children cannot erase pixels formerly occupied by an overlay
        // in the gaps between those children. Resolve the immutable Window
        // recipe in full-window coordinates, then clip it to this transaction
        // so partial gradient repainting never shifts its authored geometry.
        ControlVisualContext backplane_context;
        backplane_context.surface = active_
            ? ControlSurfaceState::normal
            : ControlSurfaceState::deactivated;
        backplane_context.high_contrast = presentation_settings_.high_contrast;
        candidate.save();
        candidate.clip_rect(paint_bounds);
        paint_surface_material(
            candidate, window_bounds,
            theme_->resolve(ControlVisualRole::window,
                            backplane_context).material);
        candidate.restore();
        std::vector<Control::Ptr> popup_roots;
        popup_roots.reserve(popups_.size());
        for (const auto& popup : popups_) popup_roots.push_back(popup->popup());
        for (std::size_t index = 0; index < paint_plane_count; ++index) {
            const PaintPlane plane = static_cast<PaintPlane>(index);
            Rect plane_bounds = paint_bounds;
            if (requested_damage.empty()) {
                plane_bounds = Rect::intersection(plane_damage_[index].bounds(),
                                                  window_bounds);
            }
            if (plane_bounds.empty()) continue;
            paint_recursive(root_, candidate, plane_bounds, plane, visited_nodes,
                            painted_controls, consumed_invalidations, chunks_rebuilt,
                            chunks_reused, commands_replayed);
            // Window-owned popup roots are composited after application content
            // in opening order, independent of consumer layout.
            for (const Control::Ptr& overlay : popup_roots) {
                if (overlay && overlay->is_alive() && overlay->window_ == this) {
                    paint_recursive(overlay, candidate, plane_bounds, plane,
                                    visited_nodes, painted_controls,
                                    consumed_invalidations, chunks_rebuilt,
                                    chunks_reused, commands_replayed);
                }
            }
        }
        const auto transaction = candidate.finish(
            display_generation_, PaintPlane::control, window_bounds);
        if (lease_epoch != surface_epoch_ || !root_->is_alive()) {
            abandon();
            return std::nullopt;
        }
        static_cast<void>(detail::replay_display_chunk(*transaction, painter));
        // A backend painter may enter a native callback while replaying. A
        // resize, scale transition, or owner retirement at that boundary
        // invalidates the candidate even though every draw command returned.
        // The host receives no receipt and therefore cannot publish it.
        if (lease_epoch != surface_epoch_ || !root_->is_alive()) {
            abandon();
            return std::nullopt;
        }
    } catch (...) {
        abandon();
        throw;
    }

    in_paint_ = false;
    ++paint_leases_completed_;
    rendered_revision_ = lease_revision;

    // Damage that arrived while callbacks were running belongs to the next
    // revision. Never consume it as though it were part of this lease.
    const bool changed_while_rendering =
        dirty_after_render_ || content_revision_ != lease_revision;
    if (!changed_while_rendering) {
        for (std::size_t index = 0; index < paint_plane_count; ++index) {
            Rect plane_bounds = paint_bounds;
            if (requested_damage.empty()) {
                plane_bounds = Rect::intersection(plane_damage_[index].bounds(),
                                                  window_bounds);
            }
            const Rect pending_bounds = plane_damage_[index].bounds();
            if (pending_bounds.empty() ||
                Rect::intersection(plane_bounds, pending_bounds) == pending_bounds) {
                plane_damage_[index].clear();
            }
        }
    }

    const bool full_window = paint_bounds.area() >= window_bounds.area();
    metrics_.record_paint(visited_nodes, painted_controls, consumed_invalidations,
                          chunks_rebuilt, chunks_reused, commands_replayed,
                          paint_bounds.area(), full_window);
    update_display_cache_metrics();

    const bool popup_paint_dirty = std::any_of(
        popups_.begin(), popups_.end(), [](const auto& popup) {
            const Control::Ptr overlay = popup->popup();
            return overlay && has_dirty(overlay->subtree_dirty_, Dirty::paint);
        });
    paint_dirty_ = dirty_after_render_ ||
                   has_dirty(root_->subtree_dirty_, Dirty::paint) ||
                   popup_paint_dirty ||
                   std::any_of(plane_damage_.begin(), plane_damage_.end(),
                               [](const DamageRegion& damage) { return !damage.empty(); });
    update_paint_lease_state();
    schedule_deferred_input_drain();
    return PaintReceipt{lease_revision, lease_epoch};
}

bool Window::notify_presented(PaintReceipt receipt,
                              std::uint64_t duration_nanoseconds) {
    require_ui_thread("paint presentation release");
    const bool current_epoch = receipt.surface_epoch == surface_epoch_;
    const bool complete_revision = receipt.rendered_revision != 0U &&
        receipt.rendered_revision <= rendered_revision_;
    const bool monotonic = receipt.rendered_revision > presented_revision_;
    if (!current_epoch || !complete_revision || !monotonic) {
        ++presentation_receipts_rejected_;
        update_paint_lease_state();
        return false;
    }
    presented_revision_ = receipt.rendered_revision;
    ++presentation_receipts_accepted_;
    metrics_.record_present(duration_nanoseconds);
    update_paint_lease_state();
    return true;
}

void Window::notify_presented(std::uint64_t duration_nanoseconds) {
    static_cast<void>(notify_presented(
        PaintReceipt{rendered_revision_, surface_epoch_}, duration_nanoseconds));
}

PaintLeaseSnapshot Window::paint_lease_snapshot() const noexcept {
    return {paint_lease_state_, content_revision_, rendered_revision_,
            presented_revision_, surface_epoch_, paint_leases_started_,
            paint_leases_completed_, paint_leases_abandoned_,
            reentrant_paint_requests_deferred_, paint_wakes_queued_,
            paint_wakes_coalesced_, presentation_receipts_accepted_,
            presentation_receipts_rejected_, paint_wake_pending_,
            dirty_after_render_};
}

bool Window::queue_live_surface_presentation(
    const Control::Ptr& control, std::shared_ptr<LiveSurface> surface) {
    require_ui_thread("live-surface presentation");
    if (!control || !surface || !control->is_alive() ||
        control->window_ != this) {
        return false;
    }
    auto [entry, inserted] = live_surface_registrations_.try_emplace(
        control->runtime_id().value,
        LiveSurfaceRegistration{control, surface, 0U, 0U, false});
    if (!inserted && entry->second.surface != surface) {
        entry->second =
            LiveSurfaceRegistration{control, std::move(surface), 0U, 0U, false};
    } else {
        entry->second.control = control;
        entry->second.surface = std::move(surface);
    }
    return true;
}

std::vector<LiveSurfacePresentation>
Window::take_live_surface_presentations() {
    require_ui_thread("live-surface presentation drain");
    std::vector<LiveSurfacePresentation> result;
    result.reserve(live_surface_registrations_.size());

    // Direct live presentation is a fast path through retained composition,
    // not an always-on-top plane. Gather overlay ownership once, then subtract
    // those pixels from each live presentation. Uncovered spectrum/video pixels
    // therefore keep advancing while a menu is open without allowing the live
    // lane to paint over the menu.
    std::vector<Rect> overlay_rectangles;
    const auto collect_visible_overlays =
        [this, &overlay_rectangles](const auto& self,
                                    const Control::Ptr& candidate) -> void {
            if (!candidate || !candidate->is_alive() ||
                candidate->window_ != this || !candidate->effectively_visible()) {
                return;
            }
            if (candidate->paint_plane_ == PaintPlane::overlay) {
                const Rect bounds = Rect::intersection(
                    absolute_bounds_of(*candidate),
                    {0.0, 0.0, client_size_.width, client_size_.height});
                if (!bounds.empty()) overlay_rectangles.push_back(bounds);
            }
            for (const Control::Ptr& child : candidate->children_) {
                self(self, child);
            }
        };
    collect_visible_overlays(collect_visible_overlays, root_);
    for (const auto& popup : popups_) {
        if (popup) {
            collect_visible_overlays(collect_visible_overlays, popup->popup());
        }
    }

    const auto subtract_rectangle = [](const Rect& source, const Rect& cover) {
        std::vector<Rect> fragments;
        const Rect overlap = Rect::intersection(source, cover);
        if (overlap.empty()) {
            fragments.push_back(source);
            return fragments;
        }
        const double source_right = source.x + source.width;
        const double source_bottom = source.y + source.height;
        const double overlap_right = overlap.x + overlap.width;
        const double overlap_bottom = overlap.y + overlap.height;
        const Rect candidates[] = {
            {source.x, source.y, source.width, overlap.y - source.y},
            {source.x, overlap_bottom, source.width,
             source_bottom - overlap_bottom},
            {source.x, overlap.y, overlap.x - source.x, overlap.height},
            {overlap_right, overlap.y, source_right - overlap_right,
             overlap.height},
        };
        for (const Rect& candidate : candidates) {
            if (!candidate.empty()) fragments.push_back(candidate);
        }
        return fragments;
    };
    for (auto iterator = live_surface_registrations_.begin();
         iterator != live_surface_registrations_.end();) {
        const Control::Ptr control = iterator->second.control.lock();
        if (!control || !control->is_alive() || control->window_ != this ||
            !iterator->second.surface) {
            iterator = live_surface_registrations_.erase(iterator);
            continue;
        }
        if (occluded_ || !popups_.empty() || !control->visible_ ||
            !control->effectively_visible()) {
            ++iterator;
            continue;
        }

        const LiveSurfaceSnapshot snapshot = iterator->second.surface->snapshot();
        if (!snapshot.has_frame) {
            ++iterator;
            continue;
        }

        const Rect destination = absolute_bounds_of(*control);
        Rect clip = Rect::intersection(
            destination, {0.0, 0.0, client_size_.width, client_size_.height});
        bool valid = !destination.empty() && !clip.empty();
        for (auto ancestor = control->parent(); ancestor && valid && !clip.empty();
             ancestor = ancestor->parent()) {
            if (!ancestor->is_alive() || ancestor->window_ != this ||
                !ancestor->visible_) {
                valid = false;
                break;
            }
            const Rect ancestor_bounds = absolute_bounds_of(*ancestor);
            const Rect viewport = ancestor->child_viewport_rectangle();
            clip = Rect::intersection(
                clip, {ancestor_bounds.x + viewport.x,
                       ancestor_bounds.y + viewport.y,
                       viewport.width, viewport.height});
        }
        if (valid && !clip.empty()) {
            std::vector<Rect> clips{clip};
            bool clipped_by_overlay = false;
            for (const Rect& overlay : overlay_rectangles) {
                std::vector<Rect> remaining;
                for (const Rect& candidate : clips) {
                    if (!Rect::intersection(candidate, overlay).empty()) {
                        clipped_by_overlay = true;
                    }
                    auto fragments = subtract_rectangle(candidate, overlay);
                    remaining.insert(remaining.end(), fragments.begin(),
                                     fragments.end());
                }
                clips = std::move(remaining);
                if (clips.empty()) break;
            }
            const bool same_generation =
                iterator->second.sampled_epoch == snapshot.epoch &&
                iterator->second.sampled_generation ==
                    snapshot.published_generation;
            if (!same_generation || clipped_by_overlay ||
                iterator->second.sampled_with_overlay_clip) {
                for (const Rect& visible_clip : clips) {
                    result.push_back(LiveSurfacePresentation{
                        control->runtime_id(), iterator->second.surface,
                        destination, visible_clip});
                }
            }
            iterator->second.sampled_epoch = snapshot.epoch;
            iterator->second.sampled_generation = snapshot.published_generation;
            iterator->second.sampled_with_overlay_clip = clipped_by_overlay;
        }
        ++iterator;
    }
    return result;
}

void Window::set_paint_wake_handler(std::function<void()> wake) {
    require_ui_thread("paint wake-handler mutation");
    paint_wake_handler_ = std::move(wake);
    paint_wake_pending_ = false;
    if (paint_wake_handler_ && paint_dirty_ && !occluded_) {
        request_paint_wake();
    }
}

DamageRegion Window::take_damage() {
    require_ui_thread("damage mutation");
    // Hosts request damage before entering their native paint transaction.
    // Commit layout first so geometry changes discovered during arrange are
    // included in that same transaction instead of becoming stranded after
    // the host has already consumed the old region.
    ensure_layout(true);
    DamageRegion result;
    for (DamageRegion& plane : plane_damage_) {
        for (const Rect rect : plane.rectangles()) {
            result.add(rect);
        }
        plane = {};
    }
    acknowledge_paint_wake_if_damage_drained();
    return result;
}

DamageRegion Window::take_damage(PaintPlane plane) {
    require_ui_thread("plane damage mutation");
    if (!is_valid_paint_plane(plane)) {
        throw std::invalid_argument("invalid paint plane");
    }
    ensure_layout(true);
    const std::size_t index = paint_plane_index(plane);
    DamageRegion result = std::move(plane_damage_[index]);
    plane_damage_[index] = {};
    const bool popup_paint_dirty = std::any_of(
        popups_.begin(), popups_.end(), [](const auto& popup) {
            const Control::Ptr overlay = popup->popup();
            return overlay && has_dirty(overlay->subtree_dirty_, Dirty::paint);
        });
    paint_dirty_ = has_dirty(root_->subtree_dirty_, Dirty::paint) ||
                   popup_paint_dirty ||
                   std::any_of(plane_damage_.begin(), plane_damage_.end(),
                               [](const DamageRegion& damage) { return !damage.empty(); });
    acknowledge_paint_wake_if_damage_drained();
    return result;
}

bool Window::needs_frame() const noexcept {
    return paint_dirty_;
}

std::optional<FrameTime> Window::next_wake() const noexcept {
    std::optional<FrameTime> result;
    for (const auto& request : frame_requests_) {
        if (!request->connected()) {
            continue;
        }
        if (request->kind == detail::FrameRequestKind::ui_timer) {
            if (!result || request->deadline < *result) {
                result = request->deadline;
            }
            continue;
        }
        if (occluded_) {
            continue;
        }
        const auto target = request->target.lock();
        if (!target || !target->is_alive() || target->window_ != this ||
            !target->effectively_visible()) {
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

FrameRequestToken Window::schedule_ui_timer(
    Component& owner, FrameInterval interval, FrameTime first_deadline,
    std::function<void(FrameTime)> callback) {
    require_ui_thread("UI timer scheduling");
    if (!owner.is_alive()) {
        throw std::logic_error("GUI.Forms UI timer requires a live component owner");
    }
    if (interval < minimum_ui_timer_interval) {
        throw std::invalid_argument("GUI.Forms UI timer interval is below the bound");
    }
    if (!callback) {
        throw std::invalid_argument("GUI.Forms UI timer requires a callback");
    }
    compact_frame_requests();
    if (frame_requests_.size() >= maximum_scheduled_frame_requests) {
        throw std::length_error("GUI.Forms scheduled frame request limit reached");
    }
    auto request = std::make_shared<detail::ScheduledFrameRequest>(
        first_deadline, interval, std::move(callback));
    frame_requests_.push_back(request);
    owner.own_revocable(request);
    metrics_.record_frame_request();
    update_frame_schedule_metrics();
    return FrameRequestToken(request);
}

FramePollResult Window::poll_frame_schedule(FrameTime now) {
    require_ui_thread("frame schedule polling");
    if (in_frame_poll_) {
        metrics_.record_reentrant_frame_poll();
        FramePollResult deferred;
        deferred.reentrant_poll_deferred = true;
        deferred.damage_pending = needs_frame();
        deferred.suppressed_by_occlusion = occluded_;
        deferred.next_wake = next_wake();
        return deferred;
    }
    in_frame_poll_ = true;
    struct Reset final {
        bool& value;
        ~Reset() { value = false; }
    } reset{in_frame_poll_};
    compact_frame_requests();

    FramePollResult result;
    result.suppressed_by_occlusion = occluded_;
    if (occluded_) metrics_.record_occluded_frame_poll();
    std::unordered_set<std::uint64_t> invalidated_controls;
    std::unordered_set<std::uint64_t> frame_callbacks;
    std::unordered_set<std::uint64_t> faulted_controls;
    const auto requests = frame_requests_;
    for (const auto& request : requests) {
        if (!request->connected() || request->deadline > now) {
            continue;
        }
        if (request->kind == detail::FrameRequestKind::ui_timer) {
            ++result.ui_timer_ticks;
            const FrameInterval lateness = now - request->deadline;
            const auto skipped = lateness / request->interval;
            result.coalesced_requests += static_cast<std::uint64_t>(skipped);
            request->deadline += request->interval * (skipped + 1);
            metrics_.record_callback_emitted();
            const auto callback = request->callback;
            if (callback) {
                try {
                    callback(now);
                } catch (...) {
                    request->disconnect();
                    ++result.callback_faults;
                    metrics_.record_frame_callback_fault();
                }
            }
            continue;
        }
        if (occluded_) {
            result.suppressed_by_occlusion = true;
            continue;
        }
        const auto target = request->target.lock();
        if (!target || !target->is_alive() || target->window_ != this) {
            request->disconnect();
            continue;
        }
        if (!target->effectively_visible()) {
            if (request->kind == detail::FrameRequestKind::active_surface) {
                request->deadline = now + request->interval;
            }
            continue;
        }
        if (faulted_controls.contains(target->runtime_id().value)) {
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

        if (frame_callbacks.insert(target->runtime_id().value).second) {
            metrics_.record_callback_emitted();
            try {
                target->on_frame(now);
            } catch (...) {
                request->disconnect();
                faulted_controls.insert(target->runtime_id().value);
                ++result.callback_faults;
                metrics_.record_frame_callback_fault();
                continue;
            }
            if (!target->is_alive() || target->window_ != this) {
                continue;
            }
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
        if (paint_dirty_) request_paint_wake();
    }
    update_paint_lease_state();
    update_frame_schedule_metrics();
}

bool Window::check_access() const noexcept {
    return std::this_thread::get_id() == ui_thread_;
}

void Window::verify_access(std::string_view operation) {
    require_ui_thread(operation);
}

Control::Ptr Window::find(std::string_view stable_id) const {
    const auto found = stable_ids_.find(std::string(stable_id));
    return found == stable_ids_.end() ? Control::Ptr{} : found->second.lock();
}

Control::Ptr Window::hit_test(Point position) {
    require_ui_thread("hit test");
    for (std::uint32_t pass = 0U;
         pass < maximum_semantic_snapshot_passes; ++pass) {
        ensure_layout(true);
        const std::uint64_t generation = semantic_generation_;
        Control::Ptr target;
        std::vector<Control::Ptr> popup_roots;
        popup_roots.reserve(popups_.size());
        for (const auto& popup : popups_) popup_roots.push_back(popup->popup());
        for (auto popup = popup_roots.rbegin(); popup != popup_roots.rend();
             ++popup) {
            if (const Control::Ptr& overlay = *popup;
                overlay && overlay->is_alive() && overlay->window_ == this) {
                target = hit_test_recursive(overlay, position);
                if (target) break;
            }
        }
        if (!target) target = hit_test_recursive(root_, position);
        if (generation == semantic_generation_) return target;
        metrics_.record_callback_arbitration_retry(
            pass + 1U == maximum_semantic_snapshot_passes);
    }
    return {};
}

bool Window::request_focus(const Control::Ptr& control) {
    require_ui_thread("focus mutation");
    if (control && (!eligible(control) || !control->focusable_)) {
        return false;
    }
    if (control && !focus_allowed_by_active_scope(control)) {
        metrics_.record_focus_scope_rejection();
        return false;
    }
    Control::Ptr previous = focused_.lock();
    if (previous == control) {
        return true;
    }
    if (validating_) {
        ++validation_reentrant_requests_rejected_;
        return false;
    }
    if (previous && (!control || control->causes_validation())) {
        AutoValidate mode = AutoValidate::enable_prevent_focus_change;
        for (Control::Ptr current = previous; current; current = current->parent()) {
            const AutoValidate authored = current->authored_auto_validate();
            if (authored != AutoValidate::inherit) {
                mode = authored;
                break;
            }
        }
        if (mode != AutoValidate::disable) {
            const bool valid = validate_focus_transition(previous, control, mode);
            if (!valid && mode == AutoValidate::enable_prevent_focus_change &&
                focused_.lock() == previous && eligible(previous)) {
                ++validation_focus_moves_blocked_;
                return false;
            }
        }
    }
    // Validation handlers may dispose, detach, hide, or disable either end.
    // Re-evaluate both endpoints before publishing any focus notification.
    if (previous && (!previous->is_alive() || previous->attached_window() != this)) {
        previous.reset();
        focused_.reset();
    }
    if (control && (!eligible(control) || !control->focusable_ ||
                    !focus_allowed_by_active_scope(control))) {
        return false;
    }
    if (previous) {
        focused_.reset();
        previous->publish_change(
            static_cast<const void*>(&previous->focus_observed_),
            [this, previous] {
                if (!previous->is_alive() || previous->window_ != this) return;
                metrics_.record_callback_emitted();
                previous->on_focus_changed(false);
                if (previous->is_alive()) {
                    previous->focus_observed_.emit(false);
                }
                if (previous->is_alive()) {
                    previous->invalidate(invalidation::focus);
                }
            });
    }
    if (control && eligible(control) && control->focusable_) {
        focused_ = control;
        control->publish_change(
            static_cast<const void*>(&control->focus_observed_),
            [this, control] {
                if (!eligible(control)) return;
                metrics_.record_callback_emitted();
                control->on_focus_changed(true);
                if (control->is_alive()) control->focus_observed_.emit(true);
                if (eligible(control)) {
                    control->invalidate(invalidation::focus);
                } else {
                    focused_.reset();
                }
            });
    }
    metrics_.record_focus_transition();
    return true;
}

void Window::set_accept_button(const Control::Ptr& control) {
    require_ui_thread("accept button mutation");
    if (control && (control->attached_window() != this || !control->is_alive() ||
                    !control->supports_dialog_command())) {
        throw std::invalid_argument(
            "GUI.Forms accept button must be a live dialog command in its Window");
    }
    Control::Ptr existing = accept_button_.lock();
    if (existing == control) return;
    if (existing && existing->is_alive()) existing->notify_default(false);
    try {
        if (control) control->notify_default(true);
    } catch (...) {
        if (existing && existing->is_alive()) existing->notify_default(true);
        throw;
    }
    accept_button_ = control;
}

void Window::set_cancel_button(const Control::Ptr& control) {
    require_ui_thread("cancel button mutation");
    if (control && (control->attached_window() != this || !control->is_alive() ||
                    !control->supports_dialog_command())) {
        throw std::invalid_argument(
            "GUI.Forms cancel button must be a live dialog command in its Window");
    }
    if (control) {
        control->assign_cancel_dialog_result();
        if (control->attached_window() != this || !control->is_alive() ||
            !control->supports_dialog_command()) {
            throw std::logic_error(
                "GUI.Forms cancel button became unavailable during assignment");
        }
    }
    cancel_button_ = control;
}

DialogKeySnapshot Window::dialog_key_snapshot() const noexcept {
    return {mnemonic_attempts_, mnemonics_handled_, accept_attempts_,
            accept_handled_, cancel_attempts_, cancel_handled_,
            dialog_command_rejections_, mnemonic_candidates_,
            mnemonic_collisions_, mnemonic_cycles_};
}

void Window::set_dialog_result(DialogResult result) {
    require_ui_thread("dialog result mutation");
    switch (result) {
    case DialogResult::none:
    case DialogResult::ok:
    case DialogResult::cancel:
    case DialogResult::abort:
    case DialogResult::retry:
    case DialogResult::ignore:
    case DialogResult::yes:
    case DialogResult::no:
    case DialogResult::try_again:
    case DialogResult::continue_: break;
    default:
        throw std::invalid_argument("Window DialogResult is not a defined value");
    }
    if (dialog_result_ == result) return;
    dialog_result_ = result;
    dialog_result_changed_.emit(dialog_result_);
}

bool Window::move_focus_after(const Control::Ptr& origin) {
    require_ui_thread("mnemonic focus traversal");
    if (!origin || origin->attached_window() != this) return false;
    const Control::Ptr scope = active_focus_scope_root();
    const Control::Ptr traversal_root = scope ? scope : root_;
    if (!contains_control(traversal_root, origin)) return false;

    std::vector<Control::Ptr> ordered;
    std::function<void(const Control::Ptr&)> collect =
        [&](const Control::Ptr& control) {
            if (!control || !eligible(control)) return;
            ordered.push_back(control);
            std::vector<Control::Ptr> children(control->children().begin(),
                                               control->children().end());
            std::stable_sort(children.begin(), children.end(),
                [](const Control::Ptr& left, const Control::Ptr& right) {
                    return left->tab_index() < right->tab_index();
                });
            for (const Control::Ptr& child : children) collect(child);
        };
    collect(traversal_root);
    const auto found = std::find(ordered.begin(), ordered.end(), origin);
    if (found == ordered.end()) return false;
    for (auto current = std::next(found); current != ordered.end(); ++current) {
        if ((*current)->focusable() && (*current)->tab_stop() &&
            request_focus(*current)) return true;
    }
    return false;
}

bool Window::validate_command_activation(const Control::Ptr& destination) {
    require_ui_thread("dialog command activation");
    if (!destination || !eligible(destination) ||
        !focus_allowed_by_active_scope(destination)) return false;
    const Control::Ptr previous = focused_.lock();
    if (!previous || previous == destination ||
        !destination->causes_validation()) return true;
    if (validating_) {
        ++validation_reentrant_requests_rejected_;
        return false;
    }
    AutoValidate mode = AutoValidate::enable_prevent_focus_change;
    for (Control::Ptr current = previous; current; current = current->parent()) {
        const AutoValidate authored = current->authored_auto_validate();
        if (authored != AutoValidate::inherit) {
            mode = authored;
            break;
        }
    }
    if (mode == AutoValidate::disable) return true;
    const bool accepted = validate_focus_transition(previous, destination, mode);
    return accepted || mode == AutoValidate::enable_allow_focus_change;
}

bool Window::dispatch_mnemonic(char32_t character) {
    ++mnemonic_attempts_;
    const Control::Ptr scope = active_focus_scope_root();
    const Control::Ptr start = scope ? scope : root_;
    if (!start) return false;

    std::vector<Control::Ptr> candidates;
    std::function<void(const Control::Ptr&)> collect =
        [&](const Control::Ptr& control) {
            if (!control || !eligible(control) ||
                !contains_control(start, control)) {
                return;
            }
            if (control->mnemonic_matches(character)) {
                candidates.push_back(control);
            }
            std::vector<Control::Ptr> children(control->children().begin(),
                                               control->children().end());
            std::stable_sort(children.begin(), children.end(),
                [](const Control::Ptr& left, const Control::Ptr& right) {
                    if (!left) return false;
                    if (!right) return true;
                    return left->tab_index() < right->tab_index();
                });
            for (const Control::Ptr& child : children) collect(child);
        };
    collect(start);
    mnemonic_candidates_ += candidates.size();
    if (candidates.size() > 1U) ++mnemonic_collisions_;
    if (candidates.empty()) {
        mnemonic_cursor_.reset();
        mnemonic_cursor_character_ = 0;
        return false;
    }

    const char32_t folded = character >= U'A' && character <= U'Z'
        ? character + (U'a' - U'A') : character;
    std::size_t first{};
    if (candidates.size() > 1U && mnemonic_cursor_character_ == folded) {
        if (const Control::Ptr prior = mnemonic_cursor_.lock()) {
            const auto found = std::find(candidates.begin(), candidates.end(), prior);
            if (found != candidates.end()) {
                first = (static_cast<std::size_t>(
                    std::distance(candidates.begin(), found)) + 1U) %
                    candidates.size();
                ++mnemonic_cycles_;
            }
        }
    }

    for (std::size_t offset = 0U; offset < candidates.size(); ++offset) {
        const std::size_t index = (first + offset) % candidates.size();
        const Control::Ptr& candidate = candidates[index];
        if (!candidate || !eligible(candidate) ||
            !contains_control(start, candidate)) {
            continue;
        }
        if (candidate->process_mnemonic_self(character)) {
            mnemonic_cursor_ = candidate;
            mnemonic_cursor_character_ = folded;
            ++mnemonics_handled_;
            metrics_.record_activation();
            return true;
        }
    }
    return false;
}

bool Window::dispatch_dialog_button(bool accept) {
    std::uint64_t& attempts = accept ? accept_attempts_ : cancel_attempts_;
    std::uint64_t& handled = accept ? accept_handled_ : cancel_handled_;
    ++attempts;
    Control::Ptr target = accept ? accept_button_.lock() : cancel_button_.lock();
    if (!target || !eligible(target) || !focus_allowed_by_active_scope(target)) {
        return false;
    }
    if (!target->perform_dialog_command()) {
        ++dialog_command_rejections_;
        // A live target owned this dialog key even when its validation gate
        // rejected activation. Do not leak it to another mnemonic or host.
        return true;
    }
    ++handled;
    metrics_.record_activation();
    return true;
}

bool Window::validate_control(const Control::Ptr& control,
                              Control* destination, bool bulk) {
    require_ui_thread("control validation");
    if (!control || !eligible(control)) return true;
    if (validating_) {
        ++validation_reentrant_requests_rejected_;
        return false;
    }
    validating_ = true;
    struct Reset final {
        bool& value;
        ~Reset() { value = false; }
    } reset{validating_};
    ++validation_attempts_;
    if (bulk) ++validation_bulk_controls_visited_;
    const bool accepted = control->perform_validation(destination, bulk);
    if (accepted) ++validation_succeeded_;
    else ++validation_cancelled_;
    return accepted;
}

bool Window::validate_children(const Control::Ptr& container,
                               ValidationConstraints constraints) {
    require_ui_thread("child validation");
    if (!container || !eligible(container)) return true;
    if (validating_) {
        ++validation_reentrant_requests_rejected_;
        return false;
    }
    constexpr auto known = static_cast<std::uint8_t>(
        ValidationConstraints::immediate_children |
        ValidationConstraints::selectable |
        ValidationConstraints::enabled |
        ValidationConstraints::visible |
        ValidationConstraints::tab_stop);
    if ((static_cast<std::uint8_t>(constraints) & ~known) != 0U) {
        throw std::invalid_argument(
            "GUI.Forms ValidationConstraints contains unknown flags");
    }
    validating_ = true;
    struct Reset final {
        bool& value;
        ~Reset() { value = false; }
    } reset{validating_};

    bool accepted = true;
    std::function<void(const Control::Ptr&, bool)> visit =
        [&](const Control::Ptr& parent, bool direct) {
            const std::vector<Control::Ptr> children(parent->children().begin(),
                                                     parent->children().end());
            for (const Control::Ptr& child : children) {
                if (!child || !child->is_alive() ||
                    child->attached_window() != this ||
                    child->parent() != parent) continue;
                bool selected = true;
                if (has_validation_constraint(
                        constraints, ValidationConstraints::selectable)) {
                    selected = child->focusable() &&
                               child->has_style(ControlStyles::selectable);
                }
                if (has_validation_constraint(
                        constraints, ValidationConstraints::enabled)) {
                    selected = selected && child->effectively_enabled();
                }
                if (has_validation_constraint(
                        constraints, ValidationConstraints::visible)) {
                    selected = selected && child->effectively_visible();
                }
                if (has_validation_constraint(
                        constraints, ValidationConstraints::tab_stop)) {
                    selected = selected && child->tab_stop();
                }
                if (selected) {
                    ++validation_attempts_;
                    ++validation_bulk_controls_visited_;
                    const bool valid = child->perform_validation(container.get(), true);
                    if (valid) ++validation_succeeded_;
                    else ++validation_cancelled_;
                    accepted = valid && accepted;
                }
                if (!child->is_alive() || child->attached_window() != this ||
                    child->parent() != parent) {
                    continue;
                }
                if (!has_validation_constraint(
                        constraints, ValidationConstraints::immediate_children)) {
                    visit(child, false);
                }
            }
            static_cast<void>(direct);
        };
    visit(container, true);
    return accepted;
}

ValidationSnapshot Window::validation_snapshot() const noexcept {
    return {validation_attempts_, validation_succeeded_, validation_cancelled_,
            validation_focus_moves_blocked_,
            validation_reentrant_requests_rejected_,
            validation_bulk_controls_visited_, validating_};
}

bool Window::validate_focus_transition(const Control::Ptr& previous,
                                       const Control::Ptr& destination,
                                       AutoValidate mode) {
    std::unordered_set<const Control*> destination_ancestors;
    for (Control::Ptr current = destination; current; current = current->parent()) {
        destination_ancestors.insert(current.get());
    }
    validating_ = true;
    struct Reset final {
        bool& value;
        ~Reset() { value = false; }
    } reset{validating_};
    bool accepted = true;
    for (Control::Ptr current = previous;
         current && !destination_ancestors.contains(current.get());
         current = current->parent()) {
        if (!current->is_alive() || current->attached_window() != this) break;
        ++validation_attempts_;
        const bool valid = current->perform_validation(destination.get(), false);
        if (valid) ++validation_succeeded_;
        else ++validation_cancelled_;
        accepted = valid && accepted;
        if (!valid && mode == AutoValidate::enable_prevent_focus_change) break;
    }
    return accepted;
}

FocusScopeId Window::begin_focus_scope(const Control::Ptr& root,
                                       const Control::Ptr& preferred_focus,
                                       FocusScopeOptions options) {
    require_ui_thread("focus-scope entry");
    if (!root || !eligible(root)) {
        throw std::logic_error(
            "GUI.Forms focus scope requires an eligible attached root");
    }
    for (auto current = focus_scopes_.rbegin(); current != focus_scopes_.rend();
         ++current) {
        if (!current->active) {
            continue;
        }
        if (current->options.contain_focus &&
            !contains_control(current->root.lock(), root)) {
            throw std::logic_error(
                "GUI.Forms nested focus scope must remain inside its containing scope");
        }
        break;
    }
    if (focus_scope_depth() >= maximum_focus_scope_depth) {
        throw std::length_error("GUI.Forms focus-scope nesting limit reached");
    }
    if (preferred_focus &&
        (!contains_control(root, preferred_focus) || !eligible(preferred_focus) ||
         !preferred_focus->focusable_)) {
        throw std::invalid_argument(
            "GUI.Forms preferred focus must be an eligible focusable scope descendant");
    }

    FocusScopeState state;
    state.id = FocusScopeId{next_focus_scope_id_++};
    if (!state.id) {
        state.id = FocusScopeId{next_focus_scope_id_++};
    }
    state.root = root;
    state.previous_focus = focused_;
    state.stable_id = std::string(root->stable_id().value());
    state.root_id = root->runtime_id();
    state.options = options;
    focus_scopes_.push_back(state);

    const std::size_t depth = focus_scope_depth();
    metrics_.record_focus_scope_opened(depth);
    FocusScopeChange change;
    change.scope = state.id;
    change.root_id = state.root_id;
    change.stable_id = state.stable_id;
    change.depth = depth;
    change.opened = true;
    focus_scope_changed_.emit(change);

    const bool remains_active = std::any_of(
        focus_scopes_.begin(), focus_scopes_.end(),
        [id = state.id](const FocusScopeState& candidate) {
            return candidate.id == id && candidate.active;
        });
    if (!remains_active) {
        return state.id;
    }

    if (preferred_focus) {
        static_cast<void>(request_focus(preferred_focus));
    } else if (options.focus_first &&
               !contains_control(root, focused_.lock())) {
        static_cast<void>(move_focus(true));
    }
    return state.id;
}

bool Window::end_focus_scope(FocusScopeId scope,
                             FocusScopeCloseReason reason) {
    return end_focus_scope(scope, reason, {});
}

bool Window::end_focus_scope(FocusScopeId scope,
                             FocusScopeCloseReason reason,
                             const Control::Ptr& notification_owner) {
    require_ui_thread("focus-scope exit");
    const auto found = std::find_if(
        focus_scopes_.begin(), focus_scopes_.end(),
        [scope](const FocusScopeState& state) {
            return state.id == scope && state.active;
        });
    if (found == focus_scopes_.end()) {
        return false;
    }

    const FocusScopeId closed_id = found->id;
    const RuntimeId closed_root_id = found->root_id;
    const std::string closed_stable_id = found->stable_id;
    found->active = false;
    const bool closes_top = std::next(found) == focus_scopes_.end();

    bool restored = false;
    if (closes_top) {
        Control::Ptr restoration;
        bool restoration_requested = false;
        while (!focus_scopes_.empty() && !focus_scopes_.back().active) {
            const FocusScopeState& removed = focus_scopes_.back();
            if (removed.options.restore_focus) {
                restoration = removed.previous_focus.lock();
                restoration_requested = true;
            }
            focus_scopes_.pop_back();
        }

        if (restoration_requested) {
            if (restoration && eligible(restoration) && restoration->focusable_ &&
                focus_allowed_by_active_scope(restoration)) {
                restored = request_focus(restoration) &&
                           focused_.lock() == restoration;
            } else if (const Control::Ptr active_root = active_focus_scope_root()) {
                if (!contains_control(active_root, focused_.lock())) {
                    static_cast<void>(move_focus(true));
                }
            } else {
                static_cast<void>(request_focus({}));
            }
        } else if (!focus_allowed_by_active_scope(focused_.lock())) {
            static_cast<void>(move_focus(true));
        }
    }

    const std::size_t depth = focus_scope_depth();
    metrics_.record_focus_scope_closed(restored, depth);
    FocusScopeChange change;
    change.scope = closed_id;
    change.root_id = closed_root_id;
    change.stable_id = closed_stable_id;
    change.depth = depth;
    change.close_reason = reason;
    change.restored_focus = restored;
    if (notification_owner && notification_owner->is_alive() &&
        notification_owner->window_ == this) {
        // Scope closure is an identity-bearing stream, not a scalar property.
        // Preserve every nested scope transition rather than coalescing all
        // Window scope notifications under one Event address.
        const auto notification_key = std::make_shared<std::uint8_t>();
        notification_owner->publish_change(
            static_cast<const void*>(notification_key.get()),
            [this, notification_owner, notification_key, change] {
                if (!notification_owner->is_alive() ||
                    notification_owner->window_ != this) return;
                focus_scope_changed_.emit(change);
            });
    } else {
        focus_scope_changed_.emit(change);
    }
    return true;
}

std::size_t Window::focus_scope_depth() const noexcept {
    return static_cast<std::size_t>(std::count_if(
        focus_scopes_.begin(), focus_scopes_.end(),
        [](const FocusScopeState& state) { return state.active; }));
}

Control::Ptr Window::active_focus_scope_root() const noexcept {
    for (auto current = focus_scopes_.rbegin(); current != focus_scopes_.rend();
         ++current) {
        if (current->active) {
            return current->root.lock();
        }
    }
    return {};
}

bool Window::focus_allowed_by_active_scope(
    const Control::Ptr& control) const noexcept {
    for (auto current = focus_scopes_.rbegin(); current != focus_scopes_.rend();
         ++current) {
        if (!current->active) {
            continue;
        }
        if (!current->options.contain_focus || !control) {
            return true;
        }
        return contains_control(current->root.lock(), control);
    }
    return true;
}

std::vector<Control::Ptr> Window::focus_candidates(
    const Control::Ptr& scope_root) const {
    std::vector<Control::Ptr> result;
    std::function<void(const Control::Ptr&)> collect =
        [&](const Control::Ptr& control) {
            if (!control || !eligible(control)) {
                return;
            }
            if (control->focusable_ && control->tab_stop_) {
                result.push_back(control);
            }
            std::vector<Control::Ptr> ordered(control->children_.begin(),
                                              control->children_.end());
            std::stable_sort(ordered.begin(), ordered.end(),
                             [](const Control::Ptr& left,
                                const Control::Ptr& right) {
                                 return left->tab_index_ < right->tab_index_;
                             });
            for (const Control::Ptr& child : ordered) {
                collect(child);
            }
        };
    collect(scope_root);
    return result;
}

bool Window::move_focus(bool forward) {
    require_ui_thread("focus traversal");
    const Control::Ptr scope_root = active_focus_scope_root();
    std::vector<Control::Ptr> candidates =
        focus_candidates(scope_root ? scope_root : root_);
    if (candidates.empty()) {
        return false;
    }
    const Control::Ptr current = focused_.lock();
    const auto found = std::find(candidates.begin(), candidates.end(), current);
    std::size_t index = 0;
    if (found == candidates.end()) {
        index = forward ? 0U : candidates.size() - 1U;
    } else if (forward) {
        index = (static_cast<std::size_t>(found - candidates.begin()) + 1U) %
                candidates.size();
    } else {
        const std::size_t current_index =
            static_cast<std::size_t>(found - candidates.begin());
        index = current_index == 0U ? candidates.size() - 1U
                                   : current_index - 1U;
    }
    return request_focus(candidates[index]);
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
    const Control::Ptr publication_owner = control ? control : previous;
    if (publication_owner) {
        publication_owner->publish_change(
            static_cast<const void*>(&pointer_capture_changed_),
            [this, publication_owner, change] {
                if (!publication_owner->is_alive() ||
                    publication_owner->window_ != this) return;
                pointer_capture_changed_.emit(change);
            });
    } else {
        pointer_capture_changed_.emit(change);
    }
}

bool Window::dispatch_pointer(PointerEvent event) {
    require_ui_thread("pointer dispatch");
    if (in_paint_) {
        return defer_input(DeferredInput{std::move(event)});
    }
    metrics_.record_input();
    if (event.action == PointerAction::move) {
        Control::Ptr next_hover = hit_test(event.position);
        Control::Ptr previous_hover = hovered_.lock();
        if (previous_hover != next_hover) {
            if (previous_hover && eligible(previous_hover)) {
                PointerEvent leave = event;
                leave.action = PointerAction::leave;
                leave.button = PointerButton::none;
                leave.phase = EventPhase::target;
                leave.handled = false;
                metrics_.record_callback_emitted();
                previous_hover->on_pointer(leave);
                previous_hover->publish_change(
                    previous_hover->pointer_observed_, leave);
            }
            hovered_ = next_hover;
            if (next_hover && eligible(next_hover)) {
                PointerEvent enter = event;
                enter.action = PointerAction::enter;
                enter.button = PointerButton::none;
                enter.phase = EventPhase::target;
                enter.handled = false;
                metrics_.record_callback_emitted();
                next_hover->on_pointer(enter);
                next_hover->publish_change(next_hover->pointer_observed_, enter);
            }
        }
    }
    Control::Ptr target = captured_.lock();
    if (!target) {
        target = hit_test(event.position);
    }
    if (!target) {
        return false;
    }

    // A release callback may synchronously open a modal window. Relinquish both
    // retained and platform capture before user code runs so that the modal can
    // receive its first pointer message. Keep strong local references for this
    // route and for the later click qualification.
    Control::Ptr pressed_for_release;
    const bool primary_release = event.action == PointerAction::up &&
        event.button == PointerButton::primary;
    if (primary_release) {
        pressed_for_release = pressed_.lock();
        pressed_.reset();
        release_pointer();
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
            return true;
        }
    }

    if (!eligible(target)) {
        return event.handled;
    }

    if (event.action == PointerAction::down &&
        event.button == PointerButton::primary) {
        const bool accepts_press = !target->focusable_ || request_focus(target);
        if (!accepts_press) {
            // Prevent-mode validation rejects the input transaction that
            // attempted to enter this focusable target. In particular, a
            // Button must not publish Click after focus validation failed.
            event.handled = true;
            return true;
        }
        if (eligible(target)) {
            pressed_ = target;
            capture_pointer(target, event.pointer_id == 0 ? 1 : event.pointer_id);
        }
    }

    if (eligible(target)) {
        event.phase = EventPhase::target;
        metrics_.record_callback_emitted();
        target->on_pointer(event);
        target->publish_change(target->pointer_observed_, event);
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

    if (primary_release) {
        Control::Ptr released_over = hit_test(event.position);
        if (pressed_for_release && pressed_for_release == released_over &&
            eligible(pressed_for_release)) {
            metrics_.record_callback_emitted();
            pressed_for_release->on_activate();
            metrics_.record_activation();
        }
    }
    return event.handled;
}

bool Window::dispatch_key(KeyEvent event) {
    require_ui_thread("key dispatch");
    if (in_paint_) {
        return defer_input(DeferredInput{std::move(event)});
    }
    metrics_.record_input();
    const bool traversal_key = event.action == KeyAction::down &&
        event.physical_key == PhysicalKey::tab;
    const bool forward =
        (static_cast<std::uint8_t>(event.modifiers) &
         static_cast<std::uint8_t>(Modifier::shift)) == 0U;
    const auto dispatch_dialog_key = [this, &event]() {
        if (event.action != KeyAction::down || event.repeat) return false;
        const bool alt = has_modifier(event.modifiers, Modifier::alt);
        const bool control = has_modifier(event.modifiers, Modifier::control);
        const bool meta = has_modifier(event.modifiers, Modifier::meta);
        if (alt && !control && !meta) {
            if (const auto character = physical_mnemonic(event.physical_key)) {
                return dispatch_mnemonic(*character);
            }
        }
        if (!alt && !control && !meta) {
            if (event.physical_key == PhysicalKey::enter) {
                return dispatch_dialog_button(true);
            }
            if (event.physical_key == PhysicalKey::escape) {
                return dispatch_dialog_button(false);
            }
        }
        return false;
    };
    if (focus_scopes_.empty() && dispatch_accelerator(event, true)) return true;
    Control::Ptr target = focused_.lock();
    if (!target || !eligible(target)) {
        if (dispatch_accelerator(event, false)) return true;
        if (dispatch_dialog_key()) return true;
        return traversal_key && move_focus(forward);
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
    if (!event.handled && dispatch_accelerator(event, false)) {
        return true;
    }
    if (!event.handled && dispatch_dialog_key()) return true;
    if (!event.handled && traversal_key) {
        return move_focus(forward);
    }
    return event.handled;
}

bool Window::dispatch_text(TextInputEvent event) {
    require_ui_thread("text dispatch");
    if (in_paint_) {
        return defer_input(DeferredInput{std::move(event)});
    }
    metrics_.record_input();
    Control::Ptr target = focused_.lock();
    if (!target || !eligible(target)) {
        return false;
    }
    metrics_.record_callback_emitted();
    target->on_text_input(event);
    return event.handled;
}

DeferredInputSnapshot Window::deferred_input_snapshot() const noexcept {
    return {deferred_inputs_.size(), maximum_deferred_inputs,
            deferred_inputs_received_, deferred_inputs_delivered_,
            deferred_input_moves_coalesced_,
            deferred_drag_overs_coalesced_, deferred_inputs_rejected_,
            deferred_inputs_abandoned_, deferred_input_faults_,
            deferred_input_drains_, deferred_input_drain_queued_,
            draining_deferred_input_};
}

bool Window::defer_input(DeferredInput input) {
    if (auto* pointer = std::get_if<PointerEvent>(&input);
        pointer && pointer->action == PointerAction::move &&
        !deferred_inputs_.empty()) {
        if (auto* previous =
                std::get_if<PointerEvent>(&deferred_inputs_.back());
            previous && previous->action == PointerAction::move &&
            previous->pointer_id == pointer->pointer_id) {
            *previous = std::move(*pointer);
            ++deferred_inputs_received_;
            ++deferred_input_moves_coalesced_;
            return true;
        }
    }
    if (auto* drag = std::get_if<DragEvent>(&input);
        drag && drag->action == DragAction::over &&
        !deferred_inputs_.empty()) {
        if (auto* previous =
                std::get_if<DragEvent>(&deferred_inputs_.back());
            previous && previous->action == DragAction::over &&
            previous->session_id == drag->session_id) {
            *previous = std::move(*drag);
            ++deferred_inputs_received_;
            ++deferred_drag_overs_coalesced_;
            return true;
        }
    }
    if (deferred_inputs_.size() >= maximum_deferred_inputs) {
        ++deferred_inputs_rejected_;
        return false;
    }
    deferred_inputs_.push_back(std::move(input));
    ++deferred_inputs_received_;
    return true;
}

void Window::schedule_deferred_input_drain() noexcept {
    if (deferred_inputs_.empty() || deferred_input_drain_queued_ ||
        draining_deferred_input_ || in_paint_) {
        return;
    }
    deferred_input_drain_queued_ = true;
    try {
        deferred_input_drain_operation_ =
            begin_invoke([this] { drain_deferred_input(); });
    } catch (...) {
        deferred_input_drain_queued_ = false;
        ++deferred_input_faults_;
        deferred_inputs_abandoned_ += deferred_inputs_.size();
        deferred_inputs_.clear();
    }
}

void Window::drain_deferred_input() {
    require_ui_thread("deferred input drain");
    deferred_input_drain_operation_ = {};
    deferred_input_drain_queued_ = false;
    if (in_paint_) {
        schedule_deferred_input_drain();
        return;
    }
    if (draining_deferred_input_ || deferred_inputs_.empty()) return;

    draining_deferred_input_ = true;
    std::exception_ptr first_fault;
    while (!deferred_inputs_.empty()) {
        DeferredInput input = std::move(deferred_inputs_.front());
        deferred_inputs_.pop_front();
        try {
            std::visit([this](auto&& event) {
                using Event = std::decay_t<decltype(event)>;
                if constexpr (std::is_same_v<Event, PointerEvent>) {
                    static_cast<void>(dispatch_pointer(std::move(event)));
                } else if constexpr (std::is_same_v<Event, KeyEvent>) {
                    static_cast<void>(dispatch_key(std::move(event)));
                } else if constexpr (std::is_same_v<Event, TextInputEvent>) {
                    static_cast<void>(dispatch_text(std::move(event)));
                } else if constexpr (std::is_same_v<Event, DragEvent>) {
                    static_cast<void>(dispatch_drag(std::move(event)));
                } else {
                    static_cast<void>(perform_semantic_action(
                        event.stable_id, event.action, event.value));
                }
            }, std::move(input));
            ++deferred_inputs_delivered_;
        } catch (...) {
            ++deferred_input_faults_;
            if (!first_fault) first_fault = std::current_exception();
        }
    }
    draining_deferred_input_ = false;
    ++deferred_input_drains_;
    if (!deferred_inputs_.empty()) schedule_deferred_input_drain();
    if (first_fault) std::rethrow_exception(first_fault);
}

void Window::abandon_deferred_input() noexcept {
    static_cast<void>(deferred_input_drain_operation_.cancel());
    deferred_input_drain_operation_ = {};
    deferred_inputs_abandoned_ += deferred_inputs_.size();
    deferred_inputs_.clear();
    deferred_input_drain_queued_ = false;
}

void Window::abandon_deferred_drag() noexcept {
    const std::size_t before = deferred_inputs_.size();
    std::erase_if(deferred_inputs_, [](const DeferredInput& input) {
        return std::holds_alternative<DragEvent>(input);
    });
    deferred_inputs_abandoned_ += before - deferred_inputs_.size();
    if (deferred_inputs_.empty() && deferred_input_drain_queued_) {
        static_cast<void>(deferred_input_drain_operation_.cancel());
        deferred_input_drain_operation_ = {};
        deferred_input_drain_queued_ = false;
    }
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
    if (in_paint_) {
        const std::uint64_t session = event.session_id;
        const DragEffect allowed = event.allowed_effects;
        const bool retained =
            defer_input(DeferredInput{std::move(event)});
        const DragEffect prior =
            drag_session_id_ == session &&
                    single_allowed_effect(drag_last_accepted_effect_, allowed)
                ? drag_last_accepted_effect_
                : DragEffect::none;
        return {retained, retained ? prior : DragEffect::none,
                retained, !retained};
    }
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
            drag_last_accepted_effect_ = DragEffect::none;
        }
        return aggregate;
    }

    if (drag_session_id_ != 0 && drag_session_id_ != event.session_id) {
        leave_current();
        drag_session_id_ = 0;
        drag_last_accepted_effect_ = DragEffect::none;
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

    const DragAction action = event.action;
    if (action == DragAction::over && candidate) {
        event.accepted_effect = DragEffect::none;
        event.handled = false;
        route_and_merge(candidate, std::move(event));
    } else if (action == DragAction::drop && candidate) {
        event.accepted_effect = DragEffect::none;
        event.handled = false;
        route_and_merge(candidate, std::move(event));
        drag_target_.reset();
        drag_session_id_ = 0;
    } else if (action == DragAction::drop) {
        drag_target_.reset();
        drag_session_id_ = 0;
    }
    if (action == DragAction::drop) {
        drag_last_accepted_effect_ = DragEffect::none;
    } else {
        drag_last_accepted_effect_ = aggregate.accepted_effect;
    }
    return aggregate;
}

void Window::cancel_drag() noexcept {
    drag_target_.reset();
    drag_session_id_ = 0;
    drag_last_accepted_effect_ = DragEffect::none;
    abandon_deferred_drag();
}

void Window::attach_subtree(const Control::Ptr& control, const Control::WeakPtr& parent) {
    require_ui_thread("visual-tree attachment");
    register_subtree(control);
    std::vector<Control::Ptr> controls;
    std::function<void(const Control::Ptr&, const Control::WeakPtr&)> attach =
        [&](const Control::Ptr& current, const Control::WeakPtr& current_parent) {
            current->window_ = this;
            {
                std::scoped_lock lock(current->dispatcher_mutex_);
                current->dispatcher_state_ = dispatcher_state_;
            }
            current->parent_ = current_parent;
            current->lifecycle_notification_ = true;
            controls.push_back(current);
            for (const auto& child : current->children_) {
                attach(child, current);
            }
        };
    attach(control, parent);
    std::vector<Control::Ptr> notified;
    in_lifecycle_notification_ = true;
    try {
        for (const Control::Ptr& current : controls) {
            notified.push_back(current);
            current->on_attached_to_window();
        }
    } catch (...) {
        for (auto current = controls.rbegin(); current != controls.rend(); ++current) {
            (*current)->window_ = nullptr;
            {
                std::scoped_lock lock((*current)->dispatcher_mutex_);
                (*current)->dispatcher_state_.reset();
            }
        }
        for (auto current = notified.rbegin(); current != notified.rend(); ++current) {
            (*current)->on_detached_from_window();
        }
        for (const Control::Ptr& current : controls) {
            current->lifecycle_notification_ = false;
        }
        in_lifecycle_notification_ = false;
        unregister_subtree(control);
        metrics_.set_population(stable_ids_.size(), stable_ids_.size());
        throw;
    }
    for (const Control::Ptr& current : controls) {
        current->on_attachment_committed();
    }
    for (const Control::Ptr& current : controls) {
        current->lifecycle_notification_ = false;
    }
    in_lifecycle_notification_ = false;
    metrics_.set_population(stable_ids_.size(), stable_ids_.size());
    static_cast<void>(recompute_subtree_dirty(root_));
    mark_subtree_dirty(*control, invalidation::visual_tree);
}

void Window::detach_subtree(const Control::Ptr& control) {
    require_ui_thread("visual-tree detachment");
    clear_dialog_targets_for_subtree(control);
    close_popups_for_subtree(control);
    close_focus_scopes_for_subtree(control);
    revoke_interaction_for_subtree(control, true);
    if (control->window_ != this) {
        return;
    }
    const Rect old_bounds = absolute_bounds_of(*control);
    add_subtree_damage(control);
    unregister_subtree(control);
    std::vector<Control::Ptr> controls;
    in_lifecycle_notification_ = true;
    std::function<void(const Control::Ptr&)> detach = [&](const Control::Ptr& current) {
        current->lifecycle_notification_ = true;
        controls.push_back(current);
        current->on_detaching_from_window(*this);
        current->window_ = nullptr;
        {
            std::scoped_lock lock(current->dispatcher_mutex_);
            current->dispatcher_state_.reset();
        }
        for (const auto& child : current->children_) {
            detach(child);
        }
    };
    detach(control);
    for (auto current = controls.rbegin(); current != controls.rend(); ++current) {
        (*current)->on_detached_from_window();
        (*current)->lifecycle_notification_ = false;
    }
    in_lifecycle_notification_ = false;
    touch_paint();
    paint_dirty_ = true;
    ++semantic_generation_;
    if (semantic_generation_ == 0U) ++semantic_generation_;
    metrics_.record_dirty_mark(old_bounds.area());
    metrics_.set_population(stable_ids_.size(), stable_ids_.size());
    update_display_cache_metrics();
}

void Window::dispose_subtree(const Control::Ptr& control) noexcept {
    clear_dialog_targets_for_subtree(control);
    if (control == root_) {
        abandon_deferred_input();
    }
    const std::uint64_t disposal_count = control->subtree_size();
    revoke_focus_scopes_for_subtree(control);
    revoke_interaction_for_subtree(control, false);
    const Rect old_bounds = absolute_bounds_of(*control);
    add_subtree_damage(control);
    unregister_subtree(control);
    std::vector<Control::Ptr> controls;
    in_lifecycle_notification_ = true;
    std::function<void(const Control::Ptr&)> detach = [&](const Control::Ptr& current) {
        current->lifecycle_notification_ = true;
        controls.push_back(current);
        current->on_detaching_from_window(*this);
        current->window_ = nullptr;
        {
            std::scoped_lock lock(current->dispatcher_mutex_);
            current->dispatcher_state_.reset();
        }
        for (const auto& child : current->children_) {
            detach(child);
        }
    };
    detach(control);
    for (auto current = controls.rbegin(); current != controls.rend(); ++current) {
        (*current)->on_detached_from_window();
        (*current)->lifecycle_notification_ = false;
    }
    in_lifecycle_notification_ = false;
    if (auto visual_parent = control->parent_.lock()) {
        const auto found = std::find(visual_parent->children_.begin(),
                                    visual_parent->children_.end(), control);
        if (found != visual_parent->children_.end()) {
            visual_parent->children_.erase(found);
        }
        // Explicit child disposal is also a structural mutation of its live
        // parent. Retire the parent's retained layout/paint chunk now; merely
        // setting the Window-level dirty booleans leaves recursive layout with
        // no dirty node to visit and composites cannot reconcile their model.
        if (visual_parent->is_alive()) {
            mark_dirty(*visual_parent, invalidation::visual_tree);
        }
    }
    control->parent_.reset();
    touch_paint();
    paint_dirty_ = true;
    layout_dirty_ = true;
    hit_test_dirty_ = true;
    ++semantic_generation_;
    if (semantic_generation_ == 0U) ++semantic_generation_;
    metrics_.record_dirty_mark(old_bounds.area());
    metrics_.record_disposal(disposal_count);
    metrics_.set_population(stable_ids_.size(), stable_ids_.size());
    static_cast<void>(recompute_subtree_dirty(root_));
    update_display_cache_metrics();
}

void Window::clear_dialog_targets_for_subtree(
    const Control::Ptr& control) noexcept {
    if (!control) return;
    if (Control::Ptr accept = accept_button_.lock();
        contains_control(control, accept)) {
        if (accept->is_alive()) {
            try { accept->notify_default(false); } catch (...) {}
        }
        accept_button_.reset();
    }
    if (contains_control(control, cancel_button_.lock())) cancel_button_.reset();
}

void Window::revoke_interaction_for_subtree(const Control::Ptr& control,
                                            bool notify_focus) {
    Control::Ptr focused = focused_.lock();
    if (contains_control(control, focused)) {
        focused_.reset();
        metrics_.record_focus_transition();
        metrics_.record_focus_revocation();
        if (notify_focus && focused && focused->is_alive()) {
            focused->publish_change(
                static_cast<const void*>(&focused->focus_observed_),
                [this, focused] {
                    if (!focused->is_alive() || focused->window_ != this) return;
                    metrics_.record_callback_emitted();
                    focused->on_focus_changed(false);
                    if (focused->is_alive()) {
                        focused->focus_observed_.emit(false);
                    }
                    if (focused->is_alive()) {
                        focused->invalidate(invalidation::focus);
                    }
                });
        }
    }
    if (contains_control(control, captured_.lock())) {
        change_pointer_capture({}, 0, true);
    }
    if (contains_control(control, pressed_.lock())) {
        pressed_.reset();
        metrics_.record_press_revocation();
    }
    if (contains_control(control, hovered_.lock())) {
        hovered_.reset();
    }
    if (contains_control(control, drag_target_.lock())) {
        cancel_drag();
    }
}

void Window::close_focus_scopes_for_subtree(const Control::Ptr& control) {
    std::vector<FocusScopeId> closing;
    closing.reserve(focus_scopes_.size());
    for (const FocusScopeState& state : focus_scopes_) {
        if (state.active && contains_control(control, state.root.lock())) {
            closing.push_back(state.id);
        }
    }
    for (FocusScopeId scope : closing) {
        static_cast<void>(end_focus_scope(
            scope, FocusScopeCloseReason::owner_unavailable, control));
    }
}

void Window::revoke_focus_scopes_for_subtree(
    const Control::Ptr& control) noexcept {
    std::size_t closed = 0;
    focus_scopes_.erase(
        std::remove_if(
            focus_scopes_.begin(), focus_scopes_.end(),
            [&](const FocusScopeState& state) {
                const bool remove = state.active &&
                    contains_control(control, state.root.lock());
                closed += remove ? 1U : 0U;
                return remove;
            }),
        focus_scopes_.end());
    while (closed-- > 0U) {
        metrics_.record_focus_scope_closed(false, focus_scope_depth());
    }
}

void Window::on_eligibility_changed(const Control::Ptr& control) {
    require_ui_thread("eligibility mutation");
    close_popups_for_subtree(control);
    close_focus_scopes_for_subtree(control);
    revoke_interaction_for_subtree(control, true);
}

void Window::on_hit_test_transparency_changed(const Control::Ptr& control) {
    require_ui_thread("hit-test transparency mutation");
    if (!control) return;
    if (contains_control(control, captured_.lock())) {
        change_pointer_capture({}, 0, true);
    }
    if (contains_control(control, pressed_.lock())) {
        pressed_.reset();
        metrics_.record_press_revocation();
    }
    if (contains_control(control, hovered_.lock())) hovered_.reset();
    if (contains_control(control, drag_target_.lock())) cancel_drag();
}

void Window::publish_control_availability(Control& control) {
    require_ui_thread("control availability publication");
    if (!control.is_alive() || control.window_ != this) return;
    const ControlAvailabilityChange change{
        control.runtime_id(), std::string(control.stable_id().value()),
        control.effectively_visible(), control.effectively_enabled()};
    const auto retained = control.shared_from_this();
    control.publish_change(
        static_cast<const void*>(&control_availability_changed_),
        [this, retained, change] {
            if (!retained->is_alive() || retained->window_ != this) return;
            control_availability_changed_.emit(change);
        });
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
        static std::atomic<std::uint64_t> traced_layout_invalidations{};
        if (std::getenv("GUI_FORMS_TRACE_LAYOUT_INVALIDATION") != nullptr) {
            const std::uint64_t sequence =
                traced_layout_invalidations.fetch_add(1U,
                    std::memory_order_relaxed) + 1U;
            if (sequence <= 2048U) {
                std::fprintf(stderr,
                    "gui-forms-layout-dirty=%llu|control:%.*s|flags:%u\n",
                    static_cast<unsigned long long>(sequence),
                    static_cast<int>(control.stable_id().value().size()),
                    control.stable_id().value().data(),
                    static_cast<unsigned>(requested_dirty));
            }
        }
        note_suspended_layout_request(control);
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
    if (has_dirty(requested_dirty, Dirty::semantics)) {
        ++semantic_generation_;
        if (semantic_generation_ == 0U) ++semantic_generation_;
    }
    if (has_dirty(requested_dirty, Dirty::paint)) {
        touch_paint();
        paint_dirty_ = true;
        Rect bounds = paint_damage_bounds_of(control);
        if (bounds.empty()) {
            bounds = control.requested_bounds_;
        }
        add_damage(bounds, control.paint_plane_);
        metrics_.record_dirty_mark(bounds.area());
    } else {
        metrics_.record_dirty_mark(0.0);
    }
}

void Window::mark_paint_dirty(Control& control, Rect local_damage) {
    require_ui_thread("localized paint mutation");
    if (control.window_ != this) {
        throw std::invalid_argument(
            "localized paint mutation requires an attached control");
    }
    local_damage = Rect::intersection(local_damage, control.client_rectangle());
    if (local_damage.empty()) return;

    control.dirty_ |= Dirty::paint;
    control.subtree_dirty_ |= Dirty::paint;
    for (auto ancestor = control.parent(); ancestor; ancestor = ancestor->parent()) {
        ancestor->subtree_dirty_ |= Dirty::paint;
    }
    metrics_.record_mutation();
    touch_paint();
    paint_dirty_ = true;
    const Rect bounds = absolute_bounds_of(control);
    const Rect window_damage{bounds.x + local_damage.x,
                             bounds.y + local_damage.y,
                             local_damage.width, local_damage.height};
    add_damage(window_damage, control.paint_plane_);
    metrics_.record_dirty_mark(window_damage.area());
}

void Window::mark_child_layout_slot(Control& control) {
    require_ui_thread("child layout-slot mutation");
    constexpr Dirty effects = Dirty::arrange | Dirty::hit_test |
                              Dirty::semantics | Dirty::accessibility;
    control.dirty_ |= effects;
    note_suspended_layout_request(control);
    control.subtree_dirty_ |= effects;
    for (auto ancestor = control.parent(); ancestor;
         ancestor = ancestor->parent()) {
        ancestor->subtree_dirty_ |= effects;
    }
    layout_dirty_ = true;
    hit_test_dirty_ = true;
    ++semantic_generation_;
    if (semantic_generation_ == 0U) ++semantic_generation_;
}

SemanticSnapshot Window::semantic_snapshot() {
    require_ui_thread("semantic snapshot");
    if (in_semantic_snapshot_) {
        throw std::logic_error("GUI.Forms rejects recursive semantic snapshots");
    }
    in_semantic_snapshot_ = true;
    struct Reset final {
        bool& value;
        ~Reset() { value = false; }
    } reset{in_semantic_snapshot_};

    for (std::uint32_t pass = 0U;
         pass < maximum_semantic_snapshot_passes; ++pass) {
        ensure_layout(true);
        const std::uint64_t generation = semantic_generation_;
        SemanticSnapshot snapshot;
        snapshot.generation = generation;
        const Control::Ptr focused = focused_.lock();
        std::vector<Control::Ptr> roots{root_};
        roots.reserve(1U + popups_.size());
        for (const auto& popup : popups_) roots.push_back(popup->popup());
        for (const Control::Ptr& root : roots) {
            if (!root || !root->is_alive() || root->window_ != this ||
                root->parent()) {
                continue;
            }
            append_semantic_nodes(root, nullptr, this, focused, snapshot.roots,
                                  snapshot.node_count);
        }
        if (generation == semantic_generation_) {
            return snapshot;
        }
        metrics_.record_callback_arbitration_retry(
            pass + 1U == maximum_semantic_snapshot_passes);
    }
    throw std::runtime_error(
        "GUI.Forms semantic snapshot did not converge within four passes");
}

bool Window::perform_semantic_action(std::string_view stable_id,
                                     SemanticAction action,
                                     std::string_view value) {
    require_ui_thread("semantic action");
    if (in_paint_) {
        return defer_input(DeferredInput{DeferredSemanticInput{
            std::string(stable_id), action, std::string(value)}});
    }
    const Control::Ptr control = find(stable_id);
    if (!control) {
        if (dispatch_semantic_child_action(root_, stable_id, action, value)) {
            return true;
        }
        std::vector<Control::Ptr> popup_roots;
        popup_roots.reserve(popups_.size());
        for (const auto& popup : popups_) popup_roots.push_back(popup->popup());
        for (auto popup = popup_roots.rbegin(); popup != popup_roots.rend();
             ++popup) {
            if (dispatch_semantic_child_action(
                    *popup, stable_id, action, value)) return true;
        }
        return false;
    }
    if (!eligible(control)) return false;
    if (action == SemanticAction::focus) return request_focus(control);
    return control->on_semantic_action(action, value);
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
        if (has_dirty(requested_dirty, Dirty::layout) &&
            current.layout_suspend_depth_ != 0U) {
            current.layout_deferred_ = true;
            ++current.layout_requested_revision_;
            if (current.layout_requested_revision_ == 0U) {
                ++current.layout_requested_revision_;
            }
        }
        if (has_dirty(requested_dirty, Dirty::paint)) {
            Rect bounds = paint_damage_bounds_of(current);
            if (bounds.empty()) {
                bounds = current.requested_bounds_;
            }
            // Subtree invalidation is structural (visibility, attachment, or a
            // visual-tree/layout replacement). Recompose every plane so pixels
            // formerly occupied by a transparent control are restored from the
            // backplane instead of surviving as stale fragments.
            add_damage_all_planes(bounds);
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
            if (ancestor->layout_suspend_depth_ != 0U) {
                ancestor->layout_deferred_ = true;
                ++ancestor->layout_requested_revision_;
                if (ancestor->layout_requested_revision_ == 0U) {
                    ++ancestor->layout_requested_revision_;
                }
            }
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
        touch_paint();
        paint_dirty_ = true;
        metrics_.record_dirty_mark(requested_damage_area);
    } else {
        metrics_.record_dirty_mark(0.0);
    }
}

void Window::change_paint_plane(Control& control, PaintPlane plane) {
    require_ui_thread("paint-plane mutation");
    const Rect bounds = paint_damage_bounds_of(control);
    add_damage_all_planes(bounds);
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

void Window::request_paint_wake() noexcept {
    if (paint_wake_pending_) {
        ++paint_wakes_coalesced_;
        return;
    }
    if (occluded_ || !paint_wake_handler_) return;
    paint_wake_pending_ = true;
    try {
        paint_wake_handler_();
        ++paint_wakes_queued_;
    } catch (...) {
        // Host wake seams are advisory and must never make a retained model
        // mutation fail. Clearing the coalescing bit permits a later mutation
        // or exposure transition to retry.
        paint_wake_pending_ = false;
    }
}

void Window::acknowledge_paint_wake_if_damage_drained() noexcept {
    const bool damage_remains = std::any_of(
        plane_damage_.begin(), plane_damage_.end(),
        [](const DamageRegion& damage) { return !damage.empty(); });
    if (!damage_remains) paint_wake_pending_ = false;
}

void Window::touch_paint() noexcept {
    ++content_revision_;
    if (content_revision_ == 0U) ++content_revision_;
    if (in_paint_) {
        dirty_after_render_ = true;
        paint_lease_state_ = PaintLeaseState::rendering_dirty;
    } else {
        paint_lease_state_ = occluded_ ? PaintLeaseState::occluded_dirty
                                       : PaintLeaseState::dirty_queued;
    }
    request_paint_wake();
}

void Window::update_paint_lease_state() noexcept {
    if (in_paint_) {
        paint_lease_state_ = dirty_after_render_
            ? PaintLeaseState::rendering_dirty
            : PaintLeaseState::rendering;
    } else if (occluded_ && paint_dirty_) {
        paint_lease_state_ = PaintLeaseState::occluded_dirty;
    } else if (paint_dirty_ || rendered_revision_ < content_revision_) {
        paint_lease_state_ = PaintLeaseState::dirty_queued;
    } else if (presented_revision_ < rendered_revision_) {
        paint_lease_state_ = PaintLeaseState::ready;
    } else {
        paint_lease_state_ = PaintLeaseState::clean;
        dirty_after_render_ = false;
    }
}

void Window::add_subtree_damage(const Control::Ptr& control) {
    add_damage_all_planes(paint_damage_bounds_of(*control));
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

    const auto any_runnable_layout = [this]() {
        if (has_runnable_layout_dirty(root_)) return true;
        return std::any_of(popups_.begin(), popups_.end(),
                           [this](const auto& popup) {
            return has_runnable_layout_dirty(popup->popup());
        });
    };
    if (!any_runnable_layout()) return;

    metrics_.record_flush(read_barrier);
    in_layout_ = true;
    try {
        std::uint32_t pass = 0;
        do {
            second_layout_pass_requested_ = false;
            std::uint64_t measure_visited = 0;
            std::uint64_t measured = 0;
            measure_dirty_recursive(root_, client_size_, measure_visited, measured);
            for (const auto& popup : popups_) {
                if (const Control::Ptr overlay = popup->popup()) {
                    measure_dirty_recursive(overlay, client_size_, measure_visited,
                                            measured);
                }
            }
            metrics_.record_measure(measure_visited, measured);

            std::uint64_t arrange_visited = 0;
            std::uint64_t arranged = 0;
            arrange_dirty_recursive(
                root_, {0.0, 0.0, client_size_.width, client_size_.height},
                arrange_visited, arranged);
            for (const auto& popup : popups_) {
                if (const Control::Ptr overlay = popup->popup()) {
                    arrange_dirty_recursive(overlay, overlay->requested_bounds_,
                                            arrange_visited, arranged);
                }
            }
            metrics_.record_arrange(arrange_visited, arranged);

            static_cast<void>(recompute_subtree_dirty(root_));
            for (const auto& popup : popups_) {
                if (const Control::Ptr overlay = popup->popup()) {
                    static_cast<void>(recompute_subtree_dirty(overlay));
                }
            }
            commit_layout_requests_recursive(root_);
            for (const auto& popup : popups_) {
                commit_layout_requests_recursive(popup->popup());
            }
            ++pass;
        } while (any_runnable_layout() && pass < maximum_layout_passes);

        const bool popup_layout_dirty = std::any_of(
            popups_.begin(), popups_.end(), [](const auto& popup) {
                const Control::Ptr overlay = popup->popup();
                return overlay &&
                    has_dirty(overlay->subtree_dirty_, Dirty::layout);
            });
        if (any_runnable_layout()) {
            metrics_.record_pass_limit_hit();
        }
        layout_dirty_ = has_dirty(root_->subtree_dirty_, Dirty::layout) ||
                        popup_layout_dirty;
        hit_test_dirty_ = layout_dirty_;
        in_layout_ = false;
    } catch (...) {
        // A user layout callback may fail. Preserve dirty state for an
        // explicit later retry and always release the re-entry guard.
        static_cast<void>(recompute_subtree_dirty(root_));
        for (const auto& popup : popups_) {
            if (const Control::Ptr overlay = popup->popup()) {
                static_cast<void>(recompute_subtree_dirty(overlay));
            }
        }
        layout_dirty_ = has_dirty(root_->subtree_dirty_, Dirty::layout) ||
            std::any_of(popups_.begin(), popups_.end(), [](const auto& popup) {
                const Control::Ptr overlay = popup->popup();
                return overlay &&
                    has_dirty(overlay->subtree_dirty_, Dirty::layout);
            });
        hit_test_dirty_ = layout_dirty_;
        second_layout_pass_requested_ = false;
        in_layout_ = false;
        throw;
    }
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
    if (!eligible(control) || control->window_ != this) {
        return {};
    }
    Control* const retained_parent = control->parent_.lock().get();
    const Rect bounds = absolute_bounds_of(*control);
    if (!bounds.contains(window_position)) {
        return {};
    }
    const Rect local_child_viewport = control->child_viewport_rectangle();
    if (!control->is_alive() || control->window_ != this ||
        control->parent_.lock().get() != retained_parent) {
        return {};
    }
    const Rect child_viewport{bounds.x + local_child_viewport.x,
                              bounds.y + local_child_viewport.y,
                              local_child_viewport.width,
                              local_child_viewport.height};
    if (child_viewport.contains(window_position)) {
        const std::vector<Control::Ptr> children = control->children_;
        for (auto child = children.rbegin(); child != children.rend(); ++child) {
            if (!*child || !(*child)->is_alive() ||
                (*child)->parent_.lock().get() != control.get() ||
                (*child)->window_ != this) {
                continue;
            }
            if (auto target = hit_test_recursive(*child, window_position)) {
                if (!target->is_alive() || target->window_ != this) continue;
                return target;
            }
        }
    }
    const Point local{window_position.x - bounds.x, window_position.y - bounds.y};
    const bool hit = !control->hit_test_transparent_ &&
                     control->hit_test_local(local);
    return hit && eligible(control) && control->window_ == this &&
            control->parent_.lock().get() == retained_parent
        ? control : Control::Ptr{};
}

void Window::measure_dirty_recursive(const Control::Ptr& control,
                                     Size available,
                                     std::uint64_t& visited_nodes,
                                     std::uint64_t& callbacks) {
    if (!control || control->layout_suspend_depth_ != 0U ||
        !has_dirty(control->subtree_dirty_, Dirty::measure)) {
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
    const std::vector<Control::Ptr> retained = control->children_;
    for (const auto& child : retained) {
        if (!child || !child->is_alive() ||
            child->parent_.lock().get() != control.get() ||
            child->window_ != this) {
            continue;
        }
        measure_dirty_recursive(child, child_available, visited_nodes, callbacks);
    }
    if (!control->is_alive() || control->window_ != this) return;
    if (has_dirty(control->dirty_, Dirty::measure)) {
        control->clear_dirty(Dirty::measure);
        try {
            static_cast<void>(control->measure(available));
        } catch (...) {
            control->dirty_ |= Dirty::measure | Dirty::arrange;
            control->subtree_dirty_ |= Dirty::measure | Dirty::arrange;
            throw;
        }
        ++callbacks;
        if (!control->is_alive() || control->window_ != this) return;
    }
}

void Window::arrange_dirty_recursive(const Control::Ptr& control,
                                     Rect final_bounds,
                                     std::uint64_t& visited_nodes,
                                     std::uint64_t& callbacks) {
    if (!control || control->layout_suspend_depth_ != 0U ||
        !has_dirty(control->subtree_dirty_, Dirty::arrange)) {
        return;
    }
    ++visited_nodes;
    if (!control->is_alive() || !control->visible_) {
        clear_layout_dirty_subtree(control);
        return;
    }

    if (has_dirty(control->dirty_, Dirty::arrange)) {
        const Rect old_bounds = visual_bounds_of(
            *control, control->last_painted_visual_outsets_);
        control->clear_dirty(Dirty::arrange | Dirty::hit_test);
        try {
            control->arrange(final_bounds);
        } catch (...) {
            control->dirty_ |= Dirty::arrange | Dirty::hit_test;
            control->subtree_dirty_ |= Dirty::arrange | Dirty::hit_test;
            throw;
        }
        ++callbacks;
        if (!control->is_alive() || control->window_ != this) return;
        const Rect new_bounds = visual_bounds_of(*control,
                                                  control->visual_outsets());
        if (old_bounds != new_bounds) {
            // Arrangement changes expose content below the moved control. The
            // compositor uses one target across ordered paint planes, so both
            // the vacated and occupied rectangles need full recomposition.
            add_damage_all_planes(old_bounds);
            add_damage_all_planes(new_bounds);
            touch_paint();
            paint_dirty_ = true;
            control->publish_change(control->arranged_bounds_changed_,
                                    new_bounds);
        }
    }
    const std::vector<Control::Ptr> retained = control->children_;
    for (const auto& child : retained) {
        if (!child || !child->is_alive() ||
            child->parent_.lock().get() != control.get() ||
            child->window_ != this) {
            continue;
        }
        arrange_dirty_recursive(
            child, child->layout_slot_.value_or(child->requested_bounds_),
            visited_nodes, callbacks);
    }
}

bool Window::has_runnable_layout_dirty(
    const Control::Ptr& control) const noexcept {
    if (!control || control->layout_suspend_depth_ != 0U ||
        !has_dirty(control->subtree_dirty_, Dirty::layout)) {
        return false;
    }
    if (has_dirty(control->dirty_, Dirty::layout)) return true;
    return std::any_of(control->children_.begin(), control->children_.end(),
                       [this](const Control::Ptr& child) {
        return has_runnable_layout_dirty(child);
    });
}

void Window::note_suspended_layout_request(Control& control) noexcept {
    for (Control* current = &control; current != nullptr;) {
        if (current->layout_suspend_depth_ != 0U) {
            current->layout_deferred_ = true;
            ++current->layout_requested_revision_;
            if (current->layout_requested_revision_ == 0U) {
                ++current->layout_requested_revision_;
            }
        }
        const Control::Ptr parent = current->parent_.lock();
        current = parent.get();
    }
}

void Window::commit_layout_requests_recursive(
    const Control::Ptr& control) noexcept {
    if (!control || control->layout_suspend_depth_ != 0U) return;
    for (const auto& child : control->children_) {
        commit_layout_requests_recursive(child);
    }
    if (!has_dirty(control->subtree_dirty_, Dirty::layout) &&
        control->layout_deferred_) {
        control->layout_committed_revision_ =
            control->layout_requested_revision_;
        control->layout_deferred_ = false;
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
    if (!control->is_alive() || control->window_ != this || !control->visible_) {
        clear_paint_dirty_subtree(control);
        return;
    }
    Control* const retained_parent = control->parent_.lock().get();
    const Rect bounds = absolute_bounds_of(*control);
    const Insets current_outsets = control->visual_outsets();
    if (!control->is_alive() || control->window_ != this ||
        control->parent_.lock().get() != retained_parent) {
        return;
    }
    const Rect visual_bounds = visual_bounds_of(*control, current_outsets);
    const Rect paint_intersection = Rect::intersection(visual_bounds, window_damage);
    const Rect local_child_viewport = control->child_viewport_rectangle();
    if (!control->is_alive() || control->window_ != this ||
        control->parent_.lock().get() != retained_parent) {
        return;
    }
    const Rect child_viewport{bounds.x + local_child_viewport.x,
                              bounds.y + local_child_viewport.y,
                              local_child_viewport.width,
                              local_child_viewport.height};
    const Rect child_intersection = Rect::intersection(
        child_viewport, window_damage);
    if (paint_intersection.empty() && child_intersection.empty()) {
        return;
    }

    if (control->paint_plane_ == plane && !paint_intersection.empty()) {
        const Rect logical_bounds{0.0, 0.0, bounds.width, bounds.height};
        const bool invalidated = has_dirty(control->dirty_, Dirty::paint);
        const bool rebuild = invalidated || !control->display_chunk_ ||
                             control->display_chunk_->plane() != plane ||
                             control->display_chunk_->logical_bounds() != logical_bounds;
        if (rebuild) {
            control->clear_dirty(Dirty::paint);
            detail::RecordingPainter recorder;
            control->on_paint(recorder, logical_bounds);
            if (!control->is_alive() || control->window_ != this ||
                control->parent_.lock().get() != retained_parent) {
                return;
            }
            control->on_paint_overlay(recorder, logical_bounds);
            if (!control->is_alive() || control->window_ != this ||
                control->parent_.lock().get() != retained_parent) {
                return;
            }
            control->display_chunk_ = recorder.finish(++display_generation_, plane,
                                                      logical_bounds);
            ++chunks_rebuilt;
            consumed_invalidations += invalidated ? 1U : 0U;
        } else {
            ++chunks_reused;
        }

        painter.save();
        painter.translate({bounds.x, bounds.y});
        const Rect local_damage{paint_intersection.x - bounds.x,
                                paint_intersection.y - bounds.y,
                                paint_intersection.width,
                                paint_intersection.height};
        painter.clip_rect(local_damage);
        commands_replayed +=
        detail::replay_display_chunk(*control->display_chunk_, painter);
        painter.restore();
        control->last_painted_visual_outsets_ = current_outsets;
        ++painted_controls;
    }

    if (!child_intersection.empty()) {
        const std::vector<Control::Ptr> children = control->children_;
        for (const auto& child : children) {
            if (!child || !child->is_alive() ||
                child->parent_.lock().get() != control.get() ||
                child->window_ != this) {
                continue;
            }
            paint_recursive(child, painter, child_intersection, plane, visited_nodes,
                            painted_controls, consumed_invalidations, chunks_rebuilt,
                            chunks_reused, commands_replayed);
        }
    }

    if (!control->is_alive() || control->window_ != this ||
        control->parent_.lock().get() != retained_parent) {
        return;
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
    std::uint64_t entries = root_ ? display_cache_entries(root_) : 0U;
    for (const auto& popup : popups_) {
        if (const Control::Ptr overlay = popup->popup()) {
            entries += display_cache_entries(overlay);
        }
    }
    metrics_.set_display_cache(entries, display_generation_);
}

void Window::compact_frame_requests() noexcept {
    for (const auto& request : frame_requests_) {
        if (request->kind == detail::FrameRequestKind::ui_timer) {
            continue;
        }
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

Rect Window::visual_bounds_of(const Control& control, Insets outsets) const {
    const auto bounded = [](double value) noexcept {
        return std::isfinite(value) ? std::clamp(value, 0.0, 16384.0) : 0.0;
    };
    outsets = {bounded(outsets.left), bounded(outsets.top),
               bounded(outsets.right), bounded(outsets.bottom)};
    const Rect bounds = absolute_bounds_of(control);
    return {bounds.x - outsets.left, bounds.y - outsets.top,
            bounds.width + outsets.left + outsets.right,
            bounds.height + outsets.top + outsets.bottom};
}

Rect Window::paint_damage_bounds_of(const Control& control) const {
    return Rect::united(
        visual_bounds_of(control, control.visual_outsets()),
        visual_bounds_of(control, control.last_painted_visual_outsets_));
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
