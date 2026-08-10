# ContextMenu

- Status: **OBSERVED: bundle 005 split and preferred-width enhancement; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class**
- Hierarchy: `Component → ContextMenu`
- Declaration: `include/gui_forms/components/context_menu/context_menu.hpp:62`
- Definition: `src/controls/menu/context_menu/context_menu.cpp`

ContextMenu is a nonvisual owner for a retained, focus-scoped popup menu tree. Its implementation controls transient lifetime, submenu chains, enabled/check state, root handoff, outside-pointer policy, invocation ordering, and caller-configurable preferred width without exposing implementation rows as reusable public controls.

## Visual evidence

![ContextMenu](../captures/context_menu_menu_strip.png)

## Declared methods

### `ContextMenu` (public)

```cpp
explicit ContextMenu(std::string stable_id)
```

Constructs a menu owner with stable identity and detached authored item state.

### `~ContextMenu` (public)

```cpp
~ContextMenu() override
```

Revokes popup leases, focus scope, subscriptions, and implementation controls before destruction.

### `stable_id` (public)

```cpp
[[nodiscard]] const std::string& stable_id() const noexcept
```

Returns the stable identity prefix used for popup layers, panels, rows, and semantics.

### `items` (public)

```cpp
[[nodiscard]] const std::vector<MenuItemSpec>& items() const noexcept
```

Returns the authored root menu-item tree.

### `set_items` (public)

```cpp
void set_items(std::vector<MenuItemSpec> items)
```

Validates item identities and submenu topology, replaces the model, and rebuilds an open presentation safely.

### `preferred_width` (public)

```cpp
[[nodiscard]] double preferred_width() const noexcept
```

Returns the logical minimum width requested for each menu panel.

### `set_preferred_width` (public)

```cpp
void set_preferred_width(double width)
```

Validates bounded panel width and rebuilds live menu geometry when open.

### `show` (public)

```cpp
void show(const Control::Ptr& owner, Point window_position)
```

Opens the menu relative to an invoker and anchor point under a new popup and focus-scope lease.

### `close` (public)

```cpp
void close() noexcept
```

Closes the complete submenu chain, revokes transient ownership, and publishes a real open-state transition.

### `is_open` (public)

```cpp
[[nodiscard]] bool is_open() const noexcept
```

Reports whether the root popup lease remains connected.

### `item_invoked` (public)

```cpp
[[nodiscard]] Event<const MenuItemInvocation&>& item_invoked() noexcept
```

Returns the event published after an enabled leaf is committed and the transient chain closes.

### `open_changed` (public)

```cpp
[[nodiscard]] Event<bool>& open_changed() noexcept
```

Returns the event published when root transient ownership opens or closes.

### `on_dispose` (protected)

```cpp
void on_dispose() noexcept override
```

Executes ContextMenu's on dispose operation against retained state; the signature records its exact inputs, result, constness, and failure surface.

### `set_root_navigation_handler` (private)

```cpp
void set_root_navigation_handler(std::function<bool(int)> handler)
```

Synchronously updates the retained root navigation handler property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_outside_pointer_handler` (private)

```cpp
void set_outside_pointer_handler( std::function<bool(const PointerEvent&)> handler)
```

Synchronously updates the retained outside pointer handler property. Validation, typed invalidation, and notifications are defined by the implementation.
