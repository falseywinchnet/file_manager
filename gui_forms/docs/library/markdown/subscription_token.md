# SubscriptionToken

- Status: **generated inventory; detailed review pending**
- Kind: **class**
- Hierarchy: `SubscriptionToken`
- Declaration: `include/gui_forms/event.hpp:14`
- Definition: `inline/header-only`

SubscriptionToken is a class declared in include/gui_forms/event.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `SubscriptionToken` (public)

```cpp
SubscriptionToken() = default
```

Constructs or tears down the retained SubscriptionToken object according to its ownership contract.

### `~SubscriptionToken` (public)

```cpp
~SubscriptionToken()
```

Constructs or tears down the retained SubscriptionToken object according to its ownership contract.

### `SubscriptionToken` (public)

```cpp
SubscriptionToken(SubscriptionToken&& other) noexcept : revocable_(std::move(other.revocable_))
```

Constructs or tears down the retained SubscriptionToken object according to its ownership contract.

### `operator=` (public)

```cpp
SubscriptionToken& operator=(SubscriptionToken&& other) noexcept
```

Public SubscriptionToken operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `SubscriptionToken` (public)

```cpp
SubscriptionToken(const SubscriptionToken&) = delete
```

Constructs or tears down the retained SubscriptionToken object according to its ownership contract.

### `operator=` (public)

```cpp
SubscriptionToken& operator=(const SubscriptionToken&) = delete
```

Public SubscriptionToken operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `disconnect` (public)

```cpp
void disconnect() noexcept
```

Public SubscriptionToken operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `connected` (public)

```cpp
[[nodiscard]] bool connected() const noexcept
```

Reports the current connected value without mutation.

### `SubscriptionToken` (private)

```cpp
explicit SubscriptionToken(std::shared_ptr<detail::Revocable> revocable) : revocable_(std::move(revocable))
```

Constructs or tears down the retained SubscriptionToken object according to its ownership contract.
