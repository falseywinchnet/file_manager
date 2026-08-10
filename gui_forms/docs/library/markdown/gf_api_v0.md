# gf_api_v0

- Status: **OBSERVED: bundle 011 complete GUI.Forms ABI table review**
- Kind: **struct**
- Hierarchy: `gf_api_v0`
- Declaration: `include/gui_forms/c_api.h:367`
- Definition: `inline/header-only`

gf_api_v0 is the size-versioned negotiated function table for generational control ownership, properties, layout/scroll, hierarchy, input, fields, dialogs/services, property proxies, callbacks, host execution, dispatch, and deterministic retirement.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `last_error` (public)

```cpp
gf_result (*last_error)(gf_error_view* error)
```

ABI table entry for last error. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `control_create` (public)

```cpp
gf_result (*control_create)(gf_string_view stable_id, gf_handle* control)
```

ABI table entry for control create. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `retain` (public)

```cpp
gf_result (*retain)(gf_handle handle)
```

ABI table entry for retain. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `release` (public)

```cpp
gf_result (*release)(gf_handle handle)
```

ABI table entry for release. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `dispose` (public)

```cpp
gf_result (*dispose)(gf_handle handle)
```

ABI table entry for dispose. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `component_state` (public)

```cpp
gf_result (*component_state)(gf_handle handle, uint32_t* state)
```

ABI table entry for component state. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `stable_id` (public)

```cpp
gf_result (*stable_id)(gf_handle handle, char* buffer, uint64_t capacity, uint64_t* required_size)
```

ABI table entry for stable id. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `set_visible` (public)

```cpp
gf_result (*set_visible)(gf_handle handle, uint32_t visible)
```

Synchronously updates the retained visible property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_visible` (public)

```cpp
gf_result (*get_visible)(gf_handle handle, uint32_t* visible)
```

ABI table entry for get visible. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `set_bounds` (public)

```cpp
gf_result (*set_bounds)(gf_handle handle, gf_rect bounds)
```

Synchronously updates the retained bounds property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_bounds` (public)

```cpp
gf_result (*get_bounds)(gf_handle handle, gf_rect* bounds)
```

ABI table entry for get bounds. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `add_child` (public)

```cpp
gf_result (*add_child)(gf_handle parent, gf_handle child)
```

Adds child to gf_api_v0's retained ownership model after validating identity and lifetime constraints.

### `remove_child` (public)

```cpp
gf_result (*remove_child)(gf_handle parent, gf_handle child)
```

Removes the exact child entry and publishes the resulting retained-state change when one exists.

### `subscribe` (public)

```cpp
gf_result (*subscribe)(gf_handle sender, uint32_t event_kind, gf_event_callback callback, void* context, gf_event_token* token)
```

Connects a revocable callback in deterministic registration order.

### `disconnect` (public)

```cpp
gf_result (*disconnect)(gf_event_token token)
```

ABI table entry for disconnect. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `control_create_kind` (public)

```cpp
gf_result (*control_create_kind)(uint32_t kind, gf_string_view stable_id, gf_handle* control)
```

ABI table entry for control create kind. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `set_name` (public)

```cpp
gf_result (*set_name)(gf_handle handle, gf_string_view name)
```

Synchronously updates the retained name property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_name` (public)

```cpp
gf_result (*get_name)(gf_handle handle, char* buffer, uint64_t capacity, uint64_t* required_size)
```

ABI table entry for get name. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `set_text` (public)

```cpp
gf_result (*set_text)(gf_handle handle, gf_string_view text)
```

Synchronously updates the retained text property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_text` (public)

```cpp
gf_result (*get_text)(gf_handle handle, char* buffer, uint64_t capacity, uint64_t* required_size)
```

ABI table entry for get text. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `set_enabled` (public)

```cpp
gf_result (*set_enabled)(gf_handle handle, uint32_t enabled)
```

Synchronously updates the retained enabled property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_enabled` (public)

```cpp
gf_result (*get_enabled)(gf_handle handle, uint32_t* enabled)
```

ABI table entry for get enabled. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `run_window` (public)

```cpp
gf_result (*run_window)(gf_handle form, uint32_t flags)
```

ABI table entry for run window. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `last_host_trace` (public)

```cpp
gf_result (*last_host_trace)(gf_handle form, char* buffer, uint64_t capacity, uint64_t* required_size)
```

ABI table entry for last host trace. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `subscribe_v2` (public)

```cpp
gf_result (*subscribe_v2)(gf_handle sender, uint32_t event_kind, gf_event_callback_v2 callback, void* context, gf_event_token* token)
```

Connects a revocable callback in deterministic registration order.

### `begin_invoke` (public)

```cpp
gf_result (*begin_invoke)(gf_handle control, gf_dispatch_callback callback, void* context)
```

ABI table entry for begin invoke. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `request_close` (public)

```cpp
gf_result (*request_close)(gf_handle form)
```

ABI table entry for request close. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `callback_fault_count` (public)

```cpp
gf_result (*callback_fault_count)(gf_handle control, uint64_t* count)
```

ABI table entry for callback fault count. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `set_control_png` (public)

```cpp
gf_result (*set_control_png)(gf_handle control, const uint8_t* encoded_png, uint64_t encoded_size)
```

Synchronously updates the retained control png property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_child_index` (public)

```cpp
gf_result (*set_child_index)(gf_handle parent, gf_handle child, uint64_t index)
```

Synchronously updates the retained child index property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_control_colors` (public)

```cpp
gf_result (*set_control_colors)(gf_handle control, uint32_t foreground_argb, uint32_t background_argb)
```

Synchronously updates the retained control colors property. Validation, typed invalidation, and notifications are defined by the implementation.

### `subscribe_pointer` (public)

```cpp
gf_result (*subscribe_pointer)(gf_handle sender, gf_pointer_callback callback, void* context, gf_event_token* token)
```

Connects a revocable callback in deterministic registration order.

### `set_check_state` (public)

```cpp
gf_result (*set_check_state)(gf_handle control, uint32_t check_state)
```

Synchronously updates the retained check state property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_check_state` (public)

```cpp
gf_result (*get_check_state)(gf_handle control, uint32_t* check_state)
```

ABI table entry for get check state. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `subscribe_key` (public)

```cpp
gf_result (*subscribe_key)(gf_handle sender, gf_key_callback callback, void* context, gf_event_token* token)
```

Connects a revocable callback in deterministic registration order.

### `subscribe_text` (public)

```cpp
gf_result (*subscribe_text)(gf_handle sender, gf_text_callback callback, void* context, gf_event_token* token)
```

Connects a revocable callback in deterministic registration order.

### `set_range` (public)

```cpp
gf_result (*set_range)(gf_handle control, double minimum, double maximum)
```

Synchronously updates the retained range property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_range` (public)

```cpp
gf_result (*get_range)(gf_handle control, double* minimum, double* maximum)
```

ABI table entry for get range. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `set_range_value` (public)

```cpp
gf_result (*set_range_value)(gf_handle control, double value)
```

Synchronously updates the retained range value property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_range_value` (public)

```cpp
gf_result (*get_range_value)(gf_handle control, double* value)
```

ABI table entry for get range value. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `set_pointer_capture` (public)

```cpp
gf_result (*set_pointer_capture)(gf_handle control, uint32_t captured)
```

Synchronously updates the retained pointer capture property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_pointer_capture` (public)

```cpp
gf_result (*get_pointer_capture)(gf_handle control, uint32_t* captured)
```

ABI table entry for get pointer capture. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `show_path_dialog` (public)

```cpp
gf_result (*show_path_dialog)(gf_handle owner, uint32_t kind, gf_string_view title, gf_string_view initial_directory, gf_string_view suggested_name, gf_string_view default_extension, gf_string_view filter, uint32_t flags, uint32_t* accepted)
```

ABI table entry for show path dialog. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `last_dialog_path` (public)

```cpp
gf_result (*last_dialog_path)(gf_handle owner, char* buffer, uint64_t capacity, uint64_t* required_size)
```

ABI table entry for last dialog path. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `show_tooltip` (public)

```cpp
gf_result (*show_tooltip)(gf_handle owner, gf_string_view text, double x, double y, uint32_t duration_milliseconds)
```

ABI table entry for show tooltip. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `hide_tooltip` (public)

```cpp
gf_result (*hide_tooltip)(gf_handle owner)
```

ABI table entry for hide tooltip. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `set_field_selection` (public)

```cpp
gf_result (*set_field_selection)(gf_handle control, uint64_t selection_start_utf8, uint64_t selection_length_utf8, uint32_t caret_visible)
```

Synchronously updates the retained field selection property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_field_edit_state` (public)

```cpp
gf_result (*set_field_edit_state)(gf_handle control, uint64_t anchor_utf8, uint64_t caret_utf8, uint32_t caret_visible)
```

Synchronously updates the retained field edit state property. Validation, typed invalidation, and notifications are defined by the implementation.

### `field_position_from_point` (public)

```cpp
gf_result (*field_position_from_point)(gf_handle control, double local_x, uint64_t* position_utf8)
```

ABI table entry for field position from point. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `write_clipboard_text` (public)

```cpp
gf_result (*write_clipboard_text)(gf_handle owner, gf_string_view text)
```

ABI table entry for write clipboard text. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `read_clipboard_text` (public)

```cpp
gf_result (*read_clipboard_text)(gf_handle owner, char* buffer, uint64_t capacity, uint64_t* required_size, uint32_t* has_text)
```

ABI table entry for read clipboard text. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `field_navigate` (public)

```cpp
gf_result (*field_navigate)(gf_handle control, uint64_t position_utf8, int32_t direction, uint64_t* result_utf8)
```

ABI table entry for field navigate. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `field_replace` (public)

```cpp
gf_result (*field_replace)(gf_handle control, uint64_t start_utf8, uint64_t length_utf8, gf_string_view replacement, gf_field_edit_result* result)
```

ABI table entry for field replace. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `field_history` (public)

```cpp
gf_result (*field_history)(gf_handle control, int32_t direction, gf_field_edit_result* result)
```

ABI table entry for field history. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `field_clear_history` (public)

```cpp
gf_result (*field_clear_history)(gf_handle control)
```

ABI table entry for field clear history. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `set_control_pixels` (public)

```cpp
gf_result (*set_control_pixels)(gf_handle control, const uint8_t* pixels, uint32_t width, uint32_t height, uint64_t row_bytes, uint32_t pixel_format)
```

Synchronously updates the retained control pixels property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_control_absolute_bounds` (public)

```cpp
gf_result (*get_control_absolute_bounds)(gf_handle control, gf_rect* bounds)
```

ABI table entry for get control absolute bounds. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `subscribe_key_preview` (public)

```cpp
gf_result (*subscribe_key_preview)(gf_handle form, gf_key_callback callback, void* context, gf_event_token* token)
```

Connects a revocable callback in deterministic registration order.

### `set_cursor` (public)

```cpp
gf_result (*set_cursor)(gf_handle control, uint32_t cursor_kind)
```

Synchronously updates the retained cursor property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_cursor` (public)

```cpp
gf_result (*get_cursor)(gf_handle control, uint32_t* cursor_kind)
```

ABI table entry for get cursor. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `set_auto_scroll_offset` (public)

```cpp
gf_result (*set_auto_scroll_offset)(gf_handle control, gf_point offset)
```

Synchronously updates the retained auto scroll offset property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_auto_scroll` (public)

```cpp
gf_result (*set_auto_scroll)(gf_handle control, uint32_t enabled)
```

Synchronously updates the retained auto scroll property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_auto_scroll_margin` (public)

```cpp
gf_result (*set_auto_scroll_margin)(gf_handle control, gf_size margin)
```

Synchronously updates the retained auto scroll margin property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_auto_scroll_min_size` (public)

```cpp
gf_result (*set_auto_scroll_min_size)(gf_handle control, gf_size size)
```

Synchronously updates the retained auto scroll min size property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_auto_scroll_position` (public)

```cpp
gf_result (*set_auto_scroll_position)(gf_handle control, gf_point position)
```

Synchronously updates the retained auto scroll position property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_scroll_state` (public)

```cpp
gf_result (*get_scroll_state)(gf_handle control, gf_scroll_state* state)
```

ABI table entry for get scroll state. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `set_scroll_axis_state` (public)

```cpp
gf_result (*set_scroll_axis_state)(gf_handle control, uint32_t orientation, gf_scroll_axis_state state)
```

Synchronously updates the retained scroll axis state property. Validation, typed invalidation, and notifications are defined by the implementation.

### `scroll_control_into_view` (public)

```cpp
gf_result (*scroll_control_into_view)(gf_handle control, gf_handle child)
```

ABI table entry for scroll control into view. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `suspend_layout` (public)

```cpp
gf_result (*suspend_layout)(gf_handle control)
```

ABI table entry for suspend layout. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `resume_layout` (public)

```cpp
gf_result (*resume_layout)(gf_handle control, uint32_t perform_layout)
```

ABI table entry for resume layout. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `perform_control_layout` (public)

```cpp
gf_result (*perform_control_layout)(gf_handle control)
```

ABI table entry for perform control layout. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `get_layout_state` (public)

```cpp
gf_result (*get_layout_state)(gf_handle control, gf_layout_state* state)
```

ABI table entry for get layout state. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `property_grid_set_selected_controls` (public)

```cpp
gf_result (*property_grid_set_selected_controls)( gf_handle property_grid, const gf_handle* controls, uint64_t count)
```

ABI table entry for property grid set selected controls. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `property_grid_set_sort` (public)

```cpp
gf_result (*property_grid_set_sort)(gf_handle property_grid, uint32_t property_sort)
```

ABI table entry for property grid set sort. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `property_grid_get_sort` (public)

```cpp
gf_result (*property_grid_get_sort)(gf_handle property_grid, uint32_t* property_sort)
```

ABI table entry for property grid get sort. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `property_grid_refresh` (public)

```cpp
gf_result (*property_grid_refresh)(gf_handle property_grid)
```

ABI table entry for property grid refresh. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `property_object_define` (public)

```cpp
gf_result (*property_object_define)( gf_handle property_object, const gf_property_descriptor_v1* descriptor, const gf_property_callbacks_v1* callbacks)
```

ABI table entry for property object define. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `property_object_notify_changed` (public)

```cpp
gf_result (*property_object_notify_changed)( gf_handle property_object, gf_string_view property_name)
```

ABI table entry for property object notify changed. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `property_grid_try_set_text` (public)

```cpp
gf_result (*property_grid_try_set_text)( gf_handle property_grid, gf_string_view property_name, gf_string_view text, uint32_t* committed)
```

ABI table entry for property grid try set text. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `property_grid_reset_property` (public)

```cpp
gf_result (*property_grid_reset_property)( gf_handle property_grid, gf_string_view property_name, uint32_t* committed)
```

ABI table entry for property grid reset property. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `property_grid_activate_editor` (public)

```cpp
gf_result (*property_grid_activate_editor)( gf_handle property_grid, gf_string_view property_name, uint32_t* activated)
```

ABI table entry for property grid activate editor. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `set_control_text_alignment` (public)

```cpp
gf_result (*set_control_text_alignment)(gf_handle control, uint32_t content_alignment)
```

Synchronously updates the retained control text alignment property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_button_appearance` (public)

```cpp
gf_result (*set_button_appearance)(gf_handle button, uint32_t visual_style, double flat_border_width)
```

Synchronously updates the retained button appearance property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_panel_border_style` (public)

```cpp
gf_result (*set_panel_border_style)(gf_handle panel, uint32_t border_style)
```

Synchronously updates the retained panel border style property. Validation, typed invalidation, and notifications are defined by the implementation.

### `attach_popup` (public)

```cpp
gf_result (*attach_popup)(gf_handle owner, gf_handle popup)
```

ABI table entry for attach popup. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `detach_popup` (public)

```cpp
gf_result (*detach_popup)(gf_handle popup)
```

ABI table entry for detach popup. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.
