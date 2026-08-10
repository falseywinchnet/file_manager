# SizeI

- Status: **OBSERVED: bundle 009 drawing geometry review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `SizeI`
- Declaration: `include/gui_forms/drawing/geometry/size_i/size_i.hpp:7`
- Definition: `inline/header-only`

SizeI carries signed integer width and height.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const SizeI&, const SizeI&) = default
```

Compares width and height.
