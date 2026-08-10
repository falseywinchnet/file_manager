# PopupToken

- Status: **OBSERVED: bundle 007 token/attachment split; M4 build and focused tests pass**
- Kind: **class**
- Hierarchy: `PopupToken`
- Declaration: `include/gui_forms/window/window.hpp:44`
- Definition: `src/core/window/popup/popup_token.cpp`

PopupToken is a move-only revocation handle for one Window-owned retained overlay root; dropping or disconnecting it closes the overlay through the same ordered state machine.

## Visual evidence

![PopupToken](../captures/native_window_host.png)

## Declared methods

### `PopupToken` (public)

```cpp
PopupToken() = default
```

Creates an empty token, transfers one attachment on move, prohibits copying, or is privately constructed by Window for an opened overlay.

### `~PopupToken` (public)

```cpp
~PopupToken()
```

Disconnects any remaining popup lease.

### `PopupToken` (public)

```cpp
PopupToken(PopupToken&& other) noexcept : attachment_(std::move(other.attachment_))
```

Creates an empty token, transfers one attachment on move, prohibits copying, or is privately constructed by Window for an opened overlay.

### `operator=` (public)

```cpp
PopupToken& operator=(PopupToken&& other) noexcept
```

Closes the current lease before accepting a moved attachment.

### `PopupToken` (public)

```cpp
PopupToken(const PopupToken&) = delete
```

Creates an empty token, transfers one attachment on move, prohibits copying, or is privately constructed by Window for an opened overlay.

### `operator=` (public)

```cpp
PopupToken& operator=(const PopupToken&) = delete
```

Closes the current lease before accepting a moved attachment.

### `disconnect` (public)

```cpp
void disconnect() noexcept
```

Idempotently closes the Window attachment and releases the observation handle.

### `connected` (public)

```cpp
[[nodiscard]] bool connected() const noexcept
```

Reports whether the underlying attachment remains live.

### `closed_event` (public)

```cpp
[[nodiscard]] Event<>* closed_event() noexcept
```

Returns the attachment's ordered close event while connected.

### `PopupToken` (private)

```cpp
explicit PopupToken(std::shared_ptr<detail::PopupAttachment> attachment) : attachment_(std::move(attachment))
```

Creates an empty token, transfers one attachment on move, prohibits copying, or is privately constructed by Window for an opened overlay.
