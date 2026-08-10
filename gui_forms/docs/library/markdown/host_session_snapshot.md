# HostSessionSnapshot

- Status: **OBSERVED: bundle 012 isolated host-session telemetry projection; native and MinGW M4 builds pass**
- Kind: **struct**
- Hierarchy: `HostSessionSnapshot`
- Declaration: `include/gui_forms/host/types/host_session_snapshot/host_session_snapshot.hpp:11`
- Definition: `src/core/host/types/host_session_snapshot/host_session_snapshot.cpp`

HostSessionSnapshot is the complete machine-readable phase, sequence, event, fault, close, display, modal, drag, attachment, activation, occlusion, closed, and shutdown state.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `to_json` (public)

```cpp
[[nodiscard]] std::string to_json() const
```

Serializes the complete session snapshot with stable capability and phase names.
