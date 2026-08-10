# Theme

- Status: **OBSERVED: bundle 009 theme split; focused M4 tests and native showcase pass**
- Kind: **class**
- Hierarchy: `Theme`
- Declaration: `include/gui_forms/theme/theme/theme.hpp:11`
- Definition: `src/core/theme/theme/theme.cpp`

Theme is an immutable validated role/state recipe table plus structural spacing, geometry, typography, and motion tokens; it resolves logical visual context without exposing renderer or platform objects.

## Visual evidence

![Theme](../captures/drawing_raster_material.png)

## Declared methods

### `create` (public)

```cpp
[[nodiscard]] static std::shared_ptr<const Theme> create( ThemeDefinition definition)
```

Validates bounded identity, every ordinary/selected/high-contrast recipe, and ordered finite structural tokens before publishing shared immutable ownership.

### `id` (public)

```cpp
[[nodiscard]] std::string_view id() const noexcept
```

Returns the immutable theme identity.

### `basic_style` (public)

```cpp
[[nodiscard]] const BasicControlStyle& basic_style() const noexcept
```

Returns the compatibility color projection used by explicitly legacy-styled controls.

### `structure` (public)

```cpp
[[nodiscard]] const ThemeStructureTokens& structure() const noexcept
```

Returns immutable spacing, geometry, typography, and motion token scales.

### `resolve` (public)

```cpp
[[nodiscard]] const ControlVisualRecipe& resolve( ControlVisualRole role, ControlVisualContext context) const noexcept
```

Bounds role/state indices and chooses ordinary, selected, high-contrast, or high-contrast-selected recipe deterministically.

### `definition` (public)

```cpp
[[nodiscard]] const ThemeDefinition& definition() const noexcept
```

Returns the complete immutable definition for inspection or derivation.

### `Theme` (private)

```cpp
explicit Theme(ThemeDefinition definition) : definition_(std::move(definition))
```

Privately takes an already validated ThemeDefinition so published themes remain immutable.
