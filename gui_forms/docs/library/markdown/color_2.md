# Color

- Status: **OBSERVED: bundle 011 paint-value review**
- Kind: **struct**
- Hierarchy: `Color`
- Declaration: `include/gui_forms/types/paint_types/paint_types.hpp:11`
- Definition: `src/core/drawing/color/color.cpp`

gui_forms::Color is an explicit unpremultiplied RGBA value with opaque alpha default.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator==` (public)

```cpp
friend constexpr bool operator==(const Color&, const Color&) = default
```

Compares the complete value identity used by deterministic retained-state decisions.

### `rgba` (public)

```cpp
[[nodiscard]] static constexpr Color rgba(std::uint8_t red_value, std::uint8_t green_value, std::uint8_t blue_value, std::uint8_t alpha_value = 255) noexcept
```

Executes Color's rgba operation against retained state; the signature records its exact inputs, result, constness, and failure surface.
