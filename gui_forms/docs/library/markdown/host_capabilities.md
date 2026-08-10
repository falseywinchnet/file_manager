# HostCapabilities

- Status: **OBSERVED: bundle 012 isolated host-capability projection; native and MinGW M4 builds pass**
- Kind: **struct**
- Hierarchy: `HostCapabilities`
- Declaration: `include/gui_forms/host/types/host_capabilities/host_capabilities.hpp:10`
- Definition: `src/core/host/types/host_capabilities/host_capabilities.cpp`

HostCapabilities declares protocol version, platform label, and exact available portable service/event bits; absence is authoritative.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

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
