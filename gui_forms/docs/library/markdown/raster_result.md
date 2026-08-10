# RasterResult

- Status: **OBSERVED: bundle 009 Skia execution result review; focused M4 tests pass**
- Kind: **struct**
- Hierarchy: `RasterResult`
- Declaration: `src/render/skia/executor/skia_executor.hpp:28`
- Definition: `inline/header-only`

RasterResult couples a closed renderer error to the count of commands committed before completion.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operatorbool` (public)

```cpp
[[nodiscard]] explicit operator bool() const noexcept
```

Reports success only when no raster error occurred.
