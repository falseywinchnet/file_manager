# Window

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Window`  
Declaration: `include/gui_forms/window.hpp:285`  
Definition: `src/core/dispatcher.cpp, src/core/window.cpp`

Window is a class declared in include/gui_forms/window.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `Window`

```cpp
explicit Window(Control::Ptr root, Size client_size =
```

Constructs or tears down the retained Window object according to its ownership contract.

### `~Window`

```cpp
~Window()
```

Constructs or tears down the retained Window object according to its ownership contract.

### `Window`

```cpp
Window(const Window&) = delete
```

Constructs or tears down the retained Window object according to its ownership contract.

### `operator=`

```cpp
Window& operator=(const Window&) = delete
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `root`

```cpp
[[nodiscard]] Control::Ptr root() const noexcept
```

Reports the current root value without mutation.

### `client_size`

```cpp
[[nodiscard]] Size client_size() const noexcept
```

Reports the current client size value without mutation.

### `resize`

```cpp
void resize(Size client_size)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_scale`

```cpp
void set_scale(double scale)
```

Synchronously updates the retained scale property. Validation, typed invalidation, and notifications are defined by the implementation.

### `scale`

```cpp
[[nodiscard]] double scale() const noexcept
```

Reports the current scale value without mutation.

### `presentation_settings`

```cpp
[[nodiscard]] const PresentationSettings& presentation_settings() const noexcept
```

Reports the current presentation settings value without mutation.

### `set_presentation_settings`

```cpp
void set_presentation_settings(PresentationSettings settings)
```

Synchronously updates the retained presentation settings property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_text_scale`

```cpp
void set_text_scale(double text_scale)
```

Synchronously updates the retained text scale property. Validation, typed invalidation, and notifications are defined by the implementation.

### `presentation_changed`

```cpp
[[nodiscard]] Event<const PresentationSettings&>& presentation_changed() noexcept
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `theme`

```cpp
[[nodiscard]] const Theme& theme() const noexcept
```

Reports the current theme value without mutation.

### `theme_ptr`

```cpp
[[nodiscard]] std::shared_ptr<const Theme> theme_ptr() const noexcept
```

Reports the current theme ptr value without mutation.

### `set_theme`

```cpp
void set_theme(std::shared_ptr<const Theme> theme)
```

Synchronously updates the retained theme property. Validation, typed invalidation, and notifications are defined by the implementation.

### `theme_changed`

```cpp
[[nodiscard]] Event<const Theme&>& theme_changed() noexcept
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `active`

```cpp
[[nodiscard]] bool active() const noexcept
```

Reports the current active value without mutation.

### `set_active`

```cpp
void set_active(bool active)
```

Synchronously updates the retained active property. Validation, typed invalidation, and notifications are defined by the implementation.

### `active_changed`

```cpp
[[nodiscard]] Event<bool>& active_changed() noexcept
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `host_services`

```cpp
[[nodiscard]] HostServices* host_services() const noexcept
```

Reports the current host services value without mutation.

### `begin_update`

```cpp
[[nodiscard]] UpdateScope begin_update()
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `perform_layout`

```cpp
void perform_layout()
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `flush`

```cpp
void flush()
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `paint`

```cpp
std::optional<PaintReceipt> paint( Painter& painter, Rect requested_damage =
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `notify_presented`

```cpp
[[nodiscard]] bool notify_presented( PaintReceipt receipt, std::uint64_t duration_nanoseconds = 0U)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `notify_presented`

```cpp
void notify_presented(std::uint64_t duration_nanoseconds = 0U)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `paint_lease_snapshot`

```cpp
[[nodiscard]] PaintLeaseSnapshot paint_lease_snapshot() const noexcept
```

Reports the current paint lease snapshot value without mutation.

### `queue_live_surface_presentation`

```cpp
[[nodiscard]] bool queue_live_surface_presentation( const Control::Ptr& control, std::shared_ptr<LiveSurface> surface)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `take_live_surface_presentations`

```cpp
[[nodiscard]] std::vector<LiveSurfacePresentation> take_live_surface_presentations()
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `has_live_surface_presentations`

```cpp
[[nodiscard]] bool has_live_surface_presentations() const noexcept
```

Reports the current has live surface presentations value without mutation.

### `take_damage`

```cpp
[[nodiscard]] DamageRegion take_damage()
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `take_damage`

```cpp
[[nodiscard]] DamageRegion take_damage(PaintPlane plane)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `needs_frame`

```cpp
[[nodiscard]] bool needs_frame() const noexcept
```

Reports the current needs frame value without mutation.

### `next_wake`

```cpp
[[nodiscard]] std::optional<FrameTime> next_wake() const noexcept
```

Reports the current next wake value without mutation.

### `schedule_paint`

```cpp
[[nodiscard]] FrameRequestToken schedule_paint(const Control::Ptr& control, FrameTime deadline)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `activate_surface`

```cpp
[[nodiscard]] FrameRequestToken activate_surface(const Control::Ptr& control, FrameInterval interval, FrameTime first_deadline)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `schedule_ui_timer`

```cpp
[[nodiscard]] FrameRequestToken schedule_ui_timer( Component& owner, FrameInterval interval, FrameTime first_deadline, std::function<void(FrameTime)> callback)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `poll_frame_schedule`

```cpp
[[nodiscard]] FramePollResult poll_frame_schedule(FrameTime now)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `cancel_frame_requests`

```cpp
void cancel_frame_requests()
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_occluded`

```cpp
void set_occluded(bool occluded, FrameTime transition_time)
```

Synchronously updates the retained occluded property. Validation, typed invalidation, and notifications are defined by the implementation.

### `occluded`

```cpp
[[nodiscard]] bool occluded() const noexcept
```

Reports the current occluded value without mutation.

### `check_access`

```cpp
[[nodiscard]] bool check_access() const noexcept
```

Reports the current check access value without mutation.

### `verify_access`

```cpp
void verify_access(std::string_view operation = "window access")
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `invoke_required`

```cpp
[[nodiscard]] bool invoke_required() const noexcept
```

Reports the current invoke required value without mutation.

### `begin_invoke`

```cpp
[[nodiscard]] DispatchOperation begin_invoke(std::function<void()> callback)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `begin_invoke`

```cpp
[[nodiscard]] DispatchOperation begin_invoke( const Control::Ptr& owner, std::function<void()> callback)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `invoke`

```cpp
void invoke(std::function<void()> callback)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `invoke`

```cpp
void invoke(const Control::Ptr& owner, std::function<void()> callback)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `drain_posted_work`

```cpp
[[nodiscard]] DispatchDrainResult drain_posted_work( std::size_t maximum_callbacks = maximum_callbacks_per_dispatch_turn)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `dispatcher_snapshot`

```cpp
[[nodiscard]] DispatcherSnapshot dispatcher_snapshot() const noexcept
```

Reports the current dispatcher snapshot value without mutation.

### `set_dispatch_wake_handler`

```cpp
void set_dispatch_wake_handler(std::function<void()> wake)
```

Synchronously updates the retained dispatch wake handler property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_paint_wake_handler`

```cpp
void set_paint_wake_handler(std::function<void()> wake)
```

Synchronously updates the retained paint wake handler property. Validation, typed invalidation, and notifications are defined by the implementation.

### `shutdown_dispatcher`

```cpp
void shutdown_dispatcher() noexcept
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `load_png`

```cpp
[[nodiscard]] ImageLoadResult load_png(std::span<const std::byte> encoded)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `load_bgra32_premultiplied`

```cpp
[[nodiscard]] ImageLoadResult load_bgra32_premultiplied( std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes, std::span<const std::byte> pixels)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `replace_png`

```cpp
[[nodiscard]] ImageLoadResult replace_png(ImageId image, std::span<const std::byte> encoded)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `replace_png`

```cpp
[[nodiscard]] ImageLoadResult replace_png(ImageId image, std::span<const std::byte> encoded, Control& consumer)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `replace_bgra32_premultiplied`

```cpp
[[nodiscard]] ImageLoadResult replace_bgra32_premultiplied( ImageId image, std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes, std::span<const std::byte> pixels, Control& consumer)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `update_bgra32_premultiplied`

```cpp
[[nodiscard]] ImageLoadResult update_bgra32_premultiplied( ImageId image, std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes, std::span<const std::byte> pixels, Control& consumer)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `patch_bgra32_premultiplied`

```cpp
[[nodiscard]] ImageLoadResult patch_bgra32_premultiplied( ImageId image, std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height, std::uint64_t source_row_bytes, std::span<const std::byte> pixels, Control& consumer, Rect local_damage)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `remove_image`

```cpp
[[nodiscard]] bool remove_image(ImageId image)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `image_resources`

```cpp
[[nodiscard]] const ImageRegistry& image_resources() const noexcept
```

Reports the current image resources value without mutation.

### `image_resource_snapshot`

```cpp
[[nodiscard]] ImageRegistrySnapshot image_resource_snapshot() const noexcept
```

Reports the current image resource snapshot value without mutation.

### `find`

```cpp
[[nodiscard]] Control::Ptr find(std::string_view stable_id) const
```

Reports the current find value without mutation.

### `hit_test`

```cpp
[[nodiscard]] Control::Ptr hit_test(Point position)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `request_focus`

```cpp
bool request_focus(const Control::Ptr& control)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `focused_control`

```cpp
[[nodiscard]] Control::Ptr focused_control() const noexcept
```

Reports the current focused control value without mutation.

### `validate_control`

```cpp
bool validate_control(const Control::Ptr& control, Control* destination = nullptr, bool bulk = false)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `validate_children`

```cpp
bool validate_children( const Control::Ptr& container, ValidationConstraints constraints = ValidationConstraints::selectable)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `validation_snapshot`

```cpp
[[nodiscard]] ValidationSnapshot validation_snapshot() const noexcept
```

Reports the current validation snapshot value without mutation.

### `begin_focus_scope`

```cpp
[[nodiscard]] FocusScopeId begin_focus_scope( const Control::Ptr& root, const Control::Ptr& preferred_focus =
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `end_focus_scope`

```cpp
bool end_focus_scope( FocusScopeId scope, FocusScopeCloseReason reason = FocusScopeCloseReason::explicit_close)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `focus_scope_depth`

```cpp
[[nodiscard]] std::size_t focus_scope_depth() const noexcept
```

Reports the current focus scope depth value without mutation.

### `active_focus_scope_root`

```cpp
[[nodiscard]] Control::Ptr active_focus_scope_root() const noexcept
```

Reports the current active focus scope root value without mutation.

### `move_focus`

```cpp
bool move_focus(bool forward = true)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_accept_button`

```cpp
void set_accept_button(const Control::Ptr& control)
```

Synchronously updates the retained accept button property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_cancel_button`

```cpp
void set_cancel_button(const Control::Ptr& control)
```

Synchronously updates the retained cancel button property. Validation, typed invalidation, and notifications are defined by the implementation.

### `accept_button`

```cpp
[[nodiscard]] Control::Ptr accept_button() const noexcept
```

Reports the current accept button value without mutation.

### `cancel_button`

```cpp
[[nodiscard]] Control::Ptr cancel_button() const noexcept
```

Reports the current cancel button value without mutation.

### `dialog_key_snapshot`

```cpp
[[nodiscard]] DialogKeySnapshot dialog_key_snapshot() const noexcept
```

Reports the current dialog key snapshot value without mutation.

### `dialog_result`

```cpp
[[nodiscard]] DialogResult dialog_result() const noexcept
```

Reports the current dialog result value without mutation.

### `set_dialog_result`

```cpp
void set_dialog_result(DialogResult result)
```

Synchronously updates the retained dialog result property. Validation, typed invalidation, and notifications are defined by the implementation.

### `dialog_result_changed`

```cpp
[[nodiscard]] Event<DialogResult>& dialog_result_changed() noexcept
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `focus_scope_changed`

```cpp
[[nodiscard]] Event<const FocusScopeChange&>& focus_scope_changed() noexcept
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `capture_pointer`

```cpp
void capture_pointer(const Control::Ptr& control, std::uint64_t pointer_id = 1)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `release_pointer`

```cpp
void release_pointer()
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `captured_control`

```cpp
[[nodiscard]] Control::Ptr captured_control() const noexcept
```

Reports the current captured control value without mutation.

### `captured_pointer_id`

```cpp
[[nodiscard]] std::uint64_t captured_pointer_id() const noexcept
```

Reports the current captured pointer id value without mutation.

### `pointer_capture_changed`

```cpp
[[nodiscard]] Event<const PointerCaptureChange&>& pointer_capture_changed() noexcept
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `control_availability_changed`

```cpp
[[nodiscard]] Event<const ControlAvailabilityChange&>& control_availability_changed() noexcept
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `open_popup`

```cpp
[[nodiscard]] PopupToken open_popup(const Control::Ptr& owner, const Control::Ptr& popup, PopupOptions options =
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `register_accelerator`

```cpp
[[nodiscard]] AcceleratorToken register_accelerator( Component& owner, KeyGesture gesture, std::function<bool()> callback, AcceleratorOptions options =
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `pressed_control`

```cpp
[[nodiscard]] Control::Ptr pressed_control() const noexcept
```

Reports the current pressed control value without mutation.

### `dispatch_pointer`

```cpp
bool dispatch_pointer(PointerEvent event)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `dispatch_key`

```cpp
bool dispatch_key(KeyEvent event)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `dispatch_text`

```cpp
bool dispatch_text(TextInputEvent event)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `deferred_input_snapshot`

```cpp
[[nodiscard]] DeferredInputSnapshot deferred_input_snapshot() const noexcept
```

Reports the current deferred input snapshot value without mutation.

### `dispatch_drag`

```cpp
[[nodiscard]] DragDispatchResult dispatch_drag(DragEvent event)
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `cancel_drag`

```cpp
void cancel_drag() noexcept
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `metrics_snapshot`

```cpp
[[nodiscard]] MetricsSnapshot metrics_snapshot() const
```

Reports the current metrics snapshot value without mutation.

### `metrics`

```cpp
Metrics& metrics() noexcept
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `reset_activity_metrics`

```cpp
void reset_activity_metrics() noexcept
```

Returns activity metrics to its inherited or default policy.

### `semantic_snapshot`

```cpp
[[nodiscard]] SemanticSnapshot semantic_snapshot()
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `semantic_generation`

```cpp
[[nodiscard]] std::uint64_t semantic_generation() const noexcept
```

Reports the current semantic generation value without mutation.

### `perform_semantic_action`

```cpp
bool perform_semantic_action(std::string_view stable_id, SemanticAction action, std::string_view value =
```

Public Window operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
