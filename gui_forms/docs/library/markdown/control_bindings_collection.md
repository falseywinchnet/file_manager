# ControlBindingsCollection

- Status: **OBSERVED: bundle 010 per-control binding collection split; focused M4 binding tests pass**
- Kind: **class**
- Hierarchy: `ControlBindingsCollection`
- Declaration: `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:17`
- Definition: `src/core/binding/control_bindings_collection/control_bindings_collection.cpp`

ControlBindingsCollection owns the unique active binding for each canonical target property and disposes the set deterministically.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `ControlBindingsCollection` (public)

```cpp
explicit ControlBindingsCollection(Control& target) : target_(&target)
```

Binds a target; copying is prohibited.

### `~ControlBindingsCollection` (public)

```cpp
~ControlBindingsCollection()
```

Clears and disposes all bindings.

### `ControlBindingsCollection` (public)

```cpp
ControlBindingsCollection(const ControlBindingsCollection&) = delete
```

Binds a target; copying is prohibited.

### `operator=` (public)

```cpp
ControlBindingsCollection& operator=(const ControlBindingsCollection&) = delete
```

Copy assignment is prohibited.

### `add` (public)

```cpp
std::shared_ptr<Binding> add( std::string property_name, std::shared_ptr<BindingSource> source, std::string data_member)
```

Creates with default or explicit policy, or validates and starts an existing binding after enforcing target/property uniqueness.

### `add` (public)

```cpp
std::shared_ptr<Binding> add( std::string property_name, std::shared_ptr<BindingSource> source, std::string data_member, BindingOptions options)
```

Creates with default or explicit policy, or validates and starts an existing binding after enforcing target/property uniqueness.

### `add` (public)

```cpp
void add(std::shared_ptr<Binding> binding)
```

Creates with default or explicit policy, or validates and starts an existing binding after enforcing target/property uniqueness.

### `remove` (public)

```cpp
bool remove(const Binding& binding)
```

Detaches and disposes one binding identity.

### `clear` (public)

```cpp
void clear() noexcept
```

Exchanges the owned set and safely disposes every live binding.

### `find` (public)

```cpp
[[nodiscard]] std::shared_ptr<Binding> find( std::string_view property_name) const
```

Canonicalizes a property name and returns its binding.

### `items` (public)

```cpp
[[nodiscard]] std::span<const std::shared_ptr<Binding>> items() const noexcept
```

Returns the ordered shared-owner view.

### `size` (public)

```cpp
[[nodiscard]] std::size_t size() const noexcept
```

Returns binding count.

### `empty` (public)

```cpp
[[nodiscard]] bool empty() const noexcept
```

Reports whether no bindings are owned.

### `default_data_source_update_mode` (public)

```cpp
[[nodiscard]] DataSourceUpdateMode default_data_source_update_mode() const noexcept
```

Returns the policy used by the short add overload.

### `set_default_data_source_update_mode` (public)

```cpp
void set_default_data_source_update_mode( DataSourceUpdateMode mode) noexcept
```

Changes the future short-add default without mutating existing bindings.
