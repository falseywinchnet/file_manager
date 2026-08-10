# OpenTypeTag

Status: **generated inventory; detailed review pending**  
Kind: **struct**  
Hierarchy: `OpenTypeTag`  
Declaration: `include/gui_forms/text_shaping.hpp:27`  
Definition: `inline/header-only`

OpenTypeTag is a struct declared in include/gui_forms/text_shaping.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `valid`

```cpp
[[nodiscard]] constexpr bool valid() const noexcept
```

Reports the current valid value without mutation.

### `operator<=>`

```cpp
friend constexpr auto operator<=>(const OpenTypeTag &, const OpenTypeTag &) = default
```

Public OpenTypeTag operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
