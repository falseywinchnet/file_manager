# ImageLoadResult

- Status: **OBSERVED: bundle 009 image load result review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `ImageLoadResult`
- Declaration: `include/gui_forms/resources/types/image_resource_types.hpp:70`
- Definition: `inline/header-only`

ImageLoadResult couples a stable nonzero image identity to a closed registry error.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operatorbool` (public)

```cpp
[[nodiscard]] explicit operator bool() const noexcept
```

Reports success only for a nonzero identity and no error.
