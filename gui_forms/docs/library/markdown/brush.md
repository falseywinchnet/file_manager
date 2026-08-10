# Brush

- Status: **OBSERVED: bundle 009 brush interface split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `DrawingObject → Brush`
- Declaration: `include/gui_forms/drawing/brush/brush.hpp:8`
- Definition: `inline/header-only`

Brush is the disposable renderer-neutral base whose only rendering contract is an immutable BrushSnapshot.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `snapshot` (public)

```cpp
[[nodiscard]] virtual BrushSnapshot snapshot() const = 0
```

Requires each subtype to return a complete backend-neutral brush recipe.
