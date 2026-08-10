# Color

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Color`  
Declaration: `include/gui_forms/drawing.hpp:96`  
Definition: `src/core/drawing.cpp`

Color is a class declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `Color`

```cpp
constexpr Color() noexcept = default
```

Constructs or tears down the retained Color object according to its ownership contract.

### `empty`

```cpp
[[nodiscard]] static constexpr Color empty() noexcept
```

Public Color operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `from_argb`

```cpp
[[nodiscard]] static constexpr Color from_argb(std::uint32_t argb) noexcept
```

Public Color operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `from_argb`

```cpp
[[nodiscard]] static constexpr Color from_argb(std::uint8_t alpha, std::uint8_t red, std::uint8_t green, std::uint8_t blue) noexcept
```

Public Color operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `from_rgb`

```cpp
[[nodiscard]] static constexpr Color from_rgb(std::uint8_t red, std::uint8_t green, std::uint8_t blue) noexcept
```

Public Color operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `with_alpha`

```cpp
[[nodiscard]] static constexpr Color with_alpha(std::uint8_t alpha, Color color) noexcept
```

Public Color operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `from_name`

```cpp
[[nodiscard]] static Color from_name(std::string_view name)
```

Public Color operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `from_html`

```cpp
[[nodiscard]] static Color from_html(std::string_view value)
```

Public Color operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `is_empty`

```cpp
[[nodiscard]] constexpr bool is_empty() const noexcept
```

Reports the current is empty value without mutation.

### `is_known`

```cpp
[[nodiscard]] constexpr bool is_known() const noexcept
```

Reports the current is known value without mutation.

### `argb`

```cpp
[[nodiscard]] constexpr std::uint32_t argb() const noexcept
```

Reports the current argb value without mutation.

### `alpha`

```cpp
[[nodiscard]] constexpr std::uint8_t alpha() const noexcept
```

Reports the current alpha value without mutation.

### `red`

```cpp
[[nodiscard]] constexpr std::uint8_t red() const noexcept
```

Reports the current red value without mutation.

### `green`

```cpp
[[nodiscard]] constexpr std::uint8_t green() const noexcept
```

Reports the current green value without mutation.

### `blue`

```cpp
[[nodiscard]] constexpr std::uint8_t blue() const noexcept
```

Reports the current blue value without mutation.

### `brightness`

```cpp
[[nodiscard]] double brightness() const noexcept
```

Reports the current brightness value without mutation.

### `operator==`

```cpp
friend constexpr bool operator==(const Color&, const Color&) = default
```

Public Color operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
