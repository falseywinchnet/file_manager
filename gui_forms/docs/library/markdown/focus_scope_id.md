# FocusScopeId

Status: **generated inventory; detailed review pending**  
Kind: **struct**  
Hierarchy: `FocusScopeId`  
Declaration: `include/gui_forms/window.hpp:225`  
Definition: `inline/header-only`

FocusScopeId is a struct declared in include/gui_forms/window.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `operatorbool`

```cpp
[[nodiscard]] explicit constexpr operator bool() const noexcept
```

Reports the current operatorbool value without mutation.

### `operator<=>`

```cpp
friend constexpr auto operator<=>(const FocusScopeId&, const FocusScopeId&) = default
```

Public FocusScopeId operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
