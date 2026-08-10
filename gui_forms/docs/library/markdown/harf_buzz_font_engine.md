# HarfBuzzFontEngine

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `HarfBuzzFontEngine`  
Declaration: `src/render/text/harfbuzz_font_engine.hpp:45`  
Definition: `src/render/text/harfbuzz_font_engine.cpp`

HarfBuzzFontEngine is a class declared in src/render/text/harfbuzz_font_engine.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `HarfBuzzFontEngine`

```cpp
HarfBuzzFontEngine()
```

Constructs or tears down the retained HarfBuzzFontEngine object according to its ownership contract.

### `~HarfBuzzFontEngine`

```cpp
~HarfBuzzFontEngine()
```

Constructs or tears down the retained HarfBuzzFontEngine object according to its ownership contract.

### `HarfBuzzFontEngine`

```cpp
HarfBuzzFontEngine(const HarfBuzzFontEngine&) = delete
```

Constructs or tears down the retained HarfBuzzFontEngine object according to its ownership contract.

### `operator=`

```cpp
HarfBuzzFontEngine& operator=(const HarfBuzzFontEngine&) = delete
```

Public HarfBuzzFontEngine operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `register_typeface`

```cpp
[[nodiscard]] std::optional<FontFaceId> register_typeface( FontRole role, std::uint16_t weight, bool italic, std::span<const std::byte> encoded, std::uint32_t face_index = 0U)
```

Public HarfBuzzFontEngine operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `register_fallback_typeface`

```cpp
[[nodiscard]] std::optional<FontFaceId> register_fallback_typeface( std::uint16_t weight, bool italic, std::span<const std::byte> encoded, std::uint32_t face_index = 0U)
```

Public HarfBuzzFontEngine operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `shape`

```cpp
[[nodiscard]] ShapedText shape(std::string_view utf8, FontSpec font)
```

Public HarfBuzzFontEngine operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `face_count`

```cpp
[[nodiscard]] std::size_t face_count() const noexcept
```

Reports the current face count value without mutation.
