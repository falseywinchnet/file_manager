# PngValidationResult

- Status: **OBSERVED: bundle 009 PNG validation result review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `PngValidationResult`
- Declaration: `include/gui_forms/resources/types/image_resource_types.hpp:58`
- Definition: `inline/header-only`

PngValidationResult couples a closed error code to metadata that is authoritative only on success.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operatorbool` (public)

```cpp
[[nodiscard]] explicit operator bool() const noexcept
```

Reports success only when the validation error is none.
