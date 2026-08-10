# CurrencyManager

- Status: **OBSERVED: bundle 010 currency manager split; focused M4 binding tests pass**
- Kind: **class**
- Hierarchy: `BindingManagerBase → CurrencyManager`
- Declaration: `include/gui_forms/binding/currency_manager/currency_manager.hpp:11`
- Definition: `src/core/binding/currency_manager/currency_manager.cpp`

CurrencyManager is the non-owning facade over exactly one BindingSource, keeping navigation, edit, and bulk transfer vocabulary uniform.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `count` (public)

```cpp
[[nodiscard]] std::size_t count() const noexcept override
```

Delegates row count or returns zero after detachment.

### `current` (public)

```cpp
[[nodiscard]] const BindingRecord* current() const noexcept override
```

Delegates current row or returns null after detachment.

### `position` (public)

```cpp
[[nodiscard]] std::ptrdiff_t position() const noexcept override
```

Delegates currency or returns -1 after detachment.

### `binding_suspended` (public)

```cpp
[[nodiscard]] bool binding_suspended() const noexcept override
```

Delegates suspension state.

### `set_position` (public)

```cpp
bool set_position(std::ptrdiff_t position) override
```

Delegates an admitted currency transition.

### `cancel_current_edit` (public)

```cpp
void cancel_current_edit() override
```

Delegates edit rollback.

### `end_current_edit` (public)

```cpp
void end_current_edit() override
```

Delegates edit commit.

### `remove_at` (public)

```cpp
bool remove_at(std::size_t index) override
```

Delegates row removal.

### `suspend_binding` (public)

```cpp
void suspend_binding() override
```

Delegates transfer suspension.

### `resume_binding` (public)

```cpp
void resume_binding() override
```

Delegates transfer resumption.

### `pull_data` (public)

```cpp
bool pull_data() override
```

Requests every active binding to read the source.

### `push_data` (public)

```cpp
bool push_data() override
```

Requests every active binding to write the source.

### `binding_complete` (public)

```cpp
[[nodiscard]] Event<BindingCompleteEvent&>& binding_complete() noexcept override
```

Returns the source completion event.

### `current_changed` (public)

```cpp
[[nodiscard]] Event<>& current_changed() noexcept override
```

Returns the source currency event.

### `current_item_changed` (public)

```cpp
[[nodiscard]] Event<>& current_item_changed() noexcept override
```

Returns the source current-item event.

### `position_changed` (public)

```cpp
[[nodiscard]] Event<std::ptrdiff_t>& position_changed() noexcept override
```

Returns the source position event.

### `data_error` (public)

```cpp
[[nodiscard]] Event<const std::string&>& data_error() noexcept override
```

Returns the source error event.

### `list` (public)

```cpp
[[nodiscard]] std::span<const BindingRecord> list() const noexcept
```

Returns the complete source row view.

### `list_changed` (public)

```cpp
[[nodiscard]] Event<const BindingListChange&>& list_changed() noexcept
```

Returns the source list event.

### `refresh` (public)

```cpp
void refresh()
```

Publishes one non-metadata reset.

### `source` (public)

```cpp
[[nodiscard]] BindingSource& source() const noexcept
```

Returns the bound source reference.

### `CurrencyManager` (private)

```cpp
explicit CurrencyManager(BindingSource& source) : source_(&source)
```

Privately binds one source for the source's complete lifetime.
