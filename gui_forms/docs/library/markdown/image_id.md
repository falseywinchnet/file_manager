# ImageId

- Status: **OBSERVED: bundle 011 image-identity review**
- Kind: **struct**
- Hierarchy: `ImageId`
- Declaration: `include/gui_forms/types/paint_types/paint_types.hpp:87`
- Definition: `inline/header-only`

ImageId is the nonzero renderer-neutral identity returned by a Window image registry.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator<=>` (public)

```cpp
friend constexpr auto operator<=>(const ImageId&, const ImageId&) = default
```

Compares the complete value identity used by deterministic retained-state decisions.
