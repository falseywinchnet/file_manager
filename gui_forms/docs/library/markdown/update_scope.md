# UpdateScope

- Status: **OBSERVED: bundle 007 lifecycle transaction split; M4 build and focused tests pass**
- Kind: **class**
- Hierarchy: `UpdateScope`
- Declaration: `include/gui_forms/window/window.hpp:678`
- Definition: `src/core/window/lifecycle/window_lifecycle.cpp`

UpdateScope is the move-only RAII token for one Window update-depth level; it guarantees balanced close and makes an explicit layout barrier available without opening a nested native loop.

## Visual evidence

![UpdateScope](../captures/native_window_host.png)

## Declared methods

### `UpdateScope` (public)

```cpp
explicit UpdateScope(Window& window) noexcept : window_(&window)
```

Enters with one Window reference, transfers the sole close responsibility on move, and prohibits copying.

### `~UpdateScope` (public)

```cpp
~UpdateScope()
```

Closes an outstanding level exactly once.

### `UpdateScope` (public)

```cpp
UpdateScope(UpdateScope&& other) noexcept
```

Enters with one Window reference, transfers the sole close responsibility on move, and prohibits copying.

### `operator=` (public)

```cpp
UpdateScope& operator=(UpdateScope&& other) noexcept
```

Closes any currently owned level before accepting another move-only responsibility.

### `UpdateScope` (public)

```cpp
UpdateScope(const UpdateScope&) = delete
```

Enters with one Window reference, transfers the sole close responsibility on move, and prohibits copying.

### `operator=` (public)

```cpp
UpdateScope& operator=(const UpdateScope&) = delete
```

Closes any currently owned level before accepting another move-only responsibility.

### `perform_layout` (public)

```cpp
void perform_layout()
```

Requests the Window layout barrier while retaining the surrounding update level.

### `close` (public)

```cpp
void close()
```

Atomically clears its Window pointer and leaves exactly one update level.
