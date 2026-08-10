# FontFallbackResolver

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `FontFallbackResolver`  
Declaration: `include/gui_forms/text_shaping.hpp:136`  
Definition: `inline/header-only`

FontFallbackResolver is a class declared in include/gui_forms/text_shaping.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `~FontFallbackResolver`

```cpp
virtual ~FontFallbackResolver() = default
```

Constructs or tears down the retained FontFallbackResolver object according to its ownership contract.

### `resolve`

```cpp
[[nodiscard]] virtual std::optional<FontFallbackMatch> resolve(const FontFallbackRequest &request) = 0
```

Public FontFallbackResolver operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
