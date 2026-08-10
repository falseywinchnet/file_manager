# CorrespondenceView

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Panel → CorrespondenceView`  
Declaration: `include/gui_forms/collection_controls.hpp:346`  
Definition: `src/controls/collection_controls.cpp`

CorrespondenceView is a visual retained control declared in include/gui_forms/collection_controls.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `CorrespondenceView`

```cpp
explicit CorrespondenceView(StableId stable_id)
```

Constructs or tears down the retained CorrespondenceView object according to its ownership contract.

### `items`

```cpp
[[nodiscard]] std::span<const CorrespondenceItem> items() const noexcept
```

Reports the current items value without mutation.

### `set_items`

```cpp
void set_items(std::vector<CorrespondenceItem> items)
```

Synchronously updates the retained items property. Validation, typed invalidation, and notifications are defined by the implementation.

### `selected_id`

```cpp
[[nodiscard]] std::string_view selected_id() const noexcept
```

Reports the current selected id value without mutation.

### `set_selected_id`

```cpp
void set_selected_id(std::string_view stable_id)
```

Synchronously updates the retained selected id property. Validation, typed invalidation, and notifications are defined by the implementation.

### `focused_id`

```cpp
[[nodiscard]] std::string_view focused_id() const noexcept
```

Reports the current focused id value without mutation.

### `hovered_id`

```cpp
[[nodiscard]] std::string_view hovered_id() const noexcept
```

Reports the current hovered id value without mutation.

### `pinned_id`

```cpp
[[nodiscard]] std::string_view pinned_id() const noexcept
```

Reports the current pinned id value without mutation.

### `set_pinned_id`

```cpp
void set_pinned_id(std::string_view stable_id)
```

Synchronously updates the retained pinned id property. Validation, typed invalidation, and notifications are defined by the implementation.

### `expanded`

```cpp
[[nodiscard]] bool expanded(std::string_view stable_id) const
```

Reports the current expanded value without mutation.

### `compact_height`

```cpp
[[nodiscard]] double compact_height() const noexcept
```

Reports the current compact height value without mutation.

### `set_compact_height`

```cpp
void set_compact_height(double height)
```

Synchronously updates the retained compact height property. Validation, typed invalidation, and notifications are defined by the implementation.

### `expanded_height`

```cpp
[[nodiscard]] double expanded_height() const noexcept
```

Reports the current expanded height value without mutation.

### `set_expanded_height`

```cpp
void set_expanded_height(double height)
```

Synchronously updates the retained expanded height property. Validation, typed invalidation, and notifications are defined by the implementation.

### `hover_intent_delay`

```cpp
[[nodiscard]] std::chrono::milliseconds hover_intent_delay() const noexcept
```

Reports the current hover intent delay value without mutation.

### `set_hover_intent_delay`

```cpp
void set_hover_intent_delay(std::chrono::milliseconds delay)
```

Synchronously updates the retained hover intent delay property. Validation, typed invalidation, and notifications are defined by the implementation.

### `scroll_offset`

```cpp
[[nodiscard]] double scroll_offset() const noexcept
```

Reports the current scroll offset value without mutation.

### `set_scroll_offset`

```cpp
void set_scroll_offset(double offset)
```

Synchronously updates the retained scroll offset property. Validation, typed invalidation, and notifications are defined by the implementation.

### `content_height`

```cpp
[[nodiscard]] double content_height() const noexcept
```

Reports the current content height value without mutation.

### `realized_count`

```cpp
[[nodiscard]] std::size_t realized_count() const noexcept
```

Reports the current realized count value without mutation.

### `item_bounds`

```cpp
[[nodiscard]] std::optional<Rect> item_bounds( std::string_view stable_id) const noexcept
```

Reports the current item bounds value without mutation.

### `font`

```cpp
[[nodiscard]] FontSpec font() const noexcept
```

Reports the current font value without mutation.

### `set_font`

```cpp
void set_font(FontSpec font)
```

Synchronously updates the retained font property. Validation, typed invalidation, and notifications are defined by the implementation.

### `selection_changed`

```cpp
[[nodiscard]] Event<const CorrespondenceSelectionChange&>& selection_changed() noexcept
```

Public CorrespondenceView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `pin_changed`

```cpp
[[nodiscard]] Event<const CorrespondencePinChange&>& pin_changed() noexcept
```

Public CorrespondenceView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `expansion_changed`

```cpp
[[nodiscard]] Event<const CorrespondenceExpansionChange&>& expansion_changed() noexcept
```

Public CorrespondenceView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `item_activated`

```cpp
[[nodiscard]] Event<const std::string&>& item_activated() noexcept
```

Public CorrespondenceView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `context_requested`

```cpp
[[nodiscard]] Event<const ObjectContextRequest&>& context_requested() noexcept
```

Public CorrespondenceView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `arrange`

```cpp
void arrange(Rect final_bounds) override
```

Commits final geometry and arranges retained child roles within it.

### `on_paint`

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records renderer-neutral paint operations for the damaged local region.

### `on_pointer`

```cpp
void on_pointer(PointerEvent& event) override
```

Consumes normalized routed pointer input and updates retained interaction state.

### `on_key`

```cpp
void on_key(KeyEvent& event) override
```

Consumes normalized keyboard input for this control's interaction contract.

### `on_focus_changed`

```cpp
void on_focus_changed(bool focused) override
```

Updates focus-dependent retained state and invalidates affected presentation/semantics.

### `semantic_descriptor`

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the current retained state into the framework semantic/accessibility graph.

### `semantic_virtual_children`

```cpp
[[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override
```

Reports the current semantic virtual children value without mutation.

### `on_semantic_child_action`

```cpp
bool on_semantic_child_action(std::string_view stable_id, SemanticAction action, std::string_view value) override
```

Public CorrespondenceView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
