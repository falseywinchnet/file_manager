# gd_raster_service_v0

- Status: **generated inventory; detailed review pending**
- Kind: **struct**
- Hierarchy: `gd_raster_service_v0`
- Declaration: `include/gui_forms/drawing_c_api.h:214`
- Definition: `inline/header-only`

gd_raster_service_v0 is a struct declared in include/gui_forms/drawing_c_api.h.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `gd_result` (public)

```cpp
gd_result (*execute)(const void* recorder, void* bitmap, uint64_t first_command, uint64_t* commands_executed)
```

Public gd_raster_service_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*encode_png)(const void* bitmap, void* buffer, uint64_t capacity, uint64_t* required_size)
```

Public gd_raster_service_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*decode_png)(const void* data, uint64_t size, void** bitmap)
```

Public gd_raster_service_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `gd_result` (public)

```cpp
gd_result (*measure_string)(const void* font, const void* format, gd_string_view text, double layout_width, gd_size* measured)
```

Public gd_raster_service_v0 operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
