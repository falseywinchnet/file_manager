# ComponentContainer

- Status: **OBSERVED: bundle 010 ownership-container split; focused M4 lifecycle tests pass**
- Kind: **class**
- Hierarchy: `ComponentContainer`
- Declaration: `include/gui_forms/component/component_container/component_container.hpp:10`
- Definition: `src/core/component/component_container/component_container.cpp`

ComponentContainer owns a unique ordered set of shared components and disposes the captured ownership sequence exactly once.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `ComponentContainer` (public)

```cpp
ComponentContainer() = default
```

Constructs an empty live container; copying is prohibited.

### `~ComponentContainer` (public)

```cpp
~ComponentContainer()
```

Disposes remaining ownership.

### `ComponentContainer` (public)

```cpp
ComponentContainer(const ComponentContainer&) = delete
```

Constructs an empty live container; copying is prohibited.

### `operator=` (public)

```cpp
ComponentContainer& operator=(const ComponentContainer&) = delete
```

Copy assignment is prohibited.

### `add` (public)

```cpp
void add(Component::Ptr component)
```

Rejects null or post-disposal ownership and ignores an already-owned identity.

### `remove` (public)

```cpp
[[nodiscard]] Component::Ptr remove(const Component& component)
```

Transfers one matching shared owner without disposal.

### `components` (public)

```cpp
[[nodiscard]] std::span<const Component::Ptr> components() const noexcept
```

Returns the current ordered ownership view.

### `contains` (public)

```cpp
[[nodiscard]] bool contains(const Component& component) const noexcept
```

Tests pointer identity.

### `dispose` (public)

```cpp
void dispose()
```

Atomically retires the container and disposes the captured sequence in ownership order.

### `is_disposed` (public)

```cpp
[[nodiscard]] bool is_disposed() const noexcept
```

Reports whether ownership has retired.
