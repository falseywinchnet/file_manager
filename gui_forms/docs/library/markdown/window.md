# Window

- Status: **OBSERVED: bundle 007 retained-window hierarchy and lifecycle/dispatcher split; M4 macOS and MinGW builds, focused tests, and Screen Sharing pass**
- Kind: **class**
- Hierarchy: `Window`
- Declaration: `include/gui_forms/window/window.hpp:215`
- Definition: `src/core/window/dispatcher/window_dispatcher.cpp, src/core/window/lifecycle/window_lifecycle.cpp, src/core/window/presentation/window_presentation.cpp, src/core/window/scheduler/window_scheduler.cpp, src/core/window/window.cpp`

Window is GUI.Forms' portable retained-root coordinator. It owns exact tree identity, update/layout/paint transactions, damage and presentation receipts, live layers, frame requests, UI-thread dispatch, resources, focus/validation/dialog keys, pointer/drag/deferred input, popups and accelerators, metrics, and semantic projection; native adapters reach it only through the host session and wake seams.

## Visual evidence

![Window](../captures/native_window_host.png)

## Declared methods

### `Window` (public)

```cpp
explicit Window(Control::Ptr root, Size client_size =
```

Constructs one retained root on the creating UI thread, creates bounded dispatcher/frame lifetime state, attaches the tree, and seeds complete initial damage; copying is prohibited because identity and native attachment are singular.

### `~Window` (public)

```cpp
~Window()
```

Revokes wakes, deferred input, dispatch work, accelerators, popups, focus scopes, frame requests, and retained attachment in dependency order before retiring the shared lifetime token.

### `Window` (public)

```cpp
Window(const Window&) = delete
```

Constructs one retained root on the creating UI thread, creates bounded dispatcher/frame lifetime state, attaches the tree, and seeds complete initial damage; copying is prohibited because identity and native attachment are singular.

### `operator=` (public)

```cpp
Window& operator=(const Window&) = delete
```

Is deleted so retained identity, UI-thread affinity, leases, and native-host association cannot be duplicated.

### `root` (public)

```cpp
[[nodiscard]] Control::Ptr root() const noexcept
```

Returns the singular retained root owned for the Window lifetime.

### `client_size` (public)

```cpp
[[nodiscard]] Size client_size() const noexcept
```

Returns logical client extent independent of device scale.

### `resize` (public)

```cpp
void resize(Size client_size)
```

Validates nonnegative logical extent, advances the surface epoch, damages the old extent, and invalidates root and overlay geometry.

### `set_scale` (public)

```cpp
void set_scale(double scale)
```

Validates a finite positive device scale, advances the surface epoch, and conservatively invalidates all retained roots.

### `scale` (public)

```cpp
[[nodiscard]] double scale() const noexcept
```

Returns native device-pixel scale without conflating it with presentation text scale.

### `presentation_settings` (public)

```cpp
[[nodiscard]] const PresentationSettings& presentation_settings() const noexcept
```

Returns logical text-scale, contrast, motion, and sound preferences.

### `set_presentation_settings` (public)

```cpp
void set_presentation_settings(PresentationSettings settings)
```

Validates bounded text scale, dismisses geometry-sensitive transients when it changes, invalidates retained presentation, and publishes one committed event.

### `set_text_scale` (public)

```cpp
void set_text_scale(double text_scale)
```

Changes only logical text scale through the complete presentation-settings transaction.

### `presentation_changed` (public)

```cpp
[[nodiscard]] Event<const PresentationSettings&>& presentation_changed() noexcept
```

Returns the event published after presentation settings commit.

### `theme` (public)

```cpp
[[nodiscard]] const Theme& theme() const noexcept
```

Returns the active immutable theme by reference.

### `theme_ptr` (public)

```cpp
[[nodiscard]] std::shared_ptr<const Theme> theme_ptr() const noexcept
```

Returns shared ownership of the active immutable theme.

### `set_theme` (public)

```cpp
void set_theme(std::shared_ptr<const Theme> theme)
```

Rejects null, replaces immutable theme state, conservatively invalidates root and overlay style/layout/semantics, and publishes after commit.

### `theme_changed` (public)

```cpp
[[nodiscard]] Event<const Theme&>& theme_changed() noexcept
```

Returns the post-commit theme event.

### `active` (public)

```cpp
[[nodiscard]] bool active() const noexcept
```

Reports portable top-level activation state.

### `set_active` (public)

```cpp
void set_active(bool active)
```

Commits a distinct activation value, refreshes activation-dependent style/paint/semantics across roots, and publishes after commit.

### `active_changed` (public)

```cpp
[[nodiscard]] Event<bool>& active_changed() noexcept
```

Returns the post-commit activation event.

### `host_services` (public)

```cpp
[[nodiscard]] HostServices* host_services() const noexcept
```

Returns the non-owning service seam installed only for an active HostSession.

### `begin_update` (public)

```cpp
[[nodiscard]] UpdateScope begin_update()
```

Enters a nestable update transaction, records depth metrics, and returns the sole close token for that level.

### `perform_layout` (public)

```cpp
void perform_layout()
```

Runs the bounded retained layout barrier even inside an update scope without publishing paint.

### `flush` (public)

```cpp
void flush()
```

Runs the retained layout barrier; paint remains host-demand driven.

### `paint` (public)

```cpp
std::optional<PaintReceipt> paint( Painter& painter, Rect requested_damage =
```

Acquires the exclusive revision/epoch lease, applies bounded layout, replays damaged retained planes, produces an exact receipt only for a coherent current result, and defers reentrant input/paint.

### `notify_presented` (public)

```cpp
[[nodiscard]] bool notify_presented( PaintReceipt receipt, std::uint64_t duration_nanoseconds = 0U)
```

Validates an exact receipt or uses the synchronous compatibility receipt, advances presentation metrics only in order, and rejects duplicates or retired epochs.

### `notify_presented` (public)

```cpp
void notify_presented(std::uint64_t duration_nanoseconds = 0U)
```

Validates an exact receipt or uses the synchronous compatibility receipt, advances presentation metrics only in order, and rejects duplicates or retired epochs.

### `paint_lease_snapshot` (public)

```cpp
[[nodiscard]] PaintLeaseSnapshot paint_lease_snapshot() const noexcept
```

Returns revision, epoch, lease, wake, deferral, and receipt counters plus current lease state.

### `queue_live_surface_presentation` (public)

```cpp
[[nodiscard]] bool queue_live_surface_presentation( const Control::Ptr& control, std::shared_ptr<LiveSurface> surface)
```

Registers or refreshes one attached control's durable producer surface without converting producer publication into retained repaint callbacks.

### `take_live_surface_presentations` (public)

```cpp
[[nodiscard]] std::vector<LiveSurfacePresentation> take_live_surface_presentations()
```

Samples registered live surfaces into immutable geometry/clip placements for native composition and retires dead registrations.

### `has_live_surface_presentations` (public)

```cpp
[[nodiscard]] bool has_live_surface_presentations() const noexcept
```

Reports whether durable live-layer registrations keep a native display clock relevant.

### `take_damage` (public)

```cpp
[[nodiscard]] DamageRegion take_damage()
```

Moves pending aggregate or named-plane damage to the host and acknowledges the paint wake when every plane drains.

### `take_damage` (public)

```cpp
[[nodiscard]] DamageRegion take_damage(PaintPlane plane)
```

Moves pending aggregate or named-plane damage to the host and acknowledges the paint wake when every plane drains.

### `needs_frame` (public)

```cpp
[[nodiscard]] bool needs_frame() const noexcept
```

Reports retained damage or due scheduled work without polling controls.

### `next_wake` (public)

```cpp
[[nodiscard]] std::optional<FrameTime> next_wake() const noexcept
```

Returns the earliest eligible frame/timer deadline, respecting occlusion.

### `schedule_paint` (public)

```cpp
[[nodiscard]] FrameRequestToken schedule_paint(const Control::Ptr& control, FrameTime deadline)
```

Adds one bounded, revocable deadline request for an attached control.

### `activate_surface` (public)

```cpp
[[nodiscard]] FrameRequestToken activate_surface(const Control::Ptr& control, FrameInterval interval, FrameTime first_deadline)
```

Adds a bounded periodic surface request with an enforced minimum interval.

### `schedule_ui_timer` (public)

```cpp
[[nodiscard]] FrameRequestToken schedule_ui_timer( Component& owner, FrameInterval interval, FrameTime first_deadline, std::function<void(FrameTime)> callback)
```

Adds a bounded periodic callback for a live Component owner with fault isolation and a minimum interval.

### `poll_frame_schedule` (public)

```cpp
[[nodiscard]] FramePollResult poll_frame_schedule(FrameTime now)
```

Runs one nonreentrant bounded schedule turn, coalesces due work, disconnects faulting callbacks, invalidates due surfaces, and returns the next wake.

### `cancel_frame_requests` (public)

```cpp
void cancel_frame_requests()
```

Revokes every outstanding frame/surface/timer request and refreshes scheduling metrics.

### `set_occluded` (public)

```cpp
void set_occluded(bool occluded, FrameTime transition_time)
```

Transitions scheduling and paint leases between visible and occluded policy without discarding dirty state.

### `occluded` (public)

```cpp
[[nodiscard]] bool occluded() const noexcept
```

Reports portable host occlusion state.

### `check_access` (public)

```cpp
[[nodiscard]] bool check_access() const noexcept
```

Compares the caller with immutable Window UI-thread affinity.

### `verify_access` (public)

```cpp
void verify_access(std::string_view operation = "window access")
```

Throws a named wrong-thread failure and records its metric when access is invalid.

### `invoke_required` (public)

```cpp
[[nodiscard]] bool invoke_required() const noexcept
```

Reports whether caller marshaling is required.

### `begin_invoke` (public)

```cpp
[[nodiscard]] DispatchOperation begin_invoke(std::function<void()> callback)
```

Posts unowned or control-owned work into the bounded FIFO dispatcher and coalesces one host wake.

### `begin_invoke` (public)

```cpp
[[nodiscard]] DispatchOperation begin_invoke( const Control::Ptr& owner, std::function<void()> callback)
```

Posts unowned or control-owned work into the bounded FIFO dispatcher and coalesces one host wake.

### `invoke` (public)

```cpp
void invoke(std::function<void()> callback)
```

Runs inline on the UI thread or posts and blocks without a nested message pump when a host wake seam exists.

### `invoke` (public)

```cpp
void invoke(const Control::Ptr& owner, std::function<void()> callback)
```

Runs inline on the UI thread or posts and blocks without a nested message pump when a host wake seam exists.

### `drain_posted_work` (public)

```cpp
[[nodiscard]] DispatchDrainResult drain_posted_work( std::size_t maximum_callbacks = maximum_callbacks_per_dispatch_turn)
```

Consumes one bounded queue snapshot in FIFO order, validates owners, isolates faults/cancellation, updates telemetry, and rearms one wake if work remains.

### `dispatcher_snapshot` (public)

```cpp
[[nodiscard]] DispatcherSnapshot dispatcher_snapshot() const noexcept
```

Returns thread-safe queue, outcome, invocation, wake, bound, and shutdown telemetry.

### `set_dispatch_wake_handler` (public)

```cpp
void set_dispatch_wake_handler(std::function<void()> wake)
```

Installs or removes the host wake primitive, immediately wakes queued work, and refuses removal while a synchronous waiter exists.

### `set_paint_wake_handler` (public)

```cpp
void set_paint_wake_handler(std::function<void()> wake)
```

Installs the host paint wake primitive used by cross-window or asynchronous retained invalidation.

### `shutdown_dispatcher` (public)

```cpp
void shutdown_dispatcher() noexcept
```

Atomically stops acceptance, removes the wake seam, abandons deferred input on the UI thread, cancels pending work, and releases waiters.

### `load_png` (public)

```cpp
[[nodiscard]] ImageLoadResult load_png(std::span<const std::byte> encoded)
```

Loads the sole admitted encoded image format into the generational resource registry on the UI thread.

### `load_bgra32_premultiplied` (public)

```cpp
[[nodiscard]] ImageLoadResult load_bgra32_premultiplied( std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes, std::span<const std::byte> pixels)
```

Loads validated raw premultiplied pixels into the generational resource registry.

### `replace_png` (public)

```cpp
[[nodiscard]] ImageLoadResult replace_png(ImageId image, std::span<const std::byte> encoded)
```

Replaces an existing PNG generation globally or for a named attached consumer and applies the appropriate conservative or scoped invalidation.

### `replace_png` (public)

```cpp
[[nodiscard]] ImageLoadResult replace_png(ImageId image, std::span<const std::byte> encoded, Control& consumer)
```

Replaces an existing PNG generation globally or for a named attached consumer and applies the appropriate conservative or scoped invalidation.

### `replace_bgra32_premultiplied` (public)

```cpp
[[nodiscard]] ImageLoadResult replace_bgra32_premultiplied( ImageId image, std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes, std::span<const std::byte> pixels, Control& consumer)
```

Replaces raw pixels for an attached consumer and invalidates its paint/semantic projection.

### `update_bgra32_premultiplied` (public)

```cpp
[[nodiscard]] ImageLoadResult update_bgra32_premultiplied( ImageId image, std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes, std::span<const std::byte> pixels, Control& consumer)
```

Publishes a live raw-pixel generation for an attached consumer without admitting renderer types.

### `patch_bgra32_premultiplied` (public)

```cpp
[[nodiscard]] ImageLoadResult patch_bgra32_premultiplied( ImageId image, std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height, std::uint64_t source_row_bytes, std::span<const std::byte> pixels, Control& consumer, Rect local_damage)
```

Validates and patches a raw subregion, then applies exact consumer-local damage.

### `remove_image` (public)

```cpp
[[nodiscard]] bool remove_image(ImageId image)
```

Retires a generational image and invalidates all display chunks because the unscoped API cannot name consumers.

### `image_resources` (public)

```cpp
[[nodiscard]] const ImageRegistry& image_resources() const noexcept
```

Returns the renderer-neutral image registry.

### `image_resource_snapshot` (public)

```cpp
[[nodiscard]] ImageRegistrySnapshot image_resource_snapshot() const noexcept
```

Returns generational image counts and byte/accounting telemetry.

### `find` (public)

```cpp
[[nodiscard]] Control::Ptr find(std::string_view stable_id) const
```

Resolves an attached control by authoritative unique stable ID.

### `hit_test` (public)

```cpp
[[nodiscard]] Control::Ptr hit_test(Point position)
```

Applies the layout read barrier and finds the frontmost eligible retained target across overlays and root.

### `request_focus` (public)

```cpp
bool request_focus(const Control::Ptr& control)
```

Validates attachment, eligibility, active focus-scope containment, and AutoValidate before committing focus events.

### `focused_control` (public)

```cpp
[[nodiscard]] Control::Ptr focused_control() const noexcept
```

Returns the currently focused live control if retained.

### `validate_control` (public)

```cpp
bool validate_control(const Control::Ptr& control, Control* destination = nullptr, bool bulk = false)
```

Runs one nonreentrant cancelable validation transaction for an eligible control and records exact outcome metrics.

### `validate_children` (public)

```cpp
bool validate_children( const Control::Ptr& container, ValidationConstraints constraints = ValidationConstraints::selectable)
```

Traverses a bounded retained subtree under declared validation constraints and stops on cancellation.

### `validation_snapshot` (public)

```cpp
[[nodiscard]] ValidationSnapshot validation_snapshot() const noexcept
```

Returns validation attempts, outcomes, blocked focus moves, reentrancy, visits, and active state.

### `begin_focus_scope` (public)

```cpp
[[nodiscard]] FocusScopeId begin_focus_scope( const Control::Ptr& root, const Control::Ptr& preferred_focus =
```

Validates nested ownership and depth, captures previous focus, optionally focuses a preferred/first descendant, and publishes the opened scope.

### `end_focus_scope` (public)

```cpp
bool end_focus_scope( FocusScopeId scope, FocusScopeCloseReason reason = FocusScopeCloseReason::explicit_close)
```

Closes the active matching scope, applies the named reason, optionally restores eligible prior focus, and publishes after state commit.

### `focus_scope_depth` (public)

```cpp
[[nodiscard]] std::size_t focus_scope_depth() const noexcept
```

Returns active transient/modal focus containment depth.

### `active_focus_scope_root` (public)

```cpp
[[nodiscard]] Control::Ptr active_focus_scope_root() const noexcept
```

Returns the live root of the innermost active focus scope.

### `move_focus` (public)

```cpp
bool move_focus(bool forward = true)
```

Builds stable eligible tab-order candidates within the active scope and commits the next validated destination.

### `set_accept_button` (public)

```cpp
void set_accept_button(const Control::Ptr& control)
```

Validates and retains the attached command target used for Return dialog routing.

### `set_cancel_button` (public)

```cpp
void set_cancel_button(const Control::Ptr& control)
```

Validates and retains the attached command target used for Escape dialog routing.

### `accept_button` (public)

```cpp
[[nodiscard]] Control::Ptr accept_button() const noexcept
```

Returns the current live default command target.

### `cancel_button` (public)

```cpp
[[nodiscard]] Control::Ptr cancel_button() const noexcept
```

Returns the current live cancellation command target.

### `dialog_key_snapshot` (public)

```cpp
[[nodiscard]] DialogKeySnapshot dialog_key_snapshot() const noexcept
```

Returns mnemonic/default/cancel attempts, outcomes, candidates, collisions, cycles, and rejections.

### `dialog_result` (public)

```cpp
[[nodiscard]] DialogResult dialog_result() const noexcept
```

Returns the committed portable dialog result.

### `set_dialog_result` (public)

```cpp
void set_dialog_result(DialogResult result)
```

Validates a defined result, commits a distinct value, and publishes it.

### `dialog_result_changed` (public)

```cpp
[[nodiscard]] Event<DialogResult>& dialog_result_changed() noexcept
```

Returns the post-commit dialog-result event.

### `focus_scope_changed` (public)

```cpp
[[nodiscard]] Event<const FocusScopeChange&>& focus_scope_changed() noexcept
```

Returns opened/closed focus-scope publications.

### `capture_pointer` (public)

```cpp
void capture_pointer(const Control::Ptr& control, std::uint64_t pointer_id = 1)
```

Validates an attached eligible control and commits portable pointer capture for a nonzero pointer identity.

### `release_pointer` (public)

```cpp
void release_pointer()
```

Revokes current portable pointer capture and publishes the transition.

### `captured_control` (public)

```cpp
[[nodiscard]] Control::Ptr captured_control() const noexcept
```

Returns the current live retained capture owner.

### `captured_pointer_id` (public)

```cpp
[[nodiscard]] std::uint64_t captured_pointer_id() const noexcept
```

Returns the captured pointer identity or zero.

### `pointer_capture_changed` (public)

```cpp
[[nodiscard]] Event<const PointerCaptureChange&>& pointer_capture_changed() noexcept
```

Returns capture transitions consumed by HostSession services.

### `control_availability_changed` (public)

```cpp
[[nodiscard]] Event<const ControlAvailabilityChange&>& control_availability_changed() noexcept
```

Returns effective visibility/enabled transitions for providers and native projection.

### `open_popup` (public)

```cpp
[[nodiscard]] PopupToken open_popup(const Control::Ptr& owner, const Control::Ptr& popup, PopupOptions options =
```

Validates owner policy and a live detached popup root, attaches it as a retained overlay, binds revocable owner lifetime, and returns the close token.

### `register_accelerator` (public)

```cpp
[[nodiscard]] AcceleratorToken register_accelerator( Component& owner, KeyGesture gesture, std::function<bool()> callback, AcceleratorOptions options =
```

Validates owner/gesture/callback, records priority policy, binds owner lifetime, and returns the revocation token.

### `pressed_control` (public)

```cpp
[[nodiscard]] Control::Ptr pressed_control() const noexcept
```

Returns the control retaining qualified pointer press state.

### `dispatch_pointer` (public)

```cpp
bool dispatch_pointer(PointerEvent event)
```

Defers safely under a paint lease or routes preview/target/bubble pointer input with hover, press, capture, focus, and mutation revalidation.

### `dispatch_key` (public)

```cpp
bool dispatch_key(KeyEvent event)
```

Defers safely under paint, arbitrates preemptive accelerators, focus route, dialog keys/mnemonics, ordinary accelerators, and traversal in deterministic order.

### `dispatch_text` (public)

```cpp
bool dispatch_text(TextInputEvent event)
```

Defers safely under paint or routes committed/composition text through the focused retained route.

### `deferred_input_snapshot` (public)

```cpp
[[nodiscard]] DeferredInputSnapshot deferred_input_snapshot() const noexcept
```

Returns fixed-capacity queue, delivery, compaction, rejection, abandonment, fault, and drain telemetry.

### `dispatch_drag` (public)

```cpp
[[nodiscard]] DragDispatchResult dispatch_drag(DragEvent event)
```

Validates session ordering and typed payload, defers under paint, routes enter/over/leave/drop, and enforces one admitted effect.

### `cancel_drag` (public)

```cpp
void cancel_drag() noexcept
```

Terminates retained drag ownership and abandons deferred drag work.

### `metrics_snapshot` (public)

```cpp
[[nodiscard]] MetricsSnapshot metrics_snapshot() const
```

Returns structured retained layout/paint/input/semantic/resource activity metrics.

### `metrics` (public)

```cpp
Metrics& metrics() noexcept
```

Returns mutable structured instrumentation for internal coordinated emitters.

### `reset_activity_metrics` (public)

```cpp
void reset_activity_metrics() noexcept
```

Resets interval activity counters without erasing lifetime/identity state.

### `semantic_snapshot` (public)

```cpp
[[nodiscard]] SemanticSnapshot semantic_snapshot()
```

Builds a mutation-safe bounded semantic tree across root and overlays, retrying when callbacks mutate identity.

### `semantic_generation` (public)

```cpp
[[nodiscard]] std::uint64_t semantic_generation() const noexcept
```

Returns the generation native accessibility adapters use to avoid redundant publication.

### `perform_semantic_action` (public)

```cpp
bool perform_semantic_action(std::string_view stable_id, SemanticAction action, std::string_view value =
```

Defers during paint or resolves stable real/virtual identity and enters the same focus, press, value, disclosure, and scroll paths as ordinary input.

### `attach_subtree` (private)

```cpp
void attach_subtree(const Control::Ptr& control, const Control::WeakPtr& parent)
```

Recursively validates ownership and stable identity, installs Window/dispatcher affinity, and publishes lifecycle attachment.

### `detach_subtree` (private)

```cpp
void detach_subtree(const Control::Ptr& control)
```

Revokes interaction/transients, unregisters identity/dispatcher affinity, and publishes lifecycle detachment without implicit disposal.

### `dispose_subtree` (private)

```cpp
void dispose_subtree(const Control::Ptr& control) noexcept
```

Performs noexcept retained teardown, revoking dependents before irreversible disposed state.

### `revoke_interaction_for_subtree` (private)

```cpp
void revoke_interaction_for_subtree(const Control::Ptr& control, bool notify_focus)
```

Clears focus, capture, pressed, hover, drag, dialogs, scopes, popups, accelerators, and scheduled work owned by a departing subtree.

### `close_focus_scopes_for_subtree` (private)

```cpp
void close_focus_scopes_for_subtree(const Control::Ptr& control)
```

Closes active scopes rooted within a subtree through ordinary notifications.

### `end_focus_scope` (private)

```cpp
bool end_focus_scope( FocusScopeId scope, FocusScopeCloseReason reason, const Control::Ptr& notification_owner)
```

Closes the active matching scope, applies the named reason, optionally restores eligible prior focus, and publishes after state commit.

### `revoke_focus_scopes_for_subtree` (private)

```cpp
void revoke_focus_scopes_for_subtree(const Control::Ptr& control) noexcept
```

Noexcept-removes departing scope state during teardown.

### `close_popups_for_subtree` (private)

```cpp
void close_popups_for_subtree(const Control::Ptr& control) noexcept
```

Disconnects every popup whose owner belongs to the named subtree.

### `close_popup` (private)

```cpp
void close_popup(detail::PopupAttachment& popup) noexcept
```

Revokes overlay ownership, detaches its retained root, and publishes close through the owner's ordered change channel when possible.

### `close_accelerator` (private)

```cpp
void close_accelerator(detail::AcceleratorAttachment& accelerator) noexcept
```

Revokes and removes one exact accelerator attachment.

### `dispatch_accelerator` (private)

```cpp
[[nodiscard]] bool dispatch_accelerator(const KeyEvent& event, bool preemptive)
```

Tests a stable reverse-registration snapshot at the requested preemptive phase and invokes only live owners.

### `focus_allowed_by_active_scope` (private)

```cpp
[[nodiscard]] bool focus_allowed_by_active_scope( const Control::Ptr& control) const noexcept
```

Checks containment against the innermost active focus scope.

### `focus_candidates` (private)

```cpp
[[nodiscard]] std::vector<Control::Ptr> focus_candidates( const Control::Ptr& scope_root) const
```

Builds stable tab-index/tree-order eligible candidates inside a scope root.

### `validate_focus_transition` (private)

```cpp
[[nodiscard]] bool validate_focus_transition( const Control::Ptr& previous, const Control::Ptr& destination, AutoValidate mode)
```

Applies the source container's AutoValidate policy before a focus commit.

### `change_pointer_capture` (private)

```cpp
void change_pointer_capture(const Control::Ptr& control, std::uint64_t pointer_id, bool revoked)
```

Commits exact capture identity, publishes revoked/ordinary transitions, and synchronizes semantic state.

### `on_eligibility_changed` (private)

```cpp
void on_eligibility_changed(const Control::Ptr& control)
```

Revokes interaction invalidated by effective enabled/visible changes and publishes availability.

### `on_hit_test_transparency_changed` (private)

```cpp
void on_hit_test_transparency_changed(const Control::Ptr& control)
```

Revokes now-invalid hover/press/capture targets and refreshes hit-test state.

### `publish_control_availability` (private)

```cpp
void publish_control_availability(Control& control)
```

Publishes stable/runtime identity with current effective visibility and enabled state.

### `register_subtree` (private)

```cpp
void register_subtree(const Control::Ptr& control)
```

Enforces unique stable IDs recursively before a subtree becomes addressable.

### `unregister_subtree` (private)

```cpp
void unregister_subtree(const Control::Ptr& control)
```

Removes authoritative stable-ID mappings recursively.

### `mark_dirty` (private)

```cpp
void mark_dirty(Control& control, Dirty dirty)
```

Promotes typed invalidation through layout/style/paint/hit-test/semantic dependencies, cache retirement, damage, metrics, and wake coalescing.

### `mark_paint_dirty` (private)

```cpp
void mark_paint_dirty(Control& control, Rect local_damage)
```

Transforms exact local damage through visual outsets into the control's paint plane.

### `mark_subtree_dirty` (private)

```cpp
void mark_subtree_dirty(Control& control, Dirty dirty)
```

Recursively applies typed invalidation while respecting lifecycle and layout suspension.

### `mark_child_layout_slot` (private)

```cpp
void mark_child_layout_slot(Control& control)
```

Invalidates the exact parent layout responsibility for a changed child.

### `change_paint_plane` (private)

```cpp
void change_paint_plane(Control& control, PaintPlane plane)
```

Damages old/new projections and migrates retained paint-plane ownership.

### `add_damage` (private)

```cpp
void add_damage(Rect damage, PaintPlane plane)
```

Clips and records exact damage in one retained paint plane.

### `add_damage_all_planes` (private)

```cpp
void add_damage_all_planes(Rect damage)
```

Records the same clipped damage across every plane.

### `request_paint_wake` (private)

```cpp
void request_paint_wake() noexcept
```

Coalesces one thread-safe host paint wake unless occluded or already pending.

### `acknowledge_paint_wake_if_damage_drained` (private)

```cpp
void acknowledge_paint_wake_if_damage_drained() noexcept
```

Clears the pending wake only after all retained damage drains.

### `touch_paint` (private)

```cpp
void touch_paint() noexcept
```

Advances nonzero content revision and transitions the lease state for a new mutation.

### `update_paint_lease_state` (private)

```cpp
void update_paint_lease_state() noexcept
```

Derives the legal lease state from occlusion, active render, dirty revision, and ready/presented progress.

### `defer_input` (private)

```cpp
[[nodiscard]] bool defer_input(DeferredInput input)
```

Queues bounded critical input during paint while compacting only replaceable moves/drag-overs.

### `schedule_deferred_input_drain` (private)

```cpp
void schedule_deferred_input_drain() noexcept
```

Posts one coalesced dispatcher drain after the paint lease releases.

### `drain_deferred_input` (private)

```cpp
void drain_deferred_input()
```

Delivers one ordered deferred batch outside paint with per-event fault accounting.

### `abandon_deferred_input` (private)

```cpp
void abandon_deferred_input() noexcept
```

Cancels queued deferred input and its posted drain during shutdown.

### `abandon_deferred_drag` (private)

```cpp
void abandon_deferred_drag() noexcept
```

Removes only deferred drag events when a drag session terminates.

### `add_subtree_damage` (private)

```cpp
void add_subtree_damage(const Control::Ptr& control)
```

Adds current and last-painted visual extents for a subtree.

### `ensure_layout` (private)

```cpp
void ensure_layout(bool read_barrier)
```

Runs bounded measure/arrange passes across root and overlays, respects update scopes/suspension, and records second-pass requests.

### `leave_update_scope` (private)

```cpp
void leave_update_scope()
```

Validates balanced nesting, updates metrics, and flushes layout only at the outer boundary.

### `flush_if_outermost` (private)

```cpp
void flush_if_outermost()
```

Applies the retained layout barrier when no update transaction remains.

### `route_to` (private)

```cpp
[[nodiscard]] std::vector<Control::Ptr> route_to(const Control::Ptr& target) const
```

Builds root-to-target retained ancestry for preview/target/bubble dispatch.

### `drop_target_at` (private)

```cpp
[[nodiscard]] Control::Ptr drop_target_at(Point position)
```

Finds the frontmost eligible typed-drop target across overlays and root.

### `route_drag` (private)

```cpp
[[nodiscard]] DragDispatchResult route_drag(const Control::Ptr& target, DragEvent event)
```

Routes one typed drag event with mutation revalidation and accepted-effect enforcement.

### `hit_test_recursive` (private)

```cpp
[[nodiscard]] Control::Ptr hit_test_recursive(const Control::Ptr& control, Point window_position) const
```

Searches visible enabled retained children front-to-back with inverse coordinate mapping.

### `paint_recursive` (private)

```cpp
void paint_recursive(const Control::Ptr& control, Painter& painter, Rect window_damage, PaintPlane plane, std::uint64_t& visited_nodes, std::uint64_t& painted_controls, std::uint64_t& consumed_invalidations, std::uint64_t& chunks_rebuilt, std::uint64_t& chunks_reused, std::uint64_t& commands_replayed)
```

Rebuilds or replays display chunks within damage, clip, plane, and lifecycle bounds while recording cache metrics.

### `measure_dirty_recursive` (private)

```cpp
void measure_dirty_recursive(const Control::Ptr& control, Size available, std::uint64_t& visited_nodes, std::uint64_t& callbacks)
```

Measures only runnable dirty nodes and propagates bounded desired-size changes.

### `arrange_dirty_recursive` (private)

```cpp
void arrange_dirty_recursive(const Control::Ptr& control, Rect final_bounds, std::uint64_t& visited_nodes, std::uint64_t& callbacks)
```

Arranges only runnable dirty nodes, commits geometry, and propagates resulting damage.

### `has_runnable_layout_dirty` (private)

```cpp
[[nodiscard]] bool has_runnable_layout_dirty( const Control::Ptr& control) const noexcept
```

Detects layout work not blocked by suspension.

### `note_suspended_layout_request` (private)

```cpp
void note_suspended_layout_request(Control& control) noexcept
```

Records a coalesced request owned by a suspended subtree.

### `commit_layout_requests_recursive` (private)

```cpp
void commit_layout_requests_recursive(const Control::Ptr& control) noexcept
```

Promotes deferred suspended requests after layout resumes.

### `recompute_subtree_dirty` (private)

```cpp
[[nodiscard]] Dirty recompute_subtree_dirty(const Control::Ptr& control) noexcept
```

Re-derives aggregate dirtiness from exact node and child state.

### `clear_layout_dirty_subtree` (private)

```cpp
void clear_layout_dirty_subtree(const Control::Ptr& control) noexcept
```

Clears completed measure/arrange dirtiness recursively.

### `clear_paint_dirty_subtree` (private)

```cpp
void clear_paint_dirty_subtree(const Control::Ptr& control) noexcept
```

Clears completed paint dirtiness recursively after coherent replay.

### `display_cache_entries` (private)

```cpp
[[nodiscard]] std::uint64_t display_cache_entries( const Control::Ptr& control) const noexcept
```

Counts retained display chunks recursively for structured metrics.

### `update_display_cache_metrics` (private)

```cpp
void update_display_cache_metrics() noexcept
```

Publishes current display-chunk population and generation.

### `compact_frame_requests` (private)

```cpp
void compact_frame_requests() noexcept
```

Erases disconnected/dead scheduled requests without disturbing order.

### `active_surface_count` (private)

```cpp
[[nodiscard]] std::size_t active_surface_count() const noexcept
```

Counts connected periodic surface requests.

### `update_frame_schedule_metrics` (private)

```cpp
void update_frame_schedule_metrics() noexcept
```

Publishes bounded request/surface counts.

### `absolute_bounds_of` (private)

```cpp
[[nodiscard]] Rect absolute_bounds_of(const Control& control) const
```

Accumulates retained parent transforms into root-client logical bounds.

### `visual_bounds_of` (private)

```cpp
[[nodiscard]] Rect visual_bounds_of(const Control& control, Insets outsets) const
```

Expands absolute bounds by validated bounded visual outsets.

### `paint_damage_bounds_of` (private)

```cpp
[[nodiscard]] Rect paint_damage_bounds_of(const Control& control) const
```

Unites current and last-painted visual bounds so shrinking effects erase cleanly.

### `eligible` (private)

```cpp
[[nodiscard]] bool eligible(const Control::Ptr& control) const noexcept
```

Tests live attachment plus the control's effective input eligibility.

### `move_focus_after` (private)

```cpp
[[nodiscard]] bool move_focus_after(const Control::Ptr& origin)
```

Finds the next stable focus candidate after an origin inside the active scope.

### `validate_command_activation` (private)

```cpp
[[nodiscard]] bool validate_command_activation( const Control::Ptr& destination)
```

Applies focus validation before mnemonic/default/cancel command activation.

### `dispatch_mnemonic` (private)

```cpp
[[nodiscard]] bool dispatch_mnemonic(char32_t character)
```

Builds and cycles stable mnemonic candidates while accounting collisions and scope containment.

### `dispatch_dialog_button` (private)

```cpp
[[nodiscard]] bool dispatch_dialog_button(bool accept)
```

Validates and performs the configured accept or cancel button through normal semantic press behavior.

### `clear_dialog_targets_for_subtree` (private)

```cpp
void clear_dialog_targets_for_subtree(const Control::Ptr& control) noexcept
```

Clears accept/cancel references owned by a departing subtree.

### `require_ui_thread` (private)

```cpp
void require_ui_thread(std::string_view operation)
```

Rejects named wrong-thread mutations and records the structured rejection metric.
