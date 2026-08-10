# gf_api_v0

- Status: **generated inventory; detailed review pending**
- Kind: **struct**
- Hierarchy: `gf_api_v0`
- Declaration: `include/gui_forms/c_api.h:367`
- Definition: `inline/header-only`

gf_api_v0 is a struct declared in include/gui_forms/c_api.h.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `gf_result` (public)

```cpp
gf_result (*last_error)(gf_error_view* error)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*control_create)(gf_string_view stable_id, gf_handle* control)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*retain)(gf_handle handle)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*release)(gf_handle handle)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*dispose)(gf_handle handle)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*component_state)(gf_handle handle, uint32_t* state)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*stable_id)(gf_handle handle, char* buffer, uint64_t capacity, uint64_t* required_size)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_visible)(gf_handle handle, uint32_t visible)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*get_visible)(gf_handle handle, uint32_t* visible)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_bounds)(gf_handle handle, gf_rect bounds)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*get_bounds)(gf_handle handle, gf_rect* bounds)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*add_child)(gf_handle parent, gf_handle child)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*remove_child)(gf_handle parent, gf_handle child)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*subscribe)(gf_handle sender, uint32_t event_kind, gf_event_callback callback, void* context, gf_event_token* token)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*disconnect)(gf_event_token token)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*control_create_kind)(uint32_t kind, gf_string_view stable_id, gf_handle* control)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_name)(gf_handle handle, gf_string_view name)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*get_name)(gf_handle handle, char* buffer, uint64_t capacity, uint64_t* required_size)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_text)(gf_handle handle, gf_string_view text)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*get_text)(gf_handle handle, char* buffer, uint64_t capacity, uint64_t* required_size)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_enabled)(gf_handle handle, uint32_t enabled)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*get_enabled)(gf_handle handle, uint32_t* enabled)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*run_window)(gf_handle form, uint32_t flags)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*last_host_trace)(gf_handle form, char* buffer, uint64_t capacity, uint64_t* required_size)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*subscribe_v2)(gf_handle sender, uint32_t event_kind, gf_event_callback_v2 callback, void* context, gf_event_token* token)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*begin_invoke)(gf_handle control, gf_dispatch_callback callback, void* context)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*request_close)(gf_handle form)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*callback_fault_count)(gf_handle control, uint64_t* count)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_control_png)(gf_handle control, const uint8_t* encoded_png, uint64_t encoded_size)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_child_index)(gf_handle parent, gf_handle child, uint64_t index)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_control_colors)(gf_handle control, uint32_t foreground_argb, uint32_t background_argb)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*subscribe_pointer)(gf_handle sender, gf_pointer_callback callback, void* context, gf_event_token* token)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_check_state)(gf_handle control, uint32_t check_state)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*get_check_state)(gf_handle control, uint32_t* check_state)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*subscribe_key)(gf_handle sender, gf_key_callback callback, void* context, gf_event_token* token)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*subscribe_text)(gf_handle sender, gf_text_callback callback, void* context, gf_event_token* token)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_range)(gf_handle control, double minimum, double maximum)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*get_range)(gf_handle control, double* minimum, double* maximum)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_range_value)(gf_handle control, double value)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*get_range_value)(gf_handle control, double* value)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_pointer_capture)(gf_handle control, uint32_t captured)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*get_pointer_capture)(gf_handle control, uint32_t* captured)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*show_path_dialog)(gf_handle owner, uint32_t kind, gf_string_view title, gf_string_view initial_directory, gf_string_view suggested_name, gf_string_view default_extension, gf_string_view filter, uint32_t flags, uint32_t* accepted)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*last_dialog_path)(gf_handle owner, char* buffer, uint64_t capacity, uint64_t* required_size)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*show_tooltip)(gf_handle owner, gf_string_view text, double x, double y, uint32_t duration_milliseconds)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*hide_tooltip)(gf_handle owner)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_field_selection)(gf_handle control, uint64_t selection_start_utf8, uint64_t selection_length_utf8, uint32_t caret_visible)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_field_edit_state)(gf_handle control, uint64_t anchor_utf8, uint64_t caret_utf8, uint32_t caret_visible)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*field_position_from_point)(gf_handle control, double local_x, uint64_t* position_utf8)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*write_clipboard_text)(gf_handle owner, gf_string_view text)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*read_clipboard_text)(gf_handle owner, char* buffer, uint64_t capacity, uint64_t* required_size, uint32_t* has_text)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*field_navigate)(gf_handle control, uint64_t position_utf8, int32_t direction, uint64_t* result_utf8)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*field_replace)(gf_handle control, uint64_t start_utf8, uint64_t length_utf8, gf_string_view replacement, gf_field_edit_result* result)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*field_history)(gf_handle control, int32_t direction, gf_field_edit_result* result)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*field_clear_history)(gf_handle control)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_control_pixels)(gf_handle control, const uint8_t* pixels, uint32_t width, uint32_t height, uint64_t row_bytes, uint32_t pixel_format)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*get_control_absolute_bounds)(gf_handle control, gf_rect* bounds)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*subscribe_key_preview)(gf_handle form, gf_key_callback callback, void* context, gf_event_token* token)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_cursor)(gf_handle control, uint32_t cursor_kind)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*get_cursor)(gf_handle control, uint32_t* cursor_kind)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_auto_scroll_offset)(gf_handle control, gf_point offset)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_auto_scroll)(gf_handle control, uint32_t enabled)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_auto_scroll_margin)(gf_handle control, gf_size margin)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_auto_scroll_min_size)(gf_handle control, gf_size size)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_auto_scroll_position)(gf_handle control, gf_point position)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*get_scroll_state)(gf_handle control, gf_scroll_state* state)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_scroll_axis_state)(gf_handle control, uint32_t orientation, gf_scroll_axis_state state)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*scroll_control_into_view)(gf_handle control, gf_handle child)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*suspend_layout)(gf_handle control)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*resume_layout)(gf_handle control, uint32_t perform_layout)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*perform_control_layout)(gf_handle control)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*get_layout_state)(gf_handle control, gf_layout_state* state)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*property_grid_set_selected_controls)( gf_handle property_grid, const gf_handle* controls, uint64_t count)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*property_grid_set_sort)(gf_handle property_grid, uint32_t property_sort)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*property_grid_get_sort)(gf_handle property_grid, uint32_t* property_sort)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*property_grid_refresh)(gf_handle property_grid)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*property_object_define)( gf_handle property_object, const gf_property_descriptor_v1* descriptor, const gf_property_callbacks_v1* callbacks)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*property_object_notify_changed)( gf_handle property_object, gf_string_view property_name)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*property_grid_try_set_text)( gf_handle property_grid, gf_string_view property_name, gf_string_view text, uint32_t* committed)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*property_grid_reset_property)( gf_handle property_grid, gf_string_view property_name, uint32_t* committed)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*property_grid_activate_editor)( gf_handle property_grid, gf_string_view property_name, uint32_t* activated)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_control_text_alignment)(gf_handle control, uint32_t content_alignment)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_button_appearance)(gf_handle button, uint32_t visual_style, double flat_border_width)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*set_panel_border_style)(gf_handle panel, uint32_t border_style)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*attach_popup)(gf_handle owner, gf_handle popup)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gf_result` (public)

```cpp
gf_result (*detach_popup)(gf_handle popup)
```

Public gf_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
