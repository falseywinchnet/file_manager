# BindingRecord

- Status: **generated inventory; detailed review pending**
- Kind: **struct**
- Hierarchy: `BindingRecord`
- Declaration: `include/gui_forms/binding/types/binding_contract_types.hpp:46`
- Definition: `inline/header-only`

BindingRecord is a struct declared in include/gui_forms/binding/types/binding_contract_types.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `BindingRecord` (public)

```cpp
BindingRecord() = default
```

Constructs or tears down the retained BindingRecord object according to its ownership contract.

### `BindingRecord` (public)

```cpp
BindingRecord(std::string identity, std::map<std::string, BindingValue> values, bool can_edit = true, std::map<std::string, std::string> error_values =
```

Constructs or tears down the retained BindingRecord object according to its ownership contract.

### `stable_id` (public)

```cpp
: stable_id(std::move(identity)), fields(std::move(values)), editable(can_edit), errors(std::move(error_values))
```

Public BindingRecord operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `operator==` (public)

```cpp
friend bool operator==(const BindingRecord&, const BindingRecord&) = default
```

Public BindingRecord operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
