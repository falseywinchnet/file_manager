# DispatcherState

- Status: **OBSERVED: bundle 007 source-private dispatcher state split; M4 build and focused tests pass**
- Kind: **struct**
- Hierarchy: `DispatcherState`
- Declaration: `src/core/dispatcher/state/dispatcher_state.hpp:31`
- Definition: `inline/header-only`

DispatcherState owns the mutex-protected bounded FIFO, immutable UI thread, coalesced host wake, sequences, outcome counters, and acceptance lifecycle.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `DispatcherState` (public)

```cpp
explicit DispatcherState(std::thread::id thread) : ui_thread(thread)
```

Binds the immutable UI-thread identity and initializes an accepting empty queue.
