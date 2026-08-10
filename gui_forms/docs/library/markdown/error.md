# Error

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `runtime_error → Error`  
Declaration: `include/gui_forms/c_api.hpp:13`  
Definition: `inline/header-only`

Error is a class declared in include/gui_forms/c_api.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `Error`

```cpp
Error(gf_result result, std::string message) : std::runtime_error(std::move(message)), result_(result)
```

Constructs or tears down the retained Error object according to its ownership contract.

### `result`

```cpp
[[nodiscard]] gf_result result() const noexcept
```

Reports the current result value without mutation.
