# MaterialBorder

- Status: **OBSERVED: bundle 009 material border review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `MaterialBorder`
- Declaration: `include/gui_forms/surface_material/types/surface_material_types.hpp:63`
- Definition: `inline/header-only`

MaterialBorder pairs an authored color and logical stroke width.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const MaterialBorder&, const MaterialBorder&) = default
```

Compares color and width.
