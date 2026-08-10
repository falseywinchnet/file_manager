# RuntimeId

- Status: **OBSERVED: bundle 011 retained-control identity review**
- Kind: **struct**
- Hierarchy: `RuntimeId`
- Declaration: `include/gui_forms/control/control/control.hpp:49`
- Definition: `inline/header-only`

RuntimeId is the monotonic process-local identity used for exact retained instances and child operations.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `operator<=>` (public)

```cpp
friend constexpr auto operator<=>(const RuntimeId&, const RuntimeId&) = default
```

Compares the complete value identity used by deterministic retained-state decisions.
