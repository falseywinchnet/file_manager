# Api

- Status: **OBSERVED: bundle 011 C++ ABI wrapper review**
- Kind: **class**
- Hierarchy: `Api`
- Declaration: `include/gui_forms/c_api.hpp:24`
- Definition: `inline/header-only`

Api negotiates ABI 0.4 once, retains the immutable function table, and translates thread-local C diagnostics into typed C++ exceptions.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `Api` (public)

```cpp
Api()
```

Constructs or tears down the retained Api object according to its ownership contract.

### `table` (public)

```cpp
[[nodiscard]] const gf_api_v0& table() const noexcept
```

Reports the current table value without mutation.

### `throw_last` (public)

```cpp
[[noreturn]] void throw_last(gf_result result) const
```

Reports the current throw last value without mutation.
