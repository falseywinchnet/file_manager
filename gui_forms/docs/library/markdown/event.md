# Event

- Status: **OBSERVED: bundle 011 revocable-event state-machine review**
- Kind: **class**
- Hierarchy: `Event`
- Declaration: `include/gui_forms/event/event/event.hpp:59`
- Definition: `inline/header-only`

Event owns registration-ordered revocable callback slots, emits a stable snapshot, skips slots revoked before their turn, and compacts disconnected ownership after publication.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

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

Executes Event's operator= operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `subscribe` (public)

```cpp
[[nodiscard]] SubscriptionToken subscribe(Callback callback)
```

Connects a revocable callback in deterministic registration order.

### `subscribe` (public)

```cpp
[[nodiscard]] SubscriptionToken subscribe(Component& owner, Callback callback)
```

Connects a revocable callback in deterministic registration order.

### `emit` (public)

```cpp
void emit(Arguments... arguments)
```

Publishes a stable callback snapshot so mutation during delivery affects only later emissions.

### `disconnect_all` (public)

```cpp
void disconnect_all() noexcept
```

Executes Event's disconnect all operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `statistics` (public)

```cpp
[[nodiscard]] EventStatistics statistics() const noexcept
```

Reports the current statistics value without mutation.

### `subscribe_impl` (private)

```cpp
[[nodiscard]] SubscriptionToken subscribe_impl(Component* owner, Callback callback)
```

Connects a revocable callback in deterministic registration order.

### `compact` (private)

```cpp
void compact() noexcept
```

Executes Event's compact operation against retained state; the signature records its exact inputs, result, constness, and failure surface.
