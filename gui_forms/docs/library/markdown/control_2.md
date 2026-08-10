# Control

Status: **generated inventory; detailed review pending**  
Kind: **class / visual retained control**  
Hierarchy: `Component → enable_shared_from_this → Control`  
Declaration: `include/gui_forms/control.hpp:291`  
Definition: `src/core/control.cpp, src/core/dispatcher.cpp`

Control is a visual retained control declared in include/gui_forms/control.hpp.

## Visual evidence

Capture pending; this page has not yet passed the Screen Sharing crop gate.

## Public methods

### `Control`

```cpp
explicit Control(StableId stable_id)
```

Constructs or tears down the retained Control object according to its ownership contract.

### `~Control`

```cpp
~Control() override
```

Constructs or tears down the retained Control object according to its ownership contract.

### `Control`

```cpp
Control(const Control&) = delete
```

Constructs or tears down the retained Control object according to its ownership contract.

### `operator=`

```cpp
Control& operator=(const Control&) = delete
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `runtime_id`

```cpp
[[nodiscard]] RuntimeId runtime_id() const noexcept
```

Reports the current runtime id value without mutation.

### `stable_id`

```cpp
[[nodiscard]] const StableId& stable_id() const noexcept
```

Reports the current stable id value without mutation.

### `name`

```cpp
[[nodiscard]] const std::string& name() const noexcept
```

Reports the current name value without mutation.

### `set_name`

```cpp
void set_name(std::string name)
```

Synchronously updates the retained name property. Validation, typed invalidation, and notifications are defined by the implementation.

### `name_changed`

```cpp
[[nodiscard]] Event<const std::string&>& name_changed() noexcept
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `visible_changed`

```cpp
[[nodiscard]] Event<bool>& visible_changed() noexcept
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `enabled_changed`

```cpp
[[nodiscard]] Event<bool>& enabled_changed() noexcept
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `tag`

```cpp
[[nodiscard]] const std::any& tag() const noexcept
```

Reports the current tag value without mutation.

### `set_tag`

```cpp
void set_tag(std::any tag)
```

Synchronously updates the retained tag property. Validation, typed invalidation, and notifications are defined by the implementation.

### `parent`

```cpp
[[nodiscard]] Ptr parent() const noexcept
```

Reports the current parent value without mutation.

### `children`

```cpp
[[nodiscard]] std::span<const Ptr> children() const noexcept
```

Reports the current children value without mutation.

### `attached`

```cpp
[[nodiscard]] bool attached() const noexcept
```

Reports the current attached value without mutation.

### `add_child`

```cpp
void add_child(Ptr child)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `remove_child`

```cpp
[[nodiscard]] Ptr remove_child(RuntimeId child)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `set_child_index`

```cpp
bool set_child_index(RuntimeId child, std::size_t index)
```

Synchronously updates the retained child index property. Validation, typed invalidation, and notifications are defined by the implementation.

### `child_index`

```cpp
[[nodiscard]] std::optional<std::size_t> child_index( RuntimeId child) const noexcept
```

Reports the current child index value without mutation.

### `clear_children`

```cpp
void clear_children()
```

Removes the explicit children value and restores fallback behavior.

### `requested_bounds`

```cpp
[[nodiscard]] Rect requested_bounds() const noexcept
```

Reports the current requested bounds value without mutation.

### `set_requested_bounds`

```cpp
void set_requested_bounds(Rect bounds)
```

Synchronously updates the retained requested bounds property. Validation, typed invalidation, and notifications are defined by the implementation.

### `set_bounds`

```cpp
void set_bounds(Rect values, BoundsSpecified specified = BoundsSpecified::all)
```

Synchronously updates the retained bounds property. Validation, typed invalidation, and notifications are defined by the implementation.

### `left`

```cpp
[[nodiscard]] double left() const noexcept
```

Reports the current left value without mutation.

### `top`

```cpp
[[nodiscard]] double top() const noexcept
```

Reports the current top value without mutation.

### `width`

```cpp
[[nodiscard]] double width() const noexcept
```

Reports the current width value without mutation.

### `height`

```cpp
[[nodiscard]] double height() const noexcept
```

Reports the current height value without mutation.

### `right`

```cpp
[[nodiscard]] double right() const noexcept
```

Reports the current right value without mutation.

### `bottom`

```cpp
[[nodiscard]] double bottom() const noexcept
```

Reports the current bottom value without mutation.

### `minimum_size`

```cpp
[[nodiscard]] Size minimum_size() const noexcept
```

Reports the current minimum size value without mutation.

### `set_minimum_size`

```cpp
void set_minimum_size(Size size)
```

Synchronously updates the retained minimum size property. Validation, typed invalidation, and notifications are defined by the implementation.

### `maximum_size`

```cpp
[[nodiscard]] Size maximum_size() const noexcept
```

Reports the current maximum size value without mutation.

### `set_maximum_size`

```cpp
void set_maximum_size(Size size)
```

Synchronously updates the retained maximum size property. Validation, typed invalidation, and notifications are defined by the implementation.

### `arranged_bounds`

```cpp
[[nodiscard]] Rect arranged_bounds() const
```

Reports the current arranged bounds value without mutation.

### `committed_arranged_bounds`

```cpp
[[nodiscard]] Rect committed_arranged_bounds() const noexcept
```

Reports the current committed arranged bounds value without mutation.

### `client_rectangle`

```cpp
[[nodiscard]] Rect client_rectangle() const noexcept
```

Reports the current client rectangle value without mutation.

### `display_rectangle`

```cpp
[[nodiscard]] virtual Rect display_rectangle() const noexcept
```

Reports the current display rectangle value without mutation.

### `absolute_bounds`

```cpp
[[nodiscard]] Rect absolute_bounds() const
```

Reports the current absolute bounds value without mutation.

### `point_to_window`

```cpp
[[nodiscard]] Point point_to_window(Point local) const
```

Reports the current point to window value without mutation.

### `point_from_window`

```cpp
[[nodiscard]] Point point_from_window(Point window_point) const
```

Reports the current point from window value without mutation.

### `rectangle_to_window`

```cpp
[[nodiscard]] Rect rectangle_to_window(Rect local) const
```

Reports the current rectangle to window value without mutation.

### `rectangle_from_window`

```cpp
[[nodiscard]] Rect rectangle_from_window(Rect window_rectangle) const
```

Reports the current rectangle from window value without mutation.

### `contains`

```cpp
[[nodiscard]] bool contains(const Control& candidate) const noexcept
```

Reports the current contains value without mutation.

### `get_child_at_point`

```cpp
[[nodiscard]] Ptr get_child_at_point( Point client_point, GetChildAtPointSkip skip = GetChildAtPointSkip::none) const
```

Reports the current get child at point value without mutation.

### `get_next_control`

```cpp
[[nodiscard]] Ptr get_next_control(const Ptr& control, bool forward) const
```

Reports the current get next control value without mutation.

### `bring_to_front`

```cpp
void bring_to_front()
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `send_to_back`

```cpp
void send_to_back()
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `attached_window`

```cpp
[[nodiscard]] Window* attached_window() const noexcept
```

Reports the current attached window value without mutation.

### `effective_text_scale`

```cpp
[[nodiscard]] double effective_text_scale() const noexcept
```

Reports the current effective text scale value without mutation.

### `effective_font`

```cpp
[[nodiscard]] FontSpec effective_font(FontSpec authored) const noexcept
```

Reports the current effective font value without mutation.

### `effective_theme`

```cpp
[[nodiscard]] const Theme& effective_theme() const noexcept
```

Reports the current effective theme value without mutation.

### `theme_override`

```cpp
[[nodiscard]] std::shared_ptr<const Theme> theme_override() const noexcept
```

Reports the current theme override value without mutation.

### `set_theme_override`

```cpp
void set_theme_override(std::shared_ptr<const Theme> theme)
```

Synchronously updates the retained theme override property. Validation, typed invalidation, and notifications are defined by the implementation.

### `clear_theme_override`

```cpp
void clear_theme_override()
```

Removes the explicit theme override value and restores fallback behavior.

### `visual_status`

```cpp
[[nodiscard]] ControlVisualStatus visual_status() const noexcept
```

Reports the current visual status value without mutation.

### `set_visual_status`

```cpp
void set_visual_status(ControlVisualStatus status)
```

Synchronously updates the retained visual status property. Validation, typed invalidation, and notifications are defined by the implementation.

### `visual_context`

```cpp
[[nodiscard]] ControlVisualContext visual_context( bool hovered = false, bool pressed = false, bool selected = false, bool focused = false, bool defaulted = false) const noexcept
```

Reports the current visual context value without mutation.

### `invoke_required`

```cpp
[[nodiscard]] bool invoke_required() const noexcept
```

Reports the current invoke required value without mutation.

### `begin_invoke`

```cpp
[[nodiscard]] DispatchOperation begin_invoke(std::function<void()> callback)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `invoke`

```cpp
void invoke(std::function<void()> callback)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `margin`

```cpp
[[nodiscard]] Insets margin() const noexcept
```

Reports the current margin value without mutation.

### `set_margin`

```cpp
void set_margin(Insets margin)
```

Synchronously updates the retained margin property. Validation, typed invalidation, and notifications are defined by the implementation.

### `padding`

```cpp
[[nodiscard]] Insets padding() const noexcept
```

Reports the current padding value without mutation.

### `set_padding`

```cpp
void set_padding(Insets padding)
```

Synchronously updates the retained padding property. Validation, typed invalidation, and notifications are defined by the implementation.

### `auto_scroll_offset`

```cpp
[[nodiscard]] Point auto_scroll_offset() const noexcept
```

Reports the current auto scroll offset value without mutation.

### `set_auto_scroll_offset`

```cpp
void set_auto_scroll_offset(Point offset)
```

Synchronously updates the retained auto scroll offset property. Validation, typed invalidation, and notifications are defined by the implementation.

### `dock`

```cpp
[[nodiscard]] DockStyle dock() const noexcept
```

Reports the current dock value without mutation.

### `set_dock`

```cpp
void set_dock(DockStyle dock)
```

Synchronously updates the retained dock property. Validation, typed invalidation, and notifications are defined by the implementation.

### `anchor`

```cpp
[[nodiscard]] AnchorStyles anchor() const noexcept
```

Reports the current anchor value without mutation.

### `set_anchor`

```cpp
void set_anchor(AnchorStyles anchor)
```

Synchronously updates the retained anchor property. Validation, typed invalidation, and notifications are defined by the implementation.

### `auto_size`

```cpp
[[nodiscard]] virtual bool auto_size() const noexcept
```

Reports the current auto size value without mutation.

### `set_auto_size`

```cpp
virtual void set_auto_size(bool auto_size)
```

Synchronously updates the retained auto size property. Validation, typed invalidation, and notifications are defined by the implementation.

### `auto_size_mode`

```cpp
[[nodiscard]] AutoSizeMode auto_size_mode() const noexcept
```

Reports the current auto size mode value without mutation.

### `set_auto_size_mode`

```cpp
void set_auto_size_mode(AutoSizeMode mode)
```

Synchronously updates the retained auto size mode property. Validation, typed invalidation, and notifications are defined by the implementation.

### `auto_size_changed`

```cpp
[[nodiscard]] Event<bool>& auto_size_changed() noexcept
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `get_preferred_size`

```cpp
[[nodiscard]] virtual Size get_preferred_size(Size proposed)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `suspend_layout`

```cpp
void suspend_layout()
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `resume_layout`

```cpp
void resume_layout(bool perform_layout = true)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `perform_layout`

```cpp
void perform_layout()
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `layout_transaction_state`

```cpp
[[nodiscard]] LayoutTransactionState layout_transaction_state() const noexcept
```

Reports the current layout transaction state value without mutation.

### `visible`

```cpp
[[nodiscard]] bool visible() const noexcept
```

Reports the current visible value without mutation.

### `set_visible`

```cpp
void set_visible(bool visible)
```

Synchronously updates the retained visible property. Validation, typed invalidation, and notifications are defined by the implementation.

### `enabled`

```cpp
[[nodiscard]] bool enabled() const noexcept
```

Reports the current enabled value without mutation.

### `set_enabled`

```cpp
void set_enabled(bool enabled)
```

Synchronously updates the retained enabled property. Validation, typed invalidation, and notifications are defined by the implementation.

### `focusable`

```cpp
[[nodiscard]] bool focusable() const noexcept
```

Reports the current focusable value without mutation.

### `set_focusable`

```cpp
void set_focusable(bool focusable)
```

Synchronously updates the retained focusable property. Validation, typed invalidation, and notifications are defined by the implementation.

### `causes_validation`

```cpp
[[nodiscard]] bool causes_validation() const noexcept
```

Reports the current causes validation value without mutation.

### `set_causes_validation`

```cpp
void set_causes_validation(bool causes_validation)
```

Synchronously updates the retained causes validation property. Validation, typed invalidation, and notifications are defined by the implementation.

### `causes_validation_changed`

```cpp
[[nodiscard]] Event<bool>& causes_validation_changed() noexcept
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `validating`

```cpp
[[nodiscard]] Event<ControlValidationEvent&>& validating() noexcept
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `validated`

```cpp
[[nodiscard]] Event<>& validated() noexcept
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `tab_index`

```cpp
[[nodiscard]] std::uint32_t tab_index() const noexcept
```

Reports the current tab index value without mutation.

### `set_tab_index`

```cpp
void set_tab_index(std::uint32_t index)
```

Synchronously updates the retained tab index property. Validation, typed invalidation, and notifications are defined by the implementation.

### `tab_stop`

```cpp
[[nodiscard]] bool tab_stop() const noexcept
```

Reports the current tab stop value without mutation.

### `set_tab_stop`

```cpp
void set_tab_stop(bool enabled)
```

Synchronously updates the retained tab stop property. Validation, typed invalidation, and notifications are defined by the implementation.

### `allow_drop`

```cpp
[[nodiscard]] bool allow_drop() const noexcept
```

Reports the current allow drop value without mutation.

### `set_allow_drop`

```cpp
void set_allow_drop(bool allow_drop)
```

Synchronously updates the retained allow drop property. Validation, typed invalidation, and notifications are defined by the implementation.

### `hit_test_transparent`

```cpp
[[nodiscard]] bool hit_test_transparent() const noexcept
```

Reports the current hit test transparent value without mutation.

### `set_hit_test_transparent`

```cpp
void set_hit_test_transparent(bool transparent)
```

Synchronously updates the retained hit test transparent property. Validation, typed invalidation, and notifications are defined by the implementation.

### `styles`

```cpp
[[nodiscard]] ControlStyles styles() const noexcept
```

Reports the current styles value without mutation.

### `has_style`

```cpp
[[nodiscard]] bool has_style(ControlStyles style) const noexcept
```

Reports the current has style value without mutation.

### `set_style`

```cpp
void set_style(ControlStyles style, bool enabled)
```

Synchronously updates the retained style property. Validation, typed invalidation, and notifications are defined by the implementation.

### `double_buffered`

```cpp
[[nodiscard]] bool double_buffered() const noexcept
```

Reports the current double buffered value without mutation.

### `set_double_buffered`

```cpp
void set_double_buffered(bool enabled)
```

Synchronously updates the retained double buffered property. Validation, typed invalidation, and notifications are defined by the implementation.

### `cursor`

```cpp
[[nodiscard]] std::optional<CursorKind> cursor() const noexcept
```

Reports the current cursor value without mutation.

### `set_cursor`

```cpp
void set_cursor(std::optional<CursorKind> cursor)
```

Synchronously updates the retained cursor property. Validation, typed invalidation, and notifications are defined by the implementation.

### `accessible_name`

```cpp
[[nodiscard]] const std::string& accessible_name() const noexcept
```

Reports the current accessible name value without mutation.

### `set_accessible_name`

```cpp
void set_accessible_name(std::string name)
```

Synchronously updates the retained accessible name property. Validation, typed invalidation, and notifications are defined by the implementation.

### `accessible_description`

```cpp
[[nodiscard]] const std::string& accessible_description() const noexcept
```

Reports the current accessible description value without mutation.

### `set_accessible_description`

```cpp
void set_accessible_description(std::string description)
```

Synchronously updates the retained accessible description property. Validation, typed invalidation, and notifications are defined by the implementation.

### `effective_cursor`

```cpp
[[nodiscard]] CursorKind effective_cursor() const noexcept
```

Reports the current effective cursor value without mutation.

### `effectively_visible`

```cpp
[[nodiscard]] bool effectively_visible() const noexcept
```

Reports the current effectively visible value without mutation.

### `effectively_enabled`

```cpp
[[nodiscard]] bool effectively_enabled() const noexcept
```

Reports the current effectively enabled value without mutation.

### `eligible_for_input`

```cpp
[[nodiscard]] bool eligible_for_input() const noexcept
```

Reports the current eligible for input value without mutation.

### `set_pointer_capture`

```cpp
void set_pointer_capture(bool captured)
```

Synchronously updates the retained pointer capture property. Validation, typed invalidation, and notifications are defined by the implementation.

### `has_pointer_capture`

```cpp
[[nodiscard]] bool has_pointer_capture() const noexcept
```

Reports the current has pointer capture value without mutation.

### `dirty`

```cpp
[[nodiscard]] Dirty dirty() const noexcept
```

Reports the current dirty value without mutation.

### `subtree_dirty`

```cpp
[[nodiscard]] Dirty subtree_dirty() const noexcept
```

Reports the current subtree dirty value without mutation.

### `paint_plane`

```cpp
[[nodiscard]] PaintPlane paint_plane() const noexcept
```

Reports the current paint plane value without mutation.

### `set_paint_plane`

```cpp
void set_paint_plane(PaintPlane plane)
```

Synchronously updates the retained paint plane property. Validation, typed invalidation, and notifications are defined by the implementation.

### `display_chunk_info`

```cpp
[[nodiscard]] std::optional<DisplayChunkInfo> display_chunk_info() const noexcept
```

Reports the current display chunk info value without mutation.

### `invalidate`

```cpp
void invalidate(Dirty dirty)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `invalidate`

```cpp
void invalidate(Rect local_damage)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `invalidate_subtree`

```cpp
void invalidate_subtree(Dirty dirty)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `invalidate_declared`

```cpp
void invalidate_declared(Dirty declared_effects)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `begin_init`

```cpp
void begin_init()
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `end_init`

```cpp
void end_init()
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `initializing`

```cpp
[[nodiscard]] bool initializing() const noexcept
```

Reports the current initializing value without mutation.

### `initialization_depth`

```cpp
[[nodiscard]] std::uint64_t initialization_depth() const noexcept
```

Reports the current initialization depth value without mutation.

### `initialization_completed`

```cpp
[[nodiscard]] Event<Dirty, bool>& initialization_completed() noexcept
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `pointer_observed`

```cpp
[[nodiscard]] Event<const PointerEvent&>& pointer_observed() noexcept
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `focus_observed`

```cpp
[[nodiscard]] Event<bool>& focus_observed() noexcept
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `arranged_bounds_changed`

```cpp
[[nodiscard]] Event<Rect>& arranged_bounds_changed() noexcept
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `help_requested`

```cpp
[[nodiscard]] Event<HelpRequestEvent&>& help_requested() noexcept
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `data_bindings`

```cpp
[[nodiscard]] ControlBindingsCollection& data_bindings()
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `data_bindings`

```cpp
[[nodiscard]] const ControlBindingsCollection& data_bindings() const
```

Reports the current data bindings value without mutation.

### `has_bindable_property`

```cpp
[[nodiscard]] bool has_bindable_property(std::string_view name) const
```

Reports the current has bindable property value without mutation.

### `bindable_property_names`

```cpp
[[nodiscard]] std::vector<std::string> bindable_property_names() const
```

Reports the current bindable property names value without mutation.

### `property_descriptor`

```cpp
[[nodiscard]] std::optional<PropertyDescriptor> property_descriptor( std::string_view name) const
```

Reports the current property descriptor value without mutation.

### `property_descriptors`

```cpp
[[nodiscard]] std::vector<PropertyDescriptor> property_descriptors() const
```

Reports the current property descriptors value without mutation.

### `property_value`

```cpp
[[nodiscard]] std::optional<BindingValue> property_value( std::string_view name) const
```

Reports the current property value value without mutation.

### `set_property_value`

```cpp
void set_property_value(std::string_view name, BindingValue value)
```

Synchronously updates the retained property value property. Validation, typed invalidation, and notifications are defined by the implementation.

### `subscribe_property_changed`

```cpp
[[nodiscard]] SubscriptionToken subscribe_property_changed( std::string_view name, Component& owner, std::function<void()> changed)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `reset_property`

```cpp
bool reset_property(std::string_view name)
```

Returns property to its inherited or default policy.

### `should_serialize_property`

```cpp
[[nodiscard]] bool should_serialize_property(std::string_view name) const
```

Reports the current should serialize property value without mutation.

### `property_value_origin`

```cpp
[[nodiscard]] PropertyValueOrigin property_value_origin( std::string_view name) const
```

Reports the current property value origin value without mutation.

### `measure`

```cpp
[[nodiscard]] virtual Size measure(Size available)
```

Computes desired size from the available constraint without arranging children.

### `arrange`

```cpp
virtual void arrange(Rect final_bounds)
```

Commits final geometry and arranges retained child roles within it.

### `on_paint`

```cpp
virtual void on_paint(Painter& painter, Rect local_damage)
```

Records renderer-neutral paint operations for the damaged local region.

### `visual_outsets`

```cpp
[[nodiscard]] virtual Insets visual_outsets() const noexcept
```

Reports the current visual outsets value without mutation.

### `hit_test_local`

```cpp
[[nodiscard]] virtual bool hit_test_local(Point local_point) const
```

Reports the current hit test local value without mutation.

### `on_pointer_preview`

```cpp
virtual void on_pointer_preview(PointerEvent& event)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_pointer`

```cpp
virtual void on_pointer(PointerEvent& event)
```

Consumes normalized routed pointer input and updates retained interaction state.

### `on_pointer_bubble`

```cpp
virtual void on_pointer_bubble(PointerEvent& event)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_key_preview`

```cpp
virtual void on_key_preview(KeyEvent& event)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_key`

```cpp
virtual void on_key(KeyEvent& event)
```

Consumes normalized keyboard input for this control's interaction contract.

### `on_key_bubble`

```cpp
virtual void on_key_bubble(KeyEvent& event)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_text_input`

```cpp
virtual void on_text_input(TextInputEvent& event)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `process_mnemonic`

```cpp
virtual bool process_mnemonic(char32_t character)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_frame`

```cpp
virtual void on_frame(FrameTime now)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `semantic_descriptor`

```cpp
[[nodiscard]] virtual SemanticDescriptor semantic_descriptor() const
```

Projects the current retained state into the framework semantic/accessibility graph.

### `apply_provider_semantics`

```cpp
void apply_provider_semantics(SemanticDescriptor& descriptor) const
```

Reports the current apply provider semantics value without mutation.

### `semantic_virtual_children`

```cpp
[[nodiscard]] virtual std::vector<SemanticNode> semantic_virtual_children() const
```

Reports the current semantic virtual children value without mutation.

### `on_semantic_action`

```cpp
virtual bool on_semantic_action(SemanticAction action, std::string_view value)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_semantic_child_action`

```cpp
virtual bool on_semantic_child_action(std::string_view stable_id, SemanticAction action, std::string_view value)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_drag_preview`

```cpp
virtual void on_drag_preview(DragEvent& event)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_drag`

```cpp
virtual void on_drag(DragEvent& event)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_drag_bubble`

```cpp
virtual void on_drag_bubble(DragEvent& event)
```

Public Control operation. Its exact signature is inventoried here; follow the linked implementation for callback order and failure behavior.

### `on_focus_changed`

```cpp
virtual void on_focus_changed(bool focused)
```

Updates focus-dependent retained state and invalidates affected presentation/semantics.

### `on_activate`

```cpp
virtual void on_activate()
```

Runs the control's single authoritative activation path.
