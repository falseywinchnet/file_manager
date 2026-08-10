# SizeF

- Status: **OBSERVED: bundle 009 drawing geometry review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `SizeF`
- Declaration: `include/gui_forms/drawing/geometry/size_f/size_f.hpp:5`
- Definition: `inline/header-only`

SizeF carries logical width and height.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const SizeF&, const SizeF&) = default
```

Compares width and height.
