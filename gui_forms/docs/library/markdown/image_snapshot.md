# ImageSnapshot

- Status: **OBSERVED: bundle 012 isolated immutable-image snapshot; native and MinGW M4 builds pass**
- Kind: **struct**
- Hierarchy: `ImageSnapshot`
- Declaration: `include/gui_forms/drawing/image_snapshot/image_snapshot.hpp:15`
- Definition: `src/core/drawing/image_snapshot/image_snapshot.cpp`

ImageSnapshot couples stable identity, dimensions, format, generation, and optional shared immutable pixels.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

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
