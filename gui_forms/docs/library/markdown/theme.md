# Theme

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Theme`  
Declaration: `include/gui_forms/theme.hpp:182`  
Definition: `src/core/theme.cpp`

Theme is a class declared in include/gui_forms/theme.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `create`

```cpp
[[nodiscard]] static std::shared_ptr<const Theme> create( ThemeDefinition definition)
```

Public Theme operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `id`

```cpp
[[nodiscard]] std::string_view id() const noexcept
```

Reports the current id value without mutation.

### `basic_style`

```cpp
[[nodiscard]] const BasicControlStyle& basic_style() const noexcept
```

Reports the current basic style value without mutation.

### `structure`

```cpp
[[nodiscard]] const ThemeStructureTokens& structure() const noexcept
```

Reports the current structure value without mutation.

### `resolve`

```cpp
[[nodiscard]] const ControlVisualRecipe& resolve( ControlVisualRole role, ControlVisualContext context) const noexcept
```

Reports the current resolve value without mutation.

### `definition`

```cpp
[[nodiscard]] const ThemeDefinition& definition() const noexcept
```

Reports the current definition value without mutation.
