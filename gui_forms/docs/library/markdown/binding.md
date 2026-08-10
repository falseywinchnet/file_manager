# Binding

- Status: **OBSERVED: bundle 010 directional transfer state-machine split; focused M4 binding tests pass**
- Kind: **class**
- Hierarchy: `Component → enable_shared_from_this → Binding`
- Declaration: `include/gui_forms/binding/binding/binding.hpp:16`
- Definition: `src/core/binding/binding/binding.cpp`

Binding connects one explicit Control property to one BindingSource field with validated conversion, update timing, null projection, completion policy, error routing, reentrancy suppression, and teardown.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `Binding` (public)

```cpp
Binding(Control& target, std::string property_name, std::shared_ptr<BindingSource> source, std::string data_member, BindingOptions options =
```

Canonicalizes endpoint names, validates update/format options, and requires live source, target, and declared bindable property.

### `~Binding` (public)

```cpp
~Binding() override
```

Disposes and contains destructor exceptions.

### `target` (public)

```cpp
[[nodiscard]] Control* target() const noexcept
```

Returns the non-owning target pointer while active.

### `source` (public)

```cpp
[[nodiscard]] std::shared_ptr<BindingSource> source() const noexcept
```

Locks and returns the source owner while available.

### `property_name` (public)

```cpp
[[nodiscard]] const std::string& property_name() const noexcept
```

Returns canonical target property identity.

### `data_member` (public)

```cpp
[[nodiscard]] const std::string& data_member() const noexcept
```

Returns canonical source field identity.

### `options` (public)

```cpp
[[nodiscard]] const BindingOptions& options() const noexcept
```

Returns current transfer policy.

### `set_options` (public)

```cpp
void set_options(BindingOptions options)
```

Validates and commits changed policy, then refreshes the control when active.

### `active` (public)

```cpp
[[nodiscard]] bool active() const noexcept
```

Reports whether subscriptions and endpoints are connected.

### `read_value` (public)

```cpp
bool read_value()
```

Explicitly transfers source to control.

### `write_value` (public)

```cpp
bool write_value()
```

Explicitly transfers control to source.

### `validate` (public)

```cpp
bool validate()
```

Runs the configured validation-time source write or accepts when policy does not require it.

### `snapshot` (public)

```cpp
[[nodiscard]] BindingSnapshot snapshot() const
```

Copies endpoint and directional transfer/reentrancy telemetry.

### `format` (public)

```cpp
[[nodiscard]] Event<BindingConvertEvent&>& format() noexcept
```

Returns source-to-control conversion interception.

### `parse` (public)

```cpp
[[nodiscard]] Event<BindingConvertEvent&>& parse() noexcept
```

Returns control-to-source conversion interception.

### `binding_complete` (public)

```cpp
[[nodiscard]] Event<BindingCompleteEvent&>& binding_complete() noexcept
```

Returns per-binding completion observation.

### `verify_dispose_thread` (protected)

```cpp
void verify_dispose_thread() override
```

Enforces target Window affinity before teardown.

### `on_dispose` (protected)

```cpp
void on_dispose() noexcept override
```

Unregisters, disconnects every subscription/event, and retires endpoint references.

### `start` (private)

```cpp
void start()
```

Registers with source and connects source, target-change, validation, and disposal observations before the initial read.

### `update_control` (private)

```cpp
bool update_control(bool automatic)
```

Guards reentrancy, fetches current source/null/error state, applies formatting or typed conversion, writes the declared property, and completes atomically.

### `update_source` (private)

```cpp
bool update_source(bool automatic)
```

Guards reentrancy, reads target, applies parsing/null projection or typed conversion, writes the current field, and completes atomically.

### `complete` (private)

```cpp
bool complete(BindingCompleteContext context, BindingCompleteState state, std::string error =
```

Emits per-binding then source completion, routes error text, and returns cancellation-aware success.

### `source_changed` (private)

```cpp
void source_changed(const BindingListChange& change)
```

Automatically reads on source change when policy admits.

### `target_changed` (private)

```cpp
void target_changed()
```

Automatically writes on property change when policy admits.
