# UserControl

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `ContainerControl → UserControl`  
Declaration: `include/gui_forms/container_controls.hpp:51`  
Definition: `src/controls/container_controls.cpp`

UserControl is a visual retained control declared in include/gui_forms/container_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `UserControl`

```cpp
explicit UserControl(StableId stable_id)
```

Constructs or tears down the retained UserControl object according to its ownership contract.

### `loaded`

```cpp
[[nodiscard]] Event<>& loaded() noexcept
```

Public UserControl operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `is_loaded`

```cpp
[[nodiscard]] bool is_loaded() const noexcept
```

Reports the current is loaded value without mutation.

### `is_attached`

```cpp
[[nodiscard]] bool is_attached() const noexcept
```

Reports the current is attached value without mutation.

### `attachment_count`

```cpp
[[nodiscard]] std::uint64_t attachment_count() const noexcept
```

Reports the current attachment count value without mutation.
