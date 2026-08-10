# Color

Status: **generated inventory; detailed review pending**  
Kind: **struct**  
Hierarchy: `Color`  
Declaration: `include/gui_forms/types.hpp:71`  
Definition: `src/core/drawing.cpp`

Color is a struct declared in include/gui_forms/types.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `operator==`

```cpp
friend constexpr bool operator==(const Color&, const Color&) = default
```

Public Color operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `rgba`

```cpp
[[nodiscard]] static constexpr Color rgba(std::uint8_t red_value, std::uint8_t green_value, std::uint8_t blue_value, std::uint8_t alpha_value = 255) noexcept
```

Public Color operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
