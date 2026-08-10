# DecodeResult

- Status: **OBSERVED: bundle 009 PNG decode result review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `DecodeResult`
- Declaration: `src/render/skia/executor/drawing_skia.hpp:37`
- Definition: `inline/header-only`

DecodeResult owns a decoded bitmap only when the closed raster error indicates success.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operatorbool` (public)

```cpp
[[nodiscard]] explicit operator bool() const noexcept
```

Reports success only for a non-null bitmap and no error.
