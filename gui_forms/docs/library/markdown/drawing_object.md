# DrawingObject

Status: **generated inventory; detailed review pending**  
Kind: **class**  
Hierarchy: `DrawingObject`  
Declaration: `include/gui_forms/drawing.hpp:265`  
Definition: `src/core/drawing.cpp`

DrawingObject is a class declared in include/gui_forms/drawing.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `DrawingObject`

```cpp
DrawingObject()
```

Constructs or tears down the retained DrawingObject object according to its ownership contract.

### `~DrawingObject`

```cpp
virtual ~DrawingObject()
```

Constructs or tears down the retained DrawingObject object according to its ownership contract.

### `DrawingObject`

```cpp
DrawingObject(const DrawingObject&) = delete
```

Constructs or tears down the retained DrawingObject object according to its ownership contract.

### `operator=`

```cpp
DrawingObject& operator=(const DrawingObject&) = delete
```

Public DrawingObject operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `dispose`

```cpp
void dispose()
```

Public DrawingObject operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `state`

```cpp
[[nodiscard]] ObjectState state() const
```

Reports the current state value without mutation.

### `is_disposed`

```cpp
[[nodiscard]] bool is_disposed() const
```

Reports the current is disposed value without mutation.

### `owner_thread`

```cpp
[[nodiscard]] std::thread::id owner_thread() const noexcept
```

Reports the current owner thread value without mutation.

### `verify_access`

```cpp
void verify_access() const
```

Reports the current verify access value without mutation.

### `handoff_to_current_thread`

```cpp
void handoff_to_current_thread()
```

Public DrawingObject operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
