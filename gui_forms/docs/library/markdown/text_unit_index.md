# TextUnitIndex

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `TextUnitIndex`  
Declaration: `include/gui_forms/text.hpp:14`  
Definition: `inline/header-only`

TextUnitIndex is a class declared in include/gui_forms/text.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `TextUnitIndex`

```cpp
explicit constexpr TextUnitIndex(std::size_t value = 0) noexcept : value_(value)
```

Constructs or tears down the retained TextUnitIndex object according to its ownership contract.

### `value`

```cpp
[[nodiscard]] constexpr std::size_t value() const noexcept
```

Reports the current value value without mutation.

### `operator<=>`

```cpp
friend constexpr auto operator<=>(const TextUnitIndex &, const TextUnitIndex &) = default
```

Public TextUnitIndex operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
