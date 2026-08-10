# MetricsSnapshot

- Status: **OBSERVED: bundle 010 structured telemetry split; focused M4 core tests pass**
- Kind: **struct**
- Hierarchy: `MetricsSnapshot`
- Declaration: `include/gui_forms/metrics/types/metrics_types.hpp:8`
- Definition: `src/core/metrics/snapshot/metrics_snapshot.cpp`

MetricsSnapshot is the renderer-neutral population, traversal, damage, input, lifetime, scheduler, presentation, and renderer telemetry record.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `to_json` (public)

```cpp
[[nodiscard]] std::string to_json() const
```

Serializes every field with escaped renderer identity into one deterministic JSON object.
