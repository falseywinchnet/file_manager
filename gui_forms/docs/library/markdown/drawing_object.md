# DrawingObject

- Status: **OBSERVED: bundle 009 drawing lifetime split; focused M4 tests pass**
- Kind: **class**
- Hierarchy: `DrawingObject`
- Declaration: `include/gui_forms/drawing/object/drawing_object.hpp:10`
- Definition: `src/core/drawing/object/drawing_object.cpp`

DrawingObject is the thread-affine disposable base for mutable drawing resources, with explicit alive/disposing/disposed transitions and controlled serialized handoff.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Declared methods

### `DrawingObject` (public)

```cpp
DrawingObject()
```

Captures the constructing thread as the initial exclusive owner; copying is prohibited.

### `~DrawingObject` (public)

```cpp
virtual ~DrawingObject()
```

Provides polymorphic destruction without implicitly invoking application-visible disposal work.

### `DrawingObject` (public)

```cpp
DrawingObject(const DrawingObject&) = delete
```

Captures the constructing thread as the initial exclusive owner; copying is prohibited.

### `operator=` (public)

```cpp
DrawingObject& operator=(const DrawingObject&) = delete
```

Prohibits copying of stateful drawing identity.

### `dispose` (public)

```cpp
void dispose()
```

Verifies thread access, performs the single alive-to-disposing-to-disposed transition, and calls the subtype cleanup hook once.

### `state` (public)

```cpp
[[nodiscard]] ObjectState state() const
```

Verifies access and returns the authoritative lifecycle phase.

### `is_disposed` (public)

```cpp
[[nodiscard]] bool is_disposed() const
```

Verifies access and reports terminal disposal.

### `owner_thread` (public)

```cpp
[[nodiscard]] std::thread::id owner_thread() const noexcept
```

Returns the current exclusive owner thread identity.

### `verify_access` (public)

```cpp
void verify_access() const
```

Throws when mutable drawing state is touched from a nonowner thread.

### `handoff_to_current_thread` (public)

```cpp
void handoff_to_current_thread()
```

Transfers serialized ownership to the caller's thread; it does not authorize concurrent access.

### `require_alive` (protected)

```cpp
void require_alive() const
```

Requires owner-thread access and the alive phase.

### `on_dispose` (protected)

```cpp
virtual void on_dispose() noexcept
```

Provides the no-throw subtype cleanup hook.
