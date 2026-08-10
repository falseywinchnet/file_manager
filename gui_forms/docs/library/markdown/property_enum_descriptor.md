# PropertyEnumDescriptor

- Status: **OBSERVED: bundle 010 enum schema review; focused M4 binding tests pass**
- Kind: **struct**
- Hierarchy: `PropertyEnumDescriptor`
- Declaration: `include/gui_forms/binding/value/binding_value.hpp:183`
- Definition: `inline/header-only`

PropertyEnumDescriptor supplies bounded named choices and explicit flags semantics for descriptor-aware conversion.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend bool operator==(const PropertyEnumDescriptor&, const PropertyEnumDescriptor&) = default
```

Compares type name, choices, and flags policy.
