# RackModulePanel

- Status: **OBSERVED: bundle 006 source-private responsive-control split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Panel → RackModulePanel`
- Declaration: `src/controls/panel/instrument_rack/rack_module_panel/rack_module_panel.hpp:13`
- Definition: `src/controls/panel/instrument_rack/rack_module_panel/rack_module_panel.cpp`

RackModulePanel is InstrumentRack's source-private retained module shell. It owns responsive horizontal/compact child arrangement and reorder semantics while InstrumentRack retains model, event, focus, scrolling, and reconciliation authority.

## Visual evidence

![RackModulePanel](../captures/instrument_rack.png)

## Declared methods

### `RackModulePanel` (public)

```cpp
explicit RackModulePanel(StableId stable_id)
```

Constructs a bordered module shell with reorder accessibility guidance.

### `set_compact` (public)

```cpp
void set_compact(bool compact)
```

Commits responsive stacked-field mode and invalidates child layout.

### `compact` (public)

```cpp
[[nodiscard]] bool compact() const noexcept
```

Reports whether fields are arranged vertically for a narrow slot.

### `arrange` (public)

```cpp
void arrange(Rect final_bounds) override
```

Arranges enable/remove edges, weighted horizontal fields or compact stacked fields, and status text.

### `semantic_descriptor` (public)

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects a named group with descendant inclusion and increment/decrement reorder actions.

### `on_semantic_action` (public)

```cpp
bool on_semantic_action(SemanticAction action, std::string_view value) override
```

Maps decrement/increment to the rack-owned move callback.
