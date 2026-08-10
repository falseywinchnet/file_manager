# TextShaper

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `TextShaper`  
Declaration: `include/gui_forms/text_shaping.hpp:117`  
Definition: `inline/header-only`

TextShaper is a class declared in include/gui_forms/text_shaping.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `~TextShaper`

```cpp
virtual ~TextShaper() = default
```

Constructs or tears down the retained TextShaper object according to its ownership contract.

### `shape`

```cpp
[[nodiscard]] virtual GlyphRun shape(const ShapingRequest &request) = 0
```

Public TextShaper operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
