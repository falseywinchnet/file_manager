# HarfBuzzFontEngine

- Status: **OBSERVED: bundle 009 text-engine split; focused M4 shaping tests pass**
- Kind: **class**
- Hierarchy: `HarfBuzzFontEngine`
- Declaration: `src/render/text/harfbuzz/harfbuzz_font_engine.hpp:45`
- Definition: `src/render/text/harfbuzz/harfbuzz_font_engine.cpp`

HarfBuzzFontEngine owns bounded FreeType faces and HarfBuzz shaping state behind renderer-neutral font IDs, deterministic fallback ordering, absolute UTF-8 clusters, and immutable shaped runs.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `HarfBuzzFontEngine` (public)

```cpp
HarfBuzzFontEngine()
```

Constructs the private FreeType/HarfBuzz implementation; copying is prohibited and moves transfer complete engine ownership.

### `~HarfBuzzFontEngine` (public)

```cpp
~HarfBuzzFontEngine()
```

Releases HarfBuzz fonts/faces and FreeType ownership in dependency-safe order.

### `HarfBuzzFontEngine` (public)

```cpp
HarfBuzzFontEngine(const HarfBuzzFontEngine&) = delete
```

Constructs the private FreeType/HarfBuzz implementation; copying is prohibited and moves transfer complete engine ownership.

### `operator=` (public)

```cpp
HarfBuzzFontEngine& operator=(const HarfBuzzFontEngine&) = delete
```

Moves complete engine ownership; copying is prohibited.

### `register_typeface` (public)

```cpp
[[nodiscard]] std::optional<FontFaceId> register_typeface( FontRole role, std::uint16_t weight, bool italic, std::span<const std::byte> encoded, std::uint32_t face_index = 0U)
```

Validates bounded bytes and unique stable ID, owns the font bytes, creates FreeType/HarfBuzz faces, and commits only a usable face.

### `register_fallback_typeface` (public)

```cpp
[[nodiscard]] std::optional<FontFaceId> register_fallback_typeface( std::uint16_t weight, bool italic, std::span<const std::byte> encoded, std::uint32_t face_index = 0U)
```

Registers a face and appends its ID once to deterministic fallback order.

### `shape` (public)

```cpp
[[nodiscard]] ShapedText shape(std::string_view utf8, FontSpec font)
```

Validates request UTF-8/ranges, segments unsupported clusters across primary/fallback faces, shapes with HarfBuzz, and returns absolute cluster metadata and aggregate metrics.

### `face_count` (public)

```cpp
[[nodiscard]] std::size_t face_count() const noexcept
```

Returns the number of committed faces.
