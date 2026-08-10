# DecodeResult

- Status: **OBSERVED: bundle 009 PNG decode result review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `DecodeResult`
- Declaration: `src/render/skia/executor/skia_executor.hpp:37`
- Definition: `inline/header-only`

DecodeResult owns a decoded bitmap only when the closed raster error indicates success.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operatorbool` (public)

```cpp
[[nodiscard]] explicit operator bool() const noexcept
```

Reports success only for a non-null bitmap and no error.
