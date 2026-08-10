# StableId

- Status: **generated inventory; detailed review pending**
- Kind: **class**
- Hierarchy: `StableId`
- Declaration: `include/gui_forms/control.hpp:48`
- Definition: `src/core/control.cpp`

StableId is a class declared in include/gui_forms/control.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `StableId` (public)

```cpp
explicit StableId(std::string value)
```

Constructs or tears down the retained StableId object according to its ownership contract.

### `value` (public)

```cpp
[[nodiscard]] std::string_view value() const noexcept
```

Reports the current value value without mutation.

### `operator==` (public)

```cpp
friend bool operator==(const StableId&, const StableId&) = default
```

Public StableId operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
