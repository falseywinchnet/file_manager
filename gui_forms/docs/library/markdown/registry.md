# Registry

- Status: **OBSERVED: bundle 010 private ABI registry state-machine split; native C11/C++/export tests and complete M4 MinGW build pass**
- Kind: **class**
- Hierarchy: `Registry`
- Declaration: `src/abi/registry/registry.hpp:82`
- Definition: `inline/header-only`

Registry is the single compatibility ABI authority for generational handles, thread affinity, control trees, subscriptions, host sessions, queued callbacks, transient services, property proxies, and deterministic retirement.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `create` (public)

```cpp
gf_result create(std::uint32_t kind, gf_string_view stable_id, gf_handle* output)
```

Validates a control kind and stable UTF-8 identity, constructs the matching native adapter, records owner affinity, and allocates a generational handle.

### `set_string` (public)

```cpp
gf_result set_string(gf_handle handle, gf_string_view input, bool is_text)
```

Resolves one admitted string property, validates UTF-8, and commits it on the owner thread.

### `get_string` (public)

```cpp
gf_result get_string(gf_handle handle, char* buffer, std::uint64_t capacity, std::uint64_t* required_size, bool is_text)
```

Copies one admitted string property through the ABI size-query/buffer contract.

### `set_enabled` (public)

```cpp
gf_result set_enabled(gf_handle handle, std::uint32_t enabled)
```

Commits retained enabled state through an owner-thread handle lookup.

### `get_enabled` (public)

```cpp
gf_result get_enabled(gf_handle handle, std::uint32_t* enabled)
```

Returns retained enabled state through a validated output pointer.

### `set_cursor` (public)

```cpp
gf_result set_cursor(gf_handle handle, std::uint32_t cursor_kind)
```

Maps the closed ABI cursor vocabulary to native cursor state.

### `get_cursor` (public)

```cpp
gf_result get_cursor(gf_handle handle, std::uint32_t* cursor_kind)
```

Projects native cursor state back to the ABI vocabulary.

### `set_auto_scroll_offset` (public)

```cpp
gf_result set_auto_scroll_offset(gf_handle handle, gf_point offset)
```

Commits scroll rendering offset on a scrollable adapter.

### `set_auto_scroll` (public)

```cpp
gf_result set_auto_scroll(gf_handle handle, std::uint32_t enabled)
```

Enables or disables retained auto-scroll policy.

### `set_auto_scroll_margin` (public)

```cpp
gf_result set_auto_scroll_margin(gf_handle handle, gf_size margin)
```

Validates and commits auto-scroll margins.

### `set_auto_scroll_min_size` (public)

```cpp
gf_result set_auto_scroll_min_size(gf_handle handle, gf_size size)
```

Validates and commits virtual content minimum size.

### `set_auto_scroll_position` (public)

```cpp
gf_result set_auto_scroll_position(gf_handle handle, gf_point position)
```

Commits logical scroll position through the scroll model.

### `get_scroll_state` (public)

```cpp
gf_result get_scroll_state(gf_handle handle, gf_scroll_state* state)
```

Projects complete auto-scroll and per-axis state into the ABI record.

### `set_scroll_axis_state` (public)

```cpp
gf_result set_scroll_axis_state(gf_handle handle, std::uint32_t orientation, gf_scroll_axis_state state)
```

Validates and commits one horizontal or vertical axis policy/value set.

### `scroll_control_into_view` (public)

```cpp
gf_result scroll_control_into_view(gf_handle handle, gf_handle child_handle)
```

Resolves a descendant and updates scroll position to expose it.

### `suspend_layout` (public)

```cpp
gf_result suspend_layout(gf_handle handle)
```

Enters retained layout suspension for the target control.

### `resume_layout` (public)

```cpp
gf_result resume_layout(gf_handle handle, std::uint32_t perform_layout)
```

Leaves suspension and optionally performs pending layout.

### `perform_control_layout` (public)

```cpp
gf_result perform_control_layout(gf_handle handle)
```

Requests an explicit retained layout pass.

### `get_layout_state` (public)

```cpp
gf_result get_layout_state(gf_handle handle, gf_layout_state* state)
```

Projects suspension depth and pending-layout state.

### `property_grid_set_selected_controls` (public)

```cpp
gf_result property_grid_set_selected_controls( gf_handle grid_handle, const gf_handle* handles, std::uint64_t count)
```

Validates proxy handles and sets the native PropertyGrid multi-selection.

### `property_object_define` (public)

```cpp
gf_result property_object_define( gf_handle handle, const gf_property_descriptor_v1* descriptor, const gf_property_callbacks_v1* callbacks)
```

Copies one foreign property descriptor/callback table into a proxy control.

### `property_object_notify_changed` (public)

```cpp
gf_result property_object_notify_changed( gf_handle handle, gf_string_view property_name)
```

Publishes one explicit foreign property change by canonical name.

### `property_grid_try_set_text` (public)

```cpp
gf_result property_grid_try_set_text( gf_handle handle, gf_string_view property_name, gf_string_view text_value, std::uint32_t* committed)
```

Routes text editing through native converter/descriptor/property policy.

### `property_grid_reset_property` (public)

```cpp
gf_result property_grid_reset_property( gf_handle handle, gf_string_view property_name, std::uint32_t* committed)
```

Routes reset through native reset/serialization policy.

### `property_grid_activate_editor` (public)

```cpp
gf_result property_grid_activate_editor( gf_handle handle, gf_string_view property_name, std::uint32_t* activated)
```

Invokes the named native editor for the selected property.

### `property_grid_set_sort` (public)

```cpp
gf_result property_grid_set_sort(gf_handle grid_handle, std::uint32_t property_sort)
```

Maps and commits the closed property sort vocabulary.

### `property_grid_get_sort` (public)

```cpp
gf_result property_grid_get_sort(gf_handle grid_handle, std::uint32_t* property_sort)
```

Projects current property sort vocabulary.

### `property_grid_refresh` (public)

```cpp
gf_result property_grid_refresh(gf_handle grid_handle)
```

Refreshes descriptors/values from selected native proxies.

### `set_control_png` (public)

```cpp
gf_result set_control_png(gf_handle handle, const std::uint8_t* encoded, std::uint64_t encoded_size)
```

Copies and validates bounded PNG input into a RasterControl.

### `set_control_pixels` (public)

```cpp
gf_result set_control_pixels(gf_handle handle, const std::uint8_t* pixels, std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes, std::uint32_t pixel_format)
```

Copies validated premultiplied BGRA dimensions, stride, and bytes into a RasterControl.

### `compatibility_paint_target` (public)

```cpp
gf_result compatibility_paint_target( gf_handle handle, std::weak_ptr<RasterControl>* target, std::thread::id* owner_thread)
```

Returns a weak RasterControl and exact owner thread for Windows endpoint binding.

### `set_child_index` (public)

```cpp
gf_result set_child_index(gf_handle parent_handle, gf_handle child_handle, std::uint64_t index)
```

Reorders one exact retained child within its parent.

### `set_control_colors` (public)

```cpp
gf_result set_control_colors(gf_handle handle, std::uint32_t foreground_argb, std::uint32_t background_argb)
```

Maps authored ARGB foreground/background colors to the applicable adapter.

### `set_control_text_alignment` (public)

```cpp
gf_result set_control_text_alignment(gf_handle handle, std::uint32_t content_alignment)
```

Maps the closed ABI alignment vocabulary to an applicable text adapter.

### `set_button_appearance` (public)

```cpp
gf_result set_button_appearance(gf_handle handle, std::uint32_t visual_style, double flat_border_width)
```

Maps bounded visual style, border, flat, and image/text relationship options to ButtonBase.

### `set_panel_border_style` (public)

```cpp
gf_result set_panel_border_style(gf_handle handle, std::uint32_t border_style)
```

Maps the closed border vocabulary and width to Panel.

### `set_field_selection` (public)

```cpp
gf_result set_field_selection(gf_handle handle, std::uint64_t start, std::uint64_t length, std::uint32_t caret_visible)
```

Validates UTF-8 scalar boundaries and commits field anchor/caret direction.

### `set_field_edit_state` (public)

```cpp
gf_result set_field_edit_state(gf_handle handle, std::uint64_t anchor, std::uint64_t caret, std::uint32_t caret_visible)
```

Commits field read-only/password/multiline/viewport policy.

### `field_position_from_point` (public)

```cpp
gf_result field_position_from_point(gf_handle handle, double local_x, std::uint64_t* position)
```

Maps field-local geometry to an exact UTF-8 scalar boundary.

### `write_clipboard_text` (public)

```cpp
gf_result write_clipboard_text(gf_handle owner_handle, gf_string_view input)
```

Uses the active host clipboard service or bounded deterministic fallback.

### `read_clipboard_text` (public)

```cpp
gf_result read_clipboard_text(gf_handle owner_handle, char* buffer, std::uint64_t capacity, std::uint64_t* required_size, std::uint32_t* has_text)
```

Reads host/fallback clipboard text through the ABI buffer contract.

### `field_navigate` (public)

```cpp
gf_result field_navigate(gf_handle handle, std::uint64_t position, std::int32_t direction, std::uint64_t* result)
```

Routes one closed Unicode navigation command and returns edit telemetry.

### `field_replace` (public)

```cpp
gf_result field_replace(gf_handle handle, std::uint64_t start, std::uint64_t length, gf_string_view input, gf_field_edit_result* result)
```

Validates and commits one UTF-8 field replacement with edit telemetry.

### `field_history` (public)

```cpp
gf_result field_history(gf_handle handle, std::int32_t direction, gf_field_edit_result* result)
```

Moves one step through field undo or redo history.

### `field_clear_history` (public)

```cpp
gf_result field_clear_history(gf_handle handle)
```

Drops field undo and redo state.

### `set_check_state` (public)

```cpp
gf_result set_check_state(gf_handle handle, std::uint32_t check_state)
```

Maps and commits the closed unchecked/checked/indeterminate vocabulary.

### `get_check_state` (public)

```cpp
gf_result get_check_state(gf_handle handle, std::uint32_t* check_state)
```

Projects current check state.

### `set_range` (public)

```cpp
gf_result set_range(gf_handle handle, double minimum, double maximum)
```

Validates and commits numeric minimum/maximum on an applicable range control.

### `get_range` (public)

```cpp
gf_result get_range(gf_handle handle, double* minimum, double* maximum)
```

Returns numeric bounds.

### `set_range_value` (public)

```cpp
gf_result set_range_value(gf_handle handle, double value)
```

Validates and commits current range value.

### `get_range_value` (public)

```cpp
gf_result get_range_value(gf_handle handle, double* value)
```

Returns current range value.

### `set_pointer_capture` (public)

```cpp
gf_result set_pointer_capture(gf_handle handle, std::uint32_t captured)
```

Requests or releases Window pointer capture for an admitted control.

### `get_pointer_capture` (public)

```cpp
gf_result get_pointer_capture(gf_handle handle, std::uint32_t* captured)
```

Reports whether the control currently owns capture.

### `show_path_dialog` (public)

```cpp
gf_result show_path_dialog(gf_handle owner_handle, std::uint32_t kind, gf_string_view title, gf_string_view initial_directory, gf_string_view suggested_name, gf_string_view default_extension, gf_string_view filter, std::uint32_t flags, std::uint32_t* accepted)
```

Builds a host-neutral path dialog request, routes it to the active host, and records the result by request identity.

### `last_dialog_path` (public)

```cpp
gf_result last_dialog_path(gf_handle owner_handle, char* buffer, std::uint64_t capacity, std::uint64_t* required_size)
```

Copies the last accepted path through the ABI buffer contract.

### `show_tooltip` (public)

```cpp
gf_result show_tooltip(gf_handle owner_handle, gf_string_view text, double x, double y, std::uint32_t duration_milliseconds)
```

Builds and routes a bounded host tooltip request for owner-relative geometry.

### `hide_tooltip` (public)

```cpp
gf_result hide_tooltip(gf_handle owner_handle)
```

Dismisses the owner tooltip through the active host service.

### `run_window` (public)

```cpp
gf_result run_window(gf_handle handle, std::uint32_t flags)
```

Transitions the root handle through created/running/stopping/stopped host phases using the admitted headless, AppKit, or Win32 adapter.

### `last_host_trace` (public)

```cpp
gf_result last_host_trace(gf_handle handle, char* buffer, std::uint64_t capacity, std::uint64_t* required_size)
```

Copies the latest structured host trace through the ABI buffer contract.

### `retain` (public)

```cpp
gf_result retain(gf_handle handle)
```

Increments an admitted slot reference count without overflow.

### `release` (public)

```cpp
gf_result release(gf_handle handle)
```

Decrements a slot reference and invalidates/disposes ownership when the last reference retires.

### `dispose` (public)

```cpp
gf_result dispose(gf_handle handle)
```

Runs owner-thread Component disposal without releasing the handle reference.

### `component_state` (public)

```cpp
gf_result component_state(gf_handle handle, std::uint32_t* output)
```

Projects native alive/disposing/disposed state.

### `stable_id` (public)

```cpp
gf_result stable_id(gf_handle handle, char* buffer, std::uint64_t capacity, std::uint64_t* required_size)
```

Copies immutable native stable identity.

### `set_visible` (public)

```cpp
gf_result set_visible(gf_handle handle, std::uint32_t visible)
```

Commits retained visibility.

### `get_visible` (public)

```cpp
gf_result get_visible(gf_handle handle, std::uint32_t* visible)
```

Returns retained visibility.

### `set_bounds` (public)

```cpp
gf_result set_bounds(gf_handle handle, gf_rect bounds)
```

Validates finite geometry and commits retained bounds.

### `get_bounds` (public)

```cpp
gf_result get_bounds(gf_handle handle, gf_rect* bounds)
```

Returns retained local bounds.

### `get_control_absolute_bounds` (public)

```cpp
gf_result get_control_absolute_bounds(gf_handle handle, gf_rect* bounds)
```

Flushes required layout and returns exact window-relative bounds.

### `add_child` (public)

```cpp
gf_result add_child(gf_handle parent_handle, gf_handle child_handle)
```

Validates owner threads, tree identity, and cycle/ownership rules before retained attachment.

### `remove_child` (public)

```cpp
gf_result remove_child(gf_handle parent_handle, gf_handle child_handle)
```

Detaches one exact retained child.

### `attach_popup` (public)

```cpp
gf_result attach_popup(gf_handle owner_handle, gf_handle popup_handle)
```

Registers an owned popup plane through Window and records its revocable token.

### `detach_popup` (public)

```cpp
gf_result detach_popup(gf_handle popup_handle)
```

Revokes one popup token and clears attachment state.

### `subscribe` (public)

```cpp
gf_result subscribe(gf_handle sender_handle, std::uint32_t event_kind, gf_event_callback callback, void* context, gf_event_token* output)
```

Allocates a generational subscription slot for the legacy event callback surface.

### `disconnect` (public)

```cpp
gf_result disconnect(gf_event_token token)
```

Disconnects native observation and invalidates the subscription slot.

### `subscribe_v2` (public)

```cpp
gf_result subscribe_v2(gf_handle sender_handle, std::uint32_t event_kind, gf_event_callback_v2 callback, void* context, gf_event_token* output)
```

Allocates a typed extended-event callback subscription.

### `subscribe_pointer` (public)

```cpp
gf_result subscribe_pointer(gf_handle sender_handle, gf_pointer_callback callback, void* context, gf_event_token* output)
```

Connects projected pointer observation for a compatible adapter.

### `subscribe_key` (public)

```cpp
gf_result subscribe_key(gf_handle sender_handle, gf_key_callback callback, void* context, gf_event_token* output)
```

Connects projected key observation for a compatible adapter.

### `subscribe_key_preview` (public)

```cpp
gf_result subscribe_key_preview(gf_handle sender_handle, gf_key_callback callback, void* context, gf_event_token* output)
```

Connects the root FormControl preview seam.

### `subscribe_text` (public)

```cpp
gf_result subscribe_text(gf_handle sender_handle, gf_text_callback callback, void* context, gf_event_token* output)
```

Connects projected text/composition observation.

### `begin_invoke` (public)

```cpp
gf_result begin_invoke(gf_handle control_handle, gf_dispatch_callback callback, void* context)
```

Queues one caller callback through Window dispatch and records cancellation/fault ownership.

### `request_close` (public)

```cpp
gf_result request_close(gf_handle form_handle)
```

Requests the root host loop to enter stopping through its owner-thread path.

### `callback_fault_count` (public)

```cpp
gf_result callback_fault_count(gf_handle control_handle, std::uint64_t* count)
```

Returns contained foreign callback failures for the control record.

### `root_record_locked` (private)

```cpp
std::shared_ptr<ControlRecord> root_record_locked( const std::shared_ptr<ControlRecord>& record)
```

Finds the root ControlRecord while registry mutex ownership is held.

### `emit_v2` (private)

```cpp
std::uint32_t emit_v2(gf_handle sender_handle, std::uint32_t event_kind)
```

Copies a stable typed ABI event and invokes matching connected callbacks outside mutation paths.

### `pump_pending` (private)

```cpp
void pump_pending(const std::shared_ptr<ControlRecord>& root)
```

Moves pending dispatch records into Window execution without holding the registry mutex across callbacks.

### `cancel_pending` (private)

```cpp
void cancel_pending(const std::shared_ptr<ControlRecord>& root) noexcept
```

Retires queued dispatch ownership during close or invalidation.

### `publish_host` (private)

```cpp
void publish_host(const std::shared_ptr<ControlRecord>& root, std::function<void()> wake, std::function<void()> request_close, std::function<gui_forms::HostDialogResult( const gui_forms::HostDialogRequest&)> dialog =
```

Commits host callbacks/services and transitions the record to running.

### `mark_host_stopping` (private)

```cpp
void mark_host_stopping(const std::shared_ptr<ControlRecord>& root) noexcept
```

Transitions a running host toward stopping once.

### `close_requested` (private)

```cpp
bool close_requested(const std::shared_ptr<ControlRecord>& root)
```

Reports whether the record has entered stopping.

### `finish_host` (private)

```cpp
void finish_host(const std::shared_ptr<ControlRecord>& root) noexcept
```

Clears host services, cancels work, stores trace, and commits stopped.

### `copy_view` (private)

```cpp
static bool copy_view(gf_string_view input, std::string& output)
```

Implements the ABI size-query/exact-buffer copy law for UTF-8 strings.

### `parse_filters` (private)

```cpp
static std::vector<gui_forms::HostFileDialogFilter> parse_filters(std::string_view serialized)
```

Validates and converts ABI dialog filters into host-neutral records.

### `managed_trace` (private)

```cpp
std::string managed_trace(const std::shared_ptr<ControlRecord>& root)
```

Builds bounded compatibility host lifecycle diagnostics.

### `first_button` (private)

```cpp
static std::shared_ptr<gui_forms::ButtonBase> first_button( const std::shared_ptr<Control>& root)
```

Finds the first ButtonBase in a retained subtree for dialog/default mapping.

### `first_pointer_control` (private)

```cpp
static std::shared_ptr<Control> first_pointer_control( const std::shared_ptr<Control>& root)
```

Finds the first pointer-capable adapter in a retained subtree.

### `named_controls_snapshot` (private)

```cpp
std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<Control>>> named_controls_snapshot(const std::shared_ptr<ControlRecord>& root)
```

Collects stable-ID-to-control mappings without exposing executable registry state.

### `refresh_named_controls` (private)

```cpp
void refresh_named_controls( const std::shared_ptr<ControlRecord>& root, std::unordered_map<std::string, std::shared_ptr<Control>>& result)
```

Rebuilds a host adapter's stable named-control projection.

### `contains_control` (private)

```cpp
static bool contains_control(const std::shared_ptr<Control>& root, const std::shared_ptr<Control>& candidate)
```

Tests retained subtree membership by pointer identity.

### `require_thread` (private)

```cpp
gf_result require_thread(const ControlRecord& record) const
```

Rejects a handle operation performed off its recorded UI thread.

### `get_control` (private)

```cpp
gf_result get_control(gf_handle handle, std::shared_ptr<ControlRecord>& output)
```

Resolves and retains one live Control from a generational handle.

### `control_locked` (private)

```cpp
gf_result control_locked(gf_handle handle, std::shared_ptr<ControlRecord>& output)
```

Validates slot kind/generation and returns ControlRecord under mutex.

### `subscription_locked` (private)

```cpp
gf_result subscription_locked(gf_event_token token, std::shared_ptr<SubscriptionRecord>& output)
```

Validates slot kind/generation and returns SubscriptionRecord under mutex.

### `slot_locked` (private)

```cpp
RegistrySlot* slot_locked(gf_handle handle)
```

Decodes handle index/generation and rejects stale or empty slots.

### `allocate_locked` (private)

```cpp
gf_handle allocate_locked(SlotKind kind, std::shared_ptr<ControlRecord> control, std::shared_ptr<SubscriptionRecord> subscription)
```

Reuses or appends a slot, preserving nonzero generation, and returns an encoded handle.

### `invalidate_control_locked` (private)

```cpp
void invalidate_control_locked(gf_handle handle, ControlRecord& record)
```

Revokes popup/host/pending work, detaches hierarchy, disposes native state, and advances generation.

### `invalidate_slot_locked` (private)

```cpp
void invalidate_slot_locked(gf_handle handle)
```

Disconnects and clears subscription state and advances generation.

### `emit_changed` (private)

```cpp
void emit_changed(gf_handle sender_handle, const std::shared_ptr<ControlRecord>& sender)
```

Snapshots matching legacy callbacks and emits state change without holding registry mutation ownership.
