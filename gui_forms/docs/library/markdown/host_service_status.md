# HostServiceStatus

- Status: **OBSERVED: bundle 007 service result value split; M4 builds and focused tests pass**
- Kind: **struct**
- Hierarchy: `HostServiceStatus`
- Declaration: `include/gui_forms/host/types/host_types.hpp:43`
- Definition: `inline/header-only`

HostServiceStatus carries the portable service rejection/failure category without native error types.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `accepted` (public)

```cpp
[[nodiscard]] bool accepted() const noexcept
```

Reports the error-free result.
