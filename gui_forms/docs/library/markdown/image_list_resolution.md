# ImageListResolution

- Status: **OBSERVED: bundle 011 image-list review**
- Kind: **struct**
- Hierarchy: `ImageListResolution`
- Declaration: `include/gui_forms/image_list/image_list/image_list.hpp:48`
- Definition: `inline/header-only`

ImageListResolution returns the exact registered ImageId plus source pixel size, chosen scale variant, revision, and stable entry identity.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operatorbool` (public)

```cpp
[[nodiscard]] explicit operator bool() const noexcept
```

Reports whether the record contains a usable resolved value.
