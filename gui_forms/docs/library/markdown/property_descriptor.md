# PropertyDescriptor

- Status: **generated inventory; detailed review pending**
- Kind: **struct**
- Hierarchy: `PropertyDescriptor`
- Declaration: `include/gui_forms/binding/value/binding_value.hpp:201`
- Definition: `inline/header-only`

PropertyDescriptor is a struct declared in include/gui_forms/binding/value/binding_value.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `PropertyDescriptor` (public)

```cpp
PropertyDescriptor() = default
```

Constructs or tears down the retained PropertyDescriptor object according to its ownership contract.

### `PropertyDescriptor` (public)

```cpp
PropertyDescriptor(std::string authored_name, BindingValueKind value_kind, std::string authored_category, std::string authored_description, std::optional<BindingValue> authored_default, Dirty authored_effects, bool authored_subtree_effect = false) : name(std::move(authored_name)), kind(value_kind), category(std::move(authored_category)), description(std::move(authored_description)), default_value(std::move(authored_default)), invalidation_effects(authored_effects), invalidates_subtree(authored_subtree_effect)
```

Constructs or tears down the retained PropertyDescriptor object according to its ownership contract.
