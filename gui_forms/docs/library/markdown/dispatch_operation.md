# DispatchOperation

- Status: **OBSERVED: bundle 007 operation/state split; M4 build and focused tests pass**
- Kind: **class**
- Hierarchy: `DispatchOperation`
- Declaration: `include/gui_forms/dispatcher/operation/dispatch_operation.hpp:28`
- Definition: `src/core/dispatcher/operation/dispatch_operation.cpp`

DispatchOperation observes and explicitly cancels one bounded FIFO work record; dropping it does not cancel fire-and-forget work.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `DispatchOperation` (public)

```cpp
DispatchOperation() = default
```

Creates an invalid observer or is internally bound to one shared DispatchWork record.

### `DispatchOperation` (public)

```cpp
explicit DispatchOperation(std::shared_ptr<detail::DispatchWork> work) : work_(std::move(work))
```

Creates an invalid observer or is internally bound to one shared DispatchWork record.

### `sequence` (public)

```cpp
[[nodiscard]] std::uint64_t sequence() const noexcept
```

Returns the nonzero FIFO sequence or zero for an invalid observer.

### `state` (public)

```cpp
[[nodiscard]] DispatchOperationState state() const noexcept
```

Atomically reports invalid, pending, running, completed, cancelled, or faulted.

### `pending` (public)

```cpp
[[nodiscard]] bool pending() const noexcept
```

Tests the pending state.

### `cancel` (public)

```cpp
[[nodiscard]] bool cancel() noexcept
```

Atomically changes only pending work to cancelled and releases synchronous waiters.

### `exception` (public)

```cpp
[[nodiscard]] std::exception_ptr exception() const noexcept
```

Returns the recorded callback exception under its fault lock.

### `wait_and_rethrow` (private)

```cpp
void wait_and_rethrow() const
```

Waits without pumping until terminal state, returns on success, rethrows the callback fault, or raises DispatchCancelledError.
