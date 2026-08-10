# AcceleratorToken

- Status: **OBSERVED: bundle 007 token/attachment split; M4 build and focused tests pass**
- Kind: **class**
- Hierarchy: `AcceleratorToken`
- Declaration: `include/gui_forms/window/window.hpp:105`
- Definition: `src/core/window/accelerator/accelerator_token.cpp`

AcceleratorToken is the move-only revocation handle for one Window accelerator registration whose lifetime is also bounded by its Component owner.

## Visual evidence

![AcceleratorToken](../captures/native_window_host.png)

## Declared methods

### `AcceleratorToken` (public)

```cpp
AcceleratorToken() = default
```

Creates empty, transfers one attachment on move, prohibits copying, or is privately constructed by Window after registration.

### `~AcceleratorToken` (public)

```cpp
~AcceleratorToken()
```

Disconnects any remaining registration.

### `AcceleratorToken` (public)

```cpp
AcceleratorToken(AcceleratorToken&& other) noexcept : attachment_(std::move(other.attachment_))
```

Creates empty, transfers one attachment on move, prohibits copying, or is privately constructed by Window after registration.

### `operator=` (public)

```cpp
AcceleratorToken& operator=(AcceleratorToken&& other) noexcept
```

Revokes the current registration before accepting a moved attachment.

### `AcceleratorToken` (public)

```cpp
AcceleratorToken(const AcceleratorToken&) = delete
```

Creates empty, transfers one attachment on move, prohibits copying, or is privately constructed by Window after registration.

### `operator=` (public)

```cpp
AcceleratorToken& operator=(const AcceleratorToken&) = delete
```

Revokes the current registration before accepting a moved attachment.

### `disconnect` (public)

```cpp
void disconnect() noexcept
```

Idempotently removes the exact accelerator from its Window.

### `connected` (public)

```cpp
[[nodiscard]] bool connected() const noexcept
```

Reports whether the registration is still eligible for arbitration.

### `AcceleratorToken` (private)

```cpp
explicit AcceleratorToken( std::shared_ptr<detail::AcceleratorAttachment> attachment) : attachment_(std::move(attachment))
```

Creates empty, transfers one attachment on move, prohibits copying, or is privately constructed by Window after registration.
