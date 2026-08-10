# CompoundFieldSpec

- Status: **generated inventory; detailed review pending**
- Kind: **struct**
- Hierarchy: `CompoundFieldSpec`
- Declaration: `src/controls/panel/property_grid/property_grid_utilities.hpp:13`
- Definition: `inline/header-only`

CompoundFieldSpec is a struct declared in src/controls/panel/property_grid/property_grid_utilities.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

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

Public CompoundFieldSpec operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
