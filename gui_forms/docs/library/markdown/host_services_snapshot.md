# HostServicesSnapshot

- Status: **OBSERVED: bundle 007 service telemetry review; M4 builds and focused tests pass**
- Kind: **struct**
- Hierarchy: `HostServicesSnapshot`
- Declaration: `include/gui_forms/host/types/host_types.hpp:254`
- Definition: `src/core/host/types/host_types.cpp`

HostServicesSnapshot is machine-readable capability, cursor/capture, monitor/clipboard/dialog/sound, coalescing-policy, modal-depth, rejection, and shutdown state.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `to_json` (public)

```cpp
[[nodiscard]] std::string to_json() const
```

Serializes the complete portable service snapshot with stable names.
