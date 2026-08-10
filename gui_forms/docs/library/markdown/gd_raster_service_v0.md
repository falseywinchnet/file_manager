# gd_raster_service_v0

- Status: **OBSERVED: bundle 011 drawing raster-service review**
- Kind: **struct**
- Hierarchy: `gd_raster_service_v0`
- Declaration: `include/gui_forms/drawing_c_api.h:214`
- Definition: `inline/header-only`

gd_raster_service_v0 is the size-versioned host callback table for registering, updating, unregistering, and invalidating shared raster surfaces.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `execute` (public)

```cpp
gd_result (*execute)(const void* recorder, void* bitmap, uint64_t first_command, uint64_t* commands_executed)
```

ABI table entry for execute. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `encode_png` (public)

```cpp
gd_result (*encode_png)(const void* bitmap, void* buffer, uint64_t capacity, uint64_t* required_size)
```

ABI table entry for encode png. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `decode_png` (public)

```cpp
gd_result (*decode_png)(const void* data, uint64_t size, void** bitmap)
```

ABI table entry for decode png. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.

### `measure_string` (public)

```cpp
gd_result (*measure_string)(const void* font, const void* format, gd_string_view text, double layout_width, gd_size* measured)
```

ABI table entry for measure string. It applies the documented result-code, bounded-buffer, ownership, and thread-affinity laws.
