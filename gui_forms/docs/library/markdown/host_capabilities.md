# HostCapabilities

Status: **generated inventory; detailed review pending**  
Kind: **struct**  
Hierarchy: `HostCapabilities`  
Declaration: `include/gui_forms/host.hpp:55`  
Definition: `src/core/host.cpp`

HostCapabilities is a struct declared in include/gui_forms/host.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `supports`

```cpp
[[nodiscard]] bool supports(HostCapability capability) const noexcept
```

Reports the current supports value without mutation.

### `to_json`

```cpp
[[nodiscard]] std::string to_json() const
```

Reports the current to json value without mutation.
