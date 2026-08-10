# Size

- Status: **OBSERVED: bundle 011 geometry review**
- Kind: **struct**
- Hierarchy: `Size`
- Declaration: `include/gui_forms/types/geometry/geometry.hpp:13`
- Definition: `inline/header-only`

Size is a renderer-neutral logical width and height.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const Size&, const Size&) = default
```

Compares the complete value identity used by deterministic retained-state decisions.
