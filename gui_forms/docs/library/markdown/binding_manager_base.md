# BindingManagerBase

- Status: **OBSERVED: bundle 010 currency interface split; focused M4 binding tests pass**
- Kind: **class**
- Hierarchy: `BindingManagerBase`
- Declaration: `include/gui_forms/binding/binding_manager_base/binding_manager_base.hpp:12`
- Definition: `inline/header-only`

BindingManagerBase is the renderer-neutral currency/edit/transfer contract shared by list, grid, property, and settings surfaces.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `~BindingManagerBase` (public)

```cpp
virtual ~BindingManagerBase() = default
```

Provides polymorphic destruction.

### `count` (public)

```cpp
[[nodiscard]] virtual std::size_t count() const noexcept = 0
```

Returns source row cardinality.

### `current` (public)

```cpp
[[nodiscard]] virtual const BindingRecord* current() const noexcept = 0
```

Returns the current row or null.

### `position` (public)

```cpp
[[nodiscard]] virtual std::ptrdiff_t position() const noexcept = 0
```

Returns current zero-based currency or -1.

### `binding_suspended` (public)

```cpp
[[nodiscard]] virtual bool binding_suspended() const noexcept = 0
```

Reports whether automatic transfer publication is suspended.

### `set_position` (public)

```cpp
virtual bool set_position(std::ptrdiff_t position) = 0
```

Moves currency if the requested position is admitted.

### `cancel_current_edit` (public)

```cpp
virtual void cancel_current_edit() = 0
```

Restores the current edit snapshot.

### `end_current_edit` (public)

```cpp
virtual void end_current_edit() = 0
```

Commits the current edit snapshot.

### `remove_at` (public)

```cpp
virtual bool remove_at(std::size_t index) = 0
```

Removes an admitted row.

### `suspend_binding` (public)

```cpp
virtual void suspend_binding() = 0
```

Defers automatic transfer publication.

### `resume_binding` (public)

```cpp
virtual void resume_binding() = 0
```

Resumes and coalesces pending source publication.

### `pull_data` (public)

```cpp
virtual bool pull_data() = 0
```

Transfers every active binding from source to control.

### `push_data` (public)

```cpp
virtual bool push_data() = 0
```

Transfers every active binding from control to source.

### `binding_complete` (public)

```cpp
[[nodiscard]] virtual Event<BindingCompleteEvent&>& binding_complete() noexcept = 0
```

Returns aggregate transfer completion observation.

### `current_changed` (public)

```cpp
[[nodiscard]] virtual Event<>& current_changed() noexcept = 0
```

Returns currency-row change observation.

### `current_item_changed` (public)

```cpp
[[nodiscard]] virtual Event<>& current_item_changed() noexcept = 0
```

Returns current-row content observation.

### `position_changed` (public)

```cpp
[[nodiscard]] virtual Event<std::ptrdiff_t>& position_changed() noexcept = 0
```

Returns numeric currency change observation.

### `data_error` (public)

```cpp
[[nodiscard]] virtual Event<const std::string&>& data_error() noexcept = 0
```

Returns portable source validation error observation.
