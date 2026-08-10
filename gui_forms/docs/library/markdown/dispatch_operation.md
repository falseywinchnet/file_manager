# DispatchOperation

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `DispatchOperation`  
Declaration: `include/gui_forms/dispatcher.hpp:39`  
Definition: `src/core/dispatcher.cpp`

DispatchOperation is a class declared in include/gui_forms/dispatcher.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `DispatchOperation`

```cpp
DispatchOperation() = default
```

Constructs or tears down the retained DispatchOperation object according to its ownership contract.

### `DispatchOperation`

```cpp
explicit DispatchOperation(std::shared_ptr<detail::DispatchWork> work) : work_(std::move(work))
```

Constructs or tears down the retained DispatchOperation object according to its ownership contract.

### `sequence`

```cpp
[[nodiscard]] std::uint64_t sequence() const noexcept
```

Reports the current sequence value without mutation.

### `state`

```cpp
[[nodiscard]] DispatchOperationState state() const noexcept
```

Reports the current state value without mutation.

### `pending`

```cpp
[[nodiscard]] bool pending() const noexcept
```

Reports the current pending value without mutation.

### `cancel`

```cpp
[[nodiscard]] bool cancel() noexcept
```

Public DispatchOperation operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `exception`

```cpp
[[nodiscard]] std::exception_ptr exception() const noexcept
```

Reports the current exception value without mutation.
