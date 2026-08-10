# OpenTypeTag

- Status: **OBSERVED: bundle 009 OpenType tag review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `OpenTypeTag`
- Declaration: `include/gui_forms/text_shaping/types/text_shaping_types.hpp:23`
- Definition: `inline/header-only`

OpenTypeTag stores one big-endian four-byte script or feature tag.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `valid` (public)

```cpp
[[nodiscard]] constexpr bool valid() const noexcept
```

Reports a nonzero packed tag.

### `operator<=>` (public)

```cpp
friend constexpr auto operator<=>(const OpenTypeTag&, const OpenTypeTag&) = default
```

Provides packed-tag ordering and equality.
