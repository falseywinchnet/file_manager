# BasicControlStyle

- Status: **OBSERVED: bundle 009 compatibility palette review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `BasicControlStyle`
- Declaration: `include/gui_forms/theme/types/theme_types.hpp:12`
- Definition: `inline/header-only`

BasicControlStyle retains the compact compatibility palette used by callers that do not select full role recipes.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const BasicControlStyle&, const BasicControlStyle&) = default
```

Compares every compatibility color.
