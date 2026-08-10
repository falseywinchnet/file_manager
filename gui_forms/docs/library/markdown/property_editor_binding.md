# PropertyEditorBinding

Status: **generated inventory; detailed review pending**  
Kind: **struct**  
Hierarchy: `PropertyEditorBinding`  
Declaration: `include/gui_forms/inspection_controls.hpp:210`  
Definition: `inline/header-only`

PropertyEditorBinding is a struct declared in include/gui_forms/inspection_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `void`

```cpp
std::function<void(const BindingValue&)> synchronize
```

Public PropertyEditorBinding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `SubscriptionToken`

```cpp
std::function<SubscriptionToken( Component&, std::function<void(BindingValue)>)> connect_committed
```

Public PropertyEditorBinding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `SubscriptionToken`

```cpp
std::function<SubscriptionToken( Component&, std::function<void(const PropertyEditorInputError&)>)> connect_failed
```

Public PropertyEditorBinding operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
