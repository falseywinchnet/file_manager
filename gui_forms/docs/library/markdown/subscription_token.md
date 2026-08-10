# SubscriptionToken

- Status: **OBSERVED: bundle 011 revocable-event review**
- Kind: **class**
- Hierarchy: `SubscriptionToken`
- Declaration: `include/gui_forms/event/event/event.hpp:14`
- Definition: `inline/header-only`

SubscriptionToken is the move-only RAII revocation handle for a retained Event slot; destruction and explicit disconnect are idempotent.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

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

Executes SubscriptionToken's operator= operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `SubscriptionToken` (public)

```cpp
SubscriptionToken(const SubscriptionToken&) = delete
```

Constructs or tears down the retained SubscriptionToken object according to its ownership contract.

### `operator=` (public)

```cpp
SubscriptionToken& operator=(const SubscriptionToken&) = delete
```

Executes SubscriptionToken's operator= operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `disconnect` (public)

```cpp
void disconnect() noexcept
```

Executes SubscriptionToken's disconnect operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

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
