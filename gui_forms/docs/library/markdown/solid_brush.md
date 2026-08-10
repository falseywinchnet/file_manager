# SolidBrush

- Status: **OBSERVED: bundle 009 solid brush split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `Brush → SolidBrush`
- Declaration: `include/gui_forms/drawing/brush/brush.hpp:13`
- Definition: `src/core/drawing/brush/solid_brush.cpp`

SolidBrush retains one color under DrawingObject lifetime and thread-affinity rules.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `SolidBrush` (public)

```cpp
explicit SolidBrush(Color color)
```

Retains one authored color.

### `color` (public)

```cpp
[[nodiscard]] Color color() const
```

Requires liveness and returns the retained color.

### `snapshot` (public)

```cpp
[[nodiscard]] BrushSnapshot snapshot() const override
```

Requires liveness and projects a solid BrushSnapshot.
