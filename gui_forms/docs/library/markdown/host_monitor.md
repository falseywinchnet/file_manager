# HostMonitor

- Status: **OBSERVED: bundle 007 monitor value split; M4 macOS/MinGW builds and focused tests pass**
- Kind: **struct**
- Hierarchy: `HostMonitor`
- Declaration: `include/gui_forms/host/types/host_types.hpp:21`
- Definition: `inline/header-only`

HostMonitor carries stable adapter identity, full/work logical rectangles, device scale, and primary status.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend bool operator==(const HostMonitor&, const HostMonitor&) = default
```

Compares the complete normalized monitor record.
