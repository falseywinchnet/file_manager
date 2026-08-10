# Component

- Status: **OBSERVED: bundle 010 component state-machine split; focused M4 lifecycle tests pass**
- Kind: **class**
- Hierarchy: `Component`
- Declaration: `include/gui_forms/component/component/component.hpp:11`
- Definition: `src/core/component/component/component.cpp`

Component owns the alive-to-disposing-to-disposed transition and revokes acquired work in strict reverse order before subclass teardown.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `Component` (public)

```cpp
Component() = default
```

Constructs an alive component; copying is prohibited.

### `~Component` (public)

```cpp
virtual ~Component()
```

Provides polymorphic destruction without implicitly re-entering virtual teardown.

### `Component` (public)

```cpp
Component(const Component&) = delete
```

Constructs an alive component; copying is prohibited.

### `operator=` (public)

```cpp
Component& operator=(const Component&) = delete
```

Copy assignment is prohibited.

### `dispose` (public)

```cpp
void dispose()
```

Verifies thread policy, enters disposing once, revokes owned work, runs subclass teardown, and commits disposed.

### `component_state` (public)

```cpp
[[nodiscard]] ComponentState component_state() const noexcept
```

Returns the exact lifecycle state.

### `is_alive` (public)

```cpp
[[nodiscard]] bool is_alive() const noexcept
```

Reports whether mutation remains admissible.

### `is_disposed` (public)

```cpp
[[nodiscard]] bool is_disposed() const noexcept
```

Reports completed teardown.

### `own_revocable` (public)

```cpp
void own_revocable(const std::weak_ptr<detail::Revocable>& revocable)
```

Adds weak revocation authority while alive or immediately disconnects work acquired after teardown begins.

### `verify_dispose_thread` (protected)

```cpp
virtual void verify_dispose_thread()
```

Allows an owner to enforce disposal affinity before state changes.

### `on_dispose` (protected)

```cpp
virtual void on_dispose() noexcept
```

Provides noexcept subclass teardown after revocation.

### `revoke_owned_work` (protected)

```cpp
void revoke_owned_work() noexcept
```

Exchanges and disconnects the revocation stack in reverse acquisition order.
