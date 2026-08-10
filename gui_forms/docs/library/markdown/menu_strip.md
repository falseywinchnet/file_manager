# MenuStrip

- Status: **OBSERVED: bundle 005 split and item-padding enhancement; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Control → MenuStrip`
- Declaration: `include/gui_forms/controls/menu_strip/menu_strip.hpp:39`
- Definition: `src/controls/menu/menu_strip/menu_strip.cpp`

MenuStrip is the retained top-level menu coordinator. It lays out root items, tracks hot and active identities, transfers navigation across an owned ContextMenu, supports mnemonic and semantic entry, and exposes caller-controlled item padding while keeping invocation and open-state events ordered.

## Visual evidence

![MenuStrip](../captures/context_menu_menu_strip.png)

## Declared methods

### `MenuStrip` (public)

```cpp
explicit MenuStrip(StableId stable_id)
```

Constructs a focusable menu bar and its lifetime-bound ContextMenu coordinator.

### `~MenuStrip` (public)

```cpp
~MenuStrip() override
```

Closes transient state and disconnects owned menu subscriptions before destruction.

### `items` (public)

```cpp
[[nodiscard]] const std::vector<MenuStripItemSpec>& items() const noexcept
```

Returns the authored top-level menu records and submenu trees.

### `set_items` (public)

```cpp
void set_items(std::vector<MenuStripItemSpec> items)
```

Validates and replaces menu topology, closes stale transient state, and refreshes geometry and semantics.

### `use_mnemonic` (public)

```cpp
[[nodiscard]] bool use_mnemonic() const noexcept
```

Reports whether ampersand-style access keys participate in activation and painting.

### `set_use_mnemonic` (public)

```cpp
void set_use_mnemonic(bool value)
```

Toggles mnemonic parsing, remeasures labels, and refreshes keyboard routing.

### `item_padding` (public)

```cpp
[[nodiscard]] double item_padding() const noexcept
```

Returns the logical horizontal inset applied around each top-level label.

### `set_item_padding` (public)

```cpp
void set_item_padding(double padding)
```

Validates bounded padding and invalidates menu measurement, paint, and hit testing.

### `active_index` (public)

```cpp
[[nodiscard]] std::optional<std::size_t> active_index() const noexcept
```

Returns the currently open top-level item index, if any.

### `open` (public)

```cpp
bool open(std::size_t index)
```

Validates and opens an enabled top-level item's submenu with root-navigation handoff.

### `close` (public)

```cpp
void close() noexcept
```

Closes the owned ContextMenu and clears active and hot root state.

### `is_open` (public)

```cpp
[[nodiscard]] bool is_open() const noexcept
```

Reports whether the owned menu popup currently has transient ownership.

### `item_invoked` (public)

```cpp
[[nodiscard]] Event<const MenuStripInvocation&>& item_invoked() noexcept
```

Returns the event forwarding a committed leaf invocation from the owned context menu.

### `open_changed` (public)

```cpp
[[nodiscard]] Event<std::optional<std::size_t>>& open_changed() noexcept
```

Returns the event forwarding real changes to the menu strip's open state.

### `measure` (public)

```cpp
[[nodiscard]] Size measure(Size available) override
```

Computes desired menu-bar width from labels, mnemonics, font metrics, and item padding.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records menu-bar background, hot/active states, labels, access-key cues, disabled state, and focus.

### `on_pointer` (public)

```cpp
void on_pointer(PointerEvent& event) override
```

Handles hover transfer, root switching, outside routing, press qualification, and dismissal.

### `on_key` (public)

```cpp
void on_key(KeyEvent& event) override
```

Implements root traversal, open/close, submenu handoff, activation, and Escape behavior.

### `on_focus_changed` (public)

```cpp
void on_focus_changed(bool focused) override
```

Clears transient hot state when focus leaves unless the owned popup retains the interaction scope.

### `semantic_descriptor` (public)

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects the control as a menu bar with expanded state.

### `semantic_virtual_children` (public)

```cpp
[[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override
```

Exposes stable root menu items with enabled, focused, and expanded metadata.

### `on_semantic_child_action` (public)

```cpp
bool on_semantic_child_action(std::string_view stable_id, SemanticAction action, std::string_view value) override
```

Maps virtual focus, expand, collapse, and press to the normal menu coordinator.

### `on_dispose` (protected)

```cpp
void on_dispose() noexcept override
```

Public MenuStrip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `mnemonic_matches` (protected)

```cpp
[[nodiscard]] bool mnemonic_matches( char32_t character) const noexcept override
```

Reports the current mnemonic matches value without mutation.

### `process_mnemonic_self` (protected)

```cpp
bool process_mnemonic_self(char32_t character) override
```

Public MenuStrip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `item_bounds` (private)

```cpp
[[nodiscard]] std::vector<Rect> item_bounds() const
```

Reports the current item bounds value without mutation.

### `index_at` (private)

```cpp
[[nodiscard]] std::optional<std::size_t> index_at(Point window_point) const
```

Reports the current index at value without mutation.

### `next_enabled` (private)

```cpp
[[nodiscard]] std::optional<std::size_t> next_enabled( std::size_t start, int direction) const
```

Reports the current next enabled value without mutation.

### `navigate_root` (private)

```cpp
bool navigate_root(int direction)
```

Public MenuStrip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `handle_popup_pointer` (private)

```cpp
bool handle_popup_pointer(const PointerEvent& event)
```

Public MenuStrip operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_hot` (private)

```cpp
void set_hot(std::optional<std::size_t> index)
```

Synchronously updates the retained hot property. Validation, typed invalidation, and notifications are defined by the implementation.
