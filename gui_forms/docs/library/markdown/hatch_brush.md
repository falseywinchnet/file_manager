# HatchBrush

- Status: **OBSERVED: bundle 009 hatch brush split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `Brush → HatchBrush`
- Declaration: `include/gui_forms/drawing/brush/hatch_brush.hpp:7`
- Definition: `src/core/drawing/brush/hatch_brush.cpp`

HatchBrush retains one validated hatch vocabulary value plus foreground/background colors.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `HatchBrush` (public)

```cpp
HatchBrush(HatchStyle style, Color foreground, Color background = Color::from_argb(0U, 0U, 0U, 0U))
```

Validates the closed hatch vocabulary and retains its two colors.

### `snapshot` (public)

```cpp
[[nodiscard]] BrushSnapshot snapshot() const override
```

Requires liveness and projects a hatch BrushSnapshot.
