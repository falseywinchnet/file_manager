# BindingSource

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `Component → enable_shared_from_this → BindingSource`  
Declaration: `include/gui_forms/binding.hpp:185`  
Definition: `src/core/binding.cpp`

BindingSource is a class declared in include/gui_forms/binding.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `BindingSource`

```cpp
explicit BindingSource(Window& window)
```

Constructs or tears down the retained BindingSource object according to its ownership contract.

### `~BindingSource`

```cpp
~BindingSource() override
```

Constructs or tears down the retained BindingSource object according to its ownership contract.

### `set_records`

```cpp
void set_records(std::vector<BindingRecord> records, bool metadata_changed = false)
```

Synchronously updates the retained records property. Validation, typed invalidation, and notifications are defined by the implementation.

### `records`

```cpp
[[nodiscard]] std::span<const BindingRecord> records() const noexcept
```

Reports the current records value without mutation.

### `count`

```cpp
[[nodiscard]] std::size_t count() const noexcept
```

Reports the current count value without mutation.

### `position`

```cpp
[[nodiscard]] std::ptrdiff_t position() const noexcept
```

Reports the current position value without mutation.

### `set_position`

```cpp
bool set_position(std::ptrdiff_t position)
```

Synchronously updates the retained position property. Validation, typed invalidation, and notifications are defined by the implementation.

### `move_first`

```cpp
bool move_first()
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `move_last`

```cpp
bool move_last()
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `move_next`

```cpp
bool move_next()
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `move_previous`

```cpp
bool move_previous()
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `current`

```cpp
[[nodiscard]] const BindingRecord* current() const noexcept
```

Reports the current current value without mutation.

### `current_field`

```cpp
[[nodiscard]] std::optional<BindingValue> current_field( std::string_view field) const
```

Reports the current current field value without mutation.

### `current_error`

```cpp
[[nodiscard]] std::string current_error(std::string_view field) const
```

Reports the current current error value without mutation.

### `bindings`

```cpp
[[nodiscard]] std::vector<std::shared_ptr<Binding>> bindings() const
```

Reports the current bindings value without mutation.

### `set_current_field`

```cpp
bool set_current_field(std::string_view field, BindingValue value)
```

Synchronously updates the retained current field property. Validation, typed invalidation, and notifications are defined by the implementation.

### `add`

```cpp
std::size_t add(BindingRecord record)
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `insert`

```cpp
std::size_t insert(std::size_t index, BindingRecord record)
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `remove_at`

```cpp
bool remove_at(std::size_t index)
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `remove_current`

```cpp
bool remove_current()
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `clear`

```cpp
void clear()
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `find`

```cpp
[[nodiscard]] std::optional<std::size_t> find( std::string_view field, const BindingValue& value) const
```

Reports the current find value without mutation.

### `allow_edit`

```cpp
[[nodiscard]] bool allow_edit() const noexcept
```

Reports the current allow edit value without mutation.

### `set_allow_edit`

```cpp
void set_allow_edit(bool allow)
```

Synchronously updates the retained allow edit property. Validation, typed invalidation, and notifications are defined by the implementation.

### `allow_new`

```cpp
[[nodiscard]] bool allow_new() const noexcept
```

Reports the current allow new value without mutation.

### `set_allow_new`

```cpp
void set_allow_new(bool allow)
```

Synchronously updates the retained allow new property. Validation, typed invalidation, and notifications are defined by the implementation.

### `allow_remove`

```cpp
[[nodiscard]] bool allow_remove() const noexcept
```

Reports the current allow remove value without mutation.

### `set_allow_remove`

```cpp
void set_allow_remove(bool allow)
```

Synchronously updates the retained allow remove property. Validation, typed invalidation, and notifications are defined by the implementation.

### `begin_edit`

```cpp
bool begin_edit()
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `cancel_edit`

```cpp
void cancel_edit()
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `end_edit`

```cpp
void end_edit()
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `binding_suspended`

```cpp
[[nodiscard]] bool binding_suspended() const noexcept
```

Reports the current binding suspended value without mutation.

### `suspend_binding`

```cpp
void suspend_binding()
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `resume_binding`

```cpp
void resume_binding()
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `raise_list_changed_events`

```cpp
[[nodiscard]] bool raise_list_changed_events() const noexcept
```

Reports the current raise list changed events value without mutation.

### `set_raise_list_changed_events`

```cpp
void set_raise_list_changed_events(bool raise) noexcept
```

Synchronously updates the retained raise list changed events property. Validation, typed invalidation, and notifications are defined by the implementation.

### `reset_bindings`

```cpp
void reset_bindings(bool metadata_changed = false)
```

Returns bindings to its inherited or default policy.

### `reset_current_item`

```cpp
void reset_current_item()
```

Returns current item to its inherited or default policy.

### `reset_item`

```cpp
bool reset_item(std::size_t index)
```

Returns item to its inherited or default policy.

### `data_member`

```cpp
[[nodiscard]] const std::string& data_member() const noexcept
```

Reports the current data member value without mutation.

### `set_data_member`

```cpp
void set_data_member(std::string member)
```

Synchronously updates the retained data member property. Validation, typed invalidation, and notifications are defined by the implementation.

### `currency_manager`

```cpp
[[nodiscard]] CurrencyManager& currency_manager() noexcept
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `currency_manager`

```cpp
[[nodiscard]] const CurrencyManager& currency_manager() const noexcept
```

Reports the current currency manager value without mutation.

### `snapshot`

```cpp
[[nodiscard]] BindingSourceSnapshot snapshot() const
```

Reports the current snapshot value without mutation.

### `list_changed`

```cpp
[[nodiscard]] Event<const BindingListChange&>& list_changed() noexcept
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `current_changed`

```cpp
[[nodiscard]] Event<>& current_changed() noexcept
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `current_item_changed`

```cpp
[[nodiscard]] Event<>& current_item_changed() noexcept
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `position_changed`

```cpp
[[nodiscard]] Event<std::ptrdiff_t>& position_changed() noexcept
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `data_error`

```cpp
[[nodiscard]] Event<const std::string&>& data_error() noexcept
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `data_source_changed`

```cpp
[[nodiscard]] Event<>& data_source_changed() noexcept
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `data_member_changed`

```cpp
[[nodiscard]] Event<const std::string&>& data_member_changed() noexcept
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `binding_complete`

```cpp
[[nodiscard]] Event<BindingCompleteEvent&>& binding_complete() noexcept
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `disposed_event`

```cpp
[[nodiscard]] Event<>& disposed_event() noexcept
```

Public BindingSource operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
