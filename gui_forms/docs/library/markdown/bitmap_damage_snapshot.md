# BitmapDamageSnapshot

- Status: **OBSERVED: bundle 009 bitmap damage review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `BitmapDamageSnapshot`
- Declaration: `include/gui_forms/drawing/bitmap/bitmap.hpp:40`
- Definition: `inline/header-only`

BitmapDamageSnapshot reports generation interval, history completeness, and bounded changed rectangles.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `empty` (public)

```cpp
[[nodiscard]] bool empty() const noexcept
```

Reports whether no changed rectangles were retained.
