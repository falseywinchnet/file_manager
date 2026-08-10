# Slot

- Status: **OBSERVED: bundle 011 revocable-event state review**
- Kind: **struct**
- Hierarchy: `Revocable → Slot`
- Declaration: `include/gui_forms/event/event/event.hpp:105`
- Definition: `inline/header-only`

Event::Slot owns one callback, weakly references shared event state, and guarantees idempotent disconnection plus exact statistics.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `Slot` (public)

```cpp
Slot(std::weak_ptr<State> event_state, Callback event_callback) : state(std::move(event_state)), callback(std::move(event_callback))
```

Constructs or tears down the retained Slot object according to its ownership contract.

### `disconnect` (public)

```cpp
void disconnect() noexcept override
```

Executes Slot's disconnect operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `connected` (public)

```cpp
[[nodiscard]] bool connected() const noexcept override
```

Reports the current connected value without mutation.
