# Control

- Status: **OBSERVED: bundle 011 C++ ABI wrapper review**
- Kind: **class**
- Hierarchy: `Control`
- Declaration: `include/gui_forms/c_api.hpp:48`
- Definition: `src/core/control/control/control.cpp, src/core/control/dispatcher/control_dispatcher.cpp`

The abi0::Control wrapper gives a generational C handle deterministic RAII retain/release, value-like copying, move transfer, bounded strings, and exception translation without becoming a retained visual object.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `Control` (public)

```cpp
Control() = default
```

Constructs or tears down the retained Control object according to its ownership contract.

### `Control` (public)

```cpp
Control(const Api& api, std::string_view stable_id, std::uint32_t kind = GF_CONTROL_GENERIC) : api_(&api)
```

Constructs or tears down the retained Control object according to its ownership contract.

### `Control` (public)

```cpp
Control(const Control& other) : api_(other.api_), handle_(other.handle_)
```

Constructs or tears down the retained Control object according to its ownership contract.

### `operator=` (public)

```cpp
Control& operator=(const Control& other)
```

Executes Control's operator= operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `Control` (public)

```cpp
Control(Control&& other) noexcept : api_(std::exchange(other.api_, nullptr)), handle_(std::exchange(other.handle_,
```

Constructs or tears down the retained Control object according to its ownership contract.

### `operator=` (public)

```cpp
Control& operator=(Control&& other) noexcept
```

Executes Control's operator= operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `~Control` (public)

```cpp
~Control()
```

Constructs or tears down the retained Control object according to its ownership contract.

### `swap` (public)

```cpp
void swap(Control& other) noexcept
```

Executes Control's swap operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `reset` (public)

```cpp
void reset() noexcept
```

Executes Control's reset operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `get` (public)

```cpp
[[nodiscard]] gf_handle get() const noexcept
```

Reports the current get value without mutation.

### `set_visible` (public)

```cpp
void set_visible(bool visible) const
```

Synchronously updates the retained visible property. Validation, typed invalidation, and notifications are defined by the implementation.

### `visible` (public)

```cpp
[[nodiscard]] bool visible() const
```

Reports the current visible value without mutation.

### `set_enabled` (public)

```cpp
void set_enabled(bool enabled) const
```

Synchronously updates the retained enabled property. Validation, typed invalidation, and notifications are defined by the implementation.

### `enabled` (public)

```cpp
[[nodiscard]] bool enabled() const
```

Reports the current enabled value without mutation.

### `set_name` (public)

```cpp
void set_name(std::string_view name) const
```

Synchronously updates the retained name property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_text` (public)

```cpp
void set_text(std::string_view text) const
```

Synchronously updates the retained text property. Validation, typed invalidation, and notifications are defined by the implementation.

### `name` (public)

```cpp
[[nodiscard]] std::string name() const
```

Reports the current name value without mutation.

### `text` (public)

```cpp
[[nodiscard]] std::string text() const
```

Reports the current text value without mutation.

### `run_window` (public)

```cpp
void run_window(std::uint32_t flags = GF_WINDOW_RUN_DEFAULT) const
```

Reports the current run window value without mutation.

### `last_host_trace` (public)

```cpp
[[nodiscard]] std::string last_host_trace() const
```

Reports the current last host trace value without mutation.

### `add_child` (public)

```cpp
void add_child(const Control& child) const
```

Adds child to Control's retained ownership model after validating identity and lifetime constraints.

### `request_close` (public)

```cpp
void request_close() const
```

Reports the current request close value without mutation.

### `callback_fault_count` (public)

```cpp
[[nodiscard]] std::uint64_t callback_fault_count() const
```

Reports the current callback fault count value without mutation.

### `stable_id` (public)

```cpp
[[nodiscard]] std::string stable_id() const
```

Reports the current stable id value without mutation.

### `dispose` (public)

```cpp
void dispose()
```

Executes Control's dispose operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `set_string` (private)

```cpp
void set_string(SetString operation, std::string_view value) const
```

Synchronously updates the retained string property. Validation, typed invalidation, and notifications are defined by the implementation.

### `get_string` (private)

```cpp
[[nodiscard]] std::string get_string(GetString operation) const
```

Reports the current get string value without mutation.

### `check` (private)

```cpp
void check(gf_result result) const
```

Reports the current check value without mutation.
