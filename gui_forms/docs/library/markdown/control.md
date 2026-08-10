# Control

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Control`  
Declaration: `include/gui_forms/c_api.hpp:48`  
Definition: `src/core/control.cpp, src/core/dispatcher.cpp`

Control is a visual retained control declared in include/gui_forms/c_api.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `Control`

```cpp
Control() = default
```

Constructs or tears down the retained Control object according to its ownership contract.

### `Control`

```cpp
Control(const Api& api, std::string_view stable_id, std::uint32_t kind = GF_CONTROL_GENERIC) : api_(&api)
```

Constructs or tears down the retained Control object according to its ownership contract.

### `Control`

```cpp
Control(const Control& other) : api_(other.api_), handle_(other.handle_)
```

Constructs or tears down the retained Control object according to its ownership contract.

### `operator=`

```cpp
Control& operator=(const Control& other)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `Control`

```cpp
Control(Control&& other) noexcept : api_(std::exchange(other.api_, nullptr)), handle_(std::exchange(other.handle_,
```

Constructs or tears down the retained Control object according to its ownership contract.

### `operator=`

```cpp
Control& operator=(Control&& other) noexcept
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `~Control`

```cpp
~Control()
```

Constructs or tears down the retained Control object according to its ownership contract.

### `swap`

```cpp
void swap(Control& other) noexcept
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `reset`

```cpp
void reset() noexcept
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `get`

```cpp
[[nodiscard]] gf_handle get() const noexcept
```

Reports the current get value without mutation.

### `set_visible`

```cpp
void set_visible(bool visible) const
```

Synchronously updates the retained visible property. Validation, typed invalidation, and notifications are defined by the implementation.

### `visible`

```cpp
[[nodiscard]] bool visible() const
```

Reports the current visible value without mutation.

### `set_enabled`

```cpp
void set_enabled(bool enabled) const
```

Synchronously updates the retained enabled property. Validation, typed invalidation, and notifications are defined by the implementation.

### `enabled`

```cpp
[[nodiscard]] bool enabled() const
```

Reports the current enabled value without mutation.

### `set_name`

```cpp
void set_name(std::string_view name) const
```

Synchronously updates the retained name property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_text`

```cpp
void set_text(std::string_view text) const
```

Synchronously updates the retained text property. Validation, typed invalidation, and notifications are defined by the implementation.

### `name`

```cpp
[[nodiscard]] std::string name() const
```

Reports the current name value without mutation.

### `text`

```cpp
[[nodiscard]] std::string text() const
```

Reports the current text value without mutation.

### `run_window`

```cpp
void run_window(std::uint32_t flags = GF_WINDOW_RUN_DEFAULT) const
```

Reports the current run window value without mutation.

### `last_host_trace`

```cpp
[[nodiscard]] std::string last_host_trace() const
```

Reports the current last host trace value without mutation.

### `add_child`

```cpp
void add_child(const Control& child) const
```

Reports the current add child value without mutation.

### `request_close`

```cpp
void request_close() const
```

Reports the current request close value without mutation.

### `callback_fault_count`

```cpp
[[nodiscard]] std::uint64_t callback_fault_count() const
```

Reports the current callback fault count value without mutation.

### `stable_id`

```cpp
[[nodiscard]] std::string stable_id() const
```

Reports the current stable id value without mutation.

### `dispose`

```cpp
void dispose()
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
