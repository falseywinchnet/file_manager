# HostDispatchResult

- Status: **OBSERVED: bundle 007 lifecycle result review; M4 builds and focused tests pass**
- Kind: **struct**
- Hierarchy: `HostDispatchResult`
- Declaration: `include/gui_forms/host/types/host_types.hpp:370`
- Definition: `inline/header-only`

HostDispatchResult carries handled/deferred/capacity/close/drag outcome plus portable rejection or callback-fault category.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `accepted` (public)

```cpp
[[nodiscard]] bool accepted() const noexcept
```

Reports whether no portable dispatch error occurred.
