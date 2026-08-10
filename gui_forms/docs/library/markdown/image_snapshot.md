# ImageSnapshot

- Status: **OBSERVED: bundle 009 immutable image snapshot review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `ImageSnapshot`
- Declaration: `include/gui_forms/drawing/types/drawing_types.hpp:40`
- Definition: `src/core/drawing/image_reference/image_reference.cpp`

ImageSnapshot couples stable identity, dimensions, format, generation, and optional shared immutable pixels.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `has_pixels` (public)

```cpp
[[nodiscard]] bool has_pixels() const noexcept
```

Reports whether shared pixel storage is retained.

### `row_bytes` (public)

```cpp
[[nodiscard]] std::size_t row_bytes() const noexcept
```

Returns storage stride or zero.

### `pixels` (public)

```cpp
[[nodiscard]] std::span<const std::byte> pixels() const noexcept
```

Returns const storage bytes or an empty span.
