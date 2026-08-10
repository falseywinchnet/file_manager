# PropertyDescriptor

- Status: **OBSERVED: bundle 010 inert property descriptor review; focused M4 inspection tests pass**
- Kind: **struct**
- Hierarchy: `PropertyDescriptor`
- Declaration: `include/gui_forms/binding/value/binding_value.hpp:145`
- Definition: `inline/header-only`

PropertyDescriptor exposes renderer/language-neutral schema, serialization, mutability, invalidation, enum, standard-value, converter, and editor metadata without executable callbacks.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `PropertyDescriptor` (public)

```cpp
PropertyDescriptor() = default
```

Default construction supplies a writable text behavior descriptor; authored construction commits the core schema and invalidation contract.

### `PropertyDescriptor` (public)

```cpp
PropertyDescriptor(std::string authored_name, BindingValueKind value_kind, std::string authored_category, std::string authored_description, std::optional<BindingValue> authored_default, Dirty authored_effects, bool authored_subtree_effect = false) : name(std::move(authored_name)), kind(value_kind), category(std::move(authored_category)), description(std::move(authored_description)), default_value(std::move(authored_default)), invalidation_effects(authored_effects), invalidates_subtree(authored_subtree_effect)
```

Default construction supplies a writable text behavior descriptor; authored construction commits the core schema and invalidation contract.
