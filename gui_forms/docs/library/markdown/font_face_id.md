# FontFaceId

- Status: **OBSERVED: bundle 009 font identity review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `FontFaceId`
- Declaration: `include/gui_forms/text_shaping/types/text_shaping_types.hpp:13`
- Definition: `inline/header-only`

FontFaceId is the ordered nonzero identity shared across shaping and raster registries.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `valid` (public)

```cpp
[[nodiscard]] constexpr bool valid() const noexcept
```

Reports nonzero identity.

### `operator<=>` (public)

```cpp
friend constexpr auto operator<=>(const FontFaceId&, const FontFaceId&) = default
```

Provides value ordering and equality.
