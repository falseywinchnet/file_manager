# gd_api_v0

- Status: **generated inventory; detailed review pending**
- Kind: **struct**
- Hierarchy: `gd_api_v0`
- Declaration: `include/gui_forms/drawing_c_api.h:229`
- Definition: `inline/header-only`

gd_api_v0 is a struct declared in include/gui_forms/drawing_c_api.h.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `gd_result` (public)

```cpp
gd_result (*last_error)(gd_error_view* error)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*retain)(gd_handle handle)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*release)(gd_handle handle)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*dispose)(gd_handle handle)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*object_state)(gd_handle handle, uint32_t* state)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*object_kind)(gd_handle handle, uint32_t* kind)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*solid_brush_create)(gd_color color, gd_handle* brush)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*pen_create)(gd_color color, double width, gd_handle* pen)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*pen_set_width)(gd_handle pen, double width)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*pen_set_dash_style)(gd_handle pen, uint32_t style)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*pen_set_dash_pattern)(gd_handle pen, const double* entries, uint64_t count)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*font_create)(gd_string_view family, double size, uint32_t style, uint32_t unit, uint32_t charset, gd_handle* font)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*string_format_create)(uint32_t flags, gd_handle* format)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*string_format_set)(gd_handle format, uint32_t alignment, uint32_t line_alignment, uint32_t trimming, uint32_t flags)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_create)(gd_handle* recorder)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_save)(gd_handle recorder, uint64_t* token)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_restore)(gd_handle recorder, uint64_t token)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_translate)(gd_handle recorder, double x, double y)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_set_transform)(gd_handle recorder, gd_matrix transform)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_set_clip)(gd_handle recorder, gd_rect clip)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_reset_clip)(gd_handle recorder)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_set_quality)(gd_handle recorder, uint32_t smoothing, uint32_t interpolation, uint32_t pixel_offset, uint32_t compositing, uint32_t compositing_quality)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_is_visible)(gd_handle recorder, gd_point point, uint32_t* visible)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_clear)(gd_handle recorder, gd_color color)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_fill_rectangle)(gd_handle recorder, gd_handle brush, gd_rect rect)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_draw_rectangle)(gd_handle recorder, gd_handle pen, gd_rect rect)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_draw_line)(gd_handle recorder, gd_handle pen, gd_point from, gd_point to)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_draw_string)(gd_handle recorder, gd_string_view text, gd_handle font, gd_handle brush, gd_point origin, gd_handle format)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_close)(gd_handle recorder)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_command_count)(gd_handle recorder, uint64_t* count)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_trace)(gd_handle recorder, char* buffer, uint64_t capacity, uint64_t* required_size)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*graphics_path_create)(uint32_t fill_mode, gd_handle* path)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*graphics_path_reset)(gd_handle path)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*graphics_path_start_figure)(gd_handle path)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*graphics_path_close_figure)(gd_handle path)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*graphics_path_add_line)(gd_handle path, gd_point from, gd_point to)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*graphics_path_add_rectangle)(gd_handle path, gd_rect rectangle)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*graphics_path_add_ellipse)(gd_handle path, gd_rect bounds)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*graphics_path_bounds)(gd_handle path, gd_rect* bounds)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*image_reference_create)(uint64_t stable_id, uint32_t width, uint32_t height, uint32_t pixel_format, uint64_t generation, gd_handle* image)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*image_attributes_create)(gd_handle* attributes)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*image_attributes_set_color_matrix)(gd_handle attributes, const double* entries, uint64_t count)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*image_attributes_reset)(gd_handle attributes)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_draw_ellipse)(gd_handle recorder, gd_handle pen, gd_rect bounds)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_fill_ellipse)(gd_handle recorder, gd_handle brush, gd_rect bounds)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_fill_polygon)(gd_handle recorder, gd_handle brush, const gd_point* points, uint64_t count, uint32_t fill_mode)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_draw_path)(gd_handle recorder, gd_handle pen, gd_handle path)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_fill_path)(gd_handle recorder, gd_handle brush, gd_handle path)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_draw_image)(gd_handle recorder, gd_handle image, gd_rect destination, gd_rect source, gd_handle attributes)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*bitmap_create)(uint32_t width, uint32_t height, uint32_t pixel_format, gd_handle* bitmap)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*bitmap_dimensions)(gd_handle bitmap, uint32_t* width, uint32_t* height, uint32_t* pixel_format, uint64_t* generation)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*bitmap_get_pixel)(gd_handle bitmap, uint32_t x, uint32_t y, gd_color* color)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*bitmap_set_pixel)(gd_handle bitmap, uint32_t x, uint32_t y, gd_color color)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*bitmap_lock)(gd_handle bitmap, uint32_t mode, gd_bitmap_lock_view* view)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*bitmap_unlock)(gd_handle bitmap, uint64_t token)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*bitmap_clone)(gd_handle bitmap, gd_rect_i source, gd_handle* clone)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*bitmap_make_transparent)(gd_handle bitmap, gd_color key)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*bitmap_thumbnail)(gd_handle bitmap, uint32_t width, uint32_t height, gd_handle* thumbnail)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*bitmap_adjusted)(gd_handle bitmap, gd_handle attributes, gd_handle* adjusted)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_draw_bitmap)(gd_handle recorder, gd_handle bitmap, gd_rect destination, gd_rect source, gd_handle attributes)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*graphics_path_add_arc)(gd_handle path, gd_rect bounds, double start_angle, double sweep_angle)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*graphics_path_add_path)(gd_handle path, gd_handle appended, uint32_t connect)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*graphics_path_transform)(gd_handle path, gd_matrix transform)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*graphics_path_is_visible)(gd_handle path, gd_point point, uint32_t* visible)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*graphics_path_points)(gd_handle path, gd_point* points, uint64_t capacity, uint64_t* required_count)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*graphics_path_clone)(gd_handle path, gd_handle* clone)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*image_attributes_set_remap_table)(gd_handle attributes, const gd_color_remap* entries, uint64_t count)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*image_attributes_reset_remap_table)(gd_handle attributes)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*hatch_brush_create)(uint32_t style, gd_color foreground, gd_color background, gd_handle* brush)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*linear_gradient_brush_create)(gd_rect bounds, gd_color first, gd_color second, double angle, uint32_t wrap_mode, gd_handle* brush)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*linear_gradient_set_blend)(gd_handle brush, const double* factors, const double* positions, uint64_t count)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*linear_gradient_set_interpolation)(gd_handle brush, const gd_color* colors, const double* positions, uint64_t count)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*linear_gradient_set_wrap_mode)(gd_handle brush, uint32_t wrap_mode)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*path_gradient_brush_create)(const gd_point* points, uint64_t count, uint32_t wrap_mode, gd_handle* brush)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*path_gradient_set_center_color)(gd_handle brush, gd_color color)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*path_gradient_set_center_point)(gd_handle brush, gd_point point)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*path_gradient_set_surround_colors)(gd_handle brush, const gd_color* colors, uint64_t count)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*path_gradient_set_interpolation)(gd_handle brush, const gd_color* colors, const double* positions, uint64_t count)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*region_create_rectangle)(gd_rect rectangle, gd_handle* region)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*region_create_path)(gd_handle path, gd_handle* region)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*region_union_rectangle)(gd_handle region, gd_rect rectangle)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*region_union_path)(gd_handle region, gd_handle path)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*region_exclude_rectangle)(gd_handle region, gd_rect rectangle)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*region_is_visible)(gd_handle region, gd_point point, uint32_t* visible)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*region_bounds)(gd_handle region, gd_rect* bounds)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*raster_service_install)(const gd_raster_service_v0* service)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_execute)(gd_handle recorder, gd_handle bitmap, uint64_t* commands_executed)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*bitmap_encode_png)(gd_handle bitmap, void* buffer, uint64_t capacity, uint64_t* required_size)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*bitmap_decode_png)(const void* data, uint64_t size, gd_handle* bitmap)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*image_attributes_clone)(gd_handle attributes, gd_handle* clone)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*bitmap_export_hbitmap)(gd_handle bitmap, gd_color background, uintptr_t* hbitmap)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*bitmap_import_hbitmap)(uintptr_t hbitmap, gd_handle* bitmap)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*native_surface_capture)(uintptr_t surface, uint32_t kind, gd_handle* bitmap, gd_rect* bounds)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*native_surface_present)(uintptr_t surface, uint32_t kind, gd_handle bitmap)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*bitmap_acquire_hdc)(gd_handle bitmap, uintptr_t* hdc, uint64_t* lease_token)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*bitmap_release_hdc)(gd_handle bitmap, uint64_t lease_token)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*measure_string)(gd_string_view text, gd_handle font, gd_handle format, double layout_width, gd_size* measured)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*recorder_execute_from)(gd_handle recorder, gd_handle bitmap, uint64_t first_command, uint64_t* commands_executed)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*bitmap_edit_begin)(gd_handle bitmap, gd_rect_i bounds, gd_bitmap_edit_view* view)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*bitmap_edit_commit)(gd_handle bitmap, uint64_t token, uint64_t* generation)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*bitmap_edit_cancel)(gd_handle bitmap, uint64_t token)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*bitmap_changes_since)(gd_handle bitmap, uint64_t generation, gd_rect_i* rectangles, uint64_t capacity, uint64_t* required_count, gd_bitmap_damage_summary* summary)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*native_surface_refresh)(uintptr_t surface, uint32_t kind, gd_handle bitmap, gd_rect* bounds)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*native_surface_publish_retained)(uintptr_t surface, uint32_t kind, gd_handle bitmap)
```

Public gd_api_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
