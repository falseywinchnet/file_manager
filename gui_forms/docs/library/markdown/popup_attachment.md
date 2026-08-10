# PopupAttachment

- Status: **OBSERVED: bundle 007 source-private popup attachment split; M4 build and focused tests pass**
- Kind: **class**
- Hierarchy: `Revocable → PopupAttachment`
- Declaration: `src/core/window/popup/popup_attachment.hpp:7`
- Definition: `inline/header-only`

PopupAttachment is the source-private revocable edge among Window, weak owner, weak overlay root, and ordered close event.

## Visual evidence

![PopupAttachment](../captures/native_window_host.png)

## Declared methods

### `PopupAttachment` (public)

```cpp
PopupAttachment(Window& window, Control::Ptr owner, Control::Ptr popup) : window_(&window), owner_(std::move(owner)), popup_(std::move(popup))
```

Binds one Window and weak retained owner/popup pair.

### `disconnect` (public)

```cpp
void disconnect() noexcept override
```

Requests ordinary Window close while the edge remains connected.

### `connected` (public)

```cpp
[[nodiscard]] bool connected() const noexcept override
```

Reports whether Window ownership remains live.

### `owner` (public)

```cpp
[[nodiscard]] Control::Ptr owner() const noexcept
```

Locks and returns the retained popup owner.

### `popup` (public)

```cpp
[[nodiscard]] Control::Ptr popup() const noexcept
```

Locks and returns the detached-overlay retained root.

### `closed` (public)

```cpp
[[nodiscard]] Event<>& closed() noexcept
```

Returns the ordered close event.

### `revoke` (public)

```cpp
void revoke(bool publish_closed = true) noexcept
```

Severs Window/owner/popup state and optionally publishes close.

### `publish_closed` (public)

```cpp
void publish_closed()
```

Publishes the deferred close event through the chosen ordering channel.
