# TextRange

- Status: **OBSERVED: bundle 009 strongly typed text range review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `TextRange`
- Declaration: `include/gui_forms/text/types/text_types.hpp:32`
- Definition: `inline/header-only`

TextRange carries half-open start/end positions in one compile-time coordinate system.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const TextRange&, const TextRange&) = default
```

Compares both typed endpoints.

### `empty` (public)

```cpp
[[nodiscard]] constexpr bool empty() const noexcept
```

Reports equal start and end positions.
