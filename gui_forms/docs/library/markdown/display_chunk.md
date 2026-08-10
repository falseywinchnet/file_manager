# DisplayChunk

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `DisplayChunk`  
Declaration: `src/core/display_chunk.hpp:55`  
Definition: `src/core/display_chunk.cpp`

DisplayChunk is a class declared in src/core/display_chunk.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `DisplayChunk`

```cpp
DisplayChunk(std::uint64_t generation, PaintPlane plane, Rect logical_bounds, std::vector<DisplayCommand> commands)
```

Constructs or tears down the retained DisplayChunk object according to its ownership contract.

### `info`

```cpp
[[nodiscard]] DisplayChunkInfo info() const noexcept
```

Reports the current info value without mutation.

### `generation`

```cpp
[[nodiscard]] std::uint64_t generation() const noexcept
```

Reports the current generation value without mutation.

### `plane`

```cpp
[[nodiscard]] PaintPlane plane() const noexcept
```

Reports the current plane value without mutation.

### `logical_bounds`

```cpp
[[nodiscard]] Rect logical_bounds() const noexcept
```

Reports the current logical bounds value without mutation.

### `commands`

```cpp
[[nodiscard]] const std::vector<DisplayCommand>& commands() const noexcept
```

Reports the current commands value without mutation.
