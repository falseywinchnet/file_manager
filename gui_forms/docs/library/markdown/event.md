# Event

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Event`  
Declaration: `include/gui_forms/event.hpp:59`  
Definition: `inline/header-only`

Event is a class declared in include/gui_forms/event.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `Event`

```cpp
Event() : state_(std::make_shared<State>())
```

Constructs or tears down the retained Event object according to its ownership contract.

### `~Event`

```cpp
~Event()
```

Constructs or tears down the retained Event object according to its ownership contract.

### `Event`

```cpp
Event(const Event&) = delete
```

Constructs or tears down the retained Event object according to its ownership contract.

### `operator=`

```cpp
Event& operator=(const Event&) = delete
```

Public Event operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `subscribe`

```cpp
[[nodiscard]] SubscriptionToken subscribe(Callback callback)
```

Public Event operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `subscribe`

```cpp
[[nodiscard]] SubscriptionToken subscribe(Component& owner, Callback callback)
```

Public Event operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `emit`

```cpp
void emit(Arguments... arguments)
```

Public Event operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `disconnect_all`

```cpp
void disconnect_all() noexcept
```

Public Event operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `statistics`

```cpp
[[nodiscard]] EventStatistics statistics() const noexcept
```

Reports the current statistics value without mutation.
