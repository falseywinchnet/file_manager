# DisplayChunk

- Status: **OBSERVED: bundle 008 immutable display chunk review; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `DisplayChunk`
- Declaration: `src/core/display/chunk/display_chunk.hpp:10`
- Definition: `src/core/display/chunk/display_chunk.cpp`

DisplayChunk is an immutable generation- and plane-bound command sequence that can be cached per control and replayed only after a complete recording transaction succeeds.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `DisplayChunk` (public)

```cpp
DisplayChunk(std::uint64_t generation, PaintPlane plane, Rect logical_bounds, std::vector<DisplayCommand> commands)
```

Takes ownership of a complete command vector and binds it to exact generation, paint plane, and logical bounds.

### `info` (public)

```cpp
[[nodiscard]] DisplayChunkInfo info() const noexcept
```

Returns public immutable chunk telemetry without exposing command storage.

### `generation` (public)

```cpp
[[nodiscard]] std::uint64_t generation() const noexcept
```

Returns the retained display generation.

### `plane` (public)

```cpp
[[nodiscard]] PaintPlane plane() const noexcept
```

Returns the chunk's backplane, control, or overlay plane.

### `logical_bounds` (public)

```cpp
[[nodiscard]] Rect logical_bounds() const noexcept
```

Returns the coordinate bounds in which commands were authored.

### `commands` (public)

```cpp
[[nodiscard]] const std::vector<DisplayCommand>& commands() const noexcept
```

Returns const internal command storage to trusted replay code.
