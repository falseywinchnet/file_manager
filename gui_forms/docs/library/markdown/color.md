# Color

- Status: **OBSERVED: bundle 009 GUI.Drawing color split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `Color`
- Declaration: `include/gui_forms/drawing/color/color.hpp:10`
- Definition: `src/core/drawing/color/color.cpp`

gui_drawing::Color is a compact empty/known/ARGB value with deterministic named/HTML parsing and explicit linear-sRGB, XYZ D65, OKLab, and OKLCH conversion support.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `Color` (public)

```cpp
constexpr Color() noexcept = default
```

Constructs empty color publicly or exact private ARGB/empty/known state for validated factories.

### `empty` (public)

```cpp
[[nodiscard]] static constexpr Color empty() noexcept
```

Returns the explicit no-color sentinel.

### `from_argb` (public)

```cpp
[[nodiscard]] static constexpr Color from_argb(std::uint32_t argb) noexcept
```

Constructs straight-alpha ARGB from packed or channel values.

### `from_argb` (public)

```cpp
[[nodiscard]] static constexpr Color from_argb(std::uint8_t alpha, std::uint8_t red, std::uint8_t green, std::uint8_t blue) noexcept
```

Constructs straight-alpha ARGB from packed or channel values.

### `from_rgb` (public)

```cpp
[[nodiscard]] static constexpr Color from_rgb(std::uint8_t red, std::uint8_t green, std::uint8_t blue) noexcept
```

Constructs an opaque RGB color.

### `with_alpha` (public)

```cpp
[[nodiscard]] static constexpr Color with_alpha(std::uint8_t alpha, Color color) noexcept
```

Replaces alpha while preserving empty and known-color identity semantics.

### `from_name` (public)

```cpp
[[nodiscard]] static Color from_name(std::string_view name)
```

Performs case-insensitive lookup in the bounded admitted named-color vocabulary and returns empty when unknown.

### `from_html` (public)

```cpp
[[nodiscard]] static Color from_html(std::string_view value)
```

Parses admitted names or #RGB/#RRGGBB/#AARRGGBB forms and rejects malformed lengths/digits.

### `is_empty` (public)

```cpp
[[nodiscard]] constexpr bool is_empty() const noexcept
```

Reports the explicit no-color sentinel.

### `is_known` (public)

```cpp
[[nodiscard]] constexpr bool is_known() const noexcept
```

Reports construction through the admitted named-color table.

### `argb` (public)

```cpp
[[nodiscard]] constexpr std::uint32_t argb() const noexcept
```

Returns packed straight-alpha ARGB.

### `alpha` (public)

```cpp
[[nodiscard]] constexpr std::uint8_t alpha() const noexcept
```

Returns the alpha channel.

### `red` (public)

```cpp
[[nodiscard]] constexpr std::uint8_t red() const noexcept
```

Returns the red channel.

### `green` (public)

```cpp
[[nodiscard]] constexpr std::uint8_t green() const noexcept
```

Returns the green channel.

### `blue` (public)

```cpp
[[nodiscard]] constexpr std::uint8_t blue() const noexcept
```

Returns the blue channel.

### `brightness` (public)

```cpp
[[nodiscard]] double brightness() const noexcept
```

Returns deterministic HSL-style lightness from channel extrema, or zero for empty.

### `operator==` (public)

```cpp
friend constexpr bool operator==(const Color&, const Color&) = default
```

Compares packed channels plus empty/known identity flags.

### `Color` (private)

```cpp
constexpr Color(std::uint32_t argb, bool empty, bool known) noexcept : argb_(argb), empty_(empty), known_(known)
```

Constructs empty color publicly or exact private ARGB/empty/known state for validated factories.

### `known` (private)

```cpp
[[nodiscard]] static constexpr Color known(std::uint32_t argb) noexcept
```

Privately constructs a table-recognized color.
