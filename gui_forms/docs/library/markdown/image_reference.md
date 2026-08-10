# ImageReference

- Status: **OBSERVED: bundle 009 image-reference split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `DrawingObject → ImageReference`
- Declaration: `include/gui_forms/drawing/image_reference/image_reference.hpp:13`
- Definition: `src/core/drawing/image_reference/image_reference.cpp`

ImageReference is renderer-neutral stable image identity and dimensions without pixel storage or backend handles.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `ImageReference` (public)

```cpp
ImageReference(std::uint64_t stable_id, std::uint32_t width, std::uint32_t height, PixelFormat pixel_format, std::uint64_t generation = 1)
```

Validates nonzero identity, bounded dimensions, pixel format, and generation.

### `snapshot` (public)

```cpp
[[nodiscard]] ImageSnapshot snapshot() const
```

Requires liveness and copies identity/dimension/format/generation metadata.
