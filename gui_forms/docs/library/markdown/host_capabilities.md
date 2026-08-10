# HostCapabilities

- Status: **OBSERVED: bundle 007 host capability value split; M4 macOS/MinGW builds and focused tests pass**
- Kind: **struct**
- Hierarchy: `HostCapabilities`
- Declaration: `include/gui_forms/host/types/host_types.hpp:52`
- Definition: `src/core/host/types/host_types.cpp`

HostCapabilities declares protocol version, platform label, and exact available portable service/event bits; absence is authoritative.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `supports` (public)

```cpp
[[nodiscard]] bool supports(HostCapability capability) const noexcept
```

Tests whether every requested capability bit is available.

### `to_json` (public)

```cpp
[[nodiscard]] std::string to_json() const
```

Serializes protocol, platform, raw bits, and stable capability names.
