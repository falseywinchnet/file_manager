# SkiaExecutor

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `SkiaExecutor`  
Declaration: `src/render/skia/drawing_skia.hpp:53`  
Definition: `src/render/skia/drawing_skia.cpp`

SkiaExecutor is a class declared in src/render/skia/drawing_skia.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `SkiaExecutor`

```cpp
SkiaExecutor()
```

Constructs or tears down the retained SkiaExecutor object according to its ownership contract.

### `~SkiaExecutor`

```cpp
~SkiaExecutor()
```

Constructs or tears down the retained SkiaExecutor object according to its ownership contract.

### `SkiaExecutor`

```cpp
SkiaExecutor(const SkiaExecutor&) = delete
```

Constructs or tears down the retained SkiaExecutor object according to its ownership contract.

### `operator=`

```cpp
SkiaExecutor& operator=(const SkiaExecutor&) = delete
```

Public SkiaExecutor operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `register_typeface`

```cpp
[[nodiscard]] bool register_typeface(std::string_view family, std::span<const std::byte> encoded, std::uint32_t style = 0U)
```

Public SkiaExecutor operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `execute`

```cpp
[[nodiscard]] RasterResult execute(const GraphicsRecorder& recorder, Bitmap& target, std::size_t first_command = 0U)
```

Public SkiaExecutor operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `measure_string`

```cpp
[[nodiscard]] SizeF measure_string(std::string_view utf8, const FontSnapshot& font, const StringFormatSnapshot& format, double layout_width = 0.0)
```

Public SkiaExecutor operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `decode_png`

```cpp
[[nodiscard]] DecodeResult decode_png( std::span<const std::byte> encoded, const PngCodecLimits& limits =
```

Public SkiaExecutor operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `encode_png`

```cpp
[[nodiscard]] std::vector<std::byte> encode_png( const Bitmap& bitmap, RasterError* error = nullptr, const PngCodecLimits& limits =
```

Public SkiaExecutor operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
