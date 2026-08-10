# Utf8ValidationResult

- Status: **OBSERVED: bundle 009 UTF-8 validation result review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `Utf8ValidationResult`
- Declaration: `include/gui_forms/text/types/text_types.hpp:50`
- Definition: `inline/header-only`

Utf8ValidationResult preserves the first closed validation failure and exact byte/scalar/UTF-16 counts.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `valid` (public)

```cpp
[[nodiscard]] constexpr bool valid() const noexcept
```

Reports whether the complete sequence passed validation.
