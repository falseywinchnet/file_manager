# StableId

- Status: **OBSERVED: bundle 012 isolated retained-control identity owner; native and MinGW M4 builds pass**
- Kind: **class**
- Hierarchy: `StableId`
- Declaration: `include/gui_forms/control/stable_id/stable_id.hpp:10`
- Definition: `src/core/control/stable_id/stable_id.cpp`

StableId owns the immutable authored UTF-8 identity used for lookup, semantics, traces, and cross-boundary correlation.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `StableId` (public)

```cpp
explicit StableId(std::string value)
```

Constructs or tears down the retained StableId object according to its ownership contract.

### `value` (public)

```cpp
[[nodiscard]] std::string_view value() const noexcept
```

Reports the current value value without mutation.

### `operator==` (public)

```cpp
friend bool operator==(const StableId&, const StableId&) = default
```

Compares the complete value identity used by deterministic retained-state decisions.
