# Registry

- Status: **generated inventory; detailed review pending**
- Kind: **class**
- Hierarchy: `Registry`
- Declaration: `src/abi/registry/registry.hpp:82`
- Definition: `inline/header-only`

Registry is a class declared in src/abi/registry/registry.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `create` (public)

```cpp
gf_result create(std::uint32_t kind, gf_string_view stable_id, gf_handle* output)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_string` (public)

```cpp
gf_result set_string(gf_handle handle, gf_string_view input, bool is_text)
```

Synchronously updates the retained string property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_string` (public)

```cpp
gf_result get_string(gf_handle handle, char* buffer, std::uint64_t capacity, std::uint64_t* required_size, bool is_text)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_enabled` (public)

```cpp
gf_result set_enabled(gf_handle handle, std::uint32_t enabled)
```

Synchronously updates the retained enabled property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_enabled` (public)

```cpp
gf_result get_enabled(gf_handle handle, std::uint32_t* enabled)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_cursor` (public)

```cpp
gf_result set_cursor(gf_handle handle, std::uint32_t cursor_kind)
```

Synchronously updates the retained cursor property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_cursor` (public)

```cpp
gf_result get_cursor(gf_handle handle, std::uint32_t* cursor_kind)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_auto_scroll_offset` (public)

```cpp
gf_result set_auto_scroll_offset(gf_handle handle, gf_point offset)
```

Synchronously updates the retained auto scroll offset property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_auto_scroll` (public)

```cpp
gf_result set_auto_scroll(gf_handle handle, std::uint32_t enabled)
```

Synchronously updates the retained auto scroll property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_auto_scroll_margin` (public)

```cpp
gf_result set_auto_scroll_margin(gf_handle handle, gf_size margin)
```

Synchronously updates the retained auto scroll margin property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_auto_scroll_min_size` (public)

```cpp
gf_result set_auto_scroll_min_size(gf_handle handle, gf_size size)
```

Synchronously updates the retained auto scroll min size property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_auto_scroll_position` (public)

```cpp
gf_result set_auto_scroll_position(gf_handle handle, gf_point position)
```

Synchronously updates the retained auto scroll position property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_scroll_state` (public)

```cpp
gf_result get_scroll_state(gf_handle handle, gf_scroll_state* state)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_scroll_axis_state` (public)

```cpp
gf_result set_scroll_axis_state(gf_handle handle, std::uint32_t orientation, gf_scroll_axis_state state)
```

Synchronously updates the retained scroll axis state property. Validation, typed invalidation, and notifications are defined by the implementation.

### `scroll_control_into_view` (public)

```cpp
gf_result scroll_control_into_view(gf_handle handle, gf_handle child_handle)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `suspend_layout` (public)

```cpp
gf_result suspend_layout(gf_handle handle)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `resume_layout` (public)

```cpp
gf_result resume_layout(gf_handle handle, std::uint32_t perform_layout)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `perform_control_layout` (public)

```cpp
gf_result perform_control_layout(gf_handle handle)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `get_layout_state` (public)

```cpp
gf_result get_layout_state(gf_handle handle, gf_layout_state* state)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `property_grid_set_selected_controls` (public)

```cpp
gf_result property_grid_set_selected_controls( gf_handle grid_handle, const gf_handle* handles, std::uint64_t count)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `property_object_define` (public)

```cpp
gf_result property_object_define( gf_handle handle, const gf_property_descriptor_v1* descriptor, const gf_property_callbacks_v1* callbacks)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `property_object_notify_changed` (public)

```cpp
gf_result property_object_notify_changed( gf_handle handle, gf_string_view property_name)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `property_grid_try_set_text` (public)

```cpp
gf_result property_grid_try_set_text( gf_handle handle, gf_string_view property_name, gf_string_view text_value, std::uint32_t* committed)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `property_grid_reset_property` (public)

```cpp
gf_result property_grid_reset_property( gf_handle handle, gf_string_view property_name, std::uint32_t* committed)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `property_grid_activate_editor` (public)

```cpp
gf_result property_grid_activate_editor( gf_handle handle, gf_string_view property_name, std::uint32_t* activated)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `property_grid_set_sort` (public)

```cpp
gf_result property_grid_set_sort(gf_handle grid_handle, std::uint32_t property_sort)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `property_grid_get_sort` (public)

```cpp
gf_result property_grid_get_sort(gf_handle grid_handle, std::uint32_t* property_sort)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `property_grid_refresh` (public)

```cpp
gf_result property_grid_refresh(gf_handle grid_handle)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_control_png` (public)

```cpp
gf_result set_control_png(gf_handle handle, const std::uint8_t* encoded, std::uint64_t encoded_size)
```

Synchronously updates the retained control png property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_control_pixels` (public)

```cpp
gf_result set_control_pixels(gf_handle handle, const std::uint8_t* pixels, std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes, std::uint32_t pixel_format)
```

Synchronously updates the retained control pixels property. Validation, typed invalidation, and notifications are defined by the implementation.

### `compatibility_paint_target` (public)

```cpp
gf_result compatibility_paint_target( gf_handle handle, std::weak_ptr<RasterControl>* target, std::thread::id* owner_thread)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_child_index` (public)

```cpp
gf_result set_child_index(gf_handle parent_handle, gf_handle child_handle, std::uint64_t index)
```

Synchronously updates the retained child index property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_control_colors` (public)

```cpp
gf_result set_control_colors(gf_handle handle, std::uint32_t foreground_argb, std::uint32_t background_argb)
```

Synchronously updates the retained control colors property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_control_text_alignment` (public)

```cpp
gf_result set_control_text_alignment(gf_handle handle, std::uint32_t content_alignment)
```

Synchronously updates the retained control text alignment property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_button_appearance` (public)

```cpp
gf_result set_button_appearance(gf_handle handle, std::uint32_t visual_style, double flat_border_width)
```

Synchronously updates the retained button appearance property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_panel_border_style` (public)

```cpp
gf_result set_panel_border_style(gf_handle handle, std::uint32_t border_style)
```

Synchronously updates the retained panel border style property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_field_selection` (public)

```cpp
gf_result set_field_selection(gf_handle handle, std::uint64_t start, std::uint64_t length, std::uint32_t caret_visible)
```

Synchronously updates the retained field selection property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_field_edit_state` (public)

```cpp
gf_result set_field_edit_state(gf_handle handle, std::uint64_t anchor, std::uint64_t caret, std::uint32_t caret_visible)
```

Synchronously updates the retained field edit state property. Validation, typed invalidation, and notifications are defined by the implementation.

### `field_position_from_point` (public)

```cpp
gf_result field_position_from_point(gf_handle handle, double local_x, std::uint64_t* position)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `write_clipboard_text` (public)

```cpp
gf_result write_clipboard_text(gf_handle owner_handle, gf_string_view input)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `read_clipboard_text` (public)

```cpp
gf_result read_clipboard_text(gf_handle owner_handle, char* buffer, std::uint64_t capacity, std::uint64_t* required_size, std::uint32_t* has_text)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `field_navigate` (public)

```cpp
gf_result field_navigate(gf_handle handle, std::uint64_t position, std::int32_t direction, std::uint64_t* result)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `field_replace` (public)

```cpp
gf_result field_replace(gf_handle handle, std::uint64_t start, std::uint64_t length, gf_string_view input, gf_field_edit_result* result)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `field_history` (public)

```cpp
gf_result field_history(gf_handle handle, std::int32_t direction, gf_field_edit_result* result)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `field_clear_history` (public)

```cpp
gf_result field_clear_history(gf_handle handle)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_check_state` (public)

```cpp
gf_result set_check_state(gf_handle handle, std::uint32_t check_state)
```

Synchronously updates the retained check state property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_check_state` (public)

```cpp
gf_result get_check_state(gf_handle handle, std::uint32_t* check_state)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_range` (public)

```cpp
gf_result set_range(gf_handle handle, double minimum, double maximum)
```

Synchronously updates the retained range property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_range` (public)

```cpp
gf_result get_range(gf_handle handle, double* minimum, double* maximum)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_range_value` (public)

```cpp
gf_result set_range_value(gf_handle handle, double value)
```

Synchronously updates the retained range value property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_range_value` (public)

```cpp
gf_result get_range_value(gf_handle handle, double* value)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_pointer_capture` (public)

```cpp
gf_result set_pointer_capture(gf_handle handle, std::uint32_t captured)
```

Synchronously updates the retained pointer capture property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_pointer_capture` (public)

```cpp
gf_result get_pointer_capture(gf_handle handle, std::uint32_t* captured)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `show_path_dialog` (public)

```cpp
gf_result show_path_dialog(gf_handle owner_handle, std::uint32_t kind, gf_string_view title, gf_string_view initial_directory, gf_string_view suggested_name, gf_string_view default_extension, gf_string_view filter, std::uint32_t flags, std::uint32_t* accepted)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `last_dialog_path` (public)

```cpp
gf_result last_dialog_path(gf_handle owner_handle, char* buffer, std::uint64_t capacity, std::uint64_t* required_size)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `show_tooltip` (public)

```cpp
gf_result show_tooltip(gf_handle owner_handle, gf_string_view text, double x, double y, std::uint32_t duration_milliseconds)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `hide_tooltip` (public)

```cpp
gf_result hide_tooltip(gf_handle owner_handle)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `run_window` (public)

```cpp
gf_result run_window(gf_handle handle, std::uint32_t flags)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `last_host_trace` (public)

```cpp
gf_result last_host_trace(gf_handle handle, char* buffer, std::uint64_t capacity, std::uint64_t* required_size)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `retain` (public)

```cpp
gf_result retain(gf_handle handle)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `release` (public)

```cpp
gf_result release(gf_handle handle)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `dispose` (public)

```cpp
gf_result dispose(gf_handle handle)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `component_state` (public)

```cpp
gf_result component_state(gf_handle handle, std::uint32_t* output)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `stable_id` (public)

```cpp
gf_result stable_id(gf_handle handle, char* buffer, std::uint64_t capacity, std::uint64_t* required_size)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_visible` (public)

```cpp
gf_result set_visible(gf_handle handle, std::uint32_t visible)
```

Synchronously updates the retained visible property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_visible` (public)

```cpp
gf_result get_visible(gf_handle handle, std::uint32_t* visible)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_bounds` (public)

```cpp
gf_result set_bounds(gf_handle handle, gf_rect bounds)
```

Synchronously updates the retained bounds property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_bounds` (public)

```cpp
gf_result get_bounds(gf_handle handle, gf_rect* bounds)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `get_control_absolute_bounds` (public)

```cpp
gf_result get_control_absolute_bounds(gf_handle handle, gf_rect* bounds)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `add_child` (public)

```cpp
gf_result add_child(gf_handle parent_handle, gf_handle child_handle)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `remove_child` (public)

```cpp
gf_result remove_child(gf_handle parent_handle, gf_handle child_handle)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `attach_popup` (public)

```cpp
gf_result attach_popup(gf_handle owner_handle, gf_handle popup_handle)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `detach_popup` (public)

```cpp
gf_result detach_popup(gf_handle popup_handle)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `subscribe` (public)

```cpp
gf_result subscribe(gf_handle sender_handle, std::uint32_t event_kind, gf_event_callback callback, void* context, gf_event_token* output)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `disconnect` (public)

```cpp
gf_result disconnect(gf_event_token token)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `subscribe_v2` (public)

```cpp
gf_result subscribe_v2(gf_handle sender_handle, std::uint32_t event_kind, gf_event_callback_v2 callback, void* context, gf_event_token* output)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `subscribe_pointer` (public)

```cpp
gf_result subscribe_pointer(gf_handle sender_handle, gf_pointer_callback callback, void* context, gf_event_token* output)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `subscribe_key` (public)

```cpp
gf_result subscribe_key(gf_handle sender_handle, gf_key_callback callback, void* context, gf_event_token* output)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `subscribe_key_preview` (public)

```cpp
gf_result subscribe_key_preview(gf_handle sender_handle, gf_key_callback callback, void* context, gf_event_token* output)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `subscribe_text` (public)

```cpp
gf_result subscribe_text(gf_handle sender_handle, gf_text_callback callback, void* context, gf_event_token* output)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `begin_invoke` (public)

```cpp
gf_result begin_invoke(gf_handle control_handle, gf_dispatch_callback callback, void* context)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `request_close` (public)

```cpp
gf_result request_close(gf_handle form_handle)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `callback_fault_count` (public)

```cpp
gf_result callback_fault_count(gf_handle control_handle, std::uint64_t* count)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `root_record_locked` (private)

```cpp
std::shared_ptr<ControlRecord> root_record_locked( const std::shared_ptr<ControlRecord>& record)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `emit_v2` (private)

```cpp
std::uint32_t emit_v2(gf_handle sender_handle, std::uint32_t event_kind)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `pump_pending` (private)

```cpp
void pump_pending(const std::shared_ptr<ControlRecord>& root)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `cancel_pending` (private)

```cpp
void cancel_pending(const std::shared_ptr<ControlRecord>& root) noexcept
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `publish_host` (private)

```cpp
void publish_host(const std::shared_ptr<ControlRecord>& root, std::function<void()> wake, std::function<void()> request_close, std::function<gui_forms::HostDialogResult( const gui_forms::HostDialogRequest&)> dialog =
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `mark_host_stopping` (private)

```cpp
void mark_host_stopping(const std::shared_ptr<ControlRecord>& root) noexcept
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `close_requested` (private)

```cpp
bool close_requested(const std::shared_ptr<ControlRecord>& root)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `finish_host` (private)

```cpp
void finish_host(const std::shared_ptr<ControlRecord>& root) noexcept
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `copy_view` (private)

```cpp
static bool copy_view(gf_string_view input, std::string& output)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `parse_filters` (private)

```cpp
static std::vector<gui_forms::HostFileDialogFilter> parse_filters(std::string_view serialized)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `managed_trace` (private)

```cpp
std::string managed_trace(const std::shared_ptr<ControlRecord>& root)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `first_button` (private)

```cpp
static std::shared_ptr<gui_forms::ButtonBase> first_button( const std::shared_ptr<Control>& root)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `first_pointer_control` (private)

```cpp
static std::shared_ptr<Control> first_pointer_control( const std::shared_ptr<Control>& root)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `named_controls_snapshot` (private)

```cpp
std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<Control>>> named_controls_snapshot(const std::shared_ptr<ControlRecord>& root)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `refresh_named_controls` (private)

```cpp
void refresh_named_controls( const std::shared_ptr<ControlRecord>& root, std::unordered_map<std::string, std::shared_ptr<Control>>& result)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `contains_control` (private)

```cpp
static bool contains_control(const std::shared_ptr<Control>& root, const std::shared_ptr<Control>& candidate)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `require_thread` (private)

```cpp
gf_result require_thread(const ControlRecord& record) const
```

Reports the current require thread value without mutation.

### `get_control` (private)

```cpp
gf_result get_control(gf_handle handle, std::shared_ptr<ControlRecord>& output)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `control_locked` (private)

```cpp
gf_result control_locked(gf_handle handle, std::shared_ptr<ControlRecord>& output)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `subscription_locked` (private)

```cpp
gf_result subscription_locked(gf_event_token token, std::shared_ptr<SubscriptionRecord>& output)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `slot_locked` (private)

```cpp
Slot* slot_locked(gf_handle handle)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `allocate_locked` (private)

```cpp
gf_handle allocate_locked(SlotKind kind, std::shared_ptr<ControlRecord> control, std::shared_ptr<SubscriptionRecord> subscription)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `invalidate_control_locked` (private)

```cpp
void invalidate_control_locked(gf_handle handle, ControlRecord& record)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `invalidate_slot_locked` (private)

```cpp
void invalidate_slot_locked(gf_handle handle)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `emit_changed` (private)

```cpp
void emit_changed(gf_handle sender_handle, const std::shared_ptr<ControlRecord>& sender)
```

Public Registry operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
