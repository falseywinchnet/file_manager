# BindingContext

- Status: **OBSERVED: bundle 010 Window currency-context split; focused M4 binding tests pass**
- Kind: **class**
- Hierarchy: `Component → BindingContext`
- Declaration: `include/gui_forms/binding/binding_context/binding_context.hpp:18`
- Definition: `src/core/binding/binding_context/binding_context.cpp`

BindingContext owns weak Window-scoped membership and disposal subscriptions for BindingSources while each source retains its own currency manager.

## Visual evidence

Not applicable: this is a nonvisual contract, value, service, or state owner.

## Declared methods

### `BindingContext` (public)

```cpp
explicit BindingContext(Window& window)
```

Binds Window lifetime and verifies construction affinity.

### `~BindingContext` (public)

```cpp
~BindingContext() override
```

Disposes and contains destructor exceptions.

### `add` (public)

```cpp
void add(const std::shared_ptr<BindingSource>& source)
```

Ensures a source has a context entry by resolving its manager.

### `manager` (public)

```cpp
CurrencyManager& manager(const std::shared_ptr<BindingSource>& source)
```

Validates shared owner, liveness, same Window, and uniqueness; installs disposal removal and returns the source manager.

### `contains` (public)

```cpp
[[nodiscard]] bool contains(const BindingSource& source) const noexcept
```

Tests source pointer membership.

### `remove` (public)

```cpp
bool remove(const BindingSource& source)
```

Verifies affinity and removes one source with publication.

### `clear` (public)

```cpp
void clear()
```

Verifies affinity, retires all entries, and publishes removal for each still-live source.

### `size` (public)

```cpp
[[nodiscard]] std::size_t size() const noexcept
```

Returns membership count.

### `collection_changed` (public)

```cpp
[[nodiscard]] Event<const BindingContextChange&>& collection_changed() noexcept
```

Returns ordered add/remove observation.

### `verify_dispose_thread` (protected)

```cpp
void verify_dispose_thread() override
```

Enforces Window affinity while available.

### `on_dispose` (protected)

```cpp
void on_dispose() noexcept override
```

Drops entries, observations, and Window lifetime without publishing into teardown.

### `bound_window` (private)

```cpp
[[nodiscard]] Window* bound_window() const noexcept
```

Resolves weak Window lifetime without extending ownership.

### `remove_entry` (private)

```cpp
bool remove_entry(BindingSource* source, bool publish)
```

Disconnects and erases one exact source entry and optionally publishes removal.
