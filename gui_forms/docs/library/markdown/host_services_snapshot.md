# HostServicesSnapshot

- Status: **OBSERVED: bundle 012 isolated host-service telemetry projection; native and MinGW M4 builds pass**
- Kind: **struct**
- Hierarchy: `HostServicesSnapshot`
- Declaration: `include/gui_forms/host/types/host_services_snapshot/host_services_snapshot.hpp:11`
- Definition: `src/core/host/types/host_services_snapshot/host_services_snapshot.cpp`

HostServicesSnapshot is machine-readable capability, cursor/capture, monitor/clipboard/dialog/sound, coalescing-policy, modal-depth, rejection, and shutdown state.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `to_json` (public)

```cpp
[[nodiscard]] std::string to_json() const
```

Serializes the complete portable service snapshot with stable names.
