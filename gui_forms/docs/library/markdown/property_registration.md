# PropertyRegistration

- Status: **generated inventory; detailed review pending**
- Kind: **struct**
- Hierarchy: `PropertyRegistration`
- Declaration: `include/gui_forms/binding/value/binding_value.hpp:258`
- Definition: `inline/header-only`

PropertyRegistration is a struct declared in include/gui_forms/binding/value/binding_value.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `PropertyRegistration` (public)

```cpp
PropertyRegistration() = default
```

Constructs or tears down the retained PropertyRegistration object according to its ownership contract.

### `PropertyRegistration` (public)

```cpp
PropertyRegistration(PropertyDescriptor authored_descriptor, Getter authored_get, Setter authored_set, ChangeConnector authored_change =
```

Constructs or tears down the retained PropertyRegistration object according to its ownership contract.

### `descriptor` (public)

```cpp
: descriptor(std::move(authored_descriptor)), get(std::move(authored_get)), set(std::move(authored_set)), connect_changed(std::move(authored_change)), reset(std::move(authored_reset)), should_serialize(std::move(authored_should_serialize)), origin(std::move(authored_origin))
```

Public PropertyRegistration operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
