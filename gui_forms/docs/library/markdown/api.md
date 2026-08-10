# Api

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Api`  
Declaration: `include/gui_forms/c_api.hpp:24`  
Definition: `inline/header-only`

Api is a class declared in include/gui_forms/c_api.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `Api`

```cpp
Api()
```

Constructs or tears down the retained Api object according to its ownership contract.

### `table`

```cpp
[[nodiscard]] const gf_api_v0& table() const noexcept
```

Reports the current table value without mutation.

### `throw_last`

```cpp
[[noreturn]] void throw_last(gf_result result) const
```

Reports the current throw last value without mutation.
