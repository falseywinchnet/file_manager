# Event

- Status: **generated inventory; detailed review pending**
- Kind: **class**
- Hierarchy: `Event`
- Declaration: `include/gui_forms/event.hpp:59`
- Definition: `inline/header-only`

Event is a class declared in include/gui_forms/event.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `Event` (public)

```cpp
Event() : state_(std::make_shared<State>())
```

Constructs or tears down the retained Event object according to its ownership contract.

### `~Event` (public)

```cpp
~Event()
```

Constructs or tears down the retained Event object according to its ownership contract.

### `Event` (public)

```cpp
Event(const Event&) = delete
```

Constructs or tears down the retained Event object according to its ownership contract.

### `operator=` (public)

```cpp
Event& operator=(const Event&) = delete
```

Public Event operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `subscribe` (public)

```cpp
[[nodiscard]] SubscriptionToken subscribe(Callback callback)
```

Public Event operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `subscribe` (public)

```cpp
[[nodiscard]] SubscriptionToken subscribe(Component& owner, Callback callback)
```

Public Event operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `emit` (public)

```cpp
void emit(Arguments... arguments)
```

Public Event operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `disconnect_all` (public)

```cpp
void disconnect_all() noexcept
```

Public Event operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `statistics` (public)

```cpp
[[nodiscard]] EventStatistics statistics() const noexcept
```

Reports the current statistics value without mutation.

### `subscribe_impl` (private)

```cpp
[[nodiscard]] SubscriptionToken subscribe_impl(Component* owner, Callback callback)
```

Public Event operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `compact` (private)

```cpp
void compact() noexcept
```

Public Event operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
