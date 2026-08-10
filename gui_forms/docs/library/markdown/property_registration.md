# PropertyRegistration

- Status: **OBSERVED: bundle 010 executable property registration review; focused M4 binding tests pass**
- Kind: **struct**
- Hierarchy: `PropertyRegistration`
- Declaration: `include/gui_forms/binding/value/binding_value.hpp:202`
- Definition: `inline/header-only`

PropertyRegistration privately couples an inert descriptor to explicit getter, setter, change, reset, serialization, and origin callbacks registered by a retained Control.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `PropertyRegistration` (public)

```cpp
PropertyRegistration() = default
```

Default construction is empty; authored construction moves the descriptor and callback set.

### `PropertyRegistration` (public)

```cpp
PropertyRegistration(PropertyDescriptor authored_descriptor, Getter authored_get, Setter authored_set, ChangeConnector authored_change =
```

Default construction is empty; authored construction moves the descriptor and callback set.

### `descriptor` (public)

```cpp
: descriptor(std::move(authored_descriptor)), get(std::move(authored_get)), set(std::move(authored_set)), connect_changed(std::move(authored_change)), reset(std::move(authored_reset)), should_serialize(std::move(authored_should_serialize)), origin(std::move(authored_origin))
```

Parser-visible declaration of the inert descriptor stored before executable callbacks.
