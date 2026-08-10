# AnchoredPopupLayer

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Panel → AnchoredPopupLayer`  
Declaration: `include/gui_forms/popup_controls.hpp:57`  
Definition: `src/controls/popup_controls.cpp`

AnchoredPopupLayer is a visual retained control declared in include/gui_forms/popup_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `AnchoredPopupLayer`

```cpp
AnchoredPopupLayer(StableId stable_id, Control::Ptr anchor, AnchoredPopupPlacement placement =
```

Constructs or tears down the retained AnchoredPopupLayer object according to its ownership contract.

### `anchor`

```cpp
[[nodiscard]] Control::Ptr anchor() const noexcept
```

Reports the current anchor value without mutation.

### `set_anchor`

```cpp
void set_anchor(Control::Ptr anchor)
```

Synchronously updates the retained anchor property. Validation, typed invalidation, and notifications are defined by the implementation.

### `content`

```cpp
[[nodiscard]] Control::Ptr content() const noexcept
```

Reports the current content value without mutation.

### `set_content`

```cpp
void set_content(Control::Ptr content)
```

Synchronously updates the retained content property. Validation, typed invalidation, and notifications are defined by the implementation.

### `placement`

```cpp
[[nodiscard]] AnchoredPopupPlacement placement() const noexcept
```

Reports the current placement value without mutation.

### `set_placement`

```cpp
void set_placement(AnchoredPopupPlacement placement)
```

Synchronously updates the retained placement property. Validation, typed invalidation, and notifications are defined by the implementation.

### `resolved_placement`

```cpp
[[nodiscard]] AnchoredPopupPlacementResult resolved_placement() const noexcept
```

Reports the current resolved placement value without mutation.

### `dismiss_on_click_away`

```cpp
[[nodiscard]] bool dismiss_on_click_away() const noexcept
```

Reports the current dismiss on click away value without mutation.

### `set_dismiss_on_click_away`

```cpp
void set_dismiss_on_click_away(bool enabled)
```

Synchronously updates the retained dismiss on click away property. Validation, typed invalidation, and notifications are defined by the implementation.

### `dismiss_on_escape`

```cpp
[[nodiscard]] bool dismiss_on_escape() const noexcept
```

Reports the current dismiss on escape value without mutation.

### `set_dismiss_on_escape`

```cpp
void set_dismiss_on_escape(bool enabled)
```

Synchronously updates the retained dismiss on escape property. Validation, typed invalidation, and notifications are defined by the implementation.

### `dismiss_requested`

```cpp
[[nodiscard]] Event<PopupDismissReason>& dismiss_requested() noexcept
```

Public AnchoredPopupLayer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `measure`

```cpp
[[nodiscard]] Size measure(Size available) override
```

Computes desired size from the available constraint without arranging children.

### `arrange`

```cpp
void arrange(Rect final_bounds) override
```

Commits final geometry and arranges retained child roles within it.

### `on_pointer`

```cpp
void on_pointer(PointerEvent& event) override
```

Consumes normalized routed pointer input and updates retained interaction state.

### `on_key_preview`

```cpp
void on_key_preview(KeyEvent& event) override
```

Public AnchoredPopupLayer operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `semantic_descriptor`

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the current retained state into the framework semantic/accessibility graph.
