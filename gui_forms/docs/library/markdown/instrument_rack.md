# InstrumentRack

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Panel → InstrumentRack`  
Declaration: `include/gui_forms/instrument_controls.hpp:78`  
Definition: `src/controls/instrument_controls.cpp`

InstrumentRack is a visual retained control declared in include/gui_forms/instrument_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `InstrumentRack`

```cpp
explicit InstrumentRack(StableId stable_id)
```

Constructs or tears down the retained InstrumentRack object according to its ownership contract.

### `~InstrumentRack`

```cpp
~InstrumentRack() override
```

Constructs or tears down the retained InstrumentRack object according to its ownership contract.

### `modules`

```cpp
[[nodiscard]] const std::vector<InstrumentModuleSpec>& modules() const noexcept
```

Reports the current modules value without mutation.

### `set_modules`

```cpp
void set_modules(std::vector<InstrumentModuleSpec> modules)
```

Synchronously updates the retained modules property. Validation, typed invalidation, and notifications are defined by the implementation.

### `field_editor`

```cpp
[[nodiscard]] Control::Ptr field_editor(std::string_view module_id, std::string_view field_id) const
```

Reports the current field editor value without mutation.

### `module_bounds`

```cpp
[[nodiscard]] std::optional<Rect> module_bounds( std::string_view module_id) const noexcept
```

Reports the current module bounds value without mutation.

### `set_module_enabled`

```cpp
bool set_module_enabled(std::string_view module_id, bool enabled)
```

Synchronously updates the retained module enabled property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_module_state`

```cpp
bool set_module_state(std::string_view module_id, InstrumentModuleState state, std::string status_text)
```

Synchronously updates the retained module state property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_field_value`

```cpp
bool set_field_value(std::string_view module_id, std::string_view field_id, std::string value)
```

Synchronously updates the retained field value property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_field_validation`

```cpp
bool set_field_validation(std::string_view module_id, std::string_view field_id, std::string message)
```

Synchronously updates the retained field validation property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_action_content`

```cpp
void set_action_content(Control::Ptr content, double minimum_width = 170.0)
```

Synchronously updates the retained action content property. Validation, typed invalidation, and notifications are defined by the implementation.

### `action_content`

```cpp
[[nodiscard]] Control::Ptr action_content() const noexcept
```

Reports the current action content value without mutation.

### `module_width`

```cpp
[[nodiscard]] double module_width() const noexcept
```

Reports the current module width value without mutation.

### `set_module_width`

```cpp
void set_module_width(double width)
```

Synchronously updates the retained module width property. Validation, typed invalidation, and notifications are defined by the implementation.

### `module_height`

```cpp
[[nodiscard]] double module_height() const noexcept
```

Reports the current module height value without mutation.

### `set_module_height`

```cpp
void set_module_height(double height)
```

Synchronously updates the retained module height property. Validation, typed invalidation, and notifications are defined by the implementation.

### `rack_gap`

```cpp
[[nodiscard]] double rack_gap() const noexcept
```

Reports the current rack gap value without mutation.

### `set_rack_gap`

```cpp
void set_rack_gap(double gap)
```

Synchronously updates the retained rack gap property. Validation, typed invalidation, and notifications are defined by the implementation.

### `content_height`

```cpp
[[nodiscard]] double content_height() const noexcept
```

Reports the current content height value without mutation.

### `preferred_height`

```cpp
[[nodiscard]] double preferred_height(double available_width) const
```

Reports the current preferred height value without mutation.

### `scroll_offset`

```cpp
[[nodiscard]] double scroll_offset() const noexcept
```

Reports the current scroll offset value without mutation.

### `set_scroll_offset`

```cpp
void set_scroll_offset(double offset)
```

Synchronously updates the retained scroll offset property. Validation, typed invalidation, and notifications are defined by the implementation.

### `field_changed`

```cpp
[[nodiscard]] Event<const InstrumentFieldChange&>& field_changed() noexcept
```

Public InstrumentRack operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `field_committed`

```cpp
[[nodiscard]] Event<const InstrumentFieldChange&>& field_committed() noexcept
```

Public InstrumentRack operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `module_toggled`

```cpp
[[nodiscard]] Event<const InstrumentModuleToggle&>& module_toggled() noexcept
```

Public InstrumentRack operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `remove_requested`

```cpp
[[nodiscard]] Event<const InstrumentModuleRequest&>& remove_requested() noexcept
```

Public InstrumentRack operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `move_requested`

```cpp
[[nodiscard]] Event<const InstrumentModuleMoveRequest&>& move_requested() noexcept
```

Public InstrumentRack operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `measure`

```cpp
[[nodiscard]] Size measure(Size available) override
```

Computes desired size from the available constraint without arranging children.

### `arrange`

```cpp
void arrange(Rect final_bounds) override
```

Commits final geometry and arranges retained child roles within it.

### `on_pointer`

```cpp
void on_pointer(PointerEvent& event) override
```

Consumes normalized routed pointer input and updates retained interaction state.

### `on_pointer_bubble`

```cpp
void on_pointer_bubble(PointerEvent& event) override
```

Public InstrumentRack operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_key_preview`

```cpp
void on_key_preview(KeyEvent& event) override
```

Public InstrumentRack operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `semantic_descriptor`

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the current retained state into the framework semantic/accessibility graph.
