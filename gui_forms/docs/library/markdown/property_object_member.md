# PropertyObjectMember

- Status: **OBSERVED: bundle 010 property member review; focused M4 binding tests pass**
- Kind: **struct**
- Hierarchy: `PropertyObjectMember`
- Declaration: `include/gui_forms/binding/value/binding_value.hpp:99`
- Definition: `inline/header-only`

PropertyObjectMember is one inert inspectable object member with optional schema, nullability, enum, standard-value, converter, and editor metadata.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `operator==` (public)

```cpp
friend bool operator==(const PropertyObjectMember&, const PropertyObjectMember&) = default
```

Compares the complete member snapshot.
