#pragma once

#include "gui_forms/control.hpp"
#include "gui_forms/display.hpp"
#include "gui_forms/dispatcher.hpp"
#include "gui_forms/event.hpp"
#include "gui_forms/metrics.hpp"
#include "gui_forms/resources.hpp"
#include "gui_forms/scheduler.hpp"

#include <chrono>
#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <variant>
#include <vector>

namespace gui_forms {

class UpdateScope;
class Timer;
class ToolTip;
class ImageList;
class HostServices;
class HostSession;
class BindingSource;
class BindingContext;
class LiveSurface;
namespace detail {
class PopupAttachment;
class AcceleratorAttachment;
}

class PopupToken final {
public:
    PopupToken() = default;
    ~PopupToken() { disconnect(); }
    PopupToken(PopupToken&& other) noexcept
        : attachment_(std::move(other.attachment_)) {}
    PopupToken& operator=(PopupToken&& other) noexcept {
        if (this != &other) {
            disconnect();
            attachment_ = std::move(other.attachment_);
        }
        return *this;
    }
    PopupToken(const PopupToken&) = delete;
    PopupToken& operator=(const PopupToken&) = delete;

    void disconnect() noexcept;
    [[nodiscard]] bool connected() const noexcept;
    [[nodiscard]] Event<>* closed_event() noexcept;

private:
    friend class Window;
    explicit PopupToken(std::shared_ptr<detail::PopupAttachment> attachment)
        : attachment_(std::move(attachment)) {}
    std::shared_ptr<detail::PopupAttachment> attachment_;
};

struct KeyGesture final {
    std::uint32_t physical_key{};
    Modifier modifiers{Modifier::none};
    friend constexpr auto operator<=>(const KeyGesture&,
                                      const KeyGesture&) = default;
};

struct AcceleratorOptions final {
    // Application navigation gestures such as Alt+Left may preempt ordinary
    // editor word-navigation. Active popup/focus scopes always retain first
    // refusal so menus and modal editors remain contained.
    bool before_focused_route{};
};

// Presentation preferences are expressed in logical UI terms and remain
// independent from Window::scale(), which is the host/device pixel scale.
// Keeping the axes separate prevents a 2x display from becoming a 200% text
// request and lets headless conformance exercise either dimension explicitly.
struct PresentationSettings final {
    double text_scale{1.0};
    bool high_contrast{};
    bool reduced_motion{};
    bool sound_enabled{true};
    friend constexpr bool operator==(const PresentationSettings&,
                                     const PresentationSettings&) = default;
};

// A newest-generation live layer is compositor work, not a retained-control
// repaint. The producer owns pixel publication; Window owns stable geometry;
// the host consumes these immutable placement snapshots without replaying the
// control tree.
struct LiveSurfacePresentation final {
    RuntimeId control{};
    std::shared_ptr<LiveSurface> surface;
    Rect destination{};
    Rect clip{};
};

enum class PaintLeaseState : std::uint8_t {
    clean,
    // New mutations collapse into one queued render opportunity. `dirty` is
    // retained as a source-compatible spelling for the original public name.
    dirty_queued,
    dirty = dirty_queued,
    rendering,
    ready,
    occluded_dirty,
    retired,
    // A mutation arrived while the exclusive lease was active. Completion of
    // that lease may publish its coherent result, but exactly one later drain
    // remains eligible; intermediate revisions are never queued individually.
    rendering_dirty,
};

struct PaintLeaseSnapshot final {
    PaintLeaseState state{PaintLeaseState::dirty_queued};
    std::uint64_t content_revision{1U};
    std::uint64_t rendered_revision{};
    std::uint64_t presented_revision{};
    std::uint64_t surface_epoch{1U};
    std::uint64_t leases_started{};
    std::uint64_t leases_completed{};
    std::uint64_t leases_abandoned{};
    std::uint64_t reentrant_requests_deferred{};
    std::uint64_t render_wakes_queued{};
    std::uint64_t render_wakes_coalesced{};
    std::uint64_t presentation_receipts_accepted{};
    std::uint64_t presentation_receipts_rejected{};
    bool render_wake_queued{};
    bool dirty_after_render{};
};

// Exact proof that one coherent retained transaction finished replaying into
// the host's private backing surface. A receipt is bound to both the sampled
// content revision and the surface epoch; native adapters must not publish a
// missing receipt or one from a replaced surface.
struct PaintReceipt final {
    std::uint64_t rendered_revision{};
    std::uint64_t surface_epoch{};

    [[nodiscard]] explicit constexpr operator bool() const noexcept {
        return rendered_revision != 0U && surface_epoch != 0U;
    }
    friend constexpr auto operator<=>(const PaintReceipt&,
                                      const PaintReceipt&) = default;
};

inline constexpr std::size_t maximum_deferred_inputs = 1024U;

// Renderer-neutral lease-time input diagnostics. Pointer moves may compact,
// but critical input is either retained in order or counted as capacity
// rejected; no event silently enters application code while paint is active.
struct DeferredInputSnapshot final {
    std::size_t pending{};
    std::size_t capacity{maximum_deferred_inputs};
    std::uint64_t deferred{};
    std::uint64_t delivered{};
    std::uint64_t coalesced_moves{};
    std::uint64_t coalesced_drag_overs{};
    std::uint64_t rejected_capacity{};
    std::uint64_t abandoned{};
    std::uint64_t faults{};
    std::uint64_t drains{};
    bool drain_queued{};
    bool draining{};
};

class AcceleratorToken final {
public:
    AcceleratorToken() = default;
    ~AcceleratorToken() { disconnect(); }
    AcceleratorToken(AcceleratorToken&& other) noexcept
        : attachment_(std::move(other.attachment_)) {}
    AcceleratorToken& operator=(AcceleratorToken&& other) noexcept {
        if (this != &other) {
            disconnect();
            attachment_ = std::move(other.attachment_);
        }
        return *this;
    }
    AcceleratorToken(const AcceleratorToken&) = delete;
    AcceleratorToken& operator=(const AcceleratorToken&) = delete;

    void disconnect() noexcept;
    [[nodiscard]] bool connected() const noexcept;

private:
    friend class Window;
    explicit AcceleratorToken(
        std::shared_ptr<detail::AcceleratorAttachment> attachment)
        : attachment_(std::move(attachment)) {}
    std::shared_ptr<detail::AcceleratorAttachment> attachment_;
};

struct PointerCaptureChange final {
    bool captured{};
    RuntimeId control_id{};
    std::string stable_id;
    std::uint64_t pointer_id{};
};

struct ControlAvailabilityChange final {
    RuntimeId control_id{};
    std::string stable_id;
    bool effectively_visible{};
    bool effectively_enabled{};
};

struct PopupOptions final {
    // Interactive menus and editors require an enabled owner. Passive
    // providers such as ToolTip may opt out while still requiring a live,
    // attached, effectively visible owner.
    bool require_enabled_owner{true};
};

struct FocusScopeId final {
    std::uint64_t value{};
    [[nodiscard]] explicit constexpr operator bool() const noexcept {
        return value != 0;
    }
    friend constexpr auto operator<=>(const FocusScopeId&,
                                      const FocusScopeId&) = default;
};

inline constexpr std::size_t maximum_focus_scope_depth = 32U;

struct FocusScopeOptions final {
    // Contained scopes reject attempts to move keyboard focus outside their
    // retained subtree while the scope is active.
    bool contain_focus{true};
    // Closing a scope restores the focus that was active when it opened.
    bool restore_focus{true};
    // If the previous focus is outside the scope, focus the first eligible
    // descendant in stable retained-tree order.
    bool focus_first{true};
};

enum class FocusScopeCloseReason : std::uint8_t {
    explicit_close,
    owner_unavailable,
};

struct FocusScopeChange final {
    FocusScopeId scope{};
    RuntimeId root_id{};
    std::string stable_id;
    std::size_t depth{};
    FocusScopeCloseReason close_reason{FocusScopeCloseReason::explicit_close};
    bool opened{};
    bool restored_focus{};
};

struct ValidationSnapshot final {
    std::uint64_t attempts{};
    std::uint64_t succeeded{};
    std::uint64_t cancelled{};
    std::uint64_t focus_moves_blocked{};
    std::uint64_t reentrant_requests_rejected{};
    std::uint64_t bulk_controls_visited{};
    bool validating{};
};

struct DialogKeySnapshot final {
    std::uint64_t mnemonic_attempts{};
    std::uint64_t mnemonics_handled{};
    std::uint64_t accept_attempts{};
    std::uint64_t accept_handled{};
    std::uint64_t cancel_attempts{};
    std::uint64_t cancel_handled{};
    std::uint64_t command_rejections{};
    std::uint64_t mnemonic_candidates{};
    std::uint64_t mnemonic_collisions{};
    std::uint64_t mnemonic_cycles{};
};

class Window {
public:
    explicit Window(Control::Ptr root, Size client_size = {});
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    [[nodiscard]] Control::Ptr root() const noexcept { return root_; }
    [[nodiscard]] Size client_size() const noexcept { return client_size_; }
    void resize(Size client_size);
    void set_scale(double scale);
    [[nodiscard]] double scale() const noexcept { return scale_; }
    [[nodiscard]] const PresentationSettings& presentation_settings() const noexcept {
        return presentation_settings_;
    }
    void set_presentation_settings(PresentationSettings settings);
    void set_text_scale(double text_scale);
    [[nodiscard]] Event<const PresentationSettings&>& presentation_changed() noexcept {
        return presentation_changed_;
    }
    [[nodiscard]] const Theme& theme() const noexcept { return *theme_; }
    [[nodiscard]] std::shared_ptr<const Theme> theme_ptr() const noexcept {
        return theme_;
    }
    void set_theme(std::shared_ptr<const Theme> theme);
    [[nodiscard]] Event<const Theme&>& theme_changed() noexcept {
        return theme_changed_;
    }
    [[nodiscard]] bool active() const noexcept { return active_; }
    void set_active(bool active);
    [[nodiscard]] Event<bool>& active_changed() noexcept {
        return active_changed_;
    }
    // Non-owning portable service seam, installed for the lifetime of a
    // HostSession. Renderer-free and headless windows may legitimately return
    // null when no host is attached.
    [[nodiscard]] HostServices* host_services() const noexcept {
        return host_services_;
    }

    [[nodiscard]] UpdateScope begin_update();
    void perform_layout();
    void flush();
    // Returns an exact presentation receipt only when a complete candidate
    // replay remains current for its surface epoch. Ignoring the result is
    // valid for renderer-neutral inspection; a native host must require it.
    std::optional<PaintReceipt> paint(
        Painter& painter, Rect requested_damage = {});
    // Preferred host release. Returns false for a duplicate, out-of-order, or
    // replaced-epoch receipt and never advances the presentation revision in
    // that case.
    [[nodiscard]] bool notify_presented(
        PaintReceipt receipt, std::uint64_t duration_nanoseconds = 0U);
    // Synchronous compatibility spelling. It acknowledges the latest complete
    // receipt in the current epoch; asynchronous/native hosts must use the
    // exact-receipt overload above.
    void notify_presented(std::uint64_t duration_nanoseconds = 0U);
    [[nodiscard]] PaintLeaseSnapshot paint_lease_snapshot() const noexcept;

    [[nodiscard]] bool queue_live_surface_presentation(
        const Control::Ptr& control, std::shared_ptr<LiveSurface> surface);
    [[nodiscard]] std::vector<LiveSurfacePresentation>
        take_live_surface_presentations();
    // A registration is retained while its control remains attached. Native
    // display clocks use this to stay armed without polling the control tree or
    // requiring one UI callback per producer publication.
    [[nodiscard]] bool has_live_surface_presentations() const noexcept {
        return !live_surface_registrations_.empty();
    }

    [[nodiscard]] DamageRegion take_damage();
    [[nodiscard]] DamageRegion take_damage(PaintPlane plane);
    [[nodiscard]] bool needs_frame() const noexcept;
    [[nodiscard]] std::optional<FrameTime> next_wake() const noexcept;
    [[nodiscard]] FrameRequestToken schedule_paint(const Control::Ptr& control,
                                                   FrameTime deadline);
    [[nodiscard]] FrameRequestToken activate_surface(const Control::Ptr& control,
                                                     FrameInterval interval,
                                                     FrameTime first_deadline);
    [[nodiscard]] FrameRequestToken schedule_ui_timer(
        Component& owner, FrameInterval interval, FrameTime first_deadline,
        std::function<void(FrameTime)> callback);
    [[nodiscard]] FramePollResult poll_frame_schedule(FrameTime now);
    void cancel_frame_requests();
    void set_occluded(bool occluded, FrameTime transition_time);
    [[nodiscard]] bool occluded() const noexcept { return occluded_; }
    [[nodiscard]] bool check_access() const noexcept;
    void verify_access(std::string_view operation = "window access");
    [[nodiscard]] bool invoke_required() const noexcept { return !check_access(); }
    [[nodiscard]] DispatchOperation begin_invoke(std::function<void()> callback);
    [[nodiscard]] DispatchOperation begin_invoke(
        const Control::Ptr& owner, std::function<void()> callback);
    // Synchronous Invoke never starts a nested message pump. UI-thread calls
    // execute inline; worker calls require an installed running host wake seam.
    void invoke(std::function<void()> callback);
    void invoke(const Control::Ptr& owner, std::function<void()> callback);
    [[nodiscard]] DispatchDrainResult drain_posted_work(
        std::size_t maximum_callbacks = maximum_callbacks_per_dispatch_turn);
    [[nodiscard]] DispatcherSnapshot dispatcher_snapshot() const noexcept;
    // Host adapters install a thread-safe wake primitive. Installing a handler
    // after work was queued immediately publishes one coalesced wake.
    void set_dispatch_wake_handler(std::function<void()> wake);
    // Host adapters also install a paint wake seam. Retained invalidation may
    // originate from another Window's callback, so relying only on the host
    // currently dispatching input can strand valid dirty state indefinitely.
    // Wakes coalesce until the host consumes damage and remain suppressed while
    // the model is explicitly occluded.
    void set_paint_wake_handler(std::function<void()> wake);
    void shutdown_dispatcher() noexcept;

    [[nodiscard]] ImageLoadResult load_png(std::span<const std::byte> encoded);
    [[nodiscard]] ImageLoadResult load_bgra32_premultiplied(
        std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes,
        std::span<const std::byte> pixels);
    [[nodiscard]] ImageLoadResult replace_png(ImageId image,
                                               std::span<const std::byte> encoded);
    [[nodiscard]] ImageLoadResult replace_png(ImageId image,
                                               std::span<const std::byte> encoded,
                                               Control& consumer);
    [[nodiscard]] ImageLoadResult replace_bgra32_premultiplied(
        ImageId image, std::uint32_t width, std::uint32_t height,
        std::uint64_t row_bytes, std::span<const std::byte> pixels,
        Control& consumer);
    [[nodiscard]] ImageLoadResult update_bgra32_premultiplied(
        ImageId image, std::uint32_t width, std::uint32_t height,
        std::uint64_t row_bytes, std::span<const std::byte> pixels,
        Control& consumer);
    [[nodiscard]] ImageLoadResult patch_bgra32_premultiplied(
        ImageId image, std::uint32_t x, std::uint32_t y,
        std::uint32_t width, std::uint32_t height,
        std::uint64_t source_row_bytes, std::span<const std::byte> pixels,
        Control& consumer, Rect local_damage);
    [[nodiscard]] bool remove_image(ImageId image);
    [[nodiscard]] const ImageRegistry& image_resources() const noexcept {
        return image_resources_;
    }
    [[nodiscard]] ImageRegistrySnapshot image_resource_snapshot() const noexcept {
        return image_resources_.snapshot();
    }

    [[nodiscard]] Control::Ptr find(std::string_view stable_id) const;
    [[nodiscard]] Control::Ptr hit_test(Point position);
    bool request_focus(const Control::Ptr& control);
    [[nodiscard]] Control::Ptr focused_control() const noexcept { return focused_.lock(); }
    bool validate_control(const Control::Ptr& control,
                          Control* destination = nullptr,
                          bool bulk = false);
    bool validate_children(
        const Control::Ptr& container,
        ValidationConstraints constraints = ValidationConstraints::selectable);
    [[nodiscard]] ValidationSnapshot validation_snapshot() const noexcept;
    [[nodiscard]] FocusScopeId begin_focus_scope(
        const Control::Ptr& root,
        const Control::Ptr& preferred_focus = {},
        FocusScopeOptions options = {});
    bool end_focus_scope(
        FocusScopeId scope,
        FocusScopeCloseReason reason = FocusScopeCloseReason::explicit_close);
    [[nodiscard]] std::size_t focus_scope_depth() const noexcept;
    [[nodiscard]] Control::Ptr active_focus_scope_root() const noexcept;
    bool move_focus(bool forward = true);
    void set_accept_button(const Control::Ptr& control);
    void set_cancel_button(const Control::Ptr& control);
    [[nodiscard]] Control::Ptr accept_button() const noexcept {
        return accept_button_.lock();
    }
    [[nodiscard]] Control::Ptr cancel_button() const noexcept {
        return cancel_button_.lock();
    }
    [[nodiscard]] DialogKeySnapshot dialog_key_snapshot() const noexcept;
    [[nodiscard]] DialogResult dialog_result() const noexcept {
        return dialog_result_;
    }
    void set_dialog_result(DialogResult result);
    [[nodiscard]] Event<DialogResult>& dialog_result_changed() noexcept {
        return dialog_result_changed_;
    }
    [[nodiscard]] Event<const FocusScopeChange&>& focus_scope_changed() noexcept {
        return focus_scope_changed_;
    }
    void capture_pointer(const Control::Ptr& control, std::uint64_t pointer_id = 1);
    void release_pointer();
    [[nodiscard]] Control::Ptr captured_control() const noexcept { return captured_.lock(); }
    [[nodiscard]] std::uint64_t captured_pointer_id() const noexcept {
        return captured_pointer_id_;
    }
    [[nodiscard]] Event<const PointerCaptureChange&>& pointer_capture_changed() noexcept {
        return pointer_capture_changed_;
    }
    [[nodiscard]] Event<const ControlAvailabilityChange&>&
    control_availability_changed() noexcept {
        return control_availability_changed_;
    }
    [[nodiscard]] PopupToken open_popup(const Control::Ptr& owner,
                                        const Control::Ptr& popup,
                                        PopupOptions options = {});
    // Window accelerators are tried only after the focused retained route
    // declines a key, preserving editor/menu ownership of their native keys.
    // Registrations are revoked by either the token or owner disposal.
    [[nodiscard]] AcceleratorToken register_accelerator(
        Component& owner, KeyGesture gesture, std::function<bool()> callback,
        AcceleratorOptions options = {});
    [[nodiscard]] Control::Ptr pressed_control() const noexcept { return pressed_.lock(); }

    // During an active paint lease these return true when the input was
    // retained for posted delivery and false only when the fixed queue bound
    // rejected it. Outside paint the return value remains the routed handled
    // result.
    bool dispatch_pointer(PointerEvent event);
    bool dispatch_key(KeyEvent event);
    bool dispatch_text(TextInputEvent event);
    [[nodiscard]] DeferredInputSnapshot deferred_input_snapshot() const noexcept;
    [[nodiscard]] DragDispatchResult dispatch_drag(DragEvent event);
    void cancel_drag() noexcept;

    [[nodiscard]] MetricsSnapshot metrics_snapshot() const { return metrics_.snapshot(); }
    Metrics& metrics() noexcept { return metrics_; }
    void reset_activity_metrics() noexcept { metrics_.reset_activity(); }
    [[nodiscard]] SemanticSnapshot semantic_snapshot();
    [[nodiscard]] std::uint64_t semantic_generation() const noexcept {
        return semantic_generation_;
    }
    bool perform_semantic_action(std::string_view stable_id,
                                 SemanticAction action,
                                 std::string_view value = {});

private:
    friend class HostSession;
    friend class Control;
    friend class UpdateScope;
    friend class Timer;
    friend class ToolTip;
    friend class ErrorProvider;
    friend class HelpProvider;
    friend class ImageList;
    friend class BindingSource;
    friend class BindingContext;
    friend class detail::PopupAttachment;
    friend class detail::AcceleratorAttachment;

    void attach_subtree(const Control::Ptr& control, const Control::WeakPtr& parent);
    void detach_subtree(const Control::Ptr& control);
    void dispose_subtree(const Control::Ptr& control) noexcept;
    void revoke_interaction_for_subtree(const Control::Ptr& control,
                                        bool notify_focus);
    void close_focus_scopes_for_subtree(const Control::Ptr& control);
    bool end_focus_scope(
        FocusScopeId scope, FocusScopeCloseReason reason,
        const Control::Ptr& notification_owner);
    void revoke_focus_scopes_for_subtree(const Control::Ptr& control) noexcept;
    void close_popups_for_subtree(const Control::Ptr& control) noexcept;
    void close_popup(detail::PopupAttachment& popup) noexcept;
    void close_accelerator(detail::AcceleratorAttachment& accelerator) noexcept;
    [[nodiscard]] bool dispatch_accelerator(const KeyEvent& event,
                                            bool preemptive);
    [[nodiscard]] bool focus_allowed_by_active_scope(
        const Control::Ptr& control) const noexcept;
    [[nodiscard]] std::vector<Control::Ptr> focus_candidates(
        const Control::Ptr& scope_root) const;
    [[nodiscard]] bool validate_focus_transition(
        const Control::Ptr& previous, const Control::Ptr& destination,
        AutoValidate mode);
    void change_pointer_capture(const Control::Ptr& control,
                                std::uint64_t pointer_id,
                                bool revoked);
    void on_eligibility_changed(const Control::Ptr& control);
    void on_hit_test_transparency_changed(const Control::Ptr& control);
    void publish_control_availability(Control& control);
    void register_subtree(const Control::Ptr& control);
    void unregister_subtree(const Control::Ptr& control);
    void mark_dirty(Control& control, Dirty dirty);
    void mark_paint_dirty(Control& control, Rect local_damage);
    void mark_subtree_dirty(Control& control, Dirty dirty);
    void mark_child_layout_slot(Control& control);
    void change_paint_plane(Control& control, PaintPlane plane);
    void add_damage(Rect damage, PaintPlane plane);
    void add_damage_all_planes(Rect damage);
    void request_paint_wake() noexcept;
    void acknowledge_paint_wake_if_damage_drained() noexcept;
    void touch_paint() noexcept;
    void update_paint_lease_state() noexcept;
    struct DeferredSemanticInput final {
        std::string stable_id;
        SemanticAction action{SemanticAction::focus};
        std::string value;
    };
    using DeferredInput = std::variant<
        PointerEvent, KeyEvent, TextInputEvent, DragEvent,
        DeferredSemanticInput>;
    [[nodiscard]] bool defer_input(DeferredInput input);
    void schedule_deferred_input_drain() noexcept;
    void drain_deferred_input();
    void abandon_deferred_input() noexcept;
    void abandon_deferred_drag() noexcept;
    void add_subtree_damage(const Control::Ptr& control);
    void ensure_layout(bool read_barrier);
    void leave_update_scope();
    void flush_if_outermost();
    [[nodiscard]] std::vector<Control::Ptr> route_to(const Control::Ptr& target) const;
    [[nodiscard]] Control::Ptr drop_target_at(Point position);
    [[nodiscard]] DragDispatchResult route_drag(const Control::Ptr& target,
                                                DragEvent event);
    [[nodiscard]] Control::Ptr hit_test_recursive(const Control::Ptr& control,
                                                  Point window_position) const;
    void paint_recursive(const Control::Ptr& control,
                         Painter& painter,
                         Rect window_damage,
                         PaintPlane plane,
                         std::uint64_t& visited_nodes,
                         std::uint64_t& painted_controls,
                         std::uint64_t& consumed_invalidations,
                         std::uint64_t& chunks_rebuilt,
                         std::uint64_t& chunks_reused,
                         std::uint64_t& commands_replayed);
    void measure_dirty_recursive(const Control::Ptr& control,
                                 Size available,
                                 std::uint64_t& visited_nodes,
                                 std::uint64_t& callbacks);
    void arrange_dirty_recursive(const Control::Ptr& control,
                                 Rect final_bounds,
                                 std::uint64_t& visited_nodes,
                                 std::uint64_t& callbacks);
    [[nodiscard]] bool has_runnable_layout_dirty(
        const Control::Ptr& control) const noexcept;
    void note_suspended_layout_request(Control& control) noexcept;
    void commit_layout_requests_recursive(const Control::Ptr& control) noexcept;
    [[nodiscard]] Dirty recompute_subtree_dirty(const Control::Ptr& control) noexcept;
    void clear_layout_dirty_subtree(const Control::Ptr& control) noexcept;
    void clear_paint_dirty_subtree(const Control::Ptr& control) noexcept;
    [[nodiscard]] std::uint64_t display_cache_entries(
        const Control::Ptr& control) const noexcept;
    void update_display_cache_metrics() noexcept;
    void compact_frame_requests() noexcept;
    [[nodiscard]] std::size_t active_surface_count() const noexcept;
    void update_frame_schedule_metrics() noexcept;
    [[nodiscard]] Rect absolute_bounds_of(const Control& control) const;
    [[nodiscard]] Rect visual_bounds_of(const Control& control,
                                        Insets outsets) const;
    [[nodiscard]] Rect paint_damage_bounds_of(const Control& control) const;
    [[nodiscard]] bool eligible(const Control::Ptr& control) const noexcept;
    [[nodiscard]] bool move_focus_after(const Control::Ptr& origin);
    [[nodiscard]] bool validate_command_activation(
        const Control::Ptr& destination);
    [[nodiscard]] bool dispatch_mnemonic(char32_t character);
    [[nodiscard]] bool dispatch_dialog_button(bool accept);
    void clear_dialog_targets_for_subtree(const Control::Ptr& control) noexcept;
    void require_ui_thread(std::string_view operation);

    Control::Ptr root_;
    Size client_size_{};
    double scale_{1.0};
    PresentationSettings presentation_settings_{};
    Event<const PresentationSettings&> presentation_changed_;
    std::shared_ptr<const Theme> theme_;
    Event<const Theme&> theme_changed_;
    Event<bool> active_changed_;
    bool active_{true};
    std::unordered_map<std::string, Control::WeakPtr> stable_ids_;
    Control::WeakPtr focused_;
    Control::WeakPtr accept_button_;
    Control::WeakPtr cancel_button_;
    std::uint64_t mnemonic_attempts_{};
    std::uint64_t mnemonics_handled_{};
    std::uint64_t accept_attempts_{};
    std::uint64_t accept_handled_{};
    std::uint64_t cancel_attempts_{};
    std::uint64_t cancel_handled_{};
    std::uint64_t dialog_command_rejections_{};
    std::uint64_t mnemonic_candidates_{};
    std::uint64_t mnemonic_collisions_{};
    std::uint64_t mnemonic_cycles_{};
    Control::WeakPtr mnemonic_cursor_;
    char32_t mnemonic_cursor_character_{};
    DialogResult dialog_result_{DialogResult::none};
    Event<DialogResult> dialog_result_changed_;
    std::uint64_t validation_attempts_{};
    std::uint64_t validation_succeeded_{};
    std::uint64_t validation_cancelled_{};
    std::uint64_t validation_focus_moves_blocked_{};
    std::uint64_t validation_reentrant_requests_rejected_{};
    std::uint64_t validation_bulk_controls_visited_{};
    bool validating_{};
    struct FocusScopeState final {
        FocusScopeId id{};
        Control::WeakPtr root;
        Control::WeakPtr previous_focus;
        std::string stable_id;
        RuntimeId root_id{};
        FocusScopeOptions options{};
        bool active{true};
    };
    std::vector<FocusScopeState> focus_scopes_;
    Event<const FocusScopeChange&> focus_scope_changed_;
    std::uint64_t next_focus_scope_id_{1U};
    Control::WeakPtr captured_;
    std::uint64_t captured_pointer_id_{};
    Event<const PointerCaptureChange&> pointer_capture_changed_;
    Event<const ControlAvailabilityChange&> control_availability_changed_;
    std::vector<std::shared_ptr<detail::PopupAttachment>> popups_;
    std::vector<std::shared_ptr<detail::AcceleratorAttachment>> accelerators_;
    Control::WeakPtr pressed_;
    Control::WeakPtr hovered_;
    Control::WeakPtr drag_target_;
    std::uint64_t drag_session_id_{};
    DragEffect drag_last_accepted_effect_{DragEffect::none};
    std::array<DamageRegion, paint_plane_count> plane_damage_;
    struct LiveSurfaceRegistration final {
        Control::WeakPtr control;
        std::shared_ptr<LiveSurface> surface;
        std::uint64_t sampled_epoch{};
        std::uint64_t sampled_generation{};
        bool sampled_with_overlay_clip{};
    };
    std::unordered_map<std::uint64_t, LiveSurfaceRegistration>
        live_surface_registrations_;
    Metrics metrics_;
    ImageRegistry image_resources_;
    std::uint64_t display_generation_{};
    std::vector<std::shared_ptr<detail::ScheduledFrameRequest>> frame_requests_;
    bool in_frame_poll_{};
    std::shared_ptr<detail::WindowLifetime> lifetime_;
    std::thread::id ui_thread_;
    std::shared_ptr<detail::DispatcherState> dispatcher_state_;
    std::uint64_t update_depth_{};
    bool layout_dirty_{true};
    bool paint_dirty_{true};
    bool hit_test_dirty_{true};
    bool in_layout_{};
    bool in_paint_{};
    PaintLeaseState paint_lease_state_{PaintLeaseState::dirty_queued};
    std::uint64_t content_revision_{1U};
    std::uint64_t rendered_revision_{};
    std::uint64_t presented_revision_{};
    std::uint64_t surface_epoch_{1U};
    std::uint64_t paint_leases_started_{};
    std::uint64_t paint_leases_completed_{};
    std::uint64_t paint_leases_abandoned_{};
    std::uint64_t reentrant_paint_requests_deferred_{};
    std::uint64_t paint_wakes_queued_{};
    std::uint64_t paint_wakes_coalesced_{};
    std::uint64_t presentation_receipts_accepted_{};
    std::uint64_t presentation_receipts_rejected_{};
    bool dirty_after_render_{};
    std::deque<DeferredInput> deferred_inputs_;
    std::uint64_t deferred_inputs_received_{};
    std::uint64_t deferred_inputs_delivered_{};
    std::uint64_t deferred_input_moves_coalesced_{};
    std::uint64_t deferred_drag_overs_coalesced_{};
    std::uint64_t deferred_inputs_rejected_{};
    std::uint64_t deferred_inputs_abandoned_{};
    std::uint64_t deferred_input_faults_{};
    std::uint64_t deferred_input_drains_{};
    DispatchOperation deferred_input_drain_operation_;
    bool deferred_input_drain_queued_{};
    bool draining_deferred_input_{};
    bool in_lifecycle_notification_{};
    bool second_layout_pass_requested_{};
    bool occluded_{};
    std::function<void()> paint_wake_handler_;
    bool paint_wake_pending_{};
    std::uint64_t semantic_generation_{1U};
    bool in_semantic_snapshot_{};
    HostServices* host_services_{};
};

class UpdateScope {
public:
    explicit UpdateScope(Window& window) noexcept : window_(&window) {}
    ~UpdateScope();
    UpdateScope(UpdateScope&& other) noexcept;
    UpdateScope& operator=(UpdateScope&& other) noexcept;
    UpdateScope(const UpdateScope&) = delete;
    UpdateScope& operator=(const UpdateScope&) = delete;

    void perform_layout();
    void close();

private:
    Window* window_{};
};

} // namespace gui_forms
