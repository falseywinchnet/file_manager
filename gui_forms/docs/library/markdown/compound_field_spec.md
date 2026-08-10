# CompoundFieldSpec

- Status: **OBSERVED: bundle 011 property-grid editor review**
- Kind: **struct**
- Hierarchy: `CompoundFieldSpec`
- Declaration: `src/controls/panel/property_grid/property_grid_utilities.hpp:13`
- Definition: `inline/header-only`

CompoundFieldSpec assigns a stable field name and getter/setter pair to one structured property editor subfield.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `CompoundFieldSpec` (public)

```cpp
CompoundFieldSpec(std::string_view authored_name, BindingValueKind authored_kind, PropertyEditorKind authored_editor = PropertyEditorKind::text, std::vector<std::string> authored_choices =
```

Constructs or tears down the retained CompoundFieldSpec object according to its ownership contract.

### `name` (public)

```cpp
: name(authored_name), kind(authored_kind), editor(authored_editor), choices(std::move(authored_choices))
```

Executes CompoundFieldSpec's name operation against retained state; the signature records its exact inputs, result, constness, and failure surface.
