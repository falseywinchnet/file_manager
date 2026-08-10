# TextRange

Status: **generated inventory; detailed review pending**  
Kind: **struct**  
Hierarchy: `TextRange`  
Declaration: `include/gui_forms/text.hpp:39`  
Definition: `inline/header-only`

TextRange is a struct declared in include/gui_forms/text.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `operator==`

```cpp
friend constexpr bool operator==(const TextRange &, const TextRange &) = default
```

Public TextRange operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `empty`

```cpp
[[nodiscard]] constexpr bool empty() const noexcept
```

Reports the current empty value without mutation.
