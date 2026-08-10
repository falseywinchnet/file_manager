# TabPage

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Panel → TabPage`  
Declaration: `include/gui_forms/container_controls.hpp:291`  
Definition: `src/controls/container_controls.cpp`

TabPage is a visual retained control declared in include/gui_forms/container_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `TabPage`

```cpp
explicit TabPage(StableId stable_id, std::string text =
```

Constructs or tears down the retained TabPage object according to its ownership contract.

### `text`

```cpp
[[nodiscard]] const std::string& text() const noexcept
```

Reports the current text value without mutation.

### `set_text`

```cpp
void set_text(std::string text)
```

Synchronously updates the retained text property. Validation, typed invalidation, and notifications are defined by the implementation.

### `semantic_descriptor`

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the current retained state into the framework semantic/accessibility graph.
