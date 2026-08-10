# TextureBrush

- Status: **OBSERVED: bundle 009 texture brush split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `Brush → TextureBrush`
- Declaration: `include/gui_forms/drawing/texture_brush/texture_brush.hpp:11`
- Definition: `src/core/drawing/texture_brush/texture_brush.cpp`

TextureBrush retains an immutable bitmap/image snapshot plus wrap policy and affine texture transform, so later source mutation cannot alter an authored brush unexpectedly.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `TextureBrush` (public)

```cpp
explicit TextureBrush(const Bitmap& image, WrapMode wrap_mode = WrapMode::tile)
```

Snapshots a live Bitmap/ImageReference or privately clones an existing validated brush recipe.

### `TextureBrush` (public)

```cpp
explicit TextureBrush(const ImageReference& image, WrapMode wrap_mode = WrapMode::tile)
```

Snapshots a live Bitmap/ImageReference or privately clones an existing validated brush recipe.

### `set_wrap_mode` (public)

```cpp
void set_wrap_mode(WrapMode mode)
```

Validates and commits the closed wrap vocabulary.

### `set_transform` (public)

```cpp
void set_transform(Matrix transform)
```

Requires a finite affine transform and commits it.

### `reset_transform` (public)

```cpp
void reset_transform()
```

Restores identity texture mapping.

### `translate_transform` (public)

```cpp
void translate_transform(double x, double y)
```

Composes a validated translation after current mapping.

### `scale_transform` (public)

```cpp
void scale_transform(double x, double y)
```

Validates finite nonzero scale and composes it.

### `rotate_transform` (public)

```cpp
void rotate_transform(double degrees)
```

Composes a validated origin rotation.

### `clone` (public)

```cpp
[[nodiscard]] std::unique_ptr<TextureBrush> clone() const
```

Returns independent brush state retaining the same immutable image snapshot.

### `snapshot` (public)

```cpp
[[nodiscard]] BrushSnapshot snapshot() const override
```

Requires liveness and returns the full texture recipe.

### `TextureBrush` (private)

```cpp
explicit TextureBrush(BrushSnapshot value)
```

Snapshots a live Bitmap/ImageReference or privately clones an existing validated brush recipe.
