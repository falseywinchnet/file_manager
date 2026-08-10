# AcceleratorAttachment

- Status: **OBSERVED: bundle 007 source-private accelerator attachment split; M4 build and focused tests pass**
- Kind: **class**
- Hierarchy: `Revocable → AcceleratorAttachment`
- Declaration: `src/core/window/accelerator/accelerator_attachment.hpp:7`
- Definition: `inline/header-only`

AcceleratorAttachment is the source-private revocable edge among Window, Component owner, physical gesture, phase policy, and callback.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `AcceleratorAttachment` (public)

```cpp
AcceleratorAttachment(Window& window, Component& owner, KeyGesture gesture, std::function<bool()> callback, AcceleratorOptions options) : window_(&window), owner_(&owner), gesture_(gesture), callback_(std::move(callback)), options_(options)
```

Binds one Window, live owner, gesture, callback, and preemptive-phase option.

### `disconnect` (public)

```cpp
void disconnect() noexcept override
```

Requests exact removal from the Window while connected.

### `connected` (public)

```cpp
[[nodiscard]] bool connected() const noexcept override
```

Reports whether arbitration may still observe the edge.

### `owner` (public)

```cpp
[[nodiscard]] Component* owner() const noexcept
```

Returns the non-owning Component lifetime anchor.

### `gesture` (public)

```cpp
[[nodiscard]] KeyGesture gesture() const noexcept
```

Returns physical-key and modifier identity.

### `options` (public)

```cpp
[[nodiscard]] AcceleratorOptions options() const noexcept
```

Returns accelerator phase policy.

### `invoke` (public)

```cpp
bool invoke()
```

Calls the live callback and returns its handled result.

### `revoke` (public)

```cpp
void revoke() noexcept
```

Severs Window/owner/callback state without publication.
