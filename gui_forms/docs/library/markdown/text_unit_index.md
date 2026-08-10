# TextUnitIndex

- Status: **OBSERVED: bundle 009 strongly typed text index review; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `TextUnitIndex`
- Declaration: `include/gui_forms/text/types/text_types.hpp:10`
- Definition: `inline/header-only`

TextUnitIndex prevents accidental mixing of byte, UTF-16, scalar, grapheme, and line coordinate systems.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `TextUnitIndex` (public)

```cpp
explicit constexpr TextUnitIndex(std::size_t value = 0) noexcept : value_(value)
```

Constructs an explicit index from a unit count.

### `value` (public)

```cpp
[[nodiscard]] constexpr std::size_t value() const noexcept
```

Returns the stored unit count.

### `operator<=>` (public)

```cpp
friend constexpr auto operator<=>(const TextUnitIndex&, const TextUnitIndex&) = default
```

Orders indices only within the same tagged coordinate system.
