# XyzD65

- Status: **OBSERVED: bundle 009 color-space value review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `XyzD65`
- Declaration: `include/gui_forms/drawing/color/color.hpp:85`
- Definition: `inline/header-only`

XyzD65 carries normalized CIE XYZ D65 coordinates plus straight alpha.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const XyzD65&, const XyzD65&) = default
```

Compares XYZ and alpha.
