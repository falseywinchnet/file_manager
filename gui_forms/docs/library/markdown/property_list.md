# PropertyList

- Status: **OBSERVED: bundle 006 control/Impl source split; M4 build, focused tests, and Screen Sharing pass**
- Kind: **class / visual retained control**
- Hierarchy: `Panel → PropertyList`
- Declaration: `include/gui_forms/controls/panel/property_list/property_list.hpp:13`
- Definition: `src/controls/panel/property_list/property_list.cpp`

PropertyList is a lightweight retained settings/property surface over caller-authored stable groups and rows. It owns stock or replacement child editors, reset buttons, optional header content, one vertical scroll plane, validation/disclosure layout, focus reveal, typed events, renderer-neutral painting, and virtual semantics while consumers retain domain meaning.

## Visual evidence

![PropertyList](../captures/property_grid.png)

## Declared methods

### `PropertyList` (public)

```cpp
explicit PropertyList(StableId stable_id)
```

Constructs the grouped row coordinator and source-private retained state.

### `~PropertyList` (public)

```cpp
~PropertyList() override
```

Releases row/editor subscriptions and owned implementation state after disposal.

### `groups` (public)

```cpp
[[nodiscard]] const std::vector<PropertyGroupSpec>& groups() const noexcept
```

Returns the normalized authored group and row model.

### `set_groups` (public)

```cpp
void set_groups(std::vector<PropertyGroupSpec> groups)
```

Validates stable UTF-8 identities/topology, rebuilds real editors and reset controls, and reconciles focus, scrolling, and semantics.

### `set_value` (public)

```cpp
bool set_value(std::string_view row_id, std::string value)
```

Commits one row's text, synchronizes its editor without user publication, and emits a noncommitted model change.

### `set_description` (public)

```cpp
bool set_description(std::string_view row_id, std::string description)
```

Commits row description and refreshes editor/virtual semantic metadata.

### `set_validation` (public)

```cpp
bool set_validation(std::string_view row_id, std::string message)
```

Commits validation text, updates visual status/description, and recomputes variable row height.

### `set_reset_enabled` (public)

```cpp
bool set_reset_enabled(std::string_view row_id, bool enabled)
```

Commits per-row reset availability and synchronizes the real reset button.

### `set_group_expanded` (public)

```cpp
bool set_group_expanded(std::string_view group_id, bool expanded)
```

Commits group disclosure, reflows the single scroll plane, and publishes a real transition.

### `set_row_expanded` (public)

```cpp
bool set_row_expanded(std::string_view row_id, bool expanded)
```

Commits a hierarchical row disclosure, updates descendant visibility/layout, and publishes a real transition.

### `row_expanded` (public)

```cpp
[[nodiscard]] std::optional<bool> row_expanded( std::string_view row_id) const
```

Returns retained disclosure state for an existing row.

### `value` (public)

```cpp
[[nodiscard]] std::optional<std::string> value( std::string_view row_id) const
```

Returns retained text for an existing row.

### `editor` (public)

```cpp
[[nodiscard]] Control::Ptr editor(std::string_view row_id) const
```

Returns the ordinary retained editor owned by a stable row.

### `replace_editor` (public)

```cpp
bool replace_editor(std::string_view row_id, Control::Ptr editor)
```

Validates an unparented live control, assumes ownership, preserves row layout/focus/semantics, and disposes the replaced editor.

### `reset_button` (public)

```cpp
[[nodiscard]] std::shared_ptr<Button> reset_button( std::string_view row_id) const
```

Returns the real retained reset control for a resettable row.

### `set_header_content` (public)

```cpp
void set_header_content(Control::Ptr content, double height)
```

Validates and owns optional preview/summary content in the same scroll plane with bounded height.

### `header_content` (public)

```cpp
[[nodiscard]] Control::Ptr header_content() const noexcept
```

Returns the optional caller-owned header subtree.

### `header_height` (public)

```cpp
[[nodiscard]] double header_height() const noexcept
```

Returns retained logical header extent.

### `set_header_height` (public)

```cpp
void set_header_height(double height)
```

Validates nonnegative bounded height and reflows content.

### `label_width` (public)

```cpp
[[nodiscard]] double label_width() const noexcept
```

Returns the logical name-column width.

### `set_label_width` (public)

```cpp
void set_label_width(double width)
```

Validates bounded label width and refreshes editor/value geometry.

### `scroll_offset` (public)

```cpp
[[nodiscard]] double scroll_offset() const noexcept
```

Returns the retained vertical scroll origin.

### `set_scroll_offset` (public)

```cpp
void set_scroll_offset(double offset)
```

Clamps offset to content/viewport bounds, re-arranges children, and refreshes paint/hit testing/semantics.

### `content_height` (public)

```cpp
[[nodiscard]] double content_height() const noexcept
```

Returns total logical header, group, row, validation, and spacing height.

### `value_changed` (public)

```cpp
[[nodiscard]] Event<const PropertyValueChange&>& value_changed() noexcept
```

Returns the event for live row value transitions.

### `value_committed` (public)

```cpp
[[nodiscard]] Event<const PropertyValueChange&>& value_committed() noexcept
```

Returns the event for qualified editor commits.

### `group_changed` (public)

```cpp
[[nodiscard]] Event<const PropertyGroupChange&>& group_changed() noexcept
```

Returns the event for committed group disclosure transitions.

### `row_expansion_changed` (public)

```cpp
[[nodiscard]] Event<const PropertyRowExpansionChange&>& row_expansion_changed() noexcept
```

Returns the event for committed row disclosure transitions.

### `reset_requested` (public)

```cpp
[[nodiscard]] Event<const PropertyResetRequest&>& reset_requested() noexcept
```

Returns the event requesting consumer-authorized reset of a stable row.

### `measure` (public)

```cpp
[[nodiscard]] Size measure(Size available) override
```

Computes desired viewport extent from available size and content height.

### `arrange` (public)

```cpp
void arrange(Rect final_bounds) override
```

Commits viewport bounds, clamps scrolling, and arranges only current group/row/header child roles.

### `on_paint` (public)

```cpp
void on_paint(Painter& painter, Rect local_damage) override
```

Records group bars, row labels/value backplanes, hierarchy guides, disclosures, validation, required/reset cues, hover, and focus.

### `on_pointer` (public)

```cpp
void on_pointer(PointerEvent& event) override
```

Handles wheel scrolling and group/row disclosure hit targets while real editors retain their own input.

### `on_key` (public)

```cpp
void on_key(KeyEvent& event) override
```

Implements disclosure and scrolling keys from the PropertyList surface.

### `semantic_descriptor` (public)

```cpp
[[nodiscard]] SemanticDescriptor semantic_descriptor() const override
```

Projects a grouped property/settings surface with row count and descendant inclusion.

### `semantic_virtual_children` (public)

```cpp
[[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override
```

Exposes stable group and row nodes, hierarchy/disclosure state, validation, values, and actions around real editor descendants.

### `on_semantic_child_action` (public)

```cpp
bool on_semantic_child_action(std::string_view stable_id, SemanticAction action, std::string_view value) override
```

Maps virtual selection, expand/collapse, set-value, and reset actions through ordinary row state and events.

### `on_dispose` (protected)

```cpp
void on_dispose() noexcept override
```

Executes PropertyList's on dispose operation against retained state; the signature records its exact inputs, result, constness, and failure surface.
