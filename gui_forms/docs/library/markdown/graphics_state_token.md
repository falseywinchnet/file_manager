# GraphicsStateToken

- Status: **OBSERVED: bundle 009 graphics save-token review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `GraphicsStateToken`
- Declaration: `include/gui_forms/drawing/graphics_recorder/graphics_recorder.hpp:30`
- Definition: `inline/header-only`

GraphicsStateToken is the nonzero exact LIFO identity of one saved recorder state.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const GraphicsStateToken&, const GraphicsStateToken&) = default
```

Compares token identity.
