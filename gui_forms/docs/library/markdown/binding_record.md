# BindingRecord

- Status: **OBSERVED: bundle 010 source record review; focused M4 binding tests pass**
- Kind: **struct**
- Hierarchy: `BindingRecord`
- Declaration: `include/gui_forms/binding/types/binding_contract_types.hpp:46`
- Definition: `inline/header-only`

BindingRecord is one stable editable row with canonical field values and field/record error projections.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `BindingRecord` (public)

```cpp
BindingRecord() = default
```

Default construction is empty; authored construction moves identity, fields, edit policy, and errors.

### `BindingRecord` (public)

```cpp
BindingRecord(std::string identity, std::map<std::string, BindingValue> values, bool can_edit = true, std::map<std::string, std::string> error_values =
```

Default construction is empty; authored construction moves identity, fields, edit policy, and errors.

### `stable_id` (public)

```cpp
: stable_id(std::move(identity)), fields(std::move(values)), editable(can_edit), errors(std::move(error_values))
```

Parser-visible start of the retained stable identity field.

### `operator==` (public)

```cpp
friend bool operator==(const BindingRecord&, const BindingRecord&) = default
```

Compares the complete source record.
