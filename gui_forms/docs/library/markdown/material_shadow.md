# MaterialShadow

- Status: **OBSERVED: bundle 009 material shadow review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `MaterialShadow`
- Declaration: `include/gui_forms/surface_material/types/surface_material_types.hpp:69`
- Definition: `inline/header-only`

MaterialShadow specifies offset, blur, spread, and color for one bounded surface shadow.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const MaterialShadow&, const MaterialShadow&) = default
```

Compares all authored shadow fields.
