# SkiaExecutor

- Status: **OBSERVED: bundle 009 CPU Skia executor split; focused M4 raster/trace tests pass**
- Kind: **class**
- Hierarchy: `SkiaExecutor`
- Declaration: `src/render/skia/executor/drawing_skia.hpp:53`
- Definition: `src/render/skia/executor/drawing_skia.cpp`

SkiaExecutor is the private CPU-only terminal for GUI.Drawing command streams, PNG-only codecs, deterministic text metrics, owned typefaces, and thread-affine execution.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `SkiaExecutor` (public)

```cpp
SkiaExecutor()
```

Creates a thread-affine private CPU raster implementation; moves transfer ownership and copying is prohibited.

### `~SkiaExecutor` (public)

```cpp
~SkiaExecutor()
```

Releases Skia CPU surfaces, decoded images, and typeface state.

### `SkiaExecutor` (public)

```cpp
SkiaExecutor(const SkiaExecutor&) = delete
```

Creates a thread-affine private CPU raster implementation; moves transfer ownership and copying is prohibited.

### `operator=` (public)

```cpp
SkiaExecutor& operator=(const SkiaExecutor&) = delete
```

Moves complete executor ownership; copying is prohibited.

### `register_typeface` (public)

```cpp
[[nodiscard]] bool register_typeface(std::string_view family, std::span<const std::byte> encoded, std::uint32_t style = 0U)
```

Validates and owns a bounded typeface under stable ID.

### `execute` (public)

```cpp
[[nodiscard]] RasterResult execute(const GraphicsRecorder& recorder, Bitmap& target, std::size_t first_command = 0U)
```

Validates target dimensions/stride and replays a closed command stream into caller-owned CPU pixels with isolated structured errors.

### `measure_string` (public)

```cpp
[[nodiscard]] SizeF measure_string(std::string_view utf8, const FontSnapshot& font, const StringFormatSnapshot& format, double layout_width = 0.0)
```

Measures UTF-8 through the registered CPU text stack and renderer-neutral snapshots.

### `decode_png` (public)

```cpp
[[nodiscard]] DecodeResult decode_png( std::span<const std::byte> encoded, const PngCodecLimits& limits =
```

Decodes only admitted PNG input under codec/dimension/byte limits into a Bitmap.

### `encode_png` (public)

```cpp
[[nodiscard]] std::vector<std::byte> encode_png( const Bitmap& bitmap, RasterError* error = nullptr, const PngCodecLimits& limits =
```

Encodes a live Bitmap snapshot to PNG under output limits.

### `owner_thread` (private)

```cpp
[[nodiscard]] bool owner_thread() const noexcept
```

Returns the exclusive executor thread identity.
