# HostSessionSnapshot

- Status: **OBSERVED: bundle 007 lifecycle telemetry review; M4 builds and focused tests pass**
- Kind: **struct**
- Hierarchy: `HostSessionSnapshot`
- Declaration: `include/gui_forms/host/types/host_types.hpp:383`
- Definition: `src/core/host/types/host_types.cpp`

HostSessionSnapshot is the complete machine-readable phase, sequence, event, fault, close, display, modal, drag, attachment, activation, occlusion, closed, and shutdown state.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `to_json` (public)

```cpp
[[nodiscard]] std::string to_json() const
```

Serializes the complete session snapshot with stable capability and phase names.
