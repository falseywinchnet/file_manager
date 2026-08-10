# GUI.Forms ownership inventory

Status: **OBSERVED lexical evidence for a future lifecycle round; no ownership change authorized**.

## Snapshot

| Field | Value |
|---|---|
| Generated UTC | `2026-08-10T16:34:08+00:00` |
| Branch | `main` |
| Commit | `8c6806bab7faddf942b2b02e64ee54a47dc3c7cc` |
| Dirty entries at scan | `46` |
| Scope | `production include/src` |
| C/C++ files | `472` |
| Corpus SHA-256 | `8e3ac1f3f9bb85969af7c32f61b28eabf5fd1bd669a0aedabdbd3f5fdd58a263` |

The report was produced by `gui_forms_rewrite/tools/trace_ownership.py`.
Run it again at the start of any future lifecycle round; do not treat this dirty-tree snapshot as timeless.

## What this proves—and what it does not

The scanner removes ordinary comments/string contents, finds explicit smart-pointer templates and aliases, approximates class-scope ownership edges, and records lifetime operations. It exposes where ownership decisions are written and where a later semantic/AST audit should concentrate.

It does **not** prove runtime reachability, cycle absence, destruction order, callback capture behavior, thread safety, or whether a raw pointer is always non-owning. Macro expansion, inherited unqualified aliases, complex declarators, and type erasure require compilation-database/AST and behavioral evidence. Every lifecycle proposal must still trace construction, transfer, revocation, disposal, and destruction dynamically.

## Inventory summary

### Pointer and alias syntax

| Kind | Occurrences |
|---|---:|
| `alias:shared_ptr` | 2 |
| `alias:weak_ptr` | 1 |
| `qualified-alias:shared_ptr` | 503 |
| `qualified-alias:weak_ptr` | 29 |
| `raw_pointer` | 661 |
| `shared_ptr` | 540 |
| `unique_ptr` | 65 |
| `weak_ptr` | 111 |

### Lifetime operations

| Operation | Occurrences |
|---|---:|
| `component_container` | 14 |
| `delete_expression` | 1 |
| `dynamic_pointer_cast` | 103 |
| `make_shared` | 85 |
| `make_unique` | 32 |
| `new_expression` | 4 |
| `owner_revocable` | 8 |
| `shared_from_this` | 53 |
| `static_pointer_cast` | 27 |
| `subscription_token` | 107 |
| `weak_from_this` | 9 |
| `weak_lock` | 301 |

### Files with the densest explicit ownership surface

| File | Pointer/alias occurrences |
|---|---:|
| `src/core/window/window.cpp` | 178 |
| `src/abi/registry/registry.hpp` | 169 |
| `src/controls/showcase_controls.cpp` | 116 |
| `src/host/macos/application/macos_host.mm` | 97 |
| `include/gui_forms/window/window.hpp` | 94 |
| `src/controls/panel/instrument_rack/instrument_rack.cpp` | 55 |
| `src/controls/panel/property_grid/property_grid.cpp` | 43 |
| `src/controls/panel/property_list/property_list.cpp` | 40 |
| `src/host/macos/services/appkit_host_services.mm` | 40 |
| `src/controls/guidance/error_provider/error_provider.cpp` | 37 |
| `include/gui_forms/c_api.h` | 34 |
| `src/render/skia/raster/skia_raster.cpp` | 32 |
| `src/host/windows/win32_compat_shim.cpp` | 30 |
| `src/controls/guidance/help_provider/help_provider.cpp` | 28 |
| `src/abi/drawing_c_api.cpp` | 27 |
| `src/controls/gallery_controls.cpp` | 26 |
| `src/controls/tool_tip/tool_tip.cpp` | 26 |
| `src/host/windows/application/windows_host.cpp` | 25 |
| `include/gui_forms/components/tool_tip/tool_tip.hpp` | 20 |
| `include/gui_forms/control/control/control.hpp` | 20 |
| `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp` | 19 |
| `include/gui_forms/components/error_provider/error_provider.hpp` | 18 |
| `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp` | 17 |
| `src/core/control/control/control.cpp` | 17 |
| `src/core/binding/binding_source/binding_source.cpp` | 16 |
| `include/gui_forms/controls/panel/card/card.hpp` | 15 |
| `include/gui_forms/drawing_c_api.h` | 15 |
| `src/abi/c_api.cpp` | 15 |
| `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp` | 14 |
| `include/gui_forms/controls/panel/property_grid/property_grid.hpp` | 14 |

## Retained/lifecycle-focused class declarations

This is a review queue, not a proposed graph rewrite.

| Owner | Kind | Target | Location | Declaration evidence |
|---|---|---|---|---|
| `AbiPropertyObjectControl` | `shared_ptr` | `PropertyState` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:607` | `std::map<std::string, std::shared_ptr<PropertyState>> properties_;` |
| `Aggregate` | `shared_ptr` | `Control` | `src/controls/guidance/error_provider/error_provider.cpp:384` | `std::shared_ptr<Control> target;` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:24` | `AnchoredPopupLayer(StableId stable_id, Control::Ptr anchor,` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:27` | `[[nodiscard]] Control::Ptr anchor() const noexcept { return anchor_.lock(); }` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:28` | `void set_anchor(Control::Ptr anchor);` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:29` | `[[nodiscard]] Control::Ptr content() const noexcept { return content_; }` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:30` | `void set_content(Control::Ptr content);` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:57` | `void validate_anchor(const Control::Ptr& anchor) const;` |
| `AnchoredPopupLayer` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:59` | `Control::WeakPtr anchor_;` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:60` | `Control::Ptr content_;` |
| `Binding` | `shared_ptr` | `BindingSource` | `include/gui_forms/binding/binding/binding.hpp:20` | `std::shared_ptr<BindingSource> source, std::string data_member,` |
| `Binding` | `weak_ptr` | `BindingSource` | `include/gui_forms/binding/binding/binding.hpp:65` | `std::weak_ptr<BindingSource> source_;` |
| `BindingContext` | `shared_ptr` | `BindingSource` | `include/gui_forms/binding/binding_context/binding_context.hpp:22` | `void add(const std::shared_ptr<BindingSource>& source);` |
| `BindingContext` | `shared_ptr` | `BindingSource` | `include/gui_forms/binding/binding_context/binding_context.hpp:23` | `CurrencyManager& manager(const std::shared_ptr<BindingSource>& source);` |
| `BindingContext` | `weak_ptr` | `detail::WindowLifetime` | `include/gui_forms/binding/binding_context/binding_context.hpp:44` | `std::weak_ptr<detail::WindowLifetime> window_lifetime_;` |
| `BindingSource` | `shared_ptr` | `Binding` | `include/gui_forms/binding/binding_source/binding_source.hpp:132` | `void register_binding(const std::shared_ptr<Binding>& binding);` |
| `BindingSource` | `weak_ptr` | `detail::WindowLifetime` | `include/gui_forms/binding/binding_source/binding_source.hpp:137` | `std::weak_ptr<detail::WindowLifetime> window_lifetime_;` |
| `BindingSource` | `unique_ptr` | `CurrencyManager` | `include/gui_forms/binding/binding_source/binding_source.hpp:139` | `std::unique_ptr<CurrencyManager> currency_manager_;` |
| `BindingSource` | `weak_ptr` | `Binding` | `include/gui_forms/binding/binding_source/binding_source.hpp:165` | `std::vector<std::weak_ptr<Binding>> bindings_;` |
| `ButtonBase` | `shared_ptr` | `ImageList` | `include/gui_forms/controls/button_base/button_base.hpp:62` | `void set_image_list(std::shared_ptr<ImageList> image_list);` |
| `ButtonBase` | `shared_ptr` | `ImageList` | `include/gui_forms/controls/button_base/button_base.hpp:150` | `std::shared_ptr<ImageList> image_list_;` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:36` | `[[nodiscard]] Control::Ptr header() const noexcept { return header_; }` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:37` | `[[nodiscard]] Control::Ptr body() const noexcept { return body_; }` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:38` | `[[nodiscard]] Control::Ptr footer() const noexcept { return footer_; }` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:39` | `[[nodiscard]] Control::Ptr set_header(Control::Ptr control);` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:39` | `[[nodiscard]] Control::Ptr set_header(Control::Ptr control);` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:40` | `[[nodiscard]] Control::Ptr set_body(Control::Ptr control);` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:40` | `[[nodiscard]] Control::Ptr set_body(Control::Ptr control);` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:41` | `[[nodiscard]] Control::Ptr set_footer(Control::Ptr control);` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:41` | `[[nodiscard]] Control::Ptr set_footer(Control::Ptr control);` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:80` | `Control::Ptr replace_section(Control::Ptr& slot, Control::Ptr replacement);` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:80` | `Control::Ptr replace_section(Control::Ptr& slot, Control::Ptr replacement);` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:80` | `Control::Ptr replace_section(Control::Ptr& slot, Control::Ptr replacement);` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:83` | `Control::Ptr header_;` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:84` | `Control::Ptr body_;` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:85` | `Control::Ptr footer_;` |
| `CommandBinding` | `shared_ptr` | `Command` | `include/gui_forms/commands/command_binding/command_binding.hpp:13` | `CommandBinding(std::shared_ptr<Command> command,` |
| `CommandBinding` | `shared_ptr` | `ButtonBase` | `include/gui_forms/commands/command_binding/command_binding.hpp:14` | `std::shared_ptr<ButtonBase> button,` |
| `CommandBinding` | `shared_ptr` | `Command` | `include/gui_forms/commands/command_binding/command_binding.hpp:32` | `std::shared_ptr<Command> command_;` |
| `CommandBinding` | `shared_ptr` | `ButtonBase` | `include/gui_forms/commands/command_binding/command_binding.hpp:33` | `std::shared_ptr<ButtonBase> button_;` |
| `CompatibilityPaintBinding` | `weak_ptr` | `RasterControl` | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.hpp:17` | `std::weak_ptr<RasterControl> target;` |
| `CompatibilityPaintBinding` | `shared_ptr` | `gui_forms::LiveSurface` | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.hpp:19` | `std::shared_ptr<gui_forms::LiveSurface> surface;` |
| `CompatibilityPaintWrite` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.hpp:23` | `std::shared_ptr<gui_forms::host::WindowsCompatibilityPaintEndpoint> endpoint;` |
| `Component` | `alias:shared_ptr` | `Component` | `include/gui_forms/component/component/component.hpp:13` | `using Ptr = std::shared_ptr<Component>;` |
| `Component` | `shared_ptr` | `Component` | `include/gui_forms/component/component/component.hpp:13` | `using Ptr = std::shared_ptr<Component>;` |
| `Component` | `weak_ptr` | `detail::Revocable` | `include/gui_forms/component/component/component.hpp:30` | `void own_revocable(const std::weak_ptr<detail::Revocable>& revocable);` |
| `Component` | `weak_ptr` | `detail::Revocable` | `include/gui_forms/component/component/component.hpp:39` | `std::vector<std::weak_ptr<detail::Revocable>> owned_revocables_;` |
| `ComponentContainer` | `qualified-alias:shared_ptr` | `Component` | `include/gui_forms/component/component_container/component_container.hpp:17` | `void add(Component::Ptr component);` |
| `ComponentContainer` | `qualified-alias:shared_ptr` | `Component` | `include/gui_forms/component/component_container/component_container.hpp:18` | `[[nodiscard]] Component::Ptr remove(const Component& component);` |
| `ComponentContainer` | `qualified-alias:shared_ptr` | `Component` | `include/gui_forms/component/component_container/component_container.hpp:19` | `[[nodiscard]] std::span<const Component::Ptr> components() const noexcept {` |
| `ComponentContainer` | `qualified-alias:shared_ptr` | `Component` | `include/gui_forms/component/component_container/component_container.hpp:27` | `std::vector<Component::Ptr> components_;` |
| `ContainerControl` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/scrollable_control/container_control/container_control.hpp:11` | `[[nodiscard]] bool contains_descendant(const Control::Ptr& control) const noexcept;` |
| `ContainerControl` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/scrollable_control/container_control/container_control.hpp:12` | `[[nodiscard]] Control::Ptr active_control() const noexcept;` |
| `ContainerControl` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/scrollable_control/container_control/container_control.hpp:13` | `bool request_active_control(const Control::Ptr& control);` |
| `ContextMenu` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/components/context_menu/context_menu.hpp:79` | `void show(const Control::Ptr& owner, Point window_position);` |
| `ContextMenu` | `qualified-alias:shared_ptr` | `Control` | `src/controls/menu/context_menu/context_menu.cpp:332` | `void show(const Control::Ptr& invoker, Point position,` |
| `ContextMenu` | `qualified-alias:shared_ptr` | `Control` | `src/controls/menu/context_menu/context_menu.cpp:477` | `[[nodiscard]] Control::Ptr first_focusable(std::size_t depth) const {` |
| `ContextMenu` | `qualified-alias:shared_ptr` | `Control` | `src/controls/menu/context_menu/context_menu.cpp:653` | `Control::Ptr owner_control;` |
| `Control` | `alias:shared_ptr` | `Control` | `include/gui_forms/control/control/control.hpp:284` | `using Ptr = std::shared_ptr<Control>;` |
| `Control` | `shared_ptr` | `Control` | `include/gui_forms/control/control/control.hpp:284` | `using Ptr = std::shared_ptr<Control>;` |
| `Control` | `alias:weak_ptr` | `Control` | `include/gui_forms/control/control/control.hpp:285` | `using WeakPtr = std::weak_ptr<Control>;` |
| `Control` | `weak_ptr` | `Control` | `include/gui_forms/control/control/control.hpp:285` | `using WeakPtr = std::weak_ptr<Control>;` |
| `Control` | `shared_ptr` | `const Theme` | `include/gui_forms/control/control/control.hpp:370` | `void set_theme_override(std::shared_ptr<const Theme> theme);` |
| `Control` | `shared_ptr` | `detail::DispatcherState` | `include/gui_forms/control/control/control.hpp:677` | `std::shared_ptr<detail::DispatcherState> dispatcher_state_;` |
| `Control` | `shared_ptr` | `const detail::DisplayChunk` | `include/gui_forms/control/control/control.hpp:701` | `std::shared_ptr<const detail::DisplayChunk> display_chunk_;` |
| `Control` | `shared_ptr` | `const Theme` | `include/gui_forms/control/control/control.hpp:716` | `std::shared_ptr<const Theme> theme_override_;` |
| `Control` | `unique_ptr` | `ControlBindingsCollection` | `include/gui_forms/control/control/control.hpp:737` | `mutable std::unique_ptr<ControlBindingsCollection> data_bindings_;` |
| `ControlBindingsCollection` | `shared_ptr` | `BindingSource` | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:25` | `std::string property_name, std::shared_ptr<BindingSource> source,` |
| `ControlBindingsCollection` | `shared_ptr` | `BindingSource` | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:28` | `std::string property_name, std::shared_ptr<BindingSource> source,` |
| `ControlBindingsCollection` | `shared_ptr` | `Binding` | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:30` | `void add(std::shared_ptr<Binding> binding);` |
| `ControlBindingsCollection` | `shared_ptr` | `Binding` | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:51` | `std::vector<std::shared_ptr<Binding>> bindings_;` |
| `ControlCheckpoint` | `qualified-alias:shared_ptr` | `Control` | `src/core/window/presentation/window_presentation.cpp:57` | `Control::Ptr control;` |
| `ControlCheckpoint` | `shared_ptr` | `const detail::DisplayChunk` | `src/core/window/presentation/window_presentation.cpp:58` | `std::shared_ptr<const detail::DisplayChunk> chunk;` |
| `ControlFactory` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/control/static_tree/control_factory/control_factory.hpp:23` | `using Creator = std::function<Control::Ptr(StableId)>;` |
| `ControlFactory` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/control/static_tree/control_factory/control_factory.hpp:26` | `[[nodiscard]] Control::Ptr create(std::string_view type, StableId stable_id) const;` |
| `ControlRecord` | `shared_ptr` | `Control` | `src/abi/registry/registry.hpp:43` | `std::shared_ptr<Control> control;` |
| `ControlRecord` | `weak_ptr` | `Control` | `src/abi/registry/registry.hpp:45` | `std::vector<std::pair<std::weak_ptr<Control>, gui_forms::PaintPlane>>` |
| `DateTimePicker` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/date_time_picker/date_time_picker.hpp:117` | `Control::Ptr popup_calendar_;` |
| `DispatchWork` | `qualified-alias:weak_ptr` | `Control` | `src/core/dispatcher/state/dispatcher_state.hpp:21` | `Control::WeakPtr owner;` |
| `DispatchWork` | `weak_ptr` | `DispatcherState` | `src/core/dispatcher/state/dispatcher_state.hpp:24` | `std::weak_ptr<DispatcherState> dispatcher;` |
| `DispatcherState` | `shared_ptr` | `DispatchWork` | `src/core/dispatcher/state/dispatcher_state.hpp:35` | `std::deque<std::shared_ptr<DispatchWork>> queue;` |
| `ErrorProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/error_provider/error_provider.hpp:77` | `[[nodiscard]] bool can_extend(const std::shared_ptr<Control>& target) const;` |
| `ErrorProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/error_provider/error_provider.hpp:78` | `void set_error(const std::shared_ptr<Control>& target, std::string error);` |
| `ErrorProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/error_provider/error_provider.hpp:83` | `void set_icon_alignment(const std::shared_ptr<Control>& target,` |
| `ErrorProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/error_provider/error_provider.hpp:86` | `void set_icon_padding(const std::shared_ptr<Control>& target, double padding);` |
| `ErrorProvider` | `shared_ptr` | `BindingSource` | `include/gui_forms/components/error_provider/error_provider.hpp:113` | `void set_data_source(std::shared_ptr<BindingSource> source);` |
| `ErrorProvider` | `shared_ptr` | `BindingSource` | `include/gui_forms/components/error_provider/error_provider.hpp:118` | `void bind_to_data_and_errors(std::shared_ptr<BindingSource> source,` |
| `ErrorProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/error_provider/error_provider.hpp:146` | `[[nodiscard]] Entry& require_entry(const std::shared_ptr<Control>& target);` |
| `ErrorProvider` | `weak_ptr` | `detail::WindowLifetime` | `include/gui_forms/components/error_provider/error_provider.hpp:156` | `std::weak_ptr<detail::WindowLifetime> window_lifetime_;` |
| `ErrorProvider` | `weak_ptr` | `BindingSource` | `include/gui_forms/components/error_provider/error_provider.hpp:170` | `std::weak_ptr<BindingSource> data_source_;` |
| `ErrorProvider` | `weak_ptr` | `Control` | `include/gui_forms/components/error_provider/error_provider.hpp:172` | `std::unordered_map<std::uint64_t, std::weak_ptr<Control>> bound_targets_;` |
| `ErrorProvider` | `weak_ptr` | `Control` | `include/gui_forms/components/error_provider/error_provider.hpp:173` | `std::unordered_map<const Binding*, std::pair<std::weak_ptr<Control>, std::string>>` |
| `ErrorProvider` | `weak_ptr` | `Control` | `src/controls/guidance/error_provider/error_provider.cpp:24` | `std::weak_ptr<Control> target;` |
| `ErrorProvider` | `unique_ptr` | `PopupToken` | `src/controls/guidance/error_provider/error_provider.cpp:30` | `std::unique_ptr<PopupToken> popup;` |
| `Event` | `shared_ptr` | `State` | `include/gui_forms/event/event/event.hpp:152` | `std::shared_ptr<State> state_;` |
| `FieldState` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/instrument_rack/instrument_rack.cpp:95` | `Control::Ptr editor;` |
| `FocusScopeState` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:599` | `Control::WeakPtr root;` |
| `FocusScopeState` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:600` | `Control::WeakPtr previous_focus;` |
| `GalleryControl` | `shared_ptr` | `GalleryContext` | `src/controls/gallery/control/gallery_control.hpp:16` | `std::shared_ptr<GalleryContext> context);` |
| `GalleryControl` | `shared_ptr` | `GalleryContext` | `src/controls/gallery/control/gallery_control.hpp:35` | `std::shared_ptr<GalleryContext> context_;` |
| `GalleryTree` | `qualified-alias:shared_ptr` | `Control` | `src/controls/gallery/tree/gallery_tree.hpp:11` | `Control::Ptr root;` |
| `HelpProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/help_provider/help_provider.hpp:65` | `[[nodiscard]] bool can_extend(const std::shared_ptr<Control>& target) const;` |
| `HelpProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/help_provider/help_provider.hpp:66` | `void set_help_string(const std::shared_ptr<Control>& target, std::string text);` |
| `HelpProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/help_provider/help_provider.hpp:68` | `void set_help_keyword(const std::shared_ptr<Control>& target,` |
| `HelpProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/help_provider/help_provider.hpp:71` | `void set_help_navigator(const std::shared_ptr<Control>& target,` |
| `HelpProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/help_provider/help_provider.hpp:74` | `void set_show_help(const std::shared_ptr<Control>& target, bool show);` |
| `HelpProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/help_provider/help_provider.hpp:86` | `bool request_help(const std::shared_ptr<Control>& target, Point position,` |
| `HelpProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/help_provider/help_provider.hpp:105` | `[[nodiscard]] Entry& require_entry(const std::shared_ptr<Control>& target);` |
| `HelpProvider` | `weak_ptr` | `detail::WindowLifetime` | `include/gui_forms/components/help_provider/help_provider.hpp:111` | `std::weak_ptr<detail::WindowLifetime> window_lifetime_;` |
| `HelpProvider` | `weak_ptr` | `Control` | `src/controls/guidance/help_provider/help_provider.cpp:20` | `std::weak_ptr<Control> target;` |
| `ImageList` | `weak_ptr` | `detail::WindowLifetime` | `include/gui_forms/image_list/image_list/image_list.hpp:147` | `std::weak_ptr<detail::WindowLifetime> window_lifetime_;` |
| `ImageSnapshot` | `shared_ptr` | `const PixelStorage` | `include/gui_forms/drawing/image_snapshot/image_snapshot.hpp:27` | `std::shared_ptr<const PixelStorage> storage_;` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/instrument_rack/instrument_rack.hpp:21` | `[[nodiscard]] Control::Ptr field_editor(std::string_view module_id,` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/instrument_rack/instrument_rack.hpp:37` | `void set_action_content(Control::Ptr content,` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/instrument_rack/instrument_rack.hpp:39` | `[[nodiscard]] Control::Ptr action_content() const noexcept;` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/instrument_rack/instrument_rack.cpp:173` | `void connect_focus(ModuleState& module, Control::Ptr control,` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/instrument_rack/instrument_rack.cpp:182` | `Control::Ptr create_field_editor(ModuleState& module,` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/instrument_rack/instrument_rack.cpp:577` | `Control::Ptr action_content;` |
| `Item` | `qualified-alias:shared_ptr` | `Control` | `src/controls/scrollable_control/container_control/flow_layout_panel/flow_layout_panel.cpp:97` | `Control::Ptr control;` |
| `Item` | `qualified-alias:shared_ptr` | `Control` | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:305` | `Control::Ptr control;` |
| `LiveSurface` | `shared_ptr` | `detail::LiveSurfaceState` | `include/gui_forms/live_surface/surface/live_surface.hpp:38` | `explicit LiveSurface(std::shared_ptr<detail::LiveSurfaceState> state) noexcept;` |
| `LiveSurface` | `shared_ptr` | `detail::LiveSurfaceState` | `include/gui_forms/live_surface/surface/live_surface.hpp:39` | `std::shared_ptr<detail::LiveSurfaceState> state_;` |
| `LiveSurfaceRegistration` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:622` | `Control::WeakPtr control;` |
| `LiveSurfaceState` | `shared_ptr` | `LiveSurfaceBuffer` | `src/core/live_surface/state/live_surface_state.hpp:28` | `std::vector<std::shared_ptr<LiveSurfaceBuffer>> buffers;` |
| `LiveSurfaceState` | `shared_ptr` | `LiveSurfaceWake` | `src/core/live_surface/state/live_surface_state.hpp:39` | `std::vector<std::shared_ptr<LiveSurfaceWake>> wakes;` |
| `LiveSurfaceWakeConnection` | `weak_ptr` | `detail::LiveSurfaceState` | `include/gui_forms/live_surface/wake_connection/live_surface_wake_connection.hpp:32` | `std::weak_ptr<detail::LiveSurfaceState> state,` |
| `LiveSurfaceWakeConnection` | `weak_ptr` | `detail::LiveSurfaceState` | `include/gui_forms/live_surface/wake_connection/live_surface_wake_connection.hpp:35` | `std::weak_ptr<detail::LiveSurfaceState> state_;` |
| `LiveSurfaceWriteLease` | `shared_ptr` | `detail::LiveSurfaceState` | `include/gui_forms/live_surface/write_lease/live_surface_write_lease.hpp:43` | `LiveSurfaceWriteLease(std::shared_ptr<detail::LiveSurfaceState> state,` |
| `LiveSurfaceWriteLease` | `shared_ptr` | `detail::LiveSurfaceState` | `include/gui_forms/live_surface/write_lease/live_surface_write_lease.hpp:47` | `std::shared_ptr<detail::LiveSurfaceState> state_;` |
| `MacApplicationWindow` | `unique_ptr` | `Window` | `include/gui_forms/platform/macos_host.hpp:48` | `std::unique_ptr<Window> model;` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:47` | `[[nodiscard]] Control::Ptr master() const noexcept { return master_; }` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:48` | `[[nodiscard]] Control::Ptr detail() const noexcept { return detail_; }` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:49` | `[[nodiscard]] Control::Ptr set_master(Control::Ptr control);` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:49` | `[[nodiscard]] Control::Ptr set_master(Control::Ptr control);` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:50` | `[[nodiscard]] Control::Ptr set_detail(Control::Ptr control);` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:50` | `[[nodiscard]] Control::Ptr set_detail(Control::Ptr control);` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:86` | `[[nodiscard]] Control::Ptr replace_role(Control::Ptr& slot,` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:86` | `[[nodiscard]] Control::Ptr replace_role(Control::Ptr& slot,` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:88` | `Control::Ptr replacement);` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:95` | `Control::Ptr master_;` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:96` | `Control::Ptr detail_;` |
| `ModuleState` | `shared_ptr` | `RackModulePanel` | `src/controls/panel/instrument_rack/instrument_rack.cpp:106` | `std::shared_ptr<RackModulePanel> panel;` |
| `ModuleState` | `shared_ptr` | `CheckBox` | `src/controls/panel/instrument_rack/instrument_rack.cpp:107` | `std::shared_ptr<CheckBox> enable;` |
| `ModuleState` | `shared_ptr` | `Label` | `src/controls/panel/instrument_rack/instrument_rack.cpp:109` | `std::shared_ptr<Label> status;` |
| `ModuleState` | `shared_ptr` | `Button` | `src/controls/panel/instrument_rack/instrument_rack.cpp:110` | `std::shared_ptr<Button> remove;` |
| `NumericUpDown` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/numeric_up_down/numeric_up_down.hpp:49` | `Control::Ptr spinner_;` |
| `ObjectView` | `shared_ptr` | `ImageList` | `include/gui_forms/controls/panel/object_view/object_view.hpp:109` | `void set_image_list(std::shared_ptr<ImageList> image_list);` |
| `ObjectView` | `shared_ptr` | `ImageList` | `include/gui_forms/controls/panel/object_view/object_view.hpp:178` | `std::shared_ptr<ImageList> image_list_;` |
| `OwnerEdit` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/property_grid/property_grid.cpp:783` | `Control::Ptr object;` |
| `PanelState` | `shared_ptr` | `MenuPanel` | `src/controls/menu/context_menu/context_menu.cpp:27` | `std::shared_ptr<MenuPanel> panel;` |
| `PanelState` | `shared_ptr` | `MenuRow` | `src/controls/menu/context_menu/context_menu.cpp:28` | `std::vector<std::shared_ptr<MenuRow>> rows;` |
| `PopupAttachment` | `qualified-alias:shared_ptr` | `Control` | `src/core/window/popup/popup_attachment.hpp:9` | `PopupAttachment(Window& window, Control::Ptr owner, Control::Ptr popup)` |
| `PopupAttachment` | `qualified-alias:shared_ptr` | `Control` | `src/core/window/popup/popup_attachment.hpp:9` | `PopupAttachment(Window& window, Control::Ptr owner, Control::Ptr popup)` |
| `PopupAttachment` | `qualified-alias:shared_ptr` | `Control` | `src/core/window/popup/popup_attachment.hpp:18` | `[[nodiscard]] Control::Ptr owner() const noexcept { return owner_.lock(); }` |
| `PopupAttachment` | `qualified-alias:shared_ptr` | `Control` | `src/core/window/popup/popup_attachment.hpp:19` | `[[nodiscard]] Control::Ptr popup() const noexcept { return popup_.lock(); }` |
| `PopupAttachment` | `qualified-alias:weak_ptr` | `Control` | `src/core/window/popup/popup_attachment.hpp:32` | `Control::WeakPtr owner_;` |
| `PopupAttachment` | `qualified-alias:weak_ptr` | `Control` | `src/core/window/popup/popup_attachment.hpp:33` | `Control::WeakPtr popup_;` |
| `PopupToken` | `shared_ptr` | `detail::PopupAttachment` | `include/gui_forms/window/window.hpp:70` | `std::shared_ptr<detail::PopupAttachment> attachment_;` |
| `PropertyEditorBinding` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/inspection/property_editor_registry/property_editor_registry.hpp:15` | `Control::Ptr control;` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:20` | `[[nodiscard]] Control::Ptr selected_object() const noexcept;` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:21` | `void set_selected_object(Control::Ptr object);` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:22` | `[[nodiscard]] std::vector<Control::Ptr> selected_objects() const;` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:23` | `void set_selected_objects(std::vector<Control::Ptr> objects);` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:36` | `[[nodiscard]] Control::Ptr editor(std::string_view property_name) const;` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:61` | `[[nodiscard]] Event<Control::Ptr>& selected_object_changed() noexcept {` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:80` | `Event<Control::Ptr> selected_object_changed_;` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/property_grid/property_grid.cpp:325` | `[[nodiscard]] Control::Ptr target() const noexcept {` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/property_grid/property_grid.cpp:331` | `[[nodiscard]] std::vector<Control::Ptr> targets() const {` |
| `PropertyGrid` | `qualified-alias:weak_ptr` | `Control` | `src/controls/panel/property_grid/property_grid.cpp:1056` | `std::vector<Control::WeakPtr> selected;` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_list/property_list.hpp:30` | `[[nodiscard]] Control::Ptr editor(std::string_view row_id) const;` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_list/property_list.hpp:31` | `bool replace_editor(std::string_view row_id, Control::Ptr editor);` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_list/property_list.hpp:34` | `void set_header_content(Control::Ptr content, double height);` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_list/property_list.hpp:35` | `[[nodiscard]] Control::Ptr header_content() const noexcept;` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/property_list/property_list.cpp:429` | `Control::Ptr header;` |
| `RackModulePanel` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/instrument_rack/rack_module_panel/rack_module_panel.hpp:19` | `std::vector<Control::Ptr> fields;` |
| `RasterControl` | `shared_ptr` | `gui_forms::LiveSurface` | `src/abi/control_adapters/raster_control/raster_control.hpp:322` | `std::shared_ptr<gui_forms::LiveSurface> live_surface_;` |
| `RasterControl` | `shared_ptr` | `LiveWakeState` | `src/abi/control_adapters/raster_control/raster_control.hpp:325` | `std::shared_ptr<LiveWakeState> live_wake_state_;` |
| `Registry` | `shared_ptr` | `ControlRecord` | `src/abi/registry/registry.hpp:2605` | `const std::shared_ptr<ControlRecord>& record) {` |
| `Registry` | `shared_ptr` | `ControlRecord` | `src/abi/registry/registry.hpp:2717` | `void cancel_pending(const std::shared_ptr<ControlRecord>& root) noexcept {` |
| `Registry` | `shared_ptr` | `ControlRecord` | `src/abi/registry/registry.hpp:3032` | `const std::shared_ptr<ControlRecord>& sender) {` |
| `RegistrySlot` | `shared_ptr` | `ControlRecord` | `src/abi/registry/registry.hpp:78` | `std::shared_ptr<ControlRecord> control;` |
| `RegistrySlot` | `shared_ptr` | `SubscriptionRecord` | `src/abi/registry/registry.hpp:79` | `std::shared_ptr<SubscriptionRecord> subscription;` |
| `RowState` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/property_list/property_list.cpp:29` | `Control::Ptr editor;` |
| `RowState` | `shared_ptr` | `Button` | `src/controls/panel/property_list/property_list.cpp:30` | `std::shared_ptr<Button> reset_button;` |
| `ScaledGroupBox` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/group_box/scaled_group_box/scaled_group_box.hpp:18` | `void add_at(Control::Ptr child, Rect design_bounds);` |
| `ScaledPanel` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/scaled_panel/scaled_panel.hpp:16` | `void add_at(Control::Ptr child, Rect design_bounds);` |
| `ScheduledFrameRequest` | `qualified-alias:weak_ptr` | `Control` | `src/core/scheduler/request/scheduled_frame_request.hpp:19` | `Control::WeakPtr request_target,` |
| `ScheduledFrameRequest` | `qualified-alias:weak_ptr` | `Control` | `src/core/scheduler/request/scheduled_frame_request.hpp:30` | `Control::WeakPtr target;` |
| `ScrollableControl` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/scrollable_control/scrollable_control.hpp:84` | `void scroll_control_into_view(const Control::Ptr& control);` |
| `ShowcaseContext` | `qualified-alias:shared_ptr` | `Control` | `src/controls/showcase_controls.cpp:63` | `std::vector<Control::Ptr> pages;` |
| `ShowcaseContext` | `shared_ptr` | `Timer` | `src/controls/showcase_controls.cpp:71` | `std::shared_ptr<Timer> ui_timer;` |
| `ShowcaseContext` | `shared_ptr` | `BindingSource` | `src/controls/showcase_controls.cpp:75` | `std::shared_ptr<BindingSource> binding_source;` |
| `ShowcaseTree` | `qualified-alias:shared_ptr` | `Control` | `src/controls/showcase_controls.hpp:12` | `Control::Ptr root;` |
| `Slot` | `weak_ptr` | `State` | `include/gui_forms/event/event/event.hpp:122` | `std::weak_ptr<State> state;` |
| `Slot` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/instrument_rack/instrument_rack.cpp:498` | `Control::Ptr action;` |
| `SourceEntry` | `weak_ptr` | `BindingSource` | `include/gui_forms/binding/binding_context/binding_context.hpp:38` | `std::weak_ptr<BindingSource> source;` |
| `SplitContainer` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp:53` | `[[nodiscard]] Control::Ptr splitter_control() const noexcept {` |
| `SplitContainer` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp:147` | `Control::Ptr splitter_;` |
| `State` | `shared_ptr` | `Slot` | `include/gui_forms/event/event/event.hpp:128` | `std::vector<std::shared_ptr<Slot>> slots;` |
| `SubscriptionToken` | `shared_ptr` | `detail::Revocable` | `include/gui_forms/event/event/event.hpp:46` | `std::shared_ptr<detail::Revocable> revocable_;` |
| `TabControl` | `shared_ptr` | `TabPage` | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:38` | `void add_page(std::shared_ptr<TabPage> page);` |
| `TabControl` | `shared_ptr` | `TabPage` | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:49` | `void set_selected_tab(const std::shared_ptr<TabPage>& page);` |
| `TabControl` | `shared_ptr` | `TabPage` | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:78` | `const std::shared_ptr<TabPage>& page) const;` |
| `TabControl` | `shared_ptr` | `TabPage` | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:80` | `void remember_page_focus(const std::shared_ptr<TabPage>& page);` |
| `TabControl` | `shared_ptr` | `TabPage` | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:81` | `void restore_page_focus(const std::shared_ptr<TabPage>& page,` |
| `TabControl` | `weak_ptr` | `TabPage` | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:85` | `std::vector<std::weak_ptr<TabPage>> pages_;` |
| `TabControl` | `weak_ptr` | `TabPage` | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:86` | `std::weak_ptr<TabPage> selected_page_;` |
| `TabControl` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:87` | `std::unordered_map<std::uint64_t, Control::WeakPtr> remembered_focus_;` |
| `TableLayoutPanel` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.hpp:88` | `[[nodiscard]] Control::Ptr control_from_position(std::size_t column,` |
| `Timer` | `weak_ptr` | `detail::WindowLifetime` | `include/gui_forms/timer/timer/timer.hpp:49` | `std::weak_ptr<detail::WindowLifetime> window_lifetime_;` |
| `Timer` | `shared_ptr` | `CallbackState` | `include/gui_forms/timer/timer/timer.hpp:51` | `std::shared_ptr<CallbackState> callback_state_;` |
| `ToolTip` | `shared_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:35` | `void set_tool_tip(const std::shared_ptr<Control>& target, std::string text);` |
| `ToolTip` | `shared_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:63` | `void show(const std::shared_ptr<Control>& target);` |
| `ToolTip` | `shared_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:64` | `void show(const std::shared_ptr<Control>& target,` |
| `ToolTip` | `shared_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:86` | `void target_pointer(const std::shared_ptr<Control>& target,` |
| `ToolTip` | `shared_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:88` | `void target_focus(const std::shared_ptr<Control>& target, bool focused);` |
| `ToolTip` | `shared_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:89` | `void target_moved(const std::shared_ptr<Control>& target);` |
| `ToolTip` | `shared_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:90` | `void schedule_show(const std::shared_ptr<Control>& target,` |
| `ToolTip` | `shared_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:94` | `void show_now(const std::shared_ptr<Control>& target,` |
| `ToolTip` | `weak_ptr` | `detail::WindowLifetime` | `include/gui_forms/components/tool_tip/tool_tip.hpp:101` | `std::weak_ptr<detail::WindowLifetime> window_lifetime_;` |
| `ToolTip` | `unique_ptr` | `Timer` | `include/gui_forms/components/tool_tip/tool_tip.hpp:102` | `std::unique_ptr<Timer> timer_;` |
| `ToolTip` | `weak_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:104` | `std::weak_ptr<Control> pending_target_;` |
| `ToolTip` | `weak_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:105` | `std::weak_ptr<Control> visible_target_;` |
| `ToolTip` | `shared_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:106` | `std::shared_ptr<Control> overlay_layer_;` |
| `ToolTip` | `shared_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:107` | `std::shared_ptr<Control> overlay_bubble_;` |
| `ToolTip` | `unique_ptr` | `PopupHolder` | `include/gui_forms/components/tool_tip/tool_tip.hpp:109` | `std::unique_ptr<PopupHolder> popup_;` |
| `ToolTip` | `weak_ptr` | `Control` | `src/controls/tool_tip/tool_tip.cpp:24` | `std::weak_ptr<Control> target;` |
| `TreeView` | `shared_ptr` | `ImageList` | `include/gui_forms/controls/panel/tree_view/tree_view.hpp:70` | `void set_image_list(std::shared_ptr<ImageList> image_list);` |
| `TreeView` | `shared_ptr` | `ImageList` | `include/gui_forms/controls/panel/tree_view/tree_view.hpp:124` | `std::shared_ptr<ImageList> image_list_;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:217` | `explicit Window(Control::Ptr root, Size client_size = {});` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:222` | `[[nodiscard]] Control::Ptr root() const noexcept { return root_; }` |
| `Window` | `shared_ptr` | `const Theme` | `include/gui_forms/window/window.hpp:239` | `void set_theme(std::shared_ptr<const Theme> theme);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:275` | `const Control::Ptr& control, std::shared_ptr<LiveSurface> surface);` |
| `Window` | `shared_ptr` | `LiveSurface` | `include/gui_forms/window/window.hpp:275` | `const Control::Ptr& control, std::shared_ptr<LiveSurface> surface);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:289` | `[[nodiscard]] FrameRequestToken schedule_paint(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:291` | `[[nodiscard]] FrameRequestToken activate_surface(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:306` | `const Control::Ptr& owner, std::function<void()> callback);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:310` | `void invoke(const Control::Ptr& owner, std::function<void()> callback);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:355` | `[[nodiscard]] Control::Ptr find(std::string_view stable_id) const;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:356` | `[[nodiscard]] Control::Ptr hit_test(Point position);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:357` | `bool request_focus(const Control::Ptr& control);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:358` | `[[nodiscard]] Control::Ptr focused_control() const noexcept { return focused_.lock(); }` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:359` | `bool validate_control(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:363` | `const Control::Ptr& container,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:367` | `const Control::Ptr& root,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:368` | `const Control::Ptr& preferred_focus = {},` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:374` | `[[nodiscard]] Control::Ptr active_focus_scope_root() const noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:376` | `void set_accept_button(const Control::Ptr& control);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:377` | `void set_cancel_button(const Control::Ptr& control);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:378` | `[[nodiscard]] Control::Ptr accept_button() const noexcept {` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:381` | `[[nodiscard]] Control::Ptr cancel_button() const noexcept {` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:395` | `void capture_pointer(const Control::Ptr& control, std::uint64_t pointer_id = 1);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:397` | `[[nodiscard]] Control::Ptr captured_control() const noexcept { return captured_.lock(); }` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:408` | `[[nodiscard]] PopupToken open_popup(const Control::Ptr& owner,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:409` | `const Control::Ptr& popup,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:417` | `[[nodiscard]] Control::Ptr pressed_control() const noexcept { return pressed_.lock(); }` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:455` | `void attach_subtree(const Control::Ptr& control, const Control::WeakPtr& parent);` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:455` | `void attach_subtree(const Control::Ptr& control, const Control::WeakPtr& parent);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:456` | `void detach_subtree(const Control::Ptr& control);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:457` | `void dispose_subtree(const Control::Ptr& control) noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:458` | `void revoke_interaction_for_subtree(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:460` | `void close_focus_scopes_for_subtree(const Control::Ptr& control);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:463` | `const Control::Ptr& notification_owner);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:464` | `void revoke_focus_scopes_for_subtree(const Control::Ptr& control) noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:465` | `void close_popups_for_subtree(const Control::Ptr& control) noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:471` | `const Control::Ptr& control) const noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:472` | `[[nodiscard]] std::vector<Control::Ptr> focus_candidates(` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:473` | `const Control::Ptr& scope_root) const;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:475` | `const Control::Ptr& previous, const Control::Ptr& destination,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:475` | `const Control::Ptr& previous, const Control::Ptr& destination,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:477` | `void change_pointer_capture(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:480` | `void on_eligibility_changed(const Control::Ptr& control);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:481` | `void on_hit_test_transparency_changed(const Control::Ptr& control);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:483` | `void register_subtree(const Control::Ptr& control);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:484` | `void unregister_subtree(const Control::Ptr& control);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:509` | `void add_subtree_damage(const Control::Ptr& control);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:513` | `[[nodiscard]] std::vector<Control::Ptr> route_to(const Control::Ptr& target) const;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:513` | `[[nodiscard]] std::vector<Control::Ptr> route_to(const Control::Ptr& target) const;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:514` | `[[nodiscard]] Control::Ptr drop_target_at(Point position);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:515` | `[[nodiscard]] DragDispatchResult route_drag(const Control::Ptr& target,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:517` | `[[nodiscard]] Control::Ptr hit_test_recursive(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:517` | `[[nodiscard]] Control::Ptr hit_test_recursive(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:519` | `void paint_recursive(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:529` | `void measure_dirty_recursive(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:533` | `void arrange_dirty_recursive(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:538` | `const Control::Ptr& control) const noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:540` | `void commit_layout_requests_recursive(const Control::Ptr& control) noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:541` | `[[nodiscard]] Dirty recompute_subtree_dirty(const Control::Ptr& control) noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:542` | `void clear_layout_dirty_subtree(const Control::Ptr& control) noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:543` | `void clear_paint_dirty_subtree(const Control::Ptr& control) noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:545` | `const Control::Ptr& control) const noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:554` | `[[nodiscard]] bool eligible(const Control::Ptr& control) const noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:555` | `[[nodiscard]] bool move_focus_after(const Control::Ptr& origin);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:557` | `const Control::Ptr& destination);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:560` | `void clear_dialog_targets_for_subtree(const Control::Ptr& control) noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:563` | `Control::Ptr root_;` |
| `Window` | `shared_ptr` | `const Theme` | `include/gui_forms/window/window.hpp:568` | `std::shared_ptr<const Theme> theme_;` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:572` | `std::unordered_map<std::string, Control::WeakPtr> stable_ids_;` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:573` | `Control::WeakPtr focused_;` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:574` | `Control::WeakPtr accept_button_;` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:575` | `Control::WeakPtr cancel_button_;` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:586` | `Control::WeakPtr mnemonic_cursor_;` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:609` | `Control::WeakPtr captured_;` |
| `Window` | `shared_ptr` | `detail::PopupAttachment` | `include/gui_forms/window/window.hpp:613` | `std::vector<std::shared_ptr<detail::PopupAttachment>> popups_;` |
| `Window` | `shared_ptr` | `detail::AcceleratorAttachment` | `include/gui_forms/window/window.hpp:614` | `std::vector<std::shared_ptr<detail::AcceleratorAttachment>> accelerators_;` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:615` | `Control::WeakPtr pressed_;` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:616` | `Control::WeakPtr hovered_;` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:617` | `Control::WeakPtr drag_target_;` |
| `Window` | `shared_ptr` | `detail::ScheduledFrameRequest` | `include/gui_forms/window/window.hpp:633` | `std::vector<std::shared_ptr<detail::ScheduledFrameRequest>> frame_requests_;` |
| `Window` | `shared_ptr` | `detail::WindowLifetime` | `include/gui_forms/window/window.hpp:635` | `std::shared_ptr<detail::WindowLifetime> lifetime_;` |
| `Window` | `shared_ptr` | `detail::DispatcherState` | `include/gui_forms/window/window.hpp:637` | `std::shared_ptr<detail::DispatcherState> dispatcher_state_;` |
| `WindowsApplicationWindow` | `unique_ptr` | `Window` | `include/gui_forms/platform/windows_host.hpp:46` | `std::unique_ptr<Window> model;` |
| `WindowsCompatibilityPaintEndpoint` | `unique_ptr` | `Implementation` | `include/gui_forms/platform/windows_compatibility_paint_endpoint/windows_compatibility_paint_endpoint.hpp:51` | `std::unique_ptr<Implementation> implementation) noexcept;` |
| `WindowsCompatibilityPaintEndpoint` | `unique_ptr` | `Implementation` | `include/gui_forms/platform/windows_compatibility_paint_endpoint/windows_compatibility_paint_endpoint.hpp:53` | `std::unique_ptr<Implementation> implementation_;` |
| `WindowsCompatibilityPaintEndpoint` | `shared_ptr` | `LiveSurface` | `src/host/windows/paint_endpoint/windows_compatibility_paint_endpoint.cpp:453` | `std::shared_ptr<LiveSurface> surface;` |
| `WindowsHostState` | `unique_ptr` | `Window` | `src/host/windows/application/windows_host.cpp:2670` | `std::unique_ptr<Window> model_;` |

## All detected smart-pointer and alias evidence

| Owner/scope | Kind | Target | Scope | Location |
|---|---|---|---|---|
| `Binding` | `shared_ptr` | `BindingSource` | class declaration | `include/gui_forms/binding/binding/binding.hpp:20` |
| `Binding` | `shared_ptr` | `BindingSource` | local/signature/use | `include/gui_forms/binding/binding/binding.hpp:25` |
| `Binding` | `weak_ptr` | `BindingSource` | class declaration | `include/gui_forms/binding/binding/binding.hpp:65` |
| `BindingContext` | `shared_ptr` | `BindingSource` | class declaration | `include/gui_forms/binding/binding_context/binding_context.hpp:22` |
| `BindingContext` | `shared_ptr` | `BindingSource` | class declaration | `include/gui_forms/binding/binding_context/binding_context.hpp:23` |
| `SourceEntry` | `weak_ptr` | `BindingSource` | class declaration | `include/gui_forms/binding/binding_context/binding_context.hpp:38` |
| `BindingContext` | `weak_ptr` | `detail::WindowLifetime` | class declaration | `include/gui_forms/binding/binding_context/binding_context.hpp:44` |
| `BindingSource` | `shared_ptr` | `Binding` | local/signature/use | `include/gui_forms/binding/binding_source/binding_source.hpp:48` |
| `BindingSource` | `shared_ptr` | `Binding` | class declaration | `include/gui_forms/binding/binding_source/binding_source.hpp:132` |
| `BindingSource` | `weak_ptr` | `detail::WindowLifetime` | class declaration | `include/gui_forms/binding/binding_source/binding_source.hpp:137` |
| `BindingSource` | `unique_ptr` | `CurrencyManager` | class declaration | `include/gui_forms/binding/binding_source/binding_source.hpp:139` |
| `BindingSource` | `weak_ptr` | `Binding` | class declaration | `include/gui_forms/binding/binding_source/binding_source.hpp:165` |
| `ControlBindingsCollection` | `shared_ptr` | `Binding` | local/signature/use | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:24` |
| `ControlBindingsCollection` | `shared_ptr` | `BindingSource` | class declaration | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:25` |
| `ControlBindingsCollection` | `shared_ptr` | `Binding` | local/signature/use | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:27` |
| `ControlBindingsCollection` | `shared_ptr` | `BindingSource` | class declaration | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:28` |
| `ControlBindingsCollection` | `shared_ptr` | `Binding` | class declaration | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:30` |
| `ControlBindingsCollection` | `shared_ptr` | `Binding` | local/signature/use | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:33` |
| `ControlBindingsCollection` | `shared_ptr` | `Binding` | local/signature/use | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:35` |
| `ControlBindingsCollection` | `shared_ptr` | `Binding` | class declaration | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:51` |
| `PropertyObjectMember` | `shared_ptr` | `const PropertyEnumDescriptor` | class declaration | `include/gui_forms/binding/value/binding_value.hpp:52` |
| `PropertyDescriptor` | `shared_ptr` | `const PropertyEnumDescriptor` | class declaration | `include/gui_forms/binding/value/binding_value.hpp:178` |
| `PropertyCollectionValue` | `shared_ptr` | `const PropertyCollectionData` | local/signature/use | `include/gui_forms/binding/value/property_collection_value/property_collection_value.hpp:29` |
| `PropertyCollectionValue` | `shared_ptr` | `const PropertyCollectionData` | class declaration | `include/gui_forms/binding/value/property_collection_value/property_collection_value.hpp:31` |
| `PropertyObjectValue` | `shared_ptr` | `const PropertyObjectData` | local/signature/use | `include/gui_forms/binding/value/property_object_value/property_object_value.hpp:29` |
| `PropertyObjectValue` | `shared_ptr` | `const PropertyObjectData` | class declaration | `include/gui_forms/binding/value/property_object_value/property_object_value.hpp:31` |
| `CommandBinding` | `shared_ptr` | `Command` | class declaration | `include/gui_forms/commands/command_binding/command_binding.hpp:13` |
| `CommandBinding` | `shared_ptr` | `ButtonBase` | class declaration | `include/gui_forms/commands/command_binding/command_binding.hpp:14` |
| `CommandBinding` | `shared_ptr` | `Command` | local/signature/use | `include/gui_forms/commands/command_binding/command_binding.hpp:22` |
| `CommandBinding` | `shared_ptr` | `ButtonBase` | local/signature/use | `include/gui_forms/commands/command_binding/command_binding.hpp:25` |
| `CommandBinding` | `shared_ptr` | `Command` | class declaration | `include/gui_forms/commands/command_binding/command_binding.hpp:32` |
| `CommandBinding` | `shared_ptr` | `ButtonBase` | class declaration | `include/gui_forms/commands/command_binding/command_binding.hpp:33` |
| `Component` | `alias:shared_ptr` | `Component` | alias definition | `include/gui_forms/component/component/component.hpp:13` |
| `Component` | `shared_ptr` | `Component` | class declaration | `include/gui_forms/component/component/component.hpp:13` |
| `Component` | `weak_ptr` | `detail::Revocable` | class declaration | `include/gui_forms/component/component/component.hpp:30` |
| `Component` | `weak_ptr` | `detail::Revocable` | class declaration | `include/gui_forms/component/component/component.hpp:39` |
| `ComponentContainer` | `qualified-alias:shared_ptr` | `Component` | class declaration | `include/gui_forms/component/component_container/component_container.hpp:17` |
| `ComponentContainer` | `qualified-alias:shared_ptr` | `Component` | class declaration | `include/gui_forms/component/component_container/component_container.hpp:18` |
| `ComponentContainer` | `qualified-alias:shared_ptr` | `Component` | class declaration | `include/gui_forms/component/component_container/component_container.hpp:19` |
| `ComponentContainer` | `qualified-alias:shared_ptr` | `Component` | class declaration | `include/gui_forms/component/component_container/component_container.hpp:27` |
| `MenuItemSpec` | `shared_ptr` | `Command` | local/signature/use | `include/gui_forms/components/context_menu/context_menu.hpp:34` |
| `MenuItemSpec` | `shared_ptr` | `Command` | class declaration | `include/gui_forms/components/context_menu/context_menu.hpp:43` |
| `ContextMenu` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/components/context_menu/context_menu.hpp:79` |
| `ContextMenu` | `unique_ptr` | `Impl` | class declaration | `include/gui_forms/components/context_menu/context_menu.hpp:102` |
| `ErrorProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:77` |
| `ErrorProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:78` |
| `ErrorProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:83` |
| `ErrorProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:86` |
| `ErrorProvider` | `shared_ptr` | `Control` | local/signature/use | `include/gui_forms/components/error_provider/error_provider.hpp:108` |
| `ErrorProvider` | `shared_ptr` | `BindingSource` | local/signature/use | `include/gui_forms/components/error_provider/error_provider.hpp:110` |
| `ErrorProvider` | `shared_ptr` | `BindingSource` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:113` |
| `ErrorProvider` | `shared_ptr` | `BindingSource` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:118` |
| `ErrorProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:146` |
| `ErrorProvider` | `weak_ptr` | `detail::WindowLifetime` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:156` |
| `ErrorProvider` | `unique_ptr` | `Entry` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:157` |
| `ErrorProvider` | `unique_ptr` | `ToolTip` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:158` |
| `ErrorProvider` | `weak_ptr` | `BindingSource` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:170` |
| `ErrorProvider` | `weak_ptr` | `Control` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:172` |
| `ErrorProvider` | `weak_ptr` | `Control` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:173` |
| `HelpProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:65` |
| `HelpProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:66` |
| `HelpProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:68` |
| `HelpProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:71` |
| `HelpProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:74` |
| `HelpProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:86` |
| `HelpProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:105` |
| `HelpProvider` | `weak_ptr` | `detail::WindowLifetime` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:111` |
| `HelpProvider` | `unique_ptr` | `Entry` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:112` |
| `HelpProvider` | `unique_ptr` | `AcceleratorHolder` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:113` |
| `ToolTip` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:35` |
| `ToolTip` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:63` |
| `ToolTip` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:64` |
| `ToolTip` | `shared_ptr` | `Control` | local/signature/use | `include/gui_forms/components/tool_tip/tool_tip.hpp:68` |
| `ToolTip` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:86` |
| `ToolTip` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:88` |
| `ToolTip` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:89` |
| `ToolTip` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:90` |
| `ToolTip` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:94` |
| `ToolTip` | `weak_ptr` | `detail::WindowLifetime` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:101` |
| `ToolTip` | `unique_ptr` | `Timer` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:102` |
| `ToolTip` | `unique_ptr` | `Entry` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:103` |
| `ToolTip` | `weak_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:104` |
| `ToolTip` | `weak_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:105` |
| `ToolTip` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:106` |
| `ToolTip` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:107` |
| `ToolTip` | `unique_ptr` | `PopupHolder` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:109` |
| `Control` | `alias:shared_ptr` | `Control` | alias definition | `include/gui_forms/control/control/control.hpp:284` |
| `Control` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/control/control/control.hpp:284` |
| `Control` | `alias:weak_ptr` | `Control` | alias definition | `include/gui_forms/control/control/control.hpp:285` |
| `Control` | `weak_ptr` | `Control` | class declaration | `include/gui_forms/control/control/control.hpp:285` |
| `Control` | `shared_ptr` | `const Theme` | local/signature/use | `include/gui_forms/control/control/control.hpp:367` |
| `Control` | `shared_ptr` | `const Theme` | class declaration | `include/gui_forms/control/control/control.hpp:370` |
| `Control` | `shared_ptr` | `detail::DispatcherState` | class declaration | `include/gui_forms/control/control/control.hpp:677` |
| `Control` | `shared_ptr` | `const detail::DisplayChunk` | class declaration | `include/gui_forms/control/control/control.hpp:701` |
| `Control` | `shared_ptr` | `const Theme` | class declaration | `include/gui_forms/control/control/control.hpp:716` |
| `Control` | `unique_ptr` | `ControlBindingsCollection` | class declaration | `include/gui_forms/control/control/control.hpp:737` |
| `<file/function>` | `shared_ptr` | `ControlType` | local/signature/use | `include/gui_forms/control/control/control.hpp:751` |
| `ControlFactory` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/control/static_tree/control_factory/control_factory.hpp:23` |
| `ControlFactory` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/control/static_tree/control_factory/control_factory.hpp:26` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `include/gui_forms/control/static_tree/control_factory/control_factory.hpp:32` |
| `ButtonBase` | `shared_ptr` | `ImageList` | local/signature/use | `include/gui_forms/controls/button_base/button_base.hpp:59` |
| `ButtonBase` | `shared_ptr` | `ImageList` | class declaration | `include/gui_forms/controls/button_base/button_base.hpp:62` |
| `ButtonBase` | `shared_ptr` | `ImageList` | class declaration | `include/gui_forms/controls/button_base/button_base.hpp:150` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:47` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:48` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:49` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:49` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:50` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:50` |
| `MasterDetailView` | `shared_ptr` | `SplitContainer` | local/signature/use | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:51` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:86` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:86` |
| `MasterDetailView` | `shared_ptr` | `SplitterPanel` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:87` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:88` |
| `MasterDetailView` | `shared_ptr` | `SplitContainer` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:94` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:95` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:96` |
| `MenuStrip` | `unique_ptr` | `ContextMenu` | class declaration | `include/gui_forms/controls/menu_strip/menu_strip.hpp:93` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:24` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:27` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:28` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:29` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:30` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:57` |
| `AnchoredPopupLayer` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:59` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:60` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:36` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:37` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:38` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:39` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:39` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:40` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:40` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:41` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:41` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:80` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:80` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:80` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:83` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:84` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:85` |
| `ReviewCard` | `shared_ptr` | `Label` | local/signature/use | `include/gui_forms/controls/panel/card/review_card/review_card.hpp:45` |
| `ReviewCard` | `shared_ptr` | `Label` | local/signature/use | `include/gui_forms/controls/panel/card/review_card/review_card.hpp:48` |
| `ReviewCard` | `shared_ptr` | `Label` | local/signature/use | `include/gui_forms/controls/panel/card/review_card/review_card.hpp:51` |
| `ReviewCard` | `shared_ptr` | `Label` | class declaration | `include/gui_forms/controls/panel/card/review_card/review_card.hpp:64` |
| `ReviewCard` | `shared_ptr` | `Label` | class declaration | `include/gui_forms/controls/panel/card/review_card/review_card.hpp:65` |
| `ReviewCard` | `shared_ptr` | `Label` | class declaration | `include/gui_forms/controls/panel/card/review_card/review_card.hpp:66` |
| `ColorValueEditor` | `shared_ptr` | `TextBox` | local/signature/use | `include/gui_forms/controls/panel/color_value_editor/color_value_editor.hpp:19` |
| `ColorValueEditor` | `shared_ptr` | `TextBox` | class declaration | `include/gui_forms/controls/panel/color_value_editor/color_value_editor.hpp:46` |
| `ComboBox` | `shared_ptr` | `Panel` | class declaration | `include/gui_forms/controls/panel/combo_box/combo_box.hpp:83` |
| `ComboBox` | `shared_ptr` | `ListBox` | class declaration | `include/gui_forms/controls/panel/combo_box/combo_box.hpp:84` |
| `DateTimePicker` | `shared_ptr` | `Panel` | class declaration | `include/gui_forms/controls/panel/date_time_picker/date_time_picker.hpp:116` |
| `DateTimePicker` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/date_time_picker/date_time_picker.hpp:117` |
| `FlagsValueEditor` | `shared_ptr` | `Panel` | class declaration | `include/gui_forms/controls/panel/flags_value_editor/flags_value_editor.hpp:55` |
| `FlagsValueEditor` | `shared_ptr` | `CheckedListBox` | class declaration | `include/gui_forms/controls/panel/flags_value_editor/flags_value_editor.hpp:56` |
| `ScaledGroupBox` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/group_box/scaled_group_box/scaled_group_box.hpp:18` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/instrument_rack/instrument_rack.hpp:21` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/instrument_rack/instrument_rack.hpp:37` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/instrument_rack/instrument_rack.hpp:39` |
| `InstrumentRack` | `unique_ptr` | `Impl` | class declaration | `include/gui_forms/controls/panel/instrument_rack/instrument_rack.hpp:79` |
| `NumericUpDown` | `shared_ptr` | `TextBox` | local/signature/use | `include/gui_forms/controls/panel/numeric_up_down/numeric_up_down.hpp:32` |
| `NumericUpDown` | `shared_ptr` | `TextBox` | class declaration | `include/gui_forms/controls/panel/numeric_up_down/numeric_up_down.hpp:48` |
| `NumericUpDown` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/numeric_up_down/numeric_up_down.hpp:49` |
| `ObjectView` | `shared_ptr` | `ImageList` | local/signature/use | `include/gui_forms/controls/panel/object_view/object_view.hpp:106` |
| `ObjectView` | `shared_ptr` | `ImageList` | class declaration | `include/gui_forms/controls/panel/object_view/object_view.hpp:109` |
| `ObjectView` | `shared_ptr` | `ImageList` | class declaration | `include/gui_forms/controls/panel/object_view/object_view.hpp:178` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:20` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:21` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:22` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:23` |
| `PropertyGrid` | `shared_ptr` | `PropertyList` | local/signature/use | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:28` |
| `PropertyGrid` | `shared_ptr` | `PropertyValueConverterRegistry` | local/signature/use | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:29` |
| `PropertyGrid` | `shared_ptr` | `PropertyValueConverterRegistry` | class declaration | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:32` |
| `PropertyGrid` | `shared_ptr` | `PropertyEditorRegistry` | local/signature/use | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:33` |
| `PropertyGrid` | `shared_ptr` | `PropertyEditorRegistry` | class declaration | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:35` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:36` |
| `PropertyGrid` | `shared_ptr` | `Button` | local/signature/use | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:37` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:61` |
| `PropertyGrid` | `unique_ptr` | `Impl` | class declaration | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:79` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:80` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_list/property_list.hpp:30` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_list/property_list.hpp:31` |
| `PropertyList` | `shared_ptr` | `Button` | local/signature/use | `include/gui_forms/controls/panel/property_list/property_list.hpp:32` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_list/property_list.hpp:34` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_list/property_list.hpp:35` |
| `PropertyList` | `unique_ptr` | `Impl` | class declaration | `include/gui_forms/controls/panel/property_list/property_list.hpp:76` |
| `ScaledPanel` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/scaled_panel/scaled_panel.hpp:16` |
| `TreeView` | `shared_ptr` | `ImageList` | local/signature/use | `include/gui_forms/controls/panel/tree_view/tree_view.hpp:67` |
| `TreeView` | `shared_ptr` | `ImageList` | class declaration | `include/gui_forms/controls/panel/tree_view/tree_view.hpp:70` |
| `TreeView` | `shared_ptr` | `ImageList` | class declaration | `include/gui_forms/controls/panel/tree_view/tree_view.hpp:124` |
| `RasterCanvas` | `shared_ptr` | `gui_drawing::Bitmap` | local/signature/use | `include/gui_forms/controls/raster_canvas/raster_canvas.hpp:19` |
| `RasterCanvas` | `shared_ptr` | `gui_drawing::Bitmap` | class declaration | `include/gui_forms/controls/raster_canvas/raster_canvas.hpp:22` |
| `RasterCanvas` | `shared_ptr` | `gui_drawing::Bitmap` | class declaration | `include/gui_forms/controls/raster_canvas/raster_canvas.hpp:74` |
| `ContainerControl` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/container_control.hpp:11` |
| `ContainerControl` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/container_control.hpp:12` |
| `ContainerControl` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/container_control.hpp:13` |
| `SplitContainer` | `shared_ptr` | `SplitterPanel` | local/signature/use | `include/gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp:47` |
| `SplitContainer` | `shared_ptr` | `SplitterPanel` | local/signature/use | `include/gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp:50` |
| `SplitContainer` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp:53` |
| `SplitContainer` | `shared_ptr` | `SplitterPanel` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp:142` |
| `SplitContainer` | `shared_ptr` | `SplitterPanel` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp:145` |
| `SplitContainer` | `shared_ptr` | `SplitterPanel` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp:146` |
| `SplitContainer` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp:147` |
| `TabControl` | `shared_ptr` | `TabPage` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:38` |
| `TabControl` | `shared_ptr` | `TabPage` | local/signature/use | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:39` |
| `TabControl` | `shared_ptr` | `TabPage` | local/signature/use | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:40` |
| `TabControl` | `shared_ptr` | `TabPage` | local/signature/use | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:42` |
| `TabControl` | `shared_ptr` | `TabPage` | local/signature/use | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:45` |
| `TabControl` | `shared_ptr` | `TabPage` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:49` |
| `TabControl` | `shared_ptr` | `TabPage` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:78` |
| `TabControl` | `shared_ptr` | `TabPage` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:80` |
| `TabControl` | `shared_ptr` | `TabPage` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:81` |
| `TabControl` | `weak_ptr` | `TabPage` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:85` |
| `TabControl` | `weak_ptr` | `TabPage` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:86` |
| `TabControl` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:87` |
| `TableLayoutPanel` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.hpp:88` |
| `ScrollableControl` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/scrollable_control/scrollable_control.hpp:84` |
| `DispatchOperation` | `shared_ptr` | `detail::DispatchWork` | local/signature/use | `include/gui_forms/dispatcher/operation/dispatch_operation.hpp:33` |
| `DispatchOperation` | `shared_ptr` | `detail::DispatchWork` | class declaration | `include/gui_forms/dispatcher/operation/dispatch_operation.hpp:46` |
| `Bitmap` | `unique_ptr` | `Bitmap` | local/signature/use | `include/gui_forms/drawing/bitmap/bitmap.hpp:66` |
| `Bitmap` | `unique_ptr` | `Bitmap` | local/signature/use | `include/gui_forms/drawing/bitmap/bitmap.hpp:67` |
| `Bitmap` | `unique_ptr` | `Bitmap` | local/signature/use | `include/gui_forms/drawing/bitmap/bitmap.hpp:69` |
| `Bitmap` | `shared_ptr` | `PixelStorage` | class declaration | `include/gui_forms/drawing/bitmap/bitmap.hpp:98` |
| `GraphicsPath` | `unique_ptr` | `GraphicsPath` | local/signature/use | `include/gui_forms/drawing/graphics_path/graphics_path.hpp:63` |
| `ImageAttributes` | `unique_ptr` | `ImageAttributes` | local/signature/use | `include/gui_forms/drawing/image_attributes/image_attributes.hpp:31` |
| `ImageSnapshot` | `shared_ptr` | `const PixelStorage` | class declaration | `include/gui_forms/drawing/image_snapshot/image_snapshot.hpp:27` |
| `TextureBrush` | `unique_ptr` | `TextureBrush` | local/signature/use | `include/gui_forms/drawing/texture_brush/texture_brush.hpp:24` |
| `SubscriptionToken` | `shared_ptr` | `detail::Revocable` | local/signature/use | `include/gui_forms/event/event/event.hpp:43` |
| `SubscriptionToken` | `shared_ptr` | `detail::Revocable` | class declaration | `include/gui_forms/event/event/event.hpp:46` |
| `Slot` | `weak_ptr` | `State` | local/signature/use | `include/gui_forms/event/event/event.hpp:106` |
| `Slot` | `weak_ptr` | `State` | class declaration | `include/gui_forms/event/event/event.hpp:122` |
| `State` | `shared_ptr` | `Slot` | class declaration | `include/gui_forms/event/event/event.hpp:128` |
| `Event` | `shared_ptr` | `State` | class declaration | `include/gui_forms/event/event/event.hpp:152` |
| `ImageList` | `weak_ptr` | `detail::WindowLifetime` | class declaration | `include/gui_forms/image_list/image_list/image_list.hpp:147` |
| `PropertyEditorBinding` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/inspection/property_editor_registry/property_editor_registry.hpp:15` |
| `PropertyEditorRegistry` | `shared_ptr` | `PropertyEditorRegistry` | local/signature/use | `include/gui_forms/inspection/property_editor_registry/property_editor_registry.hpp:37` |
| `PropertyValueConverterRegistry` | `shared_ptr` | `PropertyValueConverterRegistry` | local/signature/use | `include/gui_forms/inspection/property_value_converter_registry/property_value_converter_registry.hpp:60` |
| `LiveSurfaceFrame` | `shared_ptr` | `const detail::LiveSurfaceBuffer` | class declaration | `include/gui_forms/live_surface/frame/live_surface_frame.hpp:45` |
| `LiveSurfaceFrame` | `shared_ptr` | `const detail::LiveSurfaceBuffer` | class declaration | `include/gui_forms/live_surface/frame/live_surface_frame.hpp:49` |
| `LiveSurface` | `shared_ptr` | `LiveSurface` | local/signature/use | `include/gui_forms/live_surface/surface/live_surface.hpp:22` |
| `LiveSurface` | `shared_ptr` | `detail::LiveSurfaceState` | class declaration | `include/gui_forms/live_surface/surface/live_surface.hpp:38` |
| `LiveSurface` | `shared_ptr` | `detail::LiveSurfaceState` | class declaration | `include/gui_forms/live_surface/surface/live_surface.hpp:39` |
| `LiveSurfaceWakeConnection` | `weak_ptr` | `detail::LiveSurfaceState` | class declaration | `include/gui_forms/live_surface/wake_connection/live_surface_wake_connection.hpp:32` |
| `LiveSurfaceWakeConnection` | `shared_ptr` | `detail::LiveSurfaceWake` | class declaration | `include/gui_forms/live_surface/wake_connection/live_surface_wake_connection.hpp:33` |
| `LiveSurfaceWakeConnection` | `weak_ptr` | `detail::LiveSurfaceState` | class declaration | `include/gui_forms/live_surface/wake_connection/live_surface_wake_connection.hpp:35` |
| `LiveSurfaceWakeConnection` | `shared_ptr` | `detail::LiveSurfaceWake` | class declaration | `include/gui_forms/live_surface/wake_connection/live_surface_wake_connection.hpp:36` |
| `LiveSurfaceWriteLease` | `shared_ptr` | `detail::LiveSurfaceState` | class declaration | `include/gui_forms/live_surface/write_lease/live_surface_write_lease.hpp:43` |
| `LiveSurfaceWriteLease` | `shared_ptr` | `detail::LiveSurfaceBuffer` | class declaration | `include/gui_forms/live_surface/write_lease/live_surface_write_lease.hpp:44` |
| `LiveSurfaceWriteLease` | `shared_ptr` | `detail::LiveSurfaceState` | class declaration | `include/gui_forms/live_surface/write_lease/live_surface_write_lease.hpp:47` |
| `LiveSurfaceWriteLease` | `shared_ptr` | `detail::LiveSurfaceBuffer` | class declaration | `include/gui_forms/live_surface/write_lease/live_surface_write_lease.hpp:48` |
| `MacApplicationWindow` | `unique_ptr` | `Window` | class declaration | `include/gui_forms/platform/macos_host.hpp:48` |
| `<file/function>` | `unique_ptr` | `HostServices` | local/signature/use | `include/gui_forms/platform/macos_host.hpp:54` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `include/gui_forms/platform/macos_host.hpp:56` |
| `WindowsCompatibilityPaintEndpoint` | `shared_ptr` | `WindowsCompatibilityPaintEndpoint` | local/signature/use | `include/gui_forms/platform/windows_compatibility_paint_endpoint/windows_compatibility_paint_endpoint.hpp:19` |
| `WindowsCompatibilityPaintEndpoint` | `shared_ptr` | `LiveSurface` | local/signature/use | `include/gui_forms/platform/windows_compatibility_paint_endpoint/windows_compatibility_paint_endpoint.hpp:31` |
| `WindowsCompatibilityPaintEndpoint` | `unique_ptr` | `Implementation` | class declaration | `include/gui_forms/platform/windows_compatibility_paint_endpoint/windows_compatibility_paint_endpoint.hpp:51` |
| `WindowsCompatibilityPaintEndpoint` | `unique_ptr` | `Implementation` | class declaration | `include/gui_forms/platform/windows_compatibility_paint_endpoint/windows_compatibility_paint_endpoint.hpp:53` |
| `WindowsHostOptions` | `shared_ptr` | `Control` | local/signature/use | `include/gui_forms/platform/windows_host.hpp:28` |
| `WindowsApplicationWindow` | `unique_ptr` | `Window` | class declaration | `include/gui_forms/platform/windows_host.hpp:46` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `include/gui_forms/platform/windows_host.hpp:52` |
| `FrameRequestToken` | `shared_ptr` | `detail::Revocable` | class declaration | `include/gui_forms/scheduler/frame_request_token/frame_request_token.hpp:25` |
| `FrameRequestToken` | `shared_ptr` | `detail::Revocable` | class declaration | `include/gui_forms/scheduler/frame_request_token/frame_request_token.hpp:27` |
| `Theme` | `shared_ptr` | `const Theme` | local/signature/use | `include/gui_forms/theme/theme/theme.hpp:13` |
| `<file/function>` | `shared_ptr` | `const Theme` | local/signature/use | `include/gui_forms/theme/theme/theme.hpp:35` |
| `Timer` | `weak_ptr` | `detail::WindowLifetime` | class declaration | `include/gui_forms/timer/timer/timer.hpp:49` |
| `Timer` | `shared_ptr` | `CallbackState` | class declaration | `include/gui_forms/timer/timer/timer.hpp:51` |
| `Painter` | `shared_ptr` | `LiveSurface` | class declaration | `include/gui_forms/types/painter/painter.hpp:70` |
| `LiveSurfacePresentation` | `shared_ptr` | `LiveSurface` | class declaration | `include/gui_forms/window/presentation/presentation_types.hpp:27` |
| `PopupToken` | `shared_ptr` | `detail::PopupAttachment` | local/signature/use | `include/gui_forms/window/window.hpp:68` |
| `PopupToken` | `shared_ptr` | `detail::PopupAttachment` | class declaration | `include/gui_forms/window/window.hpp:70` |
| `AcceleratorToken` | `shared_ptr` | `detail::AcceleratorAttachment` | local/signature/use | `include/gui_forms/window/window.hpp:129` |
| `AcceleratorToken` | `shared_ptr` | `detail::AcceleratorAttachment` | class declaration | `include/gui_forms/window/window.hpp:131` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:217` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:222` |
| `Window` | `shared_ptr` | `const Theme` | local/signature/use | `include/gui_forms/window/window.hpp:236` |
| `Window` | `shared_ptr` | `const Theme` | class declaration | `include/gui_forms/window/window.hpp:239` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:275` |
| `Window` | `shared_ptr` | `LiveSurface` | class declaration | `include/gui_forms/window/window.hpp:275` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:289` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:291` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:306` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:310` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:355` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:356` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:357` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:358` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:359` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:363` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:367` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:368` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:374` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:376` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:377` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:378` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:381` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:395` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:397` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:408` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:409` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:417` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:455` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:455` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:456` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:457` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:458` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:460` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:463` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:464` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:465` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:471` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:472` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:473` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:475` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:475` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:477` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:480` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:481` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:483` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:484` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:509` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:513` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:513` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:514` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:515` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:517` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:517` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:519` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:529` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:533` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:538` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:540` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:541` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:542` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:543` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:545` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:554` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:555` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:557` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:560` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:563` |
| `Window` | `shared_ptr` | `const Theme` | class declaration | `include/gui_forms/window/window.hpp:568` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:572` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:573` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:574` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:575` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:586` |
| `FocusScopeState` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:599` |
| `FocusScopeState` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:600` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:609` |
| `Window` | `shared_ptr` | `detail::PopupAttachment` | class declaration | `include/gui_forms/window/window.hpp:613` |
| `Window` | `shared_ptr` | `detail::AcceleratorAttachment` | class declaration | `include/gui_forms/window/window.hpp:614` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:615` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:616` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:617` |
| `LiveSurfaceRegistration` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:622` |
| `LiveSurfaceRegistration` | `shared_ptr` | `LiveSurface` | class declaration | `include/gui_forms/window/window.hpp:623` |
| `Window` | `shared_ptr` | `detail::ScheduledFrameRequest` | class declaration | `include/gui_forms/window/window.hpp:633` |
| `Window` | `shared_ptr` | `detail::WindowLifetime` | class declaration | `include/gui_forms/window/window.hpp:635` |
| `Window` | `shared_ptr` | `detail::DispatcherState` | class declaration | `include/gui_forms/window/window.hpp:637` |
| `AbiPropertyObjectControl` | `weak_ptr` | `PropertyState` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:120` |
| `AbiPropertyObjectControl` | `weak_ptr` | `PropertyState` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:131` |
| `AbiPropertyObjectControl` | `weak_ptr` | `PropertyState` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:158` |
| `AbiPropertyObjectControl` | `weak_ptr` | `gui_forms::Button` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:173` |
| `AbiPropertyObjectControl` | `weak_ptr` | `gui_forms::Button` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:201` |
| `AbiPropertyObjectControl` | `shared_ptr` | `PropertyState` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:607` |
| `RasterControl` | `shared_ptr` | `gui_forms::LiveSurface` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:120` |
| `RasterControl` | `shared_ptr` | `gui_forms::LiveSurface` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:129` |
| `RasterControl` | `weak_ptr` | `RasterControl` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:241` |
| `RasterControl` | `weak_ptr` | `LiveWakeState` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:242` |
| `RasterControl` | `weak_ptr` | `RasterControl` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:298` |
| `RasterControl` | `weak_ptr` | `LiveWakeState` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:301` |
| `RasterControl` | `shared_ptr` | `gui_forms::LiveSurface` | class declaration | `src/abi/control_adapters/raster_control/raster_control.hpp:322` |
| `RasterControl` | `shared_ptr` | `LiveWakeState` | class declaration | `src/abi/control_adapters/raster_control/raster_control.hpp:325` |
| `ObjectRecord` | `shared_ptr` | `gui_drawing::DrawingObject` | class declaration | `src/abi/drawing_c_api.cpp:92` |
| `Slot` | `shared_ptr` | `ObjectRecord` | class declaration | `src/abi/drawing_c_api.cpp:104` |
| `Registry` | `unique_ptr` | `Object` | local/signature/use | `src/abi/drawing_c_api.cpp:127` |
| `Registry` | `shared_ptr` | `Object` | local/signature/use | `src/abi/drawing_c_api.cpp:134` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:185` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:199` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:211` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:221` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:245` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:249` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:283` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:284` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:285` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:286` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:322` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:351` |
| `<file/function>` | `unique_ptr` | `gui_drawing::Bitmap` | local/signature/use | `src/abi/drawing_c_api.cpp:1558` |
| `<file/function>` | `unique_ptr` | `gui_drawing::Bitmap` | local/signature/use | `src/abi/drawing_c_api.cpp:1640` |
| `CapturedSurface` | `unique_ptr` | `Bitmap` | class declaration | `src/abi/drawing_platform.hpp:13` |
| `<file/function>` | `unique_ptr` | `Bitmap` | local/signature/use | `src/abi/drawing_platform.hpp:18` |
| `<file/function>` | `unique_ptr` | `Bitmap` | local/signature/use | `src/abi/drawing_platform_stub.cpp:9` |
| `<file/function>` | `unique_ptr` | `Bitmap` | local/signature/use | `src/abi/drawing_platform_windows.cpp:183` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:19` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:28` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:35` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:38` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:49` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:57` |
| `<file/function>` | `weak_ptr` | `RasterControl` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:74` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:204` |
| `CompatibilityPaintBinding` | `weak_ptr` | `RasterControl` | class declaration | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.hpp:17` |
| `CompatibilityPaintBinding` | `shared_ptr` | `gui_forms::LiveSurface` | class declaration | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.hpp:19` |
| `CompatibilityPaintWrite` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | class declaration | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.hpp:23` |
| `ControlRecord` | `shared_ptr` | `Control` | class declaration | `src/abi/registry/registry.hpp:43` |
| `ControlRecord` | `weak_ptr` | `Control` | class declaration | `src/abi/registry/registry.hpp:45` |
| `RegistrySlot` | `shared_ptr` | `ControlRecord` | class declaration | `src/abi/registry/registry.hpp:78` |
| `RegistrySlot` | `shared_ptr` | `SubscriptionRecord` | class declaration | `src/abi/registry/registry.hpp:79` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:217` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:246` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:266` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:279` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:292` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:309` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:326` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:339` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:359` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:379` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:398` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:417` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:488` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:511` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:512` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:538` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:551` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:560` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:573` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:592` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:603` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:606` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:617` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:624` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:657` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:679` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:719` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:749` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:779` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:800` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:824` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:842` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:866` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:897` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:916` |
| `Registry` | `weak_ptr` | `RasterControl` | local/signature/use | `src/abi/registry/registry.hpp:936` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:942` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:963` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:964` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:984` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1033` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1065` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1087` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1112` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1135` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1157` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1181` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1219` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1260` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1289` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1312` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1330` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1348` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1378` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1399` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1418` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1434` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1453` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1472` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1485` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1525` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1526` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1602` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1637` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1668` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1690` |
| `Registry` | `unique_ptr` | `ControlRecord, std::function<void(ControlRecord*)>` | local/signature/use | `src/abi/registry/registry.hpp:1710` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:1745` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:1848` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1906` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1924` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1941` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1959` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1960` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2003` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2022` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2042` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2055` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2070` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2084` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2098` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2117` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2118` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2135` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2136` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2143` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2153` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2154` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2176` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2182` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2189` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2200` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2206` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2222` |
| `Registry` | `shared_ptr` | `SubscriptionRecord` | local/signature/use | `src/abi/registry/registry.hpp:2240` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2270` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2353` |
| `Registry` | `weak_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2382` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2416` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2451` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2496` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2533` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2567` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2593` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2604` |
| `Registry` | `shared_ptr` | `ControlRecord` | class declaration | `src/abi/registry/registry.hpp:2605` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2606` |
| `Registry` | `shared_ptr` | `SubscriptionRecord` | local/signature/use | `src/abi/registry/registry.hpp:2618` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2619` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2622` |
| `Registry` | `unique_ptr` | `ControlRecord, std::function<void(ControlRecord*)>` | local/signature/use | `src/abi/registry/registry.hpp:2642` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2675` |
| `Registry` | `unique_ptr` | `ControlRecord, std::function<void(ControlRecord*)>` | local/signature/use | `src/abi/registry/registry.hpp:2684` |
| `Registry` | `shared_ptr` | `ControlRecord` | class declaration | `src/abi/registry/registry.hpp:2717` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2732` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2769` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2778` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2783` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2848` |
| `Registry` | `shared_ptr` | `gui_forms::ButtonBase` | local/signature/use | `src/abi/registry/registry.hpp:2865` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2866` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2881` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2882` |
| `Registry` | `shared_ptr` | `std::unordered_map<std::string, std::shared_ptr<Control>>` | local/signature/use | `src/abi/registry/registry.hpp:2895` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2895` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2896` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2898` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2904` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2905` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2918` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2919` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2939` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2947` |
| `Registry` | `shared_ptr` | `SubscriptionRecord` | local/signature/use | `src/abi/registry/registry.hpp:2963` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2988` |
| `Registry` | `shared_ptr` | `SubscriptionRecord` | local/signature/use | `src/abi/registry/registry.hpp:2989` |
| `Registry` | `shared_ptr` | `ControlRecord` | class declaration | `src/abi/registry/registry.hpp:3032` |
| `Registry` | `shared_ptr` | `SubscriptionRecord` | local/signature/use | `src/abi/registry/registry.hpp:3033` |
| `<file/function>` | `shared_ptr` | `const PropertyEnumDescriptor` | local/signature/use | `src/controls/basic/basic_control_rendering.cpp:11` |
| `<file/function>` | `shared_ptr` | `const PropertyEnumDescriptor` | local/signature/use | `src/controls/basic/basic_control_rendering.hpp:18` |
| `<file/function>` | `shared_ptr` | `ImageList` | local/signature/use | `src/controls/button_base/button_base.cpp:146` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/button_base/radio_button/radio_button.cpp:51` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/button_base/radio_button/radio_button.cpp:52` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/button_base/radio_button/radio_button.cpp:75` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/button_base/radio_button/radio_button.cpp:76` |
| `<file/function>` | `shared_ptr` | `Command` | local/signature/use | `src/controls/commands/command_binding/command_binding.cpp:8` |
| `<file/function>` | `shared_ptr` | `ButtonBase` | local/signature/use | `src/controls/commands/command_binding/command_binding.cpp:9` |
| `<file/function>` | `weak_ptr` | `Command` | local/signature/use | `src/controls/commands/command_binding/command_binding.cpp:17` |
| `<file/function>` | `weak_ptr` | `ButtonBase` | local/signature/use | `src/controls/commands/command_binding/command_binding.cpp:24` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:52` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:53` |
| `<file/function>` | `shared_ptr` | `SplitterPanel` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:53` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:54` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:72` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:84` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:84` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:88` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:88` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:237` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:238` |
| `GalleryControl` | `shared_ptr` | `GalleryContext` | class declaration | `src/controls/gallery/control/gallery_control.hpp:16` |
| `GalleryControl` | `shared_ptr` | `GalleryContext` | class declaration | `src/controls/gallery/control/gallery_control.hpp:35` |
| `GalleryTree` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/gallery/tree/gallery_tree.hpp:11` |
| `GalleryTree` | `shared_ptr` | `GalleryContext` | class declaration | `src/controls/gallery/tree/gallery_tree.hpp:12` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:55` |
| `GalleryButton` | `shared_ptr` | `GalleryContext` | local/signature/use | `src/controls/gallery_controls.cpp:164` |
| `GalleryButton` | `shared_ptr` | `GalleryContext` | class declaration | `src/controls/gallery_controls.cpp:180` |
| `GalleryCheckBox` | `shared_ptr` | `GalleryContext` | local/signature/use | `src/controls/gallery_controls.cpp:187` |
| `GalleryCheckBox` | `shared_ptr` | `GalleryContext` | class declaration | `src/controls/gallery_controls.cpp:214` |
| `GalleryRadioButton` | `shared_ptr` | `GalleryContext` | local/signature/use | `src/controls/gallery_controls.cpp:221` |
| `GalleryRadioButton` | `shared_ptr` | `GalleryContext` | class declaration | `src/controls/gallery_controls.cpp:239` |
| `GalleryLinkLabel` | `shared_ptr` | `GalleryContext` | local/signature/use | `src/controls/gallery_controls.cpp:246` |
| `GalleryLinkLabel` | `shared_ptr` | `GalleryContext` | class declaration | `src/controls/gallery_controls.cpp:258` |
| `GalleryTrackBar` | `shared_ptr` | `GalleryContext` | local/signature/use | `src/controls/gallery_controls.cpp:265` |
| `GalleryTrackBar` | `shared_ptr` | `GalleryContext` | class declaration | `src/controls/gallery_controls.cpp:283` |
| `GalleryStatusLabel` | `shared_ptr` | `GalleryContext` | local/signature/use | `src/controls/gallery_controls.cpp:290` |
| `GalleryStatusLabel` | `shared_ptr` | `GalleryContext` | class declaration | `src/controls/gallery_controls.cpp:304` |
| `GalleryLifecycleCard` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:340` |
| `GalleryLifecycleCard` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:365` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:393` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:441` |
| `<file/function>` | `shared_ptr` | `GalleryContext` | local/signature/use | `src/controls/gallery_controls.cpp:498` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:548` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:582` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:611` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:905` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:909` |
| `ErrorProvider` | `weak_ptr` | `Control` | class declaration | `src/controls/guidance/error_provider/error_provider.cpp:24` |
| `ErrorProvider` | `shared_ptr` | `ErrorLayer` | class declaration | `src/controls/guidance/error_provider/error_provider.cpp:28` |
| `ErrorProvider` | `shared_ptr` | `ErrorGlyph` | class declaration | `src/controls/guidance/error_provider/error_provider.cpp:29` |
| `ErrorProvider` | `unique_ptr` | `PopupToken` | class declaration | `src/controls/guidance/error_provider/error_provider.cpp:30` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:92` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:111` |
| `<file/function>` | `weak_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:119` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:131` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:180` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:200` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:275` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:277` |
| `<file/function>` | `shared_ptr` | `BindingSource` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:280` |
| `<file/function>` | `shared_ptr` | `BindingSource` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:343` |
| `Aggregate` | `shared_ptr` | `Control` | class declaration | `src/controls/guidance/error_provider/error_provider.cpp:384` |
| `HelpProvider` | `weak_ptr` | `Control` | class declaration | `src/controls/guidance/help_provider/help_provider.cpp:20` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:69` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:88` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:122` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:141` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:160` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:177` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:224` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:250` |
| `<file/function>` | `shared_ptr` | `PropertyEditorRegistry` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:91` |
| `<file/function>` | `weak_ptr` | `NumericUpDown` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:111` |
| `<file/function>` | `weak_ptr` | `NumericUpDown` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:121` |
| `<file/function>` | `weak_ptr` | `FlagsValueEditor` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:163` |
| `<file/function>` | `weak_ptr` | `FlagsValueEditor` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:173` |
| `<file/function>` | `weak_ptr` | `ColorValueEditor` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:210` |
| `<file/function>` | `weak_ptr` | `ColorValueEditor` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:220` |
| `<file/function>` | `weak_ptr` | `ColorValueEditor` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:236` |
| `<file/function>` | `shared_ptr` | `PropertyValueConverterRegistry` | local/signature/use | `src/controls/inspection/property_value_converter_registry/property_value_converter_registry.cpp:127` |
| `PanelState` | `shared_ptr` | `MenuPanel` | class declaration | `src/controls/menu/context_menu/context_menu.cpp:27` |
| `PanelState` | `shared_ptr` | `MenuRow` | class declaration | `src/controls/menu/context_menu/context_menu.cpp:28` |
| `ContextMenu` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/menu/context_menu/context_menu.cpp:332` |
| `ContextMenu` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/menu/context_menu/context_menu.cpp:355` |
| `ContextMenu` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/menu/context_menu/context_menu.cpp:477` |
| `ContextMenu` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/menu/context_menu/context_menu.cpp:558` |
| `ContextMenu` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/menu/context_menu/context_menu.cpp:653` |
| `ContextMenu` | `shared_ptr` | `MenuLayer` | class declaration | `src/controls/menu/context_menu/context_menu.cpp:654` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/menu/context_menu/context_menu.cpp:700` |
| `MenuSnapshot` | `shared_ptr` | `Command` | class declaration | `src/controls/menu/menu_utilities.hpp:25` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/anchored_popup_layer/anchored_popup_layer.cpp:15` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/anchored_popup_layer/anchored_popup_layer.cpp:24` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/anchored_popup_layer/anchored_popup_layer.cpp:30` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/anchored_popup_layer/anchored_popup_layer.cpp:39` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/anchored_popup_layer/anchored_popup_layer.cpp:94` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:36` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:36` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:37` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:47` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:56` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:56` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:60` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:60` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:64` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:64` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:137` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:138` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:139` |
| `<file/function>` | `weak_ptr` | `ColorValueEditor` | local/signature/use | `src/controls/panel/color_value_editor/color_value_editor.cpp:24` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/combo_box/combo_box.cpp:227` |
| `<file/function>` | `weak_ptr` | `ComboBox` | local/signature/use | `src/controls/panel/combo_box/combo_box.cpp:260` |
| `<file/function>` | `weak_ptr` | `CorrespondenceView` | local/signature/use | `src/controls/panel/correspondence_view/correspondence_view.cpp:422` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/date_time_picker/date_time_picker.cpp:291` |
| `<file/function>` | `weak_ptr` | `DateTimePicker` | local/signature/use | `src/controls/panel/date_time_picker/date_time_picker.cpp:317` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:113` |
| `<file/function>` | `weak_ptr` | `FlagsValueEditor` | local/signature/use | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:152` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/group_box/scaled_group_box/scaled_group_box.cpp:30` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/group_box/scaled_group_box/scaled_group_box.cpp:63` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:70` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:70` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:74` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:75` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:80` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:80` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:83` |
| `FieldState` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:95` |
| `ModuleState` | `shared_ptr` | `RackModulePanel` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:106` |
| `ModuleState` | `shared_ptr` | `CheckBox` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:107` |
| `ModuleState` | `shared_ptr` | `Label` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:109` |
| `ModuleState` | `shared_ptr` | `Button` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:110` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:173` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:182` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:186` |
| `InstrumentRack` | `weak_ptr` | `ComboBox` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:221` |
| `InstrumentRack` | `weak_ptr` | `TextBox` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:304` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:452` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:488` |
| `Slot` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:498` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:577` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:641` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:691` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:703` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:811` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:827` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:839` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:932` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:932` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/rack_module_panel/rack_module_panel.cpp:46` |
| `RackModulePanel` | `shared_ptr` | `CheckBox` | class declaration | `src/controls/panel/instrument_rack/rack_module_panel/rack_module_panel.hpp:18` |
| `RackModulePanel` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/instrument_rack/rack_module_panel/rack_module_panel.hpp:19` |
| `RackModulePanel` | `shared_ptr` | `Label` | class declaration | `src/controls/panel/instrument_rack/rack_module_panel/rack_module_panel.hpp:21` |
| `RackModulePanel` | `shared_ptr` | `Button` | class declaration | `src/controls/panel/instrument_rack/rack_module_panel/rack_module_panel.hpp:22` |
| `<file/function>` | `weak_ptr` | `NumericUpDown` | local/signature/use | `src/controls/panel/numeric_up_down/numeric_up_down.cpp:46` |
| `<file/function>` | `shared_ptr` | `ImageList` | local/signature/use | `src/controls/panel/object_view/object_view.cpp:270` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:325` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:327` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:328` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:331` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:332` |
| `PropertyGrid` | `qualified-alias:weak_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:334` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:335` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:363` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:500` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:512` |
| `PropertyGrid` | `qualified-alias:weak_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:632` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:637` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:652` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:677` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:756` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:757` |
| `OwnerEdit` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:783` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:792` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:931` |
| `PropertyGrid` | `shared_ptr` | `PropertyList` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:1053` |
| `PropertyGrid` | `shared_ptr` | `PropertyValueConverterRegistry` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:1054` |
| `PropertyGrid` | `shared_ptr` | `PropertyEditorRegistry` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:1055` |
| `PropertyGrid` | `qualified-alias:weak_ptr` | `Control` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:1056` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1119` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1123` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1124` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1125` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1128` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1132` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1136` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1143` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1150` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1155` |
| `<file/function>` | `shared_ptr` | `PropertyList` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1180` |
| `<file/function>` | `shared_ptr` | `PropertyValueConverterRegistry` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1184` |
| `<file/function>` | `shared_ptr` | `PropertyValueConverterRegistry` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1190` |
| `<file/function>` | `shared_ptr` | `PropertyEditorRegistry` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1202` |
| `<file/function>` | `shared_ptr` | `PropertyEditorRegistry` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1208` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1220` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1225` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1228` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1234` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1313` |
| `RowState` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/property_list/property_list.cpp:29` |
| `RowState` | `shared_ptr` | `Button` | class declaration | `src/controls/panel/property_list/property_list.cpp:30` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:86` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:90` |
| `PropertyList` | `weak_ptr` | `ComboBox` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:212` |
| `PropertyList` | `weak_ptr` | `CheckBox` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:230` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/property_list/property_list.cpp:429` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:665` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:667` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:670` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:691` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:704` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:714` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:716` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:719` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:728` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:740` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/scaled_panel/scaled_panel.cpp:28` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/scaled_panel/scaled_panel.cpp:61` |
| `<file/function>` | `shared_ptr` | `ImageList` | local/signature/use | `src/controls/panel/tree_view/tree_view.cpp:145` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:18` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:22` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:31` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:36` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:37` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:40` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:62` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:85` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/container_layout_utilities.hpp:19` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/flow_layout_panel/flow_layout_panel.cpp:73` |
| `Item` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/scrollable_control/container_control/flow_layout_panel/flow_layout_panel.cpp:97` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/flow_layout_panel/flow_layout_panel.cpp:115` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/flow_layout_panel/flow_layout_panel.cpp:116` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/scaled_layout_utilities.hpp:34` |
| `<file/function>` | `shared_ptr` | `SplitterPanel` | local/signature/use | `src/controls/scrollable_control/container_control/split_container/split_container.cpp:547` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/split_container/split_container.cpp:549` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:33` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:34` |
| `<file/function>` | `weak_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:36` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:49` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:56` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:68` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:85` |
| `<file/function>` | `weak_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:97` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:101` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:123` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:125` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:127` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:135` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:140` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:142` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:162` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:163` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:181` |
| `<file/function>` | `weak_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:275` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_page/tab_page.cpp:25` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:250` |
| `Item` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:305` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:313` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:314` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:336` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:337` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:360` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:523` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:533` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:536` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/scrollable_control.cpp:156` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/scrollable_control.cpp:157` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/scrollable_control.cpp:283` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/scrollable_control.cpp:372` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/scrollable_control.cpp:402` |
| `ShowcaseContext` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/showcase_controls.cpp:63` |
| `ShowcaseContext` | `shared_ptr` | `Button` | class declaration | `src/controls/showcase_controls.cpp:64` |
| `ShowcaseContext` | `shared_ptr` | `Label` | class declaration | `src/controls/showcase_controls.cpp:66` |
| `ShowcaseContext` | `shared_ptr` | `EasingBoard` | class declaration | `src/controls/showcase_controls.cpp:67` |
| `ShowcaseContext` | `shared_ptr` | `Button` | class declaration | `src/controls/showcase_controls.cpp:68` |
| `ShowcaseContext` | `shared_ptr` | `ProgressBar` | class declaration | `src/controls/showcase_controls.cpp:69` |
| `ShowcaseContext` | `shared_ptr` | `Timer` | class declaration | `src/controls/showcase_controls.cpp:71` |
| `ShowcaseContext` | `shared_ptr` | `ToolTip` | class declaration | `src/controls/showcase_controls.cpp:72` |
| `ShowcaseContext` | `shared_ptr` | `ErrorProvider` | class declaration | `src/controls/showcase_controls.cpp:73` |
| `ShowcaseContext` | `shared_ptr` | `HelpProvider` | class declaration | `src/controls/showcase_controls.cpp:74` |
| `ShowcaseContext` | `shared_ptr` | `BindingSource` | class declaration | `src/controls/showcase_controls.cpp:75` |
| `ShowcaseContext` | `shared_ptr` | `Label` | class declaration | `src/controls/showcase_controls.cpp:76` |
| `ShowcaseContext` | `shared_ptr` | `ProgressBar` | class declaration | `src/controls/showcase_controls.cpp:77` |
| `ShowcaseContext` | `shared_ptr` | `TrackBar` | class declaration | `src/controls/showcase_controls.cpp:78` |
| `ShowcaseContext` | `shared_ptr` | `ScaledPanel` | class declaration | `src/controls/showcase_controls.cpp:79` |
| `ShowcaseContext` | `shared_ptr` | `Button` | class declaration | `src/controls/showcase_controls.cpp:80` |
| `ShowcaseContext` | `shared_ptr` | `Button` | class declaration | `src/controls/showcase_controls.cpp:81` |
| `ShowcaseContext` | `shared_ptr` | `Button` | class declaration | `src/controls/showcase_controls.cpp:82` |
| `ShowcaseContext` | `shared_ptr` | `Button` | class declaration | `src/controls/showcase_controls.cpp:83` |
| `ShowcaseContext` | `shared_ptr` | `Button` | class declaration | `src/controls/showcase_controls.cpp:84` |
| `ShowcaseContext` | `shared_ptr` | `TextBox` | class declaration | `src/controls/showcase_controls.cpp:85` |
| `ShowcaseContext` | `shared_ptr` | `Button` | class declaration | `src/controls/showcase_controls.cpp:86` |
| `ShowcaseContext` | `shared_ptr` | `TextBox` | class declaration | `src/controls/showcase_controls.cpp:87` |
| `ShowcaseContext` | `shared_ptr` | `CheckBox` | class declaration | `src/controls/showcase_controls.cpp:88` |
| `ShowcaseContext` | `shared_ptr` | `TrackBar` | class declaration | `src/controls/showcase_controls.cpp:89` |
| `ShowcaseContext` | `shared_ptr` | `Button` | class declaration | `src/controls/showcase_controls.cpp:90` |
| `ShowcaseContext` | `shared_ptr` | `Button` | class declaration | `src/controls/showcase_controls.cpp:91` |
| `ShowcaseContext` | `shared_ptr` | `Label` | class declaration | `src/controls/showcase_controls.cpp:92` |
| `ShowcaseContext` | `shared_ptr` | `Label` | class declaration | `src/controls/showcase_controls.cpp:93` |
| `ShowcaseContext` | `shared_ptr` | `Button` | class declaration | `src/controls/showcase_controls.cpp:94` |
| `ShowcaseContext` | `shared_ptr` | `Label` | class declaration | `src/controls/showcase_controls.cpp:99` |
| `ShowcaseContext` | `shared_ptr` | `TextBox` | class declaration | `src/controls/showcase_controls.cpp:100` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:181` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:191` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:222` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:223` |
| `<file/function>` | `weak_ptr` | `CheckBox` | local/signature/use | `src/controls/showcase_controls.cpp:337` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:361` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:362` |
| `<file/function>` | `shared_ptr` | `TrackBar` | local/signature/use | `src/controls/showcase_controls.cpp:372` |
| `<file/function>` | `weak_ptr` | `VScrollBar` | local/signature/use | `src/controls/showcase_controls.cpp:408` |
| `<file/function>` | `weak_ptr` | `HScrollBar` | local/signature/use | `src/controls/showcase_controls.cpp:416` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:469` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:470` |
| `<file/function>` | `weak_ptr` | `SplitContainer` | local/signature/use | `src/controls/showcase_controls.cpp:513` |
| `<file/function>` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:514` |
| `<file/function>` | `weak_ptr` | `ContainerControl` | local/signature/use | `src/controls/showcase_controls.cpp:559` |
| `<file/function>` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:560` |
| `<file/function>` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:591` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:601` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:602` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:636` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:747` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:748` |
| `<file/function>` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:817` |
| `<file/function>` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:818` |
| `<file/function>` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:819` |
| `<file/function>` | `weak_ptr` | `Panel` | local/signature/use | `src/controls/showcase_controls.cpp:835` |
| `<file/function>` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:836` |
| `<file/function>` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:837` |
| `<file/function>` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:838` |
| `<file/function>` | `weak_ptr` | `ScaledGroupBox` | local/signature/use | `src/controls/showcase_controls.cpp:910` |
| `<file/function>` | `weak_ptr` | `Panel` | local/signature/use | `src/controls/showcase_controls.cpp:911` |
| `<file/function>` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:912` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:936` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:937` |
| `<file/function>` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:966` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:991` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:992` |
| `<file/function>` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1076` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:1179` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1180` |
| `<file/function>` | `weak_ptr` | `TextBox` | local/signature/use | `src/controls/showcase_controls.cpp:1225` |
| `<file/function>` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1226` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:1251` |
| `<file/function>` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1286` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:1321` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1322` |
| `<file/function>` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1373` |
| `<file/function>` | `weak_ptr` | `ComboBox` | local/signature/use | `src/controls/showcase_controls.cpp:1374` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:1394` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1395` |
| `<file/function>` | `shared_ptr` | `NumericUpDown` | local/signature/use | `src/controls/showcase_controls.cpp:1407` |
| `<file/function>` | `weak_ptr` | `PropertyGrid` | local/signature/use | `src/controls/showcase_controls.cpp:1471` |
| `<file/function>` | `weak_ptr` | `NumericUpDown` | local/signature/use | `src/controls/showcase_controls.cpp:1472` |
| `<file/function>` | `weak_ptr` | `ComboBox` | local/signature/use | `src/controls/showcase_controls.cpp:1473` |
| `<file/function>` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1474` |
| `<file/function>` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1537` |
| `<file/function>` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1547` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:1591` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1592` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:1750` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1751` |
| `<file/function>` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1831` |
| `<file/function>` | `weak_ptr` | `TabControl` | local/signature/use | `src/controls/showcase_controls.cpp:1832` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:1886` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1887` |
| `<file/function>` | `weak_ptr` | `CheckedListBox` | local/signature/use | `src/controls/showcase_controls.cpp:1959` |
| `<file/function>` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1960` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:1996` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1997` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:2110` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:2111` |
| `<file/function>` | `weak_ptr` | `DateTimePicker` | local/signature/use | `src/controls/showcase_controls.cpp:2226` |
| `<file/function>` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2227` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:2276` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:2277` |
| `<file/function>` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:2646` |
| `<file/function>` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:2654` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:2673` |
| `<file/function>` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:2755` |
| `ShowcaseTree` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/showcase_controls.hpp:12` |
| `ToolTip` | `weak_ptr` | `Control` | class declaration | `src/controls/tool_tip/tool_tip.cpp:24` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:81` |
| `<file/function>` | `weak_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:108` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:197` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:209` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:218` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:222` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:260` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:264` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:277` |
| `<file/function>` | `weak_ptr` | `detail::WindowLifetime` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:308` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:358` |
| `ToolTipBubble` | `shared_ptr` | `Label` | class declaration | `src/controls/tool_tip/tool_tip_bubble/tool_tip_bubble.hpp:19` |
| `<file/function>` | `shared_ptr` | `BindingSource` | local/signature/use | `src/core/binding/binding/binding.cpp:24` |
| `<file/function>` | `weak_ptr` | `Binding` | local/signature/use | `src/core/binding/binding/binding.cpp:59` |
| `<file/function>` | `shared_ptr` | `BindingSource` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:32` |
| `<file/function>` | `shared_ptr` | `BindingSource` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:42` |
| `<file/function>` | `unique_ptr` | `CurrencyManager` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:25` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:151` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:153` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:488` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/control_bindings_collection/control_bindings_collection.cpp:23` |
| `<file/function>` | `shared_ptr` | `BindingSource` | local/signature/use | `src/core/binding/control_bindings_collection/control_bindings_collection.cpp:24` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/control_bindings_collection/control_bindings_collection.cpp:32` |
| `<file/function>` | `shared_ptr` | `BindingSource` | local/signature/use | `src/core/binding/control_bindings_collection/control_bindings_collection.cpp:33` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/control_bindings_collection/control_bindings_collection.cpp:42` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/control_bindings_collection/control_bindings_collection.cpp:78` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/control_bindings_collection/control_bindings_collection.cpp:85` |
| `PropertyValueFactoryAccess` | `shared_ptr` | `const PropertyObjectData` | local/signature/use | `src/core/binding/value/binding_value.cpp:37` |
| `PropertyValueFactoryAccess` | `shared_ptr` | `const PropertyCollectionData` | local/signature/use | `src/core/binding/value/binding_value.cpp:41` |
| `<file/function>` | `weak_ptr` | `detail::Revocable` | local/signature/use | `src/core/component/component/component.cpp:23` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Component` | local/signature/use | `src/core/component/component_container/component_container.cpp:13` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Component` | local/signature/use | `src/core/component/component_container/component_container.cpp:27` |
| `<file/function>` | `shared_ptr` | `const PropertyEnumDescriptor` | local/signature/use | `src/core/control/control/control.cpp:53` |
| `<file/function>` | `shared_ptr` | `const PropertyEnumDescriptor` | local/signature/use | `src/core/control/control/control.cpp:63` |
| `<file/function>` | `shared_ptr` | `const PropertyEnumDescriptor` | local/signature/use | `src/core/control/control/control.cpp:73` |
| `<file/function>` | `shared_ptr` | `const PropertyEnumDescriptor` | local/signature/use | `src/core/control/control/control.cpp:83` |
| `<file/function>` | `shared_ptr` | `const Theme` | local/signature/use | `src/core/control/control/control.cpp:553` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/control/control/control.cpp:1064` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/control/control/control.cpp:1465` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/control/control/control.cpp:1492` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/control/control/control.cpp:2091` |
| `<file/function>` | `shared_ptr` | `detail::DispatcherState` | local/signature/use | `src/core/control/dispatcher/control_dispatcher.cpp:14` |
| `<file/function>` | `shared_ptr` | `detail::DispatcherState` | local/signature/use | `src/core/control/dispatcher/control_dispatcher.cpp:26` |
| `<file/function>` | `shared_ptr` | `detail::DispatcherState` | local/signature/use | `src/core/control/dispatcher/control_dispatcher.cpp:35` |
| `<file/function>` | `shared_ptr` | `detail::DispatcherState` | local/signature/use | `src/core/control/dispatcher/control_dispatcher.cpp:48` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/control/static_tree/control_factory/control_factory.cpp:17` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/control/static_tree/control_factory/control_factory.cpp:22` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/control/static_tree/control_factory/control_factory.cpp:29` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/control/static_tree/control_factory/control_factory.cpp:30` |
| `DispatchWork` | `qualified-alias:weak_ptr` | `Control` | class declaration | `src/core/dispatcher/state/dispatcher_state.hpp:21` |
| `DispatchWork` | `weak_ptr` | `DispatcherState` | class declaration | `src/core/dispatcher/state/dispatcher_state.hpp:24` |
| `DispatcherState` | `shared_ptr` | `DispatchWork` | class declaration | `src/core/dispatcher/state/dispatcher_state.hpp:35` |
| `<file/function>` | `shared_ptr` | `DispatcherState` | local/signature/use | `src/core/dispatcher/state/dispatcher_state.hpp:54` |
| `<file/function>` | `qualified-alias:weak_ptr` | `Control` | local/signature/use | `src/core/dispatcher/state/dispatcher_state.hpp:55` |
| `<file/function>` | `shared_ptr` | `DispatcherState` | local/signature/use | `src/core/dispatcher/state/post_dispatch.cpp:13` |
| `<file/function>` | `qualified-alias:weak_ptr` | `Control` | local/signature/use | `src/core/dispatcher/state/post_dispatch.cpp:14` |
| `<file/function>` | `shared_ptr` | `DispatchWork` | local/signature/use | `src/core/dispatcher/state/post_dispatch.cpp:27` |
| `DisplayCommand` | `shared_ptr` | `LiveSurface` | class declaration | `src/core/display/command/display_command.hpp:44` |
| `<file/function>` | `shared_ptr` | `LiveSurface` | local/signature/use | `src/core/display/recording_painter/recording_painter.cpp:193` |
| `<file/function>` | `shared_ptr` | `const DisplayChunk` | local/signature/use | `src/core/display/recording_painter/recording_painter.cpp:243` |
| `RecordingPainter` | `shared_ptr` | `LiveSurface` | class declaration | `src/core/display/recording_painter/recording_painter.hpp:39` |
| `RecordingPainter` | `shared_ptr` | `const DisplayChunk` | local/signature/use | `src/core/display/recording_painter/recording_painter.hpp:50` |
| `<file/function>` | `unique_ptr` | `Bitmap` | local/signature/use | `src/core/drawing/bitmap/bitmap.cpp:129` |
| `<file/function>` | `unique_ptr` | `Bitmap` | local/signature/use | `src/core/drawing/bitmap/bitmap.cpp:151` |
| `<file/function>` | `unique_ptr` | `Bitmap` | local/signature/use | `src/core/drawing/bitmap/bitmap.cpp:173` |
| `<file/function>` | `unique_ptr` | `GraphicsPath` | local/signature/use | `src/core/drawing/graphics_path/graphics_path.cpp:391` |
| `<file/function>` | `unique_ptr` | `ImageAttributes` | local/signature/use | `src/core/drawing/image_attributes/image_attributes.cpp:45` |
| `<file/function>` | `unique_ptr` | `TextureBrush` | local/signature/use | `src/core/drawing/texture_brush/texture_brush.cpp:60` |
| `<file/function>` | `unique_ptr` | `TextureBrush` | local/signature/use | `src/core/drawing/texture_brush/texture_brush.cpp:62` |
| `<file/function>` | `shared_ptr` | `LiveSurfaceBuffer` | local/signature/use | `src/core/live_surface/buffer/live_surface_buffer.cpp:29` |
| `<file/function>` | `shared_ptr` | `LiveSurfaceBuffer` | local/signature/use | `src/core/live_surface/buffer/live_surface_buffer.hpp:9` |
| `<file/function>` | `shared_ptr` | `const detail::LiveSurfaceBuffer` | local/signature/use | `src/core/live_surface/frame/live_surface_frame.cpp:10` |
| `LiveSurfaceState` | `shared_ptr` | `LiveSurfaceBuffer` | class declaration | `src/core/live_surface/state/live_surface_state.hpp:28` |
| `LiveSurfaceState` | `shared_ptr` | `LiveSurfaceWake` | class declaration | `src/core/live_surface/state/live_surface_state.hpp:39` |
| `<file/function>` | `shared_ptr` | `detail::LiveSurfaceState` | local/signature/use | `src/core/live_surface/surface/live_surface.cpp:13` |
| `<file/function>` | `shared_ptr` | `LiveSurface` | local/signature/use | `src/core/live_surface/surface/live_surface.cpp:18` |
| `<file/function>` | `shared_ptr` | `LiveSurface` | local/signature/use | `src/core/live_surface/surface/live_surface.cpp:30` |
| `<file/function>` | `shared_ptr` | `detail::LiveSurfaceBuffer` | local/signature/use | `src/core/live_surface/surface/live_surface.cpp:37` |
| `<file/function>` | `shared_ptr` | `detail::LiveSurfaceWake` | local/signature/use | `src/core/live_surface/surface/live_surface.cpp:43` |
| `<file/function>` | `weak_ptr` | `detail::LiveSurfaceState` | local/signature/use | `src/core/live_surface/wake_connection/live_surface_wake_connection.cpp:12` |
| `<file/function>` | `shared_ptr` | `detail::LiveSurfaceWake` | local/signature/use | `src/core/live_surface/wake_connection/live_surface_wake_connection.cpp:13` |
| `<file/function>` | `shared_ptr` | `detail::LiveSurfaceState` | local/signature/use | `src/core/live_surface/write_lease/live_surface_write_lease.cpp:13` |
| `<file/function>` | `shared_ptr` | `detail::LiveSurfaceBuffer` | local/signature/use | `src/core/live_surface/write_lease/live_surface_write_lease.cpp:14` |
| `<file/function>` | `shared_ptr` | `detail::LiveSurfaceWake` | local/signature/use | `src/core/live_surface/write_lease/live_surface_write_lease.cpp:57` |
| `<file/function>` | `shared_ptr` | `detail::Revocable` | local/signature/use | `src/core/scheduler/frame_request_token/frame_request_token.cpp:22` |
| `<file/function>` | `qualified-alias:weak_ptr` | `Control` | local/signature/use | `src/core/scheduler/request/scheduled_frame_request.cpp:8` |
| `ScheduledFrameRequest` | `qualified-alias:weak_ptr` | `Control` | class declaration | `src/core/scheduler/request/scheduled_frame_request.hpp:19` |
| `ScheduledFrameRequest` | `qualified-alias:weak_ptr` | `Control` | class declaration | `src/core/scheduler/request/scheduled_frame_request.hpp:30` |
| `<file/function>` | `weak_ptr` | `CallbackState` | local/signature/use | `src/core/timer/timer/timer.cpp:76` |
| `<file/function>` | `shared_ptr` | `LiveSurface` | local/signature/use | `src/core/types/painter/painter.cpp:177` |
| `<file/function>` | `shared_ptr` | `detail::DispatcherState` | local/signature/use | `src/core/window/dispatcher/window_dispatcher.cpp:14` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/dispatcher/window_dispatcher.cpp:30` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/dispatcher/window_dispatcher.cpp:54` |
| `<file/function>` | `shared_ptr` | `detail::DispatchWork` | local/signature/use | `src/core/window/dispatcher/window_dispatcher.cpp:84` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/dispatcher/window_dispatcher.cpp:110` |
| `<file/function>` | `shared_ptr` | `detail::DispatchWork` | local/signature/use | `src/core/window/dispatcher/window_dispatcher.cpp:234` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/lifecycle/window_lifecycle.cpp:21` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/lifecycle/window_lifecycle.cpp:79` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/lifecycle/window_lifecycle.cpp:98` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/lifecycle/window_lifecycle.cpp:128` |
| `<file/function>` | `shared_ptr` | `const Theme` | local/signature/use | `src/core/window/lifecycle/window_lifecycle.cpp:135` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/lifecycle/window_lifecycle.cpp:144` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/lifecycle/window_lifecycle.cpp:158` |
| `PopupAttachment` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/core/window/popup/popup_attachment.hpp:9` |
| `PopupAttachment` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/core/window/popup/popup_attachment.hpp:9` |
| `PopupAttachment` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/core/window/popup/popup_attachment.hpp:18` |
| `PopupAttachment` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/core/window/popup/popup_attachment.hpp:19` |
| `PopupAttachment` | `qualified-alias:weak_ptr` | `Control` | class declaration | `src/core/window/popup/popup_attachment.hpp:32` |
| `PopupAttachment` | `qualified-alias:weak_ptr` | `Control` | class declaration | `src/core/window/popup/popup_attachment.hpp:33` |
| `ControlCheckpoint` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/core/window/presentation/window_presentation.cpp:57` |
| `ControlCheckpoint` | `shared_ptr` | `const detail::DisplayChunk` | class declaration | `src/core/window/presentation/window_presentation.cpp:58` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:61` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:63` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:66` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:133` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:159` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:225` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:273` |
| `<file/function>` | `shared_ptr` | `LiveSurface` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:273` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:306` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:317` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:354` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:465` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/scheduler/window_scheduler.cpp:42` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/scheduler/window_scheduler.cpp:61` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:43` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:43` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:66` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:73` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:76` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:86` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:88` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:131` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:140` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:142` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:303` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:304` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:339` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:340` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:361` |
| `<file/function>` | `shared_ptr` | `detail::PopupAttachment` | local/signature/use | `src/core/window/window.cpp:362` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:382` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:384` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:387` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:393` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:394` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:399` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:413` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:422` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:432` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:494` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:501` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:513` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:559` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:562` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:563` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:566` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:567` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:568` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:571` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:574` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:574` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:577` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:589` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:593` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:601` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:615` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:616` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:619` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:620` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:621` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:629` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:632` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:632` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:637` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:652` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:665` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:685` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:700` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:721` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:746` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:747` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:748` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:750` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:802` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:803` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:806` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:815` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:829` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:830` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:906` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:925` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:941` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:987` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:998` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1012` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1013` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1014` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1015` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1016` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1023` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1026` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1027` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1030` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1040` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1041` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1046` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1063` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1076` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1079` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1098` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1119` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1120` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1146` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1158` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1221` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1264` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1317` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1453` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1454` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1462` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1522` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1531` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1560` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1561` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1604` |
| `<file/function>` | `qualified-alias:weak_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1604` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1607` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1608` |
| `<file/function>` | `qualified-alias:weak_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1608` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1609` |
| `<file/function>` | `qualified-alias:weak_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1609` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1623` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1626` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1641` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1649` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1652` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1661` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1673` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1675` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1675` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1703` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1714` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1716` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1716` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1764` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1766` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1776` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1778` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1814` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1829` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1846` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1853` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1882` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1883` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1885` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1885` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1904` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2028` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2029` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2032` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2058` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2063` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2225` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2269` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2282` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2291` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2304` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2320` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2326` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2338` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2338` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2339` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2347` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2347` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2367` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2385` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2388` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2407` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2431` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2472` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2486` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2493` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2507` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2513` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2526` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2535` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2543` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2551` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2632` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2658` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2669` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2734` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `src/host/macos/application/macos_host.mm:434` |
| `<file/function>` | `unique_ptr` | `HostSession` | local/signature/use | `src/host/macos/application/macos_host.mm:435` |
| `<file/function>` | `unique_ptr` | `HostServices` | local/signature/use | `src/host/macos/application/macos_host.mm:436` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `src/host/macos/application/macos_host.mm:459` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `src/host/macos/application/macos_host.mm:738` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `src/host/macos/application/macos_host.mm:1775` |
| `<file/function>` | `unique_ptr` | `HostServices` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:484` |
| `DibPainter` | `shared_ptr` | `LiveSurface` | local/signature/use | `src/host/windows/application/windows_host.cpp:992` |
| `WindowsHostState` | `unique_ptr` | `Window` | local/signature/use | `src/host/windows/application/windows_host.cpp:1654` |
| `WindowsHostState` | `unique_ptr` | `Window` | class declaration | `src/host/windows/application/windows_host.cpp:2670` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `src/host/windows/application/windows_host.cpp:2731` |
| `<file/function>` | `unique_ptr` | `WindowsHostState` | local/signature/use | `src/host/windows/application/windows_host.cpp:2874` |
| `PendingFrame` | `shared_ptr` | `LiveSurface` | class declaration | `src/host/windows/paint_endpoint/windows_compatibility_paint_endpoint.cpp:205` |
| `WindowsCompatibilityPaintEndpoint` | `shared_ptr` | `LiveSurface` | class declaration | `src/host/windows/paint_endpoint/windows_compatibility_paint_endpoint.cpp:453` |
| `<file/function>` | `unique_ptr` | `Implementation` | local/signature/use | `src/host/windows/paint_endpoint/windows_compatibility_paint_endpoint.cpp:474` |
| `<file/function>` | `shared_ptr` | `WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/host/windows/paint_endpoint/windows_compatibility_paint_endpoint.cpp:481` |
| `<file/function>` | `shared_ptr` | `WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/host/windows/paint_endpoint/windows_compatibility_paint_endpoint.cpp:486` |
| `<file/function>` | `shared_ptr` | `LiveSurface` | local/signature/use | `src/host/windows/paint_endpoint/windows_compatibility_paint_endpoint.cpp:502` |
| `CoreGraphicsRaster` | `unique_ptr` | `Impl` | class declaration | `src/render/coregraphics/raster/coregraphics_raster.hpp:78` |
| `<file/function>` | `unique_ptr` | `SkCodec` | local/signature/use | `src/render/skia/executor/skia_executor.cpp:834` |
| `DecodeResult` | `unique_ptr` | `Bitmap` | class declaration | `src/render/skia/executor/skia_executor.hpp:38` |
| `SkiaExecutor` | `unique_ptr` | `Impl` | class declaration | `src/render/skia/executor/skia_executor.hpp:81` |
| `<file/function>` | `unique_ptr` | `SkCodec` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:447` |
| `<file/function>` | `shared_ptr` | `LiveSurface` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:749` |
| `SkiaRaster` | `shared_ptr` | `LiveSurface` | class declaration | `src/render/skia/raster/skia_raster.hpp:68` |
| `SkiaRaster` | `unique_ptr` | `Impl` | class declaration | `src/render/skia/raster/skia_raster.hpp:81` |
| `HarfBuzzFontEngine` | `unique_ptr` | `Impl` | class declaration | `src/render/text/harfbuzz/harfbuzz_font_engine.hpp:65` |

## Raw-pointer plumbing

Raw pointers are recorded because a future lifecycle round must distinguish non-owning direct access, platform handles, optional links, array traversal, and accidental ownership. Their presence is not a defect under the house policy.

| Owner/scope | Target | Scope | Location | Evidence |
|---|---|---|---|---|
| `Binding` | `Control` | class declaration | `include/gui_forms/binding/binding/binding.hpp:24` | `[[nodiscard]] Control* target() const noexcept { return target_; }` |
| `Binding` | `Control` | class declaration | `include/gui_forms/binding/binding/binding.hpp:64` | `Control* target_{};` |
| `BindingContext` | `Window` | class declaration | `include/gui_forms/binding/binding_context/binding_context.hpp:42` | `[[nodiscard]] Window* bound_window() const noexcept;` |
| `BindingContext` | `BindingSource` | class declaration | `include/gui_forms/binding/binding_context/binding_context.hpp:43` | `bool remove_entry(BindingSource* source, bool publish);` |
| `BindingManagerBase` | `const BindingRecord` | class declaration | `include/gui_forms/binding/binding_manager_base/binding_manager_base.hpp:16` | `[[nodiscard]] virtual const BindingRecord* current() const noexcept = 0;` |
| `BindingSource` | `const BindingRecord` | class declaration | `include/gui_forms/binding/binding_source/binding_source.hpp:44` | `[[nodiscard]] const BindingRecord* current() const noexcept;` |
| `BindingSource` | `const Binding` | class declaration | `include/gui_forms/binding/binding_source/binding_source.hpp:133` | `void unregister_binding(const Binding* binding) noexcept;` |
| `BindingSource` | `Window` | class declaration | `include/gui_forms/binding/binding_source/binding_source.hpp:135` | `[[nodiscard]] Window* bound_window() const noexcept;` |
| `ControlBindingsCollection` | `Control` | class declaration | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:50` | `Control* target_{};` |
| `CurrencyManager` | `const BindingRecord` | class declaration | `include/gui_forms/binding/currency_manager/currency_manager.hpp:14` | `[[nodiscard]] const BindingRecord* current() const noexcept override;` |
| `CurrencyManager` | `BindingSource` | class declaration | `include/gui_forms/binding/currency_manager/currency_manager.hpp:39` | `BindingSource* source_{};` |
| `BindingCompleteEvent` | `Binding` | class declaration | `include/gui_forms/binding/types/binding_contract_types.hpp:79` | `Binding* binding{};` |
| `BindingContextChange` | `BindingSource` | class declaration | `include/gui_forms/binding/types/binding_contract_types.hpp:123` | `BindingSource* source{};` |
| `PropertyCollectionValue` | `const PropertyCollectionData` | class declaration | `include/gui_forms/binding/value/property_collection_value/property_collection_value.hpp:21` | `[[nodiscard]] const PropertyCollectionData* data() const noexcept {` |
| `PropertyObjectValue` | `const PropertyObjectData` | class declaration | `include/gui_forms/binding/value/property_object_value/property_object_value.hpp:21` | `[[nodiscard]] const PropertyObjectData* data() const noexcept {` |
| `gf_string_view` | `const char` | class declaration | `include/gui_forms/c_api.h:61` | `const char* data;` |
| `<file/function>` | `void` | local/signature/use | `include/gui_forms/c_api.h:174` | `void* context,` |
| `<file/function>` | `char` | local/signature/use | `include/gui_forms/c_api.h:176` | `char* text_buffer,` |
| `<file/function>` | `void` | local/signature/use | `include/gui_forms/c_api.h:180` | `void* context, const gf_property_value* value);` |
| `<file/function>` | `void` | local/signature/use | `include/gui_forms/c_api.h:181` | `typedef uint32_t (*gf_property_reset_callback)(void* context);` |
| `<file/function>` | `void` | local/signature/use | `include/gui_forms/c_api.h:183` | `void* context, uint32_t* should_serialize);` |
| `<file/function>` | `void` | local/signature/use | `include/gui_forms/c_api.h:185` | `void* context,` |
| `<file/function>` | `char` | local/signature/use | `include/gui_forms/c_api.h:187` | `char* text_buffer,` |
| `<file/function>` | `void` | local/signature/use | `include/gui_forms/c_api.h:191` | `void* context,` |
| `<file/function>` | `char` | local/signature/use | `include/gui_forms/c_api.h:194` | `char* value_text_buffer,` |
| `<file/function>` | `void` | local/signature/use | `include/gui_forms/c_api.h:198` | `void* context,` |
| `<file/function>` | `char` | local/signature/use | `include/gui_forms/c_api.h:201` | `char* edited_text_buffer,` |
| `gf_property_callbacks_v1` | `void` | class declaration | `include/gui_forms/c_api.h:208` | `void* context;` |
| `<file/function>` | `void` | local/signature/use | `include/gui_forms/c_api.h:337` | `typedef void (*gf_event_callback)(gf_handle sender, uint32_t event_kind, void* context);` |
| `<file/function>` | `void` | local/signature/use | `include/gui_forms/c_api.h:340` | `void* context);` |
| `<file/function>` | `void` | local/signature/use | `include/gui_forms/c_api.h:341` | `typedef uint32_t (*gf_dispatch_callback)(void* context, uint32_t cancelled);` |
| `<file/function>` | `void` | local/signature/use | `include/gui_forms/c_api.h:348` | `void* context);` |
| `<file/function>` | `void` | local/signature/use | `include/gui_forms/c_api.h:354` | `void* context);` |
| `<file/function>` | `void` | local/signature/use | `include/gui_forms/c_api.h:360` | `void* context);` |
| `gf_api_v0` | `char` | class declaration | `include/gui_forms/c_api.h:378` | `char* buffer,` |
| `gf_api_v0` | `void` | class declaration | `include/gui_forms/c_api.h:390` | `void* context,` |
| `gf_api_v0` | `char` | class declaration | `include/gui_forms/c_api.h:400` | `char* buffer,` |
| `gf_api_v0` | `char` | class declaration | `include/gui_forms/c_api.h:405` | `char* buffer,` |
| `gf_api_v0` | `char` | class declaration | `include/gui_forms/c_api.h:414` | `char* buffer,` |
| `gf_api_v0` | `void` | class declaration | `include/gui_forms/c_api.h:422` | `void* context,` |
| `gf_api_v0` | `void` | class declaration | `include/gui_forms/c_api.h:426` | `void* context);` |
| `gf_api_v0` | `void` | class declaration | `include/gui_forms/c_api.h:444` | `void* context,` |
| `gf_api_v0` | `void` | class declaration | `include/gui_forms/c_api.h:454` | `void* context,` |
| `gf_api_v0` | `void` | class declaration | `include/gui_forms/c_api.h:458` | `void* context,` |
| `gf_api_v0` | `char` | class declaration | `include/gui_forms/c_api.h:480` | `char* buffer,` |
| `gf_api_v0` | `char` | class declaration | `include/gui_forms/c_api.h:508` | `char* buffer,` |
| `gf_api_v0` | `void` | class declaration | `include/gui_forms/c_api.h:549` | `void* context,` |
| `<file/function>` | `char` | local/signature/use | `include/gui_forms/c_api.h:661` | `char* buffer,` |
| `<file/function>` | `const void` | local/signature/use | `include/gui_forms/c_api.h:669` | `const void* pixels);` |
| `Control` | `const Api` | class declaration | `include/gui_forms/c_api.hpp:198` | `const Api* api_{};` |
| `ErrorProvider` | `Window` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:142` | `[[nodiscard]] Window* bound_window() const noexcept;` |
| `ErrorProvider` | `Entry` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:144` | `[[nodiscard]] Entry* find_entry(const Control& target);` |
| `ErrorProvider` | `const Entry` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:145` | `[[nodiscard]] const Entry* find_entry(const Control& target) const;` |
| `HelpProvider` | `Window` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:101` | `[[nodiscard]] Window* bound_window() const noexcept;` |
| `HelpProvider` | `Entry` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:103` | `[[nodiscard]] Entry* find_entry(const Control& target);` |
| `HelpProvider` | `const Entry` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:104` | `[[nodiscard]] const Entry* find_entry(const Control& target) const;` |
| `ToolTip` | `Window` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:82` | `[[nodiscard]] Window* bound_window() const noexcept;` |
| `ToolTip` | `Entry` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:84` | `[[nodiscard]] Entry* find_entry(const Control& target);` |
| `ToolTip` | `const Entry` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:85` | `[[nodiscard]] const Entry* find_entry(const Control& target) const;` |
| `ControlValidationEvent` | `Control` | class declaration | `include/gui_forms/control/control/control.hpp:153` | `Control* control{};` |
| `ControlValidationEvent` | `Control` | class declaration | `include/gui_forms/control/control/control.hpp:154` | `Control* destination{};` |
| `Control` | `Window` | class declaration | `include/gui_forms/control/control/control.hpp:360` | `[[nodiscard]] Window* attached_window() const noexcept { return window_; }` |
| `Control` | `Window` | class declaration | `include/gui_forms/control/control/control.hpp:568` | `[[nodiscard]] Window* window() const noexcept { return window_; }` |
| `Control` | `Control` | class declaration | `include/gui_forms/control/control/control.hpp:649` | `[[nodiscard]] bool perform_validation(Control* destination, bool bulk);` |
| `Control` | `const BindableProperty` | class declaration | `include/gui_forms/control/control/control.hpp:653` | `[[nodiscard]] const BindableProperty* find_bindable_property(` |
| `Control` | `const void` | class declaration | `include/gui_forms/control/control/control.hpp:658` | `void publish_change(const void* event_key,` |
| `DeferredInitializationChange` | `const void` | class declaration | `include/gui_forms/control/control/control.hpp:662` | `const void* event_key{};` |
| `Control` | `Window` | class declaration | `include/gui_forms/control/control/control.hpp:673` | `Window* window_{};` |
| `TableLayoutPanel` | `const CellMetadata` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.hpp:114` | `[[nodiscard]] const CellMetadata* metadata_for(const Control& child) const;` |
| `ScrollProperties` | `ScrollableControl` | class declaration | `include/gui_forms/controls/scrollable_control/scroll_properties/scroll_properties.hpp:87` | `ScrollableControl* owner_{};` |
| `ScrollableControl` | `const char` | class declaration | `include/gui_forms/controls/scrollable_control/scrollable_control.hpp:143` | `static void validate_size(Size size, const char* message);` |
| `ScrollableControl` | `const char` | class declaration | `include/gui_forms/controls/scrollable_control/scrollable_control.hpp:144` | `static void validate_axis_value(double value, const char* message);` |
| `BitmapLockView` | `const std::byte` | class declaration | `include/gui_forms/drawing/bitmap/bitmap.hpp:15` | `const std::byte* data{};` |
| `BitmapLockView` | `std::byte` | class declaration | `include/gui_forms/drawing/bitmap/bitmap.hpp:16` | `std::byte* writable_data{};` |
| `BitmapEditView` | `const std::byte` | class declaration | `include/gui_forms/drawing/bitmap/bitmap.hpp:32` | `const std::byte* data{};` |
| `BitmapEditView` | `std::byte` | class declaration | `include/gui_forms/drawing/bitmap/bitmap.hpp:33` | `std::byte* writable_data{};` |
| `gd_string_view` | `const char` | class declaration | `include/gui_forms/drawing_c_api.h:33` | `const char* data;` |
| `gd_bitmap_lock_view` | `const void` | class declaration | `include/gui_forms/drawing_c_api.h:178` | `const void* data;` |
| `gd_bitmap_lock_view` | `void` | class declaration | `include/gui_forms/drawing_c_api.h:179` | `void* writable_data;` |
| `gd_bitmap_edit_view` | `const void` | class declaration | `include/gui_forms/drawing_c_api.h:188` | `const void* data;` |
| `gd_bitmap_edit_view` | `void` | class declaration | `include/gui_forms/drawing_c_api.h:189` | `void* writable_data;` |
| `gd_raster_service_v0` | `const void` | class declaration | `include/gui_forms/drawing_c_api.h:217` | `gd_result (*execute)(const void* recorder, void* bitmap,` |
| `gd_raster_service_v0` | `void` | class declaration | `include/gui_forms/drawing_c_api.h:217` | `gd_result (*execute)(const void* recorder, void* bitmap,` |
| `gd_raster_service_v0` | `const void` | class declaration | `include/gui_forms/drawing_c_api.h:220` | `gd_result (*encode_png)(const void* bitmap, void* buffer,` |
| `gd_raster_service_v0` | `void` | class declaration | `include/gui_forms/drawing_c_api.h:220` | `gd_result (*encode_png)(const void* bitmap, void* buffer,` |
| `gd_raster_service_v0` | `const void` | class declaration | `include/gui_forms/drawing_c_api.h:222` | `gd_result (*decode_png)(const void* data, uint64_t size,` |
| `gd_raster_service_v0` | `const void` | class declaration | `include/gui_forms/drawing_c_api.h:224` | `gd_result (*measure_string)(const void* font, const void* format,` |
| `gd_raster_service_v0` | `const void` | class declaration | `include/gui_forms/drawing_c_api.h:224` | `gd_result (*measure_string)(const void* font, const void* format,` |
| `gd_api_v0` | `char` | class declaration | `include/gui_forms/drawing_c_api.h:285` | `char* buffer,` |
| `gd_api_v0` | `void` | class declaration | `include/gui_forms/drawing_c_api.h:403` | `gd_result (*bitmap_encode_png)(gd_handle bitmap, void* buffer,` |
| `gd_api_v0` | `const void` | class declaration | `include/gui_forms/drawing_c_api.h:405` | `gd_result (*bitmap_decode_png)(const void* data, uint64_t size,` |
| `Event` | `Component` | class declaration | `include/gui_forms/event/event/event.hpp:132` | `[[nodiscard]] SubscriptionToken subscribe_impl(Component* owner, Callback callback) {` |
| `SemanticFeedback` | `Window` | class declaration | `include/gui_forms/feedback/semantic_feedback/semantic_feedback.hpp:45` | `Window* window_{};` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/feedback/types/feedback_types.hpp:33` | `[[nodiscard]] const char* semantic_feedback_kind_name(` |
| `HostSession` | `HostServices` | class declaration | `include/gui_forms/host/session/host_session.hpp:17` | `HostServices* services = nullptr);` |
| `HostSession` | `Window` | class declaration | `include/gui_forms/host/session/host_session.hpp:35` | `Window* window_{};` |
| `HostSession` | `HostServices` | class declaration | `include/gui_forms/host/session/host_session.hpp:36` | `HostServices* services_{};` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/host/types/host_types.hpp:303` | `[[nodiscard]] const char* host_dispatch_error_name(HostDispatchError error) noexcept;` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/host/types/host_types.hpp:304` | `[[nodiscard]] const char* host_lifecycle_phase_name(HostLifecyclePhase phase) noexcept;` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/host/types/host_types.hpp:305` | `[[nodiscard]] const char* host_event_name(const HostEventPayload& payload) noexcept;` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/host/types/host_types.hpp:306` | `[[nodiscard]] const char* host_service_error_name(HostServiceError error) noexcept;` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/host/types/host_types.hpp:307` | `[[nodiscard]] const char* cursor_kind_name(CursorKind cursor) noexcept;` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/host/types/host_types.hpp:308` | `[[nodiscard]] const char* drag_effect_name(DragEffect effect) noexcept;` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/host/types/host_types.hpp:309` | `[[nodiscard]] const char* host_dialog_kind_name(` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/host/types/host_types.hpp:311` | `[[nodiscard]] const char* host_dialog_outcome_name(HostDialogOutcome outcome) noexcept;` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/host/types/host_types.hpp:312` | `[[nodiscard]] const char* host_dialog_choice_name(HostDialogChoice choice) noexcept;` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/host/types/host_types.hpp:313` | `[[nodiscard]] const char* host_sound_cue_name(HostSoundCue cue) noexcept;` |
| `ImageList` | `Window` | class declaration | `include/gui_forms/image_list/image_list/image_list.hpp:70` | `[[nodiscard]] Window* bound_window() const noexcept;` |
| `PropertyValueConverterRegistry` | `const PropertyValueConverter` | class declaration | `include/gui_forms/inspection/property_value_converter_registry/property_value_converter_registry.hpp:49` | `[[nodiscard]] const PropertyValueConverter* find(` |
| `WindowLifetime` | `Window` | class declaration | `include/gui_forms/scheduler/types/scheduler_types.hpp:16` | `Window* window{};` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/semantics/types/semantic_types.hpp:95` | `[[nodiscard]] const char* semantic_role_name(SemanticRole role) noexcept;` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/semantics/types/semantic_types.hpp:96` | `[[nodiscard]] const char* semantic_action_name(SemanticAction action) noexcept;` |
| `CallbackState` | `Timer` | class declaration | `include/gui_forms/timer/timer/timer.hpp:42` | `Timer* owner{};` |
| `Timer` | `Window` | class declaration | `include/gui_forms/timer/timer/timer.hpp:45` | `[[nodiscard]] Window* bound_window() const noexcept;` |
| `UpdateScope` | `Window` | class declaration | `include/gui_forms/window/update_scope/update_scope.hpp:22` | `Window* window_{};` |
| `Window` | `HostServices` | class declaration | `include/gui_forms/window/window.hpp:251` | `[[nodiscard]] HostServices* host_services() const noexcept {` |
| `Window` | `Control` | class declaration | `include/gui_forms/window/window.hpp:360` | `Control* destination = nullptr,` |
| `Window` | `HostServices` | class declaration | `include/gui_forms/window/window.hpp:677` | `HostServices* host_services_{};` |
| `<file/function>` | `char` | local/signature/use | `src/abi/c_api.cpp:73` | `char* buffer,` |
| `<file/function>` | `void` | local/signature/use | `src/abi/c_api.cpp:105` | `void* context,` |
| `<file/function>` | `char` | local/signature/use | `src/abi/c_api.cpp:116` | `gf_result api_get_name(gf_handle handle, char* buffer, std::uint64_t capacity,` |
| `<file/function>` | `char` | local/signature/use | `src/abi/c_api.cpp:123` | `gf_result api_get_text(gf_handle handle, char* buffer, std::uint64_t capacity,` |
| `<file/function>` | `char` | local/signature/use | `src/abi/c_api.cpp:257` | `gf_result api_last_host_trace(gf_handle handle, char* buffer,` |
| `<file/function>` | `void` | local/signature/use | `src/abi/c_api.cpp:265` | `gf_event_callback_v2 callback, void* context,` |
| `<file/function>` | `void` | local/signature/use | `src/abi/c_api.cpp:272` | `void* context) noexcept {` |
| `<file/function>` | `void` | local/signature/use | `src/abi/c_api.cpp:336` | `void* context, gf_event_token* token) noexcept {` |
| `<file/function>` | `void` | local/signature/use | `src/abi/c_api.cpp:348` | `void* context, gf_event_token* token) noexcept {` |
| `<file/function>` | `void` | local/signature/use | `src/abi/c_api.cpp:352` | `void* context,` |
| `<file/function>` | `void` | local/signature/use | `src/abi/c_api.cpp:359` | `void* context, gf_event_token* token) noexcept {` |
| `<file/function>` | `char` | local/signature/use | `src/abi/c_api.cpp:395` | `gf_result api_last_dialog_path(gf_handle owner, char* buffer,` |
| `<file/function>` | `char` | local/signature/use | `src/abi/c_api.cpp:443` | `gf_result api_read_clipboard_text(gf_handle owner, char* buffer,` |
| `<file/function>` | `char` | local/signature/use | `src/abi/c_api.cpp:543` | `std::uint64_t endpoint, char* buffer, std::uint64_t capacity,` |
| `<file/function>` | `const void` | local/signature/use | `src/abi/c_api.cpp:575` | `std::uint64_t row_bytes, const void* pixels) {` |
| `PropertyState` | `char` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:519` | `[&](char* buffer, std::uint64_t capacity,` |
| `PropertyState` | `char` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:562` | `[&](char* buffer, std::uint64_t capacity,` |
| `PropertyState` | `char` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:576` | `[&](char* buffer, std::uint64_t capacity,` |
| `PropertyState` | `char` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:598` | `[&](char* buffer, std::uint64_t capacity,` |
| `Registry` | `Slot` | local/signature/use | `src/abi/drawing_c_api.cpp:145` | `Slot* slot = find_locked(handle);` |
| `Registry` | `Slot` | local/signature/use | `src/abi/drawing_c_api.cpp:165` | `Slot* slot = find_locked(handle);` |
| `Registry` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:300` | `for (ObjectRecord* record : {recorder_record.get(), font_record.get(),` |
| `Registry` | `Slot` | local/signature/use | `src/abi/drawing_c_api.cpp:327` | `Slot* slot = find_locked(handle);` |
| `Registry` | `Slot` | class declaration | `src/abi/drawing_c_api.cpp:366` | `Slot* find_locked(gd_handle handle) noexcept {` |
| `<file/function>` | `char` | local/signature/use | `src/abi/drawing_c_api.cpp:692` | `gd_result api_recorder_trace(gd_handle handle, char* buffer,` |
| `<file/function>` | `void` | local/signature/use | `src/abi/drawing_c_api.cpp:1522` | `gd_result api_bitmap_encode_png(gd_handle bitmap, void* buffer,` |
| `<file/function>` | `const void` | local/signature/use | `src/abi/drawing_c_api.cpp:1541` | `gd_result api_bitmap_decode_png(const void* data, std::uint64_t size,` |
| `<file/function>` | `void` | local/signature/use | `src/abi/drawing_c_api.cpp:1551` | `void* decoded = nullptr;` |
| `HdcLease` | `Bitmap` | class declaration | `src/abi/drawing_platform_windows.cpp:22` | `Bitmap* bitmap{};` |
| `HdcLease` | `void` | class declaration | `src/abi/drawing_platform_windows.cpp:27` | `void* pixels{};` |
| `CaptureStaging` | `void` | class declaration | `src/abi/drawing_platform_windows.cpp:72` | `void* pixels{};` |
| `<file/function>` | `void` | local/signature/use | `src/abi/drawing_platform_windows.cpp:132` | `void copy_as_bgra(const ImageSnapshot& snapshot, void* destination) {` |
| `<file/function>` | `void` | local/signature/use | `src/abi/drawing_platform_windows.cpp:148` | `void* pixels = nullptr;` |
| `<file/function>` | `void` | local/signature/use | `src/abi/drawing_platform_windows.cpp:246` | `void* pixels = nullptr;` |
| `<file/function>` | `const void` | local/signature/use | `src/abi/drawing_platform_windows.cpp:383` | `const void* pixels = snapshot.pixels().data();` |
| `<file/function>` | `void` | local/signature/use | `src/abi/drawing_platform_windows.cpp:417` | `void* pixels = nullptr;` |
| `<file/function>` | `const char` | local/signature/use | `src/abi/drawing_skia_c_api.cpp:99` | `if (const char* trace = std::getenv("GUI_DRAWING_TRACE_FONTS");` |
| `<file/function>` | `const char` | local/signature/use | `src/abi/drawing_skia_c_api.cpp:218` | `if (const char* trace = std::getenv("GUI_DRAWING_TRACE_FONTS");` |
| `<file/function>` | `const void` | local/signature/use | `src/abi/drawing_skia_c_api.cpp:270` | `gd_result execute(const void* recorder, void* bitmap,` |
| `<file/function>` | `void` | local/signature/use | `src/abi/drawing_skia_c_api.cpp:270` | `gd_result execute(const void* recorder, void* bitmap,` |
| `<file/function>` | `const void` | local/signature/use | `src/abi/drawing_skia_c_api.cpp:293` | `gd_result encode_png(const void* bitmap, void* buffer, std::uint64_t capacity,` |
| `<file/function>` | `void` | local/signature/use | `src/abi/drawing_skia_c_api.cpp:293` | `gd_result encode_png(const void* bitmap, void* buffer, std::uint64_t capacity,` |
| `<file/function>` | `const void` | local/signature/use | `src/abi/drawing_skia_c_api.cpp:314` | `gd_result decode_png(const void* data, std::uint64_t size, void** bitmap) {` |
| `<file/function>` | `const void` | local/signature/use | `src/abi/drawing_skia_c_api.cpp:331` | `gd_result measure_string(const void* font, const void* format,` |
| `<file/function>` | `const void` | local/signature/use | `src/abi/drawing_skia_c_api.cpp:331` | `gd_result measure_string(const void* font, const void* format,` |
| `<file/function>` | `char` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:159` | `std::uint64_t token, char* buffer, std::uint64_t capacity,` |
| `<file/function>` | `const void` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:182` | `std::uint64_t row_bytes, const void* pixels) {` |
| `<file/function>` | `char` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.hpp:37` | `std::uint64_t token, char* buffer, std::uint64_t capacity,` |
| `<file/function>` | `const void` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.hpp:41` | `std::uint32_t height, std::uint64_t row_bytes, const void* pixels);` |
| `SubscriptionRecord` | `void` | class declaration | `src/abi/registry/registry.hpp:23` | `void* context{};` |
| `DispatchRecord` | `void` | class declaration | `src/abi/registry/registry.hpp:32` | `void* context{};` |
| `Registry` | `char` | class declaration | `src/abi/registry/registry.hpp:241` | `gf_result get_string(gf_handle handle, char* buffer, std::uint64_t capacity,` |
| `Registry` | `char` | class declaration | `src/abi/registry/registry.hpp:1206` | `gf_result read_clipboard_text(gf_handle owner_handle, char* buffer,` |
| `Registry` | `char` | class declaration | `src/abi/registry/registry.hpp:1595` | `gf_result last_dialog_path(gf_handle owner_handle, char* buffer,` |
| `Registry` | `char` | class declaration | `src/abi/registry/registry.hpp:1899` | `gf_result last_host_trace(gf_handle handle, char* buffer,` |
| `Registry` | `ComponentState::alive:` | local/signature/use | `src/abi/registry/registry.hpp:2008` | `case ComponentState::alive: *output = GF_COMPONENT_ALIVE; break;` |
| `Registry` | `ComponentState::disposing:` | local/signature/use | `src/abi/registry/registry.hpp:2009` | `case ComponentState::disposing: *output = GF_COMPONENT_DISPOSING; break;` |
| `Registry` | `ComponentState::disposed:` | local/signature/use | `src/abi/registry/registry.hpp:2010` | `case ComponentState::disposed: *output = GF_COMPONENT_DISPOSED; break;` |
| `Registry` | `char` | class declaration | `src/abi/registry/registry.hpp:2016` | `char* buffer,` |
| `Registry` | `Window` | local/signature/use | `src/abi/registry/registry.hpp:2169` | `Window* const window = owner->control->attached_window();` |
| `Registry` | `void` | class declaration | `src/abi/registry/registry.hpp:2215` | `void* context,` |
| `Registry` | `void` | class declaration | `src/abi/registry/registry.hpp:2256` | `void* context,` |
| `Registry` | `void` | class declaration | `src/abi/registry/registry.hpp:2346` | `gf_pointer_callback callback, void* context,` |
| `Registry` | `void` | class declaration | `src/abi/registry/registry.hpp:2409` | `gf_key_callback callback, void* context,` |
| `Registry` | `void` | class declaration | `src/abi/registry/registry.hpp:2444` | `gf_key_callback callback, void* context,` |
| `Registry` | `void` | class declaration | `src/abi/registry/registry.hpp:2489` | `gf_text_callback callback, void* context,` |
| `Registry` | `void` | class declaration | `src/abi/registry/registry.hpp:2525` | `void* context) {` |
| `Registry` | `RegistrySlot` | local/signature/use | `src/abi/registry/registry.hpp:2629` | `RegistrySlot* slot = slot_locked(token);` |
| `Registry` | `const char` | local/signature/use | `src/abi/registry/registry.hpp:2850` | `const char* phase = "idle";` |
| `Registry` | `RegistrySlot` | local/signature/use | `src/abi/registry/registry.hpp:2948` | `RegistrySlot* slot = slot_locked(handle);` |
| `Registry` | `RegistrySlot` | local/signature/use | `src/abi/registry/registry.hpp:2964` | `RegistrySlot* slot = slot_locked(token);` |
| `Registry` | `RegistrySlot` | class declaration | `src/abi/registry/registry.hpp:2976` | `RegistrySlot* slot_locked(gf_handle handle) {` |
| `Registry` | `RegistrySlot` | local/signature/use | `src/abi/registry/registry.hpp:3007` | `if (RegistrySlot* slot = slot_locked(token);` |
| `Registry` | `RegistrySlot` | local/signature/use | `src/abi/registry/registry.hpp:3018` | `RegistrySlot* slot = slot_locked(handle);` |
| `Registry` | `RegistrySlot` | local/signature/use | `src/abi/registry/registry.hpp:3038` | `RegistrySlot* slot = slot_locked(token);` |
| `Registry` | `void` | local/signature/use | `src/abi/registry/registry.hpp:3051` | `void* const context = subscription->context;` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/button_base/button/button.cpp:58` | `Window* owner = attached_window();` |
| `<file/function>` | `const char` | local/signature/use | `src/controls/commands/command/command.cpp:35` | `void require_command_text(std::string_view value, const char* field) {` |
| `<file/function>` | `const char` | local/signature/use | `src/controls/easing_preview/easing_preview.cpp:138` | `const char* name = !policy.enabled` |
| `GalleryContext` | `Window` | class declaration | `src/controls/gallery/context/gallery_context.hpp:14` | `Window* window{};` |
| `GalleryControl` | `NodeSpec` | class declaration | `src/controls/gallery/control/gallery_control.hpp:34` | `const dml::NodeSpec* specification_{};` |
| `GalleryCheckBox` | `const char` | local/signature/use | `src/controls/gallery_controls.cpp:203` | `const char* state = check_state() == CheckState::checked` |
| `<file/function>` | `NodeSpec` | local/signature/use | `src/controls/gallery_controls.cpp:586` | `const dml::NodeSpec* gallery_child = dml::find(child->stable_id().value());` |
| `<file/function>` | `NodeSpec` | local/signature/use | `src/controls/gallery_controls.cpp:615` | `const dml::NodeSpec* gallery_child = dml::find(child->stable_id().value());` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_glyph/error_glyph.cpp:94` | `Window* owner = attached_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_glyph/error_glyph.cpp:129` | `Window* owner = attached_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:51` | `if (Window* owner = bound_window()) {` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:76` | `Window* ErrorProvider::bound_window() const noexcept {` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:85` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:93` | `Window* owner = bound_window();` |
| `<file/function>` | `ErrorProvider::Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:100` | `ErrorProvider::Entry* ErrorProvider::find_entry(const Control& target) {` |
| `<file/function>` | `const ErrorProvider::Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:105` | `const ErrorProvider::Entry* ErrorProvider::find_entry(const Control& target) const {` |
| `<file/function>` | `Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:116` | `if (Entry* existing = find_entry(*target)) return *existing;` |
| `<file/function>` | `Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:123` | `if (Entry* mapped = find_entry(*current)) position_visual(*mapped);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:153` | `if (Window* owner = bound_window()) owner->verify_access("ErrorProvider error query");` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:154` | `const Entry* entry = find_entry(target);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:193` | `if (Window* owner = bound_window()) {` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:196` | `const Entry* entry = find_entry(target);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:214` | `if (Window* owner = bound_window()) {` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:217` | `const Entry* entry = find_entry(target);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:264` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:276` | `Window* owner = bound_window();` |
| `<file/function>` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:361` | `Control* raw = event.binding->target();` |
| `<file/function>` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:398` | `Control* raw = binding ? binding->target() : nullptr;` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:449` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:502` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:558` | `if (Window* owner = bound_window()) owner->verify_access("ErrorProvider snapshot");` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:587` | `if (Window* owner = bound_window()) owner->verify_access("ErrorProvider disposal");` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:53` | `Window* HelpProvider::bound_window() const noexcept {` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:62` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:70` | `Window* owner = bound_window();` |
| `<file/function>` | `HelpProvider::Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:77` | `HelpProvider::Entry* HelpProvider::find_entry(const Control& target) {` |
| `<file/function>` | `const HelpProvider::Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:82` | `const HelpProvider::Entry* HelpProvider::find_entry(const Control& target) const {` |
| `<file/function>` | `Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:93` | `if (Entry* existing = find_entry(*target)) return *existing;` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:136` | `if (Window* owner = bound_window()) owner->verify_access("HelpProvider string query");` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:137` | `const Entry* entry = find_entry(target);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:155` | `if (Window* owner = bound_window()) owner->verify_access("HelpProvider keyword query");` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:156` | `const Entry* entry = find_entry(target);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:172` | `if (Window* owner = bound_window()) owner->verify_access("HelpProvider navigator query");` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:173` | `const Entry* entry = find_entry(target);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:186` | `if (Window* owner = bound_window()) owner->verify_access("HelpProvider show-help query");` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:187` | `const Entry* entry = find_entry(target);` |
| `<file/function>` | `Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:193` | `Entry* entry = find_entry(target);` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:229` | `const Entry* entry = find_entry(*target);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:248` | `Window* owner = bound_window();` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:253` | `const Entry* entry = find_entry(*current);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:288` | `if (Window* owner = bound_window()) owner->verify_access("HelpProvider disposal");` |
| `<file/function>` | `const PropertyValueConverter` | local/signature/use | `src/controls/inspection/property_value_converter_registry/property_value_converter_registry.cpp:64` | `const PropertyValueConverter* PropertyValueConverterRegistry::find(` |
| `<file/function>` | `const PropertyValueConverter` | local/signature/use | `src/controls/inspection/property_value_converter_registry/property_value_converter_registry.cpp:79` | `const PropertyValueConverter* converter = find(service);` |
| `<file/function>` | `const PropertyValueConverter` | local/signature/use | `src/controls/inspection/property_value_converter_registry/property_value_converter_registry.cpp:109` | `const PropertyValueConverter* converter = find(service);` |
| `ContextMenu` | `Window` | class declaration | `src/controls/menu/context_menu/context_menu.cpp:652` | `Window* window{};` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/panel/anchored_popup_layer/anchored_popup_layer.cpp:89` | `if (Window* owner = window()) {` |
| `<file/function>` | `const char` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:25` | `void require_instrument_text(std::string_view value, const char* field) {` |
| `<file/function>` | `const char` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:32` | `void require_finite_positive(double value, const char* field) {` |
| `InstrumentRack` | `ModuleState` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:120` | `ModuleState* find_module(std::string_view id) noexcept {` |
| `InstrumentRack` | `const ModuleState` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:125` | `const ModuleState* find_module(std::string_view id) const noexcept {` |
| `InstrumentRack` | `InstrumentFieldSpec` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:137` | `InstrumentFieldSpec* find_field(ModuleState& module,` |
| `InstrumentRack` | `ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:159` | `ModuleState* module = find_module(module_id);` |
| `InstrumentRack` | `ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:224` | `ModuleState* module = find_module(module_id);` |
| `InstrumentRack` | `InstrumentFieldSpec` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:227` | `InstrumentFieldSpec* field = find_field(*module, field_id);` |
| `InstrumentRack` | `ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:252` | `ModuleState* module = find_module(module_id);` |
| `InstrumentRack` | `InstrumentFieldSpec` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:254` | `InstrumentFieldSpec* field = find_field(*module, field_id);` |
| `InstrumentRack` | `ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:273` | `ModuleState* module = find_module(module_id);` |
| `InstrumentRack` | `InstrumentFieldSpec` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:275` | `InstrumentFieldSpec* field = find_field(*module, field_id);` |
| `InstrumentRack` | `ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:305` | `ModuleState* module = find_module(module_id);` |
| `InstrumentRack` | `InstrumentFieldSpec` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:308` | `InstrumentFieldSpec* field = find_field(*module, field_id);` |
| `InstrumentRack` | `ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:376` | `ModuleState* module = find_module(module_id);` |
| `Slot` | `ModuleState` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:497` | `ModuleState* module{};` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:643` | `if (Window* window = attached_window()) focused = window->focused_control();` |
| `<file/function>` | `const Impl::ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:705` | `const Impl::ModuleState* module = impl_->find_module(module_id);` |
| `<file/function>` | `const Impl::ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:717` | `const Impl::ModuleState* module = impl_->find_module(module_id);` |
| `<file/function>` | `Impl::ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:724` | `Impl::ModuleState* module = impl_->find_module(module_id);` |
| `<file/function>` | `Impl::ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:739` | `Impl::ModuleState* module = impl_->find_module(module_id);` |
| `<file/function>` | `Impl::ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:758` | `Impl::ModuleState* module = impl_->find_module(module_id);` |
| `<file/function>` | `InstrumentFieldSpec` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:760` | `InstrumentFieldSpec* field = impl_->find_field(*module, field_id);` |
| `<file/function>` | `Impl::ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:794` | `Impl::ModuleState* module = impl_->find_module(module_id);` |
| `<file/function>` | `InstrumentFieldSpec` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:796` | `InstrumentFieldSpec* field = impl_->find_field(*module, field_id);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:931` | `Window* host = attached_window();` |
| `PropertyList` | `RowState` | class declaration | `src/controls/panel/property_list/property_list.cpp:58` | `RowState* find_row(std::string_view id) noexcept {` |
| `PropertyList` | `const RowState` | class declaration | `src/controls/panel/property_list/property_list.cpp:63` | `const RowState* find_row(std::string_view id) const noexcept {` |
| `PropertyList` | `const PropertyRowSpec` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:70` | `const PropertyRowSpec* current = &spec(state);` |
| `PropertyList` | `const RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:73` | `const RowState* parent = find_row(current->parent_id);` |
| `PropertyList` | `RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:170` | `RowState* row = find_row(row_id);` |
| `PropertyList` | `RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:181` | `RowState* row = find_row(row_id);` |
| `PropertyList` | `RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:198` | `RowState* row = find_row(row_id);` |
| `PropertyList` | `RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:215` | `RowState* row = find_row(row_id);` |
| `PropertyList` | `RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:232` | `RowState* row = find_row(row_id);` |
| `PropertyList` | `RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:370` | `RowState* row = find_row(row_id);` |
| `PropertyList` | `RowState` | class declaration | `src/controls/panel/property_list/property_list.cpp:404` | `RowState* disclosure_at(Point absolute) noexcept {` |
| `<file/function>` | `Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:526` | `Impl::RowState* row = impl_->find_row(row_id);` |
| `<file/function>` | `Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:568` | `Impl::RowState* row = impl_->find_row(row_id);` |
| `<file/function>` | `Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:588` | `Impl::RowState* row = impl_->find_row(row_id);` |
| `<file/function>` | `Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:610` | `Impl::RowState* row = impl_->find_row(row_id);` |
| `<file/function>` | `Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:638` | `Impl::RowState* row = impl_->find_row(id);` |
| `<file/function>` | `const Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:653` | `const Impl::RowState* row = impl_->find_row(id);` |
| `<file/function>` | `const Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:660` | `const Impl::RowState* row = impl_->find_row(id);` |
| `<file/function>` | `const Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:666` | `const Impl::RowState* row = impl_->find_row(id);` |
| `<file/function>` | `Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:672` | `Impl::RowState* row = impl_->find_row(id);` |
| `<file/function>` | `const Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:715` | `const Impl::RowState* row = impl_->find_row(id);` |
| `<file/function>` | `Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:910` | `if (Impl::RowState* row = impl_->disclosure_at(event.position)) {` |
| `<file/function>` | `const Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:1025` | `const Impl::RowState* row = impl_->find_row(id);` |
| `<file/function>` | `const char` | local/signature/use | `src/controls/panel/property_list/property_list_utilities.hpp:29` | `inline void require_property_text(std::string_view value, const char* field) {` |
| `<file/function>` | `const char` | local/signature/use | `src/controls/range_control/range_control_rendering.cpp:12` | `void require_finite(double value, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `src/controls/range_control/range_control_rendering.hpp:13` | `void require_finite(double value, const char* message);` |
| `<file/function>` | `const Window` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:32` | `const Window* owner = window();` |
| `<file/function>` | `const Control` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:57` | `for (const Control* current = this; current != nullptr;) {` |
| `<file/function>` | `const char` | local/signature/use | `src/controls/scrollable_control/container_control/split_container/split_container.cpp:18` | `void require_finite_nonnegative(double value, const char* message) {` |
| `<file/function>` | `const TableLayoutPanel::CellMetadata` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:178` | `const TableLayoutPanel::CellMetadata* TableLayoutPanel::metadata_for(` |
| `<file/function>` | `const CellMetadata` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:212` | `const CellMetadata* metadata = metadata_for(child);` |
| `<file/function>` | `const CellMetadata` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:228` | `const CellMetadata* metadata = metadata_for(child);` |
| `<file/function>` | `const CellMetadata` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:244` | `const CellMetadata* metadata = metadata_for(child);` |
| `<file/function>` | `const CellMetadata` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:339` | `const CellMetadata* metadata = std::as_const(*this).metadata_for(*child);` |
| `<file/function>` | `const CellMetadata` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:361` | `const CellMetadata* metadata = std::as_const(*this).metadata_for(*child);` |
| `<file/function>` | `const CellMetadata` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:540` | `const CellMetadata* metadata = metadata_for(*child);` |
| `<file/function>` | `const char` | local/signature/use | `src/controls/scrollable_control/scrollable_control.cpp:22` | `void ScrollableControl::validate_size(Size size, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `src/controls/scrollable_control/scrollable_control.cpp:30` | `const char* message) {` |
| `<file/function>` | `HostServices` | local/signature/use | `src/controls/showcase_controls.cpp:340` | `HostServices* services = source.attached_window() == nullptr` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/showcase_controls.cpp:1495` | `if (Window* window = grid->attached_window()) {` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/showcase_controls.cpp:1524` | `if (Window* window = grid->attached_window()) {` |
| `<file/function>` | `HostServices` | local/signature/use | `src/controls/showcase_controls.cpp:2358` | `HostServices* services = source.attached_window() == nullptr` |
| `<file/function>` | `HostServices` | local/signature/use | `src/controls/showcase_controls.cpp:2461` | `HostServices* services = source.attached_window() == nullptr` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:55` | `Window* ToolTip::bound_window() const noexcept {` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:64` | `Window* owner = bound_window();` |
| `<file/function>` | `ToolTip::Entry` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:71` | `ToolTip::Entry* ToolTip::find_entry(const Control& target) {` |
| `<file/function>` | `const ToolTip::Entry` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:76` | `const ToolTip::Entry* ToolTip::find_entry(const Control& target) const {` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:84` | `Window* owner = bound_window();` |
| `<file/function>` | `Entry` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:97` | `if (Entry* existing = find_entry(*target)) {` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:125` | `const Entry* entry = find_entry(target);` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:224` | `const Entry* entry = find_entry(*target);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:280` | `Window* owner = bound_window();` |
| `<file/function>` | `Entry` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:281` | `Entry* entry = target ? find_entry(*target) : nullptr;` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:324` | `Window* owner = bound_window();` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:366` | `if (const Entry* entry = find_entry(*target)) text = entry->text;` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:392` | `if (const Entry* entry = find_entry(*target)) text = entry->text;` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:408` | `if (Window* owner = bound_window()) owner->verify_access("ToolTip disposal");` |
| `<file/function>` | `const BindableProperty` | local/signature/use | `src/core/binding/binding/binding.cpp:55` | `const BindableProperty* property = target_->find_bindable_property(property_name_);` |
| `<file/function>` | `const BindableProperty` | local/signature/use | `src/core/binding/binding/binding.cpp:141` | `const BindableProperty* property =` |
| `<file/function>` | `const BindableProperty` | local/signature/use | `src/core/binding/binding/binding.cpp:210` | `const BindableProperty* property =` |
| `<file/function>` | `Window` | local/signature/use | `src/core/binding/binding/binding.cpp:287` | `if (Window* owner = source->bound_window()) {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:36` | `Window* BindingContext::bound_window() const noexcept {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:43` | `Window* owner = bound_window();` |
| `<file/function>` | `BindingSource` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:56` | `BindingSource* key = source.get();` |
| `<file/function>` | `Window` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:75` | `if (Window* owner = bound_window()) owner->verify_access("BindingContext removal");` |
| `<file/function>` | `BindingSource` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:79` | `bool BindingContext::remove_entry(BindingSource* source, bool publish) {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:93` | `if (Window* owner = bound_window()) owner->verify_access("BindingContext clear");` |
| `<file/function>` | `BindingSource` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:100` | `for (BindingSource* source : removed) {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:107` | `if (Window* owner = bound_window()) owner->verify_access("BindingContext disposal");` |
| `<file/function>` | `Window` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:36` | `Window* BindingSource::bound_window() const noexcept {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:45` | `Window* owner = bound_window();` |
| `<file/function>` | `const BindingRecord` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:99` | `if (const BindingRecord* before = current()) previous_id = before->stable_id;` |
| `<file/function>` | `const BindingRecord` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:118` | `const BindingRecord* after = current();` |
| `<file/function>` | `const BindingRecord` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:128` | `const BindingRecord* BindingSource::current() const noexcept {` |
| `<file/function>` | `const BindingRecord` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:135` | `const BindingRecord* record = current();` |
| `<file/function>` | `const BindingRecord` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:143` | `const BindingRecord* record = current();` |
| `<file/function>` | `const BindingRecord` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:200` | `const BindingRecord* record = current();` |
| `<file/function>` | `BindingRecord` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:210` | `BindingRecord* record = position_ < 0 ? nullptr` |
| `<file/function>` | `const BindingRecord` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:429` | `if (const BindingRecord* record = current()) {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:465` | `if (Window* owner = bound_window()) owner->verify_access("BindingSource disposal");` |
| `<file/function>` | `const Binding` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:499` | `void BindingSource::unregister_binding(const Binding* binding) noexcept {` |
| `<file/function>` | `const BindingRecord` | local/signature/use | `src/core/binding/currency_manager/currency_manager.cpp:25` | `const BindingRecord* CurrencyManager::current() const noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/binding/value/binding_value.cpp:78` | `const char* begin = text.data();` |
| `<file/function>` | `const char` | local/signature/use | `src/core/binding/value/binding_value.cpp:79` | `const char* end = begin + text.size();` |
| `<file/function>` | `const PropertyEnumChoice` | local/signature/use | `src/core/binding/value/binding_value.cpp:171` | `const PropertyEnumChoice* enum_choice_by_name(` |
| `<file/function>` | `const PropertyEnumChoice` | local/signature/use | `src/core/binding/value/binding_value.cpp:181` | `const PropertyEnumChoice* enum_choice_by_value(` |
| `<file/function>` | `const PropertyEnumChoice` | local/signature/use | `src/core/binding/value/binding_value.cpp:215` | `const PropertyEnumChoice* choice = enum_choice_by_name(` |
| `<file/function>` | `const PropertyEnumChoice` | local/signature/use | `src/core/binding/value/binding_value.cpp:229` | `const PropertyEnumChoice* choice = enum_choice_by_name(` |
| `<file/function>` | `const PropertyEnumChoice` | local/signature/use | `src/core/binding/value/binding_value.cpp:250` | `if (const PropertyEnumChoice* exact = enum_choice_by_value(` |
| `<file/function>` | `const BindableProperty` | local/signature/use | `src/core/control/control/control.cpp:708` | `const BindableProperty* Control::find_bindable_property(` |
| `<file/function>` | `const char` | local/signature/use | `src/core/control/control/control.cpp:924` | `void validate_insets(Insets value, const char* message) {` |
| `<file/function>` | `Control` | local/signature/use | `src/core/control/control/control.cpp:1598` | `bool Control::perform_validation(Control* destination, bool bulk) {` |
| `<file/function>` | `const Control` | local/signature/use | `src/core/control/control/control.cpp:1672` | `const Control* current = this;` |
| `<file/function>` | `Control` | local/signature/use | `src/core/control/control/control.cpp:1769` | `for (Control* current = this; current != nullptr;) {` |
| `<file/function>` | `Control` | local/signature/use | `src/core/control/control/control.cpp:1800` | `for (Control* current = this; current != nullptr;) {` |
| `<file/function>` | `const void` | local/signature/use | `src/core/control/control/control.cpp:1878` | `void Control::publish_change(const void* event_key,` |
| `<file/function>` | `const Control` | local/signature/use | `src/core/control/control/control.cpp:2300` | `const Control* current = this;` |
| `<file/function>` | `const std::byte` | local/signature/use | `src/core/drawing/bitmap/bitmap.cpp:305` | `const std::byte* current = storage_->bytes.data() + storage_row;` |
| `<file/function>` | `const std::byte` | local/signature/use | `src/core/drawing/bitmap/bitmap.cpp:306` | `const std::byte* original = active_edit_backup_.data() +` |
| `<file/function>` | `const char` | local/signature/use | `src/core/drawing/support/drawing_support.hpp:164` | `[[nodiscard]] const char* command_name(CommandKind kind) noexcept {` |
| `<file/function>` | `HostServices` | local/signature/use | `src/core/feedback/semantic_feedback/semantic_feedback.cpp:60` | `if (HostServices* services = window_->host_services()) {` |
| `<file/function>` | `HostServices` | local/signature/use | `src/core/host/session/host_session.cpp:152` | `HostServices* services)` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_types.cpp:13` | `const char* host_dispatch_error_name(HostDispatchError error) noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_types.cpp:28` | `const char* host_lifecycle_phase_name(HostLifecyclePhase phase) noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_types.cpp:39` | `const char* host_event_name(const HostEventPayload& payload) noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_types.cpp:59` | `const char* host_service_error_name(HostServiceError error) noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_types.cpp:74` | `const char* host_sound_cue_name(HostSoundCue cue) noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_types.cpp:85` | `const char* cursor_kind_name(CursorKind cursor) noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_types.cpp:99` | `const char* drag_effect_name(DragEffect effect) noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_types.cpp:109` | `const char* host_dialog_kind_name(const HostDialogRequestPayload& payload) noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_types.cpp:127` | `const char* host_dialog_outcome_name(HostDialogOutcome outcome) noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_types.cpp:131` | `const char* host_dialog_choice_name(HostDialogChoice choice) noexcept {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:43` | `if (Window* owner = bound_window(); owner && owner->check_access()) {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:49` | `Window* ImageList::bound_window() const noexcept {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:62` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:140` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:162` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:201` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:283` | `if (Window* owner = bound_window()) {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:317` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:378` | `Window* owner = bound_window();` |
| `<file/function>` | `const Variant` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:400` | `const Variant* exact{};` |
| `<file/function>` | `const Variant` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:401` | `const Variant* larger{};` |
| `<file/function>` | `const Variant` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:402` | `const Variant* smaller{};` |
| `<file/function>` | `const Variant` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:415` | `const Variant* chosen = exact ? exact : (larger ? larger : smaller);` |
| `<file/function>` | `Window` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:443` | `if (Window* owner = bound_window()) owner->verify_access("ImageList disposal");` |
| `<file/function>` | `const std::byte` | local/signature/use | `src/core/resources/image_registry/image_registry.cpp:34` | `std::uint32_t read_u32(const std::byte* bytes) noexcept {` |
| `<file/function>` | `const std::byte` | local/signature/use | `src/core/resources/image_registry/image_registry.cpp:193` | `const std::byte* const type_bytes = encoded.data() + offset + 4U;` |
| `<file/function>` | `const char` | local/signature/use | `src/core/semantics/snapshot/semantic_snapshot.cpp:63` | `const char* semantic_role_name(SemanticRole role) noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/semantics/snapshot/semantic_snapshot.cpp:101` | `const char* semantic_action_name(SemanticAction action) noexcept {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/timer/timer/timer.cpp:25` | `Window* Timer::bound_window() const noexcept {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/timer/timer/timer.cpp:34` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/core/timer/timer/timer.cpp:71` | `Window* owner = bound_window();` |
| `<file/function>` | `Timer` | local/signature/use | `src/core/timer/timer/timer.cpp:80` | `Timer* timer = state ? state->owner : nullptr;` |
| `<file/function>` | `Window` | local/signature/use | `src/core/timer/timer/timer.cpp:93` | `if (Window* owner = bound_window()) {` |
| `AcceleratorAttachment` | `Component` | class declaration | `src/core/window/accelerator/accelerator_attachment.hpp:19` | `[[nodiscard]] Component* owner() const noexcept { return owner_; }` |
| `AcceleratorAttachment` | `Window` | class declaration | `src/core/window/accelerator/accelerator_attachment.hpp:31` | `Window* window_{};` |
| `AcceleratorAttachment` | `Component` | class declaration | `src/core/window/accelerator/accelerator_attachment.hpp:32` | `Component* owner_{};` |
| `PopupAttachment` | `Window` | class declaration | `src/core/window/popup/popup_attachment.hpp:31` | `Window* window_{};` |
| `<file/function>` | `Window` | local/signature/use | `src/core/window/update_scope/update_scope.cpp:32` | `Window* closing = std::exchange(window_, nullptr);` |
| `<file/function>` | `const Control` | local/signature/use | `src/core/window/window.cpp:66` | `const Control::Ptr& control, const Control* expected_parent,` |
| `<file/function>` | `const Window` | local/signature/use | `src/core/window/window.cpp:67` | `const Window* owner) noexcept {` |
| `<file/function>` | `const Control` | local/signature/use | `src/core/window/window.cpp:74` | `const Control* expected_parent,` |
| `<file/function>` | `const Window` | local/signature/use | `src/core/window/window.cpp:75` | `const Window* owner,` |
| `<file/function>` | `Window` | local/signature/use | `src/core/window/window.cpp:137` | `Window* const owner = control->attached_window();` |
| `<file/function>` | `Component` | local/signature/use | `src/core/window/window.cpp:296` | `Component* owner = accelerator->owner();` |
| `<file/function>` | `Control` | local/signature/use | `src/core/window/window.cpp:701` | `Control* destination, bool bulk) {` |
| `<file/function>` | `Control` | local/signature/use | `src/core/window/window.cpp:2352` | `Control* const retained_parent = control->parent_.lock().get();` |
| `<file/function>` | `Control` | local/signature/use | `src/core/window/window.cpp:2499` | `for (Control* current = &control; current != nullptr;) {` |
| `<file/function>` | `Control` | local/signature/use | `src/core/window/window.cpp:2566` | `Control* const retained_parent = control->parent_.lock().get();` |
| `HeadlessHost` | `Window` | class declaration | `src/host/headless/session/headless_host.hpp:41` | `Window* window_{};` |
| `<file/function>` | `NSEvent` | local/signature/use | `src/host/macos/application/macos_host.mm:92` | `static std::uint64_t host_event_nanoseconds(NSEvent* event) {` |
| `<file/function>` | `NSPasteboard` | local/signature/use | `src/host/macos/application/macos_host.mm:259` | `static std::vector<DragDataItem> drag_items_for(NSPasteboard* pasteboard) {` |
| `<file/function>` | `NSArray<NSURL*>` | local/signature/use | `src/host/macos/application/macos_host.mm:262` | `NSArray<NSURL*>* urls = [pasteboard` |
| `<file/function>` | `NSURL` | local/signature/use | `src/host/macos/application/macos_host.mm:269` | `for (NSURL* url in urls) {` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:270` | `NSString* native_path = url.path;` |
| `<file/function>` | `const char` | local/signature/use | `src/host/macos/application/macos_host.mm:277` | `const char* path = native_path.UTF8String;` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:287` | `NSString* text = [pasteboard stringForType:NSPasteboardTypeString];` |
| `<file/function>` | `const char` | local/signature/use | `src/host/macos/application/macos_host.mm:295` | `const char* bytes = text.UTF8String;` |
| `<file/function>` | `NSData` | local/signature/use | `src/host/macos/application/macos_host.mm:304` | `NSData* data = [pasteboard dataForType:type];` |
| `<file/function>` | `const char` | local/signature/use | `src/host/macos/application/macos_host.mm:305` | `const char* media = type.UTF8String;` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:322` | `NSString* string = nil;` |
| `<file/function>` | `const char` | local/signature/use | `src/host/macos/application/macos_host.mm:331` | `const char* bytes = [string UTF8String];` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:336` | `NSString* resource_name,` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:340` | `NSString* extension = @"ttf") {` |
| `<file/function>` | `NSURL` | local/signature/use | `src/host/macos/application/macos_host.mm:341` | `NSURL* url = [[NSBundle mainBundle] URLForResource:resource_name` |
| `<file/function>` | `NSData` | local/signature/use | `src/host/macos/application/macos_host.mm:347` | `NSData* data = [NSData dataWithContentsOfURL:url` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:359` | `NSString* resource_name,` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:360` | `NSString* extension,` |
| `<file/function>` | `NSURL` | local/signature/use | `src/host/macos/application/macos_host.mm:363` | `NSURL* url = [[NSBundle mainBundle] URLForResource:resource_name` |
| `<file/function>` | `NSData` | local/signature/use | `src/host/macos/application/macos_host.mm:367` | `NSData* data = [NSData dataWithContentsOfURL:url` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:378` | `NSString* native_string(std::string_view text) {` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:384` | `NSString* native_accessibility_role(SemanticRole role) {` |
| `<file/function>` | `NSMutableAttributedString` | local/signature/use | `src/host/macos/application/macos_host.mm:442` | `NSMutableAttributedString* _markedText;` |
| `<file/function>` | `NSTrackingArea` | local/signature/use | `src/host/macos/application/macos_host.mm:444` | `NSTrackingArea* _trackingArea;` |
| `<file/function>` | `NSPanel` | local/signature/use | `src/host/macos/application/macos_host.mm:450` | `NSPanel* _tooltipPanel;` |
| `<file/function>` | `NSTimer` | local/signature/use | `src/host/macos/application/macos_host.mm:451` | `NSTimer* _tooltipTimer;` |
| `<file/function>` | `NSArray` | local/signature/use | `src/host/macos/application/macos_host.mm:455` | `NSArray* _semanticAccessibilityChildren;` |
| `<file/function>` | `NSMutableDictionary<NSString*, GUIFormsAccessibilityElement*>` | local/signature/use | `src/host/macos/application/macos_host.mm:456` | `NSMutableDictionary<NSString*, GUIFormsAccessibilityElement*>*` |
| `<file/function>` | `void` | local/signature/use | `src/host/macos/application/macos_host.mm:499` | `CVOptionFlags, CVOptionFlags*, void* context) {` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:501` | `GUIFormsView* view = (__bridge GUIFormsView*)context;` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:508` | `__weak GUIFormsView* _owner;` |
| `<file/function>` | `NSArray` | local/signature/use | `src/host/macos/application/macos_host.mm:510` | `NSArray* _semanticChildren;` |
| `<file/function>` | `NSMutableArray` | local/signature/use | `src/host/macos/application/macos_host.mm:531` | `NSMutableArray* children = [[NSMutableArray alloc]` |
| `<file/function>` | `GUIFormsAccessibilityElement` | local/signature/use | `src/host/macos/application/macos_host.mm:534` | `GUIFormsAccessibilityElement* child =` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:658` | `NSString* text = [value isKindOfClass:[NSString class]]` |
| `<file/function>` | `GUIFormsAccessibilityElement` | local/signature/use | `src/host/macos/application/macos_host.mm:710` | `static GUIFormsAccessibilityElement* reconcile_accessibility_element(` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:712` | `GUIFormsView* owner,` |
| `<file/function>` | `NSDictionary<NSString*, GUIFormsAccessibilityElement*>` | local/signature/use | `src/host/macos/application/macos_host.mm:714` | `NSDictionary<NSString*, GUIFormsAccessibilityElement*>* previous,` |
| `<file/function>` | `NSMutableDictionary<NSString*, GUIFormsAccessibilityElement*>` | local/signature/use | `src/host/macos/application/macos_host.mm:715` | `NSMutableDictionary<NSString*, GUIFormsAccessibilityElement*>* next) {` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:716` | `NSString* identifier = native_string(node.stable_id);` |
| `<file/function>` | `GUIFormsAccessibilityElement` | local/signature/use | `src/host/macos/application/macos_host.mm:717` | `GUIFormsAccessibilityElement* element = previous[identifier];` |
| `<file/function>` | `NSMutableArray` | local/signature/use | `src/host/macos/application/macos_host.mm:726` | `NSMutableArray* children = [[NSMutableArray alloc]` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:767` | `__weak GUIFormsView* weakSelf = self;` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:769` | `GUIFormsView* strongSelf = weakSelf;` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:779` | `GUIFormsView* strongSelf = weakSelf;` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:785` | `GUIFormsView* strongSelf = weakSelf;` |
| `<file/function>` | `NSMutableArray` | local/signature/use | `src/host/macos/application/macos_host.mm:957` | `NSMutableArray* result = [[NSMutableArray alloc]` |
| `<file/function>` | `NSMutableDictionary<NSString*, GUIFormsAccessibilityElement*>` | local/signature/use | `src/host/macos/application/macos_host.mm:959` | `NSMutableDictionary<NSString*, GUIFormsAccessibilityElement*>* next =` |
| `<file/function>` | `GUIFormsAccessibilityElement` | local/signature/use | `src/host/macos/application/macos_host.mm:962` | `GUIFormsAccessibilityElement* element = reconcile_accessibility_element(` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:1068` | `NSString* text = native_string(request.text);` |
| `<file/function>` | `NSFont` | local/signature/use | `src/host/macos/application/macos_host.mm:1070` | `NSFont* font = [NSFont fontWithName:@"Lucida Grande" size:12.0];` |
| `<file/function>` | `NSDictionary` | local/signature/use | `src/host/macos/application/macos_host.mm:1072` | `NSDictionary* attributes = @{NSFontAttributeName: font};` |
| `<file/function>` | `NSTextField` | local/signature/use | `src/host/macos/application/macos_host.mm:1080` | `NSTextField* label = [[NSTextField alloc]` |
| `<file/function>` | `NSPanel` | local/signature/use | `src/host/macos/application/macos_host.mm:1093` | `NSPanel* panel = [[NSPanel alloc]` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:1212` | `__weak GUIFormsView* weakSelf = self;` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:1214` | `GUIFormsView* strongSelf = weakSelf;` |
| `<file/function>` | `NSException` | local/signature/use | `src/host/macos/application/macos_host.mm:1286` | `} @catch (NSException* exception) {` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:1287` | `NSString* name = exception.name == nil ? @"NSException" : exception.name;` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:1288` | `NSString* reason = exception.reason == nil ? @"no reason" : exception.reason;` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:1289` | `NSString* diagnostic = [NSString stringWithFormat:@"%@: %@", name, reason];` |
| `<file/function>` | `const void` | local/signature/use | `src/host/macos/application/macos_host.mm:1419` | `const void* pixels = _raster.pixels();` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:1591` | `NSString* plain = [string isKindOfClass:[NSAttributedString class]]` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:1679` | `__weak GUIFormsView* _view;` |
| `<file/function>` | `NSEvent` | local/signature/use | `src/host/macos/application/macos_host.mm:1712` | `NSEvent* wake = [NSEvent otherEventWithType:NSEventTypeApplicationDefined` |
| `<file/function>` | `NSWindow` | local/signature/use | `src/host/macos/application/macos_host.mm:1730` | `NSWindow* window = notification.object;` |
| `<file/function>` | `NSMenu` | local/signature/use | `src/host/macos/application/macos_host.mm:1740` | `NSMenu* menuBar = [[NSMenu alloc] init];` |
| `<file/function>` | `NSMenuItem` | local/signature/use | `src/host/macos/application/macos_host.mm:1741` | `NSMenuItem* applicationItem = [[NSMenuItem alloc] init];` |
| `<file/function>` | `NSMenu` | local/signature/use | `src/host/macos/application/macos_host.mm:1743` | `NSMenu* applicationMenu = [[NSMenu alloc] init];` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:1744` | `NSString* quitTitle = @"Quit GUI.Forms Gallery";` |
| `<file/function>` | `NSMenuItem` | local/signature/use | `src/host/macos/application/macos_host.mm:1745` | `NSMenuItem* quit = [[NSMenuItem alloc] initWithTitle:quitTitle` |
| `<file/function>` | `NSApplication` | local/signature/use | `src/host/macos/application/macos_host.mm:1780` | `NSApplication* application = [NSApplication sharedApplication];` |
| `<file/function>` | `NSWindow` | local/signature/use | `src/host/macos/application/macos_host.mm:1790` | `NSWindow* nativeWindow = [[NSWindow alloc]` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:1799` | `GUIFormsView* view = [[GUIFormsView alloc] initWithModel:std::move(model)];` |
| `<file/function>` | `GUIFormsWindowDelegate` | local/signature/use | `src/host/macos/application/macos_host.mm:1801` | `GUIFormsWindowDelegate* delegate =` |
| `<file/function>` | `NSApplication` | local/signature/use | `src/host/macos/application/macos_host.mm:1910` | `NSApplication* application = [NSApplication sharedApplication];` |
| `<file/function>` | `NSMutableArray<NSWindow*>` | local/signature/use | `src/host/macos/application/macos_host.mm:1914` | `NSMutableArray<NSWindow*>* nativeWindows =` |
| `<file/function>` | `NSMutableArray<GUIFormsView*>` | local/signature/use | `src/host/macos/application/macos_host.mm:1916` | `NSMutableArray<GUIFormsView*>* views =` |
| `<file/function>` | `NSMutableArray<GUIFormsWindowDelegate*>` | local/signature/use | `src/host/macos/application/macos_host.mm:1918` | `NSMutableArray<GUIFormsWindowDelegate*>* delegates =` |
| `<file/function>` | `NSWindow` | local/signature/use | `src/host/macos/application/macos_host.mm:1929` | `NSWindow* nativeWindow = entry.tool_window` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:1937` | `GUIFormsView* view =` |
| `<file/function>` | `GUIFormsWindowDelegate` | local/signature/use | `src/host/macos/application/macos_host.mm:1941` | `GUIFormsWindowDelegate* delegate =` |
| `<file/function>` | `NSWindow` | local/signature/use | `src/host/macos/application/macos_host.mm:1957` | `NSWindow* nativeWindow = nativeWindows[index];` |
| `<file/function>` | `NSWindow` | local/signature/use | `src/host/macos/application/macos_host.mm:1968` | `NSWindow* ownerWindow = nativeWindows[ownerIndex];` |
| `<file/function>` | `NSScreen` | local/signature/use | `src/host/macos/application/macos_host.mm:1970` | `NSScreen* screen = ownerWindow.screen ?: NSScreen.mainScreen;` |
| `<file/function>` | `NSWindow` | local/signature/use | `src/host/macos/application/macos_host.mm:1983` | `NSWindow* nativeWindow = nativeWindows[index];` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:1984` | `GUIFormsView* view = views[index];` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:1986` | `for (GUIFormsView* initializedView in views) {` |
| `<file/function>` | `NSWindow` | local/signature/use | `src/host/macos/application/macos_host.mm:1989` | `for (NSWindow* createdWindow in nativeWindows) {` |
| `<file/function>` | `NSWindow` | local/signature/use | `src/host/macos/application/macos_host.mm:2043` | `for (NSWindow* nativeWindow in nativeWindows) {` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:2049` | `GUIFormsView* view = views[index];` |
| `<file/function>` | `NSWindow` | local/signature/use | `src/host/macos/application/macos_host.mm:2050` | `NSWindow* nativeWindow = nativeWindows[index];` |
| `<file/function>` | `NSCursor` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:89` | `NSCursor* native_cursor(CursorKind cursor) {` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:103` | `NSString* native_string(std::string_view text) {` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:110` | `NSString* string = nil;` |
| `<file/function>` | `const char` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:117` | `const char* bytes = string.UTF8String;` |
| `<file/function>` | `NSURL` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:121` | `NSURL* native_directory_url(const std::string& path) {` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:122` | `NSString* value = native_string(path);` |
| `<file/function>` | `NSArray<UTType*>` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:126` | `NSArray<UTType*>* native_allowed_types(` |
| `<file/function>` | `NSMutableArray<UTType*>` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:128` | `NSMutableArray<UTType*>* types = [[NSMutableArray alloc] init];` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:135` | `NSString* value = native_string(normalized);` |
| `<file/function>` | `UTType` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:136` | `UTType* type = value.length == 0` |
| `<file/function>` | `NSArray<NSURL*>` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:147` | `NSArray<NSURL*>* urls) {` |
| `<file/function>` | `NSURL` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:153` | `for (NSURL* url in urls) {` |
| `<file/function>` | `NSSavePanel` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:162` | `void schedule_test_panel_cancel(NSSavePanel* panel, bool enabled) {` |
| `<file/function>` | `NSAlert` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:171` | `void schedule_test_alert_cancel(NSAlert* alert, bool enabled) {` |
| `AppKitHostServices` | `NSString` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:189` | `NSString* name = [[NSString alloc]` |
| `AppKitHostServices` | `NSArray<NSScreen*>` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:203` | `NSArray<NSScreen*>* screens = [NSScreen screens];` |
| `AppKitHostServices` | `NSScreen` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:204` | `NSScreen* primary = screens.firstObject;` |
| `AppKitHostServices` | `NSScreen` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:206` | `for (NSScreen* screen in screens) {` |
| `AppKitHostServices` | `NSScreen` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:210` | `for (NSScreen* screen in screens) {` |
| `AppKitHostServices` | `NSNumber` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:213` | `NSNumber* screen_number = screen.deviceDescription[@"NSScreenNumber"];` |
| `AppKitHostServices` | `NSString` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:254` | `NSString* value = [pasteboard_ stringForType:NSPasteboardTypeString];` |
| `AppKitHostServices` | `NSData` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:258` | `NSData* encoded = [value dataUsingEncoding:NSUTF8StringEncoding];` |
| `AppKitHostServices` | `NSString` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:274` | `NSString* value = [[NSString alloc] initWithBytes:text_utf8.data()` |
| `AppKitHostServices` | `NSAlert` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:290` | `NSAlert* alert = [[NSAlert alloc] init];` |
| `AppKitHostServices` | `NSButton` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:329` | `NSButton* defaultButton = nil;` |
| `AppKitHostServices` | `NSButton` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:333` | `NSButton* button = [alert addButtonWithTitle:title];` |
| `AppKitHostServices` | `NSEvent` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:353` | `NSEventMaskKeyDown handler:^NSEvent* (NSEvent* event) {` |
| `AppKitHostServices` | `NSOpenPanel` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:373` | `NSOpenPanel* panel = [NSOpenPanel openPanel];` |
| `AppKitHostServices` | `NSArray<UTType*>` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:380` | `NSArray<UTType*>* types = native_allowed_types(payload.filters);` |
| `AppKitHostServices` | `NSSavePanel` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:388` | `NSSavePanel* panel = [NSSavePanel savePanel];` |
| `AppKitHostServices` | `NSArray<UTType*>` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:392` | `NSArray<UTType*>* types = native_allowed_types(payload.filters);` |
| `AppKitHostServices` | `NSArray<NSURL*>` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:403` | `NSArray<NSURL*>* urls = panel.URL == nil ? @[] : @[panel.URL];` |
| `AppKitHostServices` | `NSOpenPanel` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:406` | `NSOpenPanel* panel = [NSOpenPanel openPanel];` |
| `AppKitHostServices` | `NSAlert` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:416` | `NSAlert* alert = [[NSAlert alloc] init];` |
| `AppKitHostServices` | `NSColorWell` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:418` | `NSColorWell* well = [[NSColorWell alloc]` |
| `AppKitHostServices` | `NSColor` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:433` | `NSColor* color = [well.color colorUsingColorSpace:[NSColorSpace sRGBColorSpace]];` |
| `AppKitHostServices` | `NSString` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:453` | `NSString* name = @"Ping";` |
| `AppKitHostServices` | `NSSound` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:461` | `NSSound* sound = [NSSound soundNamed:name];` |
| `AppKitHostServices` | `NSPasteboard` | class declaration | `src/host/macos/services/appkit_host_services.mm:476` | `__strong NSPasteboard* pasteboard_{};` |
| `<file/function>` | `const char` | local/signature/use | `src/host/windows/application/windows_host.cpp:53` | `const char* value = std::getenv("GUI_FORMS_TRACE_WIN32_INPUT");` |
| `<file/function>` | `const char` | local/signature/use | `src/host/windows/application/windows_host.cpp:61` | `const char* value = std::getenv("GUI_FORMS_TRACE_WIN32_TEXT");` |
| `<file/function>` | `const wchar_t` | local/signature/use | `src/host/windows/application/windows_host.cpp:166` | `const wchar_t* cursor = path.data() + first.size() + 1U;` |
| `<file/function>` | `const char` | local/signature/use | `src/host/windows/application/windows_host.cpp:516` | `Function load_function(HMODULE module, const char* name) noexcept {` |
| `DibPainter` | `IWICImagingFactory` | local/signature/use | `src/host/windows/application/windows_host.cpp:601` | `IWICImagingFactory* factory{};` |
| `DibPainter` | `const wchar_t` | class declaration | `src/host/windows/application/windows_host.cpp:1306` | `[[nodiscard]] static const wchar_t* primary_font_family(FontRole role) {` |
| `DibPainter` | `const wchar_t` | class declaration | `src/host/windows/application/windows_host.cpp:1310` | `[[nodiscard]] HFONT create_font(FontSpec font, const wchar_t* family) const {` |
| `DibPainter` | `const wchar_t` | class declaration | `src/host/windows/application/windows_host.cpp:1488` | `[[nodiscard]] bool font_covers(HFONT font, const wchar_t* requested_family,` |
| `DibPainter` | `IWICStream` | local/signature/use | `src/host/windows/application/windows_host.cpp:1596` | `IWICStream* stream{};` |
| `DibPainter` | `IWICBitmapDecoder` | local/signature/use | `src/host/windows/application/windows_host.cpp:1597` | `IWICBitmapDecoder* decoder{};` |
| `DibPainter` | `IWICBitmapFrameDecode` | local/signature/use | `src/host/windows/application/windows_host.cpp:1598` | `IWICBitmapFrameDecode* frame{};` |
| `DibPainter` | `IWICFormatConverter` | local/signature/use | `src/host/windows/application/windows_host.cpp:1599` | `IWICFormatConverter* converter{};` |
| `DibPainter` | `const ImageRegistry` | class declaration | `src/host/windows/application/windows_host.cpp:1647` | `const ImageRegistry* image_registry_{};` |
| `WindowsHostState` | `const wchar_t` | local/signature/use | `src/host/windows/application/windows_host.cpp:2052` | `const wchar_t* locked = data == nullptr` |
| `WindowsHostState` | `void` | local/signature/use | `src/host/windows/application/windows_host.cpp:2083` | `void* destination = GlobalLock(storage);` |
| `WindowsHostState` | `const RECT` | local/signature/use | `src/host/windows/application/windows_host.cpp:2122` | `if (const RECT* suggested = reinterpret_cast<const RECT*>(lparam)) {` |
| `WindowsHostState` | `MINMAXINFO` | class declaration | `src/host/windows/application/windows_host.cpp:2329` | `void minimum_size(MINMAXINFO* info) const {` |
| `WindowsHostState` | `const COPYDATASTRUCT` | class declaration | `src/host/windows/application/windows_host.cpp:2350` | `LRESULT automation(const COPYDATASTRUCT* data) {` |
| `WindowsHostState` | `const char` | local/signature/use | `src/host/windows/application/windows_host.cpp:2353` | `const char* bytes = static_cast<const char*>(data->lpData);` |
| `WindowsHostState` | `const wchar_t` | local/signature/use | `src/host/windows/application/windows_host.cpp:2641` | `for (const wchar_t* name : names) {` |
| `WindowsCompatibilityPaintEndpoint` | `void` | local/signature/use | `src/host/windows/paint_endpoint/windows_compatibility_paint_endpoint.cpp:141` | `void* next_pixels{};` |
| `WindowsCompatibilityPaintEndpoint` | `const char` | local/signature/use | `src/host/windows/paint_endpoint/windows_compatibility_paint_endpoint.cpp:396` | `const char* state = released \|\| compatibility_handle == 0U` |
| `WindowsHostServices` | `const wchar_t` | local/signature/use | `src/host/windows/services/windows_host_services.hpp:76` | `const wchar_t* locked = data == nullptr` |
| `WindowsHostServices` | `void` | local/signature/use | `src/host/windows/services/windows_host_services.hpp:104` | `void* destination = GlobalLock(storage);` |
| `<file/function>` | `const char` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:79` | `Function forms_entry(const char* name) noexcept {` |
| `<file/function>` | `HDC` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:89` | `bool endpoint_dc(HWND window, HDC* output) noexcept {` |
| `<file/function>` | `HBITMAP` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:150` | `bool selected_dib32(HDC dc, HBITMAP* bitmap, DIBSECTION* section) noexcept {` |
| `<file/function>` | `DIBSECTION` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:150` | `bool selected_dib32(HDC dc, HBITMAP* bitmap, DIBSECTION* section) noexcept {` |
| `<file/function>` | `std::byte` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:175` | `std::byte* logical_dib_row(DIBSECTION& section, int y) noexcept {` |
| `<file/function>` | `BOOL` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:186` | `int source_y, DWORD operation, BOOL* result) noexcept {` |
| `<file/function>` | `std::byte` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:283` | `std::byte* destination_first =` |
| `<file/function>` | `std::byte` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:285` | `std::byte* destination_last =` |
| `<file/function>` | `std::byte` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:287` | `std::byte* source_first = logical_dib_row(source_section, source_y);` |
| `<file/function>` | `std::byte` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:288` | `std::byte* source_last =` |
| `<file/function>` | `std::byte` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:300` | `std::byte* destination_row =` |
| `<file/function>` | `std::byte` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:303` | `std::byte* source_row =` |
| `<file/function>` | `const char` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:312` | `void trace(const char* operation, const void* first,` |
| `<file/function>` | `const void` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:312` | `void trace(const char* operation, const void* first,` |
| `<file/function>` | `const void` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:313` | `const void* second = nullptr, long result = 0,` |
| `<file/function>` | `COLORREF` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:352` | `HWND window, COLORREF* color_key, BYTE* alpha, DWORD* flags) {` |
| `<file/function>` | `BYTE` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:352` | `HWND window, COLORREF* color_key, BYTE* alpha, DWORD* flags) {` |
| `<file/function>` | `DWORD` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:352` | `HWND window, COLORREF* color_key, BYTE* alpha, DWORD* flags) {` |
| `<file/function>` | `RECT` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:363` | `extern "C" BOOL WINAPI gf_compat_GetClientRect(HWND window, RECT* bounds) {` |
| `<file/function>` | `void` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:434` | `void* pixels{};` |
| `<file/function>` | `const POINT` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:480` | `HDC dc, const POINT* points, DWORD count) {` |
| `<file/function>` | `const void` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:535` | `const void* pixels, const BITMAPINFO* info, UINT usage, DWORD operation) {` |
| `<file/function>` | `const BITMAPINFO` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:535` | `const void* pixels, const BITMAPINFO* info, UINT usage, DWORD operation) {` |
| `<file/function>` | `const RECT` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:548` | `HDC dc, const RECT* rectangle, HBRUSH brush) {` |
| `<file/function>` | `const RECT` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:557` | `HDC dc, const RECT* rectangle, HBRUSH brush) {` |
| `<file/function>` | `const RECT` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:566` | `HWND window, int dx, int dy, const RECT* scroll, const RECT* clip,` |
| `<file/function>` | `const RECT` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:566` | `HWND window, int dx, int dy, const RECT* scroll, const RECT* clip,` |
| `<file/function>` | `const RECT` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:583` | `const RECT* effective_scroll = scroll != nullptr ? scroll : &surface_bounds;` |
| `<file/function>` | `const RECT` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:584` | `const RECT* effective_clip = clip != nullptr ? clip : &surface_bounds;` |
| `<file/function>` | `const PAINTSTRUCT` | local/signature/use | `src/host/windows/win32_compat_shim.cpp:611` | `HWND window, const PAINTSTRUCT* paint) {` |
| `CoreGraphicsRaster` | `const RegisteredTypeface` | local/signature/use | `src/render/coregraphics/raster/coregraphics_raster.cpp:147` | `const RegisteredTypeface* selected = nullptr;` |
| `CoreGraphicsRaster` | `const ImageRegistry` | class declaration | `src/render/coregraphics/raster/coregraphics_raster.cpp:180` | `const ImageRegistry* image_registry{};` |
| `<file/function>` | `const void` | local/signature/use | `src/render/coregraphics/raster/coregraphics_raster.cpp:443` | `const void* CoreGraphicsRaster::pixels() const noexcept {` |
| `<file/function>` | `const void` | local/signature/use | `src/render/coregraphics/raster/coregraphics_raster.cpp:720` | `const void* keys[]{kCTFontAttributeName, kCTForegroundColorAttributeName,` |
| `<file/function>` | `const void` | local/signature/use | `src/render/coregraphics/raster/coregraphics_raster.cpp:722` | `const void* values[]{font, foreground, tracking};` |
| `<file/function>` | `const void` | local/signature/use | `src/render/coregraphics/raster/coregraphics_raster.cpp:766` | `const void* keys[]{kCTFontAttributeName, kCTKernAttributeName};` |
| `<file/function>` | `const void` | local/signature/use | `src/render/coregraphics/raster/coregraphics_raster.cpp:767` | `const void* values[]{font, tracking};` |
| `CoreGraphicsRaster` | `const void` | class declaration | `src/render/coregraphics/raster/coregraphics_raster.hpp:30` | `[[nodiscard]] const void* pixels() const noexcept;` |
| `BitmapUnlock` | `Bitmap` | class declaration | `src/render/skia/executor/skia_executor.cpp:471` | `Bitmap* bitmap_;` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/executor/skia_executor.cpp:670` | `SkCanvas* canvas = surface->getCanvas();` |
| `<file/function>` | `RasterError` | local/signature/use | `src/render/skia/executor/skia_executor.cpp:900` | `const Bitmap& bitmap, RasterError* error, const PngCodecLimits& limits) {` |
| `SkiaExecutor` | `RasterError` | class declaration | `src/render/skia/executor/skia_executor.hpp:74` | `const Bitmap& bitmap, RasterError* error = nullptr,` |
| `SkiaRaster` | `const ImageRegistry` | class declaration | `src/render/skia/raster/skia_raster.cpp:151` | `const ImageRegistry* image_registry{};` |
| `SkiaRaster` | `SkCanvas` | class declaration | `src/render/skia/raster/skia_raster.cpp:155` | `[[nodiscard]] SkCanvas* canvas() const noexcept {` |
| `SkiaRaster` | `const RegisteredTypeface` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:167` | `const RegisteredTypeface* registered = nullptr;` |
| `SkiaRaster` | `const char` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:186` | `const char* family = nullptr;` |
| `SkiaRaster` | `const char` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:218` | `const char* cursor = text.data();` |
| `SkiaRaster` | `const char` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:219` | `const char* const end = cursor + text.size();` |
| `SkiaRaster` | `const char` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:221` | `const char* const scalar_start = cursor;` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:347` | `SkCanvas* canvas = impl_->canvas();` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:376` | `if (SkCanvas* canvas = impl_->canvas()) {` |
| `<file/function>` | `const void` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:491` | `const void* SkiaRaster::pixels() const noexcept {` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:514` | `if (SkCanvas* canvas = impl_->canvas()) {` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:520` | `if (SkCanvas* canvas = impl_->canvas();` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:527` | `if (SkCanvas* canvas = impl_->canvas()) {` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:533` | `if (SkCanvas* canvas = impl_->canvas()) {` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:539` | `if (SkCanvas* canvas = impl_->canvas()) {` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:545` | `if (SkCanvas* canvas = impl_->canvas()) {` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:551` | `if (SkCanvas* canvas = impl_->canvas()) {` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:557` | `if (SkCanvas* canvas = impl_->canvas()) {` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:567` | `if (SkCanvas* canvas = impl_->canvas()) {` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:585` | `SkCanvas* canvas = impl_->canvas();` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:609` | `SkCanvas* canvas = impl_->canvas();` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:639` | `SkCanvas* canvas = impl_->canvas();` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:658` | `if (SkCanvas* canvas = impl_->canvas()) {` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:671` | `if (SkCanvas* canvas = impl_->canvas(); canvas && !text.empty()) {` |
| `<file/function>` | `const char` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:706` | `const char* bytes = text.data() + run.offset;` |
| `<file/function>` | `const char` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:726` | `const char* bytes = text.data() + run.offset;` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:739` | `if (SkCanvas* canvas = impl_->canvas();` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:751` | `SkCanvas* canvas = impl_->canvas();` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:794` | `SkCanvas* canvas = impl_->canvas();` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:820` | `SkCanvas* canvas = impl_->canvas();` |
| `SkiaRaster` | `const void` | class declaration | `src/render/skia/raster/skia_raster.hpp:31` | `[[nodiscard]] const void* pixels() const noexcept;` |
| `<file/function>` | `const char` | local/signature/use | `src/render/skia/skia_raster_smoke.cpp:32` | `std::vector<std::byte> read_file(const char* path) {` |
| `HarfBuzzFontEngine` | `Face` | local/signature/use | `src/render/text/harfbuzz/harfbuzz_font_engine.cpp:117` | `std::stable_sort(result.begin(), result.end(), [&](Face* left, Face* right) {` |
| `HarfBuzzFontEngine` | `Face` | local/signature/use | `src/render/text/harfbuzz/harfbuzz_font_engine.cpp:117` | `std::stable_sort(result.begin(), result.end(), [&](Face* left, Face* right) {` |
| `HarfBuzzFontEngine` | `Face` | local/signature/use | `src/render/text/harfbuzz/harfbuzz_font_engine.cpp:118` | `const auto tier = [&](Face* face) {` |
| `Segment` | `Impl::Face` | class declaration | `src/render/text/harfbuzz/harfbuzz_font_engine.cpp:269` | `struct Segment final { Impl::Face* face{}; Utf8Range range{}; };` |
| `<file/function>` | `Impl::Face` | local/signature/use | `src/render/text/harfbuzz/harfbuzz_font_engine.cpp:274` | `Impl::Face* selected = nullptr;` |
| `<file/function>` | `Impl::Face` | local/signature/use | `src/render/text/harfbuzz/harfbuzz_font_engine.cpp:275` | `for (Impl::Face* candidate : candidates) {` |

## Lifetime-operation evidence

| Operation | Location | Evidence |
|---|---|---|
| `weak_lock` | `include/gui_forms/binding/binding/binding.hpp:26` | `return source_.lock();` |
| `subscription_token` | `include/gui_forms/binding/binding/binding.hpp:69` | `SubscriptionToken source_changed_;` |
| `subscription_token` | `include/gui_forms/binding/binding/binding.hpp:70` | `SubscriptionToken source_disposed_;` |
| `subscription_token` | `include/gui_forms/binding/binding/binding.hpp:71` | `SubscriptionToken target_changed_;` |
| `subscription_token` | `include/gui_forms/binding/binding/binding.hpp:72` | `SubscriptionToken target_validating_;` |
| `subscription_token` | `include/gui_forms/binding/binding_context/binding_context.hpp:39` | `SubscriptionToken disposed;` |
| `subscription_token` | `include/gui_forms/binding/value/binding_value.hpp:205` | `using ChangeConnector = std::function<SubscriptionToken(` |
| `subscription_token` | `include/gui_forms/commands/command_binding/command_binding.hpp:35` | `SubscriptionToken click_;` |
| `subscription_token` | `include/gui_forms/commands/command_binding/command_binding.hpp:36` | `SubscriptionToken state_;` |
| `owner_revocable` | `include/gui_forms/component/component/component.hpp:30` | `void own_revocable(const std::weak_ptr<detail::Revocable>& revocable);` |
| `component_container` | `include/gui_forms/component/component_container/component_container.hpp:10` | `class ComponentContainer final {` |
| `component_container` | `include/gui_forms/component/component_container/component_container.hpp:12` | `ComponentContainer() = default;` |
| `component_container` | `include/gui_forms/component/component_container/component_container.hpp:13` | `~ComponentContainer();` |
| `component_container` | `include/gui_forms/component/component_container/component_container.hpp:14` | `ComponentContainer(const ComponentContainer&) = delete;` |
| `component_container` | `include/gui_forms/component/component_container/component_container.hpp:14` | `ComponentContainer(const ComponentContainer&) = delete;` |
| `component_container` | `include/gui_forms/component/component_container/component_container.hpp:15` | `ComponentContainer& operator=(const ComponentContainer&) = delete;` |
| `component_container` | `include/gui_forms/component/component_container/component_container.hpp:15` | `ComponentContainer& operator=(const ComponentContainer&) = delete;` |
| `weak_lock` | `include/gui_forms/components/error_provider/error_provider.hpp:111` | `return data_source_.lock();` |
| `subscription_token` | `include/gui_forms/components/error_provider/error_provider.hpp:159` | `SubscriptionToken availability_subscription_;` |
| `subscription_token` | `include/gui_forms/components/error_provider/error_provider.hpp:160` | `SubscriptionToken presentation_subscription_;` |
| `subscription_token` | `include/gui_forms/components/error_provider/error_provider.hpp:161` | `SubscriptionToken root_bounds_subscription_;` |
| `subscription_token` | `include/gui_forms/components/error_provider/error_provider.hpp:162` | `SubscriptionToken source_list_subscription_;` |
| `subscription_token` | `include/gui_forms/components/error_provider/error_provider.hpp:163` | `SubscriptionToken source_current_subscription_;` |
| `subscription_token` | `include/gui_forms/components/error_provider/error_provider.hpp:164` | `SubscriptionToken source_completion_subscription_;` |
| `subscription_token` | `include/gui_forms/components/error_provider/error_provider.hpp:165` | `SubscriptionToken source_disposed_subscription_;` |
| `subscription_token` | `include/gui_forms/components/tool_tip/tool_tip.hpp:110` | `SubscriptionToken timer_subscription_;` |
| `subscription_token` | `include/gui_forms/components/tool_tip/tool_tip.hpp:111` | `SubscriptionToken popup_subscription_;` |
| `weak_lock` | `include/gui_forms/control/control/control.hpp:309` | `[[nodiscard]] Ptr parent() const noexcept { return parent_.lock(); }` |
| `subscription_token` | `include/gui_forms/control/control/control.hpp:518` | `[[nodiscard]] SubscriptionToken subscribe_property_changed(` |
| `make_shared` | `include/gui_forms/control/control/control.hpp:754` | `auto control = std::make_shared<ControlType>(` |
| `subscription_token` | `include/gui_forms/controls/button_base/button_base.hpp:151` | `SubscriptionToken image_list_changed_;` |
| `subscription_token` | `include/gui_forms/controls/easing_preview/easing_preview.hpp:62` | `SubscriptionToken presentation_subscription_;` |
| `subscription_token` | `include/gui_forms/controls/menu_strip/menu_strip.hpp:100` | `SubscriptionToken popup_invoked_;` |
| `subscription_token` | `include/gui_forms/controls/menu_strip/menu_strip.hpp:101` | `SubscriptionToken popup_changed_;` |
| `weak_lock` | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:27` | `[[nodiscard]] Control::Ptr anchor() const noexcept { return anchor_.lock(); }` |
| `subscription_token` | `include/gui_forms/controls/panel/color_value_editor/color_value_editor.hpp:47` | `SubscriptionToken committed_;` |
| `subscription_token` | `include/gui_forms/controls/panel/color_value_editor/color_value_editor.hpp:48` | `SubscriptionToken cancelled_;` |
| `subscription_token` | `include/gui_forms/controls/panel/combo_box/combo_box.hpp:87` | `SubscriptionToken popup_selection_;` |
| `subscription_token` | `include/gui_forms/controls/panel/combo_box/combo_box.hpp:88` | `SubscriptionToken popup_activation_;` |
| `subscription_token` | `include/gui_forms/controls/panel/combo_box/combo_box.hpp:89` | `SubscriptionToken popup_dismissal_;` |
| `subscription_token` | `include/gui_forms/controls/panel/combo_box/combo_box.hpp:90` | `SubscriptionToken popup_revocation_;` |
| `subscription_token` | `include/gui_forms/controls/panel/date_time_picker/date_time_picker.hpp:120` | `SubscriptionToken popup_commit_;` |
| `subscription_token` | `include/gui_forms/controls/panel/date_time_picker/date_time_picker.hpp:121` | `SubscriptionToken popup_cancel_;` |
| `subscription_token` | `include/gui_forms/controls/panel/date_time_picker/date_time_picker.hpp:122` | `SubscriptionToken popup_dismiss_;` |
| `subscription_token` | `include/gui_forms/controls/panel/date_time_picker/date_time_picker.hpp:123` | `SubscriptionToken popup_revocation_;` |
| `subscription_token` | `include/gui_forms/controls/panel/flags_value_editor/flags_value_editor.hpp:60` | `SubscriptionToken popup_check_;` |
| `subscription_token` | `include/gui_forms/controls/panel/flags_value_editor/flags_value_editor.hpp:61` | `SubscriptionToken popup_dismissal_;` |
| `subscription_token` | `include/gui_forms/controls/panel/flags_value_editor/flags_value_editor.hpp:62` | `SubscriptionToken popup_revocation_;` |
| `subscription_token` | `include/gui_forms/controls/panel/numeric_up_down/numeric_up_down.hpp:50` | `SubscriptionToken editor_change_;` |
| `subscription_token` | `include/gui_forms/controls/panel/numeric_up_down/numeric_up_down.hpp:51` | `SubscriptionToken spinner_step_;` |
| `subscription_token` | `include/gui_forms/controls/panel/object_view/object_view.hpp:179` | `SubscriptionToken image_list_changed_;` |
| `subscription_token` | `include/gui_forms/controls/panel/tree_view/tree_view.hpp:125` | `SubscriptionToken image_list_changed_;` |
| `subscription_token` | `include/gui_forms/controls/range_control/progress_bar/progress_bar.hpp:101` | `SubscriptionToken presentation_subscription_;` |
| `weak_lock` | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:46` | `return selected_page_.lock();` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:14` | `class SubscriptionToken final {` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:16` | `SubscriptionToken() = default;` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:17` | `~SubscriptionToken() { disconnect(); }` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:18` | `SubscriptionToken(SubscriptionToken&& other) noexcept` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:18` | `SubscriptionToken(SubscriptionToken&& other) noexcept` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:20` | `SubscriptionToken& operator=(SubscriptionToken&& other) noexcept {` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:20` | `SubscriptionToken& operator=(SubscriptionToken&& other) noexcept {` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:27` | `SubscriptionToken(const SubscriptionToken&) = delete;` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:27` | `SubscriptionToken(const SubscriptionToken&) = delete;` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:28` | `SubscriptionToken& operator=(const SubscriptionToken&) = delete;` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:28` | `SubscriptionToken& operator=(const SubscriptionToken&) = delete;` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:43` | `explicit SubscriptionToken(std::shared_ptr<detail::Revocable> revocable)` |
| `make_shared` | `include/gui_forms/event/event/event.hpp:63` | `Event() : state_(std::make_shared<State>()) {}` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:68` | `[[nodiscard]] SubscriptionToken subscribe(Callback callback) {` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:72` | `[[nodiscard]] SubscriptionToken subscribe(Component& owner, Callback callback) {` |
| `weak_lock` | `include/gui_forms/event/event/event.hpp:115` | `if (const auto event_state = state.lock()) {` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:132` | `[[nodiscard]] SubscriptionToken subscribe_impl(Component* owner, Callback callback) {` |
| `make_shared` | `include/gui_forms/event/event/event.hpp:136` | `auto slot = std::make_shared<Slot>(state_, std::move(callback));` |
| `owner_revocable` | `include/gui_forms/event/event/event.hpp:140` | `owner->own_revocable(slot);` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:142` | `return SubscriptionToken(std::move(slot));` |
| `subscription_token` | `include/gui_forms/host/session/host_session.hpp:41` | `SubscriptionToken capture_observation_;` |
| `subscription_token` | `include/gui_forms/host/session/host_session.hpp:42` | `SubscriptionToken modal_observation_;` |
| `subscription_token` | `include/gui_forms/inspection/property_editor_registry/property_editor_registry.hpp:17` | `std::function<SubscriptionToken(` |
| `subscription_token` | `include/gui_forms/inspection/property_editor_registry/property_editor_registry.hpp:19` | `std::function<SubscriptionToken(` |
| `weak_lock` | `include/gui_forms/window/window.hpp:358` | `[[nodiscard]] Control::Ptr focused_control() const noexcept { return focused_.lock(); }` |
| `weak_lock` | `include/gui_forms/window/window.hpp:379` | `return accept_button_.lock();` |
| `weak_lock` | `include/gui_forms/window/window.hpp:382` | `return cancel_button_.lock();` |
| `weak_lock` | `include/gui_forms/window/window.hpp:397` | `[[nodiscard]] Control::Ptr captured_control() const noexcept { return captured_.lock(); }` |
| `weak_lock` | `include/gui_forms/window/window.hpp:417` | `[[nodiscard]] Control::Ptr pressed_control() const noexcept { return pressed_.lock(); }` |
| `make_shared` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:33` | `auto state = std::make_shared<PropertyState>();` |
| `weak_lock` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:123` | `const auto current = weak.lock();` |
| `weak_lock` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:134` | `const auto current = weak.lock();` |
| `weak_lock` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:161` | `const auto retained = weak.lock();` |
| `make_shared` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:168` | `auto current = std::make_shared<gui_forms::BindingValue>(` |
| `make_shared` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:170` | `auto failures = std::make_shared<` |
| `weak_lock` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:175` | `const auto editor = weak_button.lock();` |
| `weak_lock` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:176` | `const auto state = weak.lock();` |
| `weak_lock` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:205` | `const auto editor = weak_button.lock();` |
| `subscription_token` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:206` | `if (!editor) return gui_forms::SubscriptionToken{};` |
| `weak_lock` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:211` | `const auto state = weak.lock();` |
| `make_shared` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:469` | `std::make_shared<gui_forms::PropertyEnumDescriptor>();` |
| `static_pointer_cast` | `src/abi/control_adapters/raster_control/raster_control.hpp:159` | `std::static_pointer_cast<RasterControl>(` |
| `shared_from_this` | `src/abi/control_adapters/raster_control/raster_control.hpp:160` | `shared_from_this()),` |
| `weak_lock` | `src/abi/control_adapters/raster_control/raster_control.hpp:243` | `const auto state = weak_state.lock();` |
| `weak_lock` | `src/abi/control_adapters/raster_control/raster_control.hpp:248` | `const auto target = weak_target.lock();` |
| `weak_lock` | `src/abi/control_adapters/raster_control/raster_control.hpp:256` | `const auto queued_state = weak_state.lock();` |
| `weak_lock` | `src/abi/control_adapters/raster_control/raster_control.hpp:261` | `if (const auto queued_target = weak_target.lock()) {` |
| `static_pointer_cast` | `src/abi/control_adapters/raster_control/raster_control.hpp:287` | `const auto self = std::static_pointer_cast<RasterControl>(` |
| `shared_from_this` | `src/abi/control_adapters/raster_control/raster_control.hpp:288` | `shared_from_this());` |
| `make_shared` | `src/abi/control_adapters/raster_control/raster_control.hpp:296` | `auto wake_state = std::make_shared<LiveWakeState>();` |
| `make_shared` | `src/abi/drawing_c_api.cpp:115` | `auto record = std::make_shared<ObjectRecord>();` |
| `make_shared` | `src/abi/drawing_c_api.cpp:116` | `record->object = std::make_shared<Object>(std::forward<Arguments>(arguments)...);` |
| `make_shared` | `src/abi/drawing_c_api.cpp:133` | `auto record = std::make_shared<ObjectRecord>();` |
| `dynamic_pointer_cast` | `src/abi/drawing_c_api.cpp:235` | `auto object = kind == 0U ? std::dynamic_pointer_cast<Object>(record->object) :` |
| `static_pointer_cast` | `src/abi/drawing_c_api.cpp:236` | `std::static_pointer_cast<Object>(record->object);` |
| `dynamic_pointer_cast` | `src/abi/drawing_c_api.cpp:267` | `auto left = left_kind == 0U ? std::dynamic_pointer_cast<Left>(left_record->object) :` |
| `static_pointer_cast` | `src/abi/drawing_c_api.cpp:268` | `std::static_pointer_cast<Left>(left_record->object);` |
| `dynamic_pointer_cast` | `src/abi/drawing_c_api.cpp:269` | `auto right = right_kind == 0U ? std::dynamic_pointer_cast<Right>(right_record->object) :` |
| `static_pointer_cast` | `src/abi/drawing_c_api.cpp:270` | `std::static_pointer_cast<Right>(right_record->object);` |
| `dynamic_pointer_cast` | `src/abi/drawing_c_api.cpp:310` | `auto recorder = std::dynamic_pointer_cast<gui_drawing::GraphicsRecorder>(recorder_record->object);` |
| `dynamic_pointer_cast` | `src/abi/drawing_c_api.cpp:311` | `auto font = std::dynamic_pointer_cast<gui_drawing::Font>(font_record->object);` |
| `dynamic_pointer_cast` | `src/abi/drawing_c_api.cpp:312` | `auto brush = std::dynamic_pointer_cast<gui_drawing::SolidBrush>(brush_record->object);` |
| `dynamic_pointer_cast` | `src/abi/drawing_c_api.cpp:313` | `auto format = std::dynamic_pointer_cast<gui_drawing::StringFormat>(format_record->object);` |
| `weak_lock` | `src/abi/drawing_c_api.cpp:982` | `const auto native = bitmap.lock(` |
| `delete_expression` | `src/abi/drawing_c_api.cpp:1554` | `delete static_cast<gui_drawing::Bitmap*>(decoded);` |
| `make_unique` | `src/abi/drawing_platform_windows.cpp:194` | `auto bitmap = std::make_unique<Bitmap>(width, height,` |
| `make_unique` | `src/abi/drawing_platform_windows.cpp:274` | `auto bitmap = std::make_unique<Bitmap>(width, height,` |
| `weak_lock` | `src/abi/drawing_platform_windows.cpp:324` | `BitmapLockView lock = bitmap.lock(BitmapLockMode::write);` |
| `weak_lock` | `src/abi/drawing_platform_windows.cpp:482` | `BitmapLockView lock = bitmap.lock(BitmapLockMode::write);` |
| `weak_lock` | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:92` | `if (const auto raster = target.lock()) {` |
| `weak_lock` | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:126` | `if (const auto raster = binding.target.lock()) {` |
| `weak_lock` | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:226` | `if (const auto raster = binding.target.lock()) {` |
| `subscription_token` | `src/abi/registry/registry.hpp:25` | `gui_forms::SubscriptionToken native_subscription;` |
| `make_shared` | `src/abi/registry/registry.hpp:94` | `auto record = std::make_shared<ControlRecord>();` |
| `make_shared` | `src/abi/registry/registry.hpp:98` | `record->control = std::make_shared<FormControl>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:101` | `record->control = std::make_shared<gui_forms::Panel>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:104` | `auto panel = std::make_shared<gui_forms::Panel>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:113` | `record->control = std::make_shared<gui_forms::Button>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:116` | `record->control = std::make_shared<gui_forms::CheckBox>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:119` | `record->control = std::make_shared<gui_forms::Label>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:122` | `record->control = std::make_shared<FieldControl>(` |
| `make_shared` | `src/abi/registry/registry.hpp:126` | `record->control = std::make_shared<FieldControl>(` |
| `make_shared` | `src/abi/registry/registry.hpp:130` | `record->control = std::make_shared<FieldControl>(` |
| `make_shared` | `src/abi/registry/registry.hpp:134` | `record->control = std::make_shared<gui_forms::TrackBar>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:137` | `record->control = std::make_shared<gui_forms::RadioButton>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:140` | `record->control = std::make_shared<gui_forms::GroupBox>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:143` | `record->control = std::make_shared<gui_forms::ProgressBar>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:146` | `record->control = std::make_shared<gui_forms::LinkLabel>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:149` | `record->control = std::make_shared<FieldControl>(` |
| `make_shared` | `src/abi/registry/registry.hpp:153` | `record->control = std::make_shared<FieldControl>(` |
| `make_shared` | `src/abi/registry/registry.hpp:157` | `record->control = std::make_shared<FieldControl>(` |
| `make_shared` | `src/abi/registry/registry.hpp:161` | `record->control = std::make_shared<FieldControl>(` |
| `make_shared` | `src/abi/registry/registry.hpp:166` | `std::make_shared<InputTransparentControl>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:170` | `std::make_shared<RasterControl>(std::move(native_id), true);` |
| `make_shared` | `src/abi/registry/registry.hpp:174` | `std::make_shared<gui_forms::PropertyGrid>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:181` | `std::make_shared<AbiPropertyObjectControl>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:184` | `record->control = std::make_shared<RasterControl>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:188` | `record->control = std::make_shared<RasterControl>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:191` | `record->control = std::make_shared<Control>(std::move(native_id));` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:224` | `std::dynamic_pointer_cast<gui_forms::ButtonBase>(record->control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:227` | `std::dynamic_pointer_cast<gui_forms::Label>(record->control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:230` | `std::dynamic_pointer_cast<gui_forms::GroupBox>(record->control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:233` | `std::dynamic_pointer_cast<FieldControl>(record->control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:343` | `const auto scrollable = std::dynamic_pointer_cast<` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:363` | `const auto scrollable = std::dynamic_pointer_cast<` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:383` | `const auto scrollable = std::dynamic_pointer_cast<` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:402` | `const auto scrollable = std::dynamic_pointer_cast<` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:421` | `const auto scrollable = std::dynamic_pointer_cast<` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:492` | `const auto scrollable = std::dynamic_pointer_cast<` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:520` | `const auto scrollable = std::dynamic_pointer_cast<` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:597` | `const auto grid = std::dynamic_pointer_cast<gui_forms::PropertyGrid>(` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:626` | `std::dynamic_pointer_cast<AbiPropertyObjectControl>(control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:663` | `std::dynamic_pointer_cast<AbiPropertyObjectControl>(record->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:685` | `std::dynamic_pointer_cast<AbiPropertyObjectControl>(record->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:724` | `const auto grid = std::dynamic_pointer_cast<gui_forms::PropertyGrid>(` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:754` | `const auto grid = std::dynamic_pointer_cast<gui_forms::PropertyGrid>(` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:784` | `const auto grid = std::dynamic_pointer_cast<gui_forms::PropertyGrid>(` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:805` | `const auto grid = std::dynamic_pointer_cast<gui_forms::PropertyGrid>(` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:829` | `const auto grid = std::dynamic_pointer_cast<gui_forms::PropertyGrid>(` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:847` | `const auto grid = std::dynamic_pointer_cast<gui_forms::PropertyGrid>(` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:870` | `const auto raster = std::dynamic_pointer_cast<RasterControl>(record->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:901` | `const auto raster = std::dynamic_pointer_cast<RasterControl>(record->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:920` | `const auto raster = std::dynamic_pointer_cast<RasterControl>(record->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:946` | `const auto raster = std::dynamic_pointer_cast<RasterControl>(record->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1001` | `if (const auto field = std::dynamic_pointer_cast<FieldControl>(record->control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1004` | `std::dynamic_pointer_cast<gui_forms::Label>(record->control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1007` | `std::dynamic_pointer_cast<gui_forms::ButtonBase>(record->control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1015` | `std::dynamic_pointer_cast<gui_forms::Panel>(record->control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1042` | `std::dynamic_pointer_cast<gui_forms::Label>(record->control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1048` | `std::dynamic_pointer_cast<gui_forms::ButtonBase>(record->control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1070` | `std::dynamic_pointer_cast<gui_forms::Button>(record->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1092` | `std::dynamic_pointer_cast<gui_forms::Panel>(record->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1116` | `const auto field = std::dynamic_pointer_cast<FieldControl>(record->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1139` | `const auto field = std::dynamic_pointer_cast<FieldControl>(record->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1161` | `const auto field = std::dynamic_pointer_cast<FieldControl>(record->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1264` | `const auto field = std::dynamic_pointer_cast<FieldControl>(record->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1293` | `const auto field = std::dynamic_pointer_cast<FieldControl>(record->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1316` | `const auto field = std::dynamic_pointer_cast<FieldControl>(record->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1334` | `const auto field = std::dynamic_pointer_cast<FieldControl>(record->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1353` | `std::dynamic_pointer_cast<gui_forms::CheckBox>(record->control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1359` | `std::dynamic_pointer_cast<gui_forms::RadioButton>(record->control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1383` | `std::dynamic_pointer_cast<gui_forms::CheckBox>(record->control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1388` | `std::dynamic_pointer_cast<gui_forms::RadioButton>(record->control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1404` | `std::dynamic_pointer_cast<gui_forms::RangeControl>(record->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1423` | `std::dynamic_pointer_cast<gui_forms::RangeControl>(record->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1439` | `std::dynamic_pointer_cast<gui_forms::RangeControl>(record->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1458` | `std::dynamic_pointer_cast<gui_forms::RangeControl>(record->control);` |
| `make_unique` | `src/abi/registry/registry.hpp:1716` | `auto model = std::make_unique<Window>(record->control, client_size);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2177` | `if (std::dynamic_pointer_cast<FormControl>(control)) {` |
| `weak_lock` | `src/abi/registry/registry.hpp:2189` | `if (const Control::Ptr control = weak.lock()) {` |
| `weak_lock` | `src/abi/registry/registry.hpp:2206` | `if (const Control::Ptr control = weak.lock()) control->set_paint_plane(plane);` |
| `make_shared` | `src/abi/registry/registry.hpp:2229` | `auto record = std::make_shared<SubscriptionRecord>();` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2278` | `!std::dynamic_pointer_cast<gui_forms::ButtonBase>(sender->control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2288` | `std::dynamic_pointer_cast<gui_forms::RangeControl>(sender->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2289` | `const auto scrollable = std::dynamic_pointer_cast<` |
| `make_shared` | `src/abi/registry/registry.hpp:2305` | `auto record = std::make_shared<SubscriptionRecord>();` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2313` | `auto button = std::dynamic_pointer_cast<gui_forms::ButtonBase>(sender->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2361` | `const auto raster = std::dynamic_pointer_cast<RasterControl>(sender->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2362` | `const auto field = std::dynamic_pointer_cast<FieldControl>(sender->control);` |
| `make_shared` | `src/abi/registry/registry.hpp:2363` | `auto record = std::make_shared<SubscriptionRecord>();` |
| `weak_lock` | `src/abi/registry/registry.hpp:2386` | `const auto control = weak_control.lock();` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2420` | `const auto field = std::dynamic_pointer_cast<FieldControl>(sender->control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2421` | `const auto raster = std::dynamic_pointer_cast<RasterControl>(sender->control);` |
| `make_shared` | `src/abi/registry/registry.hpp:2426` | `auto record = std::make_shared<SubscriptionRecord>();` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2457` | `const auto form = std::dynamic_pointer_cast<FormControl>(sender->control);` |
| `make_shared` | `src/abi/registry/registry.hpp:2462` | `auto record = std::make_shared<SubscriptionRecord>();` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2500` | `const auto field = std::dynamic_pointer_cast<FieldControl>(sender->control);` |
| `make_shared` | `src/abi/registry/registry.hpp:2505` | `auto record = std::make_shared<SubscriptionRecord>();` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2868` | `std::dynamic_pointer_cast<gui_forms::ButtonBase>(root)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2883` | `if (std::dynamic_pointer_cast<RasterControl>(root) \|\|` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2884` | `std::dynamic_pointer_cast<FieldControl>(root) \|\|` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2885` | `std::dynamic_pointer_cast<gui_forms::RangeControl>(root)) {` |
| `make_shared` | `src/abi/registry/registry.hpp:2897` | `auto result = std::make_shared<` |
| `make_shared` | `src/controls/basic/basic_control_rendering.cpp:12` | `static const auto value = std::make_shared<const PropertyEnumDescriptor>(` |
| `dynamic_pointer_cast` | `src/controls/button_base/radio_button/radio_button.cpp:53` | `auto peer = std::dynamic_pointer_cast<RadioButton>(sibling);` |
| `dynamic_pointer_cast` | `src/controls/button_base/radio_button/radio_button.cpp:77` | `auto peer = std::dynamic_pointer_cast<RadioButton>(sibling);` |
| `weak_lock` | `src/controls/commands/command_binding/command_binding.cpp:20` | `if (const auto command = weak_command.lock()) {` |
| `weak_lock` | `src/controls/commands/command_binding/command_binding.cpp:28` | `if (const auto button = weak_button.lock()) {` |
| `shared_from_this` | `src/controls/easing_preview/easing_preview.cpp:252` | `shared_from_this(), interval, FrameClock::now() + interval);` |
| `subscription_token` | `src/controls/gallery_controls.cpp:181` | `SubscriptionToken click_;` |
| `subscription_token` | `src/controls/gallery_controls.cpp:215` | `SubscriptionToken click_;` |
| `subscription_token` | `src/controls/gallery_controls.cpp:240` | `SubscriptionToken click_;` |
| `subscription_token` | `src/controls/gallery_controls.cpp:259` | `SubscriptionToken click_;` |
| `subscription_token` | `src/controls/gallery_controls.cpp:284` | `SubscriptionToken value_changed_;` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:367` | `if (auto status = std::dynamic_pointer_cast<Label>(child)) {` |
| `subscription_token` | `src/controls/gallery_controls.cpp:379` | `SubscriptionToken initialized_;` |
| `subscription_token` | `src/controls/gallery_controls.cpp:380` | `SubscriptionToken load_;` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:442` | `if (auto button = std::dynamic_pointer_cast<ButtonBase>(control)) {` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:444` | `} else if (auto range = std::dynamic_pointer_cast<RangeControl>(control)) {` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:448` | `if (auto check = std::dynamic_pointer_cast<CheckBox>(` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:452` | `if (auto classic = std::dynamic_pointer_cast<RadioButton>(` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:456` | `if (auto quiet_radio = std::dynamic_pointer_cast<RadioButton>(` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:461` | `if (auto tri = std::dynamic_pointer_cast<CheckBox>(` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:465` | `if (auto link = std::dynamic_pointer_cast<LinkLabel>(` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:470` | `if (auto slider = std::dynamic_pointer_cast<TrackBar>(` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:474` | `if (auto progress = std::dynamic_pointer_cast<ProgressBar>(` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:479` | `if (auto label = std::dynamic_pointer_cast<Label>(window->find(id))) {` |
| `make_shared` | `src/controls/gallery_controls.cpp:904` | `auto context = std::make_shared<GalleryContext>();` |
| `dynamic_pointer_cast` | `src/controls/guidance/error_glyph/error_glyph.cpp:137` | `auto self = std::dynamic_pointer_cast<ErrorGlyph>(shared_from_this());` |
| `shared_from_this` | `src/controls/guidance/error_glyph/error_glyph.cpp:137` | `auto self = std::dynamic_pointer_cast<ErrorGlyph>(shared_from_this());` |
| `subscription_token` | `src/controls/guidance/error_provider/error_provider.cpp:31` | `SubscriptionToken bounds_subscription;` |
| `make_unique` | `src/controls/guidance/error_provider/error_provider.cpp:35` | `: window_lifetime_(window.lifetime_), tool_tip_(std::make_unique<ToolTip>(window)),` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:77` | `const auto lifetime = window_lifetime_.lock();` |
| `make_unique` | `src/controls/guidance/error_provider/error_provider.cpp:117` | `auto entry = std::make_unique<Entry>();` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:122` | `if (const auto current = weak_target.lock()) {` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:162` | `if (const auto target = entry->target.lock(); target && target->is_alive()) {` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:174` | `const auto target = entry.target.lock();` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:282` | `if (data_source_.lock() == source) {` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:353` | `if (const auto target = weak.lock(); target && target->is_alive()) {` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:360` | `if (!event.binding \|\| event.binding->source() != data_source_.lock()) return;` |
| `weak_from_this` | `src/controls/guidance/error_provider/error_provider.cpp:363` | `const auto target = raw->weak_from_this().lock();` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:363` | `const auto target = raw->weak_from_this().lock();` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:376` | `const auto source = data_source_.lock();` |
| `weak_from_this` | `src/controls/guidance/error_provider/error_provider.cpp:399` | `const auto target = raw ? raw->weak_from_this().lock() : nullptr;` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:399` | `const auto target = raw ? raw->weak_from_this().lock() : nullptr;` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:431` | `if (const auto target = weak.lock(); target && target->is_alive()) {` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:436` | `const auto target = item.second.first.lock();` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:448` | `const auto target = entry.target.lock();` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:503` | `const auto target = entry.target.lock();` |
| `make_unique` | `src/controls/guidance/error_provider/error_provider.cpp:527` | `entry.popup = std::make_unique<PopupToken>(std::move(popup));` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:563` | `const auto target = entry->target.lock();` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:601` | `if (const auto target = entry->target.lock(); target && target->is_alive()) {` |
| `make_unique` | `src/controls/guidance/help_provider/help_provider.cpp:37` | `accelerator_ = std::make_unique<AcceleratorHolder>(window.register_accelerator(` |
| `weak_lock` | `src/controls/guidance/help_provider/help_provider.cpp:54` | `const auto lifetime = window_lifetime_.lock();` |
| `make_unique` | `src/controls/guidance/help_provider/help_provider.cpp:94` | `auto entry = std::make_unique<Entry>();` |
| `weak_lock` | `src/controls/guidance/help_provider/help_provider.cpp:107` | `const auto target = entry.target.lock();` |
| `weak_lock` | `src/controls/guidance/help_provider/help_provider.cpp:204` | `if (const auto target = entry->target.lock(); target && target->is_alive()) {` |
| `weak_lock` | `src/controls/guidance/help_provider/help_provider.cpp:267` | `if (const auto target = entry.target.lock(); target && target->is_alive()) {` |
| `weak_lock` | `src/controls/guidance/help_provider/help_provider.cpp:281` | `const auto target = pair.second->target.lock();` |
| `weak_lock` | `src/controls/guidance/help_provider/help_provider.cpp:295` | `if (const auto target = entry->target.lock(); target && target->is_alive()) {` |
| `make_shared` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:92` | `auto result = std::make_shared<PropertyEditorRegistry>();` |
| `make_shared` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:107` | `auto synchronizing = std::make_shared<bool>(false);` |
| `weak_lock` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:113` | `const auto retained = weak.lock();` |
| `weak_lock` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:124` | `const auto retained = weak.lock();` |
| `subscription_token` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:134` | `: SubscriptionToken{};` |
| `make_shared` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:159` | `auto synchronizing = std::make_shared<bool>(false);` |
| `weak_lock` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:165` | `const auto retained = weak.lock();` |
| `weak_lock` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:176` | `const auto retained = weak.lock();` |
| `subscription_token` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:187` | `: SubscriptionToken{};` |
| `make_shared` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:206` | `auto synchronizing = std::make_shared<bool>(false);` |
| `weak_lock` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:212` | `const auto retained = weak.lock();` |
| `weak_lock` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:223` | `const auto retained = weak.lock();` |
| `subscription_token` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:233` | `: SubscriptionToken{};` |
| `weak_lock` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:239` | `const auto retained = weak.lock();` |
| `subscription_token` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:243` | `: SubscriptionToken{};` |
| `make_shared` | `src/controls/inspection/property_value_converter_registry/property_value_converter_registry.cpp:129` | `auto result = std::make_shared<PropertyValueConverterRegistry>();` |
| `shared_from_this` | `src/controls/menu/context_menu/context_menu.cpp:288` | `if (window()) return window()->request_focus(shared_from_this());` |
| `subscription_token` | `src/controls/menu/context_menu/context_menu.cpp:658` | `SubscriptionToken popup_revocation;` |
| `make_unique` | `src/controls/menu/context_menu/context_menu.cpp:667` | `: stable_id_(std::move(stable_id)), impl_(std::make_unique<Impl>(*this)) {` |
| `make_unique` | `src/controls/menu/menu_strip/menu_strip.cpp:45` | `popup_(std::make_unique<ContextMenu>(` |
| `shared_from_this` | `src/controls/menu/menu_strip/menu_strip.cpp:127` | `popup_->show(shared_from_this(),` |
| `shared_from_this` | `src/controls/menu/menu_strip/menu_strip.cpp:398` | `if (window()) static_cast<void>(window()->request_focus(shared_from_this()));` |
| `shared_from_this` | `src/controls/menu/menu_strip/menu_strip.cpp:433` | `if (window() && !window()->request_focus(shared_from_this())) {` |
| `weak_lock` | `src/controls/panel/anchored_popup_layer/anchored_popup_layer.cpp:33` | `if (anchor_.lock() == anchor) return;` |
| `weak_lock` | `src/controls/panel/anchored_popup_layer/anchored_popup_layer.cpp:94` | `const Control::Ptr anchor = anchor_.lock();` |
| `shared_from_this` | `src/controls/panel/color_value_editor/color_value_editor.cpp:25` | `std::static_pointer_cast<ColorValueEditor>(shared_from_this());` |
| `static_pointer_cast` | `src/controls/panel/color_value_editor/color_value_editor.cpp:25` | `std::static_pointer_cast<ColorValueEditor>(shared_from_this());` |
| `weak_lock` | `src/controls/panel/color_value_editor/color_value_editor.cpp:28` | `if (const auto retained = weak.lock()) retained->commit(text);` |
| `weak_lock` | `src/controls/panel/color_value_editor/color_value_editor.cpp:31` | `if (const auto retained = weak.lock()) retained->cancel();` |
| `shared_from_this` | `src/controls/panel/combo_box/combo_box.cpp:255` | `PopupToken popup_token = window()->open_popup(shared_from_this(), layer);` |
| `shared_from_this` | `src/controls/panel/combo_box/combo_box.cpp:261` | `std::static_pointer_cast<ComboBox>(shared_from_this());` |
| `static_pointer_cast` | `src/controls/panel/combo_box/combo_box.cpp:261` | `std::static_pointer_cast<ComboBox>(shared_from_this());` |
| `weak_lock` | `src/controls/panel/combo_box/combo_box.cpp:264` | `if (const auto combo = weak.lock(); change.active_index) {` |
| `weak_lock` | `src/controls/panel/combo_box/combo_box.cpp:270` | `if (const auto combo = weak.lock()) combo->commit_popup_selection(index);` |
| `weak_lock` | `src/controls/panel/combo_box/combo_box.cpp:273` | `if (const auto combo = weak.lock()) combo->close_drop_down();` |
| `weak_lock` | `src/controls/panel/combo_box/combo_box.cpp:277` | `if (const auto combo = weak.lock()) combo->on_popup_revoked();` |
| `static_pointer_cast` | `src/controls/panel/correspondence_view/correspondence_view.cpp:420` | `const auto self = std::static_pointer_cast<CorrespondenceView>(` |
| `shared_from_this` | `src/controls/panel/correspondence_view/correspondence_view.cpp:421` | `shared_from_this());` |
| `weak_lock` | `src/controls/panel/correspondence_view/correspondence_view.cpp:427` | `if (const auto view = weak.lock()) {` |
| `shared_from_this` | `src/controls/panel/correspondence_view/correspondence_view.cpp:681` | `static_cast<void>(window()->request_focus(shared_from_this()));` |
| `shared_from_this` | `src/controls/panel/correspondence_view/correspondence_view.cpp:879` | `static_cast<void>(window()->request_focus(shared_from_this()));` |
| `shared_from_this` | `src/controls/panel/correspondence_view/correspondence_view.cpp:896` | `if (window()) static_cast<void>(window()->request_focus(shared_from_this()));` |
| `shared_from_this` | `src/controls/panel/date_time_picker/date_time_picker.cpp:291` | `const Control::Ptr owner = shared_from_this();` |
| `shared_from_this` | `src/controls/panel/date_time_picker/date_time_picker.cpp:318` | `std::static_pointer_cast<DateTimePicker>(shared_from_this());` |
| `static_pointer_cast` | `src/controls/panel/date_time_picker/date_time_picker.cpp:318` | `std::static_pointer_cast<DateTimePicker>(shared_from_this());` |
| `weak_lock` | `src/controls/panel/date_time_picker/date_time_picker.cpp:321` | `if (const auto picker = weak.lock()) picker->commit_popup_value(date);` |
| `weak_lock` | `src/controls/panel/date_time_picker/date_time_picker.cpp:324` | `if (const auto picker = weak.lock()) picker->close_drop_down();` |
| `weak_lock` | `src/controls/panel/date_time_picker/date_time_picker.cpp:327` | `if (const auto picker = weak.lock()) picker->close_drop_down();` |
| `weak_lock` | `src/controls/panel/date_time_picker/date_time_picker.cpp:331` | `if (const auto picker = weak.lock()) picker->on_popup_revoked();` |
| `make_shared` | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:51` | `std::make_shared<const PropertyEnumDescriptor>(descriptor);` |
| `make_shared` | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:75` | `std::make_shared<const PropertyEnumDescriptor>(descriptor_);` |
| `shared_from_this` | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:147` | `PopupToken token = window()->open_popup(shared_from_this(), layer);` |
| `shared_from_this` | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:153` | `std::static_pointer_cast<FlagsValueEditor>(shared_from_this());` |
| `static_pointer_cast` | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:153` | `std::static_pointer_cast<FlagsValueEditor>(shared_from_this());` |
| `weak_lock` | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:156` | `if (const auto retained = weak.lock()) {` |
| `weak_lock` | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:161` | `if (const auto retained = weak.lock()) retained->close_drop_down();` |
| `weak_lock` | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:165` | `if (const auto retained = weak.lock()) retained->on_popup_revoked();` |
| `make_shared` | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:251` | `std::make_shared<const PropertyEnumDescriptor>(descriptor_);` |
| `subscription_token` | `src/controls/panel/instrument_rack/instrument_rack.cpp:96` | `SubscriptionToken changed;` |
| `subscription_token` | `src/controls/panel/instrument_rack/instrument_rack.cpp:97` | `SubscriptionToken committed;` |
| `subscription_token` | `src/controls/panel/instrument_rack/instrument_rack.cpp:98` | `SubscriptionToken cancelled;` |
| `subscription_token` | `src/controls/panel/instrument_rack/instrument_rack.cpp:99` | `SubscriptionToken focused;` |
| `subscription_token` | `src/controls/panel/instrument_rack/instrument_rack.cpp:111` | `SubscriptionToken toggled;` |
| `subscription_token` | `src/controls/panel/instrument_rack/instrument_rack.cpp:112` | `SubscriptionToken remove_clicked;` |
| `subscription_token` | `src/controls/panel/instrument_rack/instrument_rack.cpp:113` | `SubscriptionToken enable_focused;` |
| `subscription_token` | `src/controls/panel/instrument_rack/instrument_rack.cpp:114` | `SubscriptionToken remove_focused;` |
| `subscription_token` | `src/controls/panel/instrument_rack/instrument_rack.cpp:174` | `SubscriptionToken& token) {` |
| `dynamic_pointer_cast` | `src/controls/panel/instrument_rack/instrument_rack.cpp:218` | `if (const auto choice = std::dynamic_pointer_cast<ComboBox>(state.editor)) {` |
| `weak_lock` | `src/controls/panel/instrument_rack/instrument_rack.cpp:225` | `const auto control = weak.lock();` |
| `dynamic_pointer_cast` | `src/controls/panel/instrument_rack/instrument_rack.cpp:248` | `} else if (const auto text = std::dynamic_pointer_cast<TextBox>(state.editor)) {` |
| `weak_lock` | `src/controls/panel/instrument_rack/instrument_rack.cpp:306` | `const auto control = weak.lock();` |
| `dynamic_pointer_cast` | `src/controls/panel/instrument_rack/instrument_rack.cpp:431` | `if (const auto panel = std::dynamic_pointer_cast<Panel>(` |
| `dynamic_pointer_cast` | `src/controls/panel/instrument_rack/instrument_rack.cpp:467` | `if (const auto choice = std::dynamic_pointer_cast<ComboBox>(` |
| `dynamic_pointer_cast` | `src/controls/panel/instrument_rack/instrument_rack.cpp:476` | `} else if (const auto text = std::dynamic_pointer_cast<TextBox>(` |
| `make_unique` | `src/controls/panel/instrument_rack/instrument_rack.cpp:588` | `: Panel(std::move(stable_id)), impl_(std::make_unique<Impl>(*this)) {` |
| `dynamic_pointer_cast` | `src/controls/panel/instrument_rack/instrument_rack.cpp:767` | `if (const auto text = std::dynamic_pointer_cast<TextBox>(` |
| `dynamic_pointer_cast` | `src/controls/panel/instrument_rack/instrument_rack.cpp:770` | `} else if (const auto choice = std::dynamic_pointer_cast<ComboBox>(` |
| `shared_from_this` | `src/controls/panel/list_box/checked_list_box/checked_list_box.cpp:233` | `static_cast<void>(window()->request_focus(shared_from_this()));` |
| `shared_from_this` | `src/controls/panel/list_box/list_box.cpp:352` | `if (window() != nullptr) static_cast<void>(window()->request_focus(shared_from_this()));` |
| `shared_from_this` | `src/controls/panel/list_box/list_box.cpp:469` | `static_cast<void>(window()->request_focus(shared_from_this()));` |
| `shared_from_this` | `src/controls/panel/numeric_up_down/numeric_up_down.cpp:47` | `std::static_pointer_cast<NumericUpDown>(shared_from_this());` |
| `static_pointer_cast` | `src/controls/panel/numeric_up_down/numeric_up_down.cpp:47` | `std::static_pointer_cast<NumericUpDown>(shared_from_this());` |
| `weak_lock` | `src/controls/panel/numeric_up_down/numeric_up_down.cpp:50` | `if (const auto numeric = weak.lock(); numeric && !numeric->synchronizing_) {` |
| `dynamic_pointer_cast` | `src/controls/panel/numeric_up_down/numeric_up_down.cpp:54` | `auto spin = std::dynamic_pointer_cast<SpinButtons>(spinner_);` |
| `weak_lock` | `src/controls/panel/numeric_up_down/numeric_up_down.cpp:56` | `if (const auto numeric = weak.lock()) numeric->step(direction);` |
| `shared_from_this` | `src/controls/panel/object_view/object_view.cpp:558` | `if (window()) static_cast<void>(window()->request_focus(shared_from_this()));` |
| `shared_from_this` | `src/controls/panel/object_view/object_view.cpp:737` | `if (window()) static_cast<void>(window()->request_focus(shared_from_this()));` |
| `subscription_token` | `src/controls/panel/property_grid/property_grid.cpp:53` | `SubscriptionToken commit;` |
| `subscription_token` | `src/controls/panel/property_grid/property_grid.cpp:54` | `SubscriptionToken failure;` |
| `weak_lock` | `src/controls/panel/property_grid/property_grid.cpp:327` | `const Control::Ptr result = selected.front().lock();` |
| `weak_lock` | `src/controls/panel/property_grid/property_grid.cpp:335` | `Control::Ptr retained = candidate.lock();` |
| `subscription_token` | `src/controls/panel/property_grid/property_grid.cpp:603` | `SubscriptionToken subscription = object->subscribe_property_changed(` |
| `weak_from_this` | `src/controls/panel/property_grid/property_grid.cpp:632` | `const Control::WeakPtr owner_lifetime = owner.weak_from_this();` |
| `subscription_token` | `src/controls/panel/property_grid/property_grid.cpp:633` | `SubscriptionToken committed = binding->connect_committed(` |
| `weak_lock` | `src/controls/panel/property_grid/property_grid.cpp:637` | `const Control::Ptr retained = owner_lifetime.lock();` |
| `subscription_token` | `src/controls/panel/property_grid/property_grid.cpp:646` | `SubscriptionToken failure;` |
| `weak_lock` | `src/controls/panel/property_grid/property_grid.cpp:652` | `const Control::Ptr retained = owner_lifetime.lock();` |
| `subscription_token` | `src/controls/panel/property_grid/property_grid.cpp:1064` | `std::vector<SubscriptionToken> property_subscriptions;` |
| `subscription_token` | `src/controls/panel/property_grid/property_grid.cpp:1066` | `SubscriptionToken list_commit;` |
| `subscription_token` | `src/controls/panel/property_grid/property_grid.cpp:1067` | `SubscriptionToken list_expansion;` |
| `subscription_token` | `src/controls/panel/property_grid/property_grid.cpp:1068` | `SubscriptionToken list_reset;` |
| `make_unique` | `src/controls/panel/property_grid/property_grid.cpp:1075` | `: Panel(std::move(stable_id)), impl_(std::make_unique<Impl>(*this)) {` |
| `subscription_token` | `src/controls/panel/property_list/property_list.cpp:31` | `SubscriptionToken value_subscription;` |
| `subscription_token` | `src/controls/panel/property_list/property_list.cpp:32` | `SubscriptionToken commit_subscription;` |
| `subscription_token` | `src/controls/panel/property_list/property_list.cpp:33` | `SubscriptionToken cancel_subscription;` |
| `subscription_token` | `src/controls/panel/property_list/property_list.cpp:34` | `SubscriptionToken focus_subscription;` |
| `subscription_token` | `src/controls/panel/property_list/property_list.cpp:35` | `SubscriptionToken reset_subscription;` |
| `subscription_token` | `src/controls/panel/property_list/property_list.cpp:36` | `SubscriptionToken reset_focus_subscription;` |
| `dynamic_pointer_cast` | `src/controls/panel/property_list/property_list.cpp:166` | `if (const auto text = std::dynamic_pointer_cast<TextBox>(state.editor)) {` |
| `dynamic_pointer_cast` | `src/controls/panel/property_list/property_list.cpp:202` | `std::dynamic_pointer_cast<TextBox>(row->editor)) {` |
| `dynamic_pointer_cast` | `src/controls/panel/property_list/property_list.cpp:210` | `std::dynamic_pointer_cast<ComboBox>(state.editor)) {` |
| `weak_lock` | `src/controls/panel/property_list/property_list.cpp:216` | `const auto editor = weak_choice.lock();` |
| `dynamic_pointer_cast` | `src/controls/panel/property_list/property_list.cpp:227` | `std::dynamic_pointer_cast<CheckBox>(state.editor)) {` |
| `weak_lock` | `src/controls/panel/property_list/property_list.cpp:233` | `const auto editor = weak_check.lock();` |
| `make_unique` | `src/controls/panel/property_list/property_list.cpp:438` | `: Panel(std::move(stable_id)), impl_(std::make_unique<Impl>(*this)) {` |
| `dynamic_pointer_cast` | `src/controls/panel/property_list/property_list.cpp:544` | `if (const auto text = std::dynamic_pointer_cast<TextBox>(row->editor)) {` |

Detailed action rows were capped at 400; 344 additional rows remain represented in the summary counts.

## Required next evidence before any lifecycle change

1. Build an AST-backed member/parameter/return graph from the exact compilation database, resolving aliases and templates.
2. Trace retained Control child ownership, parent/observer weakness, Window attachment, Component disposal, Event slot/token revocation, dispatcher work ownership, and ABI subscription records as separate semantic graphs.
3. Instrument construction, attachment, detachment, disposal, revocation, final destruction, and cross-thread work with stable identities and sequence numbers.
4. Identify and reproduce cycles, delayed release, premature release, or allocation pressure before proposing a replacement.
5. Compare any unique ownership, generation-handle pool, intrusive link, or stable-pool candidate against the current shared/weak lifecycle traces and failure behavior.
6. Treat C ABI generational handles and native/platform ownership as fixed boundaries unless separately approved.

## Current decision

The present rewrite preserves shared/weak retained ownership. This inventory is intentionally banked for a future lifecycle-change round and must not be used to smuggle ownership changes into syntax, Delegate/Event, sorting, or storage batches.
