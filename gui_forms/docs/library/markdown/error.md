# Error

- Status: **OBSERVED: bundle 011 C++ ABI wrapper review**
- Kind: **class**
- Hierarchy: `runtime_error → Error`
- Declaration: `include/gui_forms/c_api.hpp:13`
- Definition: `inline/header-only`

Error carries the exact C ABI result code alongside a conventional C++ runtime-error message.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `Error` (public)

```cpp
Error(gf_result result, std::string message) : std::runtime_error(std::move(message)), result_(result)
```

Constructs or tears down the retained Error object according to its ownership contract.

### `result` (public)

```cpp
[[nodiscard]] gf_result result() const noexcept
```

Reports the current result value without mutation.
