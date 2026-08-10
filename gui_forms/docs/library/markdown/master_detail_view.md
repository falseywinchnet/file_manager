# MasterDetailView

- Status: **OBSERVED: bundle 001 split; M4 build and focused tests pass**
- Kind: **class / visual retained control**
- Hierarchy: `ContainerControl → MasterDetailView`
- Declaration: `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:42`
- Definition: `src/controls/container/master_detail_view/master_detail_view.cpp`

MasterDetailView is a content-agnostic retained shell over SplitContainer with explicit role ownership, theme-derived or explicit splitter policy, responsive compact presentation, focus-safe collapse, and inspectable presentation events.

## Visual evidence

![MasterDetailView](../captures/master_detail_view.png)

## Declared methods

### `MasterDetailView` (public)

```cpp
explicit MasterDetailView(StableId stable_id)
```

Constructs private SplitContainer infrastructure and configures it from the active structural layout policy; visual attachment remains lazy.

### `initialize_control_tree` (public)

```cpp
void initialize_control_tree()
```

Attaches the owned SplitContainer exactly once.

### `master` (public)

```cpp
[[nodiscard]] Control::Ptr master() const noexcept
```

Returns the control occupying the master role.

### `detail` (public)

```cpp
[[nodiscard]] Control::Ptr detail() const noexcept
```

Returns the control occupying the detail role.

### `set_master` (public)

```cpp
[[nodiscard]] Control::Ptr set_master(Control::Ptr control)
```

Docks an unparented control into the first splitter panel and returns the detached predecessor; infrastructure and cross-role reuse are rejected.

### `set_detail` (public)

```cpp
[[nodiscard]] Control::Ptr set_detail(Control::Ptr control)
```

Docks an unparented control into the second splitter panel and returns the detached predecessor; infrastructure and cross-role reuse are rejected.

### `split_container` (public)

```cpp
[[nodiscard]] std::shared_ptr<SplitContainer> split_container() const noexcept
```

Returns the genuine retained SplitContainer used for rendering, accessibility, pointer dragging, and keyboard splitter behavior.

### `master_detail_layout` (public)

```cpp
[[nodiscard]] const MasterDetailLayout& master_detail_layout() const noexcept
```

Returns the stored explicit layout value; use effective_master_detail_layout for the active policy.

### `uses_theme_layout` (public)

```cpp
[[nodiscard]] bool uses_theme_layout() const noexcept
```

Reports whether current splitter dimensions and breakpoint derive from structural Theme tokens.

### `effective_master_detail_layout` (public)

```cpp
[[nodiscard]] MasterDetailLayout effective_master_detail_layout() const noexcept
```

Resolves inherited structural Theme geometry or the explicit bounded layout.

### `set_master_detail_layout` (public)

```cpp
void set_master_detail_layout(MasterDetailLayout layout)
```

Validates finite ordered extents, makes the explicit layout authoritative, reconfigures the splitter, and invalidates all affected retained phases.

### `reset_master_detail_layout_to_theme` (public)

```cpp
void reset_master_detail_layout_to_theme()
```

Restores inherited structural Theme geometry and reconfigures the genuine splitter.

### `display_mode` (public)

```cpp
[[nodiscard]] MasterDetailDisplayMode display_mode() const noexcept
```

Returns the requested automatic or explicit presentation mode.

### `set_display_mode` (public)

```cpp
void set_display_mode(MasterDetailDisplayMode mode)
```

Selects automatic, side-by-side, master-only, or detail-only presentation; invalid values fail atomically.

### `effective_display_mode` (public)

```cpp
[[nodiscard]] MasterDetailDisplayMode effective_display_mode() const noexcept
```

Returns the presentation currently applied after responsive resolution.

### `compact_detail_visible` (public)

```cpp
[[nodiscard]] bool compact_detail_visible() const noexcept
```

Reports which role automatic compact presentation should expose.

### `set_compact_detail_visible` (public)

```cpp
void set_compact_detail_visible(bool visible)
```

Selects master or detail for the next automatic compact arrangement.

### `show_master` (public)

```cpp
void show_master()
```

Convenience operation selecting the master role in compact automatic mode.

### `show_detail` (public)

```cpp
void show_detail()
```

Convenience operation selecting the detail role in compact automatic mode.

### `presentation_changed` (public)

```cpp
[[nodiscard]] Event<const MasterDetailPresentationChange&>& presentation_changed() noexcept
```

Returns events describing previous/current applied mode and whether accommodation caused the transition.

### `measure` (public)

```cpp
[[nodiscard]] Size measure(Size available) override
```

Measures stable snapshots of both role controls, tolerates mutation during callbacks, and combines desired size according to resolved orientation/mode.

### `arrange` (public)

```cpp
void arrange(Rect final_bounds) override
```

Configures and attaches the splitter, resolves responsive mode from final bounds, applies collapse with an explicit origin, and fills the view.

### `semantic_descriptor` (public)

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects an optional named group whose value reports side-by-side, master, or detail presentation.

### `replace_role` (private)

```cpp
[[nodiscard]] Control::Ptr replace_role(Control::Ptr& slot, const std::shared_ptr<SplitterPanel>& panel, Control::Ptr replacement)
```

Public MasterDetailView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `resolve_display_mode` (private)

```cpp
[[nodiscard]] MasterDetailDisplayMode resolve_display_mode( Size available) const noexcept
```

Reports the current resolve display mode value without mutation.

### `apply_display_mode` (private)

```cpp
void apply_display_mode(MasterDetailDisplayMode mode, bool automatic)
```

Public MasterDetailView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `configure_split` (private)

```cpp
void configure_split()
```

Public MasterDetailView operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.
