# GUI.Forms ownership inventory

Status: **OBSERVED lexical evidence for a future lifecycle round; no ownership change authorized**.

## Snapshot

| Field | Value |
|---|---|
| Generated UTC | `2026-08-10T21:10:58+00:00` |
| Branch | `main` |
| Commit | `6f29824c85e465da99f51a9faf931fe9e0da2ffe` |
| Dirty entries at scan | `111` |
| Scope | `production include/src plus support` |
| C/C++ files | `552` |
| Corpus SHA-256 | `a9ae33359fd7fbfe3ac10e51b938f254929e9e61bae84884089ef557dc77dc46` |

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
| `qualified-alias:shared_ptr` | 583 |
| `qualified-alias:weak_ptr` | 31 |
| `raw_pointer` | 1047 |
| `shared_ptr` | 2059 |
| `unique_ptr` | 135 |
| `weak_ptr` | 319 |

### Lifetime operations

| Operation | Occurrences |
|---|---:|
| `component_container` | 16 |
| `delete_expression` | 1 |
| `dynamic_pointer_cast` | 195 |
| `free_call` | 2 |
| `make_shared` | 124 |
| `make_unique` | 50 |
| `malloc_family` | 2 |
| `new_expression` | 4 |
| `owner_revocable` | 8 |
| `shared_from_this` | 56 |
| `static_pointer_cast` | 30 |
| `subscription_token` | 285 |
| `weak_from_this` | 10 |
| `weak_lock` | 291 |

### Files with the densest explicit ownership surface

| File | Pointer/alias occurrences |
|---|---:|
| `src/controls/showcase_controls.cpp` | 554 |
| `src/abi/registry/registry.hpp` | 302 |
| `src/core/window/window.cpp` | 207 |
| `src/host/macos/application/macos_host.mm` | 123 |
| `include/gui_forms/window/window.hpp` | 121 |
| `tests/inspection_controls_tests.cpp` | 79 |
| `src/controls/panel/instrument_rack/instrument_rack.cpp` | 78 |
| `tests/showcase_interaction_tests.cpp` | 78 |
| `src/controls/guidance/error_provider/error_provider.cpp` | 74 |
| `tests/core_tests.cpp` | 71 |
| `src/controls/panel/property_grid/property_grid.cpp` | 67 |
| `src/controls/panel/property_list/property_list.cpp` | 62 |
| `demo/vsync_lab.cpp` | 57 |
| `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp` | 53 |
| `tests/binding_tests.cpp` | 53 |
| `tests/basic_controls_tests.cpp` | 52 |
| `tests/layout_panel_tests.cpp` | 51 |
| `src/abi/drawing_c_api.cpp` | 50 |
| `src/controls/tool_tip/tool_tip.cpp` | 49 |
| `src/host/macos/services/appkit_host_services.mm` | 46 |
| `src/controls/gallery_controls.cpp` | 42 |
| `src/controls/guidance/help_provider/help_provider.cpp` | 40 |
| `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp` | 40 |
| `src/controls/inspection/property_editor_registry/property_editor_registry.cpp` | 35 |
| `include/gui_forms/c_api.h` | 34 |
| `src/host/windows/application/windows_host.cpp` | 34 |
| `tests/menu_controls_tests.cpp` | 34 |
| `src/render/skia/raster/skia_raster.cpp` | 32 |
| `tests/macos_host_close_tests.mm` | 32 |
| `tests/c_api_c11_tests.c` | 31 |

## Retained/lifecycle-focused class declarations

This is a review queue, not a proposed graph rewrite.

| Owner | Kind | Target | Location | Declaration evidence |
|---|---|---|---|---|
| `AbiPropertyObjectControl` | `shared_ptr` | `PropertyState` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:749` | `std::map<std::string, std::shared_ptr<PropertyState>> properties_;` |
| `Aggregate` | `shared_ptr` | `Control` | `src/controls/guidance/error_provider/error_provider.cpp:442` | `std::shared_ptr<Control> target;` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:24` | `AnchoredPopupLayer(StableId stable_id, Control::Ptr anchor,` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:27` | `[[nodiscard]] Control::Ptr anchor() const noexcept { return anchor_.lock(); }` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:28` | `void set_anchor(Control::Ptr anchor);` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:29` | `[[nodiscard]] Control::Ptr content() const noexcept { return content_; }` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:30` | `void set_content(Control::Ptr content);` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:57` | `void validate_anchor(const Control::Ptr& anchor) const;` |
| `AnchoredPopupLayer` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:59` | `Control::WeakPtr anchor_;` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:60` | `Control::Ptr content_;` |
| `AppendDispatcherCharacter` | `weak_ptr` | `ShowcaseContext` | `src/controls/showcase_controls.cpp:1241` | `std::weak_ptr<ShowcaseContext> context_;` |
| `AttachChildOnce` | `qualified-alias:shared_ptr` | `Control` | `tests/core_tests.cpp:225` | `AttachChildOnce(Control& parent, const Control::Ptr& child,` |
| `AttachChildOnce` | `qualified-alias:shared_ptr` | `Control` | `tests/core_tests.cpp:237` | `const Control::Ptr& child_;` |
| `AutomationResolver` | `shared_ptr` | `std::unordered_map< std::string, std::shared_ptr<Control>>` | `src/abi/registry/registry.hpp:2509` | `std::shared_ptr<std::unordered_map<` |
| `AutomationResolver` | `shared_ptr` | `Control` | `src/abi/registry/registry.hpp:2510` | `std::string, std::shared_ptr<Control>>> controls;` |
| `BeginWorkerDispatch` | `shared_ptr` | `RearmProbeState` | `tests/macos_host_close_tests.mm:126` | `std::shared_ptr<RearmProbeState> state_;` |
| `Binding` | `shared_ptr` | `BindingSource` | `include/gui_forms/binding/binding/binding.hpp:21` | `std::shared_ptr<BindingSource> source, std::string data_member,` |
| `Binding` | `weak_ptr` | `BindingSource` | `include/gui_forms/binding/binding/binding.hpp:86` | `std::weak_ptr<BindingSource> source_;` |
| `BindingContext` | `shared_ptr` | `BindingSource` | `include/gui_forms/binding/binding_context/binding_context.hpp:22` | `void add(const std::shared_ptr<BindingSource>& source);` |
| `BindingContext` | `shared_ptr` | `BindingSource` | `include/gui_forms/binding/binding_context/binding_context.hpp:23` | `CurrencyManager& manager(const std::shared_ptr<BindingSource>& source);` |
| `BindingContext` | `weak_ptr` | `detail::WindowLifetime` | `include/gui_forms/binding/binding_context/binding_context.hpp:51` | `std::weak_ptr<detail::WindowLifetime> window_lifetime_;` |
| `BindingNavigationClick` | `weak_ptr` | `ShowcaseContext` | `src/controls/showcase_controls.cpp:3463` | `std::weak_ptr<ShowcaseContext> context_;` |
| `BindingSource` | `weak_ptr` | `Binding` | `include/gui_forms/binding/binding_source/binding_source.hpp:128` | `using WeakBindingList = std::vector<std::weak_ptr<Binding>>;` |
| `BindingSource` | `shared_ptr` | `Binding` | `include/gui_forms/binding/binding_source/binding_source.hpp:134` | `void register_binding(const std::shared_ptr<Binding>& binding);` |
| `BindingSource` | `weak_ptr` | `detail::WindowLifetime` | `include/gui_forms/binding/binding_source/binding_source.hpp:140` | `std::weak_ptr<detail::WindowLifetime> window_lifetime_;` |
| `BindingSource` | `unique_ptr` | `CurrencyManager` | `include/gui_forms/binding/binding_source/binding_source.hpp:142` | `std::unique_ptr<CurrencyManager> currency_manager_;` |
| `BindingStatusUpdater` | `weak_ptr` | `ShowcaseContext` | `src/controls/showcase_controls.cpp:3436` | `std::weak_ptr<ShowcaseContext> context_;` |
| `ButtonBase` | `shared_ptr` | `ImageList` | `include/gui_forms/controls/button_base/button_base.hpp:62` | `void set_image_list(std::shared_ptr<ImageList> image_list);` |
| `ButtonBase` | `shared_ptr` | `ImageList` | `include/gui_forms/controls/button_base/button_base.hpp:153` | `std::shared_ptr<ImageList> image_list_;` |
| `CancelDispatcherClick` | `weak_ptr` | `ShowcaseContext` | `src/controls/showcase_controls.cpp:1376` | `std::weak_ptr<ShowcaseContext> context_;` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:41` | `[[nodiscard]] Control::Ptr header() const noexcept { return header_; }` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:42` | `[[nodiscard]] Control::Ptr body() const noexcept { return body_; }` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:43` | `[[nodiscard]] Control::Ptr footer() const noexcept { return footer_; }` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:44` | `[[nodiscard]] Control::Ptr set_header(Control::Ptr control);` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:44` | `[[nodiscard]] Control::Ptr set_header(Control::Ptr control);` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:45` | `[[nodiscard]] Control::Ptr set_body(Control::Ptr control);` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:45` | `[[nodiscard]] Control::Ptr set_body(Control::Ptr control);` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:46` | `[[nodiscard]] Control::Ptr set_footer(Control::Ptr control);` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:46` | `[[nodiscard]] Control::Ptr set_footer(Control::Ptr control);` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:85` | `Control::Ptr replace_section(Control::Ptr& slot, Control::Ptr replacement);` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:85` | `Control::Ptr replace_section(Control::Ptr& slot, Control::Ptr replacement);` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:85` | `Control::Ptr replace_section(Control::Ptr& slot, Control::Ptr replacement);` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:88` | `Control::Ptr header_;` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:89` | `Control::Ptr body_;` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/card/card.hpp:90` | `Control::Ptr footer_;` |
| `CommandBinding` | `shared_ptr` | `Command` | `include/gui_forms/commands/command_binding/command_binding.hpp:13` | `CommandBinding(std::shared_ptr<Command> command,` |
| `CommandBinding` | `shared_ptr` | `ButtonBase` | `include/gui_forms/commands/command_binding/command_binding.hpp:14` | `std::shared_ptr<ButtonBase> button,` |
| `CommandBinding` | `shared_ptr` | `Command` | `include/gui_forms/commands/command_binding/command_binding.hpp:32` | `std::shared_ptr<Command> command_;` |
| `CommandBinding` | `shared_ptr` | `ButtonBase` | `include/gui_forms/commands/command_binding/command_binding.hpp:33` | `std::shared_ptr<ButtonBase> button_;` |
| `CompatibilityPaintBinding` | `weak_ptr` | `RasterControl` | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.hpp:17` | `std::weak_ptr<RasterControl> target;` |
| `CompatibilityPaintBinding` | `shared_ptr` | `gui_forms::LiveSurface` | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.hpp:19` | `std::shared_ptr<gui_forms::LiveSurface> surface;` |
| `CompatibilityPaintWrite` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.hpp:23` | `std::shared_ptr<gui_forms::host::WindowsCompatibilityPaintEndpoint> endpoint;` |
| `CompleteDispatcherTurnOne` | `weak_ptr` | `ShowcaseContext` | `src/controls/showcase_controls.cpp:1305` | `std::weak_ptr<ShowcaseContext> context_;` |
| `CompleteDispatcherTurnTwo` | `weak_ptr` | `ShowcaseContext` | `src/controls/showcase_controls.cpp:1265` | `std::weak_ptr<ShowcaseContext> context_;` |
| `Component` | `alias:shared_ptr` | `Component` | `include/gui_forms/component/component/component.hpp:13` | `using Ptr = std::shared_ptr<Component>;` |
| `Component` | `shared_ptr` | `Component` | `include/gui_forms/component/component/component.hpp:13` | `using Ptr = std::shared_ptr<Component>;` |
| `Component` | `weak_ptr` | `detail::Revocable` | `include/gui_forms/component/component/component.hpp:30` | `void own_revocable(const std::weak_ptr<detail::Revocable>& revocable);` |
| `Component` | `weak_ptr` | `detail::Revocable` | `include/gui_forms/component/component/component.hpp:39` | `std::vector<std::weak_ptr<detail::Revocable>> owned_revocables_;` |
| `ComponentContainer` | `qualified-alias:shared_ptr` | `Component` | `include/gui_forms/component/component_container/component_container.hpp:17` | `void add(Component::Ptr component);` |
| `ComponentContainer` | `qualified-alias:shared_ptr` | `Component` | `include/gui_forms/component/component_container/component_container.hpp:18` | `[[nodiscard]] Component::Ptr remove(const Component& component);` |
| `ComponentContainer` | `qualified-alias:shared_ptr` | `Component` | `include/gui_forms/component/component_container/component_container.hpp:19` | `[[nodiscard]] std::span<const Component::Ptr> components() const noexcept {` |
| `ComponentContainer` | `qualified-alias:shared_ptr` | `Component` | `include/gui_forms/component/component_container/component_container.hpp:27` | `using ComponentList = std::vector<Component::Ptr>;` |
| `ContainerControl` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/scrollable_control/container_control/container_control.hpp:11` | `[[nodiscard]] bool contains_descendant(const Control::Ptr& control) const noexcept;` |
| `ContainerControl` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/scrollable_control/container_control/container_control.hpp:12` | `[[nodiscard]] Control::Ptr active_control() const noexcept;` |
| `ContainerControl` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/scrollable_control/container_control/container_control.hpp:13` | `bool request_active_control(const Control::Ptr& control);` |
| `ContextMenu` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/components/context_menu/context_menu.hpp:79` | `void show(const Control::Ptr& owner, Point window_position);` |
| `ContextMenu` | `qualified-alias:shared_ptr` | `Control` | `src/controls/menu/context_menu/context_menu.cpp:332` | `void show(const Control::Ptr& invoker, Point position,` |
| `ContextMenu` | `qualified-alias:shared_ptr` | `Control` | `src/controls/menu/context_menu/context_menu.cpp:476` | `[[nodiscard]] Control::Ptr first_focusable(std::size_t depth) const {` |
| `ContextMenu` | `qualified-alias:shared_ptr` | `Control` | `src/controls/menu/context_menu/context_menu.cpp:652` | `Control::Ptr owner_control;` |
| `Control` | `alias:shared_ptr` | `Control` | `include/gui_forms/control/control/control.hpp:313` | `using Ptr = std::shared_ptr<Control>;` |
| `Control` | `shared_ptr` | `Control` | `include/gui_forms/control/control/control.hpp:313` | `using Ptr = std::shared_ptr<Control>;` |
| `Control` | `alias:weak_ptr` | `Control` | `include/gui_forms/control/control/control.hpp:314` | `using WeakPtr = std::weak_ptr<Control>;` |
| `Control` | `weak_ptr` | `Control` | `include/gui_forms/control/control/control.hpp:314` | `using WeakPtr = std::weak_ptr<Control>;` |
| `Control` | `shared_ptr` | `const Theme` | `include/gui_forms/control/control/control.hpp:402` | `void set_theme_override(std::shared_ptr<const Theme> theme);` |
| `Control` | `shared_ptr` | `const PropertyEnumDescriptor` | `include/gui_forms/control/control/control.hpp:708` | `std::shared_ptr<const PropertyEnumDescriptor> enumeration,` |
| `Control` | `shared_ptr` | `detail::DispatcherState` | `include/gui_forms/control/control/control.hpp:732` | `std::shared_ptr<detail::DispatcherState> dispatcher_state_;` |
| `Control` | `shared_ptr` | `const detail::DisplayChunk` | `include/gui_forms/control/control/control.hpp:756` | `std::shared_ptr<const detail::DisplayChunk> display_chunk_;` |
| `Control` | `shared_ptr` | `const Theme` | `include/gui_forms/control/control/control.hpp:771` | `std::shared_ptr<const Theme> theme_override_;` |
| `Control` | `unique_ptr` | `ControlBindingsCollection` | `include/gui_forms/control/control/control.hpp:792` | `mutable std::unique_ptr<ControlBindingsCollection> data_bindings_;` |
| `ControlAvailabilityPublication` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:500` | `Control::Ptr control;` |
| `ControlBindingsCollection` | `shared_ptr` | `BindingSource` | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:25` | `std::string property_name, std::shared_ptr<BindingSource> source,` |
| `ControlBindingsCollection` | `shared_ptr` | `BindingSource` | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:28` | `std::string property_name, std::shared_ptr<BindingSource> source,` |
| `ControlBindingsCollection` | `shared_ptr` | `Binding` | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:30` | `void add(std::shared_ptr<Binding> binding);` |
| `ControlBindingsCollection` | `shared_ptr` | `Binding` | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:50` | `using BindingList = std::vector<std::shared_ptr<Binding>>;` |
| `ControlFactory` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/control/static_tree/control_factory/control_factory.hpp:23` | `using Creator = std::function<Control::Ptr(StableId)>;` |
| `ControlFactory` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/control/static_tree/control_factory/control_factory.hpp:26` | `[[nodiscard]] Control::Ptr create(std::string_view type, StableId stable_id) const;` |
| `ControlPointerForwarder` | `shared_ptr` | `SubscriptionRecord` | `src/abi/registry/registry.hpp:2623` | `std::shared_ptr<SubscriptionRecord> subscription;` |
| `ControlPointerForwarder` | `weak_ptr` | `Control` | `src/abi/registry/registry.hpp:2625` | `std::weak_ptr<Control> control;` |
| `ControlRecord` | `shared_ptr` | `Control` | `src/abi/registry/registry.hpp:43` | `std::shared_ptr<Control> control;` |
| `ControlRecord` | `weak_ptr` | `Control` | `src/abi/registry/registry.hpp:45` | `std::vector<std::pair<std::weak_ptr<Control>, gui_forms::PaintPlane>>` |
| `CrossThreadPost` | `qualified-alias:shared_ptr` | `Control` | `tests/dispatcher_tests.cpp:135` | `CrossThreadPost(Window& window, const Control::Ptr& root,` |
| `CrossThreadPost` | `qualified-alias:shared_ptr` | `Control` | `tests/dispatcher_tests.cpp:154` | `const Control::Ptr& root_;` |
| `CustomEditorCommit` | `qualified-alias:weak_ptr` | `Control` | `src/controls/panel/property_grid/property_grid.cpp:94` | `Control::WeakPtr owner_lifetime;` |
| `CustomEditorFailure` | `qualified-alias:weak_ptr` | `Control` | `src/controls/panel/property_grid/property_grid.cpp:106` | `Control::WeakPtr owner_lifetime;` |
| `DatePopupStatus` | `weak_ptr` | `Label` | `src/controls/showcase_controls.cpp:2690` | `std::weak_ptr<Label> status_;` |
| `DateTimePicker` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/date_time_picker/date_time_picker.hpp:117` | `Control::Ptr popup_calendar_;` |
| `DeferredLiveSurfacePaint` | `weak_ptr` | `RasterControl` | `src/abi/control_adapters/raster_control/raster_control.hpp:241` | `std::weak_ptr<RasterControl> target;` |
| `DeferredLiveSurfacePaint` | `weak_ptr` | `LiveWakeState` | `src/abi/control_adapters/raster_control/raster_control.hpp:242` | `std::weak_ptr<LiveWakeState> state;` |
| `DensityPopupChanged` | `weak_ptr` | `Label` | `src/controls/showcase_controls.cpp:1773` | `std::weak_ptr<Label> status_;` |
| `DispatchGuard` | `shared_ptr` | `ControlRecord` | `src/abi/registry/registry.hpp:2504` | `std::shared_ptr<ControlRecord> root;` |
| `DispatchPendingCallback` | `shared_ptr` | `ControlRecord` | `src/abi/registry/registry.hpp:2567` | `std::shared_ptr<ControlRecord> record;` |
| `DispatchWork` | `qualified-alias:weak_ptr` | `Control` | `src/core/dispatcher/state/dispatcher_state.hpp:21` | `Control::WeakPtr owner;` |
| `DispatchWork` | `weak_ptr` | `DispatcherState` | `src/core/dispatcher/state/dispatcher_state.hpp:24` | `std::weak_ptr<DispatcherState> dispatcher;` |
| `DispatcherState` | `shared_ptr` | `DispatchWork` | `src/core/dispatcher/state/dispatcher_state.hpp:32` | `using Queue = std::deque<std::shared_ptr<DispatchWork>>;` |
| `DispatcherWorker` | `weak_ptr` | `ShowcaseContext` | `src/controls/showcase_controls.cpp:1420` | `std::weak_ptr<ShowcaseContext> context_;` |
| `DisposeCapturedControl` | `qualified-alias:shared_ptr` | `Control` | `tests/core_tests.cpp:246` | `explicit DisposeCapturedControl(Control::Ptr control) noexcept` |
| `DisposeCapturedControl` | `qualified-alias:shared_ptr` | `Control` | `tests/core_tests.cpp:254` | `Control::Ptr control_;` |
| `DisposeCapturedControl` | `shared_ptr` | `ControlType` | `tests/support/named_callbacks.hpp:126` | `std::shared_ptr<ControlType> control_;` |
| `DisposeOwnerAfterRecording` | `shared_ptr` | `ProbeComponent` | `tests/retained_lifetime_tests.cpp:134` | `std::shared_ptr<ProbeComponent> owner_;` |
| `ErrorProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/error_provider/error_provider.hpp:80` | `[[nodiscard]] bool can_extend(const std::shared_ptr<Control>& target) const;` |
| `ErrorProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/error_provider/error_provider.hpp:81` | `void set_error(const std::shared_ptr<Control>& target, std::string error);` |
| `ErrorProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/error_provider/error_provider.hpp:86` | `void set_icon_alignment(const std::shared_ptr<Control>& target,` |
| `ErrorProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/error_provider/error_provider.hpp:89` | `void set_icon_padding(const std::shared_ptr<Control>& target, double padding);` |
| `ErrorProvider` | `shared_ptr` | `BindingSource` | `include/gui_forms/components/error_provider/error_provider.hpp:116` | `void set_data_source(std::shared_ptr<BindingSource> source);` |
| `ErrorProvider` | `shared_ptr` | `BindingSource` | `include/gui_forms/components/error_provider/error_provider.hpp:121` | `void bind_to_data_and_errors(std::shared_ptr<BindingSource> source,` |
| `ErrorProvider` | `weak_ptr` | `Control` | `include/gui_forms/components/error_provider/error_provider.hpp:148` | `const Binding*, std::pair<std::weak_ptr<Control>, std::string>>;` |
| `ErrorProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/error_provider/error_provider.hpp:154` | `[[nodiscard]] Entry& require_entry(const std::shared_ptr<Control>& target);` |
| `ErrorProvider` | `weak_ptr` | `detail::WindowLifetime` | `include/gui_forms/components/error_provider/error_provider.hpp:172` | `std::weak_ptr<detail::WindowLifetime> window_lifetime_;` |
| `ErrorProvider` | `weak_ptr` | `BindingSource` | `include/gui_forms/components/error_provider/error_provider.hpp:186` | `std::weak_ptr<BindingSource> data_source_;` |
| `ErrorProvider` | `weak_ptr` | `Control` | `include/gui_forms/components/error_provider/error_provider.hpp:188` | `std::unordered_map<std::uint64_t, std::weak_ptr<Control>> bound_targets_;` |
| `ErrorProvider` | `weak_ptr` | `Control` | `src/controls/guidance/error_provider/error_provider.cpp:25` | `std::weak_ptr<Control> target;` |
| `ErrorProvider` | `unique_ptr` | `PopupToken` | `src/controls/guidance/error_provider/error_provider.cpp:31` | `std::unique_ptr<PopupToken> popup;` |
| `ErrorProvider` | `weak_ptr` | `Control` | `src/controls/guidance/error_provider/error_provider.cpp:37` | `std::weak_ptr<Control> target;` |
| `Event` | `shared_ptr` | `State` | `include/gui_forms/event/event/event.hpp:209` | `std::shared_ptr<State> state_;` |
| `FieldState` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/instrument_rack/instrument_rack.cpp:102` | `Control::Ptr editor;` |
| `FinalSnapshotCallback` | `shared_ptr` | `ControlRecord` | `src/abi/registry/registry.hpp:2586` | `std::shared_ptr<ControlRecord> record;` |
| `Fixture` | `shared_ptr` | `BindingSource` | `tests/binding_tests.cpp:55` | `std::shared_ptr<BindingSource> source;` |
| `Fixture` | `unique_ptr` | `Window` | `tests/core_tests.cpp:427` | `std::unique_ptr<Window> window;` |
| `Fixture` | `unique_ptr` | `Window` | `tests/display_chunk_tests.cpp:85` | `std::unique_ptr<Window> window;` |
| `Fixture` | `qualified-alias:shared_ptr` | `Control` | `tests/frame_scheduler_tests.cpp:80` | `Control::Ptr root = make_control<Control>(StableId("scheduler.root"));` |
| `Fixture` | `unique_ptr` | `Window` | `tests/frame_scheduler_tests.cpp:81` | `std::unique_ptr<Window> window;` |
| `Fixture` | `shared_ptr` | `TabControl` | `tests/tab_control_tests.cpp:42` | `std::shared_ptr<TabControl> tabs;` |
| `Fixture` | `qualified-alias:shared_ptr` | `Control` | `tests/timer_tests.cpp:29` | `Control::Ptr root = make_control<Control>(StableId("timer.root"));` |
| `Fixture` | `qualified-alias:shared_ptr` | `Control` | `tests/tooltip_tests.cpp:49` | `Control::Ptr root = make_control<Control>(StableId("tooltip.root"));` |
| `FocusChangePublication` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:481` | `Control::Ptr control;` |
| `FocusContainerDescendantClick` | `weak_ptr` | `ContainerControl` | `src/controls/showcase_controls.cpp:613` | `std::weak_ptr<ContainerControl> container_;` |
| `FocusFixture` | `unique_ptr` | `Window` | `tests/focus_scope_tests.cpp:46` | `std::unique_ptr<Window> window;` |
| `FocusScopePublication` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:487` | `Control::Ptr owner;` |
| `FocusScopeState` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:684` | `Control::WeakPtr root;` |
| `FocusScopeState` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:685` | `Control::WeakPtr previous_focus;` |
| `ForeignConverterFormatter` | `weak_ptr` | `State` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:58` | `std::weak_ptr<State> state;` |
| `ForeignConverterParser` | `weak_ptr` | `State` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:73` | `std::weak_ptr<State> state;` |
| `ForeignEditorCommitConnector` | `weak_ptr` | `State` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:141` | `std::weak_ptr<State> state;` |
| `ForeignEditorCommitConnector` | `shared_ptr` | `gui_forms::BindingValue` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:142` | `std::shared_ptr<gui_forms::BindingValue> current;` |
| `ForeignEditorCommitConnector` | `shared_ptr` | `gui_forms::Event< const gui_forms::PropertyEditorInputError&>` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:143` | `std::shared_ptr<gui_forms::Event<` |
| `ForeignEditorCommitRelay` | `weak_ptr` | `State` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:119` | `std::weak_ptr<State> state;` |
| `ForeignEditorCommitRelay` | `shared_ptr` | `gui_forms::BindingValue` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:120` | `std::shared_ptr<gui_forms::BindingValue> current;` |
| `ForeignEditorCommitRelay` | `shared_ptr` | `gui_forms::Event< const gui_forms::PropertyEditorInputError&>` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:121` | `std::shared_ptr<gui_forms::Event<` |
| `ForeignEditorFailureConnector` | `shared_ptr` | `gui_forms::Event< const gui_forms::PropertyEditorInputError&>` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:158` | `std::shared_ptr<gui_forms::Event<` |
| `ForeignEditorSynchronizer` | `shared_ptr` | `gui_forms::BindingValue` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:108` | `std::shared_ptr<gui_forms::BindingValue> current;` |
| `ForeignEditorTextUpdater` | `weak_ptr` | `State` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:89` | `std::weak_ptr<State> state;` |
| `ForeignPropertyChangeConnector` | `shared_ptr` | `State` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:24` | `std::shared_ptr<State> state;` |
| `ForeignPropertyEditorFactory` | `weak_ptr` | `State` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:171` | `std::weak_ptr<State> state;` |
| `ForeignPropertyGetter` | `shared_ptr` | `State` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:9` | `std::shared_ptr<State> state;` |
| `ForeignPropertyOrigin` | `shared_ptr` | `State` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:48` | `std::shared_ptr<State> state;` |
| `ForeignPropertyResetter` | `shared_ptr` | `State` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:33` | `std::shared_ptr<State> state;` |
| `ForeignPropertySetter` | `shared_ptr` | `State` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:15` | `std::shared_ptr<State> state;` |
| `ForeignPropertyShouldSerialize` | `shared_ptr` | `State` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:42` | `std::shared_ptr<State> state;` |
| `GalleryControl` | `shared_ptr` | `GalleryContext` | `src/controls/gallery/control/gallery_control.hpp:16` | `std::shared_ptr<GalleryContext> context);` |
| `GalleryControl` | `shared_ptr` | `GalleryContext` | `src/controls/gallery/control/gallery_control.hpp:35` | `std::shared_ptr<GalleryContext> context_;` |
| `GalleryTree` | `qualified-alias:shared_ptr` | `Control` | `src/controls/gallery/tree/gallery_tree.hpp:11` | `Control::Ptr root;` |
| `HelpProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/help_provider/help_provider.hpp:65` | `[[nodiscard]] bool can_extend(const std::shared_ptr<Control>& target) const;` |
| `HelpProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/help_provider/help_provider.hpp:66` | `void set_help_string(const std::shared_ptr<Control>& target, std::string text);` |
| `HelpProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/help_provider/help_provider.hpp:68` | `void set_help_keyword(const std::shared_ptr<Control>& target,` |
| `HelpProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/help_provider/help_provider.hpp:71` | `void set_help_navigator(const std::shared_ptr<Control>& target,` |
| `HelpProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/help_provider/help_provider.hpp:74` | `void set_show_help(const std::shared_ptr<Control>& target, bool show);` |
| `HelpProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/help_provider/help_provider.hpp:86` | `bool request_help(const std::shared_ptr<Control>& target, Point position,` |
| `HelpProvider` | `shared_ptr` | `Control` | `include/gui_forms/components/help_provider/help_provider.hpp:112` | `[[nodiscard]] Entry& require_entry(const std::shared_ptr<Control>& target);` |
| `HelpProvider` | `weak_ptr` | `detail::WindowLifetime` | `include/gui_forms/components/help_provider/help_provider.hpp:118` | `std::weak_ptr<detail::WindowLifetime> window_lifetime_;` |
| `HelpProvider` | `weak_ptr` | `Control` | `src/controls/guidance/help_provider/help_provider.cpp:20` | `std::weak_ptr<Control> target;` |
| `HostClosedCallback` | `shared_ptr` | `ControlRecord` | `src/abi/registry/registry.hpp:2575` | `std::shared_ptr<ControlRecord> record;` |
| `HostFinishGuard` | `shared_ptr` | `ControlRecord` | `src/abi/registry/registry.hpp:2492` | `std::shared_ptr<ControlRecord> record;` |
| `HostReadyCallback` | `shared_ptr` | `ControlRecord` | `src/abi/registry/registry.hpp:2533` | `std::shared_ptr<ControlRecord> record;` |
| `HostServiceClick` | `shared_ptr` | `ShowcaseContext` | `src/controls/showcase_controls.cpp:3147` | `std::shared_ptr<ShowcaseContext> context_;` |
| `ImageList` | `weak_ptr` | `detail::WindowLifetime` | `include/gui_forms/image_list/image_list/image_list.hpp:147` | `std::weak_ptr<detail::WindowLifetime> window_lifetime_;` |
| `ImageSnapshot` | `shared_ptr` | `const PixelStorage` | `include/gui_forms/drawing/image_snapshot/image_snapshot.hpp:27` | `std::shared_ptr<const PixelStorage> storage_;` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/instrument_rack/instrument_rack.hpp:21` | `[[nodiscard]] Control::Ptr field_editor(std::string_view module_id,` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/instrument_rack/instrument_rack.hpp:37` | `void set_action_content(Control::Ptr content,` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/instrument_rack/instrument_rack.hpp:39` | `[[nodiscard]] Control::Ptr action_content() const noexcept;` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/instrument_rack/instrument_rack.cpp:253` | `void connect_focus(ModuleState& module, Control::Ptr control,` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/instrument_rack/instrument_rack.cpp:383` | `Control::Ptr create_field_editor(ModuleState& module,` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/instrument_rack/instrument_rack.cpp:681` | `Control::Ptr action_content;` |
| `InvokeDispatcherFromWorkerClick` | `weak_ptr` | `ShowcaseContext` | `src/controls/showcase_controls.cpp:1443` | `std::weak_ptr<ShowcaseContext> context_;` |
| `Item` | `qualified-alias:shared_ptr` | `Control` | `src/controls/scrollable_control/container_control/flow_layout_panel/flow_layout_panel.cpp:103` | `Control::Ptr control;` |
| `Item` | `qualified-alias:shared_ptr` | `Control` | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:371` | `Control::Ptr control;` |
| `KeyPreviewForwarder` | `shared_ptr` | `SubscriptionRecord` | `src/abi/registry/registry.hpp:2677` | `std::shared_ptr<SubscriptionRecord> subscription;` |
| `KeyPreviewForwarder` | `shared_ptr` | `ControlRecord` | `src/abi/registry/registry.hpp:2679` | `std::shared_ptr<ControlRecord> sender;` |
| `LabContext` | `unique_ptr` | `Timer` | `demo/vsync_lab.cpp:404` | `std::unique_ptr<Timer> telemetry_timer;` |
| `LiveSurface` | `shared_ptr` | `detail::LiveSurfaceState` | `include/gui_forms/live_surface/surface/live_surface.hpp:38` | `explicit LiveSurface(std::shared_ptr<detail::LiveSurfaceState> state) noexcept;` |
| `LiveSurface` | `shared_ptr` | `detail::LiveSurfaceState` | `include/gui_forms/live_surface/surface/live_surface.hpp:39` | `std::shared_ptr<detail::LiveSurfaceState> state_;` |
| `LiveSurfaceRegistration` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:708` | `Control::WeakPtr control;` |
| `LiveSurfaceState` | `shared_ptr` | `LiveSurfaceBuffer` | `src/core/live_surface/state/live_surface_state.hpp:28` | `std::vector<std::shared_ptr<LiveSurfaceBuffer>> buffers;` |
| `LiveSurfaceState` | `shared_ptr` | `LiveSurfaceWake` | `src/core/live_surface/state/live_surface_state.hpp:39` | `std::vector<std::shared_ptr<LiveSurfaceWake>> wakes;` |
| `LiveSurfaceWakeCallback` | `weak_ptr` | `RasterControl` | `src/abi/control_adapters/raster_control/raster_control.hpp:250` | `std::weak_ptr<RasterControl> target;` |
| `LiveSurfaceWakeCallback` | `weak_ptr` | `LiveWakeState` | `src/abi/control_adapters/raster_control/raster_control.hpp:251` | `std::weak_ptr<LiveWakeState> state;` |
| `LiveSurfaceWakeConnection` | `weak_ptr` | `detail::LiveSurfaceState` | `include/gui_forms/live_surface/wake_connection/live_surface_wake_connection.hpp:32` | `std::weak_ptr<detail::LiveSurfaceState> state,` |
| `LiveSurfaceWakeConnection` | `weak_ptr` | `detail::LiveSurfaceState` | `include/gui_forms/live_surface/wake_connection/live_surface_wake_connection.hpp:35` | `std::weak_ptr<detail::LiveSurfaceState> state_;` |
| `LiveSurfaceWriteLease` | `shared_ptr` | `detail::LiveSurfaceState` | `include/gui_forms/live_surface/write_lease/live_surface_write_lease.hpp:43` | `LiveSurfaceWriteLease(std::shared_ptr<detail::LiveSurfaceState> state,` |
| `LiveSurfaceWriteLease` | `shared_ptr` | `detail::LiveSurfaceState` | `include/gui_forms/live_surface/write_lease/live_surface_write_lease.hpp:47` | `std::shared_ptr<detail::LiveSurfaceState> state_;` |
| `MacApplicationWindow` | `unique_ptr` | `Window` | `include/gui_forms/platform/macos_host.hpp:48` | `std::unique_ptr<Window> model;` |
| `ManagedCallbackGuard` | `shared_ptr` | `ControlRecord` | `src/abi/registry/registry.hpp:2498` | `std::shared_ptr<ControlRecord> root;` |
| `MarshalledInvokeWorker` | `qualified-alias:shared_ptr` | `Control` | `tests/dispatcher_tests.cpp:181` | `MarshalledInvokeWorker(const Control::Ptr& child, std::string& trace,` |
| `MarshalledInvokeWorker` | `qualified-alias:shared_ptr` | `Control` | `tests/dispatcher_tests.cpp:194` | `const Control::Ptr& child_;` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:57` | `[[nodiscard]] Control::Ptr master() const noexcept { return master_; }` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:58` | `[[nodiscard]] Control::Ptr detail() const noexcept { return detail_; }` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:59` | `[[nodiscard]] Control::Ptr set_master(Control::Ptr control);` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:59` | `[[nodiscard]] Control::Ptr set_master(Control::Ptr control);` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:60` | `[[nodiscard]] Control::Ptr set_detail(Control::Ptr control);` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:60` | `[[nodiscard]] Control::Ptr set_detail(Control::Ptr control);` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:96` | `[[nodiscard]] Control::Ptr replace_role(Control::Ptr& slot,` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:96` | `[[nodiscard]] Control::Ptr replace_role(Control::Ptr& slot,` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:98` | `Control::Ptr replacement);` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:105` | `Control::Ptr master_;` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:106` | `Control::Ptr detail_;` |
| `ModuleState` | `shared_ptr` | `RackModulePanel` | `src/controls/panel/instrument_rack/instrument_rack.cpp:113` | `std::shared_ptr<RackModulePanel> panel;` |
| `ModuleState` | `shared_ptr` | `CheckBox` | `src/controls/panel/instrument_rack/instrument_rack.cpp:114` | `std::shared_ptr<CheckBox> enable;` |
| `ModuleState` | `shared_ptr` | `Label` | `src/controls/panel/instrument_rack/instrument_rack.cpp:116` | `std::shared_ptr<Label> status;` |
| `ModuleState` | `shared_ptr` | `Button` | `src/controls/panel/instrument_rack/instrument_rack.cpp:117` | `std::shared_ptr<Button> remove;` |
| `NumericUpDown` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/numeric_up_down/numeric_up_down.hpp:59` | `Control::Ptr spinner_;` |
| `ObjectView` | `shared_ptr` | `ImageList` | `include/gui_forms/controls/panel/object_view/object_view.hpp:109` | `void set_image_list(std::shared_ptr<ImageList> image_list);` |
| `ObjectView` | `shared_ptr` | `ImageList` | `include/gui_forms/controls/panel/object_view/object_view.hpp:182` | `std::shared_ptr<ImageList> image_list_;` |
| `ObserveCommittedCheckState` | `shared_ptr` | `CheckedListBox` | `tests/checked_list_box_tests.cpp:85` | `std::shared_ptr<CheckedListBox> list_;` |
| `ObserveCommittedSourceAmount` | `shared_ptr` | `BindingSource` | `tests/binding_tests.cpp:312` | `const std::shared_ptr<BindingSource>& source_;` |
| `ObserveManagerCommittedCombo` | `shared_ptr` | `Binding` | `tests/binding_tests.cpp:292` | `const std::shared_ptr<Binding>& binding_;` |
| `OwnerEdit` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/property_grid/property_grid.cpp:868` | `Control::Ptr object;` |
| `OwnerInvokeWaiter` | `qualified-alias:shared_ptr` | `Control` | `tests/dispatcher_tests.cpp:260` | `OwnerInvokeWaiter(const Control::Ptr& child,` |
| `OwnerInvokeWaiter` | `qualified-alias:shared_ptr` | `Control` | `tests/dispatcher_tests.cpp:273` | `const Control::Ptr& child_;` |
| `PaintControlCheckpoint` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:717` | `Control::Ptr control;` |
| `PaintControlCheckpoint` | `shared_ptr` | `const detail::DisplayChunk` | `include/gui_forms/window/window.hpp:718` | `std::shared_ptr<const detail::DisplayChunk> chunk;` |
| `PanelState` | `shared_ptr` | `MenuPanel` | `src/controls/menu/context_menu/context_menu.cpp:27` | `std::shared_ptr<MenuPanel> panel;` |
| `PanelState` | `shared_ptr` | `MenuRow` | `src/controls/menu/context_menu/context_menu.cpp:28` | `std::vector<std::shared_ptr<MenuRow>> rows;` |
| `PointerCapturePublication` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:494` | `Control::Ptr owner;` |
| `PopupAttachment` | `qualified-alias:shared_ptr` | `Control` | `src/core/window/popup/popup_attachment.hpp:9` | `PopupAttachment(Window& window, Control::Ptr owner, Control::Ptr popup)` |
| `PopupAttachment` | `qualified-alias:shared_ptr` | `Control` | `src/core/window/popup/popup_attachment.hpp:9` | `PopupAttachment(Window& window, Control::Ptr owner, Control::Ptr popup)` |
| `PopupAttachment` | `qualified-alias:shared_ptr` | `Control` | `src/core/window/popup/popup_attachment.hpp:18` | `[[nodiscard]] Control::Ptr owner() const noexcept { return owner_.lock(); }` |
| `PopupAttachment` | `qualified-alias:shared_ptr` | `Control` | `src/core/window/popup/popup_attachment.hpp:19` | `[[nodiscard]] Control::Ptr popup() const noexcept { return popup_.lock(); }` |
| `PopupAttachment` | `qualified-alias:weak_ptr` | `Control` | `src/core/window/popup/popup_attachment.hpp:32` | `Control::WeakPtr owner_;` |
| `PopupAttachment` | `qualified-alias:weak_ptr` | `Control` | `src/core/window/popup/popup_attachment.hpp:33` | `Control::WeakPtr popup_;` |
| `PopupToken` | `shared_ptr` | `detail::PopupAttachment` | `include/gui_forms/window/window.hpp:71` | `std::shared_ptr<detail::PopupAttachment> attachment_;` |
| `PostDispatcherBatchClick` | `weak_ptr` | `ShowcaseContext` | `src/controls/showcase_controls.cpp:1333` | `std::weak_ptr<ShowcaseContext> context_;` |
| `PostNestedDispatcherTurn` | `weak_ptr` | `ShowcaseContext` | `src/controls/showcase_controls.cpp:1285` | `std::weak_ptr<ShowcaseContext> context_;` |
| `PrepareCloseTestHost` | `shared_ptr` | `RearmProbeState` | `tests/macos_host_close_tests.mm:250` | `std::shared_ptr<RearmProbeState>& rearm_state_;` |
| `PrepareCloseTestHost` | `shared_ptr` | `FrameProbe` | `tests/macos_host_close_tests.mm:251` | `std::shared_ptr<FrameProbe> surface_;` |
| `PrimaryTabSelectionChanged` | `weak_ptr` | `TabControl` | `src/controls/showcase_controls.cpp:2265` | `std::weak_ptr<TabControl> tabs_;` |
| `PropertyEditorBinding` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/inspection/property_editor_registry/property_editor_registry.hpp:15` | `Control::Ptr control;` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:21` | `[[nodiscard]] Control::Ptr selected_object() const noexcept;` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:22` | `void set_selected_object(Control::Ptr object);` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:23` | `[[nodiscard]] std::vector<Control::Ptr> selected_objects() const;` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:24` | `void set_selected_objects(std::vector<Control::Ptr> objects);` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:37` | `[[nodiscard]] Control::Ptr editor(std::string_view property_name) const;` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:62` | `[[nodiscard]] Event<Control::Ptr>& selected_object_changed() noexcept {` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:81` | `Event<Control::Ptr> selected_object_changed_;` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/property_grid/property_grid.cpp:421` | `[[nodiscard]] Control::Ptr target() const noexcept {` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/property_grid/property_grid.cpp:427` | `[[nodiscard]] std::vector<Control::Ptr> targets() const {` |
| `PropertyGrid` | `qualified-alias:weak_ptr` | `Control` | `src/controls/panel/property_grid/property_grid.cpp:1144` | `std::vector<Control::WeakPtr> selected;` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_list/property_list.hpp:30` | `[[nodiscard]] Control::Ptr editor(std::string_view row_id) const;` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_list/property_list.hpp:31` | `bool replace_editor(std::string_view row_id, Control::Ptr editor);` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_list/property_list.hpp:34` | `void set_header_content(Control::Ptr content, double height);` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/property_list/property_list.hpp:35` | `[[nodiscard]] Control::Ptr header_content() const noexcept;` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/property_list/property_list.cpp:480` | `Control::Ptr header;` |
| `RackModulePanel` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/instrument_rack/rack_module_panel/rack_module_panel.hpp:19` | `std::vector<Control::Ptr> fields;` |
| `RasterControl` | `shared_ptr` | `gui_forms::LiveSurface` | `src/abi/control_adapters/raster_control/raster_control.hpp:342` | `std::shared_ptr<gui_forms::LiveSurface> live_surface_;` |
| `RasterControl` | `shared_ptr` | `LiveWakeState` | `src/abi/control_adapters/raster_control/raster_control.hpp:345` | `std::shared_ptr<LiveWakeState> live_wake_state_;` |
| `RasterKeyForwarder` | `shared_ptr` | `SubscriptionRecord` | `src/abi/registry/registry.hpp:2662` | `std::shared_ptr<SubscriptionRecord> subscription;` |
| `RasterPointerForwarder` | `shared_ptr` | `SubscriptionRecord` | `src/abi/registry/registry.hpp:2609` | `std::shared_ptr<SubscriptionRecord> subscription;` |
| `RasterTextForwarder` | `shared_ptr` | `SubscriptionRecord` | `src/abi/registry/registry.hpp:2702` | `std::shared_ptr<SubscriptionRecord> subscription;` |
| `RearmProbeState` | `shared_ptr` | `FrameProbe` | `tests/macos_host_close_tests.mm:30` | `std::shared_ptr<FrameProbe> root;` |
| `RecordNestedDispatch` | `shared_ptr` | `RearmProbeState` | `tests/macos_host_close_tests.mm:93` | `std::shared_ptr<RearmProbeState> state_;` |
| `RecordSynchronousDispatch` | `shared_ptr` | `RearmProbeState` | `tests/macos_host_close_tests.mm:140` | `std::shared_ptr<RearmProbeState> state_;` |
| `RecordWorkerDispatch` | `shared_ptr` | `RearmProbeState` | `tests/macos_host_close_tests.mm:110` | `std::shared_ptr<RearmProbeState> state_;` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | `src/abi/registry/registry.hpp:2718` | `const Control::Ptr& control) {` |
| `Registry` | `shared_ptr` | `ControlRecord` | `src/abi/registry/registry.hpp:2847` | `void cancel_pending(const std::shared_ptr<ControlRecord>& root) noexcept {` |
| `Registry` | `shared_ptr` | `ControlRecord` | `src/abi/registry/registry.hpp:3168` | `const std::shared_ptr<ControlRecord>& sender) {` |
| `RegistrySlot` | `shared_ptr` | `ControlRecord` | `src/abi/registry/registry.hpp:78` | `std::shared_ptr<ControlRecord> control;` |
| `RegistrySlot` | `shared_ptr` | `SubscriptionRecord` | `src/abi/registry/registry.hpp:79` | `std::shared_ptr<SubscriptionRecord> subscription;` |
| `ReplaceChildOnce` | `qualified-alias:shared_ptr` | `Control` | `tests/core_tests.cpp:260` | `const Control::Ptr& added, bool& invoked) noexcept` |
| `ReplaceChildOnce` | `qualified-alias:shared_ptr` | `Control` | `tests/core_tests.cpp:273` | `const Control::Ptr& added_;` |
| `RowState` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/property_list/property_list.cpp:29` | `Control::Ptr editor;` |
| `RowState` | `shared_ptr` | `Button` | `src/controls/panel/property_list/property_list.cpp:30` | `std::shared_ptr<Button> reset_button;` |
| `RunSynchronousDispatch` | `shared_ptr` | `RearmProbeState` | `tests/macos_host_close_tests.mm:168` | `std::shared_ptr<RearmProbeState> state_;` |
| `ScaledGroupBox` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/group_box/scaled_group_box/scaled_group_box.hpp:18` | `void add_at(Control::Ptr child, Rect design_bounds);` |
| `ScaledPanel` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/panel/scaled_panel/scaled_panel.hpp:16` | `void add_at(Control::Ptr child, Rect design_bounds);` |
| `ScheduleDuringFrameCallback` | `shared_ptr` | `ReentrantFrameControl` | `tests/frame_scheduler_tests.cpp:105` | `const std::shared_ptr<ReentrantFrameControl>& admitted_next_;` |
| `SchedulePaintOffThread` | `qualified-alias:shared_ptr` | `Control` | `tests/frame_scheduler_tests.cpp:111` | `SchedulePaintOffThread(Window& window, const Control::Ptr& root,` |
| `SchedulePaintOffThread` | `qualified-alias:shared_ptr` | `Control` | `tests/frame_scheduler_tests.cpp:125` | `const Control::Ptr& root_;` |
| `ScheduledFrameRequest` | `qualified-alias:weak_ptr` | `Control` | `src/core/scheduler/request/scheduled_frame_request.hpp:19` | `Control::WeakPtr request_target,` |
| `ScheduledFrameRequest` | `qualified-alias:weak_ptr` | `Control` | `src/core/scheduler/request/scheduled_frame_request.hpp:30` | `Control::WeakPtr target;` |
| `ScheduledTickCallback` | `weak_ptr` | `CallbackState` | `include/gui_forms/timer/timer/timer.hpp:45` | `std::weak_ptr<CallbackState> state;` |
| `ScrollableControl` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/scrollable_control/scrollable_control.hpp:84` | `void scroll_control_into_view(const Control::Ptr& control);` |
| `ShowcaseContext` | `qualified-alias:shared_ptr` | `Control` | `src/controls/showcase_controls.cpp:63` | `std::vector<Control::Ptr> pages;` |
| `ShowcaseContext` | `shared_ptr` | `Timer` | `src/controls/showcase_controls.cpp:71` | `std::shared_ptr<Timer> ui_timer;` |
| `ShowcaseContext` | `shared_ptr` | `BindingSource` | `src/controls/showcase_controls.cpp:75` | `std::shared_ptr<BindingSource> binding_source;` |
| `ShowcaseTimerTick` | `weak_ptr` | `ShowcaseContext` | `src/controls/showcase_controls.cpp:3540` | `std::weak_ptr<ShowcaseContext> context_;` |
| `ShowcaseTree` | `qualified-alias:shared_ptr` | `Control` | `src/controls/showcase_controls.hpp:12` | `Control::Ptr root;` |
| `Slot` | `weak_ptr` | `State` | `include/gui_forms/event/event/event.hpp:154` | `std::weak_ptr<State> state;` |
| `Slot` | `qualified-alias:shared_ptr` | `Control` | `src/controls/panel/instrument_rack/instrument_rack.cpp:587` | `Control::Ptr action;` |
| `SourceChangedCallback` | `weak_ptr` | `Binding` | `include/gui_forms/binding/binding/binding.hpp:66` | `std::weak_ptr<Binding> binding;` |
| `SourceDisposedCallback` | `weak_ptr` | `Binding` | `include/gui_forms/binding/binding/binding.hpp:71` | `std::weak_ptr<Binding> binding;` |
| `SourceEntry` | `weak_ptr` | `BindingSource` | `include/gui_forms/binding/binding_context/binding_context.hpp:38` | `std::weak_ptr<BindingSource> source;` |
| `SplitContainer` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp:54` | `[[nodiscard]] Control::Ptr splitter_control() const noexcept {` |
| `SplitContainer` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp:148` | `Control::Ptr splitter_;` |
| `State` | `shared_ptr` | `Slot` | `include/gui_forms/event/event/event.hpp:162` | `std::vector<std::shared_ptr<Slot>> slots;` |
| `SubscriptionToken` | `shared_ptr` | `detail::Revocable` | `include/gui_forms/event/event/event.hpp:47` | `std::shared_ptr<detail::Revocable> revocable_;` |
| `SwitchPropertyGridSelection` | `qualified-alias:shared_ptr` | `Control` | `tests/inspection_controls_tests.cpp:317` | `Control::Ptr replacement) noexcept` |
| `SwitchPropertyGridSelection` | `qualified-alias:shared_ptr` | `Control` | `tests/inspection_controls_tests.cpp:326` | `Control::Ptr replacement_;` |
| `TabControl` | `shared_ptr` | `TabPage` | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:38` | `void add_page(std::shared_ptr<TabPage> page);` |
| `TabControl` | `shared_ptr` | `TabPage` | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:49` | `void set_selected_tab(const std::shared_ptr<TabPage>& page);` |
| `TabControl` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:78` | `std::unordered_map<std::uint64_t, Control::WeakPtr>;` |
| `TabControl` | `shared_ptr` | `TabPage` | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:80` | `const std::shared_ptr<TabPage>& page) const;` |
| `TabControl` | `shared_ptr` | `TabPage` | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:82` | `void remember_page_focus(const std::shared_ptr<TabPage>& page);` |
| `TabControl` | `shared_ptr` | `TabPage` | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:83` | `void restore_page_focus(const std::shared_ptr<TabPage>& page,` |
| `TabControl` | `weak_ptr` | `TabPage` | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:87` | `std::vector<std::weak_ptr<TabPage>> pages_;` |
| `TabControl` | `weak_ptr` | `TabPage` | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:88` | `std::weak_ptr<TabPage> selected_page_;` |
| `TableLayoutPanel` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.hpp:93` | `[[nodiscard]] Control::Ptr control_from_position(std::size_t column,` |
| `TargetChangedCallback` | `weak_ptr` | `Binding` | `include/gui_forms/binding/binding/binding.hpp:76` | `std::weak_ptr<Binding> binding;` |
| `TargetValidatingCallback` | `weak_ptr` | `Binding` | `include/gui_forms/binding/binding/binding.hpp:81` | `std::weak_ptr<Binding> binding;` |
| `TextStateUpdater` | `weak_ptr` | `TextBox` | `src/controls/showcase_controls.cpp:1563` | `std::weak_ptr<TextBox> field_;` |
| `TextStateUpdater` | `weak_ptr` | `Label` | `src/controls/showcase_controls.cpp:1564` | `std::weak_ptr<Label> state_;` |
| `Timer` | `weak_ptr` | `detail::WindowLifetime` | `include/gui_forms/timer/timer/timer.hpp:54` | `std::weak_ptr<detail::WindowLifetime> window_lifetime_;` |
| `Timer` | `shared_ptr` | `CallbackState` | `include/gui_forms/timer/timer/timer.hpp:56` | `std::shared_ptr<CallbackState> callback_state_;` |
| `TimerCommandClick` | `weak_ptr` | `ShowcaseContext` | `src/controls/showcase_controls.cpp:3589` | `std::weak_ptr<ShowcaseContext> context_;` |
| `TimerIntervalChanged` | `weak_ptr` | `ShowcaseContext` | `src/controls/showcase_controls.cpp:3559` | `std::weak_ptr<ShowcaseContext> context_;` |
| `TimerPostsDispatch` | `shared_ptr` | `DispatchPhaseProbe` | `tests/dispatcher_tests.cpp:390` | `const std::shared_ptr<DispatchPhaseProbe>& root_;` |
| `ToolTip` | `shared_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:35` | `void set_tool_tip(const std::shared_ptr<Control>& target, std::string text);` |
| `ToolTip` | `shared_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:63` | `void show(const std::shared_ptr<Control>& target);` |
| `ToolTip` | `shared_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:64` | `void show(const std::shared_ptr<Control>& target,` |
| `ToolTip` | `shared_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:92` | `void target_pointer(const std::shared_ptr<Control>& target,` |
| `ToolTip` | `shared_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:94` | `void target_focus(const std::shared_ptr<Control>& target, bool focused);` |
| `ToolTip` | `shared_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:95` | `void target_moved(const std::shared_ptr<Control>& target);` |
| `ToolTip` | `shared_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:96` | `void schedule_show(const std::shared_ptr<Control>& target,` |
| `ToolTip` | `shared_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:100` | `void show_now(const std::shared_ptr<Control>& target,` |
| `ToolTip` | `weak_ptr` | `detail::WindowLifetime` | `include/gui_forms/components/tool_tip/tool_tip.hpp:107` | `std::weak_ptr<detail::WindowLifetime> window_lifetime_;` |
| `ToolTip` | `unique_ptr` | `Timer` | `include/gui_forms/components/tool_tip/tool_tip.hpp:108` | `std::unique_ptr<Timer> timer_;` |
| `ToolTip` | `weak_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:110` | `std::weak_ptr<Control> pending_target_;` |
| `ToolTip` | `weak_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:111` | `std::weak_ptr<Control> visible_target_;` |
| `ToolTip` | `shared_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:112` | `std::shared_ptr<Control> overlay_layer_;` |
| `ToolTip` | `shared_ptr` | `Control` | `include/gui_forms/components/tool_tip/tool_tip.hpp:113` | `std::shared_ptr<Control> overlay_bubble_;` |
| `ToolTip` | `unique_ptr` | `PopupHolder` | `include/gui_forms/components/tool_tip/tool_tip.hpp:115` | `std::unique_ptr<PopupHolder> popup_;` |
| `ToolTip` | `weak_ptr` | `Control` | `src/controls/tool_tip/tool_tip.cpp:25` | `std::weak_ptr<Control> target;` |
| `ToolTip` | `weak_ptr` | `Control` | `src/controls/tool_tip/tool_tip.cpp:34` | `std::weak_ptr<Control> target;` |
| `ToolTip` | `weak_ptr` | `Control` | `src/controls/tool_tip/tool_tip.cpp:44` | `std::weak_ptr<Control> target;` |
| `ToolTip` | `weak_ptr` | `Control` | `src/controls/tool_tip/tool_tip.cpp:54` | `std::weak_ptr<Control> target;` |
| `ToolTip` | `weak_ptr` | `detail::WindowLifetime` | `src/controls/tool_tip/tool_tip.cpp:64` | `std::weak_ptr<detail::WindowLifetime> window_lifetime;` |
| `TreeFixture` | `unique_ptr` | `Window` | `tests/invalidation_damage_tests.cpp:342` | `std::unique_ptr<Window> window;` |
| `TreeView` | `shared_ptr` | `ImageList` | `include/gui_forms/controls/panel/tree_view/tree_view.hpp:70` | `void set_image_list(std::shared_ptr<ImageList> image_list);` |
| `TreeView` | `shared_ptr` | `ImageList` | `include/gui_forms/controls/panel/tree_view/tree_view.hpp:125` | `std::shared_ptr<ImageList> image_list_;` |
| `UserControlLoaded` | `weak_ptr` | `Label` | `src/controls/showcase_controls.cpp:631` | `std::weak_ptr<Label> status_;` |
| `WeakRangeValueMirror` | `weak_ptr` | `RangeControlType` | `src/controls/showcase_controls.cpp:238` | `std::weak_ptr<RangeControlType> target_;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:234` | `explicit Window(Control::Ptr root, Size client_size = {});` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:239` | `[[nodiscard]] Control::Ptr root() const noexcept { return root_; }` |
| `Window` | `shared_ptr` | `const Theme` | `include/gui_forms/window/window.hpp:256` | `void set_theme(std::shared_ptr<const Theme> theme);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:292` | `const Control::Ptr& control, std::shared_ptr<LiveSurface> surface);` |
| `Window` | `shared_ptr` | `LiveSurface` | `include/gui_forms/window/window.hpp:292` | `const Control::Ptr& control, std::shared_ptr<LiveSurface> surface);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:306` | `[[nodiscard]] FrameRequestToken schedule_paint(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:308` | `[[nodiscard]] FrameRequestToken activate_surface(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:323` | `const Control::Ptr& owner, std::function<void()> callback);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:327` | `void invoke(const Control::Ptr& owner, std::function<void()> callback);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:372` | `[[nodiscard]] Control::Ptr find(std::string_view stable_id) const;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:373` | `[[nodiscard]] Control::Ptr hit_test(Point position);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:374` | `bool request_focus(const Control::Ptr& control);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:375` | `[[nodiscard]] Control::Ptr focused_control() const noexcept { return focused_.lock(); }` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:376` | `bool validate_control(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:380` | `const Control::Ptr& container,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:384` | `const Control::Ptr& root,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:385` | `const Control::Ptr& preferred_focus = {},` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:391` | `[[nodiscard]] Control::Ptr active_focus_scope_root() const noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:393` | `void set_accept_button(const Control::Ptr& control);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:394` | `void set_cancel_button(const Control::Ptr& control);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:395` | `[[nodiscard]] Control::Ptr accept_button() const noexcept {` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:398` | `[[nodiscard]] Control::Ptr cancel_button() const noexcept {` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:412` | `void capture_pointer(const Control::Ptr& control, std::uint64_t pointer_id = 1);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:414` | `[[nodiscard]] Control::Ptr captured_control() const noexcept { return captured_.lock(); }` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:425` | `[[nodiscard]] PopupToken open_popup(const Control::Ptr& owner,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:426` | `const Control::Ptr& popup,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:434` | `[[nodiscard]] Control::Ptr pressed_control() const noexcept { return pressed_.lock(); }` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:472` | `using ControlList = std::vector<Control::Ptr>;` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:473` | `using StableIdMap = std::unordered_map<std::string, Control::WeakPtr>;` |
| `Window` | `shared_ptr` | `detail::PopupAttachment` | `include/gui_forms/window/window.hpp:475` | `std::vector<std::shared_ptr<detail::PopupAttachment>>;` |
| `Window` | `shared_ptr` | `detail::AcceleratorAttachment` | `include/gui_forms/window/window.hpp:477` | `std::vector<std::shared_ptr<detail::AcceleratorAttachment>>;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:510` | `void attach_subtree(const Control::Ptr& control, const Control::WeakPtr& parent);` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:510` | `void attach_subtree(const Control::Ptr& control, const Control::WeakPtr& parent);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:511` | `void attach_subtree_state(const Control::Ptr& control,` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:512` | `const Control::WeakPtr& parent,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:514` | `void detach_subtree(const Control::Ptr& control);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:515` | `void detach_subtree_state(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:517` | `void dispose_subtree(const Control::Ptr& control) noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:518` | `void revoke_interaction_for_subtree(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:520` | `void close_focus_scopes_for_subtree(const Control::Ptr& control);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:523` | `const Control::Ptr& notification_owner);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:524` | `void revoke_focus_scopes_for_subtree(const Control::Ptr& control) noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:525` | `void close_popups_for_subtree(const Control::Ptr& control) noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:531` | `const Control::Ptr& control) const noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:532` | `[[nodiscard]] std::vector<Control::Ptr> focus_candidates(` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:533` | `const Control::Ptr& scope_root) const;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:534` | `static bool tab_order_less(const Control::Ptr& left,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:535` | `const Control::Ptr& right) noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:536` | `void collect_focus_candidates(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:539` | `void collect_mnemonic_candidates(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:540` | `const Control::Ptr& root,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:543` | `void validate_children_recursive(const Control::Ptr& parent,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:544` | `const Control::Ptr& container,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:548` | `const Control::Ptr& previous, const Control::Ptr& destination,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:548` | `const Control::Ptr& previous, const Control::Ptr& destination,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:550` | `void change_pointer_capture(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:553` | `void on_eligibility_changed(const Control::Ptr& control);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:554` | `void on_hit_test_transparency_changed(const Control::Ptr& control);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:556` | `void register_subtree(const Control::Ptr& control);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:558` | `const Control::Ptr& control, ControlList& controls,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:560` | `void unregister_subtree(const Control::Ptr& control);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:587` | `void add_subtree_damage(const Control::Ptr& control);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:591` | `[[nodiscard]] std::vector<Control::Ptr> route_to(const Control::Ptr& target) const;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:591` | `[[nodiscard]] std::vector<Control::Ptr> route_to(const Control::Ptr& target) const;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:592` | `[[nodiscard]] Control::Ptr drop_target_at(Point position);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:593` | `[[nodiscard]] DragDispatchResult route_drag(const Control::Ptr& target,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:595` | `[[nodiscard]] Control::Ptr hit_test_recursive(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:595` | `[[nodiscard]] Control::Ptr hit_test_recursive(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:597` | `void paint_recursive(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:607` | `void measure_dirty_recursive(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:611` | `void arrange_dirty_recursive(const Control::Ptr& control,` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:616` | `const Control::Ptr& control) const noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:620` | `void commit_layout_requests_recursive(const Control::Ptr& control) noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:621` | `[[nodiscard]] Dirty recompute_subtree_dirty(const Control::Ptr& control) noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:622` | `void clear_layout_dirty_subtree(const Control::Ptr& control) noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:623` | `void clear_paint_dirty_subtree(const Control::Ptr& control) noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:625` | `const Control::Ptr& control) const noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:634` | `[[nodiscard]] bool eligible(const Control::Ptr& control) const noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:635` | `[[nodiscard]] bool move_focus_after(const Control::Ptr& origin);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:637` | `const Control::Ptr& destination);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:642` | `const Control::Ptr& target, DragEvent event);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:645` | `void clear_dialog_targets_for_subtree(const Control::Ptr& control) noexcept;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:648` | `Control::Ptr root_;` |
| `Window` | `shared_ptr` | `const Theme` | `include/gui_forms/window/window.hpp:653` | `std::shared_ptr<const Theme> theme_;` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:658` | `Control::WeakPtr focused_;` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:659` | `Control::WeakPtr accept_button_;` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:660` | `Control::WeakPtr cancel_button_;` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:671` | `Control::WeakPtr mnemonic_cursor_;` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:695` | `Control::WeakPtr captured_;` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:701` | `Control::WeakPtr pressed_;` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:702` | `Control::WeakPtr hovered_;` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | `include/gui_forms/window/window.hpp:703` | `Control::WeakPtr drag_target_;` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:723` | `const Control::Ptr& control, PaintControlCheckpointList& checkpoints);` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | `include/gui_forms/window/window.hpp:732` | `const Control::Ptr& control, std::vector<Rect>& rectangles) const;` |
| `Window` | `shared_ptr` | `detail::ScheduledFrameRequest` | `include/gui_forms/window/window.hpp:740` | `std::vector<std::shared_ptr<detail::ScheduledFrameRequest>>;` |
| `Window` | `shared_ptr` | `detail::WindowLifetime` | `include/gui_forms/window/window.hpp:743` | `std::shared_ptr<detail::WindowLifetime> lifetime_;` |
| `Window` | `shared_ptr` | `detail::DispatcherState` | `include/gui_forms/window/window.hpp:745` | `std::shared_ptr<detail::DispatcherState> dispatcher_state_;` |
| `WindowFixture` | `unique_ptr` | `Window` | `tests/retained_lifetime_tests.cpp:108` | `std::unique_ptr<Window> window;` |
| `WindowsApplicationWindow` | `unique_ptr` | `Window` | `include/gui_forms/platform/windows_host.hpp:46` | `std::unique_ptr<Window> model;` |
| `WindowsCompatibilityPaintEndpoint` | `unique_ptr` | `Implementation` | `include/gui_forms/platform/windows_compatibility_paint_endpoint/windows_compatibility_paint_endpoint.hpp:51` | `std::unique_ptr<Implementation> implementation) noexcept;` |
| `WindowsCompatibilityPaintEndpoint` | `unique_ptr` | `Implementation` | `include/gui_forms/platform/windows_compatibility_paint_endpoint/windows_compatibility_paint_endpoint.hpp:53` | `std::unique_ptr<Implementation> implementation_;` |
| `WindowsCompatibilityPaintEndpoint` | `shared_ptr` | `LiveSurface` | `src/host/windows/paint_endpoint/windows_compatibility_paint_endpoint.cpp:453` | `std::shared_ptr<LiveSurface> surface;` |
| `WindowsDispatchPendingCallback` | `shared_ptr` | `ControlRecord` | `src/abi/registry/registry.hpp:2555` | `std::shared_ptr<ControlRecord> record;` |
| `WindowsDispatchPendingCallback` | `shared_ptr` | `std::unordered_map< std::string, std::shared_ptr<Control>>` | `src/abi/registry/registry.hpp:2556` | `std::shared_ptr<std::unordered_map<` |
| `WindowsDispatchPendingCallback` | `shared_ptr` | `Control` | `src/abi/registry/registry.hpp:2557` | `std::string, std::shared_ptr<Control>>> automation_controls;` |
| `WindowsHostState` | `unique_ptr` | `Window` | `src/host/windows/application/windows_host.cpp:2670` | `std::unique_ptr<Window> model_;` |

## All detected smart-pointer and alias evidence

| Owner/scope | Kind | Target | Scope | Location |
|---|---|---|---|---|
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `demo/gallery.cpp:12` |
| `<file/function>` | `unique_ptr` | `gui_forms::Window` | local/signature/use | `demo/gallery.cpp:15` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `demo/gallery.cpp:25` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `demo/gallery.hpp:11` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `demo/showcase.cpp:148` |
| `<file/function>` | `unique_ptr` | `gui_forms::Window` | local/signature/use | `demo/showcase.cpp:150` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `demo/showcase.cpp:153` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `demo/showcase.cpp:156` |
| `<file/function>` | `shared_ptr` | `gui_forms::PictureBox` | local/signature/use | `demo/showcase.cpp:165` |
| `<file/function>` | `shared_ptr` | `gui_forms::PictureBox` | local/signature/use | `demo/showcase.cpp:172` |
| `<file/function>` | `shared_ptr` | `gui_forms::ImageList` | local/signature/use | `demo/showcase.cpp:189` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `demo/showcase.cpp:195` |
| `<file/function>` | `shared_ptr` | `gui_forms::MaterialPanel` | local/signature/use | `demo/showcase.cpp:208` |
| `<file/function>` | `shared_ptr` | `gui_forms::MaterialPanel` | local/signature/use | `demo/showcase.cpp:226` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `demo/showcase.hpp:11` |
| `WaterfallSurface` | `shared_ptr` | `WaterfallSurface` | local/signature/use | `demo/vsync_lab.cpp:121` |
| `PaintRequest` | `weak_ptr` | `WaterfallSurface` | class declaration | `demo/vsync_lab.cpp:149` |
| `PaintRequest` | `shared_ptr` | `WaterfallSurface` | local/signature/use | `demo/vsync_lab.cpp:152` |
| `SurfaceWakeRequest` | `weak_ptr` | `WaterfallSurface` | class declaration | `demo/vsync_lab.cpp:166` |
| `SurfaceWakeRequest` | `shared_ptr` | `WaterfallSurface` | local/signature/use | `demo/vsync_lab.cpp:169` |
| `WaterfallSurface` | `shared_ptr` | `WaterfallSurface` | local/signature/use | `demo/vsync_lab.cpp:338` |
| `WaterfallSurface` | `weak_ptr` | `WaterfallSurface` | local/signature/use | `demo/vsync_lab.cpp:345` |
| `WaterfallSurface` | `weak_ptr` | `WaterfallSurface` | local/signature/use | `demo/vsync_lab.cpp:356` |
| `WaterfallSurface` | `shared_ptr` | `LiveSurface` | class declaration | `demo/vsync_lab.cpp:374` |
| `LabContext` | `shared_ptr` | `WaterfallSurface` | class declaration | `demo/vsync_lab.cpp:394` |
| `LabContext` | `shared_ptr` | `Label` | class declaration | `demo/vsync_lab.cpp:395` |
| `LabContext` | `shared_ptr` | `Label` | class declaration | `demo/vsync_lab.cpp:396` |
| `LabContext` | `shared_ptr` | `Label` | class declaration | `demo/vsync_lab.cpp:397` |
| `LabContext` | `shared_ptr` | `Label` | class declaration | `demo/vsync_lab.cpp:398` |
| `LabContext` | `shared_ptr` | `Label` | class declaration | `demo/vsync_lab.cpp:399` |
| `LabContext` | `shared_ptr` | `Label` | class declaration | `demo/vsync_lab.cpp:400` |
| `LabContext` | `shared_ptr` | `Label` | class declaration | `demo/vsync_lab.cpp:401` |
| `LabContext` | `shared_ptr` | `Button` | class declaration | `demo/vsync_lab.cpp:402` |
| `LabContext` | `unique_ptr` | `Timer` | class declaration | `demo/vsync_lab.cpp:404` |
| `<file/function>` | `shared_ptr` | `LabContext` | local/signature/use | `demo/vsync_lab.cpp:411` |
| `LabValueChanged` | `weak_ptr` | `LabContext` | class declaration | `demo/vsync_lab.cpp:416` |
| `LabValueChanged` | `shared_ptr` | `LabContext` | local/signature/use | `demo/vsync_lab.cpp:420` |
| `PauseClicked` | `weak_ptr` | `LabContext` | class declaration | `demo/vsync_lab.cpp:442` |
| `PauseClicked` | `shared_ptr` | `LabContext` | local/signature/use | `demo/vsync_lab.cpp:445` |
| `ResponseClicked` | `weak_ptr` | `LabContext` | class declaration | `demo/vsync_lab.cpp:454` |
| `ResponseClicked` | `shared_ptr` | `LabContext` | local/signature/use | `demo/vsync_lab.cpp:457` |
| `FullRepaintChanged` | `weak_ptr` | `LabContext` | class declaration | `demo/vsync_lab.cpp:467` |
| `FullRepaintChanged` | `shared_ptr` | `LabContext` | local/signature/use | `demo/vsync_lab.cpp:470` |
| `TelemetryTick` | `weak_ptr` | `LabContext` | class declaration | `demo/vsync_lab.cpp:481` |
| `TelemetryTick` | `shared_ptr` | `LabContext` | local/signature/use | `demo/vsync_lab.cpp:484` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `demo/vsync_lab.cpp:489` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `demo/vsync_lab.cpp:491` |
| `<file/function>` | `shared_ptr` | `TrackBar` | local/signature/use | `demo/vsync_lab.cpp:498` |
| `<file/function>` | `shared_ptr` | `gui_forms::TrackBar` | local/signature/use | `demo/vsync_lab.cpp:500` |
| `<file/function>` | `shared_ptr` | `LabContext` | local/signature/use | `demo/vsync_lab.cpp:509` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `demo/vsync_lab.cpp:544` |
| `<file/function>` | `shared_ptr` | `LabContext` | local/signature/use | `demo/vsync_lab.cpp:545` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `demo/vsync_lab.cpp:546` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `demo/vsync_lab.cpp:550` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `demo/vsync_lab.cpp:554` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `demo/vsync_lab.cpp:557` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `demo/vsync_lab.cpp:565` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `demo/vsync_lab.cpp:569` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `demo/vsync_lab.cpp:575` |
| `<file/function>` | `shared_ptr` | `TrackBar` | local/signature/use | `demo/vsync_lab.cpp:584` |
| `<file/function>` | `shared_ptr` | `TrackBar` | local/signature/use | `demo/vsync_lab.cpp:593` |
| `<file/function>` | `shared_ptr` | `TrackBar` | local/signature/use | `demo/vsync_lab.cpp:602` |
| `<file/function>` | `shared_ptr` | `TrackBar` | local/signature/use | `demo/vsync_lab.cpp:611` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `demo/vsync_lab.cpp:623` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `demo/vsync_lab.cpp:632` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `demo/vsync_lab.cpp:637` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `demo/vsync_lab.cpp:658` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `demo/vsync_lab.cpp:677` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `demo/vsync_lab.cpp:684` |
| `<file/function>` | `unique_ptr` | `gui_forms::Window` | local/signature/use | `demo/vsync_lab.cpp:717` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `demo/vsync_lab.hpp:13` |
| `Binding` | `shared_ptr` | `BindingSource` | class declaration | `include/gui_forms/binding/binding/binding.hpp:21` |
| `Binding` | `shared_ptr` | `BindingSource` | local/signature/use | `include/gui_forms/binding/binding/binding.hpp:26` |
| `SourceChangedCallback` | `weak_ptr` | `Binding` | class declaration | `include/gui_forms/binding/binding/binding.hpp:66` |
| `SourceDisposedCallback` | `weak_ptr` | `Binding` | class declaration | `include/gui_forms/binding/binding/binding.hpp:71` |
| `TargetChangedCallback` | `weak_ptr` | `Binding` | class declaration | `include/gui_forms/binding/binding/binding.hpp:76` |
| `TargetValidatingCallback` | `weak_ptr` | `Binding` | class declaration | `include/gui_forms/binding/binding/binding.hpp:81` |
| `Binding` | `weak_ptr` | `BindingSource` | class declaration | `include/gui_forms/binding/binding/binding.hpp:86` |
| `BindingContext` | `shared_ptr` | `BindingSource` | class declaration | `include/gui_forms/binding/binding_context/binding_context.hpp:22` |
| `BindingContext` | `shared_ptr` | `BindingSource` | class declaration | `include/gui_forms/binding/binding_context/binding_context.hpp:23` |
| `SourceEntry` | `weak_ptr` | `BindingSource` | class declaration | `include/gui_forms/binding/binding_context/binding_context.hpp:38` |
| `BindingContext` | `weak_ptr` | `detail::WindowLifetime` | class declaration | `include/gui_forms/binding/binding_context/binding_context.hpp:51` |
| `BindingSource` | `shared_ptr` | `Binding` | local/signature/use | `include/gui_forms/binding/binding_source/binding_source.hpp:48` |
| `BindingSource` | `weak_ptr` | `Binding` | class declaration | `include/gui_forms/binding/binding_source/binding_source.hpp:128` |
| `BindingSource` | `shared_ptr` | `Binding` | class declaration | `include/gui_forms/binding/binding_source/binding_source.hpp:134` |
| `BindingSource` | `weak_ptr` | `detail::WindowLifetime` | class declaration | `include/gui_forms/binding/binding_source/binding_source.hpp:140` |
| `BindingSource` | `unique_ptr` | `CurrencyManager` | class declaration | `include/gui_forms/binding/binding_source/binding_source.hpp:142` |
| `ControlBindingsCollection` | `shared_ptr` | `Binding` | local/signature/use | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:24` |
| `ControlBindingsCollection` | `shared_ptr` | `BindingSource` | class declaration | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:25` |
| `ControlBindingsCollection` | `shared_ptr` | `Binding` | local/signature/use | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:27` |
| `ControlBindingsCollection` | `shared_ptr` | `BindingSource` | class declaration | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:28` |
| `ControlBindingsCollection` | `shared_ptr` | `Binding` | class declaration | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:30` |
| `ControlBindingsCollection` | `shared_ptr` | `Binding` | local/signature/use | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:33` |
| `ControlBindingsCollection` | `shared_ptr` | `Binding` | local/signature/use | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:35` |
| `ControlBindingsCollection` | `shared_ptr` | `Binding` | class declaration | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:50` |
| `PropertyObjectMember` | `shared_ptr` | `const PropertyEnumDescriptor` | class declaration | `include/gui_forms/binding/value/binding_value.hpp:57` |
| `PropertyDescriptor` | `shared_ptr` | `const PropertyEnumDescriptor` | class declaration | `include/gui_forms/binding/value/binding_value.hpp:212` |
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
| `ErrorProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:80` |
| `ErrorProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:81` |
| `ErrorProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:86` |
| `ErrorProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:89` |
| `ErrorProvider` | `shared_ptr` | `Control` | local/signature/use | `include/gui_forms/components/error_provider/error_provider.hpp:111` |
| `ErrorProvider` | `shared_ptr` | `BindingSource` | local/signature/use | `include/gui_forms/components/error_provider/error_provider.hpp:113` |
| `ErrorProvider` | `shared_ptr` | `BindingSource` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:116` |
| `ErrorProvider` | `shared_ptr` | `BindingSource` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:121` |
| `ErrorProvider` | `unique_ptr` | `Entry` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:146` |
| `ErrorProvider` | `weak_ptr` | `Control` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:148` |
| `ErrorProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:154` |
| `ErrorProvider` | `weak_ptr` | `detail::WindowLifetime` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:172` |
| `ErrorProvider` | `unique_ptr` | `ToolTip` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:174` |
| `ErrorProvider` | `weak_ptr` | `BindingSource` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:186` |
| `ErrorProvider` | `weak_ptr` | `Control` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:188` |
| `HelpProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:65` |
| `HelpProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:66` |
| `HelpProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:68` |
| `HelpProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:71` |
| `HelpProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:74` |
| `HelpProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:86` |
| `HelpProvider` | `unique_ptr` | `Entry` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:106` |
| `HelpProvider` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:112` |
| `HelpProvider` | `weak_ptr` | `detail::WindowLifetime` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:118` |
| `HelpProvider` | `unique_ptr` | `AcceleratorHolder` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:120` |
| `ToolTip` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:35` |
| `ToolTip` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:63` |
| `ToolTip` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:64` |
| `ToolTip` | `shared_ptr` | `Control` | local/signature/use | `include/gui_forms/components/tool_tip/tool_tip.hpp:68` |
| `ToolTip` | `unique_ptr` | `Entry` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:85` |
| `ToolTip` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:92` |
| `ToolTip` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:94` |
| `ToolTip` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:95` |
| `ToolTip` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:96` |
| `ToolTip` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:100` |
| `ToolTip` | `weak_ptr` | `detail::WindowLifetime` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:107` |
| `ToolTip` | `unique_ptr` | `Timer` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:108` |
| `ToolTip` | `weak_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:110` |
| `ToolTip` | `weak_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:111` |
| `ToolTip` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:112` |
| `ToolTip` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:113` |
| `ToolTip` | `unique_ptr` | `PopupHolder` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:115` |
| `Control` | `alias:shared_ptr` | `Control` | alias definition | `include/gui_forms/control/control/control.hpp:313` |
| `Control` | `shared_ptr` | `Control` | class declaration | `include/gui_forms/control/control/control.hpp:313` |
| `Control` | `alias:weak_ptr` | `Control` | alias definition | `include/gui_forms/control/control/control.hpp:314` |
| `Control` | `weak_ptr` | `Control` | class declaration | `include/gui_forms/control/control/control.hpp:314` |
| `Control` | `shared_ptr` | `const Theme` | local/signature/use | `include/gui_forms/control/control/control.hpp:399` |
| `Control` | `shared_ptr` | `const Theme` | class declaration | `include/gui_forms/control/control/control.hpp:402` |
| `Control` | `shared_ptr` | `const PropertyEnumDescriptor` | local/signature/use | `include/gui_forms/control/control/control.hpp:701` |
| `Control` | `shared_ptr` | `const PropertyEnumDescriptor` | local/signature/use | `include/gui_forms/control/control/control.hpp:703` |
| `Control` | `shared_ptr` | `const PropertyEnumDescriptor` | local/signature/use | `include/gui_forms/control/control/control.hpp:705` |
| `Control` | `shared_ptr` | `const PropertyEnumDescriptor` | class declaration | `include/gui_forms/control/control/control.hpp:708` |
| `Control` | `shared_ptr` | `detail::DispatcherState` | class declaration | `include/gui_forms/control/control/control.hpp:732` |
| `Control` | `shared_ptr` | `const detail::DisplayChunk` | class declaration | `include/gui_forms/control/control/control.hpp:756` |
| `Control` | `shared_ptr` | `const Theme` | class declaration | `include/gui_forms/control/control/control.hpp:771` |
| `Control` | `unique_ptr` | `ControlBindingsCollection` | class declaration | `include/gui_forms/control/control/control.hpp:792` |
| `<file/function>` | `shared_ptr` | `ControlType` | local/signature/use | `include/gui_forms/control/control/control.hpp:1014` |
| `<file/function>` | `shared_ptr` | `ControlType` | local/signature/use | `include/gui_forms/control/control/control.hpp:1017` |
| `ControlFactory` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/control/static_tree/control_factory/control_factory.hpp:23` |
| `ControlFactory` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/control/static_tree/control_factory/control_factory.hpp:26` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `include/gui_forms/control/static_tree/control_factory/control_factory.hpp:33` |
| `ButtonBase` | `shared_ptr` | `ImageList` | local/signature/use | `include/gui_forms/controls/button_base/button_base.hpp:59` |
| `ButtonBase` | `shared_ptr` | `ImageList` | class declaration | `include/gui_forms/controls/button_base/button_base.hpp:62` |
| `ButtonBase` | `shared_ptr` | `ImageList` | class declaration | `include/gui_forms/controls/button_base/button_base.hpp:153` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:57` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:58` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:59` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:59` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:60` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:60` |
| `MasterDetailView` | `shared_ptr` | `SplitContainer` | local/signature/use | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:61` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:96` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:96` |
| `MasterDetailView` | `shared_ptr` | `SplitterPanel` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:97` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:98` |
| `MasterDetailView` | `shared_ptr` | `SplitContainer` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:104` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:105` |
| `MasterDetailView` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/container/master_detail_view/master_detail_view.hpp:106` |
| `MenuStrip` | `unique_ptr` | `ContextMenu` | class declaration | `include/gui_forms/controls/menu_strip/menu_strip.hpp:95` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:24` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:27` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:28` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:29` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:30` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:57` |
| `AnchoredPopupLayer` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:59` |
| `AnchoredPopupLayer` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:60` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:41` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:42` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:43` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:44` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:44` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:45` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:45` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:46` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:46` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:85` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:85` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:85` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:88` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:89` |
| `Card` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/card/card.hpp:90` |
| `ReviewCard` | `shared_ptr` | `Label` | local/signature/use | `include/gui_forms/controls/panel/card/review_card/review_card.hpp:53` |
| `ReviewCard` | `shared_ptr` | `Label` | local/signature/use | `include/gui_forms/controls/panel/card/review_card/review_card.hpp:56` |
| `ReviewCard` | `shared_ptr` | `Label` | local/signature/use | `include/gui_forms/controls/panel/card/review_card/review_card.hpp:59` |
| `ReviewCard` | `shared_ptr` | `Label` | class declaration | `include/gui_forms/controls/panel/card/review_card/review_card.hpp:72` |
| `ReviewCard` | `shared_ptr` | `Label` | class declaration | `include/gui_forms/controls/panel/card/review_card/review_card.hpp:73` |
| `ReviewCard` | `shared_ptr` | `Label` | class declaration | `include/gui_forms/controls/panel/card/review_card/review_card.hpp:74` |
| `ColorValueEditor` | `shared_ptr` | `TextBox` | local/signature/use | `include/gui_forms/controls/panel/color_value_editor/color_value_editor.hpp:20` |
| `ColorValueEditor` | `shared_ptr` | `TextBox` | class declaration | `include/gui_forms/controls/panel/color_value_editor/color_value_editor.hpp:49` |
| `ComboBox` | `shared_ptr` | `Panel` | class declaration | `include/gui_forms/controls/panel/combo_box/combo_box.hpp:94` |
| `ComboBox` | `shared_ptr` | `ListBox` | class declaration | `include/gui_forms/controls/panel/combo_box/combo_box.hpp:95` |
| `DateTimePicker` | `shared_ptr` | `Panel` | class declaration | `include/gui_forms/controls/panel/date_time_picker/date_time_picker.hpp:116` |
| `DateTimePicker` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/date_time_picker/date_time_picker.hpp:117` |
| `FlagsValueEditor` | `shared_ptr` | `Panel` | class declaration | `include/gui_forms/controls/panel/flags_value_editor/flags_value_editor.hpp:55` |
| `FlagsValueEditor` | `shared_ptr` | `CheckedListBox` | class declaration | `include/gui_forms/controls/panel/flags_value_editor/flags_value_editor.hpp:56` |
| `ScaledGroupBox` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/group_box/scaled_group_box/scaled_group_box.hpp:18` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/instrument_rack/instrument_rack.hpp:21` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/instrument_rack/instrument_rack.hpp:37` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/instrument_rack/instrument_rack.hpp:39` |
| `InstrumentRack` | `unique_ptr` | `Impl` | class declaration | `include/gui_forms/controls/panel/instrument_rack/instrument_rack.hpp:79` |
| `NumericUpDown` | `shared_ptr` | `TextBox` | local/signature/use | `include/gui_forms/controls/panel/numeric_up_down/numeric_up_down.hpp:33` |
| `EditorChangeCallback` | `weak_ptr` | `NumericUpDown` | class declaration | `include/gui_forms/controls/panel/numeric_up_down/numeric_up_down.hpp:45` |
| `SpinnerStepCallback` | `weak_ptr` | `NumericUpDown` | class declaration | `include/gui_forms/controls/panel/numeric_up_down/numeric_up_down.hpp:49` |
| `NumericUpDown` | `shared_ptr` | `TextBox` | class declaration | `include/gui_forms/controls/panel/numeric_up_down/numeric_up_down.hpp:58` |
| `NumericUpDown` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/numeric_up_down/numeric_up_down.hpp:59` |
| `ObjectView` | `shared_ptr` | `ImageList` | local/signature/use | `include/gui_forms/controls/panel/object_view/object_view.hpp:106` |
| `ObjectView` | `shared_ptr` | `ImageList` | class declaration | `include/gui_forms/controls/panel/object_view/object_view.hpp:109` |
| `ObjectView` | `shared_ptr` | `ImageList` | class declaration | `include/gui_forms/controls/panel/object_view/object_view.hpp:182` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:21` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:22` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:23` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:24` |
| `PropertyGrid` | `shared_ptr` | `PropertyList` | local/signature/use | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:29` |
| `PropertyGrid` | `shared_ptr` | `PropertyValueConverterRegistry` | local/signature/use | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:30` |
| `PropertyGrid` | `shared_ptr` | `PropertyValueConverterRegistry` | class declaration | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:33` |
| `PropertyGrid` | `shared_ptr` | `PropertyEditorRegistry` | local/signature/use | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:34` |
| `PropertyGrid` | `shared_ptr` | `PropertyEditorRegistry` | class declaration | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:36` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:37` |
| `PropertyGrid` | `shared_ptr` | `Button` | local/signature/use | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:38` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:62` |
| `PropertyGrid` | `unique_ptr` | `Impl` | class declaration | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:80` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_grid/property_grid.hpp:81` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_list/property_list.hpp:30` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_list/property_list.hpp:31` |
| `PropertyList` | `shared_ptr` | `Button` | local/signature/use | `include/gui_forms/controls/panel/property_list/property_list.hpp:32` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_list/property_list.hpp:34` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/property_list/property_list.hpp:35` |
| `PropertyList` | `unique_ptr` | `Impl` | class declaration | `include/gui_forms/controls/panel/property_list/property_list.hpp:76` |
| `ScaledPanel` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/panel/scaled_panel/scaled_panel.hpp:16` |
| `TreeView` | `shared_ptr` | `ImageList` | local/signature/use | `include/gui_forms/controls/panel/tree_view/tree_view.hpp:67` |
| `TreeView` | `shared_ptr` | `ImageList` | class declaration | `include/gui_forms/controls/panel/tree_view/tree_view.hpp:70` |
| `TreeView` | `shared_ptr` | `ImageList` | class declaration | `include/gui_forms/controls/panel/tree_view/tree_view.hpp:125` |
| `RasterCanvas` | `shared_ptr` | `gui_drawing::Bitmap` | local/signature/use | `include/gui_forms/controls/raster_canvas/raster_canvas.hpp:19` |
| `RasterCanvas` | `shared_ptr` | `gui_drawing::Bitmap` | class declaration | `include/gui_forms/controls/raster_canvas/raster_canvas.hpp:22` |
| `RasterCanvas` | `shared_ptr` | `gui_drawing::Bitmap` | class declaration | `include/gui_forms/controls/raster_canvas/raster_canvas.hpp:74` |
| `ContainerControl` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/container_control.hpp:11` |
| `ContainerControl` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/container_control.hpp:12` |
| `ContainerControl` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/container_control.hpp:13` |
| `SplitContainer` | `shared_ptr` | `SplitterPanel` | local/signature/use | `include/gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp:48` |
| `SplitContainer` | `shared_ptr` | `SplitterPanel` | local/signature/use | `include/gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp:51` |
| `SplitContainer` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp:54` |
| `SplitContainer` | `shared_ptr` | `SplitterPanel` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp:143` |
| `SplitContainer` | `shared_ptr` | `SplitterPanel` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp:146` |
| `SplitContainer` | `shared_ptr` | `SplitterPanel` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp:147` |
| `SplitContainer` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/split_container/split_container.hpp:148` |
| `TabControl` | `shared_ptr` | `TabPage` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:38` |
| `TabControl` | `shared_ptr` | `TabPage` | local/signature/use | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:39` |
| `TabControl` | `shared_ptr` | `TabPage` | local/signature/use | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:40` |
| `TabControl` | `shared_ptr` | `TabPage` | local/signature/use | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:42` |
| `TabControl` | `shared_ptr` | `TabPage` | local/signature/use | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:45` |
| `TabControl` | `shared_ptr` | `TabPage` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:49` |
| `TabControl` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:78` |
| `TabControl` | `shared_ptr` | `TabPage` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:80` |
| `TabControl` | `shared_ptr` | `TabPage` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:82` |
| `TabControl` | `shared_ptr` | `TabPage` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:83` |
| `TabControl` | `weak_ptr` | `TabPage` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:87` |
| `TabControl` | `weak_ptr` | `TabPage` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:88` |
| `TableLayoutPanel` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.hpp:93` |
| `ScrollableControl` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/controls/scrollable_control/scrollable_control.hpp:84` |
| `WeakMemberCallback` | `weak_ptr` | `Object` | local/signature/use | `include/gui_forms/detail/weak_member_callback.hpp:19` |
| `WeakMemberCallback` | `shared_ptr` | `Object` | local/signature/use | `include/gui_forms/detail/weak_member_callback.hpp:23` |
| `WeakMemberCallback` | `weak_ptr` | `Object` | class declaration | `include/gui_forms/detail/weak_member_callback.hpp:30` |
| `WeakMemberCallback` | `weak_ptr` | `Object` | local/signature/use | `include/gui_forms/detail/weak_member_callback.hpp:39` |
| `WeakMemberCallback` | `shared_ptr` | `Object` | local/signature/use | `include/gui_forms/detail/weak_member_callback.hpp:43` |
| `WeakMemberCallback` | `weak_ptr` | `Object` | class declaration | `include/gui_forms/detail/weak_member_callback.hpp:50` |
| `DispatchOperation` | `shared_ptr` | `detail::DispatchWork` | local/signature/use | `include/gui_forms/dispatcher/operation/dispatch_operation.hpp:33` |
| `DispatchOperation` | `shared_ptr` | `detail::DispatchWork` | class declaration | `include/gui_forms/dispatcher/operation/dispatch_operation.hpp:46` |
| `Bitmap` | `unique_ptr` | `Bitmap` | local/signature/use | `include/gui_forms/drawing/bitmap/bitmap.hpp:66` |
| `Bitmap` | `unique_ptr` | `Bitmap` | local/signature/use | `include/gui_forms/drawing/bitmap/bitmap.hpp:67` |
| `Bitmap` | `unique_ptr` | `Bitmap` | local/signature/use | `include/gui_forms/drawing/bitmap/bitmap.hpp:69` |
| `Bitmap` | `shared_ptr` | `PixelStorage` | class declaration | `include/gui_forms/drawing/bitmap/bitmap.hpp:98` |
| `GraphicsPath` | `unique_ptr` | `GraphicsPath` | local/signature/use | `include/gui_forms/drawing/graphics_path/graphics_path.hpp:63` |
| `ImageAttributes` | `unique_ptr` | `ImageAttributes` | local/signature/use | `include/gui_forms/drawing/image_attributes/image_attributes.hpp:34` |
| `ImageSnapshot` | `shared_ptr` | `const PixelStorage` | class declaration | `include/gui_forms/drawing/image_snapshot/image_snapshot.hpp:27` |
| `TextureBrush` | `unique_ptr` | `TextureBrush` | local/signature/use | `include/gui_forms/drawing/texture_brush/texture_brush.hpp:24` |
| `SubscriptionToken` | `shared_ptr` | `detail::Revocable` | local/signature/use | `include/gui_forms/event/event/event.hpp:44` |
| `SubscriptionToken` | `shared_ptr` | `detail::Revocable` | class declaration | `include/gui_forms/event/event/event.hpp:47` |
| `Event` | `shared_ptr` | `Slot` | local/signature/use | `include/gui_forms/event/event/event.hpp:88` |
| `Event` | `shared_ptr` | `Slot` | local/signature/use | `include/gui_forms/event/event/event.hpp:89` |
| `Event` | `shared_ptr` | `Slot` | local/signature/use | `include/gui_forms/event/event/event.hpp:100` |
| `Slot` | `weak_ptr` | `State` | local/signature/use | `include/gui_forms/event/event/event.hpp:114` |
| `Slot` | `weak_ptr` | `State` | local/signature/use | `include/gui_forms/event/event/event.hpp:118` |
| `Slot` | `shared_ptr` | `State` | local/signature/use | `include/gui_forms/event/event/event.hpp:129` |
| `Slot` | `weak_ptr` | `State` | class declaration | `include/gui_forms/event/event/event.hpp:154` |
| `State` | `shared_ptr` | `Slot` | class declaration | `include/gui_forms/event/event/event.hpp:162` |
| `Event` | `shared_ptr` | `Slot` | local/signature/use | `include/gui_forms/event/event/event.hpp:170` |
| `Event` | `shared_ptr` | `Slot` | local/signature/use | `include/gui_forms/event/event/event.hpp:181` |
| `Event` | `shared_ptr` | `Slot` | local/signature/use | `include/gui_forms/event/event/event.hpp:187` |
| `SlotDisconnected` | `shared_ptr` | `Slot` | local/signature/use | `include/gui_forms/event/event/event.hpp:197` |
| `Event` | `shared_ptr` | `Slot` | local/signature/use | `include/gui_forms/event/event/event.hpp:203` |
| `Event` | `shared_ptr` | `State` | class declaration | `include/gui_forms/event/event/event.hpp:209` |
| `ImageList` | `weak_ptr` | `detail::WindowLifetime` | class declaration | `include/gui_forms/image_list/image_list/image_list.hpp:147` |
| `PropertyEditorBinding` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/inspection/property_editor_registry/property_editor_registry.hpp:15` |
| `PropertyEditorRegistry` | `shared_ptr` | `PropertyEditorRegistry` | local/signature/use | `include/gui_forms/inspection/property_editor_registry/property_editor_registry.hpp:37` |
| `PropertyValueConverterRegistry` | `shared_ptr` | `PropertyValueConverterRegistry` | local/signature/use | `include/gui_forms/inspection/property_value_converter_registry/property_value_converter_registry.hpp:69` |
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
| `ScheduledTickCallback` | `weak_ptr` | `CallbackState` | class declaration | `include/gui_forms/timer/timer/timer.hpp:45` |
| `Timer` | `weak_ptr` | `detail::WindowLifetime` | class declaration | `include/gui_forms/timer/timer/timer.hpp:54` |
| `Timer` | `shared_ptr` | `CallbackState` | class declaration | `include/gui_forms/timer/timer/timer.hpp:56` |
| `Painter` | `shared_ptr` | `LiveSurface` | class declaration | `include/gui_forms/types/painter/painter.hpp:70` |
| `LiveSurfacePresentation` | `shared_ptr` | `LiveSurface` | class declaration | `include/gui_forms/window/presentation/presentation_types.hpp:32` |
| `PopupToken` | `shared_ptr` | `detail::PopupAttachment` | local/signature/use | `include/gui_forms/window/window.hpp:69` |
| `PopupToken` | `shared_ptr` | `detail::PopupAttachment` | class declaration | `include/gui_forms/window/window.hpp:71` |
| `AcceleratorToken` | `shared_ptr` | `detail::AcceleratorAttachment` | local/signature/use | `include/gui_forms/window/window.hpp:140` |
| `AcceleratorToken` | `shared_ptr` | `detail::AcceleratorAttachment` | class declaration | `include/gui_forms/window/window.hpp:142` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:234` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:239` |
| `Window` | `shared_ptr` | `const Theme` | local/signature/use | `include/gui_forms/window/window.hpp:253` |
| `Window` | `shared_ptr` | `const Theme` | class declaration | `include/gui_forms/window/window.hpp:256` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:292` |
| `Window` | `shared_ptr` | `LiveSurface` | class declaration | `include/gui_forms/window/window.hpp:292` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:306` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:308` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:323` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:327` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:372` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:373` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:374` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:375` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:376` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:380` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:384` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:385` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:391` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:393` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:394` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:395` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:398` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:412` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:414` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:425` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:426` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:434` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:472` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:473` |
| `Window` | `shared_ptr` | `detail::PopupAttachment` | class declaration | `include/gui_forms/window/window.hpp:475` |
| `Window` | `shared_ptr` | `detail::AcceleratorAttachment` | class declaration | `include/gui_forms/window/window.hpp:477` |
| `FocusChangePublication` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:481` |
| `FocusScopePublication` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:487` |
| `FocusScopePublication` | `shared_ptr` | `unsigned char` | class declaration | `include/gui_forms/window/window.hpp:488` |
| `PointerCapturePublication` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:494` |
| `ControlAvailabilityPublication` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:500` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:510` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:510` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:511` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:512` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:514` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:515` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:517` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:518` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:520` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:523` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:524` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:525` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:531` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:532` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:533` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:534` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:535` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:536` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:539` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:540` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:543` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:544` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:548` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:548` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:550` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:553` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:554` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:556` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:558` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:560` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:587` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:591` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:591` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:592` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:593` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:595` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:595` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:597` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:607` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:611` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:616` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:620` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:621` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:622` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:623` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:625` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:634` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:635` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:637` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:642` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:645` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:648` |
| `Window` | `shared_ptr` | `const Theme` | class declaration | `include/gui_forms/window/window.hpp:653` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:658` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:659` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:660` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:671` |
| `FocusScopeState` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:684` |
| `FocusScopeState` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:685` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:695` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:701` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:702` |
| `Window` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:703` |
| `LiveSurfaceRegistration` | `qualified-alias:weak_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:708` |
| `LiveSurfaceRegistration` | `shared_ptr` | `LiveSurface` | class declaration | `include/gui_forms/window/window.hpp:709` |
| `PaintControlCheckpoint` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:717` |
| `PaintControlCheckpoint` | `shared_ptr` | `const detail::DisplayChunk` | class declaration | `include/gui_forms/window/window.hpp:718` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:723` |
| `Window` | `qualified-alias:shared_ptr` | `Control` | class declaration | `include/gui_forms/window/window.hpp:732` |
| `Window` | `shared_ptr` | `detail::ScheduledFrameRequest` | class declaration | `include/gui_forms/window/window.hpp:740` |
| `Window` | `shared_ptr` | `detail::WindowLifetime` | class declaration | `include/gui_forms/window/window.hpp:743` |
| `Window` | `shared_ptr` | `detail::DispatcherState` | class declaration | `include/gui_forms/window/window.hpp:745` |
| `ForeignPropertyGetter` | `shared_ptr` | `State` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:9` |
| `ForeignPropertySetter` | `shared_ptr` | `State` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:15` |
| `ForeignPropertyChangeConnector` | `shared_ptr` | `State` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:24` |
| `ForeignPropertyResetter` | `shared_ptr` | `State` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:33` |
| `ForeignPropertyShouldSerialize` | `shared_ptr` | `State` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:42` |
| `ForeignPropertyOrigin` | `shared_ptr` | `State` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:48` |
| `ForeignConverterFormatter` | `weak_ptr` | `State` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:58` |
| `ForeignConverterFormatter` | `shared_ptr` | `State` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:62` |
| `ForeignConverterParser` | `weak_ptr` | `State` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:73` |
| `ForeignConverterParser` | `shared_ptr` | `State` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:77` |
| `ForeignEditorTextUpdater` | `weak_ptr` | `gui_forms::Button` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:88` |
| `ForeignEditorTextUpdater` | `weak_ptr` | `State` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:89` |
| `ForeignEditorTextUpdater` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:92` |
| `ForeignEditorTextUpdater` | `shared_ptr` | `State` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:93` |
| `ForeignEditorSynchronizer` | `shared_ptr` | `gui_forms::BindingValue` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:108` |
| `ForeignEditorCommitRelay` | `weak_ptr` | `State` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:119` |
| `ForeignEditorCommitRelay` | `shared_ptr` | `gui_forms::BindingValue` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:120` |
| `ForeignEditorCommitRelay` | `shared_ptr` | `gui_forms::Event< const gui_forms::PropertyEditorInputError&>` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:121` |
| `ForeignEditorCommitRelay` | `shared_ptr` | `State` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:126` |
| `ForeignEditorCommitConnector` | `weak_ptr` | `gui_forms::Button` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:140` |
| `ForeignEditorCommitConnector` | `weak_ptr` | `State` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:141` |
| `ForeignEditorCommitConnector` | `shared_ptr` | `gui_forms::BindingValue` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:142` |
| `ForeignEditorCommitConnector` | `shared_ptr` | `gui_forms::Event< const gui_forms::PropertyEditorInputError&>` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:143` |
| `ForeignEditorCommitConnector` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:149` |
| `ForeignEditorFailureConnector` | `shared_ptr` | `gui_forms::Event< const gui_forms::PropertyEditorInputError&>` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:158` |
| `ForeignPropertyEditorFactory` | `weak_ptr` | `State` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:171` |
| `ForeignPropertyEditorFactory` | `shared_ptr` | `State` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:175` |
| `ForeignPropertyEditorFactory` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:180` |
| `ForeignPropertyEditorFactory` | `shared_ptr` | `gui_forms::BindingValue` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:183` |
| `ForeignPropertyEditorFactory` | `shared_ptr` | `gui_forms::Event< const gui_forms::PropertyEditorInputError&>` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:185` |
| `ForeignPropertyEditorFactory` | `weak_ptr` | `gui_forms::Button` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:190` |
| `ForeignPropertyEditorFactory` | `weak_ptr` | `gui_forms::Button` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:201` |
| `AbiPropertyObjectControl` | `shared_ptr` | `gui_forms::abi::detail::AbiPropertyObjectControl::PropertyState` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:234` |
| `AbiPropertyObjectControl` | `shared_ptr` | `PropertyState` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:289` |
| `AbiPropertyObjectControl` | `shared_ptr` | `PropertyState` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:301` |
| `AbiPropertyObjectControl` | `shared_ptr` | `PropertyState` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:302` |
| `AbiPropertyObjectControl` | `weak_ptr` | `PropertyState` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:311` |
| `AbiPropertyObjectControl` | `weak_ptr` | `PropertyState` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:314` |
| `AbiPropertyObjectControl` | `shared_ptr` | `PropertyState` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:325` |
| `AbiPropertyObjectControl` | `shared_ptr` | `PropertyState` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:326` |
| `AbiPropertyObjectControl` | `weak_ptr` | `PropertyState` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:334` |
| `AbiPropertyObjectControl` | `shared_ptr` | `gui_forms::PropertyEnumDescriptor` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:572` |
| `AbiPropertyObjectControl` | `shared_ptr` | `PropertyState` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:749` |
| `RasterControl` | `shared_ptr` | `gui_forms::LiveSurface` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:120` |
| `RasterControl` | `shared_ptr` | `gui_forms::LiveSurface` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:129` |
| `RasterControl` | `shared_ptr` | `LiveWakeState` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:152` |
| `DeferredLiveSurfacePaint` | `weak_ptr` | `RasterControl` | class declaration | `src/abi/control_adapters/raster_control/raster_control.hpp:241` |
| `DeferredLiveSurfacePaint` | `weak_ptr` | `LiveWakeState` | class declaration | `src/abi/control_adapters/raster_control/raster_control.hpp:242` |
| `LiveSurfaceWakeCallback` | `weak_ptr` | `RasterControl` | class declaration | `src/abi/control_adapters/raster_control/raster_control.hpp:250` |
| `LiveSurfaceWakeCallback` | `weak_ptr` | `LiveWakeState` | class declaration | `src/abi/control_adapters/raster_control/raster_control.hpp:251` |
| `RasterControl` | `weak_ptr` | `RasterControl` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:259` |
| `RasterControl` | `weak_ptr` | `LiveWakeState` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:260` |
| `RasterControl` | `shared_ptr` | `LiveWakeState` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:261` |
| `RasterControl` | `shared_ptr` | `RasterControl` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:266` |
| `RasterControl` | `weak_ptr` | `RasterControl` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:285` |
| `RasterControl` | `weak_ptr` | `LiveWakeState` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:286` |
| `RasterControl` | `shared_ptr` | `gui_forms::abi::detail::RasterControl::LiveWakeState` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:287` |
| `RasterControl` | `shared_ptr` | `gui_forms::abi::detail::RasterControl` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:292` |
| `RasterControl` | `shared_ptr` | `gui_forms::abi::detail::RasterControl` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:308` |
| `RasterControl` | `shared_ptr` | `gui_forms::abi::detail::RasterControl::LiveWakeState` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:317` |
| `RasterControl` | `weak_ptr` | `RasterControl` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:319` |
| `RasterControl` | `weak_ptr` | `LiveWakeState` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:323` |
| `RasterControl` | `shared_ptr` | `gui_forms::LiveSurface` | class declaration | `src/abi/control_adapters/raster_control/raster_control.hpp:342` |
| `RasterControl` | `shared_ptr` | `LiveWakeState` | class declaration | `src/abi/control_adapters/raster_control/raster_control.hpp:345` |
| `ObjectRecord` | `shared_ptr` | `gui_drawing::DrawingObject` | class declaration | `src/abi/drawing_c_api.cpp:101` |
| `Slot` | `shared_ptr` | `ObjectRecord` | class declaration | `src/abi/drawing_c_api.cpp:113` |
| `Registry` | `unique_ptr` | `Object` | local/signature/use | `src/abi/drawing_c_api.cpp:130` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:181` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:193` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:205` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:216` |
| `Registry` | `shared_ptr` | `Object` | local/signature/use | `src/abi/drawing_c_api.cpp:230` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:243` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:247` |
| `Registry` | `shared_ptr` | `Left` | local/signature/use | `src/abi/drawing_c_api.cpp:265` |
| `Registry` | `shared_ptr` | `Right` | local/signature/use | `src/abi/drawing_c_api.cpp:269` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:287` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:288` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:289` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:290` |
| `Registry` | `shared_ptr` | `gui_drawing::GraphicsRecorder` | local/signature/use | `src/abi/drawing_c_api.cpp:314` |
| `Registry` | `shared_ptr` | `gui_drawing::Font` | local/signature/use | `src/abi/drawing_c_api.cpp:315` |
| `Registry` | `shared_ptr` | `gui_drawing::SolidBrush` | local/signature/use | `src/abi/drawing_c_api.cpp:316` |
| `Registry` | `shared_ptr` | `gui_drawing::StringFormat` | local/signature/use | `src/abi/drawing_c_api.cpp:317` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:329` |
| `Registry` | `unique_ptr` | `Object` | local/signature/use | `src/abi/drawing_c_api.cpp:342` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:343` |
| `Registry` | `shared_ptr` | `Object` | local/signature/use | `src/abi/drawing_c_api.cpp:345` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:354` |
| `Registry` | `shared_ptr` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:383` |
| `<file/function>` | `unique_ptr` | `gui_drawing::Bitmap` | local/signature/use | `src/abi/drawing_c_api.cpp:787` |
| `<file/function>` | `unique_ptr` | `gui_drawing::Bitmap` | local/signature/use | `src/abi/drawing_c_api.cpp:808` |
| `<file/function>` | `unique_ptr` | `gui_drawing::Bitmap` | local/signature/use | `src/abi/drawing_c_api.cpp:816` |
| `<file/function>` | `unique_ptr` | `gui_drawing::Bitmap` | local/signature/use | `src/abi/drawing_c_api.cpp:1098` |
| `<file/function>` | `unique_ptr` | `gui_drawing::Bitmap` | local/signature/use | `src/abi/drawing_c_api.cpp:1951` |
| `CapturedSurface` | `unique_ptr` | `Bitmap` | class declaration | `src/abi/drawing_platform.hpp:13` |
| `<file/function>` | `unique_ptr` | `Bitmap` | local/signature/use | `src/abi/drawing_platform.hpp:18` |
| `<file/function>` | `unique_ptr` | `Bitmap` | local/signature/use | `src/abi/drawing_platform_stub.cpp:9` |
| `<file/function>` | `unique_ptr` | `Bitmap` | local/signature/use | `src/abi/drawing_platform_windows.cpp:183` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:19` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:28` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:35` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:38` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:43` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:45` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:53` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:55` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:61` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:63` |
| `<file/function>` | `weak_ptr` | `RasterControl` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:78` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:88` |
| `<file/function>` | `shared_ptr` | `gui_forms::LiveSurface` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:96` |
| `<file/function>` | `shared_ptr` | `gui_forms::abi::detail::RasterControl` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:98` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:117` |
| `<file/function>` | `shared_ptr` | `gui_forms::abi::detail::RasterControl` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:132` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:144` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:154` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:171` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:195` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:210` |
| `<file/function>` | `shared_ptr` | `gui_forms::abi::detail::RasterControl` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:232` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:246` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:262` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:274` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:289` |
| `CompatibilityPaintBinding` | `weak_ptr` | `RasterControl` | class declaration | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.hpp:17` |
| `CompatibilityPaintBinding` | `shared_ptr` | `gui_forms::LiveSurface` | class declaration | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.hpp:19` |
| `CompatibilityPaintWrite` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | class declaration | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.hpp:23` |
| `ControlRecord` | `shared_ptr` | `Control` | class declaration | `src/abi/registry/registry.hpp:43` |
| `ControlRecord` | `weak_ptr` | `Control` | class declaration | `src/abi/registry/registry.hpp:45` |
| `RegistrySlot` | `shared_ptr` | `ControlRecord` | class declaration | `src/abi/registry/registry.hpp:78` |
| `RegistrySlot` | `shared_ptr` | `SubscriptionRecord` | class declaration | `src/abi/registry/registry.hpp:79` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:108` |
| `Registry` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `src/abi/registry/registry.hpp:118` |
| `Registry` | `shared_ptr` | `gui_forms::PropertyGrid` | local/signature/use | `src/abi/registry/registry.hpp:187` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:231` |
| `Registry` | `shared_ptr` | `gui_forms::ButtonBase` | local/signature/use | `src/abi/registry/registry.hpp:237` |
| `Registry` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `src/abi/registry/registry.hpp:240` |
| `Registry` | `shared_ptr` | `gui_forms::GroupBox` | local/signature/use | `src/abi/registry/registry.hpp:243` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::FieldControl` | local/signature/use | `src/abi/registry/registry.hpp:246` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:260` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:280` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:293` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:306` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:323` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:340` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:353` |
| `Registry` | `shared_ptr` | `gui_forms::ScrollableControl` | local/signature/use | `src/abi/registry/registry.hpp:357` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:373` |
| `Registry` | `shared_ptr` | `gui_forms::ScrollableControl` | local/signature/use | `src/abi/registry/registry.hpp:377` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:393` |
| `Registry` | `shared_ptr` | `gui_forms::ScrollableControl` | local/signature/use | `src/abi/registry/registry.hpp:397` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:412` |
| `Registry` | `shared_ptr` | `gui_forms::ScrollableControl` | local/signature/use | `src/abi/registry/registry.hpp:416` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:431` |
| `Registry` | `shared_ptr` | `gui_forms::ScrollableControl` | local/signature/use | `src/abi/registry/registry.hpp:435` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:497` |
| `Registry` | `shared_ptr` | `gui_forms::ScrollableControl` | local/signature/use | `src/abi/registry/registry.hpp:501` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:520` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:521` |
| `Registry` | `shared_ptr` | `gui_forms::ScrollableControl` | local/signature/use | `src/abi/registry/registry.hpp:529` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:547` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:560` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:569` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:582` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:601` |
| `Registry` | `shared_ptr` | `gui_forms::PropertyGrid` | local/signature/use | `src/abi/registry/registry.hpp:606` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:612` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:615` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:626` |
| `Registry` | `shared_ptr` | `PropertyValueConverterRegistry` | local/signature/use | `src/abi/registry/registry.hpp:627` |
| `Registry` | `shared_ptr` | `PropertyEditorRegistry` | local/signature/use | `src/abi/registry/registry.hpp:628` |
| `Registry` | `shared_ptr` | `PropertyValueConverterRegistry` | local/signature/use | `src/abi/registry/registry.hpp:629` |
| `Registry` | `shared_ptr` | `PropertyEditorRegistry` | local/signature/use | `src/abi/registry/registry.hpp:631` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:633` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::AbiPropertyObjectControl` | local/signature/use | `src/abi/registry/registry.hpp:634` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:666` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::AbiPropertyObjectControl` | local/signature/use | `src/abi/registry/registry.hpp:671` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:688` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::AbiPropertyObjectControl` | local/signature/use | `src/abi/registry/registry.hpp:693` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:728` |
| `Registry` | `shared_ptr` | `gui_forms::PropertyGrid` | local/signature/use | `src/abi/registry/registry.hpp:733` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:758` |
| `Registry` | `shared_ptr` | `gui_forms::PropertyGrid` | local/signature/use | `src/abi/registry/registry.hpp:763` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:788` |
| `Registry` | `shared_ptr` | `gui_forms::PropertyGrid` | local/signature/use | `src/abi/registry/registry.hpp:793` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:809` |
| `Registry` | `shared_ptr` | `gui_forms::PropertyGrid` | local/signature/use | `src/abi/registry/registry.hpp:814` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:833` |
| `Registry` | `shared_ptr` | `gui_forms::PropertyGrid` | local/signature/use | `src/abi/registry/registry.hpp:838` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:851` |
| `Registry` | `shared_ptr` | `gui_forms::PropertyGrid` | local/signature/use | `src/abi/registry/registry.hpp:856` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:875` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::RasterControl` | local/signature/use | `src/abi/registry/registry.hpp:879` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:906` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::RasterControl` | local/signature/use | `src/abi/registry/registry.hpp:910` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:925` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::RasterControl` | local/signature/use | `src/abi/registry/registry.hpp:929` |
| `Registry` | `weak_ptr` | `RasterControl` | local/signature/use | `src/abi/registry/registry.hpp:945` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:951` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::RasterControl` | local/signature/use | `src/abi/registry/registry.hpp:955` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:972` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:973` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:993` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::FieldControl` | local/signature/use | `src/abi/registry/registry.hpp:1004` |
| `Registry` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `src/abi/registry/registry.hpp:1006` |
| `Registry` | `shared_ptr` | `gui_forms::ButtonBase` | local/signature/use | `src/abi/registry/registry.hpp:1009` |
| `Registry` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `src/abi/registry/registry.hpp:1017` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1036` |
| `Registry` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `src/abi/registry/registry.hpp:1044` |
| `Registry` | `shared_ptr` | `gui_forms::ButtonBase` | local/signature/use | `src/abi/registry/registry.hpp:1050` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1068` |
| `Registry` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/abi/registry/registry.hpp:1072` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1090` |
| `Registry` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `src/abi/registry/registry.hpp:1094` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1115` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::FieldControl` | local/signature/use | `src/abi/registry/registry.hpp:1119` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1138` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::FieldControl` | local/signature/use | `src/abi/registry/registry.hpp:1142` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1160` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::FieldControl` | local/signature/use | `src/abi/registry/registry.hpp:1164` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1184` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1190` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1222` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1228` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1263` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::FieldControl` | local/signature/use | `src/abi/registry/registry.hpp:1267` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1292` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::FieldControl` | local/signature/use | `src/abi/registry/registry.hpp:1296` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1315` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::FieldControl` | local/signature/use | `src/abi/registry/registry.hpp:1319` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1333` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::FieldControl` | local/signature/use | `src/abi/registry/registry.hpp:1337` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1351` |
| `Registry` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `src/abi/registry/registry.hpp:1355` |
| `Registry` | `shared_ptr` | `gui_forms::RadioButton` | local/signature/use | `src/abi/registry/registry.hpp:1361` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1381` |
| `Registry` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `src/abi/registry/registry.hpp:1385` |
| `Registry` | `shared_ptr` | `gui_forms::RadioButton` | local/signature/use | `src/abi/registry/registry.hpp:1390` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1402` |
| `Registry` | `shared_ptr` | `gui_forms::RangeControl` | local/signature/use | `src/abi/registry/registry.hpp:1406` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1421` |
| `Registry` | `shared_ptr` | `gui_forms::RangeControl` | local/signature/use | `src/abi/registry/registry.hpp:1425` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1437` |
| `Registry` | `shared_ptr` | `gui_forms::RangeControl` | local/signature/use | `src/abi/registry/registry.hpp:1441` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1456` |
| `Registry` | `shared_ptr` | `gui_forms::RangeControl` | local/signature/use | `src/abi/registry/registry.hpp:1460` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1475` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1488` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1528` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1529` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1605` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1614` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1640` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1651` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1671` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1677` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1693` |
| `Registry` | `unique_ptr` | `gui_forms::Window` | local/signature/use | `src/abi/registry/registry.hpp:1718` |
| `Registry` | `shared_ptr` | `std::unordered_map<std::string, std::shared_ptr<Control>>` | local/signature/use | `src/abi/registry/registry.hpp:1743` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:1743` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:1793` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1851` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1869` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1886` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1904` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1905` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1922` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1938` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1949` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1968` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:1988` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2001` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2016` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2030` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2044` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2051` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2065` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2066` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2083` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2084` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2091` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2101` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2102` |
| `Registry` | `weak_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2127` |
| `Registry` | `weak_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2129` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2131` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2142` |
| `Registry` | `weak_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2147` |
| `Registry` | `weak_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2149` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2151` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2167` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::SubscriptionRecord` | local/signature/use | `src/abi/registry/registry.hpp:2174` |
| `Registry` | `shared_ptr` | `SubscriptionRecord` | local/signature/use | `src/abi/registry/registry.hpp:2185` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2215` |
| `Registry` | `shared_ptr` | `gui_forms::RangeControl` | local/signature/use | `src/abi/registry/registry.hpp:2232` |
| `Registry` | `shared_ptr` | `gui_forms::ScrollableControl` | local/signature/use | `src/abi/registry/registry.hpp:2234` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::SubscriptionRecord` | local/signature/use | `src/abi/registry/registry.hpp:2250` |
| `Registry` | `shared_ptr` | `gui_forms::ButtonBase` | local/signature/use | `src/abi/registry/registry.hpp:2258` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2291` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::RasterControl` | local/signature/use | `src/abi/registry/registry.hpp:2299` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::FieldControl` | local/signature/use | `src/abi/registry/registry.hpp:2300` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::SubscriptionRecord` | local/signature/use | `src/abi/registry/registry.hpp:2301` |
| `Registry` | `weak_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2313` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2329` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::FieldControl` | local/signature/use | `src/abi/registry/registry.hpp:2333` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::RasterControl` | local/signature/use | `src/abi/registry/registry.hpp:2334` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::SubscriptionRecord` | local/signature/use | `src/abi/registry/registry.hpp:2339` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2359` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::FormControl` | local/signature/use | `src/abi/registry/registry.hpp:2365` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::SubscriptionRecord` | local/signature/use | `src/abi/registry/registry.hpp:2370` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2389` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::FieldControl` | local/signature/use | `src/abi/registry/registry.hpp:2393` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::SubscriptionRecord` | local/signature/use | `src/abi/registry/registry.hpp:2398` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2419` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2424` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2453` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2479` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2484` |
| `HostFinishGuard` | `shared_ptr` | `ControlRecord` | class declaration | `src/abi/registry/registry.hpp:2492` |
| `ManagedCallbackGuard` | `shared_ptr` | `ControlRecord` | class declaration | `src/abi/registry/registry.hpp:2498` |
| `DispatchGuard` | `shared_ptr` | `ControlRecord` | class declaration | `src/abi/registry/registry.hpp:2504` |
| `AutomationResolver` | `shared_ptr` | `std::unordered_map< std::string, std::shared_ptr<Control>>` | class declaration | `src/abi/registry/registry.hpp:2509` |
| `AutomationResolver` | `shared_ptr` | `Control` | class declaration | `src/abi/registry/registry.hpp:2510` |
| `AutomationResolver` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2512` |
| `AutomationResolver` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2514` |
| `AutomationResolver` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2517` |
| `HostReadyCallback` | `shared_ptr` | `ControlRecord` | class declaration | `src/abi/registry/registry.hpp:2533` |
| `WindowsDispatchPendingCallback` | `shared_ptr` | `ControlRecord` | class declaration | `src/abi/registry/registry.hpp:2555` |
| `WindowsDispatchPendingCallback` | `shared_ptr` | `std::unordered_map< std::string, std::shared_ptr<Control>>` | class declaration | `src/abi/registry/registry.hpp:2556` |
| `WindowsDispatchPendingCallback` | `shared_ptr` | `Control` | class declaration | `src/abi/registry/registry.hpp:2557` |
| `DispatchPendingCallback` | `shared_ptr` | `ControlRecord` | class declaration | `src/abi/registry/registry.hpp:2567` |
| `HostClosedCallback` | `shared_ptr` | `ControlRecord` | class declaration | `src/abi/registry/registry.hpp:2575` |
| `FinalSnapshotCallback` | `shared_ptr` | `ControlRecord` | class declaration | `src/abi/registry/registry.hpp:2586` |
| `RasterPointerForwarder` | `shared_ptr` | `SubscriptionRecord` | class declaration | `src/abi/registry/registry.hpp:2609` |
| `ControlPointerForwarder` | `shared_ptr` | `SubscriptionRecord` | class declaration | `src/abi/registry/registry.hpp:2623` |
| `ControlPointerForwarder` | `weak_ptr` | `Control` | class declaration | `src/abi/registry/registry.hpp:2625` |
| `ControlPointerForwarder` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2630` |
| `RasterKeyForwarder` | `shared_ptr` | `SubscriptionRecord` | class declaration | `src/abi/registry/registry.hpp:2662` |
| `KeyPreviewForwarder` | `shared_ptr` | `SubscriptionRecord` | class declaration | `src/abi/registry/registry.hpp:2677` |
| `KeyPreviewForwarder` | `shared_ptr` | `ControlRecord` | class declaration | `src/abi/registry/registry.hpp:2679` |
| `KeyPreviewForwarder` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2694` |
| `RasterTextForwarder` | `shared_ptr` | `SubscriptionRecord` | class declaration | `src/abi/registry/registry.hpp:2702` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2717` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/abi/registry/registry.hpp:2718` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2724` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2729` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2730` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2731` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2732` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2745` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2762` |
| `Registry` | `shared_ptr` | `SubscriptionRecord` | local/signature/use | `src/abi/registry/registry.hpp:2779` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2780` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2783` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::SubscriptionRecord` | local/signature/use | `src/abi/registry/registry.hpp:2805` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2820` |
| `Registry` | `shared_ptr` | `ControlRecord` | class declaration | `src/abi/registry/registry.hpp:2847` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2862` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2899` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2908` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2913` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:2978` |
| `Registry` | `shared_ptr` | `gui_forms::ButtonBase` | local/signature/use | `src/abi/registry/registry.hpp:2995` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:2996` |
| `Registry` | `shared_ptr` | `gui_forms::ButtonBase` | local/signature/use | `src/abi/registry/registry.hpp:2997` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:3001` |
| `Registry` | `shared_ptr` | `gui_forms::ButtonBase` | local/signature/use | `src/abi/registry/registry.hpp:3007` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:3013` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:3014` |
| `Registry` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:3020` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:3023` |
| `Registry` | `shared_ptr` | `std::unordered_map<std::string, std::shared_ptr<Control>>` | local/signature/use | `src/abi/registry/registry.hpp:3029` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:3029` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:3030` |
| `Registry` | `shared_ptr` | `std::unordered_map< std::string, std::shared_ptr<Control>>` | local/signature/use | `src/abi/registry/registry.hpp:3031` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:3032` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:3033` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:3039` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:3040` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:3053` |
| `Registry` | `shared_ptr` | `Control` | local/signature/use | `src/abi/registry/registry.hpp:3054` |
| `Registry` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/abi/registry/registry.hpp:3058` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:3074` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:3082` |
| `Registry` | `shared_ptr` | `SubscriptionRecord` | local/signature/use | `src/abi/registry/registry.hpp:3098` |
| `Registry` | `shared_ptr` | `ControlRecord` | local/signature/use | `src/abi/registry/registry.hpp:3123` |
| `Registry` | `shared_ptr` | `SubscriptionRecord` | local/signature/use | `src/abi/registry/registry.hpp:3124` |
| `Registry` | `shared_ptr` | `ControlRecord` | class declaration | `src/abi/registry/registry.hpp:3168` |
| `Registry` | `shared_ptr` | `SubscriptionRecord` | local/signature/use | `src/abi/registry/registry.hpp:3169` |
| `Registry` | `shared_ptr` | `gui_forms::abi::detail::SubscriptionRecord` | local/signature/use | `src/abi/registry/registry.hpp:3182` |
| `<file/function>` | `shared_ptr` | `const PropertyEnumDescriptor` | local/signature/use | `src/controls/basic/basic_control_rendering.cpp:11` |
| `<file/function>` | `shared_ptr` | `const gui_forms::PropertyEnumDescriptor` | local/signature/use | `src/controls/basic/basic_control_rendering.cpp:12` |
| `<file/function>` | `shared_ptr` | `const PropertyEnumDescriptor` | local/signature/use | `src/controls/basic/basic_control_rendering.hpp:18` |
| `<file/function>` | `shared_ptr` | `ImageList` | local/signature/use | `src/controls/button_base/button_base.cpp:148` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/button_base/radio_button/radio_button.cpp:48` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/button_base/radio_button/radio_button.cpp:49` |
| `<file/function>` | `shared_ptr` | `gui_forms::RadioButton` | local/signature/use | `src/controls/button_base/radio_button/radio_button.cpp:50` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/button_base/radio_button/radio_button.cpp:72` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/button_base/radio_button/radio_button.cpp:73` |
| `<file/function>` | `shared_ptr` | `gui_forms::RadioButton` | local/signature/use | `src/controls/button_base/radio_button/radio_button.cpp:74` |
| `ExecuteBoundCommand` | `weak_ptr` | `Command` | class declaration | `src/controls/commands/command_binding/command_binding.cpp:10` |
| `ExecuteBoundCommand` | `shared_ptr` | `Command` | local/signature/use | `src/controls/commands/command_binding/command_binding.cpp:13` |
| `SynchronizeBoundButton` | `weak_ptr` | `ButtonBase` | class declaration | `src/controls/commands/command_binding/command_binding.cpp:20` |
| `SynchronizeBoundButton` | `shared_ptr` | `ButtonBase` | local/signature/use | `src/controls/commands/command_binding/command_binding.cpp:24` |
| `<file/function>` | `shared_ptr` | `Command` | local/signature/use | `src/controls/commands/command_binding/command_binding.cpp:39` |
| `<file/function>` | `shared_ptr` | `ButtonBase` | local/signature/use | `src/controls/commands/command_binding/command_binding.cpp:40` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:55` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:56` |
| `<file/function>` | `shared_ptr` | `SplitterPanel` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:56` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:57` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:75` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:87` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:87` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:91` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:91` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:240` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/container/master_detail_view/master_detail_view.cpp:241` |
| `GalleryControl` | `shared_ptr` | `GalleryContext` | class declaration | `src/controls/gallery/control/gallery_control.hpp:16` |
| `GalleryControl` | `shared_ptr` | `GalleryContext` | class declaration | `src/controls/gallery/control/gallery_control.hpp:35` |
| `GalleryTree` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/gallery/tree/gallery_tree.hpp:11` |
| `GalleryTree` | `shared_ptr` | `GalleryContext` | class declaration | `src/controls/gallery/tree/gallery_tree.hpp:12` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:55` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:163` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `src/controls/gallery_controls.cpp:179` |
| `GalleryButton` | `shared_ptr` | `GalleryContext` | local/signature/use | `src/controls/gallery_controls.cpp:216` |
| `GalleryButton` | `shared_ptr` | `GalleryContext` | class declaration | `src/controls/gallery_controls.cpp:237` |
| `GalleryCheckBox` | `shared_ptr` | `GalleryContext` | local/signature/use | `src/controls/gallery_controls.cpp:244` |
| `GalleryCheckBox` | `shared_ptr` | `GalleryContext` | class declaration | `src/controls/gallery_controls.cpp:276` |
| `GalleryRadioButton` | `shared_ptr` | `GalleryContext` | local/signature/use | `src/controls/gallery_controls.cpp:283` |
| `GalleryRadioButton` | `shared_ptr` | `GalleryContext` | class declaration | `src/controls/gallery_controls.cpp:306` |
| `GalleryLinkLabel` | `shared_ptr` | `GalleryContext` | local/signature/use | `src/controls/gallery_controls.cpp:313` |
| `GalleryLinkLabel` | `shared_ptr` | `GalleryContext` | class declaration | `src/controls/gallery_controls.cpp:330` |
| `GalleryTrackBar` | `shared_ptr` | `GalleryContext` | local/signature/use | `src/controls/gallery_controls.cpp:337` |
| `GalleryTrackBar` | `shared_ptr` | `GalleryContext` | class declaration | `src/controls/gallery_controls.cpp:360` |
| `GalleryStatusLabel` | `shared_ptr` | `GalleryContext` | local/signature/use | `src/controls/gallery_controls.cpp:367` |
| `GalleryStatusLabel` | `shared_ptr` | `GalleryContext` | class declaration | `src/controls/gallery_controls.cpp:381` |
| `GalleryLifecycleCard` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:413` |
| `GalleryLifecycleCard` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:438` |
| `GalleryLifecycleCard` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `src/controls/gallery_controls.cpp:440` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:524` |
| `<file/function>` | `shared_ptr` | `gui_forms::ButtonBase` | local/signature/use | `src/controls/gallery_controls.cpp:525` |
| `<file/function>` | `shared_ptr` | `gui_forms::RangeControl` | local/signature/use | `src/controls/gallery_controls.cpp:527` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `src/controls/gallery_controls.cpp:531` |
| `<file/function>` | `shared_ptr` | `gui_forms::RadioButton` | local/signature/use | `src/controls/gallery_controls.cpp:535` |
| `<file/function>` | `shared_ptr` | `gui_forms::RadioButton` | local/signature/use | `src/controls/gallery_controls.cpp:539` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `src/controls/gallery_controls.cpp:544` |
| `<file/function>` | `shared_ptr` | `gui_forms::LinkLabel` | local/signature/use | `src/controls/gallery_controls.cpp:548` |
| `<file/function>` | `shared_ptr` | `gui_forms::TrackBar` | local/signature/use | `src/controls/gallery_controls.cpp:553` |
| `<file/function>` | `shared_ptr` | `gui_forms::ProgressBar` | local/signature/use | `src/controls/gallery_controls.cpp:557` |
| `<file/function>` | `shared_ptr` | `GalleryContext` | local/signature/use | `src/controls/gallery_controls.cpp:577` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:627` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:661` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:690` |
| `<file/function>` | `shared_ptr` | `gui_forms::gallery::GalleryContext` | local/signature/use | `src/controls/gallery_controls.cpp:974` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:975` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:979` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `src/controls/gallery_controls.cpp:991` |
| `<file/function>` | `shared_ptr` | `gui_forms::ProgressBar` | local/signature/use | `src/controls/gallery_controls.cpp:1019` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:1035` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/gallery_controls.cpp:1037` |
| `<file/function>` | `shared_ptr` | `gui_forms::ErrorGlyph` | local/signature/use | `src/controls/guidance/error_glyph/error_glyph.cpp:137` |
| `ErrorProvider` | `weak_ptr` | `Control` | class declaration | `src/controls/guidance/error_provider/error_provider.cpp:25` |
| `ErrorProvider` | `shared_ptr` | `ErrorLayer` | class declaration | `src/controls/guidance/error_provider/error_provider.cpp:29` |
| `ErrorProvider` | `shared_ptr` | `ErrorGlyph` | class declaration | `src/controls/guidance/error_provider/error_provider.cpp:30` |
| `ErrorProvider` | `unique_ptr` | `PopupToken` | class declaration | `src/controls/guidance/error_provider/error_provider.cpp:31` |
| `ErrorProvider` | `weak_ptr` | `Control` | class declaration | `src/controls/guidance/error_provider/error_provider.cpp:37` |
| `ErrorProvider` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:40` |
| `<file/function>` | `unique_ptr` | `Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:97` |
| `<file/function>` | `unique_ptr` | `Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:99` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::WindowLifetime` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:110` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:125` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:145` |
| `<file/function>` | `unique_ptr` | `gui_forms::ErrorProvider::Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:151` |
| `<file/function>` | `weak_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:153` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:161` |
| `<file/function>` | `unique_ptr` | `Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:190` |
| `<file/function>` | `unique_ptr` | `Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:192` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:193` |
| `<file/function>` | `unique_ptr` | `Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:203` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:206` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:215` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:235` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:310` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:312` |
| `<file/function>` | `shared_ptr` | `BindingSource` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:315` |
| `<file/function>` | `shared_ptr` | `BindingSource` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:399` |
| `<file/function>` | `weak_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:405` |
| `<file/function>` | `weak_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:408` |
| `<file/function>` | `weak_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:410` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:411` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:421` |
| `<file/function>` | `shared_ptr` | `gui_forms::BindingSource` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:434` |
| `Aggregate` | `shared_ptr` | `Control` | class declaration | `src/controls/guidance/error_provider/error_provider.cpp:442` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:446` |
| `<file/function>` | `shared_ptr` | `gui_forms::Binding` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:448` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:451` |
| `<file/function>` | `weak_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:470` |
| `<file/function>` | `weak_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:486` |
| `<file/function>` | `weak_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:488` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:489` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:495` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:511` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:566` |
| `<file/function>` | `unique_ptr` | `Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:603` |
| `<file/function>` | `unique_ptr` | `Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:605` |
| `<file/function>` | `unique_ptr` | `Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:625` |
| `<file/function>` | `unique_ptr` | `Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:627` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:628` |
| `<file/function>` | `unique_ptr` | `Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:661` |
| `<file/function>` | `unique_ptr` | `Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:663` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:664` |
| `HelpProvider` | `weak_ptr` | `Control` | class declaration | `src/controls/guidance/help_provider/help_provider.cpp:20` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::WindowLifetime` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:58` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:73` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:93` |
| `<file/function>` | `unique_ptr` | `gui_forms::HelpProvider::Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:99` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:112` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:127` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:146` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:165` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:182` |
| `<file/function>` | `unique_ptr` | `Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:207` |
| `<file/function>` | `unique_ptr` | `Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:209` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:210` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:230` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:256` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:258` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:273` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:286` |
| `<file/function>` | `unique_ptr` | `Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:300` |
| `<file/function>` | `unique_ptr` | `Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:302` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:303` |
| `NumericEditorSynchronizer` | `weak_ptr` | `NumericUpDown` | class declaration | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:20` |
| `NumericEditorSynchronizer` | `shared_ptr` | `bool` | class declaration | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:21` |
| `NumericEditorSynchronizer` | `shared_ptr` | `NumericUpDown` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:24` |
| `NumericEditorCommitRelay` | `shared_ptr` | `bool` | class declaration | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:34` |
| `NumericEditorCommitConnector` | `weak_ptr` | `NumericUpDown` | class declaration | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:43` |
| `NumericEditorCommitConnector` | `shared_ptr` | `bool` | class declaration | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:44` |
| `NumericEditorCommitConnector` | `shared_ptr` | `NumericUpDown` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:49` |
| `FlagsEditorSynchronizer` | `weak_ptr` | `FlagsValueEditor` | class declaration | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:59` |
| `FlagsEditorSynchronizer` | `shared_ptr` | `bool` | class declaration | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:60` |
| `FlagsEditorSynchronizer` | `shared_ptr` | `FlagsValueEditor` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:63` |
| `FlagsEditorCommitRelay` | `shared_ptr` | `bool` | class declaration | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:73` |
| `FlagsEditorCommitConnector` | `weak_ptr` | `FlagsValueEditor` | class declaration | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:82` |
| `FlagsEditorCommitConnector` | `shared_ptr` | `bool` | class declaration | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:83` |
| `FlagsEditorCommitConnector` | `shared_ptr` | `FlagsValueEditor` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:88` |
| `ColorEditorSynchronizer` | `weak_ptr` | `ColorValueEditor` | class declaration | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:98` |
| `ColorEditorSynchronizer` | `shared_ptr` | `bool` | class declaration | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:99` |
| `ColorEditorSynchronizer` | `shared_ptr` | `ColorValueEditor` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:102` |
| `ColorEditorCommitRelay` | `shared_ptr` | `bool` | class declaration | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:112` |
| `ColorEditorCommitConnector` | `weak_ptr` | `ColorValueEditor` | class declaration | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:121` |
| `ColorEditorCommitConnector` | `shared_ptr` | `bool` | class declaration | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:122` |
| `ColorEditorCommitConnector` | `shared_ptr` | `ColorValueEditor` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:127` |
| `ColorEditorFailureConnector` | `weak_ptr` | `ColorValueEditor` | class declaration | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:137` |
| `ColorEditorFailureConnector` | `shared_ptr` | `ColorValueEditor` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:142` |
| `<file/function>` | `shared_ptr` | `NumericUpDown` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:153` |
| `<file/function>` | `shared_ptr` | `bool` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:163` |
| `<file/function>` | `shared_ptr` | `FlagsValueEditor` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:189` |
| `<file/function>` | `shared_ptr` | `bool` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:196` |
| `<file/function>` | `shared_ptr` | `ColorValueEditor` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:209` |
| `<file/function>` | `shared_ptr` | `bool` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:219` |
| `<file/function>` | `shared_ptr` | `PropertyEditorRegistry` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:311` |
| `<file/function>` | `shared_ptr` | `gui_forms::PropertyEditorRegistry` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:312` |
| `<file/function>` | `shared_ptr` | `PropertyValueConverterRegistry` | local/signature/use | `src/controls/inspection/property_value_converter_registry/property_value_converter_registry.cpp:192` |
| `<file/function>` | `shared_ptr` | `gui_forms::PropertyValueConverterRegistry` | local/signature/use | `src/controls/inspection/property_value_converter_registry/property_value_converter_registry.cpp:194` |
| `PanelState` | `shared_ptr` | `MenuPanel` | class declaration | `src/controls/menu/context_menu/context_menu.cpp:27` |
| `PanelState` | `shared_ptr` | `MenuRow` | class declaration | `src/controls/menu/context_menu/context_menu.cpp:28` |
| `ContextMenu` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/menu/context_menu/context_menu.cpp:332` |
| `ContextMenu` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/menu/context_menu/context_menu.cpp:354` |
| `ContextMenu` | `shared_ptr` | `gui_forms::ContextMenu::Impl::MenuRow` | local/signature/use | `src/controls/menu/context_menu/context_menu.cpp:433` |
| `ContextMenu` | `shared_ptr` | `MenuPanel` | local/signature/use | `src/controls/menu/context_menu/context_menu.cpp:461` |
| `ContextMenu` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/menu/context_menu/context_menu.cpp:476` |
| `ContextMenu` | `shared_ptr` | `gui_forms::ContextMenu::Impl::MenuRow` | local/signature/use | `src/controls/menu/context_menu/context_menu.cpp:478` |
| `ContextMenu` | `shared_ptr` | `gui_forms::ContextMenu::Impl::MenuRow` | local/signature/use | `src/controls/menu/context_menu/context_menu.cpp:481` |
| `ContextMenu` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/menu/context_menu/context_menu.cpp:557` |
| `ContextMenu` | `shared_ptr` | `MenuRow` | local/signature/use | `src/controls/menu/context_menu/context_menu.cpp:600` |
| `ContextMenu` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/menu/context_menu/context_menu.cpp:652` |
| `ContextMenu` | `shared_ptr` | `MenuLayer` | class declaration | `src/controls/menu/context_menu/context_menu.cpp:653` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/menu/context_menu/context_menu.cpp:699` |
| `MenuSnapshot` | `shared_ptr` | `Command` | class declaration | `src/controls/menu/menu_utilities.hpp:25` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/anchored_popup_layer/anchored_popup_layer.cpp:15` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/anchored_popup_layer/anchored_popup_layer.cpp:24` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/anchored_popup_layer/anchored_popup_layer.cpp:30` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/anchored_popup_layer/anchored_popup_layer.cpp:39` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/anchored_popup_layer/anchored_popup_layer.cpp:94` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:39` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:39` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:40` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:50` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:59` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:59` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:63` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:63` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:67` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:67` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:140` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:141` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/card/card.cpp:142` |
| `<file/function>` | `weak_ptr` | `ColorValueEditor` | local/signature/use | `src/controls/panel/color_value_editor/color_value_editor.cpp:25` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/combo_box/combo_box.cpp:286` |
| `<file/function>` | `shared_ptr` | `gui_forms::DropDownLayer` | local/signature/use | `src/controls/panel/combo_box/combo_box.cpp:302` |
| `<file/function>` | `shared_ptr` | `gui_forms::ListBox` | local/signature/use | `src/controls/panel/combo_box/combo_box.cpp:304` |
| `<file/function>` | `weak_ptr` | `ComboBox` | local/signature/use | `src/controls/panel/combo_box/combo_box.cpp:319` |
| `CorrespondenceView` | `weak_ptr` | `CorrespondenceView` | class declaration | `src/controls/panel/correspondence_view/correspondence_view.cpp:19` |
| `CorrespondenceView` | `shared_ptr` | `CorrespondenceView` | local/signature/use | `src/controls/panel/correspondence_view/correspondence_view.cpp:23` |
| `<file/function>` | `shared_ptr` | `gui_forms::CorrespondenceView` | local/signature/use | `src/controls/panel/correspondence_view/correspondence_view.cpp:446` |
| `<file/function>` | `weak_ptr` | `CorrespondenceView` | local/signature/use | `src/controls/panel/correspondence_view/correspondence_view.cpp:448` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/date_time_picker/date_time_picker.cpp:292` |
| `<file/function>` | `shared_ptr` | `gui_forms::CalendarPopupLayer` | local/signature/use | `src/controls/panel/date_time_picker/date_time_picker.cpp:306` |
| `<file/function>` | `shared_ptr` | `gui_forms::CalendarPopup` | local/signature/use | `src/controls/panel/date_time_picker/date_time_picker.cpp:308` |
| `<file/function>` | `weak_ptr` | `DateTimePicker` | local/signature/use | `src/controls/panel/date_time_picker/date_time_picker.cpp:318` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:116` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PropertyEditorDropDownLayer` | local/signature/use | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:137` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckedListBox` | local/signature/use | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:140` |
| `<file/function>` | `weak_ptr` | `FlagsValueEditor` | local/signature/use | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:155` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/group_box/scaled_group_box/scaled_group_box.cpp:30` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/group_box/scaled_group_box/scaled_group_box.cpp:64` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:70` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:70` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:74` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:75` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:80` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:80` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:83` |
| `FieldState` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:102` |
| `ModuleState` | `shared_ptr` | `RackModulePanel` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:113` |
| `ModuleState` | `shared_ptr` | `CheckBox` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:114` |
| `ModuleState` | `shared_ptr` | `Label` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:116` |
| `ModuleState` | `shared_ptr` | `Button` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:117` |
| `ChoiceFieldChanged` | `weak_ptr` | `ComboBox` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:138` |
| `TextFieldCancelled` | `weak_ptr` | `TextBox` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:167` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:253` |
| `InstrumentRack` | `weak_ptr` | `ComboBox` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:262` |
| `InstrumentRack` | `shared_ptr` | `ComboBox` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:265` |
| `InstrumentRack` | `weak_ptr` | `TextBox` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:347` |
| `InstrumentRack` | `shared_ptr` | `TextBox` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:349` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:383` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:387` |
| `InstrumentRack` | `shared_ptr` | `gui_forms::ComboBox` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:389` |
| `InstrumentRack` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:401` |
| `InstrumentRack` | `shared_ptr` | `gui_forms::ComboBox` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:420` |
| `InstrumentRack` | `weak_ptr` | `ComboBox` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:424` |
| `InstrumentRack` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:425` |
| `InstrumentRack` | `weak_ptr` | `TextBox` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:433` |
| `InstrumentRack` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:523` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:540` |
| `InstrumentRack` | `shared_ptr` | `gui_forms::ComboBox` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:555` |
| `InstrumentRack` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:565` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:577` |
| `Slot` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:587` |
| `InstrumentRack` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:681` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:745` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:799` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:811` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:875` |
| `<file/function>` | `shared_ptr` | `gui_forms::ComboBox` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:878` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:920` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:936` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:948` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:1041` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:1041` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/instrument_rack/rack_module_panel/rack_module_panel.cpp:46` |
| `RackModulePanel` | `shared_ptr` | `CheckBox` | class declaration | `src/controls/panel/instrument_rack/rack_module_panel/rack_module_panel.hpp:18` |
| `RackModulePanel` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/instrument_rack/rack_module_panel/rack_module_panel.hpp:19` |
| `RackModulePanel` | `shared_ptr` | `Label` | class declaration | `src/controls/panel/instrument_rack/rack_module_panel/rack_module_panel.hpp:21` |
| `RackModulePanel` | `shared_ptr` | `Button` | class declaration | `src/controls/panel/instrument_rack/rack_module_panel/rack_module_panel.hpp:22` |
| `<file/function>` | `shared_ptr` | `NumericUpDown` | local/signature/use | `src/controls/panel/numeric_up_down/numeric_up_down.cpp:34` |
| `<file/function>` | `shared_ptr` | `NumericUpDown` | local/signature/use | `src/controls/panel/numeric_up_down/numeric_up_down.cpp:41` |
| `<file/function>` | `weak_ptr` | `NumericUpDown` | local/signature/use | `src/controls/panel/numeric_up_down/numeric_up_down.cpp:52` |
| `<file/function>` | `shared_ptr` | `gui_forms::SpinButtons` | local/signature/use | `src/controls/panel/numeric_up_down/numeric_up_down.cpp:56` |
| `<file/function>` | `shared_ptr` | `ImageList` | local/signature/use | `src/controls/panel/object_view/object_view.cpp:270` |
| `CustomEditorCommit` | `qualified-alias:weak_ptr` | `Control` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:94` |
| `CustomEditorCommit` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:97` |
| `CustomEditorFailure` | `qualified-alias:weak_ptr` | `Control` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:106` |
| `CustomEditorFailure` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:109` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:421` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:423` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:424` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:427` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:428` |
| `PropertyGrid` | `qualified-alias:weak_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:430` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:431` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:459` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:596` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:608` |
| `PropertyGrid` | `qualified-alias:weak_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:722` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:755` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:840` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:841` |
| `OwnerEdit` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:868` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:877` |
| `PropertyGrid` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1016` |
| `PropertyGrid` | `shared_ptr` | `PropertyList` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:1141` |
| `PropertyGrid` | `shared_ptr` | `PropertyValueConverterRegistry` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:1142` |
| `PropertyGrid` | `shared_ptr` | `PropertyEditorRegistry` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:1143` |
| `PropertyGrid` | `qualified-alias:weak_ptr` | `Control` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:1144` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1186` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1190` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1191` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1192` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1195` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1199` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1203` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1210` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1217` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1222` |
| `<file/function>` | `shared_ptr` | `PropertyList` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1247` |
| `<file/function>` | `shared_ptr` | `PropertyValueConverterRegistry` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1251` |
| `<file/function>` | `shared_ptr` | `PropertyValueConverterRegistry` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1257` |
| `<file/function>` | `shared_ptr` | `PropertyEditorRegistry` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1269` |
| `<file/function>` | `shared_ptr` | `PropertyEditorRegistry` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1275` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1287` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1292` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1295` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1301` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1317` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1385` |
| `RowState` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/property_list/property_list.cpp:29` |
| `RowState` | `shared_ptr` | `Button` | class declaration | `src/controls/panel/property_list/property_list.cpp:30` |
| `TextCancelled` | `shared_ptr` | `TextBox` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:92` |
| `ChoiceChanged` | `weak_ptr` | `ComboBox` | class declaration | `src/controls/panel/property_list/property_list.cpp:105` |
| `ChoiceChanged` | `shared_ptr` | `ComboBox` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:109` |
| `CheckChanged` | `weak_ptr` | `CheckBox` | class declaration | `src/controls/panel/property_list/property_list.cpp:126` |
| `CheckChanged` | `shared_ptr` | `CheckBox` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:130` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:204` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:208` |
| `PropertyList` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:227` |
| `PropertyList` | `shared_ptr` | `gui_forms::ComboBox` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:236` |
| `PropertyList` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:255` |
| `PropertyList` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:267` |
| `PropertyList` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:285` |
| `PropertyList` | `shared_ptr` | `gui_forms::ComboBox` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:292` |
| `PropertyList` | `weak_ptr` | `ComboBox` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:296` |
| `PropertyList` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:297` |
| `PropertyList` | `weak_ptr` | `CheckBox` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:301` |
| `PropertyList` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/panel/property_list/property_list.cpp:480` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:597` |
| `<file/function>` | `shared_ptr` | `gui_forms::ComboBox` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:599` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:606` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:721` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:723` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:726` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:747` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:757` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:767` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:769` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:772` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:781` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:793` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/scaled_panel/scaled_panel.cpp:28` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/panel/scaled_panel/scaled_panel.cpp:62` |
| `<file/function>` | `shared_ptr` | `ImageList` | local/signature/use | `src/controls/panel/tree_view/tree_view.cpp:147` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:18` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:22` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:31` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:36` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:37` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:40` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:62` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:85` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/container_layout_utilities.hpp:19` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/flow_layout_panel/flow_layout_panel.cpp:74` |
| `Item` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/scrollable_control/container_control/flow_layout_panel/flow_layout_panel.cpp:103` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/flow_layout_panel/flow_layout_panel.cpp:116` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/flow_layout_panel/flow_layout_panel.cpp:117` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/scaled_layout_utilities.hpp:43` |
| `<file/function>` | `shared_ptr` | `SplitterPanel` | local/signature/use | `src/controls/scrollable_control/container_control/split_container/split_container.cpp:547` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/split_container/split_container.cpp:549` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:33` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:34` |
| `<file/function>` | `weak_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:36` |
| `<file/function>` | `shared_ptr` | `gui_forms::TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:37` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:49` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:50` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:56` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:58` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:59` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:69` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:86` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:88` |
| `<file/function>` | `weak_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:97` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:99` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:106` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:109` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:128` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:130` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:132` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:140` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:146` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:148` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:161` |
| `<file/function>` | `shared_ptr` | `gui_forms::TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:165` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:168` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:169` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:187` |
| `<file/function>` | `weak_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:280` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:282` |
| `<file/function>` | `shared_ptr` | `gui_forms::TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:289` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:292` |
| `<file/function>` | `shared_ptr` | `gui_forms::TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:303` |
| `<file/function>` | `shared_ptr` | `gui_forms::TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:304` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:320` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:384` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:398` |
| `<file/function>` | `shared_ptr` | `gui_forms::TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:458` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:466` |
| `<file/function>` | `shared_ptr` | `gui_forms::TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:469` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_control.cpp:494` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/tab_control/tab_page/tab_page.cpp:25` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:340` |
| `Item` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:371` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:379` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:380` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:384` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:385` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:411` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:582` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:592` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:595` |
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
| `ShowcaseContext` | `shared_ptr` | `gui_forms::ProgressBar` | local/signature/use | `src/controls/showcase_controls.cpp:138` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:181` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `src/controls/showcase_controls.cpp:184` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:191` |
| `<file/function>` | `shared_ptr` | `gui_forms::ScaledGroupBox` | local/signature/use | `src/controls/showcase_controls.cpp:193` |
| `WeakRangeValueMirror` | `weak_ptr` | `RangeControlType` | local/signature/use | `src/controls/showcase_controls.cpp:226` |
| `WeakRangeValueMirror` | `shared_ptr` | `RangeControlType` | local/signature/use | `src/controls/showcase_controls.cpp:231` |
| `WeakRangeValueMirror` | `weak_ptr` | `RangeControlType` | class declaration | `src/controls/showcase_controls.cpp:238` |
| `WeakProgressValueSetter` | `weak_ptr` | `ProgressBar` | local/signature/use | `src/controls/showcase_controls.cpp:244` |
| `WeakProgressValueSetter` | `shared_ptr` | `ProgressBar` | local/signature/use | `src/controls/showcase_controls.cpp:249` |
| `WeakProgressValueSetter` | `weak_ptr` | `ProgressBar` | class declaration | `src/controls/showcase_controls.cpp:255` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:302` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:303` |
| `SoundCueClick` | `weak_ptr` | `CheckBox` | local/signature/use | `src/controls/showcase_controls.cpp:306` |
| `SoundCueClick` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:308` |
| `SoundCueClick` | `shared_ptr` | `CheckBox` | local/signature/use | `src/controls/showcase_controls.cpp:321` |
| `SoundCueClick` | `weak_ptr` | `CheckBox` | class declaration | `src/controls/showcase_controls.cpp:333` |
| `SoundCueClick` | `shared_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:335` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:344` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:354` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:363` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:367` |
| `<file/function>` | `shared_ptr` | `gui_forms::LinkLabel` | local/signature/use | `src/controls/showcase_controls.cpp:370` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:374` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `src/controls/showcase_controls.cpp:378` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `src/controls/showcase_controls.cpp:388` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:394` |
| `<file/function>` | `shared_ptr` | `gui_forms::RadioButton` | local/signature/use | `src/controls/showcase_controls.cpp:398` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:410` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:413` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:417` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:423` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `src/controls/showcase_controls.cpp:436` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:442` |
| `<file/function>` | `weak_ptr` | `CheckBox` | local/signature/use | `src/controls/showcase_controls.cpp:451` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:457` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:458` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:465` |
| `<file/function>` | `shared_ptr` | `TrackBar` | local/signature/use | `src/controls/showcase_controls.cpp:468` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:470` |
| `<file/function>` | `shared_ptr` | `gui_forms::TrackBar` | local/signature/use | `src/controls/showcase_controls.cpp:476` |
| `<file/function>` | `shared_ptr` | `gui_forms::HScrollBar` | local/signature/use | `src/controls/showcase_controls.cpp:487` |
| `<file/function>` | `shared_ptr` | `gui_forms::VScrollBar` | local/signature/use | `src/controls/showcase_controls.cpp:497` |
| `<file/function>` | `weak_ptr` | `VScrollBar` | local/signature/use | `src/controls/showcase_controls.cpp:504` |
| `<file/function>` | `weak_ptr` | `HScrollBar` | local/signature/use | `src/controls/showcase_controls.cpp:507` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:512` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:517` |
| `<file/function>` | `shared_ptr` | `gui_forms::ProgressBar` | local/signature/use | `src/controls/showcase_controls.cpp:527` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:556` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:557` |
| `CollapseNavigationClick` | `weak_ptr` | `SplitContainer` | local/signature/use | `src/controls/showcase_controls.cpp:560` |
| `CollapseNavigationClick` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:561` |
| `CollapseNavigationClick` | `shared_ptr` | `SplitContainer` | local/signature/use | `src/controls/showcase_controls.cpp:566` |
| `CollapseNavigationClick` | `shared_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:567` |
| `CollapseNavigationClick` | `weak_ptr` | `SplitContainer` | class declaration | `src/controls/showcase_controls.cpp:575` |
| `CollapseNavigationClick` | `weak_ptr` | `Button` | class declaration | `src/controls/showcase_controls.cpp:576` |
| `FocusSplitterClick` | `weak_ptr` | `SplitContainer` | local/signature/use | `src/controls/showcase_controls.cpp:581` |
| `FocusSplitterClick` | `shared_ptr` | `SplitContainer` | local/signature/use | `src/controls/showcase_controls.cpp:586` |
| `FocusSplitterClick` | `weak_ptr` | `SplitContainer` | class declaration | `src/controls/showcase_controls.cpp:593` |
| `FocusContainerDescendantClick` | `weak_ptr` | `ContainerControl` | local/signature/use | `src/controls/showcase_controls.cpp:599` |
| `FocusContainerDescendantClick` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:600` |
| `FocusContainerDescendantClick` | `shared_ptr` | `ContainerControl` | local/signature/use | `src/controls/showcase_controls.cpp:605` |
| `FocusContainerDescendantClick` | `shared_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:606` |
| `FocusContainerDescendantClick` | `weak_ptr` | `ContainerControl` | class declaration | `src/controls/showcase_controls.cpp:613` |
| `FocusContainerDescendantClick` | `weak_ptr` | `Button` | class declaration | `src/controls/showcase_controls.cpp:614` |
| `UserControlLoaded` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:619` |
| `UserControlLoaded` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:624` |
| `UserControlLoaded` | `weak_ptr` | `Label` | class declaration | `src/controls/showcase_controls.cpp:631` |
| `<file/function>` | `shared_ptr` | `gui_forms::SplitContainer` | local/signature/use | `src/controls/showcase_controls.cpp:640` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:648` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:653` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:661` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:665` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:672` |
| `<file/function>` | `weak_ptr` | `SplitContainer` | local/signature/use | `src/controls/showcase_controls.cpp:676` |
| `<file/function>` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:677` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:680` |
| `<file/function>` | `shared_ptr` | `gui_forms::ContainerControl` | local/signature/use | `src/controls/showcase_controls.cpp:688` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `src/controls/showcase_controls.cpp:692` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:698` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:704` |
| `<file/function>` | `weak_ptr` | `ContainerControl` | local/signature/use | `src/controls/showcase_controls.cpp:708` |
| `<file/function>` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:709` |
| `<file/function>` | `shared_ptr` | `gui_forms::UserControl` | local/signature/use | `src/controls/showcase_controls.cpp:714` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `src/controls/showcase_controls.cpp:718` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:724` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:730` |
| `<file/function>` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:735` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:740` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:741` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:750` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:762` |
| `<file/function>` | `shared_ptr` | `gui_forms::FlowLayoutPanel` | local/signature/use | `src/controls/showcase_controls.cpp:768` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:775` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:777` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:796` |
| `<file/function>` | `shared_ptr` | `gui_forms::TableLayoutPanel` | local/signature/use | `src/controls/showcase_controls.cpp:801` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:817` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:827` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:833` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `src/controls/showcase_controls.cpp:840` |
| `<file/function>` | `shared_ptr` | `gui_forms::TrackBar` | local/signature/use | `src/controls/showcase_controls.cpp:848` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:857` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:866` |
| `<file/function>` | `shared_ptr` | `gui_forms::ProgressBar` | local/signature/use | `src/controls/showcase_controls.cpp:872` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:878` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:886` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:887` |
| `ToggleDockEdgeClick` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:890` |
| `ToggleDockEdgeClick` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:891` |
| `ToggleDockEdgeClick` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:892` |
| `ToggleDockEdgeClick` | `shared_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:898` |
| `ToggleDockEdgeClick` | `shared_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:899` |
| `ToggleDockEdgeClick` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:904` |
| `ToggleDockEdgeClick` | `weak_ptr` | `Button` | class declaration | `src/controls/showcase_controls.cpp:913` |
| `ToggleDockEdgeClick` | `weak_ptr` | `Button` | class declaration | `src/controls/showcase_controls.cpp:914` |
| `ToggleDockEdgeClick` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:915` |
| `SwapDockOrderClick` | `weak_ptr` | `Panel` | local/signature/use | `src/controls/showcase_controls.cpp:920` |
| `SwapDockOrderClick` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:921` |
| `SwapDockOrderClick` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:922` |
| `SwapDockOrderClick` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:923` |
| `SwapDockOrderClick` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:924` |
| `SwapDockOrderClick` | `shared_ptr` | `bool` | local/signature/use | `src/controls/showcase_controls.cpp:925` |
| `SwapDockOrderClick` | `shared_ptr` | `Panel` | local/signature/use | `src/controls/showcase_controls.cpp:932` |
| `SwapDockOrderClick` | `shared_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:933` |
| `SwapDockOrderClick` | `shared_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:934` |
| `SwapDockOrderClick` | `shared_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:935` |
| `SwapDockOrderClick` | `shared_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:938` |
| `SwapDockOrderClick` | `shared_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:939` |
| `SwapDockOrderClick` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:948` |
| `SwapDockOrderClick` | `weak_ptr` | `Panel` | class declaration | `src/controls/showcase_controls.cpp:956` |
| `SwapDockOrderClick` | `weak_ptr` | `Button` | class declaration | `src/controls/showcase_controls.cpp:957` |
| `SwapDockOrderClick` | `weak_ptr` | `Button` | class declaration | `src/controls/showcase_controls.cpp:958` |
| `SwapDockOrderClick` | `weak_ptr` | `Button` | class declaration | `src/controls/showcase_controls.cpp:959` |
| `SwapDockOrderClick` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:960` |
| `SwapDockOrderClick` | `shared_ptr` | `bool` | class declaration | `src/controls/showcase_controls.cpp:961` |
| `ResizeAnchorSpecimenClick` | `weak_ptr` | `ScaledGroupBox` | local/signature/use | `src/controls/showcase_controls.cpp:967` |
| `ResizeAnchorSpecimenClick` | `weak_ptr` | `Panel` | local/signature/use | `src/controls/showcase_controls.cpp:968` |
| `ResizeAnchorSpecimenClick` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:969` |
| `ResizeAnchorSpecimenClick` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:970` |
| `ResizeAnchorSpecimenClick` | `shared_ptr` | `bool` | local/signature/use | `src/controls/showcase_controls.cpp:971` |
| `ResizeAnchorSpecimenClick` | `shared_ptr` | `ScaledGroupBox` | local/signature/use | `src/controls/showcase_controls.cpp:978` |
| `ResizeAnchorSpecimenClick` | `shared_ptr` | `Panel` | local/signature/use | `src/controls/showcase_controls.cpp:979` |
| `ResizeAnchorSpecimenClick` | `shared_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:980` |
| `ResizeAnchorSpecimenClick` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:988` |
| `ResizeAnchorSpecimenClick` | `weak_ptr` | `ScaledGroupBox` | class declaration | `src/controls/showcase_controls.cpp:997` |
| `ResizeAnchorSpecimenClick` | `weak_ptr` | `Panel` | class declaration | `src/controls/showcase_controls.cpp:998` |
| `ResizeAnchorSpecimenClick` | `weak_ptr` | `Button` | class declaration | `src/controls/showcase_controls.cpp:999` |
| `ResizeAnchorSpecimenClick` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:1000` |
| `ResizeAnchorSpecimenClick` | `shared_ptr` | `bool` | class declaration | `src/controls/showcase_controls.cpp:1001` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:1012` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `src/controls/showcase_controls.cpp:1017` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:1024` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:1029` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:1033` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:1037` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:1042` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:1047` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:1057` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:1061` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1064` |
| `<file/function>` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:1072` |
| `<file/function>` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:1073` |
| `<file/function>` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1074` |
| `<file/function>` | `weak_ptr` | `Panel` | local/signature/use | `src/controls/showcase_controls.cpp:1077` |
| `<file/function>` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:1078` |
| `<file/function>` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:1079` |
| `<file/function>` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:1080` |
| `<file/function>` | `shared_ptr` | `bool` | local/signature/use | `src/controls/showcase_controls.cpp:1081` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:1087` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `src/controls/showcase_controls.cpp:1092` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:1099` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:1102` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:1108` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:1112` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:1122` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1126` |
| `<file/function>` | `weak_ptr` | `ScaledGroupBox` | local/signature/use | `src/controls/showcase_controls.cpp:1134` |
| `<file/function>` | `weak_ptr` | `Panel` | local/signature/use | `src/controls/showcase_controls.cpp:1135` |
| `<file/function>` | `weak_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:1136` |
| `<file/function>` | `shared_ptr` | `bool` | local/signature/use | `src/controls/showcase_controls.cpp:1137` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:1144` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1145` |
| `ToggleMotionPauseClick` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1149` |
| `ToggleMotionPauseClick` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1154` |
| `ToggleMotionPauseClick` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:1161` |
| `SetReducedMotion` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1166` |
| `SetReducedMotion` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1171` |
| `SetReducedMotion` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:1178` |
| `<file/function>` | `shared_ptr` | `gui_forms::EasingPreview` | local/signature/use | `src/controls/showcase_controls.cpp:1186` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:1202` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:1205` |
| `<file/function>` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1209` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1212` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `src/controls/showcase_controls.cpp:1216` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:1224` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1225` |
| `AppendDispatcherCharacter` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1229` |
| `AppendDispatcherCharacter` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1235` |
| `AppendDispatcherCharacter` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:1241` |
| `CompleteDispatcherTurnTwo` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1248` |
| `CompleteDispatcherTurnTwo` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1253` |
| `CompleteDispatcherTurnTwo` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:1265` |
| `PostNestedDispatcherTurn` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1271` |
| `PostNestedDispatcherTurn` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1276` |
| `PostNestedDispatcherTurn` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:1285` |
| `CompleteDispatcherTurnOne` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1291` |
| `CompleteDispatcherTurnOne` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1296` |
| `CompleteDispatcherTurnOne` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:1305` |
| `PostDispatcherBatchClick` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1311` |
| `PostDispatcherBatchClick` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1316` |
| `PostDispatcherBatchClick` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:1333` |
| `UnexpectedCancelledDispatch` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1339` |
| `UnexpectedCancelledDispatch` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1344` |
| `UnexpectedCancelledDispatch` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:1351` |
| `CancelDispatcherClick` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1357` |
| `CancelDispatcherClick` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1362` |
| `CancelDispatcherClick` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:1376` |
| `CompleteWorkerInvocation` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1382` |
| `CompleteWorkerInvocation` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1387` |
| `CompleteWorkerInvocation` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:1399` |
| `DispatcherWorker` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1404` |
| `DispatcherWorker` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1409` |
| `DispatcherWorker` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:1420` |
| `InvokeDispatcherFromWorkerClick` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1426` |
| `InvokeDispatcherFromWorkerClick` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1431` |
| `InvokeDispatcherFromWorkerClick` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:1443` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `src/controls/showcase_controls.cpp:1457` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1463` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1467` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:1473` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `src/controls/showcase_controls.cpp:1476` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `src/controls/showcase_controls.cpp:1479` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:1484` |
| `<file/function>` | `shared_ptr` | `gui_forms::LinkLabel` | local/signature/use | `src/controls/showcase_controls.cpp:1488` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1492` |
| `<file/function>` | `shared_ptr` | `gui_forms::MetricsView` | local/signature/use | `src/controls/showcase_controls.cpp:1497` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:1506` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:1510` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:1513` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:1515` |
| `<file/function>` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1529` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:1538` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1539` |
| `TextStateUpdater` | `weak_ptr` | `TextBox` | local/signature/use | `src/controls/showcase_controls.cpp:1542` |
| `TextStateUpdater` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1543` |
| `TextStateUpdater` | `shared_ptr` | `TextBox` | local/signature/use | `src/controls/showcase_controls.cpp:1552` |
| `TextStateUpdater` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1553` |
| `TextStateUpdater` | `weak_ptr` | `TextBox` | class declaration | `src/controls/showcase_controls.cpp:1563` |
| `TextStateUpdater` | `weak_ptr` | `Label` | class declaration | `src/controls/showcase_controls.cpp:1564` |
| `TextCommandClick` | `weak_ptr` | `TextBox` | local/signature/use | `src/controls/showcase_controls.cpp:1579` |
| `TextCommandClick` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1580` |
| `TextCommandClick` | `shared_ptr` | `TextBox` | local/signature/use | `src/controls/showcase_controls.cpp:1588` |
| `TextCommandClick` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1620` |
| `TextCommandClick` | `weak_ptr` | `TextBox` | class declaration | `src/controls/showcase_controls.cpp:1629` |
| `TextCommandClick` | `weak_ptr` | `Label` | class declaration | `src/controls/showcase_controls.cpp:1630` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:1640` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `src/controls/showcase_controls.cpp:1645` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `src/controls/showcase_controls.cpp:1652` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `src/controls/showcase_controls.cpp:1659` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `src/controls/showcase_controls.cpp:1667` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1675` |
| `<file/function>` | `weak_ptr` | `TextBox` | local/signature/use | `src/controls/showcase_controls.cpp:1678` |
| `<file/function>` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1679` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:1685` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:1692` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:1695` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1712` |
| `<file/function>` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1717` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1727` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:1735` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1736` |
| `ProfileSelectionChanged` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1739` |
| `ProfileSelectionChanged` | `weak_ptr` | `ComboBox` | local/signature/use | `src/controls/showcase_controls.cpp:1740` |
| `ProfileSelectionChanged` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1745` |
| `ProfileSelectionChanged` | `shared_ptr` | `ComboBox` | local/signature/use | `src/controls/showcase_controls.cpp:1746` |
| `ProfileSelectionChanged` | `weak_ptr` | `Label` | class declaration | `src/controls/showcase_controls.cpp:1754` |
| `ProfileSelectionChanged` | `weak_ptr` | `ComboBox` | class declaration | `src/controls/showcase_controls.cpp:1755` |
| `DensityPopupChanged` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1760` |
| `DensityPopupChanged` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1765` |
| `DensityPopupChanged` | `weak_ptr` | `Label` | class declaration | `src/controls/showcase_controls.cpp:1773` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:1782` |
| `<file/function>` | `shared_ptr` | `gui_forms::ListBox` | local/signature/use | `src/controls/showcase_controls.cpp:1785` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:1794` |
| `<file/function>` | `shared_ptr` | `gui_forms::ListBox` | local/signature/use | `src/controls/showcase_controls.cpp:1797` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:1808` |
| `<file/function>` | `shared_ptr` | `gui_forms::ComboBox` | local/signature/use | `src/controls/showcase_controls.cpp:1811` |
| `<file/function>` | `shared_ptr` | `gui_forms::ComboBox` | local/signature/use | `src/controls/showcase_controls.cpp:1817` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1822` |
| `<file/function>` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1826` |
| `<file/function>` | `weak_ptr` | `ComboBox` | local/signature/use | `src/controls/showcase_controls.cpp:1827` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:1834` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:1835` |
| `PropertySpecimenClick` | `weak_ptr` | `PropertyGrid` | local/signature/use | `src/controls/showcase_controls.cpp:1845` |
| `PropertySpecimenClick` | `weak_ptr` | `NumericUpDown` | local/signature/use | `src/controls/showcase_controls.cpp:1846` |
| `PropertySpecimenClick` | `weak_ptr` | `ComboBox` | local/signature/use | `src/controls/showcase_controls.cpp:1847` |
| `PropertySpecimenClick` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1848` |
| `PropertySpecimenClick` | `shared_ptr` | `PropertyGrid` | local/signature/use | `src/controls/showcase_controls.cpp:1856` |
| `PropertySpecimenClick` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1877` |
| `PropertySpecimenClick` | `shared_ptr` | `NumericUpDown` | local/signature/use | `src/controls/showcase_controls.cpp:1884` |
| `PropertySpecimenClick` | `shared_ptr` | `ComboBox` | local/signature/use | `src/controls/showcase_controls.cpp:1894` |
| `PropertySpecimenClick` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1907` |
| `PropertySpecimenClick` | `shared_ptr` | `ComboBox` | local/signature/use | `src/controls/showcase_controls.cpp:1917` |
| `PropertySpecimenClick` | `weak_ptr` | `PropertyGrid` | class declaration | `src/controls/showcase_controls.cpp:1936` |
| `PropertySpecimenClick` | `weak_ptr` | `NumericUpDown` | class declaration | `src/controls/showcase_controls.cpp:1937` |
| `PropertySpecimenClick` | `weak_ptr` | `ComboBox` | class declaration | `src/controls/showcase_controls.cpp:1938` |
| `PropertySpecimenClick` | `weak_ptr` | `Label` | class declaration | `src/controls/showcase_controls.cpp:1939` |
| `NumericValueStatus` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1945` |
| `NumericValueStatus` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1951` |
| `NumericValueStatus` | `weak_ptr` | `Label` | class declaration | `src/controls/showcase_controls.cpp:1960` |
| `PropertyValueStatus` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1966` |
| `PropertyValueStatus` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:1971` |
| `PropertyValueStatus` | `weak_ptr` | `Label` | class declaration | `src/controls/showcase_controls.cpp:1981` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:1989` |
| `<file/function>` | `shared_ptr` | `NumericUpDown` | local/signature/use | `src/controls/showcase_controls.cpp:1995` |
| `<file/function>` | `shared_ptr` | `gui_forms::NumericUpDown` | local/signature/use | `src/controls/showcase_controls.cpp:2000` |
| `<file/function>` | `shared_ptr` | `gui_forms::PropertyGrid` | local/signature/use | `src/controls/showcase_controls.cpp:2025` |
| `<file/function>` | `shared_ptr` | `gui_forms::ComboBox` | local/signature/use | `src/controls/showcase_controls.cpp:2030` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:2036` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:2038` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:2040` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:2042` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2053` |
| `<file/function>` | `weak_ptr` | `PropertyGrid` | local/signature/use | `src/controls/showcase_controls.cpp:2059` |
| `<file/function>` | `weak_ptr` | `NumericUpDown` | local/signature/use | `src/controls/showcase_controls.cpp:2060` |
| `<file/function>` | `weak_ptr` | `ComboBox` | local/signature/use | `src/controls/showcase_controls.cpp:2061` |
| `<file/function>` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2062` |
| `<file/function>` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2080` |
| `<file/function>` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2084` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:2089` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:2119` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:2120` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:2127` |
| `<file/function>` | `shared_ptr` | `gui_forms::PictureBox` | local/signature/use | `src/controls/showcase_controls.cpp:2139` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2148` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2153` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:2163` |
| `<file/function>` | `shared_ptr` | `gui_forms::PictureBox` | local/signature/use | `src/controls/showcase_controls.cpp:2168` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2180` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:2191` |
| `<file/function>` | `shared_ptr` | `gui_forms::DrawingSurface` | local/signature/use | `src/controls/showcase_controls.cpp:2194` |
| `<file/function>` | `shared_ptr` | `gui_drawing::Bitmap` | local/signature/use | `src/controls/showcase_controls.cpp:2202` |
| `<file/function>` | `shared_ptr` | `gui_forms::RasterCanvas` | local/signature/use | `src/controls/showcase_controls.cpp:2211` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2218` |
| `<file/function>` | `shared_ptr` | `gui_forms::MaterialPanel` | local/signature/use | `src/controls/showcase_controls.cpp:2223` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2229` |
| `<file/function>` | `shared_ptr` | `gui_forms::MaterialPanel` | local/signature/use | `src/controls/showcase_controls.cpp:2232` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2238` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:2244` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:2245` |
| `PrimaryTabSelectionChanged` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2248` |
| `PrimaryTabSelectionChanged` | `weak_ptr` | `TabControl` | local/signature/use | `src/controls/showcase_controls.cpp:2249` |
| `PrimaryTabSelectionChanged` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2254` |
| `PrimaryTabSelectionChanged` | `shared_ptr` | `TabControl` | local/signature/use | `src/controls/showcase_controls.cpp:2255` |
| `PrimaryTabSelectionChanged` | `weak_ptr` | `Label` | class declaration | `src/controls/showcase_controls.cpp:2264` |
| `PrimaryTabSelectionChanged` | `weak_ptr` | `TabControl` | class declaration | `src/controls/showcase_controls.cpp:2265` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:2274` |
| `<file/function>` | `shared_ptr` | `gui_forms::TabControl` | local/signature/use | `src/controls/showcase_controls.cpp:2278` |
| `<file/function>` | `shared_ptr` | `gui_forms::TabPage` | local/signature/use | `src/controls/showcase_controls.cpp:2286` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2291` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2297` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:2310` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `src/controls/showcase_controls.cpp:2316` |
| `<file/function>` | `shared_ptr` | `gui_forms::RadioButton` | local/signature/use | `src/controls/showcase_controls.cpp:2324` |
| `<file/function>` | `shared_ptr` | `gui_forms::ProgressBar` | local/signature/use | `src/controls/showcase_controls.cpp:2331` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2343` |
| `<file/function>` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2347` |
| `<file/function>` | `weak_ptr` | `TabControl` | local/signature/use | `src/controls/showcase_controls.cpp:2348` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:2352` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2363` |
| `<file/function>` | `shared_ptr` | `gui_forms::TabControl` | local/signature/use | `src/controls/showcase_controls.cpp:2368` |
| `<file/function>` | `shared_ptr` | `gui_forms::TabPage` | local/signature/use | `src/controls/showcase_controls.cpp:2378` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2382` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:2395` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:2396` |
| `CheckedItemStatus` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2399` |
| `CheckedItemStatus` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2404` |
| `CheckedItemStatus` | `weak_ptr` | `Label` | class declaration | `src/controls/showcase_controls.cpp:2415` |
| `CheckedListCommandClick` | `weak_ptr` | `CheckedListBox` | local/signature/use | `src/controls/showcase_controls.cpp:2426` |
| `CheckedListCommandClick` | `shared_ptr` | `CheckedListBox` | local/signature/use | `src/controls/showcase_controls.cpp:2432` |
| `CheckedListCommandClick` | `weak_ptr` | `CheckedListBox` | class declaration | `src/controls/showcase_controls.cpp:2447` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:2457` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckedListBox` | local/signature/use | `src/controls/showcase_controls.cpp:2461` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2471` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:2478` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckedListBox` | local/signature/use | `src/controls/showcase_controls.cpp:2482` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2496` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:2503` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:2506` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:2508` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:2510` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2516` |
| `<file/function>` | `weak_ptr` | `CheckedListBox` | local/signature/use | `src/controls/showcase_controls.cpp:2522` |
| `<file/function>` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2523` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:2537` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:2538` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:2545` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `src/controls/showcase_controls.cpp:2581` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2592` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:2599` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:2602` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:2604` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:2606` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2622` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2628` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2644` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:2651` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:2652` |
| `DateValueStatus` | `weak_ptr` | `DateTimePicker` | local/signature/use | `src/controls/showcase_controls.cpp:2655` |
| `DateValueStatus` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2656` |
| `DateValueStatus` | `shared_ptr` | `DateTimePicker` | local/signature/use | `src/controls/showcase_controls.cpp:2661` |
| `DateValueStatus` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2662` |
| `DateValueStatus` | `weak_ptr` | `DateTimePicker` | class declaration | `src/controls/showcase_controls.cpp:2671` |
| `DateValueStatus` | `weak_ptr` | `Label` | class declaration | `src/controls/showcase_controls.cpp:2672` |
| `DatePopupStatus` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2677` |
| `DatePopupStatus` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2682` |
| `DatePopupStatus` | `weak_ptr` | `Label` | class declaration | `src/controls/showcase_controls.cpp:2690` |
| `OptionalDateStatus` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2695` |
| `OptionalDateStatus` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2700` |
| `OptionalDateStatus` | `weak_ptr` | `Label` | class declaration | `src/controls/showcase_controls.cpp:2708` |
| `DateCommandClick` | `weak_ptr` | `DateTimePicker` | local/signature/use | `src/controls/showcase_controls.cpp:2719` |
| `DateCommandClick` | `shared_ptr` | `DateTimePicker` | local/signature/use | `src/controls/showcase_controls.cpp:2725` |
| `DateCommandClick` | `weak_ptr` | `DateTimePicker` | class declaration | `src/controls/showcase_controls.cpp:2741` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:2753` |
| `<file/function>` | `shared_ptr` | `gui_forms::DateTimePicker` | local/signature/use | `src/controls/showcase_controls.cpp:2766` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:2782` |
| `<file/function>` | `shared_ptr` | `gui_forms::DateTimePicker` | local/signature/use | `src/controls/showcase_controls.cpp:2787` |
| `<file/function>` | `shared_ptr` | `gui_forms::DateTimePicker` | local/signature/use | `src/controls/showcase_controls.cpp:2799` |
| `<file/function>` | `shared_ptr` | `gui_forms::DateTimePicker` | local/signature/use | `src/controls/showcase_controls.cpp:2810` |
| `<file/function>` | `shared_ptr` | `gui_forms::DateTimePicker` | local/signature/use | `src/controls/showcase_controls.cpp:2820` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:2832` |
| `<file/function>` | `shared_ptr` | `gui_forms::DateTimePicker` | local/signature/use | `src/controls/showcase_controls.cpp:2836` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:2844` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:2846` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:2847` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2852` |
| `<file/function>` | `weak_ptr` | `DateTimePicker` | local/signature/use | `src/controls/showcase_controls.cpp:2859` |
| `<file/function>` | `weak_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:2860` |
| `<file/function>` | `shared_ptr` | `Surface` | local/signature/use | `src/controls/showcase_controls.cpp:2876` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:2877` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:2885` |
| `DialogResultPublisher` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:2899` |
| `DialogResultPublisher` | `shared_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:2965` |
| `DialogInvoker` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:2970` |
| `DialogInvoker` | `shared_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:2993` |
| `HostServiceClick` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3066` |
| `HostServiceClick` | `shared_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:3147` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:3167` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:3178` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:3185` |
| `<file/function>` | `shared_ptr` | `LayoutGroup` | local/signature/use | `src/controls/showcase_controls.cpp:3199` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:3214` |
| `NavigationClick` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3238` |
| `NavigationClick` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3244` |
| `NavigationClick` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:3250` |
| `LiveMotionChanged` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3257` |
| `LiveMotionChanged` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3262` |
| `LiveMotionChanged` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:3269` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3272` |
| `<file/function>` | `shared_ptr` | `gui_forms::ScaledPanel` | local/signature/use | `src/controls/showcase_controls.cpp:3273` |
| `<file/function>` | `shared_ptr` | `gui_forms::ScaledPanel` | local/signature/use | `src/controls/showcase_controls.cpp:3278` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `src/controls/showcase_controls.cpp:3282` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `src/controls/showcase_controls.cpp:3285` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:3288` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:3292` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:3296` |
| `<file/function>` | `shared_ptr` | `gui_forms::ScaledPanel` | local/signature/use | `src/controls/showcase_controls.cpp:3301` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `src/controls/showcase_controls.cpp:3305` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:3308` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:3319` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `src/controls/showcase_controls.cpp:3329` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `src/controls/showcase_controls.cpp:3335` |
| `<file/function>` | `shared_ptr` | `gui_forms::ScaledPanel` | local/signature/use | `src/controls/showcase_controls.cpp:3341` |
| `<file/function>` | `shared_ptr` | `gui_forms::ScaledPanel` | local/signature/use | `src/controls/showcase_controls.cpp:3346` |
| `<file/function>` | `shared_ptr` | `gui_forms::ScaledPanel` | local/signature/use | `src/controls/showcase_controls.cpp:3379` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `src/controls/showcase_controls.cpp:3383` |
| `<file/function>` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3390` |
| `<file/function>` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3394` |
| `BindingStatusUpdater` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3417` |
| `BindingStatusUpdater` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3422` |
| `BindingStatusUpdater` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:3436` |
| `BindingNavigationClick` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3446` |
| `BindingNavigationClick` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3452` |
| `BindingNavigationClick` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:3463` |
| `ShowcaseHelpRequested` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3470` |
| `ShowcaseHelpRequested` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3475` |
| `ShowcaseHelpRequested` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:3482` |
| `ToggleProviderErrorClick` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3488` |
| `ToggleProviderErrorClick` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3493` |
| `ToggleProviderErrorClick` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:3509` |
| `ShowcaseTimerTick` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3515` |
| `ShowcaseTimerTick` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3520` |
| `ShowcaseTimerTick` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:3540` |
| `TimerIntervalChanged` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3546` |
| `TimerIntervalChanged` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3551` |
| `TimerIntervalChanged` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:3559` |
| `TimerCommandClick` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3569` |
| `TimerCommandClick` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3575` |
| `TimerCommandClick` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:3589` |
| `ShowPersistentTooltipClick` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3596` |
| `ShowPersistentTooltipClick` | `shared_ptr` | `Button` | local/signature/use | `src/controls/showcase_controls.cpp:3597` |
| `ShowPersistentTooltipClick` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3602` |
| `ShowPersistentTooltipClick` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:3608` |
| `ShowPersistentTooltipClick` | `shared_ptr` | `Button` | class declaration | `src/controls/showcase_controls.cpp:3609` |
| `ShowDisabledTooltipClick` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3615` |
| `ShowDisabledTooltipClick` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3620` |
| `ShowDisabledTooltipClick` | `weak_ptr` | `ShowcaseContext` | class declaration | `src/controls/showcase_controls.cpp:3627` |
| `<file/function>` | `shared_ptr` | `gui_forms::ScaledPanel` | local/signature/use | `src/controls/showcase_controls.cpp:3630` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3632` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3633` |
| `<file/function>` | `shared_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3637` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/controls/showcase_controls.cpp:3662` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/showcase_controls.cpp:3688` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/controls/showcase_controls.cpp:3689` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:3690` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `src/controls/showcase_controls.cpp:3704` |
| `<file/function>` | `weak_ptr` | `ShowcaseContext` | local/signature/use | `src/controls/showcase_controls.cpp:3710` |
| `ShowcaseTree` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/controls/showcase_controls.hpp:12` |
| `ToolTip` | `weak_ptr` | `Control` | class declaration | `src/controls/tool_tip/tool_tip.cpp:25` |
| `ToolTip` | `weak_ptr` | `Control` | class declaration | `src/controls/tool_tip/tool_tip.cpp:34` |
| `ToolTip` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:37` |
| `ToolTip` | `weak_ptr` | `Control` | class declaration | `src/controls/tool_tip/tool_tip.cpp:44` |
| `ToolTip` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:47` |
| `ToolTip` | `weak_ptr` | `Control` | class declaration | `src/controls/tool_tip/tool_tip.cpp:54` |
| `ToolTip` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:57` |
| `ToolTip` | `weak_ptr` | `detail::WindowLifetime` | class declaration | `src/controls/tool_tip/tool_tip.cpp:64` |
| `ToolTip` | `shared_ptr` | `detail::WindowLifetime` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:67` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::WindowLifetime` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:102` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:128` |
| `<file/function>` | `unique_ptr` | `gui_forms::ToolTip::Entry` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:152` |
| `<file/function>` | `weak_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:155` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:232` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:238` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:250` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:259` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:263` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:293` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:301` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:305` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:318` |
| `<file/function>` | `shared_ptr` | `gui_forms::ToolTipLayer` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:333` |
| `<file/function>` | `shared_ptr` | `gui_forms::ToolTipBubble` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:336` |
| `<file/function>` | `weak_ptr` | `detail::WindowLifetime` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:349` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:363` |
| `<file/function>` | `shared_ptr` | `gui_forms::ToolTipBubble` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:364` |
| `<file/function>` | `shared_ptr` | `Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:396` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:401` |
| `<file/function>` | `unique_ptr` | `PopupHolder` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:408` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:427` |
| `ToolTipBubble` | `shared_ptr` | `Label` | class declaration | `src/controls/tool_tip/tool_tip_bubble/tool_tip_bubble.hpp:20` |
| `<file/function>` | `shared_ptr` | `BindingSource` | local/signature/use | `src/core/binding/binding/binding.cpp:24` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/binding/binding.cpp:51` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/binding/binding.cpp:57` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/binding/binding.cpp:67` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/binding/binding.cpp:74` |
| `<file/function>` | `shared_ptr` | `gui_forms::BindingSource` | local/signature/use | `src/core/binding/binding/binding.cpp:84` |
| `<file/function>` | `weak_ptr` | `Binding` | local/signature/use | `src/core/binding/binding/binding.cpp:92` |
| `<file/function>` | `shared_ptr` | `gui_forms::Binding` | local/signature/use | `src/core/binding/binding/binding.cpp:156` |
| `<file/function>` | `shared_ptr` | `gui_forms::BindingSource` | local/signature/use | `src/core/binding/binding/binding.cpp:157` |
| `<file/function>` | `shared_ptr` | `gui_forms::Binding` | local/signature/use | `src/core/binding/binding/binding.cpp:225` |
| `<file/function>` | `shared_ptr` | `gui_forms::BindingSource` | local/signature/use | `src/core/binding/binding/binding.cpp:226` |
| `<file/function>` | `shared_ptr` | `gui_forms::BindingSource` | local/signature/use | `src/core/binding/binding/binding.cpp:282` |
| `<file/function>` | `shared_ptr` | `gui_forms::BindingSource` | local/signature/use | `src/core/binding/binding/binding.cpp:287` |
| `<file/function>` | `shared_ptr` | `gui_forms::BindingSource` | local/signature/use | `src/core/binding/binding/binding.cpp:303` |
| `<file/function>` | `shared_ptr` | `gui_forms::BindingSource` | local/signature/use | `src/core/binding/binding/binding.cpp:312` |
| `<file/function>` | `shared_ptr` | `BindingSource` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:38` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::WindowLifetime` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:43` |
| `<file/function>` | `shared_ptr` | `BindingSource` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:48` |
| `<file/function>` | `unique_ptr` | `CurrencyManager` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:25` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::WindowLifetime` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:37` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:159` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:161` |
| `<file/function>` | `weak_ptr` | `gui_forms::Binding` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:163` |
| `<file/function>` | `shared_ptr` | `gui_forms::Binding` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:164` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:501` |
| `<file/function>` | `weak_ptr` | `Binding` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:504` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:505` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:514` |
| `<file/function>` | `weak_ptr` | `Binding` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:536` |
| `<file/function>` | `weak_ptr` | `gui_forms::Binding` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:538` |
| `<file/function>` | `shared_ptr` | `gui_forms::Binding` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:539` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/control_bindings_collection/control_bindings_collection.cpp:23` |
| `<file/function>` | `shared_ptr` | `BindingSource` | local/signature/use | `src/core/binding/control_bindings_collection/control_bindings_collection.cpp:24` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/control_bindings_collection/control_bindings_collection.cpp:32` |
| `<file/function>` | `shared_ptr` | `BindingSource` | local/signature/use | `src/core/binding/control_bindings_collection/control_bindings_collection.cpp:33` |
| `<file/function>` | `shared_ptr` | `gui_forms::Binding` | local/signature/use | `src/core/binding/control_bindings_collection/control_bindings_collection.cpp:35` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/control_bindings_collection/control_bindings_collection.cpp:42` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/control_bindings_collection/control_bindings_collection.cpp:65` |
| `<file/function>` | `shared_ptr` | `gui_forms::Binding` | local/signature/use | `src/core/binding/control_bindings_collection/control_bindings_collection.cpp:73` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/control_bindings_collection/control_bindings_collection.cpp:80` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `src/core/binding/control_bindings_collection/control_bindings_collection.cpp:88` |
| `PropertyValueFactoryAccess` | `shared_ptr` | `const PropertyObjectData` | local/signature/use | `src/core/binding/value/binding_value.cpp:37` |
| `PropertyValueFactoryAccess` | `shared_ptr` | `const PropertyCollectionData` | local/signature/use | `src/core/binding/value/binding_value.cpp:41` |
| `<file/function>` | `shared_ptr` | `gui_forms::PropertyObjectData` | local/signature/use | `src/core/binding/value/binding_value.cpp:800` |
| `<file/function>` | `shared_ptr` | `gui_forms::PropertyCollectionData` | local/signature/use | `src/core/binding/value/binding_value.cpp:827` |
| `ExpiredRevocable` | `weak_ptr` | `detail::Revocable` | local/signature/use | `src/core/component/component/component.cpp:12` |
| `<file/function>` | `weak_ptr` | `detail::Revocable` | local/signature/use | `src/core/component/component/component.cpp:33` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::Revocable` | local/signature/use | `src/core/component/component/component.cpp:35` |
| `<file/function>` | `weak_ptr` | `gui_forms::detail::Revocable` | local/signature/use | `src/core/component/component/component.cpp:52` |
| `<file/function>` | `weak_ptr` | `gui_forms::detail::Revocable` | local/signature/use | `src/core/component/component/component.cpp:55` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::Revocable` | local/signature/use | `src/core/component/component/component.cpp:58` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Component` | local/signature/use | `src/core/component/component_container/component_container.cpp:13` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Component` | local/signature/use | `src/core/component/component_container/component_container.cpp:27` |
| `<file/function>` | `shared_ptr` | `Component` | local/signature/use | `src/core/component/component_container/component_container.cpp:35` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Component` | local/signature/use | `src/core/component/component_container/component_container.cpp:41` |
| `<file/function>` | `shared_ptr` | `gui_forms::Component` | local/signature/use | `src/core/component/component_container/component_container.cpp:52` |
| `<file/function>` | `shared_ptr` | `gui_forms::Component` | local/signature/use | `src/core/component/component_container/component_container.cpp:53` |
| `<file/function>` | `shared_ptr` | `const PropertyEnumDescriptor` | local/signature/use | `src/core/control/control/control.cpp:53` |
| `<file/function>` | `shared_ptr` | `const gui_forms::PropertyEnumDescriptor` | local/signature/use | `src/core/control/control/control.cpp:54` |
| `<file/function>` | `shared_ptr` | `const PropertyEnumDescriptor` | local/signature/use | `src/core/control/control/control.cpp:63` |
| `<file/function>` | `shared_ptr` | `const gui_forms::PropertyEnumDescriptor` | local/signature/use | `src/core/control/control/control.cpp:64` |
| `<file/function>` | `shared_ptr` | `const PropertyEnumDescriptor` | local/signature/use | `src/core/control/control/control.cpp:73` |
| `<file/function>` | `shared_ptr` | `const gui_forms::PropertyEnumDescriptor` | local/signature/use | `src/core/control/control/control.cpp:74` |
| `<file/function>` | `shared_ptr` | `const PropertyEnumDescriptor` | local/signature/use | `src/core/control/control/control.cpp:83` |
| `<file/function>` | `shared_ptr` | `const PropertyEnumDescriptor` | local/signature/use | `src/core/control/control/control.cpp:147` |
| `<file/function>` | `shared_ptr` | `const PropertyEnumDescriptor` | local/signature/use | `src/core/control/control/control.cpp:152` |
| `<file/function>` | `shared_ptr` | `const PropertyEnumDescriptor` | local/signature/use | `src/core/control/control/control.cpp:157` |
| `<file/function>` | `shared_ptr` | `const PropertyEnumDescriptor` | local/signature/use | `src/core/control/control/control.cpp:163` |
| `<file/function>` | `shared_ptr` | `const Theme` | local/signature/use | `src/core/control/control/control.cpp:510` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/control/control/control.cpp:998` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/control/control/control.cpp:1034` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/control/control/control.cpp:1438` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/control/control/control.cpp:1466` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/control/control/control.cpp:1759` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/control/control/control.cpp:1782` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/control/control/control.cpp:1830` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/control/control/control.cpp:2073` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/control/control/control.cpp:2324` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/control/control/control.cpp:2339` |
| `<file/function>` | `shared_ptr` | `detail::DispatcherState` | local/signature/use | `src/core/control/dispatcher/control_dispatcher.cpp:14` |
| `<file/function>` | `shared_ptr` | `detail::DispatcherState` | local/signature/use | `src/core/control/dispatcher/control_dispatcher.cpp:26` |
| `<file/function>` | `shared_ptr` | `detail::DispatcherState` | local/signature/use | `src/core/control/dispatcher/control_dispatcher.cpp:35` |
| `<file/function>` | `shared_ptr` | `detail::DispatcherState` | local/signature/use | `src/core/control/dispatcher/control_dispatcher.cpp:48` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/control/static_tree/control_factory/control_factory.cpp:21` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/control/static_tree/control_factory/control_factory.cpp:26` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/control/static_tree/control_factory/control_factory.cpp:33` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/control/static_tree/control_factory/control_factory.cpp:34` |
| `DispatchWork` | `qualified-alias:weak_ptr` | `Control` | class declaration | `src/core/dispatcher/state/dispatcher_state.hpp:21` |
| `DispatchWork` | `weak_ptr` | `DispatcherState` | class declaration | `src/core/dispatcher/state/dispatcher_state.hpp:24` |
| `DispatcherState` | `shared_ptr` | `DispatchWork` | class declaration | `src/core/dispatcher/state/dispatcher_state.hpp:32` |
| `<file/function>` | `shared_ptr` | `DispatcherState` | local/signature/use | `src/core/dispatcher/state/dispatcher_state.hpp:56` |
| `<file/function>` | `qualified-alias:weak_ptr` | `Control` | local/signature/use | `src/core/dispatcher/state/dispatcher_state.hpp:57` |
| `<file/function>` | `shared_ptr` | `DispatcherState` | local/signature/use | `src/core/dispatcher/state/post_dispatch.cpp:13` |
| `<file/function>` | `qualified-alias:weak_ptr` | `Control` | local/signature/use | `src/core/dispatcher/state/post_dispatch.cpp:14` |
| `<file/function>` | `shared_ptr` | `DispatchWork` | local/signature/use | `src/core/dispatcher/state/post_dispatch.cpp:27` |
| `DisplayCommand` | `shared_ptr` | `LiveSurface` | class declaration | `src/core/display/command/display_command.hpp:44` |
| `<file/function>` | `shared_ptr` | `LiveSurface` | local/signature/use | `src/core/display/recording_painter/recording_painter.cpp:193` |
| `<file/function>` | `shared_ptr` | `const DisplayChunk` | local/signature/use | `src/core/display/recording_painter/recording_painter.cpp:243` |
| `RecordingPainter` | `shared_ptr` | `LiveSurface` | class declaration | `src/core/display/recording_painter/recording_painter.hpp:39` |
| `RecordingPainter` | `shared_ptr` | `const DisplayChunk` | local/signature/use | `src/core/display/recording_painter/recording_painter.hpp:50` |
| `<file/function>` | `unique_ptr` | `Bitmap` | local/signature/use | `src/core/drawing/bitmap/bitmap.cpp:129` |
| `<file/function>` | `unique_ptr` | `gui_drawing::Bitmap` | local/signature/use | `src/core/drawing/bitmap/bitmap.cpp:136` |
| `<file/function>` | `unique_ptr` | `Bitmap` | local/signature/use | `src/core/drawing/bitmap/bitmap.cpp:151` |
| `<file/function>` | `unique_ptr` | `gui_drawing::Bitmap` | local/signature/use | `src/core/drawing/bitmap/bitmap.cpp:155` |
| `<file/function>` | `unique_ptr` | `Bitmap` | local/signature/use | `src/core/drawing/bitmap/bitmap.cpp:173` |
| `<file/function>` | `unique_ptr` | `gui_drawing::Bitmap` | local/signature/use | `src/core/drawing/bitmap/bitmap.cpp:178` |
| `<file/function>` | `unique_ptr` | `GraphicsPath` | local/signature/use | `src/core/drawing/graphics_path/graphics_path.cpp:381` |
| `<file/function>` | `unique_ptr` | `gui_drawing::GraphicsPath` | local/signature/use | `src/core/drawing/graphics_path/graphics_path.cpp:383` |
| `<file/function>` | `unique_ptr` | `ImageAttributes` | local/signature/use | `src/core/drawing/image_attributes/image_attributes.cpp:47` |
| `<file/function>` | `unique_ptr` | `gui_drawing::ImageAttributes` | local/signature/use | `src/core/drawing/image_attributes/image_attributes.cpp:49` |
| `<file/function>` | `unique_ptr` | `TextureBrush` | local/signature/use | `src/core/drawing/texture_brush/texture_brush.cpp:60` |
| `<file/function>` | `unique_ptr` | `TextureBrush` | local/signature/use | `src/core/drawing/texture_brush/texture_brush.cpp:62` |
| `<file/function>` | `shared_ptr` | `LiveSurfaceBuffer` | local/signature/use | `src/core/live_surface/buffer/live_surface_buffer.cpp:29` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::LiveSurfaceBuffer` | local/signature/use | `src/core/live_surface/buffer/live_surface_buffer.cpp:37` |
| `<file/function>` | `shared_ptr` | `LiveSurfaceBuffer` | local/signature/use | `src/core/live_surface/buffer/live_surface_buffer.hpp:9` |
| `<file/function>` | `shared_ptr` | `const detail::LiveSurfaceBuffer` | local/signature/use | `src/core/live_surface/frame/live_surface_frame.cpp:10` |
| `LiveSurfaceState` | `shared_ptr` | `LiveSurfaceBuffer` | class declaration | `src/core/live_surface/state/live_surface_state.hpp:28` |
| `LiveSurfaceState` | `shared_ptr` | `LiveSurfaceWake` | class declaration | `src/core/live_surface/state/live_surface_state.hpp:39` |
| `<file/function>` | `shared_ptr` | `detail::LiveSurfaceState` | local/signature/use | `src/core/live_surface/surface/live_surface.cpp:13` |
| `<file/function>` | `shared_ptr` | `LiveSurface` | local/signature/use | `src/core/live_surface/surface/live_surface.cpp:18` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::LiveSurfaceState` | local/signature/use | `src/core/live_surface/surface/live_surface.cpp:21` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::LiveSurfaceBuffer` | local/signature/use | `src/core/live_surface/surface/live_surface.cpp:26` |
| `<file/function>` | `shared_ptr` | `LiveSurface` | local/signature/use | `src/core/live_surface/surface/live_surface.cpp:30` |
| `<file/function>` | `shared_ptr` | `detail::LiveSurfaceBuffer` | local/signature/use | `src/core/live_surface/surface/live_surface.cpp:37` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::LiveSurfaceBuffer` | local/signature/use | `src/core/live_surface/surface/live_surface.cpp:39` |
| `<file/function>` | `shared_ptr` | `detail::LiveSurfaceWake` | local/signature/use | `src/core/live_surface/surface/live_surface.cpp:43` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::LiveSurfaceWake` | local/signature/use | `src/core/live_surface/surface/live_surface.cpp:56` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::LiveSurfaceWake` | local/signature/use | `src/core/live_surface/surface/live_surface.cpp:117` |
| `DisconnectedOrMatchingWake` | `shared_ptr` | `detail::LiveSurfaceWake` | local/signature/use | `src/core/live_surface/wake_connection/live_surface_wake_connection.cpp:16` |
| `<file/function>` | `weak_ptr` | `detail::LiveSurfaceState` | local/signature/use | `src/core/live_surface/wake_connection/live_surface_wake_connection.cpp:24` |
| `<file/function>` | `shared_ptr` | `detail::LiveSurfaceWake` | local/signature/use | `src/core/live_surface/wake_connection/live_surface_wake_connection.cpp:25` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::LiveSurfaceState` | local/signature/use | `src/core/live_surface/wake_connection/live_surface_wake_connection.cpp:51` |
| `<file/function>` | `shared_ptr` | `detail::LiveSurfaceState` | local/signature/use | `src/core/live_surface/write_lease/live_surface_write_lease.cpp:13` |
| `<file/function>` | `shared_ptr` | `detail::LiveSurfaceBuffer` | local/signature/use | `src/core/live_surface/write_lease/live_surface_write_lease.cpp:14` |
| `<file/function>` | `shared_ptr` | `detail::LiveSurfaceWake` | local/signature/use | `src/core/live_surface/write_lease/live_surface_write_lease.cpp:57` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::LiveSurfaceWake` | local/signature/use | `src/core/live_surface/write_lease/live_surface_write_lease.cpp:81` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::WindowLifetime` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:50` |
| `<file/function>` | `shared_ptr` | `detail::Revocable` | local/signature/use | `src/core/scheduler/frame_request_token/frame_request_token.cpp:22` |
| `<file/function>` | `qualified-alias:weak_ptr` | `Control` | local/signature/use | `src/core/scheduler/request/scheduled_frame_request.cpp:8` |
| `ScheduledFrameRequest` | `qualified-alias:weak_ptr` | `Control` | class declaration | `src/core/scheduler/request/scheduled_frame_request.hpp:19` |
| `ScheduledFrameRequest` | `qualified-alias:weak_ptr` | `Control` | class declaration | `src/core/scheduler/request/scheduled_frame_request.hpp:30` |
| `<file/function>` | `shared_ptr` | `CallbackState` | local/signature/use | `src/core/timer/timer/timer.cpp:26` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::WindowLifetime` | local/signature/use | `src/core/timer/timer/timer.cpp:33` |
| `<file/function>` | `shared_ptr` | `LiveSurface` | local/signature/use | `src/core/types/painter/painter.cpp:179` |
| `<file/function>` | `shared_ptr` | `detail::DispatcherState` | local/signature/use | `src/core/window/dispatcher/window_dispatcher.cpp:14` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/dispatcher/window_dispatcher.cpp:30` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/dispatcher/window_dispatcher.cpp:54` |
| `<file/function>` | `shared_ptr` | `detail::DispatchWork` | local/signature/use | `src/core/window/dispatcher/window_dispatcher.cpp:84` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::DispatchWork` | local/signature/use | `src/core/window/dispatcher/window_dispatcher.cpp:98` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/dispatcher/window_dispatcher.cpp:110` |
| `<file/function>` | `shared_ptr` | `detail::DispatchWork` | local/signature/use | `src/core/window/dispatcher/window_dispatcher.cpp:205` |
| `<file/function>` | `shared_ptr` | `detail::DispatchWork` | local/signature/use | `src/core/window/dispatcher/window_dispatcher.cpp:237` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::DispatchWork` | local/signature/use | `src/core/window/dispatcher/window_dispatcher.cpp:247` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/lifecycle/window_lifecycle.cpp:21` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::ScheduledFrameRequest` | local/signature/use | `src/core/window/lifecycle/window_lifecycle.cpp:54` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PopupAttachment` | local/signature/use | `src/core/window/lifecycle/window_lifecycle.cpp:78` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/lifecycle/window_lifecycle.cpp:79` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PopupAttachment` | local/signature/use | `src/core/window/lifecycle/window_lifecycle.cpp:97` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/lifecycle/window_lifecycle.cpp:98` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PopupAttachment` | local/signature/use | `src/core/window/lifecycle/window_lifecycle.cpp:127` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/lifecycle/window_lifecycle.cpp:128` |
| `<file/function>` | `shared_ptr` | `const Theme` | local/signature/use | `src/core/window/lifecycle/window_lifecycle.cpp:135` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PopupAttachment` | local/signature/use | `src/core/window/lifecycle/window_lifecycle.cpp:143` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/lifecycle/window_lifecycle.cpp:144` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PopupAttachment` | local/signature/use | `src/core/window/lifecycle/window_lifecycle.cpp:157` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/lifecycle/window_lifecycle.cpp:158` |
| `PopupAttachment` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/core/window/popup/popup_attachment.hpp:9` |
| `PopupAttachment` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/core/window/popup/popup_attachment.hpp:9` |
| `PopupAttachment` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/core/window/popup/popup_attachment.hpp:18` |
| `PopupAttachment` | `qualified-alias:shared_ptr` | `Control` | class declaration | `src/core/window/popup/popup_attachment.hpp:19` |
| `PopupAttachment` | `qualified-alias:weak_ptr` | `Control` | class declaration | `src/core/window/popup/popup_attachment.hpp:32` |
| `PopupAttachment` | `qualified-alias:weak_ptr` | `Control` | class declaration | `src/core/window/popup/popup_attachment.hpp:33` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:17` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:20` |
| `<file/function>` | `shared_ptr` | `detail::PopupAttachment` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:59` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:60` |
| `<file/function>` | `shared_ptr` | `detail::PopupAttachment` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:117` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:157` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PopupAttachment` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:159` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:175` |
| `<file/function>` | `shared_ptr` | `const detail::DisplayChunk` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:191` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:286` |
| `<file/function>` | `shared_ptr` | `LiveSurface` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:286` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:311` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:322` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PopupAttachment` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:365` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:374` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/presentation/window_presentation.cpp:396` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::ScheduledFrameRequest` | local/signature/use | `src/core/window/scheduler/window_scheduler.cpp:17` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/scheduler/window_scheduler.cpp:30` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/scheduler/window_scheduler.cpp:42` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::ScheduledFrameRequest` | local/signature/use | `src/core/window/scheduler/window_scheduler.cpp:52` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/scheduler/window_scheduler.cpp:61` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::ScheduledFrameRequest` | local/signature/use | `src/core/window/scheduler/window_scheduler.cpp:76` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::ScheduledFrameRequest` | local/signature/use | `src/core/window/scheduler/window_scheduler.cpp:102` |
| `<file/function>` | `shared_ptr` | `detail::ScheduledFrameRequest` | local/signature/use | `src/core/window/scheduler/window_scheduler.cpp:135` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::ScheduledFrameRequest` | local/signature/use | `src/core/window/scheduler/window_scheduler.cpp:136` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/scheduler/window_scheduler.cpp:164` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::ScheduledFrameRequest` | local/signature/use | `src/core/window/scheduler/window_scheduler.cpp:230` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::ScheduledFrameRequest` | local/signature/use | `src/core/window/scheduler/window_scheduler.cpp:245` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:44` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:44` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/window.cpp:51` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:71` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:78` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:81` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:91` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:93` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:136` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:145` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:147` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::AcceleratorAttachment` | local/signature/use | `src/core/window/window.cpp:328` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::AcceleratorAttachment` | local/signature/use | `src/core/window/window.cpp:352` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:364` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:365` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PopupAttachment` | local/signature/use | `src/core/window/window.cpp:386` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PopupAttachment` | local/signature/use | `src/core/window/window.cpp:401` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:402` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:403` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:424` |
| `<file/function>` | `shared_ptr` | `detail::PopupAttachment` | local/signature/use | `src/core/window/window.cpp:425` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PopupAttachment` | local/signature/use | `src/core/window/window.cpp:426` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PopupAttachment` | local/signature/use | `src/core/window/window.cpp:431` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:445` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:448` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:451` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:457` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PopupAttachment` | local/signature/use | `src/core/window/window.cpp:460` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:464` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:478` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:487` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:497` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:539` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:546` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:558` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:604` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:605` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:611` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:621` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:626` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:627` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:639` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:644` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:647` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:648` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:664` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:668` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:676` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:690` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:691` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:708` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:722` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:742` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:778` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:799` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:830` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:830` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:832` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:834` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:884` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:885` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:888` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:897` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:911` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:912` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:991` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1010` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1026` |
| `<file/function>` | `shared_ptr` | `unsigned char` | local/signature/use | `src/core/window/window.cpp:1052` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1071` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1083` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1098` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1099` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1107` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1113` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1131` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1144` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1147` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1166` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1183` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1184` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1210` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1222` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1231` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/window.cpp:1233` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1286` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1309` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1315` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/window.cpp:1317` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1363` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1491` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1492` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1500` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1505` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/window.cpp:1507` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1544` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1555` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1603` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1604` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1647` |
| `<file/function>` | `qualified-alias:weak_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1648` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1658` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1663` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1673` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1678` |
| `<file/function>` | `qualified-alias:weak_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1678` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1686` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1703` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1711` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1714` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1723` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1753` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/window.cpp:1773` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1803` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1805` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1815` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1817` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1843` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1858` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1876` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1883` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/window.cpp:1903` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1910` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1920` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1925` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/window.cpp:1929` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1934` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/window.cpp:1936` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1966` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:1976` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2012` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2034` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2061` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2062` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PopupAttachment` | local/signature/use | `src/core/window/window.cpp:2064` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2065` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2091` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PopupAttachment` | local/signature/use | `src/core/window/window.cpp:2098` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2134` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2146` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2264` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/window.cpp:2266` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PopupAttachment` | local/signature/use | `src/core/window/window.cpp:2300` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2301` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PopupAttachment` | local/signature/use | `src/core/window/window.cpp:2313` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2314` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PopupAttachment` | local/signature/use | `src/core/window/window.cpp:2322` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2323` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PopupAttachment` | local/signature/use | `src/core/window/window.cpp:2328` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PopupAttachment` | local/signature/use | `src/core/window/window.cpp:2347` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2348` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2362` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2362` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2363` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2364` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2371` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2371` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2399` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2410` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2413` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2432` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/window.cpp:2433` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2456` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2497` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/window.cpp:2498` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2511` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2517` |
| `<file/function>` | `shared_ptr` | `detail::PopupAttachment` | local/signature/use | `src/core/window/window.cpp:2525` |
| `<file/function>` | `shared_ptr` | `detail::PopupAttachment` | local/signature/use | `src/core/window/window.cpp:2532` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2533` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2550` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2556` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/window.cpp:2558` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2569` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/window.cpp:2571` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2578` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/window.cpp:2581` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2586` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/window.cpp:2589` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2594` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2675` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/window.cpp:2676` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/window.cpp:2694` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2701` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/window.cpp:2703` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::PopupAttachment` | local/signature/use | `src/core/window/window.cpp:2711` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2712` |
| `<file/function>` | `shared_ptr` | `gui_forms::detail::ScheduledFrameRequest` | local/signature/use | `src/core/window/window.cpp:2720` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/core/window/window.cpp:2724` |
| `<file/function>` | `shared_ptr` | `detail::ScheduledFrameRequest` | local/signature/use | `src/core/window/window.cpp:2741` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2747` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2761` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/core/window/window.cpp:2786` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `src/host/macos/application/macos_host.mm:434` |
| `<file/function>` | `unique_ptr` | `HostSession` | local/signature/use | `src/host/macos/application/macos_host.mm:435` |
| `<file/function>` | `unique_ptr` | `HostServices` | local/signature/use | `src/host/macos/application/macos_host.mm:436` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `src/host/macos/application/macos_host.mm:459` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `src/host/macos/application/macos_host.mm:774` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/host/macos/application/macos_host.mm:1512` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `src/host/macos/application/macos_host.mm:1672` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `src/host/macos/application/macos_host.mm:1917` |
| `<file/function>` | `unique_ptr` | `HostServices` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:501` |
| `DibPainter` | `shared_ptr` | `LiveSurface` | local/signature/use | `src/host/windows/application/windows_host.cpp:992` |
| `WindowsHostState` | `unique_ptr` | `Window` | local/signature/use | `src/host/windows/application/windows_host.cpp:1654` |
| `WindowsHostState` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/host/windows/application/windows_host.cpp:2405` |
| `WindowsHostState` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/host/windows/application/windows_host.cpp:2424` |
| `WindowsHostState` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/host/windows/application/windows_host.cpp:2440` |
| `WindowsHostState` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/host/windows/application/windows_host.cpp:2462` |
| `WindowsHostState` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/host/windows/application/windows_host.cpp:2482` |
| `WindowsHostState` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/host/windows/application/windows_host.cpp:2507` |
| `WindowsHostState` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/host/windows/application/windows_host.cpp:2548` |
| `WindowsHostState` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `src/host/windows/application/windows_host.cpp:2593` |
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
| `<file/function>` | `unique_ptr` | `Bitmap` | local/signature/use | `src/render/skia/executor/skia_executor.cpp:692` |
| `<file/function>` | `unique_ptr` | `SkCodec` | local/signature/use | `src/render/skia/executor/skia_executor.cpp:865` |
| `<file/function>` | `unique_ptr` | `gui_drawing::Bitmap` | local/signature/use | `src/render/skia/executor/skia_executor.cpp:878` |
| `DecodeResult` | `unique_ptr` | `Bitmap` | class declaration | `src/render/skia/executor/skia_executor.hpp:38` |
| `SkiaExecutor` | `unique_ptr` | `Impl` | class declaration | `src/render/skia/executor/skia_executor.hpp:81` |
| `<file/function>` | `unique_ptr` | `SkCodec` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:449` |
| `<file/function>` | `shared_ptr` | `LiveSurface` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:752` |
| `SkiaRaster` | `shared_ptr` | `LiveSurface` | class declaration | `src/render/skia/raster/skia_raster.hpp:68` |
| `SkiaRaster` | `unique_ptr` | `Impl` | class declaration | `src/render/skia/raster/skia_raster.hpp:81` |
| `HarfBuzzFontEngine` | `unique_ptr` | `Impl` | class declaration | `src/render/text/harfbuzz/harfbuzz_font_engine.hpp:70` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/animation_tests.cpp:153` |
| `<file/function>` | `shared_ptr` | `gui_forms::ProgressBar` | local/signature/use | `tests/animation_tests.cpp:154` |
| `<file/function>` | `shared_ptr` | `gui_forms::ProgressBar` | local/signature/use | `tests/animation_tests.cpp:314` |
| `<file/function>` | `shared_ptr` | `gui_forms::ProgressBar` | local/signature/use | `tests/animation_tests.cpp:400` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/animation_tests.cpp:485` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `tests/animation_tests.cpp:486` |
| `<file/function>` | `shared_ptr` | `gui_forms::RadioButton` | local/signature/use | `tests/animation_tests.cpp:487` |
| `<file/function>` | `shared_ptr` | `gui_forms::TrackBar` | local/signature/use | `tests/animation_tests.cpp:488` |
| `<file/function>` | `shared_ptr` | `gui_forms::EasingPreview` | local/signature/use | `tests/animation_tests.cpp:501` |
| `ReplaceButtonDialogResult` | `shared_ptr` | `Button` | local/signature/use | `tests/basic_controls_tests.cpp:116` |
| `ReplaceButtonDialogResult` | `shared_ptr` | `Button` | class declaration | `tests/basic_controls_tests.cpp:124` |
| `<file/function>` | `shared_ptr` | `Label` | local/signature/use | `tests/basic_controls_tests.cpp:174` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/basic_controls_tests.cpp:183` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/basic_controls_tests.cpp:188` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/basic_controls_tests.cpp:199` |
| `<file/function>` | `shared_ptr` | `gui_forms::GroupBox` | local/signature/use | `tests/basic_controls_tests.cpp:203` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/basic_controls_tests.cpp:205` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/basic_controls_tests.cpp:207` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `tests/basic_controls_tests.cpp:210` |
| `<file/function>` | `shared_ptr` | `gui_forms::RadioButton` | local/signature/use | `tests/basic_controls_tests.cpp:213` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/basic_controls_tests.cpp:243` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/basic_controls_tests.cpp:273` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/basic_controls_tests.cpp:274` |
| `<file/function>` | `shared_ptr` | `FocusSink` | local/signature/use | `tests/basic_controls_tests.cpp:275` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/basic_controls_tests.cpp:277` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/basic_controls_tests.cpp:278` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/basic_controls_tests.cpp:280` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/basic_controls_tests.cpp:281` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/basic_controls_tests.cpp:382` |
| `<file/function>` | `shared_ptr` | `FocusSink` | local/signature/use | `tests/basic_controls_tests.cpp:383` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/basic_controls_tests.cpp:397` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `tests/basic_controls_tests.cpp:427` |
| `<file/function>` | `shared_ptr` | `gui_forms::GroupBox` | local/signature/use | `tests/basic_controls_tests.cpp:457` |
| `<file/function>` | `shared_ptr` | `gui_forms::RadioButton` | local/signature/use | `tests/basic_controls_tests.cpp:459` |
| `<file/function>` | `shared_ptr` | `gui_forms::RadioButton` | local/signature/use | `tests/basic_controls_tests.cpp:460` |
| `<file/function>` | `shared_ptr` | `gui_forms::RadioButton` | local/signature/use | `tests/basic_controls_tests.cpp:461` |
| `<file/function>` | `shared_ptr` | `gui_forms::LinkLabel` | local/signature/use | `tests/basic_controls_tests.cpp:489` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/basic_controls_tests.cpp:499` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `tests/basic_controls_tests.cpp:500` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/basic_controls_tests.cpp:518` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/basic_controls_tests.cpp:529` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/basic_controls_tests.cpp:559` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/basic_controls_tests.cpp:586` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/basic_controls_tests.cpp:618` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/basic_controls_tests.cpp:635` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/basic_controls_tests.cpp:657` |
| `<file/function>` | `shared_ptr` | `gui_forms::PictureBox` | local/signature/use | `tests/basic_controls_tests.cpp:688` |
| `<file/function>` | `weak_ptr` | `int` | local/signature/use | `tests/basic_controls_tests.cpp:760` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/basic_controls_tests.cpp:762` |
| `<file/function>` | `shared_ptr` | `int` | local/signature/use | `tests/basic_controls_tests.cpp:763` |
| `<file/function>` | `shared_ptr` | `int` | local/signature/use | `tests/basic_controls_tests.cpp:767` |
| `<file/function>` | `shared_ptr` | `int` | local/signature/use | `tests/basic_controls_tests.cpp:768` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/basic_controls_tests.cpp:776` |
| `<file/function>` | `shared_ptr` | `gui_forms::DrawingSurface` | local/signature/use | `tests/basic_controls_tests.cpp:777` |
| `<file/function>` | `shared_ptr` | `gui_forms::MetricsView` | local/signature/use | `tests/basic_controls_tests.cpp:787` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/basic_controls_tests.cpp:817` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/basic_controls_tests.cpp:839` |
| `<file/function>` | `shared_ptr` | `gui_forms::ImageList` | local/signature/use | `tests/basic_controls_tests.cpp:842` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/basic_controls_tests.cpp:915` |
| `<file/function>` | `shared_ptr` | `gui_forms::ImageList` | local/signature/use | `tests/basic_controls_tests.cpp:917` |
| `Fixture` | `shared_ptr` | `Panel` | class declaration | `tests/binding_tests.cpp:49` |
| `Fixture` | `shared_ptr` | `TextBox` | class declaration | `tests/binding_tests.cpp:50` |
| `Fixture` | `shared_ptr` | `CheckBox` | class declaration | `tests/binding_tests.cpp:51` |
| `Fixture` | `shared_ptr` | `TrackBar` | class declaration | `tests/binding_tests.cpp:52` |
| `Fixture` | `shared_ptr` | `Label` | class declaration | `tests/binding_tests.cpp:53` |
| `Fixture` | `shared_ptr` | `BindingSource` | class declaration | `tests/binding_tests.cpp:55` |
| `PropertyAccessWorker` | `shared_ptr` | `TextBox` | local/signature/use | `tests/binding_tests.cpp:177` |
| `PropertyAccessWorker` | `shared_ptr` | `TextBox` | class declaration | `tests/binding_tests.cpp:198` |
| `TracePositionChange` | `shared_ptr` | `TextBox` | local/signature/use | `tests/binding_tests.cpp:205` |
| `TracePositionChange` | `shared_ptr` | `TextBox` | class declaration | `tests/binding_tests.cpp:216` |
| `ObserveCommittedCombo` | `shared_ptr` | `ComboBox` | local/signature/use | `tests/binding_tests.cpp:260` |
| `ObserveCommittedCombo` | `shared_ptr` | `ComboBox` | class declaration | `tests/binding_tests.cpp:272` |
| `ObserveManagerCommittedCombo` | `shared_ptr` | `Binding` | local/signature/use | `tests/binding_tests.cpp:278` |
| `ObserveManagerCommittedCombo` | `shared_ptr` | `ComboBox` | local/signature/use | `tests/binding_tests.cpp:279` |
| `ObserveManagerCommittedCombo` | `shared_ptr` | `Binding` | class declaration | `tests/binding_tests.cpp:292` |
| `ObserveManagerCommittedCombo` | `shared_ptr` | `ComboBox` | class declaration | `tests/binding_tests.cpp:293` |
| `ObserveCommittedSourceAmount` | `shared_ptr` | `BindingSource` | local/signature/use | `tests/binding_tests.cpp:299` |
| `ObserveCommittedSourceAmount` | `shared_ptr` | `BindingSource` | class declaration | `tests/binding_tests.cpp:312` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/binding_tests.cpp:416` |
| `<file/function>` | `shared_ptr` | `gui_forms::TrackBar` | local/signature/use | `tests/binding_tests.cpp:446` |
| `<file/function>` | `shared_ptr` | `PropertyProbe` | local/signature/use | `tests/binding_tests.cpp:527` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/binding_tests.cpp:587` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/binding_tests.cpp:588` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/binding_tests.cpp:636` |
| `<file/function>` | `shared_ptr` | `gui_forms::PictureBox` | local/signature/use | `tests/binding_tests.cpp:659` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/binding_tests.cpp:683` |
| `<file/function>` | `shared_ptr` | `gui_forms::BindingSource` | local/signature/use | `tests/binding_tests.cpp:686` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `tests/binding_tests.cpp:690` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/binding_tests.cpp:695` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `tests/binding_tests.cpp:713` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `tests/binding_tests.cpp:715` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `tests/binding_tests.cpp:717` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `tests/binding_tests.cpp:719` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `tests/binding_tests.cpp:765` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `tests/binding_tests.cpp:791` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/binding_tests.cpp:807` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/binding_tests.cpp:808` |
| `<file/function>` | `shared_ptr` | `gui_forms::BindingSource` | local/signature/use | `tests/binding_tests.cpp:811` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `tests/binding_tests.cpp:817` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `tests/binding_tests.cpp:894` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `tests/binding_tests.cpp:896` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `tests/binding_tests.cpp:898` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `tests/binding_tests.cpp:916` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/binding_tests.cpp:927` |
| `<file/function>` | `shared_ptr` | `gui_forms::ComboBox` | local/signature/use | `tests/binding_tests.cpp:928` |
| `<file/function>` | `shared_ptr` | `gui_forms::NumericUpDown` | local/signature/use | `tests/binding_tests.cpp:929` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/binding_tests.cpp:930` |
| `<file/function>` | `shared_ptr` | `gui_forms::BindingSource` | local/signature/use | `tests/binding_tests.cpp:936` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `tests/binding_tests.cpp:947` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `tests/binding_tests.cpp:949` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `tests/binding_tests.cpp:978` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `tests/binding_tests.cpp:990` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/canvas_tests.cpp:54` |
| `<file/function>` | `shared_ptr` | `gui_forms::RasterCanvas` | local/signature/use | `tests/canvas_tests.cpp:55` |
| `<file/function>` | `shared_ptr` | `gui_drawing::Bitmap` | local/signature/use | `tests/canvas_tests.cpp:60` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/canvas_tests.cpp:107` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/canvas_tests.cpp:113` |
| `<file/function>` | `shared_ptr` | `gui_forms::RasterCanvas` | local/signature/use | `tests/canvas_tests.cpp:114` |
| `<file/function>` | `shared_ptr` | `gui_drawing::Bitmap` | local/signature/use | `tests/canvas_tests.cpp:117` |
| `ObserveItemChecking` | `shared_ptr` | `CheckedListBox` | local/signature/use | `tests/checked_list_box_tests.cpp:50` |
| `ObserveItemChecking` | `shared_ptr` | `CheckedListBox` | class declaration | `tests/checked_list_box_tests.cpp:67` |
| `ObserveCommittedCheckState` | `shared_ptr` | `CheckedListBox` | local/signature/use | `tests/checked_list_box_tests.cpp:73` |
| `ObserveCommittedCheckState` | `shared_ptr` | `CheckedListBox` | class declaration | `tests/checked_list_box_tests.cpp:85` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckedListBox` | local/signature/use | `tests/checked_list_box_tests.cpp:98` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckedListBox` | local/signature/use | `tests/checked_list_box_tests.cpp:128` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckedListBox` | local/signature/use | `tests/checked_list_box_tests.cpp:157` |
| `<file/function>` | `shared_ptr` | `gui_forms::TreeView` | local/signature/use | `tests/collection_controls_tests.cpp:121` |
| `<file/function>` | `shared_ptr` | `gui_forms::TreeView` | local/signature/use | `tests/collection_controls_tests.cpp:164` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/collection_controls_tests.cpp:176` |
| `<file/function>` | `shared_ptr` | `gui_forms::TreeView` | local/signature/use | `tests/collection_controls_tests.cpp:177` |
| `<file/function>` | `shared_ptr` | `gui_forms::ObjectView` | local/signature/use | `tests/collection_controls_tests.cpp:178` |
| `<file/function>` | `shared_ptr` | `gui_forms::ImageList` | local/signature/use | `tests/collection_controls_tests.cpp:189` |
| `<file/function>` | `shared_ptr` | `gui_forms::ObjectView` | local/signature/use | `tests/collection_controls_tests.cpp:209` |
| `<file/function>` | `shared_ptr` | `gui_forms::ObjectView` | local/signature/use | `tests/collection_controls_tests.cpp:308` |
| `<file/function>` | `shared_ptr` | `gui_forms::Command` | local/signature/use | `tests/collection_controls_tests.cpp:418` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/collection_controls_tests.cpp:419` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/collection_controls_tests.cpp:420` |
| `<file/function>` | `shared_ptr` | `gui_forms::Command` | local/signature/use | `tests/collection_controls_tests.cpp:439` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/collection_controls_tests.cpp:440` |
| `<file/function>` | `shared_ptr` | `gui_forms::CorrespondenceView` | local/signature/use | `tests/collection_controls_tests.cpp:483` |
| `<file/function>` | `shared_ptr` | `gui_forms::CorrespondenceView` | local/signature/use | `tests/collection_controls_tests.cpp:497` |
| `<file/function>` | `shared_ptr` | `gui_forms::CorrespondenceView` | local/signature/use | `tests/collection_controls_tests.cpp:546` |
| `<file/function>` | `shared_ptr` | `gui_forms::Card` | local/signature/use | `tests/composition_controls_tests.cpp:45` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/composition_controls_tests.cpp:53` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/composition_controls_tests.cpp:54` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/composition_controls_tests.cpp:55` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/composition_controls_tests.cpp:66` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/composition_controls_tests.cpp:70` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/composition_controls_tests.cpp:71` |
| `<file/function>` | `shared_ptr` | `gui_forms::Card` | local/signature/use | `tests/composition_controls_tests.cpp:85` |
| `<file/function>` | `shared_ptr` | `gui_forms::Card` | local/signature/use | `tests/composition_controls_tests.cpp:129` |
| `<file/function>` | `shared_ptr` | `const Theme` | local/signature/use | `tests/composition_controls_tests.cpp:172` |
| `<file/function>` | `shared_ptr` | `gui_forms::Card` | local/signature/use | `tests/composition_controls_tests.cpp:173` |
| `<file/function>` | `shared_ptr` | `const Theme` | local/signature/use | `tests/composition_controls_tests.cpp:198` |
| `<file/function>` | `shared_ptr` | `gui_forms::Card` | local/signature/use | `tests/composition_controls_tests.cpp:201` |
| `<file/function>` | `shared_ptr` | `gui_forms::MasterDetailView` | local/signature/use | `tests/composition_controls_tests.cpp:221` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/composition_controls_tests.cpp:231` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/composition_controls_tests.cpp:232` |
| `<file/function>` | `shared_ptr` | `gui_forms::ReviewCard` | local/signature/use | `tests/composition_controls_tests.cpp:254` |
| `<file/function>` | `shared_ptr` | `gui_forms::MasterDetailView` | local/signature/use | `tests/composition_controls_tests.cpp:312` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/composition_controls_tests.cpp:320` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/composition_controls_tests.cpp:321` |
| `<file/function>` | `shared_ptr` | `SplitContainer` | local/signature/use | `tests/composition_controls_tests.cpp:329` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/composition_controls_tests.cpp:377` |
| `AttachChildOnce` | `qualified-alias:shared_ptr` | `Control` | class declaration | `tests/core_tests.cpp:225` |
| `AttachChildOnce` | `qualified-alias:shared_ptr` | `Control` | class declaration | `tests/core_tests.cpp:237` |
| `DisposeCapturedControl` | `qualified-alias:shared_ptr` | `Control` | class declaration | `tests/core_tests.cpp:246` |
| `DisposeCapturedControl` | `qualified-alias:shared_ptr` | `Control` | class declaration | `tests/core_tests.cpp:254` |
| `ReplaceChildOnce` | `qualified-alias:shared_ptr` | `Control` | class declaration | `tests/core_tests.cpp:260` |
| `ReplaceChildOnce` | `qualified-alias:shared_ptr` | `Control` | class declaration | `tests/core_tests.cpp:273` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/core_tests.cpp:388` |
| `Fixture` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:425` |
| `Fixture` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:426` |
| `Fixture` | `unique_ptr` | `Window` | class declaration | `tests/core_tests.cpp:427` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:458` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:507` |
| `<file/function>` | `shared_ptr` | `FaultingLayoutControl` | local/signature/use | `tests/core_tests.cpp:509` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:536` |
| `<file/function>` | `shared_ptr` | `MutatingLayoutControl` | local/signature/use | `tests/core_tests.cpp:538` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:541` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:561` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:563` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:565` |
| `<file/function>` | `shared_ptr` | `MutatingLayoutControl` | local/signature/use | `tests/core_tests.cpp:567` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:570` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:594` |
| `<file/function>` | `shared_ptr` | `MutatingLayoutControl` | local/signature/use | `tests/core_tests.cpp:596` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:614` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:616` |
| `<file/function>` | `shared_ptr` | `MutatingLayoutControl` | local/signature/use | `tests/core_tests.cpp:618` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:638` |
| `<file/function>` | `shared_ptr` | `MutatingLayoutControl` | local/signature/use | `tests/core_tests.cpp:640` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:643` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:663` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:666` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:669` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:674` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:709` |
| `<file/function>` | `shared_ptr` | `CallbackArbitrationControl` | local/signature/use | `tests/core_tests.cpp:711` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:714` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:738` |
| `<file/function>` | `shared_ptr` | `CallbackArbitrationControl` | local/signature/use | `tests/core_tests.cpp:740` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:759` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:761` |
| `<file/function>` | `shared_ptr` | `CallbackArbitrationControl` | local/signature/use | `tests/core_tests.cpp:763` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:780` |
| `<file/function>` | `shared_ptr` | `CallbackArbitrationControl` | local/signature/use | `tests/core_tests.cpp:782` |
| `<file/function>` | `shared_ptr` | `CallbackArbitrationControl` | local/signature/use | `tests/core_tests.cpp:785` |
| `<file/function>` | `shared_ptr` | `CallbackArbitrationControl` | local/signature/use | `tests/core_tests.cpp:788` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:829` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:831` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:833` |
| `<file/function>` | `shared_ptr` | `gui_forms::Component` | local/signature/use | `tests/core_tests.cpp:839` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:877` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:921` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:969` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/core_tests.cpp:979` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/core_tests.cpp:990` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/core_tests.cpp:1012` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/core_tests.cpp:1013` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/core_tests.cpp:1014` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/core_tests.cpp:1015` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/core_tests.cpp:1113` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/core_tests.cpp:1114` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:1152` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:1156` |
| `<file/function>` | `shared_ptr` | `ColoredOverlay` | local/signature/use | `tests/core_tests.cpp:1189` |
| `<file/function>` | `shared_ptr` | `ColoredPopup` | local/signature/use | `tests/core_tests.cpp:1193` |
| `<file/function>` | `shared_ptr` | `gui_forms::Component` | local/signature/use | `tests/core_tests.cpp:1208` |
| `<file/function>` | `shared_ptr` | `FontProbe` | local/signature/use | `tests/core_tests.cpp:1235` |
| `<file/function>` | `shared_ptr` | `gui_forms::Component` | local/signature/use | `tests/core_tests.cpp:1240` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:1243` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:1290` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/core_tests.cpp:1326` |
| `Fixture` | `shared_ptr` | `Panel` | local/signature/use | `tests/date_time_picker_tests.cpp:42` |
| `Fixture` | `shared_ptr` | `DateTimePicker` | local/signature/use | `tests/date_time_picker_tests.cpp:43` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/date_time_picker_tests.cpp:347` |
| `<file/function>` | `shared_ptr` | `gui_forms::DateTimePicker` | local/signature/use | `tests/date_time_picker_tests.cpp:348` |
| `DispatchPhaseProbe` | `weak_ptr` | `Control` | local/signature/use | `tests/dispatcher_tests.cpp:77` |
| `DispatchPhaseProbe` | `shared_ptr` | `Control` | local/signature/use | `tests/dispatcher_tests.cpp:78` |
| `CrossThreadPost` | `qualified-alias:shared_ptr` | `Control` | class declaration | `tests/dispatcher_tests.cpp:135` |
| `CrossThreadPost` | `qualified-alias:shared_ptr` | `Control` | class declaration | `tests/dispatcher_tests.cpp:154` |
| `MarshalledInvokeWorker` | `qualified-alias:shared_ptr` | `Control` | class declaration | `tests/dispatcher_tests.cpp:181` |
| `MarshalledInvokeWorker` | `qualified-alias:shared_ptr` | `Control` | class declaration | `tests/dispatcher_tests.cpp:194` |
| `OwnerInvokeWaiter` | `qualified-alias:shared_ptr` | `Control` | class declaration | `tests/dispatcher_tests.cpp:260` |
| `OwnerInvokeWaiter` | `qualified-alias:shared_ptr` | `Control` | class declaration | `tests/dispatcher_tests.cpp:273` |
| `RecordDispatchedItem` | `shared_ptr` | `DispatchRecord` | local/signature/use | `tests/dispatcher_tests.cpp:324` |
| `RecordDispatchedItem` | `shared_ptr` | `DispatchRecord` | class declaration | `tests/dispatcher_tests.cpp:331` |
| `DispatchProducer` | `shared_ptr` | `DispatchRecord` | local/signature/use | `tests/dispatcher_tests.cpp:347` |
| `TimerPostsDispatch` | `shared_ptr` | `DispatchPhaseProbe` | local/signature/use | `tests/dispatcher_tests.cpp:378` |
| `TimerPostsDispatch` | `shared_ptr` | `DispatchPhaseProbe` | class declaration | `tests/dispatcher_tests.cpp:390` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/dispatcher_tests.cpp:394` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/dispatcher_tests.cpp:431` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/dispatcher_tests.cpp:432` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/dispatcher_tests.cpp:443` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/dispatcher_tests.cpp:471` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/dispatcher_tests.cpp:494` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/dispatcher_tests.cpp:495` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/dispatcher_tests.cpp:545` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/dispatcher_tests.cpp:558` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/dispatcher_tests.cpp:569` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/dispatcher_tests.cpp:592` |
| `<file/function>` | `shared_ptr` | `DispatchPhaseProbe` | local/signature/use | `tests/dispatcher_tests.cpp:647` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/dispatcher_tests.cpp:707` |
| `Fixture` | `shared_ptr` | `ChunkControl` | local/signature/use | `tests/display_chunk_tests.cpp:79` |
| `Fixture` | `shared_ptr` | `ChunkControl` | local/signature/use | `tests/display_chunk_tests.cpp:81` |
| `Fixture` | `shared_ptr` | `ChunkControl` | local/signature/use | `tests/display_chunk_tests.cpp:83` |
| `Fixture` | `unique_ptr` | `Window` | class declaration | `tests/display_chunk_tests.cpp:85` |
| `<file/function>` | `unique_ptr` | `GraphicsPath` | local/signature/use | `tests/drawing_core_tests.cpp:273` |
| `<file/function>` | `unique_ptr` | `GraphicsPath` | local/signature/use | `tests/drawing_core_tests.cpp:396` |
| `<file/function>` | `unique_ptr` | `Bitmap` | local/signature/use | `tests/drawing_core_tests.cpp:438` |
| `<file/function>` | `unique_ptr` | `Bitmap` | local/signature/use | `tests/drawing_core_tests.cpp:444` |
| `<file/function>` | `unique_ptr` | `Bitmap` | local/signature/use | `tests/drawing_core_tests.cpp:452` |
| `<file/function>` | `unique_ptr` | `TextureBrush` | local/signature/use | `tests/drawing_core_tests.cpp:483` |
| `FocusFixture` | `shared_ptr` | `Panel` | local/signature/use | `tests/focus_scope_tests.cpp:37` |
| `FocusFixture` | `shared_ptr` | `Button` | local/signature/use | `tests/focus_scope_tests.cpp:38` |
| `FocusFixture` | `shared_ptr` | `Panel` | local/signature/use | `tests/focus_scope_tests.cpp:40` |
| `FocusFixture` | `shared_ptr` | `Button` | local/signature/use | `tests/focus_scope_tests.cpp:42` |
| `FocusFixture` | `shared_ptr` | `Button` | local/signature/use | `tests/focus_scope_tests.cpp:44` |
| `FocusFixture` | `unique_ptr` | `Window` | class declaration | `tests/focus_scope_tests.cpp:46` |
| `BeginFocusScopeOffThread` | `shared_ptr` | `Panel` | local/signature/use | `tests/focus_scope_tests.cpp:83` |
| `BeginFocusScopeOffThread` | `shared_ptr` | `Panel` | class declaration | `tests/focus_scope_tests.cpp:97` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/focus_scope_tests.cpp:157` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/focus_scope_tests.cpp:158` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/focus_scope_tests.cpp:159` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/focus_scope_tests.cpp:160` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/focus_scope_tests.cpp:162` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/focus_scope_tests.cpp:163` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/focus_scope_tests.cpp:285` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/focus_scope_tests.cpp:286` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/focus_scope_tests.cpp:287` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/focus_scope_tests.cpp:288` |
| `Fixture` | `qualified-alias:shared_ptr` | `Control` | class declaration | `tests/frame_scheduler_tests.cpp:80` |
| `Fixture` | `unique_ptr` | `Window` | class declaration | `tests/frame_scheduler_tests.cpp:81` |
| `ScheduleDuringFrameCallback` | `shared_ptr` | `ReentrantFrameControl` | local/signature/use | `tests/frame_scheduler_tests.cpp:90` |
| `ScheduleDuringFrameCallback` | `shared_ptr` | `ReentrantFrameControl` | class declaration | `tests/frame_scheduler_tests.cpp:105` |
| `SchedulePaintOffThread` | `qualified-alias:shared_ptr` | `Control` | class declaration | `tests/frame_scheduler_tests.cpp:111` |
| `SchedulePaintOffThread` | `qualified-alias:shared_ptr` | `Control` | class declaration | `tests/frame_scheduler_tests.cpp:125` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/frame_scheduler_tests.cpp:200` |
| `<file/function>` | `shared_ptr` | `ThrowingFrameControl` | local/signature/use | `tests/frame_scheduler_tests.cpp:201` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/frame_scheduler_tests.cpp:203` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/frame_scheduler_tests.cpp:233` |
| `<file/function>` | `shared_ptr` | `ReentrantFrameControl` | local/signature/use | `tests/frame_scheduler_tests.cpp:234` |
| `<file/function>` | `shared_ptr` | `ReentrantFrameControl` | local/signature/use | `tests/frame_scheduler_tests.cpp:237` |
| `<file/function>` | `shared_ptr` | `ReentrantFrameControl` | local/signature/use | `tests/frame_scheduler_tests.cpp:240` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/frame_scheduler_tests.cpp:316` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/frame_scheduler_tests.cpp:317` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/frame_scheduler_tests.cpp:318` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/gallery_interaction_tests.cpp:88` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/gallery_interaction_tests.cpp:96` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/gallery_interaction_tests.cpp:99` |
| `<file/function>` | `unique_ptr` | `gui_forms::Window` | local/signature/use | `tests/gallery_interaction_tests.cpp:164` |
| `<file/function>` | `shared_ptr` | `gui_forms::UserControl` | local/signature/use | `tests/gallery_interaction_tests.cpp:171` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/gallery_interaction_tests.cpp:173` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/gallery_interaction_tests.cpp:190` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/gallery_interaction_tests.cpp:194` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `tests/gallery_interaction_tests.cpp:229` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/gallery_interaction_tests.cpp:253` |
| `<file/function>` | `shared_ptr` | `gui_forms::gallery::GalleryControl` | local/signature/use | `tests/gallery_interaction_tests.cpp:270` |
| `<file/function>` | `shared_ptr` | `gui_forms::TrackBar` | local/signature/use | `tests/gallery_interaction_tests.cpp:275` |
| `<file/function>` | `shared_ptr` | `gui_forms::ProgressBar` | local/signature/use | `tests/gallery_interaction_tests.cpp:277` |
| `<file/function>` | `shared_ptr` | `gui_forms::gallery::GalleryControl` | local/signature/use | `tests/gallery_interaction_tests.cpp:279` |
| `<file/function>` | `shared_ptr` | `gui_forms::gallery::GalleryControl` | local/signature/use | `tests/gallery_interaction_tests.cpp:298` |
| `<file/function>` | `shared_ptr` | `gui_forms::gallery::GalleryControl` | local/signature/use | `tests/gallery_interaction_tests.cpp:300` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/gallery_interaction_tests.cpp:316` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/gallery_interaction_tests.cpp:324` |
| `<file/function>` | `shared_ptr` | `OverflowPaintProbe` | local/signature/use | `tests/gallery_interaction_tests.cpp:329` |
| `<file/function>` | `shared_ptr` | `OverflowPaintProbe` | local/signature/use | `tests/gallery_interaction_tests.cpp:332` |
| `Fixture` | `shared_ptr` | `Panel` | local/signature/use | `tests/guidance_provider_tests.cpp:46` |
| `Fixture` | `shared_ptr` | `Panel` | local/signature/use | `tests/guidance_provider_tests.cpp:47` |
| `Fixture` | `shared_ptr` | `TextBox` | local/signature/use | `tests/guidance_provider_tests.cpp:48` |
| `Fixture` | `shared_ptr` | `Button` | local/signature/use | `tests/guidance_provider_tests.cpp:50` |
| `<file/function>` | `unique_ptr` | `gui_forms::ErrorProvider` | local/signature/use | `tests/guidance_provider_tests.cpp:324` |
| `<file/function>` | `unique_ptr` | `gui_forms::ErrorProvider` | local/signature/use | `tests/guidance_provider_tests.cpp:325` |
| `Fixture` | `shared_ptr` | `InputProbe` | class declaration | `tests/host_protocol_tests.cpp:128` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/host_protocol_tests.cpp:749` |
| `<file/function>` | `shared_ptr` | `InputProbe` | local/signature/use | `tests/host_protocol_tests.cpp:751` |
| `<file/function>` | `shared_ptr` | `InputProbe` | local/signature/use | `tests/host_protocol_tests.cpp:753` |
| `<file/function>` | `shared_ptr` | `HostLeaseInputProbe` | local/signature/use | `tests/host_protocol_tests.cpp:1112` |
| `<file/function>` | `shared_ptr` | `HostLeaseInputProbe` | local/signature/use | `tests/host_protocol_tests.cpp:1142` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/input_controls_tests.cpp:89` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/input_controls_tests.cpp:116` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/input_controls_tests.cpp:147` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/input_controls_tests.cpp:179` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/input_controls_tests.cpp:198` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/input_controls_tests.cpp:220` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/input_controls_tests.cpp:257` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/input_controls_tests.cpp:258` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/input_controls_tests.cpp:261` |
| `<file/function>` | `shared_ptr` | `gui_forms::ListBox` | local/signature/use | `tests/input_controls_tests.cpp:336` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/input_controls_tests.cpp:384` |
| `<file/function>` | `shared_ptr` | `gui_forms::ComboBox` | local/signature/use | `tests/input_controls_tests.cpp:385` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/input_controls_tests.cpp:432` |
| `<file/function>` | `shared_ptr` | `gui_forms::NumericUpDown` | local/signature/use | `tests/input_controls_tests.cpp:494` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/input_controls_tests.cpp:526` |
| `<file/function>` | `shared_ptr` | `gui_forms::ListBox` | local/signature/use | `tests/input_controls_tests.cpp:540` |
| `<file/function>` | `shared_ptr` | `PropertyList` | local/signature/use | `tests/inspection_controls_tests.cpp:34` |
| `<file/function>` | `shared_ptr` | `gui_forms::PropertyList` | local/signature/use | `tests/inspection_controls_tests.cpp:35` |
| `SwitchPropertyGridSelection` | `shared_ptr` | `PropertyGrid` | local/signature/use | `tests/inspection_controls_tests.cpp:316` |
| `SwitchPropertyGridSelection` | `qualified-alias:shared_ptr` | `Control` | class declaration | `tests/inspection_controls_tests.cpp:317` |
| `SwitchPropertyGridSelection` | `shared_ptr` | `PropertyGrid` | class declaration | `tests/inspection_controls_tests.cpp:325` |
| `SwitchPropertyGridSelection` | `qualified-alias:shared_ptr` | `Control` | class declaration | `tests/inspection_controls_tests.cpp:326` |
| `DisposeWeakPropertyGrid` | `weak_ptr` | `PropertyGrid` | local/signature/use | `tests/inspection_controls_tests.cpp:331` |
| `DisposeWeakPropertyGrid` | `shared_ptr` | `PropertyGrid` | local/signature/use | `tests/inspection_controls_tests.cpp:335` |
| `DisposeWeakPropertyGrid` | `weak_ptr` | `PropertyGrid` | class declaration | `tests/inspection_controls_tests.cpp:340` |
| `SynchronizePercentEditor` | `weak_ptr` | `NumericUpDown` | local/signature/use | `tests/inspection_controls_tests.cpp:392` |
| `SynchronizePercentEditor` | `shared_ptr` | `bool` | local/signature/use | `tests/inspection_controls_tests.cpp:393` |
| `SynchronizePercentEditor` | `shared_ptr` | `NumericUpDown` | local/signature/use | `tests/inspection_controls_tests.cpp:398` |
| `SynchronizePercentEditor` | `weak_ptr` | `NumericUpDown` | class declaration | `tests/inspection_controls_tests.cpp:407` |
| `SynchronizePercentEditor` | `shared_ptr` | `bool` | class declaration | `tests/inspection_controls_tests.cpp:408` |
| `CommitPercentValue` | `shared_ptr` | `bool` | local/signature/use | `tests/inspection_controls_tests.cpp:413` |
| `CommitPercentValue` | `shared_ptr` | `bool` | class declaration | `tests/inspection_controls_tests.cpp:423` |
| `ConnectPercentEditor` | `weak_ptr` | `NumericUpDown` | local/signature/use | `tests/inspection_controls_tests.cpp:429` |
| `ConnectPercentEditor` | `shared_ptr` | `bool` | local/signature/use | `tests/inspection_controls_tests.cpp:430` |
| `ConnectPercentEditor` | `shared_ptr` | `NumericUpDown` | local/signature/use | `tests/inspection_controls_tests.cpp:437` |
| `ConnectPercentEditor` | `weak_ptr` | `NumericUpDown` | class declaration | `tests/inspection_controls_tests.cpp:444` |
| `ConnectPercentEditor` | `shared_ptr` | `bool` | class declaration | `tests/inspection_controls_tests.cpp:445` |
| `<file/function>` | `shared_ptr` | `NumericUpDown` | local/signature/use | `tests/inspection_controls_tests.cpp:452` |
| `<file/function>` | `shared_ptr` | `bool` | local/signature/use | `tests/inspection_controls_tests.cpp:458` |
| `<file/function>` | `shared_ptr` | `PropertyList` | local/signature/use | `tests/inspection_controls_tests.cpp:467` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/inspection_controls_tests.cpp:470` |
| `<file/function>` | `shared_ptr` | `gui_forms::ComboBox` | local/signature/use | `tests/inspection_controls_tests.cpp:472` |
| `<file/function>` | `shared_ptr` | `PropertyList` | local/signature/use | `tests/inspection_controls_tests.cpp:534` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/inspection_controls_tests.cpp:535` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/inspection_controls_tests.cpp:549` |
| `<file/function>` | `shared_ptr` | `gui_forms::PropertyList` | local/signature/use | `tests/inspection_controls_tests.cpp:577` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/inspection_controls_tests.cpp:591` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/inspection_controls_tests.cpp:592` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/inspection_controls_tests.cpp:609` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `tests/inspection_controls_tests.cpp:611` |
| `<file/function>` | `shared_ptr` | `gui_forms::PropertyGrid` | local/signature/use | `tests/inspection_controls_tests.cpp:614` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/inspection_controls_tests.cpp:628` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `tests/inspection_controls_tests.cpp:629` |
| `<file/function>` | `shared_ptr` | `gui_forms::NumericUpDown` | local/signature/use | `tests/inspection_controls_tests.cpp:677` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/inspection_controls_tests.cpp:704` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/inspection_controls_tests.cpp:744` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/inspection_controls_tests.cpp:759` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `tests/inspection_controls_tests.cpp:761` |
| `<file/function>` | `shared_ptr` | `gui_forms::PropertyGrid` | local/signature/use | `tests/inspection_controls_tests.cpp:763` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/inspection_controls_tests.cpp:777` |
| `<file/function>` | `shared_ptr` | `gui_forms::PropertyGrid` | local/signature/use | `tests/inspection_controls_tests.cpp:779` |
| `<file/function>` | `weak_ptr` | `PropertyGrid` | local/signature/use | `tests/inspection_controls_tests.cpp:782` |
| `<file/function>` | `shared_ptr` | `PropertyValueConverterRegistry` | local/signature/use | `tests/inspection_controls_tests.cpp:800` |
| `<file/function>` | `shared_ptr` | `PropertyEditorRegistry` | local/signature/use | `tests/inspection_controls_tests.cpp:808` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/inspection_controls_tests.cpp:813` |
| `<file/function>` | `shared_ptr` | `PercentPropertyProbe` | local/signature/use | `tests/inspection_controls_tests.cpp:814` |
| `<file/function>` | `shared_ptr` | `gui_forms::PropertyGrid` | local/signature/use | `tests/inspection_controls_tests.cpp:817` |
| `<file/function>` | `shared_ptr` | `gui_forms::NumericUpDown` | local/signature/use | `tests/inspection_controls_tests.cpp:830` |
| `<file/function>` | `shared_ptr` | `PropertyValueConverterRegistry` | local/signature/use | `tests/inspection_controls_tests.cpp:847` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/inspection_controls_tests.cpp:872` |
| `<file/function>` | `shared_ptr` | `NullablePropertyProbe` | local/signature/use | `tests/inspection_controls_tests.cpp:873` |
| `<file/function>` | `shared_ptr` | `gui_forms::PropertyGrid` | local/signature/use | `tests/inspection_controls_tests.cpp:876` |
| `<file/function>` | `shared_ptr` | `gui_forms::ComboBox` | local/signature/use | `tests/inspection_controls_tests.cpp:889` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/inspection_controls_tests.cpp:909` |
| `<file/function>` | `shared_ptr` | `AtomicPropertyProbe` | local/signature/use | `tests/inspection_controls_tests.cpp:910` |
| `<file/function>` | `shared_ptr` | `AtomicPropertyProbe` | local/signature/use | `tests/inspection_controls_tests.cpp:913` |
| `<file/function>` | `shared_ptr` | `gui_forms::PropertyGrid` | local/signature/use | `tests/inspection_controls_tests.cpp:916` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/inspection_controls_tests.cpp:930` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/inspection_controls_tests.cpp:943` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/inspection_controls_tests.cpp:944` |
| `<file/function>` | `shared_ptr` | `gui_forms::PropertyGrid` | local/signature/use | `tests/inspection_controls_tests.cpp:947` |
| `<file/function>` | `shared_ptr` | `gui_forms::FlagsValueEditor` | local/signature/use | `tests/inspection_controls_tests.cpp:955` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckedListBox` | local/signature/use | `tests/inspection_controls_tests.cpp:964` |
| `<file/function>` | `shared_ptr` | `gui_forms::ColorValueEditor` | local/signature/use | `tests/inspection_controls_tests.cpp:978` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/inspection_controls_tests.cpp:1017` |
| `<file/function>` | `shared_ptr` | `NestedPropertyProbe` | local/signature/use | `tests/inspection_controls_tests.cpp:1018` |
| `<file/function>` | `shared_ptr` | `gui_forms::PropertyGrid` | local/signature/use | `tests/inspection_controls_tests.cpp:1021` |
| `<file/function>` | `shared_ptr` | `InstrumentRack` | local/signature/use | `tests/instrument_controls_tests.cpp:140` |
| `<file/function>` | `shared_ptr` | `gui_forms::InstrumentRack` | local/signature/use | `tests/instrument_controls_tests.cpp:141` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/instrument_controls_tests.cpp:143` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/instrument_controls_tests.cpp:145` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/instrument_controls_tests.cpp:147` |
| `<file/function>` | `shared_ptr` | `InstrumentRack` | local/signature/use | `tests/instrument_controls_tests.cpp:158` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/instrument_controls_tests.cpp:159` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/instrument_controls_tests.cpp:160` |
| `<file/function>` | `shared_ptr` | `InstrumentRack` | local/signature/use | `tests/instrument_controls_tests.cpp:194` |
| `<file/function>` | `shared_ptr` | `gui_forms::ComboBox` | local/signature/use | `tests/instrument_controls_tests.cpp:207` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/instrument_controls_tests.cpp:213` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/instrument_controls_tests.cpp:224` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/instrument_controls_tests.cpp:239` |
| `<file/function>` | `shared_ptr` | `InstrumentRack` | local/signature/use | `tests/instrument_controls_tests.cpp:251` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/instrument_controls_tests.cpp:263` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/instrument_controls_tests.cpp:265` |
| `<file/function>` | `shared_ptr` | `gui_forms::InstrumentRack` | local/signature/use | `tests/instrument_controls_tests.cpp:283` |
| `TreeFixture` | `shared_ptr` | `CountingControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:330` |
| `TreeFixture` | `shared_ptr` | `CountingControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:332` |
| `TreeFixture` | `shared_ptr` | `CountingControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:334` |
| `TreeFixture` | `shared_ptr` | `CountingControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:336` |
| `TreeFixture` | `shared_ptr` | `CountingControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:338` |
| `TreeFixture` | `shared_ptr` | `CountingControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:340` |
| `TreeFixture` | `unique_ptr` | `Window` | class declaration | `tests/invalidation_damage_tests.cpp:342` |
| `<file/function>` | `shared_ptr` | `CountingControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:454` |
| `<file/function>` | `shared_ptr` | `CountingControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:457` |
| `<file/function>` | `shared_ptr` | `CountingControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:461` |
| `<file/function>` | `shared_ptr` | `CountingControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:465` |
| `<file/function>` | `shared_ptr` | `ReentrantLayoutControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:499` |
| `<file/function>` | `shared_ptr` | `TransactionPaintControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:609` |
| `<file/function>` | `shared_ptr` | `TransactionPaintControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:640` |
| `<file/function>` | `shared_ptr` | `TransactionPaintControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:658` |
| `<file/function>` | `shared_ptr` | `DeferredInputPaintControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:685` |
| `<file/function>` | `shared_ptr` | `DeferredInputPaintControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:728` |
| `<file/function>` | `shared_ptr` | `DeferredInputPaintControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:753` |
| `<file/function>` | `shared_ptr` | `DeferredInputPaintControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:776` |
| `<file/function>` | `shared_ptr` | `ReplayPressureControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:806` |
| `<file/function>` | `shared_ptr` | `ReplayPressureControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:863` |
| `<file/function>` | `shared_ptr` | `ReplayPressureControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:896` |
| `<file/function>` | `shared_ptr` | `ReplayPressureControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:933` |
| `<file/function>` | `shared_ptr` | `ReplayPressureControl` | local/signature/use | `tests/invalidation_damage_tests.cpp:956` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/invalidation_damage_tests.cpp:976` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:77` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/layout_panel_tests.cpp:79` |
| `<file/function>` | `shared_ptr` | `gui_forms::FlowLayoutPanel` | local/signature/use | `tests/layout_panel_tests.cpp:86` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:89` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:90` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:91` |
| `<file/function>` | `shared_ptr` | `gui_forms::FlowLayoutPanel` | local/signature/use | `tests/layout_panel_tests.cpp:137` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:138` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:139` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:140` |
| `<file/function>` | `shared_ptr` | `gui_forms::FlowLayoutPanel` | local/signature/use | `tests/layout_panel_tests.cpp:185` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:186` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:187` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:188` |
| `<file/function>` | `shared_ptr` | `gui_forms::FlowLayoutPanel` | local/signature/use | `tests/layout_panel_tests.cpp:212` |
| `<file/function>` | `shared_ptr` | `MeasureMutationControl` | local/signature/use | `tests/layout_panel_tests.cpp:214` |
| `<file/function>` | `shared_ptr` | `MeasureMutationControl` | local/signature/use | `tests/layout_panel_tests.cpp:217` |
| `<file/function>` | `shared_ptr` | `gui_forms::TableLayoutPanel` | local/signature/use | `tests/layout_panel_tests.cpp:240` |
| `<file/function>` | `shared_ptr` | `MeasureMutationControl` | local/signature/use | `tests/layout_panel_tests.cpp:244` |
| `<file/function>` | `shared_ptr` | `MeasureMutationControl` | local/signature/use | `tests/layout_panel_tests.cpp:247` |
| `<file/function>` | `shared_ptr` | `gui_forms::TableLayoutPanel` | local/signature/use | `tests/layout_panel_tests.cpp:271` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:284` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:285` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:286` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:287` |
| `<file/function>` | `shared_ptr` | `gui_forms::TableLayoutPanel` | local/signature/use | `tests/layout_panel_tests.cpp:331` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:335` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:336` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:337` |
| `<file/function>` | `shared_ptr` | `gui_forms::TableLayoutPanel` | local/signature/use | `tests/layout_panel_tests.cpp:379` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:384` |
| `<file/function>` | `shared_ptr` | `gui_forms::TableLayoutPanel` | local/signature/use | `tests/layout_panel_tests.cpp:414` |
| `<file/function>` | `shared_ptr` | `gui_forms::ScaledPanel` | local/signature/use | `tests/layout_panel_tests.cpp:427` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:429` |
| `<file/function>` | `shared_ptr` | `gui_forms::ScaledGroupBox` | local/signature/use | `tests/layout_panel_tests.cpp:431` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:433` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/layout_panel_tests.cpp:480` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:482` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:483` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:484` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:485` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:486` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/layout_panel_tests.cpp:517` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:518` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:519` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/layout_panel_tests.cpp:538` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:540` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:542` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:546` |
| `<file/function>` | `shared_ptr` | `Button` | local/signature/use | `tests/layout_panel_tests.cpp:549` |
| `LifecycleProbe` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/lifecycle_controls_tests.cpp:34` |
| `LifecycleProbe` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/lifecycle_controls_tests.cpp:42` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/lifecycle_controls_tests.cpp:208` |
| `<file/function>` | `shared_ptr` | `LifecycleProbe` | local/signature/use | `tests/lifecycle_controls_tests.cpp:209` |
| `<file/function>` | `shared_ptr` | `LifecycleProbe` | local/signature/use | `tests/lifecycle_controls_tests.cpp:211` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/lifecycle_controls_tests.cpp:223` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/lifecycle_controls_tests.cpp:232` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/lifecycle_controls_tests.cpp:233` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/lifecycle_controls_tests.cpp:234` |
| `<file/function>` | `shared_ptr` | `gui_forms::UserControl` | local/signature/use | `tests/lifecycle_controls_tests.cpp:235` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/lifecycle_controls_tests.cpp:236` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/lifecycle_controls_tests.cpp:261` |
| `<file/function>` | `shared_ptr` | `ThrowingAttachControl` | local/signature/use | `tests/lifecycle_controls_tests.cpp:263` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/lifecycle_controls_tests.cpp:275` |
| `<file/function>` | `shared_ptr` | `gui_forms::UserControl` | local/signature/use | `tests/lifecycle_controls_tests.cpp:280` |
| `<file/function>` | `shared_ptr` | `ThrowingAttachControl` | local/signature/use | `tests/lifecycle_controls_tests.cpp:281` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/lifecycle_controls_tests.cpp:304` |
| `<file/function>` | `shared_ptr` | `gui_forms::UserControl` | local/signature/use | `tests/lifecycle_controls_tests.cpp:306` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/lifecycle_controls_tests.cpp:323` |
| `<file/function>` | `shared_ptr` | `InitializationEventProbe` | local/signature/use | `tests/lifecycle_controls_tests.cpp:367` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/lifecycle_controls_tests.cpp:385` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/lifecycle_controls_tests.cpp:386` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/lifecycle_controls_tests.cpp:398` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/lifecycle_controls_tests.cpp:408` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/lifecycle_controls_tests.cpp:409` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/lifecycle_controls_tests.cpp:423` |
| `<file/function>` | `shared_ptr` | `LiveSurface` | local/signature/use | `tests/live_surface_tests.cpp:42` |
| `<file/function>` | `shared_ptr` | `LiveSurface` | local/signature/use | `tests/live_surface_tests.cpp:126` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/live_surface_tests.cpp:141` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/live_surface_tests.cpp:142` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/live_surface_tests.cpp:143` |
| `<file/function>` | `shared_ptr` | `LiveSurface` | local/signature/use | `tests/live_surface_tests.cpp:153` |
| `RearmProbeState` | `shared_ptr` | `FrameProbe` | class declaration | `tests/macos_host_close_tests.mm:30` |
| `<file/function>` | `shared_ptr` | `RearmProbeState` | local/signature/use | `tests/macos_host_close_tests.mm:54` |
| `RecordNestedDispatch` | `shared_ptr` | `RearmProbeState` | local/signature/use | `tests/macos_host_close_tests.mm:83` |
| `RecordNestedDispatch` | `shared_ptr` | `RearmProbeState` | class declaration | `tests/macos_host_close_tests.mm:93` |
| `RecordWorkerDispatch` | `shared_ptr` | `RearmProbeState` | local/signature/use | `tests/macos_host_close_tests.mm:98` |
| `RecordWorkerDispatch` | `shared_ptr` | `RearmProbeState` | class declaration | `tests/macos_host_close_tests.mm:110` |
| `BeginWorkerDispatch` | `shared_ptr` | `RearmProbeState` | local/signature/use | `tests/macos_host_close_tests.mm:115` |
| `BeginWorkerDispatch` | `shared_ptr` | `RearmProbeState` | class declaration | `tests/macos_host_close_tests.mm:126` |
| `RecordSynchronousDispatch` | `shared_ptr` | `RearmProbeState` | local/signature/use | `tests/macos_host_close_tests.mm:131` |
| `RecordSynchronousDispatch` | `shared_ptr` | `RearmProbeState` | class declaration | `tests/macos_host_close_tests.mm:140` |
| `RunSynchronousDispatch` | `shared_ptr` | `RearmProbeState` | local/signature/use | `tests/macos_host_close_tests.mm:152` |
| `RunSynchronousDispatch` | `shared_ptr` | `RearmProbeState` | class declaration | `tests/macos_host_close_tests.mm:168` |
| `PrepareCloseTestHost` | `shared_ptr` | `RearmProbeState` | local/signature/use | `tests/macos_host_close_tests.mm:201` |
| `PrepareCloseTestHost` | `shared_ptr` | `FrameProbe` | local/signature/use | `tests/macos_host_close_tests.mm:202` |
| `PrepareCloseTestHost` | `shared_ptr` | `RearmProbeState` | local/signature/use | `tests/macos_host_close_tests.mm:234` |
| `PrepareCloseTestHost` | `shared_ptr` | `FrameProbe` | local/signature/use | `tests/macos_host_close_tests.mm:235` |
| `PrepareCloseTestHost` | `shared_ptr` | `RearmProbeState` | class declaration | `tests/macos_host_close_tests.mm:250` |
| `PrepareCloseTestHost` | `shared_ptr` | `FrameProbe` | class declaration | `tests/macos_host_close_tests.mm:251` |
| `<file/function>` | `shared_ptr` | `RearmProbeState` | local/signature/use | `tests/macos_host_close_tests.mm:257` |
| `<file/function>` | `shared_ptr` | `RearmProbeState` | local/signature/use | `tests/macos_host_close_tests.mm:266` |
| `<file/function>` | `unique_ptr` | `gui_forms::HostServices` | local/signature/use | `tests/macos_host_close_tests.mm:312` |
| `<file/function>` | `shared_ptr` | `FrameProbe` | local/signature/use | `tests/macos_host_close_tests.mm:406` |
| `<file/function>` | `shared_ptr` | `FrameProbe` | local/signature/use | `tests/macos_host_close_tests.mm:408` |
| `<file/function>` | `unique_ptr` | `gui_forms::Window` | local/signature/use | `tests/macos_host_close_tests.mm:413` |
| `<file/function>` | `shared_ptr` | `RearmProbeState` | local/signature/use | `tests/macos_host_close_tests.mm:419` |
| `<file/function>` | `unique_ptr` | `gui_forms::Window` | local/signature/use | `tests/macos_multi_window_tests.mm:24` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/macos_multi_window_tests.mm:25` |
| `<file/function>` | `shared_ptr` | `gui_forms::MaterialPanel` | local/signature/use | `tests/material_tests.cpp:156` |
| `<file/function>` | `shared_ptr` | `gui_forms::MaterialPanel` | local/signature/use | `tests/material_tests.cpp:191` |
| `<file/function>` | `shared_ptr` | `gui_forms::MaterialPanel` | local/signature/use | `tests/material_tests.cpp:228` |
| `<file/function>` | `shared_ptr` | `gui_forms::MaterialPanel` | local/signature/use | `tests/material_tests.cpp:252` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/material_tests.cpp:348` |
| `<file/function>` | `shared_ptr` | `gui_forms::MaterialPanel` | local/signature/use | `tests/material_tests.cpp:350` |
| `<file/function>` | `shared_ptr` | `gui_forms::TableLayoutPanel` | local/signature/use | `tests/menu_controls_tests.cpp:68` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/menu_controls_tests.cpp:73` |
| `<file/function>` | `shared_ptr` | `gui_forms::Command` | local/signature/use | `tests/menu_controls_tests.cpp:80` |
| `<file/function>` | `shared_ptr` | `gui_forms::Command` | local/signature/use | `tests/menu_controls_tests.cpp:82` |
| `<file/function>` | `shared_ptr` | `gui_forms::Command` | local/signature/use | `tests/menu_controls_tests.cpp:85` |
| `<file/function>` | `shared_ptr` | `gui_forms::Command` | local/signature/use | `tests/menu_controls_tests.cpp:89` |
| `<file/function>` | `shared_ptr` | `gui_forms::Command` | local/signature/use | `tests/menu_controls_tests.cpp:91` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/menu_controls_tests.cpp:119` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/menu_controls_tests.cpp:182` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/menu_controls_tests.cpp:183` |
| `<file/function>` | `shared_ptr` | `Command` | local/signature/use | `tests/menu_controls_tests.cpp:190` |
| `<file/function>` | `shared_ptr` | `gui_forms::Command` | local/signature/use | `tests/menu_controls_tests.cpp:192` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/menu_controls_tests.cpp:201` |
| `<file/function>` | `shared_ptr` | `gui_forms::TableLayoutPanel` | local/signature/use | `tests/menu_controls_tests.cpp:229` |
| `<file/function>` | `shared_ptr` | `gui_forms::MenuStrip` | local/signature/use | `tests/menu_controls_tests.cpp:235` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/menu_controls_tests.cpp:239` |
| `<file/function>` | `shared_ptr` | `gui_forms::Command` | local/signature/use | `tests/menu_controls_tests.cpp:245` |
| `<file/function>` | `shared_ptr` | `gui_forms::Command` | local/signature/use | `tests/menu_controls_tests.cpp:246` |
| `<file/function>` | `shared_ptr` | `gui_forms::Command` | local/signature/use | `tests/menu_controls_tests.cpp:247` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/menu_controls_tests.cpp:303` |
| `<file/function>` | `shared_ptr` | `gui_forms::MenuStrip` | local/signature/use | `tests/menu_controls_tests.cpp:304` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/menu_controls_tests.cpp:306` |
| `<file/function>` | `shared_ptr` | `gui_forms::Command` | local/signature/use | `tests/menu_controls_tests.cpp:311` |
| `<file/function>` | `shared_ptr` | `gui_forms::Command` | local/signature/use | `tests/menu_controls_tests.cpp:312` |
| `<file/function>` | `shared_ptr` | `gui_forms::Command` | local/signature/use | `tests/menu_controls_tests.cpp:340` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/menu_controls_tests.cpp:350` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/png_registry_tests.cpp:314` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/png_registry_tests.cpp:326` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/png_registry_tests.cpp:327` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/popup_controls_tests.cpp:52` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/popup_controls_tests.cpp:53` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/popup_controls_tests.cpp:60` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/popup_controls_tests.cpp:61` |
| `<file/function>` | `shared_ptr` | `gui_forms::AnchoredPopupLayer` | local/signature/use | `tests/popup_controls_tests.cpp:67` |
| `<file/function>` | `shared_ptr` | `gui_forms::ContainerControl` | local/signature/use | `tests/range_controls_tests.cpp:90` |
| `<file/function>` | `shared_ptr` | `gui_forms::UserControl` | local/signature/use | `tests/range_controls_tests.cpp:91` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/range_controls_tests.cpp:92` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/range_controls_tests.cpp:93` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/range_controls_tests.cpp:112` |
| `<file/function>` | `shared_ptr` | `gui_forms::TrackBar` | local/signature/use | `tests/range_controls_tests.cpp:118` |
| `<file/function>` | `shared_ptr` | `gui_forms::TrackBar` | local/signature/use | `tests/range_controls_tests.cpp:173` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/range_controls_tests.cpp:214` |
| `<file/function>` | `shared_ptr` | `gui_forms::ProgressBar` | local/signature/use | `tests/range_controls_tests.cpp:216` |
| `<file/function>` | `shared_ptr` | `gui_forms::ProgressBar` | local/signature/use | `tests/range_controls_tests.cpp:219` |
| `<file/function>` | `shared_ptr` | `gui_forms::TrackBar` | local/signature/use | `tests/range_controls_tests.cpp:223` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/range_controls_tests.cpp:246` |
| `<file/function>` | `shared_ptr` | `gui_forms::HScrollBar` | local/signature/use | `tests/range_controls_tests.cpp:247` |
| `<file/function>` | `shared_ptr` | `gui_forms::VScrollBar` | local/signature/use | `tests/range_controls_tests.cpp:254` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/range_controls_tests.cpp:335` |
| `<file/function>` | `shared_ptr` | `gui_forms::TrackBar` | local/signature/use | `tests/range_controls_tests.cpp:336` |
| `<file/function>` | `shared_ptr` | `gui_forms::ProgressBar` | local/signature/use | `tests/range_controls_tests.cpp:353` |
| `WindowFixture` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/retained_lifetime_tests.cpp:104` |
| `WindowFixture` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/retained_lifetime_tests.cpp:105` |
| `WindowFixture` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/retained_lifetime_tests.cpp:106` |
| `WindowFixture` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/retained_lifetime_tests.cpp:107` |
| `WindowFixture` | `unique_ptr` | `Window` | class declaration | `tests/retained_lifetime_tests.cpp:108` |
| `DisposeOwnerAfterRecording` | `shared_ptr` | `ProbeComponent` | local/signature/use | `tests/retained_lifetime_tests.cpp:124` |
| `DisposeOwnerAfterRecording` | `shared_ptr` | `ProbeComponent` | class declaration | `tests/retained_lifetime_tests.cpp:134` |
| `<file/function>` | `shared_ptr` | `ProbeComponent` | local/signature/use | `tests/retained_lifetime_tests.cpp:281` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/retained_lifetime_tests.cpp:283` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/retained_lifetime_tests.cpp:285` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/retained_lifetime_tests.cpp:316` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/retained_lifetime_tests.cpp:318` |
| `<file/function>` | `weak_ptr` | `ProbeControl` | local/signature/use | `tests/retained_lifetime_tests.cpp:321` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/retained_lifetime_tests.cpp:325` |
| `<file/function>` | `weak_ptr` | `Control` | local/signature/use | `tests/retained_lifetime_tests.cpp:329` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/retained_lifetime_tests.cpp:333` |
| `<file/function>` | `weak_ptr` | `ProbeControl` | local/signature/use | `tests/retained_lifetime_tests.cpp:336` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/retained_lifetime_tests.cpp:366` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/retained_lifetime_tests.cpp:368` |
| `<file/function>` | `shared_ptr` | `ProbeComponent` | local/signature/use | `tests/retained_lifetime_tests.cpp:389` |
| `<file/function>` | `shared_ptr` | `ProbeComponent` | local/signature/use | `tests/retained_lifetime_tests.cpp:401` |
| `<file/function>` | `shared_ptr` | `ProbeControl` | local/signature/use | `tests/retained_lifetime_tests.cpp:539` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/scrollable_control_tests.cpp:40` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/scrollable_control_tests.cpp:84` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/scrollable_control_tests.cpp:85` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/scrollable_control_tests.cpp:126` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/scrollable_control_tests.cpp:127` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/scrollable_control_tests.cpp:176` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/scrollable_control_tests.cpp:177` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/semantic_tests.cpp:63` |
| `<file/function>` | `shared_ptr` | `gui_forms::GroupBox` | local/signature/use | `tests/semantic_tests.cpp:64` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/semantic_tests.cpp:66` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/semantic_tests.cpp:68` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `tests/semantic_tests.cpp:71` |
| `<file/function>` | `shared_ptr` | `gui_forms::ProgressBar` | local/signature/use | `tests/semantic_tests.cpp:74` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/semantic_tests.cpp:78` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/semantic_tests.cpp:121` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/semantic_tests.cpp:122` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `tests/semantic_tests.cpp:123` |
| `<file/function>` | `shared_ptr` | `gui_forms::TrackBar` | local/signature/use | `tests/semantic_tests.cpp:124` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/semantic_tests.cpp:127` |
| `<file/function>` | `shared_ptr` | `gui_forms::ComboBox` | local/signature/use | `tests/semantic_tests.cpp:128` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/semantic_tests.cpp:181` |
| `<file/function>` | `shared_ptr` | `gui_forms::ListBox` | local/signature/use | `tests/semantic_tests.cpp:193` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/showcase_interaction_tests.cpp:143` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/showcase_interaction_tests.cpp:150` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/showcase_interaction_tests.cpp:155` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/showcase_interaction_tests.cpp:164` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/showcase_interaction_tests.cpp:165` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/showcase_interaction_tests.cpp:190` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/showcase_interaction_tests.cpp:195` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `tests/showcase_interaction_tests.cpp:202` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `tests/showcase_interaction_tests.cpp:228` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/showcase_interaction_tests.cpp:231` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/showcase_interaction_tests.cpp:233` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/showcase_interaction_tests.cpp:235` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/showcase_interaction_tests.cpp:237` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/showcase_interaction_tests.cpp:239` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/showcase_interaction_tests.cpp:266` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/showcase_interaction_tests.cpp:268` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/showcase_interaction_tests.cpp:270` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/showcase_interaction_tests.cpp:272` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `tests/showcase_interaction_tests.cpp:307` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/showcase_interaction_tests.cpp:310` |
| `<file/function>` | `shared_ptr` | `gui_forms::ProgressBar` | local/signature/use | `tests/showcase_interaction_tests.cpp:312` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/showcase_interaction_tests.cpp:327` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `tests/showcase_interaction_tests.cpp:352` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/showcase_interaction_tests.cpp:355` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckBox` | local/signature/use | `tests/showcase_interaction_tests.cpp:357` |
| `<file/function>` | `shared_ptr` | `gui_forms::TrackBar` | local/signature/use | `tests/showcase_interaction_tests.cpp:359` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/showcase_interaction_tests.cpp:361` |
| `<file/function>` | `shared_ptr` | `gui_forms::PropertyGrid` | local/signature/use | `tests/showcase_interaction_tests.cpp:363` |
| `<file/function>` | `shared_ptr` | `gui_forms::NumericUpDown` | local/signature/use | `tests/showcase_interaction_tests.cpp:365` |
| `<file/function>` | `shared_ptr` | `NumericUpDown` | local/signature/use | `tests/showcase_interaction_tests.cpp:367` |
| `<file/function>` | `shared_ptr` | `NumericUpDown` | local/signature/use | `tests/showcase_interaction_tests.cpp:369` |
| `<file/function>` | `shared_ptr` | `NumericUpDown` | local/signature/use | `tests/showcase_interaction_tests.cpp:370` |
| `<file/function>` | `shared_ptr` | `NumericUpDown` | local/signature/use | `tests/showcase_interaction_tests.cpp:373` |
| `<file/function>` | `shared_ptr` | `gui_forms::ComboBox` | local/signature/use | `tests/showcase_interaction_tests.cpp:385` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/showcase_interaction_tests.cpp:402` |
| `<file/function>` | `shared_ptr` | `gui_forms::ColorValueEditor` | local/signature/use | `tests/showcase_interaction_tests.cpp:405` |
| `<file/function>` | `shared_ptr` | `gui_forms::FlagsValueEditor` | local/signature/use | `tests/showcase_interaction_tests.cpp:407` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `tests/showcase_interaction_tests.cpp:450` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `tests/showcase_interaction_tests.cpp:462` |
| `<file/function>` | `shared_ptr` | `gui_forms::TrackBar` | local/signature/use | `tests/showcase_interaction_tests.cpp:468` |
| `<file/function>` | `shared_ptr` | `gui_forms::ProgressBar` | local/signature/use | `tests/showcase_interaction_tests.cpp:470` |
| `<file/function>` | `shared_ptr` | `gui_forms::HScrollBar` | local/signature/use | `tests/showcase_interaction_tests.cpp:472` |
| `<file/function>` | `shared_ptr` | `gui_forms::VScrollBar` | local/signature/use | `tests/showcase_interaction_tests.cpp:474` |
| `<file/function>` | `shared_ptr` | `gui_forms::SplitContainer` | local/signature/use | `tests/showcase_interaction_tests.cpp:557` |
| `<file/function>` | `shared_ptr` | `gui_forms::ContainerControl` | local/signature/use | `tests/showcase_interaction_tests.cpp:559` |
| `<file/function>` | `shared_ptr` | `gui_forms::UserControl` | local/signature/use | `tests/showcase_interaction_tests.cpp:561` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/showcase_interaction_tests.cpp:563` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/showcase_interaction_tests.cpp:584` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/showcase_interaction_tests.cpp:585` |
| `<file/function>` | `shared_ptr` | `gui_forms::ProgressBar` | local/signature/use | `tests/showcase_interaction_tests.cpp:587` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `tests/showcase_interaction_tests.cpp:698` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/showcase_interaction_tests.cpp:702` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/showcase_interaction_tests.cpp:703` |
| `<file/function>` | `shared_ptr` | `gui_forms::ListBox` | local/signature/use | `tests/showcase_interaction_tests.cpp:745` |
| `<file/function>` | `shared_ptr` | `gui_forms::ComboBox` | local/signature/use | `tests/showcase_interaction_tests.cpp:749` |
| `<file/function>` | `qualified-alias:shared_ptr` | `Control` | local/signature/use | `tests/showcase_interaction_tests.cpp:755` |
| `<file/function>` | `shared_ptr` | `gui_forms::NumericUpDown` | local/signature/use | `tests/showcase_interaction_tests.cpp:767` |
| `<file/function>` | `shared_ptr` | `gui_forms::PictureBox` | local/signature/use | `tests/showcase_interaction_tests.cpp:777` |
| `<file/function>` | `shared_ptr` | `gui_forms::TabControl` | local/signature/use | `tests/showcase_interaction_tests.cpp:797` |
| `<file/function>` | `shared_ptr` | `gui_forms::CheckedListBox` | local/signature/use | `tests/showcase_interaction_tests.cpp:818` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `tests/showcase_interaction_tests.cpp:839` |
| `<file/function>` | `shared_ptr` | `gui_forms::DateTimePicker` | local/signature/use | `tests/showcase_interaction_tests.cpp:843` |
| `<file/function>` | `shared_ptr` | `gui_forms::DateTimePicker` | local/signature/use | `tests/showcase_interaction_tests.cpp:845` |
| `<file/function>` | `shared_ptr` | `gui_forms::DateTimePicker` | local/signature/use | `tests/showcase_interaction_tests.cpp:847` |
| `<file/function>` | `shared_ptr` | `gui_forms::DateTimePicker` | local/signature/use | `tests/showcase_interaction_tests.cpp:849` |
| `<file/function>` | `shared_ptr` | `gui_forms::DateTimePicker` | local/signature/use | `tests/showcase_interaction_tests.cpp:851` |
| `<file/function>` | `shared_ptr` | `gui_forms::DateTimePicker` | local/signature/use | `tests/showcase_interaction_tests.cpp:853` |
| `<file/function>` | `shared_ptr` | `gui_forms::DateTimePicker` | local/signature/use | `tests/showcase_interaction_tests.cpp:855` |
| `<file/function>` | `shared_ptr` | `gui_forms::DateTimePicker` | local/signature/use | `tests/showcase_interaction_tests.cpp:857` |
| `<file/function>` | `shared_ptr` | `gui_forms::DateTimePicker` | local/signature/use | `tests/showcase_interaction_tests.cpp:859` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `tests/showcase_interaction_tests.cpp:922` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/showcase_interaction_tests.cpp:931` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/showcase_interaction_tests.cpp:933` |
| `<file/function>` | `unique_ptr` | `Window` | local/signature/use | `tests/showcase_interaction_tests.cpp:979` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/showcase_interaction_tests.cpp:983` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/showcase_interaction_tests.cpp:1010` |
| `<file/function>` | `shared_ptr` | `gui_forms::SplitContainer` | local/signature/use | `tests/split_container_tests.cpp:59` |
| `<file/function>` | `shared_ptr` | `gui_forms::SplitContainer` | local/signature/use | `tests/split_container_tests.cpp:95` |
| `<file/function>` | `shared_ptr` | `gui_forms::SplitContainer` | local/signature/use | `tests/split_container_tests.cpp:151` |
| `<file/function>` | `shared_ptr` | `gui_forms::SplitContainer` | local/signature/use | `tests/split_container_tests.cpp:162` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/split_container_tests.cpp:164` |
| `<file/function>` | `shared_ptr` | `gui_forms::SplitContainer` | local/signature/use | `tests/split_container_tests.cpp:200` |
| `<file/function>` | `shared_ptr` | `gui_forms::SplitContainer` | local/signature/use | `tests/split_container_tests.cpp:218` |
| `<file/function>` | `shared_ptr` | `gui_forms::SplitContainer` | local/signature/use | `tests/split_container_tests.cpp:260` |
| `<file/function>` | `shared_ptr` | `gui_forms::SplitContainer` | local/signature/use | `tests/split_container_tests.cpp:306` |
| `<file/function>` | `shared_ptr` | `gui_forms::SplitContainer` | local/signature/use | `tests/split_container_tests.cpp:318` |
| `<file/function>` | `shared_ptr` | `TraceControl` | local/signature/use | `tests/support/headless_trace.cpp:99` |
| `<file/function>` | `shared_ptr` | `TraceControl` | local/signature/use | `tests/support/headless_trace.cpp:122` |
| `<file/function>` | `shared_ptr` | `TraceControl` | local/signature/use | `tests/support/headless_trace.cpp:124` |
| `<file/function>` | `shared_ptr` | `TraceControl` | local/signature/use | `tests/support/headless_trace.cpp:126` |
| `<file/function>` | `shared_ptr` | `TraceControl` | local/signature/use | `tests/support/headless_trace.cpp:128` |
| `DisposeCapturedControl` | `shared_ptr` | `ControlType` | local/signature/use | `tests/support/named_callbacks.hpp:118` |
| `DisposeCapturedControl` | `shared_ptr` | `ControlType` | class declaration | `tests/support/named_callbacks.hpp:126` |
| `Fixture` | `shared_ptr` | `TabControl` | class declaration | `tests/tab_control_tests.cpp:42` |
| `Fixture` | `shared_ptr` | `TabPage` | class declaration | `tests/tab_control_tests.cpp:43` |
| `Fixture` | `shared_ptr` | `TabPage` | class declaration | `tests/tab_control_tests.cpp:44` |
| `Fixture` | `shared_ptr` | `TabPage` | class declaration | `tests/tab_control_tests.cpp:45` |
| `Fixture` | `shared_ptr` | `Button` | class declaration | `tests/tab_control_tests.cpp:46` |
| `Fixture` | `shared_ptr` | `Button` | class declaration | `tests/tab_control_tests.cpp:47` |
| `<file/function>` | `shared_ptr` | `gui_forms::Label` | local/signature/use | `tests/tab_control_tests.cpp:81` |
| `<file/function>` | `shared_ptr` | `TabPage` | local/signature/use | `tests/tab_control_tests.cpp:207` |
| `<file/function>` | `shared_ptr` | `const Theme` | local/signature/use | `tests/theme_tests.cpp:99` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/theme_tests.cpp:161` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/theme_tests.cpp:162` |
| `<file/function>` | `shared_ptr` | `const Theme` | local/signature/use | `tests/theme_tests.cpp:176` |
| `<file/function>` | `shared_ptr` | `const Theme` | local/signature/use | `tests/theme_tests.cpp:188` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/theme_tests.cpp:207` |
| `<file/function>` | `shared_ptr` | `gui_forms::Button` | local/signature/use | `tests/theme_tests.cpp:208` |
| `<file/function>` | `shared_ptr` | `gui_forms::Panel` | local/signature/use | `tests/theme_tests.cpp:271` |
| `<file/function>` | `shared_ptr` | `gui_forms::TextBox` | local/signature/use | `tests/theme_tests.cpp:272` |
| `<file/function>` | `shared_ptr` | `gui_forms::ListBox` | local/signature/use | `tests/theme_tests.cpp:274` |
| `<file/function>` | `shared_ptr` | `gui_forms::ComboBox` | local/signature/use | `tests/theme_tests.cpp:278` |
| `<file/function>` | `shared_ptr` | `gui_forms::ProgressBar` | local/signature/use | `tests/theme_tests.cpp:282` |
| `<file/function>` | `shared_ptr` | `gui_forms::MenuStrip` | local/signature/use | `tests/theme_tests.cpp:286` |
| `<file/function>` | `shared_ptr` | `gui_forms::Command` | local/signature/use | `tests/theme_tests.cpp:288` |
| `Fixture` | `qualified-alias:shared_ptr` | `Control` | class declaration | `tests/timer_tests.cpp:29` |
| `<file/function>` | `shared_ptr` | `gui_forms::Control` | local/signature/use | `tests/timer_tests.cpp:161` |
| `<file/function>` | `unique_ptr` | `gui_forms::Window` | local/signature/use | `tests/timer_tests.cpp:162` |
| `Fixture` | `qualified-alias:shared_ptr` | `Control` | class declaration | `tests/tooltip_tests.cpp:49` |
| `Fixture` | `shared_ptr` | `Button` | local/signature/use | `tests/tooltip_tests.cpp:50` |
| `Fixture` | `shared_ptr` | `Button` | local/signature/use | `tests/tooltip_tests.cpp:52` |
| `<file/function>` | `unique_ptr` | `gui_forms::ToolTip` | local/signature/use | `tests/tooltip_tests.cpp:202` |
| `Fixture` | `shared_ptr` | `ContainerControl` | local/signature/use | `tests/validation_tests.cpp:30` |
| `Fixture` | `shared_ptr` | `TextBox` | local/signature/use | `tests/validation_tests.cpp:32` |
| `Fixture` | `shared_ptr` | `Button` | local/signature/use | `tests/validation_tests.cpp:34` |
| `Fixture` | `shared_ptr` | `Button` | local/signature/use | `tests/validation_tests.cpp:36` |
| `<file/function>` | `shared_ptr` | `gui_forms::ContainerControl` | local/signature/use | `tests/validation_tests.cpp:178` |
| `<file/function>` | `shared_ptr` | `gui_forms::BindingSource` | local/signature/use | `tests/validation_tests.cpp:247` |
| `<file/function>` | `shared_ptr` | `Binding` | local/signature/use | `tests/validation_tests.cpp:257` |
| `<file/function>` | `shared_ptr` | `gui_forms::host::WindowsCompatibilityPaintEndpoint` | local/signature/use | `tests/windows_live_surface_endpoint_tests.cpp:54` |
| `PolicyAction` | `unique_ptr` | `clang::ASTConsumer` | local/signature/use | `tools/house_policy_check/house_policy_check.cpp:785` |
| `PolicyActionFactory` | `unique_ptr` | `clang::FrontendAction` | local/signature/use | `tools/house_policy_check/house_policy_check.cpp:817` |

## Raw-pointer plumbing

Raw pointers are recorded because a future lifecycle round must distinguish non-owning direct access, platform handles, optional links, array traversal, and accidental ownership. Their presence is not a defect under the house policy.

| Owner/scope | Target | Scope | Location | Evidence |
|---|---|---|---|---|
| `<file/function>` | `const NodeSpec` | local/signature/use | `demo/gallery_dml.hpp:158` | `[[nodiscard]] constexpr const NodeSpec* find(std::string_view id) noexcept` |
| `<file/function>` | `NodeSpec` | local/signature/use | `demo/gallery_model.cpp:35` | `const dml::NodeSpec* const node = dml::find(stable_id);` |
| `PaintRequest` | `Window` | local/signature/use | `demo/vsync_lab.cpp:157` | `Window* owner = (*target).window();` |
| `WaterfallSurface` | `std::byte` | local/signature/use | `demo/vsync_lab.cpp:229` | `std::byte* const waterfall =` |
| `Binding` | `Control` | class declaration | `include/gui_forms/binding/binding/binding.hpp:25` | `[[nodiscard]] Control* target() const noexcept { return target_; }` |
| `Binding` | `Control` | class declaration | `include/gui_forms/binding/binding/binding.hpp:85` | `Control* target_{};` |
| `SourceDisposedCallback` | `BindingContext` | class declaration | `include/gui_forms/binding/binding_context/binding_context.hpp:42` | `BindingContext* context{};` |
| `SourceDisposedCallback` | `BindingSource` | class declaration | `include/gui_forms/binding/binding_context/binding_context.hpp:43` | `BindingSource* source{};` |
| `BindingContext` | `Window` | class declaration | `include/gui_forms/binding/binding_context/binding_context.hpp:49` | `[[nodiscard]] Window* bound_window() const noexcept;` |
| `BindingContext` | `BindingSource` | class declaration | `include/gui_forms/binding/binding_context/binding_context.hpp:50` | `bool remove_entry(BindingSource* source, bool publish);` |
| `BindingManagerBase` | `const BindingRecord` | class declaration | `include/gui_forms/binding/binding_manager_base/binding_manager_base.hpp:16` | `[[nodiscard]] virtual const BindingRecord* current() const noexcept = 0;` |
| `BindingSource` | `const BindingRecord` | class declaration | `include/gui_forms/binding/binding_source/binding_source.hpp:44` | `[[nodiscard]] const BindingRecord* current() const noexcept;` |
| `BindingSource` | `const Binding` | class declaration | `include/gui_forms/binding/binding_source/binding_source.hpp:135` | `void unregister_binding(const Binding* binding) noexcept;` |
| `BindingSource` | `Window` | class declaration | `include/gui_forms/binding/binding_source/binding_source.hpp:138` | `[[nodiscard]] Window* bound_window() const noexcept;` |
| `ControlBindingsCollection` | `Control` | class declaration | `include/gui_forms/binding/control_bindings_collection/control_bindings_collection.hpp:51` | `Control* target_{};` |
| `CurrencyManager` | `const BindingRecord` | class declaration | `include/gui_forms/binding/currency_manager/currency_manager.hpp:14` | `[[nodiscard]] const BindingRecord* current() const noexcept override;` |
| `CurrencyManager` | `BindingSource` | class declaration | `include/gui_forms/binding/currency_manager/currency_manager.hpp:39` | `BindingSource* source_{};` |
| `BindingCompleteEvent` | `Binding` | class declaration | `include/gui_forms/binding/types/binding_contract_types.hpp:89` | `Binding* binding{};` |
| `BindingContextChange` | `BindingSource` | class declaration | `include/gui_forms/binding/types/binding_contract_types.hpp:133` | `BindingSource* source{};` |
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
| `ErrorProvider` | `Window` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:150` | `[[nodiscard]] Window* bound_window() const noexcept;` |
| `ErrorProvider` | `Entry` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:152` | `[[nodiscard]] Entry* find_entry(const Control& target);` |
| `ErrorProvider` | `const Entry` | class declaration | `include/gui_forms/components/error_provider/error_provider.hpp:153` | `[[nodiscard]] const Entry* find_entry(const Control& target) const;` |
| `FocusedHelpAccelerator` | `HelpProvider` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:100` | `HelpProvider* provider{};` |
| `HelpProvider` | `Window` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:108` | `[[nodiscard]] Window* bound_window() const noexcept;` |
| `HelpProvider` | `Entry` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:110` | `[[nodiscard]] Entry* find_entry(const Control& target);` |
| `HelpProvider` | `const Entry` | class declaration | `include/gui_forms/components/help_provider/help_provider.hpp:111` | `[[nodiscard]] const Entry* find_entry(const Control& target) const;` |
| `ToolTip` | `Window` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:88` | `[[nodiscard]] Window* bound_window() const noexcept;` |
| `ToolTip` | `Entry` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:90` | `[[nodiscard]] Entry* find_entry(const Control& target);` |
| `ToolTip` | `const Entry` | class declaration | `include/gui_forms/components/tool_tip/tool_tip.hpp:91` | `[[nodiscard]] const Entry* find_entry(const Control& target) const;` |
| `DeferredEventPublication` | `EventType` | class declaration | `include/gui_forms/control/control/control.hpp:66` | `EventType* event_{};` |
| `ControlValidationEvent` | `Control` | class declaration | `include/gui_forms/control/control/control.hpp:182` | `Control* control{};` |
| `ControlValidationEvent` | `Control` | class declaration | `include/gui_forms/control/control/control.hpp:183` | `Control* destination{};` |
| `Control` | `Window` | class declaration | `include/gui_forms/control/control/control.hpp:392` | `[[nodiscard]] Window* attached_window() const noexcept { return window_; }` |
| `Control` | `Window` | class declaration | `include/gui_forms/control/control/control.hpp:600` | `[[nodiscard]] Window* window() const noexcept { return window_; }` |
| `Control` | `Control` | class declaration | `include/gui_forms/control/control/control.hpp:679` | `[[nodiscard]] bool perform_validation(Control* destination, bool bulk);` |
| `Control` | `const BindableProperty` | class declaration | `include/gui_forms/control/control/control.hpp:683` | `[[nodiscard]] const BindableProperty* find_bindable_property(` |
| `Control` | `const void` | class declaration | `include/gui_forms/control/control/control.hpp:688` | `void publish_change(const void* event_key,` |
| `DeferredInitializationChange` | `const void` | class declaration | `include/gui_forms/control/control/control.hpp:712` | `const void* event_key{};` |
| `Control` | `Window` | class declaration | `include/gui_forms/control/control/control.hpp:728` | `Window* window_{};` |
| `RegisteredPropertyGetter` | `Control` | class declaration | `include/gui_forms/control/control/control.hpp:828` | `Control* control{};` |
| `RegisteredPropertySetter` | `Control` | class declaration | `include/gui_forms/control/control/control.hpp:893` | `Control* control{};` |
| `RegisteredPropertyConnector` | `Control` | class declaration | `include/gui_forms/control/control/control.hpp:986` | `Control* control{};` |
| `TableLayoutPanel` | `const CellMetadata` | class declaration | `include/gui_forms/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.hpp:122` | `[[nodiscard]] const CellMetadata* metadata_for(const Control& child) const;` |
| `ScrollProperties` | `ScrollableControl` | class declaration | `include/gui_forms/controls/scrollable_control/scroll_properties/scroll_properties.hpp:87` | `ScrollableControl* owner_{};` |
| `ScrollableControl` | `const char` | class declaration | `include/gui_forms/controls/scrollable_control/scrollable_control.hpp:146` | `static void validate_size(Size size, const char* message);` |
| `ScrollableControl` | `const char` | class declaration | `include/gui_forms/controls/scrollable_control/scrollable_control.hpp:147` | `static void validate_axis_value(double value, const char* message);` |
| `BoundMemberFunction` | `Object` | class declaration | `include/gui_forms/detail/bound_member_function.hpp:27` | `Object* object_{};` |
| `BoundMemberFunction` | `const Object` | class declaration | `include/gui_forms/detail/bound_member_function.hpp:45` | `const Object* object_{};` |
| `BoundMemberFunction` | `Object` | class declaration | `include/gui_forms/detail/bound_member_function.hpp:63` | `Object* object_{};` |
| `BoundMemberFunction` | `const Object` | class declaration | `include/gui_forms/detail/bound_member_function.hpp:82` | `const Object* object_{};` |
| `BindingMemberGetter` | `const Object` | class declaration | `include/gui_forms/detail/property_binding_adapters.hpp:29` | `const Object* object_{};` |
| `BindingMethodGetter` | `const Object` | class declaration | `include/gui_forms/detail/property_binding_adapters.hpp:46` | `const Object* object_{};` |
| `BindingNoexceptMethodGetter` | `const Object` | class declaration | `include/gui_forms/detail/property_binding_adapters.hpp:63` | `const Object* object_{};` |
| `ConvertedPropertySetter` | `const char` | class declaration | `include/gui_forms/detail/property_binding_adapters.hpp:73` | `BindingValueKind kind, const char* error) noexcept` |
| `ConvertedPropertySetter` | `Object` | class declaration | `include/gui_forms/detail/property_binding_adapters.hpp:84` | `Object* object_{};` |
| `ConvertedPropertySetter` | `const char` | class declaration | `include/gui_forms/detail/property_binding_adapters.hpp:87` | `const char* error_{};` |
| `DirectPropertySetter` | `Object` | class declaration | `include/gui_forms/detail/property_binding_adapters.hpp:103` | `Object* object_{};` |
| `EventChangeConnector` | `Event<EventArguments...>` | class declaration | `include/gui_forms/detail/property_binding_adapters.hpp:133` | `Event<EventArguments...>* event_{};` |
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
| `Delegate` | `Object::` | class declaration | `include/gui_forms/event/delegate/delegate.hpp:17` | `template <typename Object, void (Object::*Method)(Arguments...)>` |
| `Delegate` | `Object::` | class declaration | `include/gui_forms/event/delegate/delegate.hpp:23` | `template <typename Object, void (Object::*Method)(Arguments...) const>` |
| `Delegate` | `void` | class declaration | `include/gui_forms/event/delegate/delegate.hpp:45` | `constexpr Delegate(void* context, Thunk thunk) noexcept` |
| `Delegate` | `Object::` | class declaration | `include/gui_forms/event/delegate/delegate.hpp:48` | `template <typename Object, void (Object::*Method)(Arguments...)>` |
| `Delegate` | `void` | class declaration | `include/gui_forms/event/delegate/delegate.hpp:49` | `static void invoke_member(void* context, Arguments... arguments) {` |
| `Delegate` | `Object::` | class declaration | `include/gui_forms/event/delegate/delegate.hpp:54` | `template <typename Object, void (Object::*Method)(Arguments...) const>` |
| `Delegate` | `void` | class declaration | `include/gui_forms/event/delegate/delegate.hpp:55` | `static void invoke_const_member(void* context, Arguments... arguments) {` |
| `Delegate` | `void` | class declaration | `include/gui_forms/event/delegate/delegate.hpp:65` | `void* context_{};` |
| `Event` | `Component` | class declaration | `include/gui_forms/event/event/event.hpp:166` | `[[nodiscard]] SubscriptionToken subscribe_impl(Component* owner, Callback callback) {` |
| `Event` | `Component` | class declaration | `include/gui_forms/event/event/event.hpp:176` | `[[nodiscard]] SubscriptionToken subscribe_impl(Component* owner,` |
| `Event` | `Component` | class declaration | `include/gui_forms/event/event/event.hpp:187` | `void connect(Component* owner, const std::shared_ptr<Slot>& slot) {` |
| `SemanticFeedback` | `Window` | class declaration | `include/gui_forms/feedback/semantic_feedback/semantic_feedback.hpp:45` | `Window* window_{};` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/feedback/types/feedback_types.hpp:40` | `[[nodiscard]] const char* semantic_feedback_kind_name(` |
| `HostSession` | `HostServices` | class declaration | `include/gui_forms/host/session/host_session.hpp:18` | `HostServices* services = nullptr);` |
| `DispatchVisitor` | `HostSession` | class declaration | `include/gui_forms/host/session/host_session.hpp:40` | `HostSession* session{};` |
| `DispatchVisitor` | `HostDispatchResult` | class declaration | `include/gui_forms/host/session/host_session.hpp:41` | `HostDispatchResult* result{};` |
| `DispatchVisitor` | `HostEvent` | class declaration | `include/gui_forms/host/session/host_session.hpp:42` | `HostEvent* event{};` |
| `HostSession` | `Window` | class declaration | `include/gui_forms/host/session/host_session.hpp:48` | `Window* window_{};` |
| `HostSession` | `HostServices` | class declaration | `include/gui_forms/host/session/host_session.hpp:49` | `HostServices* services_{};` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/host/types/host_types.hpp:315` | `[[nodiscard]] const char* host_dispatch_error_name(HostDispatchError error) noexcept;` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/host/types/host_types.hpp:316` | `[[nodiscard]] const char* host_lifecycle_phase_name(HostLifecyclePhase phase) noexcept;` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/host/types/host_types.hpp:317` | `[[nodiscard]] const char* host_event_name(const HostEventPayload& payload) noexcept;` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/host/types/host_types.hpp:318` | `[[nodiscard]] const char* host_service_error_name(HostServiceError error) noexcept;` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/host/types/host_types.hpp:319` | `[[nodiscard]] const char* cursor_kind_name(CursorKind cursor) noexcept;` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/host/types/host_types.hpp:320` | `[[nodiscard]] const char* drag_effect_name(DragEffect effect) noexcept;` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/host/types/host_types.hpp:321` | `[[nodiscard]] const char* host_dialog_kind_name(` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/host/types/host_types.hpp:323` | `[[nodiscard]] const char* host_dialog_outcome_name(HostDialogOutcome outcome) noexcept;` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/host/types/host_types.hpp:324` | `[[nodiscard]] const char* host_dialog_choice_name(HostDialogChoice choice) noexcept;` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/host/types/host_types.hpp:325` | `[[nodiscard]] const char* host_sound_cue_name(HostSoundCue cue) noexcept;` |
| `ImageList` | `Window` | class declaration | `include/gui_forms/image_list/image_list/image_list.hpp:70` | `[[nodiscard]] Window* bound_window() const noexcept;` |
| `PropertyValueConverterRegistry` | `const PropertyValueConverter` | class declaration | `include/gui_forms/inspection/property_value_converter_registry/property_value_converter_registry.hpp:58` | `[[nodiscard]] const PropertyValueConverter* find(` |
| `WindowLifetime` | `Window` | class declaration | `include/gui_forms/scheduler/types/scheduler_types.hpp:16` | `Window* window{};` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/semantics/types/semantic_types.hpp:95` | `[[nodiscard]] const char* semantic_role_name(SemanticRole role) noexcept;` |
| `<file/function>` | `const char` | local/signature/use | `include/gui_forms/semantics/types/semantic_types.hpp:96` | `[[nodiscard]] const char* semantic_action_name(SemanticAction action) noexcept;` |
| `CallbackState` | `Timer` | class declaration | `include/gui_forms/timer/timer/timer.hpp:42` | `Timer* owner{};` |
| `Timer` | `Window` | class declaration | `include/gui_forms/timer/timer/timer.hpp:50` | `[[nodiscard]] Window* bound_window() const noexcept;` |
| `UpdateScope` | `Window` | class declaration | `include/gui_forms/window/update_scope/update_scope.hpp:22` | `Window* window_{};` |
| `Window` | `HostServices` | class declaration | `include/gui_forms/window/window.hpp:268` | `[[nodiscard]] HostServices* host_services() const noexcept {` |
| `Window` | `Control` | class declaration | `include/gui_forms/window/window.hpp:377` | `Control* destination = nullptr,` |
| `FocusChangePublication` | `Window` | class declaration | `include/gui_forms/window/window.hpp:480` | `Window* window{};` |
| `FocusScopePublication` | `Window` | class declaration | `include/gui_forms/window/window.hpp:486` | `Window* window{};` |
| `PointerCapturePublication` | `Window` | class declaration | `include/gui_forms/window/window.hpp:493` | `Window* window{};` |
| `ControlAvailabilityPublication` | `Window` | class declaration | `include/gui_forms/window/window.hpp:499` | `Window* window{};` |
| `DeferredInputVisitor` | `Window` | class declaration | `include/gui_forms/window/window.hpp:505` | `Window* window{};` |
| `Window` | `HostServices` | class declaration | `include/gui_forms/window/window.hpp:785` | `HostServices* host_services_{};` |
| `<file/function>` | `char` | local/signature/use | `src/abi/c_api.cpp:73` | `char* buffer,` |
| `<file/function>` | `void` | local/signature/use | `src/abi/c_api.cpp:103` | `void* context,` |
| `<file/function>` | `char` | local/signature/use | `src/abi/c_api.cpp:114` | `gf_result api_get_name(gf_handle handle, char* buffer, std::uint64_t capacity,` |
| `<file/function>` | `char` | local/signature/use | `src/abi/c_api.cpp:121` | `gf_result api_get_text(gf_handle handle, char* buffer, std::uint64_t capacity,` |
| `<file/function>` | `char` | local/signature/use | `src/abi/c_api.cpp:229` | `gf_result api_last_host_trace(gf_handle handle, char* buffer,` |
| `<file/function>` | `void` | local/signature/use | `src/abi/c_api.cpp:235` | `gf_event_callback_v2 callback, void* context,` |
| `<file/function>` | `void` | local/signature/use | `src/abi/c_api.cpp:240` | `void* context) noexcept {` |
| `<file/function>` | `void` | local/signature/use | `src/abi/c_api.cpp:292` | `void* context, gf_event_token* token) noexcept {` |
| `<file/function>` | `void` | local/signature/use | `src/abi/c_api.cpp:302` | `void* context, gf_event_token* token) noexcept {` |
| `<file/function>` | `void` | local/signature/use | `src/abi/c_api.cpp:306` | `void* context,` |
| `<file/function>` | `void` | local/signature/use | `src/abi/c_api.cpp:311` | `void* context, gf_event_token* token) noexcept {` |
| `<file/function>` | `char` | local/signature/use | `src/abi/c_api.cpp:345` | `gf_result api_last_dialog_path(gf_handle owner, char* buffer,` |
| `<file/function>` | `char` | local/signature/use | `src/abi/c_api.cpp:383` | `gf_result api_read_clipboard_text(gf_handle owner, char* buffer,` |
| `<file/function>` | `char` | local/signature/use | `src/abi/c_api.cpp:467` | `std::uint64_t endpoint, char* buffer, std::uint64_t capacity,` |
| `<file/function>` | `const void` | local/signature/use | `src/abi/c_api.cpp:495` | `std::uint64_t row_bytes, const void* pixels) {` |
| `AbiPropertyObjectControl` | `Color` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:422` | `} else if (const gui_forms::Color* item = std::get_if<gui_forms::Color>(&value)) {` |
| `AbiPropertyObjectControl` | `PropertyEnumValue` | local/signature/use | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:428` | `} else if (const gui_forms::PropertyEnumValue* item =` |
| `GetterTextInvoke` | `const PropertyState` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:617` | `const PropertyState* state{};` |
| `GetterTextInvoke` | `char` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:621` | `char* buffer, std::uint64_t capacity,` |
| `FormatterTextInvoke` | `const PropertyState` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:630` | `const PropertyState* state{};` |
| `FormatterTextInvoke` | `char` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:634` | `char* buffer, std::uint64_t capacity,` |
| `ParserTextInvoke` | `const PropertyState` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:643` | `const PropertyState* state{};` |
| `ParserTextInvoke` | `char` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:648` | `char* buffer, std::uint64_t capacity,` |
| `EditorTextInvoke` | `const PropertyState` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:657` | `const PropertyState* state{};` |
| `EditorTextInvoke` | `char` | class declaration | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:662` | `char* buffer, std::uint64_t capacity,` |
| `RasterControl` | `Window` | local/signature/use | `src/abi/control_adapters/raster_control/raster_control.hpp:268` | `if (Window* owner = (*target).window();` |
| `Registry` | `Slot` | local/signature/use | `src/abi/drawing_c_api.cpp:141` | `Slot* slot = find_locked(handle);` |
| `Registry` | `Slot` | local/signature/use | `src/abi/drawing_c_api.cpp:161` | `Slot* slot = find_locked(handle);` |
| `Registry` | `ObjectRecord` | local/signature/use | `src/abi/drawing_c_api.cpp:304` | `for (ObjectRecord* record : {recorder_record.get(), font_record.get(),` |
| `Registry` | `Slot` | local/signature/use | `src/abi/drawing_c_api.cpp:359` | `Slot* slot = find_locked(handle);` |
| `Registry` | `Slot` | class declaration | `src/abi/drawing_c_api.cpp:398` | `Slot* find_locked(gd_handle handle) noexcept {` |
| `<file/function>` | `char` | local/signature/use | `src/abi/drawing_c_api.cpp:586` | `char* buffer, std::uint64_t capacity,` |
| `<file/function>` | `GraphicsRecorder` | local/signature/use | `src/abi/drawing_c_api.cpp:658` | `gui_drawing::GraphicsRecorder* graphics,` |
| `<file/function>` | `Pen` | local/signature/use | `src/abi/drawing_c_api.cpp:659` | `gui_drawing::Pen* stroke) {` |
| `<file/function>` | `GraphicsRecorder` | local/signature/use | `src/abi/drawing_c_api.cpp:672` | `gui_drawing::GraphicsRecorder* graphics,` |
| `<file/function>` | `Brush` | local/signature/use | `src/abi/drawing_c_api.cpp:673` | `gui_drawing::Brush* fill) {` |
| `<file/function>` | `GraphicsRecorder` | local/signature/use | `src/abi/drawing_c_api.cpp:686` | `gui_drawing::GraphicsRecorder* graphics,` |
| `<file/function>` | `ImageReference` | local/signature/use | `src/abi/drawing_c_api.cpp:687` | `gui_drawing::ImageReference* image, gd_rect destination, gd_rect source) {` |
| `<file/function>` | `GraphicsRecorder` | local/signature/use | `src/abi/drawing_c_api.cpp:823` | `gui_drawing::GraphicsRecorder* graphics, gui_drawing::Bitmap* bitmap,` |
| `<file/function>` | `Bitmap` | local/signature/use | `src/abi/drawing_c_api.cpp:823` | `gui_drawing::GraphicsRecorder* graphics, gui_drawing::Bitmap* bitmap,` |
| `<file/function>` | `void` | local/signature/use | `src/abi/drawing_c_api.cpp:1055` | `void* buffer, std::uint64_t capacity,` |
| `<file/function>` | `char` | local/signature/use | `src/abi/drawing_c_api.cpp:1375` | `gd_result api_recorder_trace(gd_handle handle, char* buffer,` |
| `<file/function>` | `void` | local/signature/use | `src/abi/drawing_c_api.cpp:1919` | `gd_result api_bitmap_encode_png(gd_handle bitmap, void* buffer,` |
| `<file/function>` | `const void` | local/signature/use | `src/abi/drawing_c_api.cpp:1934` | `gd_result api_bitmap_decode_png(const void* data, std::uint64_t size,` |
| `<file/function>` | `void` | local/signature/use | `src/abi/drawing_c_api.cpp:1944` | `void* decoded = nullptr;` |
| `HdcLease` | `Bitmap` | class declaration | `src/abi/drawing_platform_windows.cpp:22` | `Bitmap* bitmap{};` |
| `HdcLease` | `void` | class declaration | `src/abi/drawing_platform_windows.cpp:27` | `void* pixels{};` |
| `CaptureStaging` | `void` | class declaration | `src/abi/drawing_platform_windows.cpp:72` | `void* pixels{};` |
| `<file/function>` | `void` | local/signature/use | `src/abi/drawing_platform_windows.cpp:132` | `void copy_as_bgra(const ImageSnapshot& snapshot, void* destination) {` |
| `<file/function>` | `std::byte` | local/signature/use | `src/abi/drawing_platform_windows.cpp:134` | `std::byte* output = static_cast<std::byte*>(destination);` |
| `<file/function>` | `void` | local/signature/use | `src/abi/drawing_platform_windows.cpp:148` | `void* pixels = nullptr;` |
| `<file/function>` | `void` | local/signature/use | `src/abi/drawing_platform_windows.cpp:246` | `void* pixels = nullptr;` |
| `<file/function>` | `const std::byte` | local/signature/use | `src/abi/drawing_platform_windows.cpp:327` | `const std::byte* source_bytes = static_cast<const std::byte*>(capture_staging.pixels);` |
| `<file/function>` | `std::byte` | local/signature/use | `src/abi/drawing_platform_windows.cpp:329` | `std::byte* destination = lock.writable_data +` |
| `<file/function>` | `const void` | local/signature/use | `src/abi/drawing_platform_windows.cpp:383` | `const void* pixels = snapshot.pixels().data();` |
| `<file/function>` | `void` | local/signature/use | `src/abi/drawing_platform_windows.cpp:417` | `void* pixels = nullptr;` |
| `<file/function>` | `const char` | local/signature/use | `src/abi/drawing_skia_c_api.cpp:101` | `if (const char* trace = std::getenv("GUI_DRAWING_TRACE_FONTS");` |
| `<file/function>` | `const char` | local/signature/use | `src/abi/drawing_skia_c_api.cpp:235` | `if (const char* trace = std::getenv("GUI_DRAWING_TRACE_FONTS");` |
| `<file/function>` | `const void` | local/signature/use | `src/abi/drawing_skia_c_api.cpp:279` | `gd_result execute(const void* recorder, void* bitmap,` |
| `<file/function>` | `void` | local/signature/use | `src/abi/drawing_skia_c_api.cpp:279` | `gd_result execute(const void* recorder, void* bitmap,` |
| `<file/function>` | `const void` | local/signature/use | `src/abi/drawing_skia_c_api.cpp:302` | `gd_result encode_png(const void* bitmap, void* buffer, std::uint64_t capacity,` |
| `<file/function>` | `void` | local/signature/use | `src/abi/drawing_skia_c_api.cpp:302` | `gd_result encode_png(const void* bitmap, void* buffer, std::uint64_t capacity,` |
| `<file/function>` | `const void` | local/signature/use | `src/abi/drawing_skia_c_api.cpp:323` | `gd_result decode_png(const void* data, std::uint64_t size, void** bitmap) {` |
| `<file/function>` | `const void` | local/signature/use | `src/abi/drawing_skia_c_api.cpp:340` | `gd_result measure_string(const void* font, const void* format,` |
| `<file/function>` | `const void` | local/signature/use | `src/abi/drawing_skia_c_api.cpp:340` | `gd_result measure_string(const void* font, const void* format,` |
| `<file/function>` | `char` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:165` | `std::uint64_t token, char* buffer, std::uint64_t capacity,` |
| `<file/function>` | `const void` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:188` | `std::uint64_t row_bytes, const void* pixels) {` |
| `<file/function>` | `char` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.hpp:37` | `std::uint64_t token, char* buffer, std::uint64_t capacity,` |
| `<file/function>` | `const void` | local/signature/use | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.hpp:41` | `std::uint32_t height, std::uint64_t row_bytes, const void* pixels);` |
| `SubscriptionRecord` | `void` | class declaration | `src/abi/registry/registry.hpp:23` | `void* context{};` |
| `DispatchRecord` | `void` | class declaration | `src/abi/registry/registry.hpp:32` | `void* context{};` |
| `Registry` | `char` | class declaration | `src/abi/registry/registry.hpp:255` | `gf_result get_string(gf_handle handle, char* buffer, std::uint64_t capacity,` |
| `Registry` | `char` | class declaration | `src/abi/registry/registry.hpp:1209` | `gf_result read_clipboard_text(gf_handle owner_handle, char* buffer,` |
| `Registry` | `HostPathDialogResult` | local/signature/use | `src/abi/registry/registry.hpp:1583` | `const gui_forms::HostPathDialogResult* paths =` |
| `Registry` | `char` | class declaration | `src/abi/registry/registry.hpp:1598` | `gf_result last_dialog_path(gf_handle owner_handle, char* buffer,` |
| `Registry` | `char` | class declaration | `src/abi/registry/registry.hpp:1844` | `gf_result last_host_trace(gf_handle handle, char* buffer,` |
| `Registry` | `ComponentState::alive:` | local/signature/use | `src/abi/registry/registry.hpp:1954` | `case ComponentState::alive: *output = GF_COMPONENT_ALIVE; break;` |
| `Registry` | `ComponentState::disposing:` | local/signature/use | `src/abi/registry/registry.hpp:1955` | `case ComponentState::disposing: *output = GF_COMPONENT_DISPOSING; break;` |
| `Registry` | `ComponentState::disposed:` | local/signature/use | `src/abi/registry/registry.hpp:1956` | `case ComponentState::disposed: *output = GF_COMPONENT_DISPOSED; break;` |
| `Registry` | `char` | class declaration | `src/abi/registry/registry.hpp:1962` | `char* buffer,` |
| `Registry` | `Window` | local/signature/use | `src/abi/registry/registry.hpp:2117` | `Window* const window = (*(*owner).control).attached_window();` |
| `Registry` | `void` | class declaration | `src/abi/registry/registry.hpp:2160` | `void* context,` |
| `Registry` | `void` | class declaration | `src/abi/registry/registry.hpp:2201` | `void* context,` |
| `Registry` | `void` | class declaration | `src/abi/registry/registry.hpp:2284` | `gf_pointer_callback callback, void* context,` |
| `Registry` | `void` | class declaration | `src/abi/registry/registry.hpp:2322` | `gf_key_callback callback, void* context,` |
| `Registry` | `void` | class declaration | `src/abi/registry/registry.hpp:2352` | `gf_key_callback callback, void* context,` |
| `Registry` | `void` | class declaration | `src/abi/registry/registry.hpp:2382` | `gf_text_callback callback, void* context,` |
| `Registry` | `void` | class declaration | `src/abi/registry/registry.hpp:2411` | `void* context) {` |
| `HostFinishGuard` | `Registry` | class declaration | `src/abi/registry/registry.hpp:2491` | `Registry* registry{};` |
| `ManagedCallbackGuard` | `Registry` | class declaration | `src/abi/registry/registry.hpp:2497` | `Registry* registry{};` |
| `DispatchGuard` | `Registry` | class declaration | `src/abi/registry/registry.hpp:2503` | `Registry* registry{};` |
| `CloseRequestCallback` | `Registry` | class declaration | `src/abi/registry/registry.hpp:2522` | `Registry* registry{};` |
| `HostReadyCallback` | `Registry` | class declaration | `src/abi/registry/registry.hpp:2532` | `Registry* registry{};` |
| `WindowsDispatchPendingCallback` | `Registry` | class declaration | `src/abi/registry/registry.hpp:2554` | `Registry* registry{};` |
| `DispatchPendingCallback` | `Registry` | class declaration | `src/abi/registry/registry.hpp:2566` | `Registry* registry{};` |
| `HostClosedCallback` | `Registry` | class declaration | `src/abi/registry/registry.hpp:2573` | `Registry* registry{};` |
| `FinalSnapshotCallback` | `Registry` | class declaration | `src/abi/registry/registry.hpp:2585` | `Registry* registry{};` |
| `EmitV2Subscription` | `Registry` | class declaration | `src/abi/registry/registry.hpp:2599` | `Registry* registry{};` |
| `KeyPreviewForwarder` | `Registry` | class declaration | `src/abi/registry/registry.hpp:2676` | `Registry* registry{};` |
| `Registry` | `RegistrySlot` | local/signature/use | `src/abi/registry/registry.hpp:2790` | `RegistrySlot* slot = slot_locked(token);` |
| `Registry` | `const char` | local/signature/use | `src/abi/registry/registry.hpp:2980` | `const char* phase = "idle";` |
| `Registry` | `RegistrySlot` | local/signature/use | `src/abi/registry/registry.hpp:3083` | `RegistrySlot* slot = slot_locked(handle);` |
| `Registry` | `RegistrySlot` | local/signature/use | `src/abi/registry/registry.hpp:3099` | `RegistrySlot* slot = slot_locked(token);` |
| `Registry` | `RegistrySlot` | class declaration | `src/abi/registry/registry.hpp:3111` | `RegistrySlot* slot_locked(gf_handle handle) {` |
| `Registry` | `RegistrySlot` | local/signature/use | `src/abi/registry/registry.hpp:3143` | `if (RegistrySlot* slot = slot_locked(token);` |
| `Registry` | `RegistrySlot` | local/signature/use | `src/abi/registry/registry.hpp:3154` | `RegistrySlot* slot = slot_locked(handle);` |
| `Registry` | `RegistrySlot` | local/signature/use | `src/abi/registry/registry.hpp:3174` | `RegistrySlot* slot = slot_locked(token);` |
| `Registry` | `void` | local/signature/use | `src/abi/registry/registry.hpp:3187` | `void* const context = (*subscription).context;` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/button_base/button/button.cpp:58` | `Window* owner = attached_window();` |
| `<file/function>` | `const char` | local/signature/use | `src/controls/commands/command/command.cpp:35` | `void require_command_text(std::string_view value, const char* field) {` |
| `<file/function>` | `const char` | local/signature/use | `src/controls/easing_preview/easing_preview.cpp:138` | `const char* name = !policy.enabled` |
| `GalleryContext` | `Window` | class declaration | `src/controls/gallery/context/gallery_context.hpp:14` | `Window* window{};` |
| `GalleryControl` | `NodeSpec` | class declaration | `src/controls/gallery/control/gallery_control.hpp:34` | `const dml::NodeSpec* specification_{};` |
| `GalleryCheckBox` | `const char` | local/signature/use | `src/controls/gallery_controls.cpp:267` | `const char* state = check_state() == CheckState::checked` |
| `<file/function>` | `NodeSpec` | local/signature/use | `src/controls/gallery_controls.cpp:665` | `const dml::NodeSpec* gallery_child = dml::find((*child).stable_id().value());` |
| `<file/function>` | `NodeSpec` | local/signature/use | `src/controls/gallery_controls.cpp:694` | `const dml::NodeSpec* gallery_child = dml::find((*child).stable_id().value());` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_glyph/error_glyph.cpp:94` | `Window* owner = attached_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_glyph/error_glyph.cpp:129` | `Window* owner = attached_window();` |
| `ErrorProvider` | `ErrorProvider` | class declaration | `src/controls/guidance/error_provider/error_provider.cpp:36` | `ErrorProvider* provider{};` |
| `ErrorProvider` | `Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:42` | `Entry* entry = (*provider).find_entry(*retained);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:95` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:109` | `Window* ErrorProvider::bound_window() const noexcept {` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:118` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:126` | `Window* owner = bound_window();` |
| `<file/function>` | `ErrorProvider::Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:133` | `ErrorProvider::Entry* ErrorProvider::find_entry(const Control& target) {` |
| `<file/function>` | `const ErrorProvider::Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:138` | `const ErrorProvider::Entry* ErrorProvider::find_entry(const Control& target) const {` |
| `<file/function>` | `Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:150` | `if (Entry* existing = find_entry(*target)) return *existing;` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:183` | `if (Window* owner = bound_window()) (*owner).verify_access("ErrorProvider error query");` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:184` | `const Entry* entry = find_entry(target);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:202` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:228` | `if (Window* owner = bound_window()) {` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:231` | `const Entry* entry = find_entry(target);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:249` | `if (Window* owner = bound_window()) {` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:252` | `const Entry* entry = find_entry(target);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:299` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:311` | `Window* owner = bound_window();` |
| `<file/function>` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:419` | `Control* raw = (*event.binding).target();` |
| `<file/function>` | `Control` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:450` | `Control* raw = binding ? (*binding).target() : nullptr;` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:512` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:565` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:622` | `if (Window* owner = bound_window()) (*owner).verify_access("ErrorProvider snapshot");` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/error_provider/error_provider.cpp:649` | `if (Window* owner = bound_window()) (*owner).verify_access("ErrorProvider disposal");` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:57` | `Window* HelpProvider::bound_window() const noexcept {` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:66` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:74` | `Window* owner = bound_window();` |
| `<file/function>` | `HelpProvider::Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:81` | `HelpProvider::Entry* HelpProvider::find_entry(const Control& target) {` |
| `<file/function>` | `const HelpProvider::Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:86` | `const HelpProvider::Entry* HelpProvider::find_entry(const Control& target) const {` |
| `<file/function>` | `Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:98` | `if (Entry* existing = find_entry(*target)) return *existing;` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:141` | `if (Window* owner = bound_window()) (*owner).verify_access("HelpProvider string query");` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:142` | `const Entry* entry = find_entry(target);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:160` | `if (Window* owner = bound_window()) (*owner).verify_access("HelpProvider keyword query");` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:161` | `const Entry* entry = find_entry(target);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:177` | `if (Window* owner = bound_window()) (*owner).verify_access("HelpProvider navigator query");` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:178` | `const Entry* entry = find_entry(target);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:191` | `if (Window* owner = bound_window()) (*owner).verify_access("HelpProvider show-help query");` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:192` | `const Entry* entry = find_entry(target);` |
| `<file/function>` | `Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:198` | `Entry* entry = find_entry(target);` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:235` | `const Entry* entry = find_entry(*target);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:254` | `Window* owner = bound_window();` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:259` | `const Entry* entry = find_entry(*current);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/guidance/help_provider/help_provider.cpp:295` | `if (Window* owner = bound_window()) (*owner).verify_access("HelpProvider disposal");` |
| `FlagsEditorSynchronizer` | `const PropertyEnumValue` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:64` | `const PropertyEnumValue* flags = std::get_if<PropertyEnumValue>(&value);` |
| `ColorEditorSynchronizer` | `const Color` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:103` | `const Color* color = std::get_if<Color>(&value);` |
| `<file/function>` | `const PropertyEnumValue` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:176` | `const PropertyEnumValue* value =` |
| `<file/function>` | `const Color` | local/signature/use | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:207` | `const Color* value = std::get_if<Color>(&request.value);` |
| `<file/function>` | `const Color` | local/signature/use | `src/controls/inspection/property_value_converter_registry/property_value_converter_registry.cpp:65` | `const Color* color = std::get_if<Color>(&value);` |
| `<file/function>` | `const PropertyValueConverter` | local/signature/use | `src/controls/inspection/property_value_converter_registry/property_value_converter_registry.cpp:128` | `const PropertyValueConverter* PropertyValueConverterRegistry::find(` |
| `<file/function>` | `const PropertyValueConverter` | local/signature/use | `src/controls/inspection/property_value_converter_registry/property_value_converter_registry.cpp:144` | `const PropertyValueConverter* converter = find(service);` |
| `<file/function>` | `const PropertyValueConverter` | local/signature/use | `src/controls/inspection/property_value_converter_registry/property_value_converter_registry.cpp:174` | `const PropertyValueConverter* converter = find(service);` |
| `ContextMenu` | `Window` | class declaration | `src/controls/menu/context_menu/context_menu.cpp:651` | `Window* window{};` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/panel/anchored_popup_layer/anchored_popup_layer.cpp:89` | `if (Window* owner = window()) {` |
| `<file/function>` | `const PropertyCollectionValue` | local/signature/use | `src/controls/panel/combo_box/combo_box.cpp:100` | `const PropertyCollectionValue* collection =` |
| `<file/function>` | `const char` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:25` | `void require_instrument_text(std::string_view value, const char* field) {` |
| `<file/function>` | `const char` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:32` | `void require_finite_positive(double value, const char* field) {` |
| `EnsureModuleVisible` | `Impl` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:127` | `Impl* implementation{};` |
| `ChoiceFieldChanged` | `Impl` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:135` | `Impl* implementation{};` |
| `TextFieldChanged` | `Impl` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:146` | `Impl* implementation{};` |
| `TextFieldCommitted` | `Impl` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:155` | `Impl* implementation{};` |
| `TextFieldCancelled` | `Impl` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:164` | `Impl* implementation{};` |
| `MoveRequest` | `Impl` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:175` | `Impl* implementation{};` |
| `ModuleToggled` | `Impl` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:183` | `Impl* implementation{};` |
| `RemoveClicked` | `Impl` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:191` | `Impl* implementation{};` |
| `InstrumentRack` | `ModuleState` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:200` | `ModuleState* find_module(std::string_view id) noexcept {` |
| `InstrumentRack` | `const ModuleState` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:206` | `const ModuleState* find_module(std::string_view id) const noexcept {` |
| `InstrumentRack` | `InstrumentFieldSpec` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:219` | `InstrumentFieldSpec* find_field(ModuleState& module,` |
| `InstrumentRack` | `ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:239` | `ModuleState* module = find_module(module_id);` |
| `InstrumentRack` | `ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:264` | `ModuleState* module = find_module(module_id);` |
| `InstrumentRack` | `InstrumentFieldSpec` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:267` | `InstrumentFieldSpec* field = find_field(*module, field_id);` |
| `InstrumentRack` | `ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:293` | `ModuleState* module = find_module(module_id);` |
| `InstrumentRack` | `InstrumentFieldSpec` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:295` | `InstrumentFieldSpec* field = find_field(*module, field_id);` |
| `InstrumentRack` | `ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:315` | `ModuleState* module = find_module(module_id);` |
| `InstrumentRack` | `InstrumentFieldSpec` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:317` | `InstrumentFieldSpec* field = find_field(*module, field_id);` |
| `InstrumentRack` | `ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:348` | `ModuleState* module = find_module(module_id);` |
| `InstrumentRack` | `InstrumentFieldSpec` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:351` | `InstrumentFieldSpec* field = find_field(*module, field_id);` |
| `InstrumentRack` | `ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:371` | `ModuleState* module = find_module(module_id);` |
| `Slot` | `ModuleState` | class declaration | `src/controls/panel/instrument_rack/instrument_rack.cpp:586` | `ModuleState* module{};` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:747` | `if (Window* window = attached_window()) focused = (*window).focused_control();` |
| `<file/function>` | `const Impl::ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:813` | `const Impl::ModuleState* module = (*impl_).find_module(module_id);` |
| `<file/function>` | `const Impl::ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:825` | `const Impl::ModuleState* module = (*impl_).find_module(module_id);` |
| `<file/function>` | `Impl::ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:832` | `Impl::ModuleState* module = (*impl_).find_module(module_id);` |
| `<file/function>` | `Impl::ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:847` | `Impl::ModuleState* module = (*impl_).find_module(module_id);` |
| `<file/function>` | `Impl::ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:866` | `Impl::ModuleState* module = (*impl_).find_module(module_id);` |
| `<file/function>` | `InstrumentFieldSpec` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:868` | `InstrumentFieldSpec* field = (*impl_).find_field(*module, field_id);` |
| `<file/function>` | `Impl::ModuleState` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:903` | `Impl::ModuleState* module = (*impl_).find_module(module_id);` |
| `<file/function>` | `InstrumentFieldSpec` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:905` | `InstrumentFieldSpec* field = (*impl_).find_field(*module, field_id);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/panel/instrument_rack/instrument_rack.cpp:1040` | `Window* host = attached_window();` |
| `PropertyRefresh` | `Impl` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:83` | `Impl* implementation{};` |
| `CustomEditorCommit` | `Impl` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:93` | `Impl* implementation{};` |
| `CustomEditorFailure` | `Impl` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:105` | `Impl* implementation{};` |
| `ListCommit` | `Impl` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:117` | `Impl* implementation{};` |
| `ListExpansion` | `Impl` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:124` | `Impl* implementation{};` |
| `ListReset` | `Impl` | class declaration | `src/controls/panel/property_grid/property_grid.cpp:140` | `Impl* implementation{};` |
| `PropertyGrid` | `PropertyObjectValue` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:152` | `if (const gui_forms::PropertyObjectValue* object = std::get_if<PropertyObjectValue>(&value)) {` |
| `PropertyGrid` | `PropertyCollectionValue` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:155` | `if (const gui_forms::PropertyCollectionValue* collection =` |
| `PropertyGrid` | `PropertyObjectValue` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:182` | `const gui_forms::PropertyObjectValue* object = std::get_if<PropertyObjectValue>(&value);` |
| `PropertyGrid` | `const PropertyObjectMember` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:185` | `const PropertyObjectMember* member = nullptr;` |
| `PropertyGrid` | `PropertyCollectionValue` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:197` | `const gui_forms::PropertyCollectionValue* collection = std::get_if<PropertyCollectionValue>(&value);` |
| `PropertyGrid` | `PropertyObjectValue` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:221` | `const gui_forms::PropertyObjectValue* object = std::get_if<PropertyObjectValue>(&value);` |
| `PropertyGrid` | `PropertyCollectionValue` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:238` | `const gui_forms::PropertyCollectionValue* collection = std::get_if<PropertyCollectionValue>(&value);` |
| `PropertyGrid` | `PropertyObjectValue` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:254` | `const gui_forms::PropertyObjectValue* left_object = std::get_if<PropertyObjectValue>(&left);` |
| `PropertyGrid` | `PropertyObjectValue` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:255` | `const gui_forms::PropertyObjectValue* right_object = std::get_if<PropertyObjectValue>(&right);` |
| `PropertyGrid` | `PropertyCollectionValue` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:286` | `const gui_forms::PropertyCollectionValue* left_collection =` |
| `PropertyGrid` | `PropertyCollectionValue` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:288` | `const gui_forms::PropertyCollectionValue* right_collection =` |
| `PropertyGrid` | `ImageId` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:340` | `if (const gui_forms::ImageId* image = std::get_if<ImageId>(&value)) {` |
| `PropertyGrid` | `PropertyObjectValue` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:523` | `if (const gui_forms::PropertyObjectValue* nested_object =` |
| `PropertyGrid` | `PropertyCollectionValue` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:556` | `if (const gui_forms::PropertyCollectionValue* collection =` |
| `PropertyGrid` | `PropertyCollectionValue` | local/signature/use | `src/controls/panel/property_grid/property_grid.cpp:1027` | `const gui_forms::PropertyCollectionValue* collection = std::get_if<PropertyCollectionValue>(&*current);` |
| `<file/function>` | `Point` | local/signature/use | `src/controls/panel/property_grid/property_grid_utilities.hpp:67` | `if (const gui_forms::Point* item = std::get_if<Point>(&value)) {` |
| `<file/function>` | `Size` | local/signature/use | `src/controls/panel/property_grid/property_grid_utilities.hpp:70` | `} else if (const gui_forms::Size* item = std::get_if<Size>(&value)) {` |
| `<file/function>` | `Rect` | local/signature/use | `src/controls/panel/property_grid/property_grid_utilities.hpp:73` | `} else if (const gui_forms::Rect* item = std::get_if<Rect>(&value)) {` |
| `<file/function>` | `Insets` | local/signature/use | `src/controls/panel/property_grid/property_grid_utilities.hpp:78` | `} else if (const gui_forms::Insets* item = std::get_if<Insets>(&value)) {` |
| `<file/function>` | `Color` | local/signature/use | `src/controls/panel/property_grid/property_grid_utilities.hpp:83` | `} else if (const gui_forms::Color* item = std::get_if<Color>(&value)) {` |
| `<file/function>` | `FontSpec` | local/signature/use | `src/controls/panel/property_grid/property_grid_utilities.hpp:88` | `} else if (const gui_forms::FontSpec* item = std::get_if<FontSpec>(&value)) {` |
| `<file/function>` | `Point` | local/signature/use | `src/controls/panel/property_grid/property_grid_utilities.hpp:110` | `if (gui_forms::Point* item = std::get_if<Point>(&value)) {` |
| `<file/function>` | `Size` | local/signature/use | `src/controls/panel/property_grid/property_grid_utilities.hpp:117` | `} else if (gui_forms::Size* item = std::get_if<Size>(&value)) {` |
| `<file/function>` | `Rect` | local/signature/use | `src/controls/panel/property_grid/property_grid_utilities.hpp:124` | `} else if (gui_forms::Rect* item = std::get_if<Rect>(&value)) {` |
| `<file/function>` | `Insets` | local/signature/use | `src/controls/panel/property_grid/property_grid_utilities.hpp:133` | `} else if (gui_forms::Insets* item = std::get_if<Insets>(&value)) {` |
| `<file/function>` | `Color` | local/signature/use | `src/controls/panel/property_grid/property_grid_utilities.hpp:142` | `} else if (gui_forms::Color* item = std::get_if<Color>(&value)) {` |
| `<file/function>` | `FontSpec` | local/signature/use | `src/controls/panel/property_grid/property_grid_utilities.hpp:153` | `} else if (gui_forms::FontSpec* item = std::get_if<FontSpec>(&value)) {` |
| `TextValueChanged` | `Impl` | class declaration | `src/controls/panel/property_list/property_list.cpp:48` | `Impl* implementation{};` |
| `TextValueChanged` | `RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:52` | `RowState* row = (*implementation).find_row(row_id);` |
| `TextCommitted` | `Impl` | class declaration | `src/controls/panel/property_list/property_list.cpp:64` | `Impl* implementation{};` |
| `TextCommitted` | `RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:67` | `RowState* row = (*implementation).find_row(row_id);` |
| `TextCancelled` | `Impl` | class declaration | `src/controls/panel/property_list/property_list.cpp:86` | `Impl* implementation{};` |
| `TextCancelled` | `RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:89` | `RowState* row = (*implementation).find_row(row_id);` |
| `ChoiceChanged` | `Impl` | class declaration | `src/controls/panel/property_list/property_list.cpp:103` | `Impl* implementation{};` |
| `ChoiceChanged` | `RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:108` | `RowState* row = (*implementation).find_row(row_id);` |
| `CheckChanged` | `Impl` | class declaration | `src/controls/panel/property_list/property_list.cpp:124` | `Impl* implementation{};` |
| `CheckChanged` | `RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:129` | `RowState* row = (*implementation).find_row(row_id);` |
| `EnsureVisible` | `Impl` | class declaration | `src/controls/panel/property_list/property_list.cpp:146` | `Impl* implementation{};` |
| `ResetClicked` | `Impl` | class declaration | `src/controls/panel/property_list/property_list.cpp:154` | `Impl* implementation{};` |
| `PropertyList` | `RowState` | class declaration | `src/controls/panel/property_list/property_list.cpp:174` | `RowState* find_row(std::string_view id) noexcept {` |
| `PropertyList` | `const RowState` | class declaration | `src/controls/panel/property_list/property_list.cpp:180` | `const RowState* find_row(std::string_view id) const noexcept {` |
| `PropertyList` | `const PropertyRowSpec` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:188` | `const PropertyRowSpec* current = &spec(state);` |
| `PropertyList` | `const RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:191` | `const RowState* parent = find_row((*current).parent_id);` |
| `PropertyList` | `RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:421` | `RowState* row = find_row(row_id);` |
| `PropertyList` | `RowState` | class declaration | `src/controls/panel/property_list/property_list.cpp:455` | `RowState* disclosure_at(Point absolute) noexcept {` |
| `<file/function>` | `Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:579` | `Impl::RowState* row = (*impl_).find_row(row_id);` |
| `<file/function>` | `Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:622` | `Impl::RowState* row = (*impl_).find_row(row_id);` |
| `<file/function>` | `Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:642` | `Impl::RowState* row = (*impl_).find_row(row_id);` |
| `<file/function>` | `Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:664` | `Impl::RowState* row = (*impl_).find_row(row_id);` |
| `<file/function>` | `Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:694` | `Impl::RowState* row = (*impl_).find_row(id);` |
| `<file/function>` | `const Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:709` | `const Impl::RowState* row = (*impl_).find_row(id);` |
| `<file/function>` | `const Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:716` | `const Impl::RowState* row = (*impl_).find_row(id);` |
| `<file/function>` | `const Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:722` | `const Impl::RowState* row = (*impl_).find_row(id);` |
| `<file/function>` | `Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:728` | `Impl::RowState* row = (*impl_).find_row(id);` |
| `<file/function>` | `const Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:768` | `const Impl::RowState* row = (*impl_).find_row(id);` |
| `<file/function>` | `Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:963` | `if (Impl::RowState* row = (*impl_).disclosure_at(event.position)) {` |
| `<file/function>` | `const Impl::RowState` | local/signature/use | `src/controls/panel/property_list/property_list.cpp:1080` | `const Impl::RowState* row = (*impl_).find_row(id);` |
| `<file/function>` | `const char` | local/signature/use | `src/controls/panel/property_list/property_list_utilities.hpp:29` | `inline void require_property_text(std::string_view value, const char* field) {` |
| `<file/function>` | `const char` | local/signature/use | `src/controls/range_control/range_control_rendering.cpp:12` | `void require_finite(double value, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `src/controls/range_control/range_control_rendering.hpp:13` | `void require_finite(double value, const char* message);` |
| `<file/function>` | `const Window` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:32` | `const Window* owner = window();` |
| `<file/function>` | `const Control` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:57` | `for (const Control* current = this; current != nullptr;) {` |
| `<file/function>` | `const ContainerControl` | local/signature/use | `src/controls/scrollable_control/container_control/container_control.cpp:58` | `if (const ContainerControl* container = dynamic_cast<const ContainerControl*>(current);` |
| `<file/function>` | `const char` | local/signature/use | `src/controls/scrollable_control/container_control/split_container/split_container.cpp:18` | `void require_finite_nonnegative(double value, const char* message) {` |
| `<file/function>` | `const TableLayoutPanel::CellMetadata` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:266` | `const TableLayoutPanel::CellMetadata* TableLayoutPanel::metadata_for(` |
| `<file/function>` | `const CellMetadata` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:302` | `const CellMetadata* metadata = metadata_for(child);` |
| `<file/function>` | `const CellMetadata` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:318` | `const CellMetadata* metadata = metadata_for(child);` |
| `<file/function>` | `const CellMetadata` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:334` | `const CellMetadata* metadata = metadata_for(child);` |
| `<file/function>` | `const CellMetadata` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:387` | `const CellMetadata* metadata = std::as_const(*this).metadata_for(*child);` |
| `<file/function>` | `const CellMetadata` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:412` | `const CellMetadata* metadata = std::as_const(*this).metadata_for(*child);` |
| `<file/function>` | `const CellMetadata` | local/signature/use | `src/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.cpp:600` | `const CellMetadata* metadata = metadata_for(*child);` |
| `<file/function>` | `const char` | local/signature/use | `src/controls/scrollable_control/scrollable_control.cpp:22` | `void ScrollableControl::validate_size(Size size, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `src/controls/scrollable_control/scrollable_control.cpp:30` | `const char* message) {` |
| `SoundCueClick` | `HostServices` | local/signature/use | `src/controls/showcase_controls.cpp:313` | `HostServices* services = source.attached_window() == nullptr` |
| `PropertySpecimenClick` | `Window` | local/signature/use | `src/controls/showcase_controls.cpp:1898` | `if (Window* window = grid.attached_window()) {` |
| `PropertySpecimenClick` | `Window` | local/signature/use | `src/controls/showcase_controls.cpp:1925` | `if (Window* window = grid.attached_window()) {` |
| `DialogInvoker` | `HostServices` | local/signature/use | `src/controls/showcase_controls.cpp:2977` | `HostServices* services = source.attached_window() == nullptr` |
| `HostServiceClick` | `HostServices` | local/signature/use | `src/controls/showcase_controls.cpp:3072` | `HostServices* services = source.attached_window() == nullptr` |
| `ToolTip` | `ToolTip` | class declaration | `src/controls/tool_tip/tool_tip.cpp:33` | `ToolTip* tool_tip{};` |
| `ToolTip` | `ToolTip` | class declaration | `src/controls/tool_tip/tool_tip.cpp:43` | `ToolTip* tool_tip{};` |
| `ToolTip` | `ToolTip` | class declaration | `src/controls/tool_tip/tool_tip.cpp:53` | `ToolTip* tool_tip{};` |
| `ToolTip` | `ToolTip` | class declaration | `src/controls/tool_tip/tool_tip.cpp:63` | `ToolTip* tool_tip{};` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:101` | `Window* ToolTip::bound_window() const noexcept {` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:110` | `Window* owner = bound_window();` |
| `<file/function>` | `ToolTip::Entry` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:117` | `ToolTip::Entry* ToolTip::find_entry(const Control& target) {` |
| `<file/function>` | `const ToolTip::Entry` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:122` | `const ToolTip::Entry* ToolTip::find_entry(const Control& target) const {` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:131` | `Window* owner = bound_window();` |
| `<file/function>` | `Entry` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:144` | `if (Entry* existing = find_entry(*target)) {` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:166` | `const Entry* entry = find_entry(target);` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:265` | `const Entry* entry = find_entry(*target);` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:321` | `Window* owner = bound_window();` |
| `<file/function>` | `Entry` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:322` | `Entry* entry = target ? find_entry(*target) : nullptr;` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:362` | `Window* owner = bound_window();` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:404` | `if (const Entry* entry = find_entry(*target)) text = (*entry).text;` |
| `<file/function>` | `const Entry` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:430` | `if (const Entry* entry = find_entry(*target)) text = (*entry).text;` |
| `<file/function>` | `Window` | local/signature/use | `src/controls/tool_tip/tool_tip.cpp:446` | `if (Window* owner = bound_window()) (*owner).verify_access("ToolTip disposal");` |
| `<file/function>` | `const BindableProperty` | local/signature/use | `src/core/binding/binding/binding.cpp:88` | `const BindableProperty* property = (*target_).find_bindable_property(property_name_);` |
| `<file/function>` | `const BindableProperty` | local/signature/use | `src/core/binding/binding/binding.cpp:158` | `const BindableProperty* property =` |
| `<file/function>` | `const BindableProperty` | local/signature/use | `src/core/binding/binding/binding.cpp:227` | `const BindableProperty* property =` |
| `<file/function>` | `Window` | local/signature/use | `src/core/binding/binding/binding.cpp:304` | `if (Window* owner = (*source).bound_window()) {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:42` | `Window* BindingContext::bound_window() const noexcept {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:49` | `Window* owner = bound_window();` |
| `<file/function>` | `BindingSource` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:67` | `BindingSource* key = source.get();` |
| `<file/function>` | `Window` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:86` | `if (Window* owner = bound_window()) (*owner).verify_access("BindingContext removal");` |
| `<file/function>` | `BindingSource` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:90` | `bool BindingContext::remove_entry(BindingSource* source, bool publish) {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:104` | `if (Window* owner = bound_window()) (*owner).verify_access("BindingContext clear");` |
| `<file/function>` | `BindingSource` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:107` | `for (const std::pair<BindingSource* const, SourceEntry>& source_entry :` |
| `<file/function>` | `BindingSource` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:109` | `BindingSource* const source = source_entry.first;` |
| `<file/function>` | `BindingSource` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:114` | `for (BindingSource* source : removed) {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/binding/binding_context/binding_context.cpp:121` | `if (Window* owner = bound_window()) (*owner).verify_access("BindingContext disposal");` |
| `<file/function>` | `Window` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:36` | `Window* BindingSource::bound_window() const noexcept {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:45` | `Window* owner = bound_window();` |
| `<file/function>` | `const BindingRecord` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:104` | `if (const BindingRecord* before = current()) previous_id = (*before).stable_id;` |
| `<file/function>` | `const BindingRecord` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:124` | `const BindingRecord* after = current();` |
| `<file/function>` | `const BindingRecord` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:134` | `const BindingRecord* BindingSource::current() const noexcept {` |
| `<file/function>` | `const BindingRecord` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:141` | `const BindingRecord* record = current();` |
| `<file/function>` | `const BindingRecord` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:150` | `const BindingRecord* record = current();` |
| `<file/function>` | `const BindingRecord` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:208` | `const BindingRecord* record = current();` |
| `<file/function>` | `BindingRecord` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:218` | `BindingRecord* record = position_ < 0 ? nullptr` |
| `<file/function>` | `const BindingRecord` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:442` | `if (const BindingRecord* record = current()) {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:478` | `if (Window* owner = bound_window()) (*owner).verify_access("BindingSource disposal");` |
| `<file/function>` | `const Binding` | local/signature/use | `src/core/binding/binding_source/binding_source.cpp:511` | `void BindingSource::unregister_binding(const Binding* binding) noexcept {` |
| `<file/function>` | `const BindingRecord` | local/signature/use | `src/core/binding/currency_manager/currency_manager.cpp:25` | `const BindingRecord* CurrencyManager::current() const noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/binding/value/binding_value.cpp:78` | `const char* begin = text.data();` |
| `<file/function>` | `const char` | local/signature/use | `src/core/binding/value/binding_value.cpp:79` | `const char* end = begin + text.size();` |
| `<file/function>` | `const PropertyEnumChoice` | local/signature/use | `src/core/binding/value/binding_value.cpp:181` | `const PropertyEnumChoice* enum_choice_by_name(` |
| `<file/function>` | `const PropertyEnumChoice` | local/signature/use | `src/core/binding/value/binding_value.cpp:189` | `const PropertyEnumChoice* enum_choice_by_value(` |
| `<file/function>` | `PropertyEnumValue` | local/signature/use | `src/core/binding/value/binding_value.cpp:200` | `if (const gui_forms::PropertyEnumValue* item = std::get_if<PropertyEnumValue>(&value)) {` |
| `<file/function>` | `const PropertyEnumChoice` | local/signature/use | `src/core/binding/value/binding_value.cpp:221` | `const PropertyEnumChoice* choice = enum_choice_by_name(` |
| `<file/function>` | `const PropertyEnumChoice` | local/signature/use | `src/core/binding/value/binding_value.cpp:235` | `const PropertyEnumChoice* choice = enum_choice_by_name(` |
| `<file/function>` | `const PropertyEnumChoice` | local/signature/use | `src/core/binding/value/binding_value.cpp:256` | `if (const PropertyEnumChoice* exact = enum_choice_by_value(` |
| `<file/function>` | `PropertyObjectValue` | local/signature/use | `src/core/binding/value/binding_value.cpp:307` | `if (const gui_forms::PropertyObjectValue* object = std::get_if<PropertyObjectValue>(&value)) {` |
| `<file/function>` | `PropertyCollectionValue` | local/signature/use | `src/core/binding/value/binding_value.cpp:366` | `} else if (const gui_forms::PropertyCollectionValue* collection =` |
| `PropertyValueNormalizer` | `const PropertyDescriptor` | class declaration | `src/core/binding/value/binding_value.cpp:613` | `const PropertyDescriptor* descriptor{};` |
| `<file/function>` | `const BindableProperty` | local/signature/use | `src/core/control/control/control.cpp:667` | `const BindableProperty* Control::find_bindable_property(` |
| `<file/function>` | `const char` | local/signature/use | `src/core/control/control/control.cpp:894` | `void validate_insets(Insets value, const char* message) {` |
| `<file/function>` | `Control` | local/signature/use | `src/core/control/control/control.cpp:1578` | `bool Control::perform_validation(Control* destination, bool bulk) {` |
| `<file/function>` | `const Control` | local/signature/use | `src/core/control/control/control.cpp:1652` | `const Control* current = this;` |
| `<file/function>` | `Control` | local/signature/use | `src/core/control/control/control.cpp:1749` | `for (Control* current = this; current != nullptr;) {` |
| `<file/function>` | `Control` | local/signature/use | `src/core/control/control/control.cpp:1780` | `for (Control* current = this; current != nullptr;) {` |
| `<file/function>` | `const void` | local/signature/use | `src/core/control/control/control.cpp:1860` | `void Control::publish_change(const void* event_key,` |
| `<file/function>` | `const Control` | local/signature/use | `src/core/control/control/control.cpp:2276` | `const Control* current = this;` |
| `<file/function>` | `const std::byte` | local/signature/use | `src/core/drawing/bitmap/bitmap.cpp:306` | `const std::byte* current = (*storage_).bytes.data() + storage_row;` |
| `<file/function>` | `const std::byte` | local/signature/use | `src/core/drawing/bitmap/bitmap.cpp:307` | `const std::byte* original = active_edit_backup_.data() +` |
| `<file/function>` | `const Entry` | local/signature/use | `src/core/drawing/color/color.cpp:24` | `const Entry* found = std::begin(entries);` |
| `<file/function>` | `const char` | local/signature/use | `src/core/drawing/support/drawing_support.hpp:225` | `[[nodiscard]] const char* command_name(CommandKind kind) noexcept {` |
| `<file/function>` | `HostServices` | local/signature/use | `src/core/feedback/semantic_feedback/semantic_feedback.cpp:60` | `if (HostServices* services = (*window_).host_services()) {` |
| `<file/function>` | `char` | local/signature/use | `src/core/host/services/host_services.cpp:29` | `const unsigned char* bytes = reinterpret_cast<const unsigned char*>(text.data());` |
| `DialogResultValidator` | `const HostDialogResult` | class declaration | `src/core/host/services/host_services.cpp:250` | `const HostDialogResult* result{};` |
| `DialogResultValidator` | `const HostMessageDialogResult` | local/signature/use | `src/core/host/services/host_services.cpp:257` | `const HostMessageDialogResult* value =` |
| `DialogResultValidator` | `const HostPathDialogResult` | local/signature/use | `src/core/host/services/host_services.cpp:268` | `const HostPathDialogResult* value =` |
| `<file/function>` | `char` | local/signature/use | `src/core/host/session/host_session.cpp:29` | `const unsigned char* bytes = reinterpret_cast<const unsigned char*>(text.data());` |
| `<file/function>` | `const DragTextData` | local/signature/use | `src/core/host/session/host_session.cpp:118` | `if (const DragTextData* data = std::get_if<DragTextData>(&item)) {` |
| `<file/function>` | `const DragFileListData` | local/signature/use | `src/core/host/session/host_session.cpp:124` | `} else if (const DragFileListData* data =` |
| `<file/function>` | `const DragBinaryData` | local/signature/use | `src/core/host/session/host_session.cpp:137` | `} else if (const DragBinaryData* data =` |
| `<file/function>` | `HostServices` | local/signature/use | `src/core/host/session/host_session.cpp:155` | `HostServices* services)` |
| `<file/function>` | `const HostActivationEvent` | local/signature/use | `src/core/host/session/host_session.cpp:301` | `if (const HostActivationEvent* activation =` |
| `<file/function>` | `const HostOcclusionEvent` | local/signature/use | `src/core/host/session/host_session.cpp:304` | `} else if (const HostOcclusionEvent* occlusion =` |
| `<file/function>` | `const HostAttachEvent` | local/signature/use | `src/core/host/session/host_session.cpp:335` | `if (const HostAttachEvent* attach =` |
| `<file/function>` | `const HostResizeEvent` | local/signature/use | `src/core/host/session/host_session.cpp:339` | `} else if (const HostResizeEvent* resize =` |
| `<file/function>` | `const HostScaleEvent` | local/signature/use | `src/core/host/session/host_session.cpp:342` | `} else if (const HostScaleEvent* scale =` |
| `<file/function>` | `const HostDisplayEvent` | local/signature/use | `src/core/host/session/host_session.cpp:345` | `} else if (const HostDisplayEvent* display =` |
| `<file/function>` | `DragEvent` | local/signature/use | `src/core/host/session/host_session.cpp:355` | `if (const gui_forms::DragEvent* drag = std::get_if<DragEvent>(&event.payload);` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_capabilities/host_capabilities.cpp:57` | `const char* const name = named_capability.second;` |
| `HostEventNameVisitor` | `const char` | class declaration | `src/core/host/types/host_types.cpp:15` | `const char* operator()(const HostAttachEvent&) const noexcept { return "attach"; }` |
| `HostEventNameVisitor` | `const char` | class declaration | `src/core/host/types/host_types.cpp:16` | `const char* operator()(const HostResizeEvent&) const noexcept { return "resize"; }` |
| `HostEventNameVisitor` | `const char` | class declaration | `src/core/host/types/host_types.cpp:17` | `const char* operator()(const HostScaleEvent&) const noexcept { return "scale"; }` |
| `HostEventNameVisitor` | `const char` | class declaration | `src/core/host/types/host_types.cpp:18` | `const char* operator()(const HostActivationEvent&) const noexcept { return "activation"; }` |
| `HostEventNameVisitor` | `const char` | class declaration | `src/core/host/types/host_types.cpp:19` | `const char* operator()(const HostOcclusionEvent&) const noexcept { return "occlusion"; }` |
| `HostEventNameVisitor` | `const char` | class declaration | `src/core/host/types/host_types.cpp:20` | `const char* operator()(const HostDisplayEvent&) const noexcept { return "display"; }` |
| `HostEventNameVisitor` | `const char` | class declaration | `src/core/host/types/host_types.cpp:21` | `const char* operator()(const DragEvent&) const noexcept { return "drag"; }` |
| `HostEventNameVisitor` | `const char` | class declaration | `src/core/host/types/host_types.cpp:22` | `const char* operator()(const PointerEvent&) const noexcept { return "pointer"; }` |
| `HostEventNameVisitor` | `const char` | class declaration | `src/core/host/types/host_types.cpp:23` | `const char* operator()(const KeyEvent&) const noexcept { return "key"; }` |
| `HostEventNameVisitor` | `const char` | class declaration | `src/core/host/types/host_types.cpp:24` | `const char* operator()(const TextInputEvent&) const noexcept { return "text"; }` |
| `HostEventNameVisitor` | `const char` | class declaration | `src/core/host/types/host_types.cpp:25` | `const char* operator()(const HostCloseRequest&) const noexcept { return "close_request"; }` |
| `HostEventNameVisitor` | `const char` | class declaration | `src/core/host/types/host_types.cpp:26` | `const char* operator()(const HostClosedEvent&) const noexcept { return "closed"; }` |
| `HostEventNameVisitor` | `const char` | class declaration | `src/core/host/types/host_types.cpp:27` | `const char* operator()(const HostShutdownEvent&) const noexcept { return "shutdown"; }` |
| `HostDialogKindVisitor` | `const char` | class declaration | `src/core/host/types/host_types.cpp:31` | `const char* operator()(const HostMessageDialogRequest&) const noexcept {` |
| `HostDialogKindVisitor` | `const char` | class declaration | `src/core/host/types/host_types.cpp:34` | `const char* operator()(const HostOpenFileDialogRequest&) const noexcept {` |
| `HostDialogKindVisitor` | `const char` | class declaration | `src/core/host/types/host_types.cpp:37` | `const char* operator()(const HostSaveFileDialogRequest&) const noexcept {` |
| `HostDialogKindVisitor` | `const char` | class declaration | `src/core/host/types/host_types.cpp:40` | `const char* operator()(const HostFolderDialogRequest&) const noexcept {` |
| `HostDialogKindVisitor` | `const char` | class declaration | `src/core/host/types/host_types.cpp:43` | `const char* operator()(const HostColorDialogRequest&) const noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_types.cpp:50` | `const char* host_dispatch_error_name(HostDispatchError error) noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_types.cpp:65` | `const char* host_lifecycle_phase_name(HostLifecyclePhase phase) noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_types.cpp:76` | `const char* host_event_name(const HostEventPayload& payload) noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_types.cpp:80` | `const char* host_service_error_name(HostServiceError error) noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_types.cpp:95` | `const char* host_sound_cue_name(HostSoundCue cue) noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_types.cpp:106` | `const char* cursor_kind_name(CursorKind cursor) noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_types.cpp:120` | `const char* drag_effect_name(DragEffect effect) noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_types.cpp:130` | `const char* host_dialog_kind_name(const HostDialogRequestPayload& payload) noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_types.cpp:134` | `const char* host_dialog_outcome_name(HostDialogOutcome outcome) noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/host/types/host_types.cpp:138` | `const char* host_dialog_choice_name(HostDialogChoice choice) noexcept {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:43` | `if (Window* owner = bound_window(); owner && (*owner).check_access()) {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:49` | `Window* ImageList::bound_window() const noexcept {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:62` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:142` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:164` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:203` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:285` | `if (Window* owner = bound_window()) {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:319` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:380` | `Window* owner = bound_window();` |
| `<file/function>` | `const Variant` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:402` | `const Variant* exact{};` |
| `<file/function>` | `const Variant` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:403` | `const Variant* larger{};` |
| `<file/function>` | `const Variant` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:404` | `const Variant* smaller{};` |
| `<file/function>` | `const Variant` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:417` | `const Variant* chosen = exact ? exact : (larger ? larger : smaller);` |
| `<file/function>` | `Window` | local/signature/use | `src/core/resources/image_list/image_list/image_list.cpp:445` | `if (Window* owner = bound_window()) (*owner).verify_access("ImageList disposal");` |
| `<file/function>` | `const std::byte` | local/signature/use | `src/core/resources/image_registry/image_registry.cpp:34` | `std::uint32_t read_u32(const std::byte* bytes) noexcept {` |
| `<file/function>` | `const std::byte` | local/signature/use | `src/core/resources/image_registry/image_registry.cpp:193` | `const std::byte* const type_bytes = encoded.data() + offset + 4U;` |
| `<file/function>` | `const char` | local/signature/use | `src/core/semantics/snapshot/semantic_snapshot.cpp:63` | `const char* semantic_role_name(SemanticRole role) noexcept {` |
| `<file/function>` | `const char` | local/signature/use | `src/core/semantics/snapshot/semantic_snapshot.cpp:101` | `const char* semantic_action_name(SemanticAction action) noexcept {` |
| `<file/function>` | `Timer` | local/signature/use | `src/core/timer/timer/timer.cpp:27` | `Timer* timer = callback_state ? (*callback_state).owner : nullptr;` |
| `<file/function>` | `Window` | local/signature/use | `src/core/timer/timer/timer.cpp:32` | `Window* Timer::bound_window() const noexcept {` |
| `<file/function>` | `Window` | local/signature/use | `src/core/timer/timer/timer.cpp:41` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/core/timer/timer/timer.cpp:78` | `Window* owner = bound_window();` |
| `<file/function>` | `Window` | local/signature/use | `src/core/timer/timer/timer.cpp:95` | `if (Window* owner = bound_window()) {` |
| `AcceleratorAttachment` | `Component` | class declaration | `src/core/window/accelerator/accelerator_attachment.hpp:19` | `[[nodiscard]] Component* owner() const noexcept { return owner_; }` |
| `AcceleratorAttachment` | `Window` | class declaration | `src/core/window/accelerator/accelerator_attachment.hpp:31` | `Window* window_{};` |
| `AcceleratorAttachment` | `Component` | class declaration | `src/core/window/accelerator/accelerator_attachment.hpp:32` | `Component* owner_{};` |
| `PopupAttachment` | `Window` | class declaration | `src/core/window/popup/popup_attachment.hpp:31` | `Window* window_{};` |
| `<file/function>` | `Window` | local/signature/use | `src/core/window/update_scope/update_scope.cpp:32` | `Window* closing = std::exchange(window_, nullptr);` |
| `<file/function>` | `const Control` | local/signature/use | `src/core/window/window.cpp:71` | `const Control::Ptr& control, const Control* expected_parent,` |
| `<file/function>` | `const Window` | local/signature/use | `src/core/window/window.cpp:72` | `const Window* owner) noexcept {` |
| `<file/function>` | `const Control` | local/signature/use | `src/core/window/window.cpp:79` | `const Control* expected_parent,` |
| `<file/function>` | `const Window` | local/signature/use | `src/core/window/window.cpp:80` | `const Window* owner,` |
| `<file/function>` | `Window` | local/signature/use | `src/core/window/window.cpp:142` | `Window* const owner = (*control).attached_window();` |
| `<file/function>` | `Component` | local/signature/use | `src/core/window/window.cpp:357` | `Component* owner = (*accelerator).owner();` |
| `<file/function>` | `Control` | local/signature/use | `src/core/window/window.cpp:779` | `Control* destination, bool bulk) {` |
| `<file/function>` | `PointerEvent` | local/signature/use | `src/core/window/window.cpp:1383` | `if (gui_forms::PointerEvent* pointer = std::get_if<PointerEvent>(&input);` |
| `<file/function>` | `PointerEvent` | local/signature/use | `src/core/window/window.cpp:1386` | `if (gui_forms::PointerEvent* previous =` |
| `<file/function>` | `DragEvent` | local/signature/use | `src/core/window/window.cpp:1396` | `if (gui_forms::DragEvent* drag = std::get_if<DragEvent>(&input);` |
| `<file/function>` | `DragEvent` | local/signature/use | `src/core/window/window.cpp:1399` | `if (gui_forms::DragEvent* previous =` |
| `<file/function>` | `Control` | local/signature/use | `src/core/window/window.cpp:2376` | `Control* const retained_parent = (*control).parent_.lock().get();` |
| `<file/function>` | `Control` | local/signature/use | `src/core/window/window.cpp:2542` | `for (Control* current = &control; current != nullptr;) {` |
| `<file/function>` | `Control` | local/signature/use | `src/core/window/window.cpp:2609` | `Control* const retained_parent = (*control).parent_.lock().get();` |
| `<file/function>` | `HostMessageDialogResult` | local/signature/use | `src/host/headless/services/headless_host_services.cpp:78` | `if (const gui_forms::HostMessageDialogResult* message =` |
| `<file/function>` | `HostPathDialogResult` | local/signature/use | `src/host/headless/services/headless_host_services.cpp:81` | `} else if (const gui_forms::HostPathDialogResult* paths =` |
| `HeadlessHost` | `Window` | class declaration | `src/host/headless/session/headless_host.hpp:46` | `Window* window_{};` |
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
| `<file/function>` | `const std::byte` | local/signature/use | `src/host/macos/application/macos_host.mm:353` | `const std::byte* bytes = static_cast<const std::byte*>(data.bytes);` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:359` | `NSString* resource_name,` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:360` | `NSString* extension,` |
| `<file/function>` | `NSURL` | local/signature/use | `src/host/macos/application/macos_host.mm:363` | `NSURL* url = [[NSBundle mainBundle] URLForResource:resource_name` |
| `<file/function>` | `NSData` | local/signature/use | `src/host/macos/application/macos_host.mm:367` | `NSData* data = [NSData dataWithContentsOfURL:url` |
| `<file/function>` | `const std::byte` | local/signature/use | `src/host/macos/application/macos_host.mm:371` | `const std::byte* bytes = static_cast<const std::byte*>(data.bytes);` |
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
| `DrainPostedWorkWake` | `GUIFormsView` | class declaration | `src/host/macos/application/macos_host.mm:738` | `explicit DrainPostedWorkWake(GUIFormsView* view) noexcept` |
| `DrainPostedWorkWake` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:743` | `__weak GUIFormsView* weakView = view_;` |
| `DrainPostedWorkWake` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:745` | `GUIFormsView* strongView = weakView;` |
| `DrainPostedWorkWake` | `GUIFormsView` | class declaration | `src/host/macos/application/macos_host.mm:751` | `__weak GUIFormsView* view_;` |
| `CollectDamageWake` | `GUIFormsView` | class declaration | `src/host/macos/application/macos_host.mm:756` | `explicit CollectDamageWake(GUIFormsView* view) noexcept` |
| `CollectDamageWake` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:761` | `__weak GUIFormsView* weakView = view_;` |
| `CollectDamageWake` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:763` | `GUIFormsView* strongView = weakView;` |
| `CollectDamageWake` | `GUIFormsView` | class declaration | `src/host/macos/application/macos_host.mm:769` | `__weak GUIFormsView* view_;` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:803` | `__weak GUIFormsView* weakSelf = self;` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:805` | `GUIFormsView* strongSelf = weakSelf;` |
| `<file/function>` | `NSMutableArray` | local/signature/use | `src/host/macos/application/macos_host.mm:983` | `NSMutableArray* result = [[NSMutableArray alloc]` |
| `<file/function>` | `NSMutableDictionary<NSString*, GUIFormsAccessibilityElement*>` | local/signature/use | `src/host/macos/application/macos_host.mm:985` | `NSMutableDictionary<NSString*, GUIFormsAccessibilityElement*>* next =` |
| `<file/function>` | `GUIFormsAccessibilityElement` | local/signature/use | `src/host/macos/application/macos_host.mm:988` | `GUIFormsAccessibilityElement* element = reconcile_accessibility_element(` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:1094` | `NSString* text = native_string(request.text);` |
| `<file/function>` | `NSFont` | local/signature/use | `src/host/macos/application/macos_host.mm:1096` | `NSFont* font = [NSFont fontWithName:@"Lucida Grande" size:12.0];` |
| `<file/function>` | `NSDictionary` | local/signature/use | `src/host/macos/application/macos_host.mm:1098` | `NSDictionary* attributes = @{NSFontAttributeName: font};` |
| `<file/function>` | `NSTextField` | local/signature/use | `src/host/macos/application/macos_host.mm:1106` | `NSTextField* label = [[NSTextField alloc]` |
| `<file/function>` | `NSPanel` | local/signature/use | `src/host/macos/application/macos_host.mm:1119` | `NSPanel* panel = [[NSPanel alloc]` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:1238` | `__weak GUIFormsView* weakSelf = self;` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:1240` | `GUIFormsView* strongSelf = weakSelf;` |
| `<file/function>` | `NSException` | local/signature/use | `src/host/macos/application/macos_host.mm:1313` | `} @catch (NSException* exception) {` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:1314` | `NSString* name = exception.name == nil ? @"NSException" : exception.name;` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:1315` | `NSString* reason = exception.reason == nil ? @"no reason" : exception.reason;` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:1316` | `NSString* diagnostic = [NSString stringWithFormat:@"%@: %@", name, reason];` |
| `<file/function>` | `const void` | local/signature/use | `src/host/macos/application/macos_host.mm:1447` | `const void* pixels = _raster.pixels();` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:1619` | `NSString* plain = [string isKindOfClass:[NSAttributedString class]]` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:1707` | `__weak GUIFormsView* _view;` |
| `<file/function>` | `NSEvent` | local/signature/use | `src/host/macos/application/macos_host.mm:1740` | `NSEvent* wake = [NSEvent otherEventWithType:NSEventTypeApplicationDefined` |
| `<file/function>` | `NSWindow` | local/signature/use | `src/host/macos/application/macos_host.mm:1758` | `NSWindow* window = notification.object;` |
| `<file/function>` | `NSMenu` | local/signature/use | `src/host/macos/application/macos_host.mm:1768` | `NSMenu* menuBar = [[NSMenu alloc] init];` |
| `<file/function>` | `NSMenuItem` | local/signature/use | `src/host/macos/application/macos_host.mm:1769` | `NSMenuItem* applicationItem = [[NSMenuItem alloc] init];` |
| `<file/function>` | `NSMenu` | local/signature/use | `src/host/macos/application/macos_host.mm:1771` | `NSMenu* applicationMenu = [[NSMenu alloc] init];` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/application/macos_host.mm:1772` | `NSString* quitTitle = @"Quit GUI.Forms Gallery";` |
| `<file/function>` | `NSMenuItem` | local/signature/use | `src/host/macos/application/macos_host.mm:1773` | `NSMenuItem* quit = [[NSMenuItem alloc] initWithTitle:quitTitle` |
| `DispatchAndCollectWake` | `GUIFormsView` | class declaration | `src/host/macos/application/macos_host.mm:1805` | `GUIFormsView* view,` |
| `DispatchAndCollectWake` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:1811` | `GUIFormsView* view = view_;` |
| `DispatchAndCollectWake` | `GUIFormsView` | class declaration | `src/host/macos/application/macos_host.mm:1820` | `GUIFormsView* view_;` |
| `RequestNativeClose` | `NSWindow` | class declaration | `src/host/macos/application/macos_host.mm:1826` | `explicit RequestNativeClose(NSWindow* window) noexcept : window_(window) {}` |
| `RequestNativeClose` | `NSWindow` | local/signature/use | `src/host/macos/application/macos_host.mm:1830` | `NSWindow* window = window_;` |
| `RequestNativeClose` | `NSWindow` | class declaration | `src/host/macos/application/macos_host.mm:1837` | `NSWindow* window_;` |
| `ShowNativeDialog` | `GUIFormsView` | class declaration | `src/host/macos/application/macos_host.mm:1842` | `explicit ShowNativeDialog(GUIFormsView* view) noexcept : view_(view) {}` |
| `ShowNativeDialog` | `GUIFormsView` | class declaration | `src/host/macos/application/macos_host.mm:1850` | `GUIFormsView* view_;` |
| `ShowNativeTooltip` | `GUIFormsView` | class declaration | `src/host/macos/application/macos_host.mm:1855` | `explicit ShowNativeTooltip(GUIFormsView* view) noexcept : view_(view) {}` |
| `ShowNativeTooltip` | `GUIFormsView` | class declaration | `src/host/macos/application/macos_host.mm:1863` | `GUIFormsView* view_;` |
| `HideNativeTooltip` | `GUIFormsView` | class declaration | `src/host/macos/application/macos_host.mm:1868` | `explicit HideNativeTooltip(GUIFormsView* view) noexcept : view_(view) {}` |
| `HideNativeTooltip` | `GUIFormsView` | class declaration | `src/host/macos/application/macos_host.mm:1873` | `GUIFormsView* view_;` |
| `ReadNativeClipboard` | `GUIFormsView` | class declaration | `src/host/macos/application/macos_host.mm:1878` | `explicit ReadNativeClipboard(GUIFormsView* view) noexcept : view_(view) {}` |
| `ReadNativeClipboard` | `GUIFormsView` | class declaration | `src/host/macos/application/macos_host.mm:1886` | `GUIFormsView* view_;` |
| `WriteNativeClipboard` | `GUIFormsView` | class declaration | `src/host/macos/application/macos_host.mm:1891` | `explicit WriteNativeClipboard(GUIFormsView* view) noexcept : view_(view) {}` |
| `WriteNativeClipboard` | `GUIFormsView` | class declaration | `src/host/macos/application/macos_host.mm:1899` | `GUIFormsView* view_;` |
| `<file/function>` | `NSApplication` | local/signature/use | `src/host/macos/application/macos_host.mm:1922` | `NSApplication* application = [NSApplication sharedApplication];` |
| `<file/function>` | `NSWindow` | local/signature/use | `src/host/macos/application/macos_host.mm:1932` | `NSWindow* nativeWindow = [[NSWindow alloc]` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:1941` | `GUIFormsView* view = [[GUIFormsView alloc] initWithModel:std::move(model)];` |
| `<file/function>` | `GUIFormsWindowDelegate` | local/signature/use | `src/host/macos/application/macos_host.mm:1943` | `GUIFormsWindowDelegate* delegate =` |
| `<file/function>` | `NSApplication` | local/signature/use | `src/host/macos/application/macos_host.mm:2032` | `NSApplication* application = [NSApplication sharedApplication];` |
| `<file/function>` | `NSMutableArray<NSWindow*>` | local/signature/use | `src/host/macos/application/macos_host.mm:2036` | `NSMutableArray<NSWindow*>* nativeWindows =` |
| `<file/function>` | `NSMutableArray<GUIFormsView*>` | local/signature/use | `src/host/macos/application/macos_host.mm:2038` | `NSMutableArray<GUIFormsView*>* views =` |
| `<file/function>` | `NSMutableArray<GUIFormsWindowDelegate*>` | local/signature/use | `src/host/macos/application/macos_host.mm:2040` | `NSMutableArray<GUIFormsWindowDelegate*>* delegates =` |
| `<file/function>` | `NSWindow` | local/signature/use | `src/host/macos/application/macos_host.mm:2051` | `NSWindow* nativeWindow = entry.tool_window` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:2059` | `GUIFormsView* view =` |
| `<file/function>` | `GUIFormsWindowDelegate` | local/signature/use | `src/host/macos/application/macos_host.mm:2063` | `GUIFormsWindowDelegate* delegate =` |
| `<file/function>` | `NSWindow` | local/signature/use | `src/host/macos/application/macos_host.mm:2079` | `NSWindow* nativeWindow = nativeWindows[index];` |
| `<file/function>` | `NSWindow` | local/signature/use | `src/host/macos/application/macos_host.mm:2090` | `NSWindow* ownerWindow = nativeWindows[ownerIndex];` |
| `<file/function>` | `NSScreen` | local/signature/use | `src/host/macos/application/macos_host.mm:2092` | `NSScreen* screen = ownerWindow.screen ?: NSScreen.mainScreen;` |
| `<file/function>` | `NSWindow` | local/signature/use | `src/host/macos/application/macos_host.mm:2105` | `NSWindow* nativeWindow = nativeWindows[index];` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:2106` | `GUIFormsView* view = views[index];` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:2108` | `for (GUIFormsView* initializedView in views) {` |
| `<file/function>` | `NSWindow` | local/signature/use | `src/host/macos/application/macos_host.mm:2111` | `for (NSWindow* createdWindow in nativeWindows) {` |
| `<file/function>` | `NSWindow` | local/signature/use | `src/host/macos/application/macos_host.mm:2150` | `for (NSWindow* nativeWindow in nativeWindows) {` |
| `<file/function>` | `GUIFormsView` | local/signature/use | `src/host/macos/application/macos_host.mm:2156` | `GUIFormsView* view = views[index];` |
| `<file/function>` | `NSWindow` | local/signature/use | `src/host/macos/application/macos_host.mm:2157` | `NSWindow* nativeWindow = nativeWindows[index];` |
| `<file/function>` | `NSCursor` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:89` | `NSCursor* native_cursor(CursorKind cursor) {` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:103` | `NSString* native_string(std::string_view text) {` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:110` | `NSString* string = nil;` |
| `<file/function>` | `const char` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:117` | `const char* bytes = string.UTF8String;` |
| `<file/function>` | `NSURL` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:126` | `NSURL* native_directory_url(const std::string& path) {` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:127` | `NSString* value = native_string(path);` |
| `<file/function>` | `NSArray<UTType*>` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:131` | `NSArray<UTType*>* native_allowed_types(` |
| `<file/function>` | `NSMutableArray<UTType*>` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:133` | `NSMutableArray<UTType*>* types = [[NSMutableArray alloc] init];` |
| `<file/function>` | `NSString` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:140` | `NSString* value = native_string(normalized);` |
| `<file/function>` | `UTType` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:141` | `UTType* type = value.length == 0` |
| `<file/function>` | `NSArray<NSURL*>` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:152` | `NSArray<NSURL*>* urls) {` |
| `<file/function>` | `NSURL` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:158` | `for (NSURL* url in urls) {` |
| `<file/function>` | `NSSavePanel` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:167` | `void schedule_test_panel_cancel(NSSavePanel* panel, bool enabled) {` |
| `<file/function>` | `NSAlert` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:176` | `void schedule_test_alert_cancel(NSAlert* alert, bool enabled) {` |
| `AppKitHostServices` | `NSString` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:194` | `NSString* name = [[NSString alloc]` |
| `AppKitHostServices` | `NSArray<NSScreen*>` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:208` | `NSArray<NSScreen*>* screens = [NSScreen screens];` |
| `AppKitHostServices` | `NSScreen` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:209` | `NSScreen* primary = screens.firstObject;` |
| `AppKitHostServices` | `NSScreen` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:211` | `for (NSScreen* screen in screens) {` |
| `AppKitHostServices` | `NSScreen` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:215` | `for (NSScreen* screen in screens) {` |
| `AppKitHostServices` | `NSNumber` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:218` | `NSNumber* screen_number = screen.deviceDescription[@"NSScreenNumber"];` |
| `AppKitHostServices` | `NSString` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:259` | `NSString* value = [pasteboard_ stringForType:NSPasteboardTypeString];` |
| `AppKitHostServices` | `NSData` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:263` | `NSData* encoded = [value dataUsingEncoding:NSUTF8StringEncoding];` |
| `AppKitHostServices` | `NSString` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:279` | `NSString* value = [[NSString alloc] initWithBytes:text_utf8.data()` |
| `AppKitHostServices` | `const HostMessageDialogRequest` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:292` | `if (const HostMessageDialogRequest* payload_pointer =` |
| `AppKitHostServices` | `NSAlert` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:295` | `NSAlert* alert = [[NSAlert alloc] init];` |
| `AppKitHostServices` | `NSButton` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:334` | `NSButton* defaultButton = nil;` |
| `AppKitHostServices` | `NSString` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:339` | `NSString* const title = button_choice.first;` |
| `AppKitHostServices` | `NSButton` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:341` | `NSButton* button = [alert addButtonWithTitle:title];` |
| `AppKitHostServices` | `NSEvent` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:361` | `NSEventMaskKeyDown handler:^NSEvent* (NSEvent* event) {` |
| `AppKitHostServices` | `const HostOpenFileDialogRequest` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:381` | `if (const HostOpenFileDialogRequest* payload_pointer =` |
| `AppKitHostServices` | `NSOpenPanel` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:384` | `NSOpenPanel* panel = [NSOpenPanel openPanel];` |
| `AppKitHostServices` | `NSArray<UTType*>` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:391` | `NSArray<UTType*>* types = native_allowed_types(payload.filters);` |
| `AppKitHostServices` | `const HostSaveFileDialogRequest` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:399` | `if (const HostSaveFileDialogRequest* payload_pointer =` |
| `AppKitHostServices` | `NSSavePanel` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:402` | `NSSavePanel* panel = [NSSavePanel savePanel];` |
| `AppKitHostServices` | `NSArray<UTType*>` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:406` | `NSArray<UTType*>* types = native_allowed_types(payload.filters);` |
| `AppKitHostServices` | `NSArray<NSURL*>` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:417` | `NSArray<NSURL*>* urls = panel.URL == nil ? @[] : @[panel.URL];` |
| `AppKitHostServices` | `const HostFolderDialogRequest` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:420` | `if (const HostFolderDialogRequest* payload_pointer =` |
| `AppKitHostServices` | `NSOpenPanel` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:423` | `NSOpenPanel* panel = [NSOpenPanel openPanel];` |
| `AppKitHostServices` | `const HostColorDialogRequest` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:433` | `if (const HostColorDialogRequest* payload_pointer =` |
| `AppKitHostServices` | `NSAlert` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:436` | `NSAlert* alert = [[NSAlert alloc] init];` |
| `AppKitHostServices` | `NSColorWell` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:438` | `NSColorWell* well = [[NSColorWell alloc]` |
| `AppKitHostServices` | `NSColor` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:453` | `NSColor* color = [well.color colorUsingColorSpace:[NSColorSpace sRGBColorSpace]];` |
| `AppKitHostServices` | `NSString` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:470` | `NSString* name = @"Ping";` |
| `AppKitHostServices` | `NSSound` | local/signature/use | `src/host/macos/services/appkit_host_services.mm:478` | `NSSound* sound = [NSSound soundNamed:name];` |
| `AppKitHostServices` | `NSPasteboard` | class declaration | `src/host/macos/services/appkit_host_services.mm:493` | `__strong NSPasteboard* pasteboard_{};` |
| `<file/function>` | `const char` | local/signature/use | `src/host/windows/application/windows_host.cpp:53` | `const char* value = std::getenv("GUI_FORMS_TRACE_WIN32_INPUT");` |
| `<file/function>` | `const char` | local/signature/use | `src/host/windows/application/windows_host.cpp:61` | `const char* value = std::getenv("GUI_FORMS_TRACE_WIN32_TEXT");` |
| `<file/function>` | `const wchar_t` | local/signature/use | `src/host/windows/application/windows_host.cpp:166` | `const wchar_t* cursor = path.data() + first.size() + 1U;` |
| `<file/function>` | `const char` | local/signature/use | `src/host/windows/application/windows_host.cpp:516` | `Function load_function(HMODULE module, const char* name) noexcept {` |
| `DibPainter` | `IWICImagingFactory` | local/signature/use | `src/host/windows/application/windows_host.cpp:601` | `IWICImagingFactory* factory{};` |
| `DibPainter` | `const std::byte` | local/signature/use | `src/host/windows/application/windows_host.cpp:1034` | `const std::byte* source = frame.pixels().data();` |
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
| `WindowsHostState` | `const char` | local/signature/use | `src/host/windows/application/windows_host.cpp:2353` | `const char* bytes = static_cast<const char*>((*data).lpData);` |
| `WindowsHostState` | `const wchar_t` | local/signature/use | `src/host/windows/application/windows_host.cpp:2641` | `for (const wchar_t* name : names) {` |
| `WindowsCompatibilityPaintEndpoint` | `void` | local/signature/use | `src/host/windows/paint_endpoint/windows_compatibility_paint_endpoint.cpp:141` | `void* next_pixels{};` |
| `WindowsCompatibilityPaintEndpoint` | `const std::byte` | local/signature/use | `src/host/windows/paint_endpoint/windows_compatibility_paint_endpoint.cpp:188` | `const std::byte* source = reinterpret_cast<const std::byte*>(pixels);` |
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
| `CoreGraphicsRaster` | `const RegisteredTypeface` | local/signature/use | `src/render/coregraphics/raster/coregraphics_raster.cpp:149` | `const RegisteredTypeface* selected = nullptr;` |
| `CoreGraphicsRaster` | `const ImageRegistry` | class declaration | `src/render/coregraphics/raster/coregraphics_raster.cpp:182` | `const ImageRegistry* image_registry{};` |
| `<file/function>` | `const void` | local/signature/use | `src/render/coregraphics/raster/coregraphics_raster.cpp:446` | `const void* CoreGraphicsRaster::pixels() const noexcept {` |
| `<file/function>` | `const void` | local/signature/use | `src/render/coregraphics/raster/coregraphics_raster.cpp:723` | `const void* keys[]{kCTFontAttributeName, kCTForegroundColorAttributeName,` |
| `<file/function>` | `const void` | local/signature/use | `src/render/coregraphics/raster/coregraphics_raster.cpp:725` | `const void* values[]{font, foreground, tracking};` |
| `<file/function>` | `const void` | local/signature/use | `src/render/coregraphics/raster/coregraphics_raster.cpp:769` | `const void* keys[]{kCTFontAttributeName, kCTKernAttributeName};` |
| `<file/function>` | `const void` | local/signature/use | `src/render/coregraphics/raster/coregraphics_raster.cpp:770` | `const void* values[]{font, tracking};` |
| `CoreGraphicsRaster` | `const void` | class declaration | `src/render/coregraphics/raster/coregraphics_raster.hpp:30` | `[[nodiscard]] const void* pixels() const noexcept;` |
| `<file/function>` | `RasterError` | local/signature/use | `src/render/skia/executor/skia_executor.cpp:105` | `void assign_raster_error(RasterError* output, RasterError value) noexcept {` |
| `BitmapUnlock` | `Bitmap` | class declaration | `src/render/skia/executor/skia_executor.cpp:495` | `Bitmap* bitmap_;` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/executor/skia_executor.cpp:701` | `SkCanvas* canvas = (*surface).getCanvas();` |
| `<file/function>` | `RasterError` | local/signature/use | `src/render/skia/executor/skia_executor.cpp:931` | `const Bitmap& bitmap, RasterError* error, const PngCodecLimits& limits) {` |
| `SkiaExecutor` | `RasterError` | class declaration | `src/render/skia/executor/skia_executor.hpp:74` | `const Bitmap& bitmap, RasterError* error = nullptr,` |
| `SkiaRaster` | `const ImageRegistry` | class declaration | `src/render/skia/raster/skia_raster.cpp:153` | `const ImageRegistry* image_registry{};` |
| `SkiaRaster` | `SkCanvas` | class declaration | `src/render/skia/raster/skia_raster.cpp:157` | `[[nodiscard]] SkCanvas* canvas() const noexcept {` |
| `SkiaRaster` | `const RegisteredTypeface` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:169` | `const RegisteredTypeface* registered = nullptr;` |
| `SkiaRaster` | `const char` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:188` | `const char* family = nullptr;` |
| `SkiaRaster` | `const char` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:220` | `const char* cursor = text.data();` |
| `SkiaRaster` | `const char` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:221` | `const char* const end = cursor + text.size();` |
| `SkiaRaster` | `const char` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:223` | `const char* const scalar_start = cursor;` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:349` | `SkCanvas* canvas = (*impl_).canvas();` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:378` | `if (SkCanvas* canvas = (*impl_).canvas()) {` |
| `<file/function>` | `const void` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:494` | `const void* SkiaRaster::pixels() const noexcept {` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:517` | `if (SkCanvas* canvas = (*impl_).canvas()) {` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:523` | `if (SkCanvas* canvas = (*impl_).canvas();` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:530` | `if (SkCanvas* canvas = (*impl_).canvas()) {` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:536` | `if (SkCanvas* canvas = (*impl_).canvas()) {` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:542` | `if (SkCanvas* canvas = (*impl_).canvas()) {` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:548` | `if (SkCanvas* canvas = (*impl_).canvas()) {` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:554` | `if (SkCanvas* canvas = (*impl_).canvas()) {` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:560` | `if (SkCanvas* canvas = (*impl_).canvas()) {` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:570` | `if (SkCanvas* canvas = (*impl_).canvas()) {` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:588` | `SkCanvas* canvas = (*impl_).canvas();` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:612` | `SkCanvas* canvas = (*impl_).canvas();` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:642` | `SkCanvas* canvas = (*impl_).canvas();` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:661` | `if (SkCanvas* canvas = (*impl_).canvas()) {` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:674` | `if (SkCanvas* canvas = (*impl_).canvas(); canvas && !text.empty()) {` |
| `<file/function>` | `const char` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:709` | `const char* bytes = text.data() + run.offset;` |
| `<file/function>` | `const char` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:729` | `const char* bytes = text.data() + run.offset;` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:742` | `if (SkCanvas* canvas = (*impl_).canvas();` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:754` | `SkCanvas* canvas = (*impl_).canvas();` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:797` | `SkCanvas* canvas = (*impl_).canvas();` |
| `<file/function>` | `SkCanvas` | local/signature/use | `src/render/skia/raster/skia_raster.cpp:823` | `SkCanvas* canvas = (*impl_).canvas();` |
| `SkiaRaster` | `const void` | class declaration | `src/render/skia/raster/skia_raster.hpp:31` | `[[nodiscard]] const void* pixels() const noexcept;` |
| `<file/function>` | `const char` | local/signature/use | `src/render/skia/skia_raster_smoke.cpp:32` | `std::vector<std::byte> read_file(const char* path) {` |
| `FacePreference` | `const Face` | class declaration | `src/render/text/harfbuzz/harfbuzz_font_engine.cpp:108` | `[[nodiscard]] bool operator()(const Face* left,` |
| `FacePreference` | `const Face` | class declaration | `src/render/text/harfbuzz/harfbuzz_font_engine.cpp:109` | `const Face* right) const noexcept {` |
| `Segment` | `Impl::Face` | class declaration | `src/render/text/harfbuzz/harfbuzz_font_engine.cpp:277` | `struct Segment final { Impl::Face* face{}; Utf8Range range{}; };` |
| `<file/function>` | `Impl::Face` | local/signature/use | `src/render/text/harfbuzz/harfbuzz_font_engine.cpp:282` | `Impl::Face* selected = nullptr;` |
| `<file/function>` | `Impl::Face` | local/signature/use | `src/render/text/harfbuzz/harfbuzz_font_engine.cpp:283` | `for (Impl::Face* candidate : candidates) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/animation_tests.cpp:20` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/basic_controls_tests.cpp:34` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/binary_search_tests.cpp:14` | `void require(bool condition, const char *message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/binding_tests.cpp:19` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/c_api_c11_tests.c:11` | `static void require(int condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/c_api_c11_tests.c:18` | `static gf_string_view text(const char* value) {` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:30` | `static void dispose_sender(gf_handle sender, uint32_t event_kind, void* opaque) {` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:37` | `static void count_callback(gf_handle sender, uint32_t event_kind, void* opaque) {` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:45` | `void* opaque) {` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:54` | `void* opaque) {` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:105` | `uint32_t button, void* opaque) {` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:122` | `static uint32_t prehost_dispatch(void* opaque, uint32_t cancelled) {` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:137` | `static uint32_t nested_dispatch_second(void* opaque, uint32_t cancelled) {` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:145` | `static uint32_t nested_dispatch_first(void* opaque, uint32_t cancelled) {` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:158` | `static uint32_t m11c_dispatch(void* opaque, uint32_t cancelled) {` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:169` | `static uint32_t m11c_event(gf_handle sender, uint32_t event_kind, void* opaque) {` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:196` | `void* opaque) {` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:210` | `static void* wrong_thread_worker(void* opaque) {` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:210` | `static void* wrong_thread_worker(void* opaque) {` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:335` | `static uint32_t property_get(void* opaque, gf_property_value* value,` |
| `<file/function>` | `char` | local/signature/use | `tests/c_api_c11_tests.c:336` | `char* output, uint64_t capacity,` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:350` | `static uint32_t property_set(void* opaque,` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:362` | `static uint32_t property_reset(void* opaque) {` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:370` | `static uint32_t property_should_serialize(void* opaque,` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:379` | `static uint32_t property_format(void* opaque,` |
| `<file/function>` | `char` | local/signature/use | `tests/c_api_c11_tests.c:381` | `char* output, uint64_t capacity,` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:404` | `static uint32_t property_parse(void* opaque, gf_string_view input,` |
| `<file/function>` | `char` | local/signature/use | `tests/c_api_c11_tests.c:406` | `char* output, uint64_t capacity,` |
| `<file/function>` | `char` | local/signature/use | `tests/c_api_c11_tests.c:411` | `char* end = NULL;` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:430` | `static uint32_t property_edit(void* opaque,` |
| `<file/function>` | `char` | local/signature/use | `tests/c_api_c11_tests.c:433` | `char* output, uint64_t capacity,` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:785` | `uint32_t repeat, void* context) {` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:1060` | `void* opaque) {` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:1145` | `uint32_t repeat, void* opaque) {` |
| `<file/function>` | `void` | local/signature/use | `tests/c_api_c11_tests.c:1153` | `int32_t replacement_length, void* opaque) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/c_api_cpp_tests.cpp:11` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/canvas_tests.cpp:13` | `[[noreturn]] void fail(const char* expression, int line) {` |
| `<file/function>` | `std::byte` | local/signature/use | `tests/canvas_tests.cpp:85` | `std::byte* pixel = edit.writable_data +` |
| `<file/function>` | `const char` | local/signature/use | `tests/checked_list_box_tests.cpp:13` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/collection_controls_tests.cpp:49` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/collection_controls_tests.cpp:295` | `const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/composition_controls_tests.cpp:14` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/core_tests.cpp:400` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/date_time_picker_tests.cpp:17` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/date_time_picker_tests.cpp:21` | `const SemanticNode* find_node(const std::vector<SemanticNode>& nodes,` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/date_time_picker_tests.cpp:25` | `if (const SemanticNode* nested = find_node(node.children, stable_id)) {` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/date_time_picker_tests.cpp:200` | `const SemanticNode* calendar =` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/date_time_picker_tests.cpp:202` | `const SemanticNode* selected =` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/date_time_picker_tests.cpp:285` | `const SemanticNode* previous =` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/date_time_picker_tests.cpp:287` | `const SemanticNode* next =` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/date_time_picker_tests.cpp:299` | `const SemanticNode* calendar =` |
| `<file/function>` | `const char` | local/signature/use | `tests/delegate_event_tests.cpp:21` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/dispatcher_tests.cpp:23` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/dispatcher_tests.cpp:88` | `void require_eventually(Predicate predicate, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/display_chunk_tests.cpp:17` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/drawing_c_api_c11_tests.c:16` | `static gd_string_view view_of(const char* text) {` |
| `<file/function>` | `char` | local/signature/use | `tests/drawing_c_api_c11_tests.c:51` | `char* trace = NULL;` |
| `<file/function>` | `const char` | local/signature/use | `tests/drawing_core_tests.cpp:19` | `[[noreturn]] void fail(const char* expression, int line) {` |
| `<file/function>` | `std::byte` | local/signature/use | `tests/drawing_core_tests.cpp:88` | `std::byte* pixel = edit.writable_data +` |
| `<file/function>` | `const char` | local/signature/use | `tests/drawing_raster_c_api_tests.cpp:13` | `[[noreturn]] void fail(const char* expression, int line) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/drawing_skia_tests.cpp:20` | `[[noreturn]] void fail(const char* expression, int line) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/drawing_skia_tests.cpp:27` | `std::vector<std::byte> read_file(const char* path) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/drawing_trace_equivalence_tests.cpp:14` | `[[noreturn]] void fail(const char* expression, int line) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/focus_scope_tests.cpp:17` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/frame_scheduler_tests.cpp:19` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/guidance_provider_tests.cpp:18` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/guidance_provider_tests.cpp:22` | `const SemanticNode* find_node(const std::vector<SemanticNode>& nodes,` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/guidance_provider_tests.cpp:26` | `if (const SemanticNode* found = find_node(node.children, stable_id)) {` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/guidance_provider_tests.cpp:166` | `const SemanticNode* target = find_node(` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/guidance_provider_tests.cpp:292` | `const SemanticNode* panel = find_node(` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/guidance_provider_tests.cpp:331` | `const SemanticNode* both = find_node(` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/guidance_provider_tests.cpp:339` | `const SemanticNode* remaining = find_node(` |
| `<file/function>` | `const char` | local/signature/use | `tests/harfbuzz_font_engine_tests.cpp:19` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/harfbuzz_font_engine_tests.cpp:23` | `std::vector<std::byte> read_file(const char* path) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/headless_trace_tests.cpp:11` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/host_protocol_tests.cpp:18` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/input_controls_tests.cpp:15` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/inspection_controls_tests.cpp:20` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/inspection_controls_tests.cpp:24` | `const SemanticNode* find_semantic(const std::vector<SemanticNode>& nodes,` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/inspection_controls_tests.cpp:28` | `const SemanticNode* found = find_semantic(node.children, id);` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/inspection_controls_tests.cpp:508` | `const SemanticNode* grid = find_semantic(semantic.roots,` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/inspection_controls_tests.cpp:510` | `const SemanticNode* group = find_semantic(semantic.roots,` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/inspection_controls_tests.cpp:512` | `const SemanticNode* row = find_semantic(semantic.roots, "inspection.kind");` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/inspection_controls_tests.cpp:513` | `const SemanticNode* editor = find_semantic(semantic.roots,` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/inspection_controls_tests.cpp:712` | `const SemanticNode* bounds_semantic = find_semantic(` |
| `<file/function>` | `const char` | local/signature/use | `tests/instrument_controls_tests.cpp:15` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/instrument_controls_tests.cpp:90` | `const SemanticNode* find_semantic(const std::vector<SemanticNode>& nodes,` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/instrument_controls_tests.cpp:94` | `if (const SemanticNode* nested = find_semantic(node.children, id)) {` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/instrument_controls_tests.cpp:181` | `const SemanticNode* group = find_semantic(semantics.roots, "criteria.rack");` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/instrument_controls_tests.cpp:182` | `const SemanticNode* staged = find_semantic(semantics.roots, "criteria.content");` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/instrument_controls_tests.cpp:183` | `const SemanticNode* state = find_semantic(semantics.roots,` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/instrument_controls_tests.cpp:185` | `const SemanticNode* apply = find_semantic(semantics.roots, "criteria.apply");` |
| `<file/function>` | `const char` | local/signature/use | `tests/invalidation_damage_tests.cpp:19` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/layout_panel_tests.cpp:16` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/lifecycle_controls_tests.cpp:17` | `void require(bool condition, const char* message) {` |
| `RearmProbeState` | `Window` | class declaration | `tests/macos_host_close_tests.mm:31` | `gui_forms::Window* window{};` |
| `RearmProbeState` | `FrameRequestToken` | class declaration | `tests/macos_host_close_tests.mm:32` | `gui_forms::FrameRequestToken* initial{};` |
| `PrepareCloseTestHost` | `Window` | class declaration | `tests/macos_host_close_tests.mm:203` | `gui_forms::Window* live_window,` |
| `PrepareCloseTestHost` | `Window` | class declaration | `tests/macos_host_close_tests.mm:252` | `gui_forms::Window* live_window_;` |
| `<file/function>` | `const char` | local/signature/use | `tests/macos_host_close_tests.mm:308` | `constexpr const char* pasteboard_name = "local.gui_forms.m3b.test";` |
| `<file/function>` | `NSString` | local/signature/use | `tests/macos_host_close_tests.mm:375` | `NSString* name = [NSString stringWithUTF8String:pasteboard_name];` |
| `<file/function>` | `Window` | local/signature/use | `tests/macos_host_close_tests.mm:415` | `gui_forms::Window* const live_window = window.get();` |
| `<file/function>` | `const char` | local/signature/use | `tests/material_tests.cpp:14` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/menu_controls_tests.cpp:18` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/menu_controls_tests.cpp:22` | `const SemanticNode* find_semantic(const std::vector<SemanticNode>& nodes,` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/menu_controls_tests.cpp:26` | `if (const SemanticNode* found = find_semantic(node.children, id)) return found;` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/menu_controls_tests.cpp:127` | `const SemanticNode* menu_node = find_semantic(` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/menu_controls_tests.cpp:129` | `const SemanticNode* details_node = find_semantic(` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/menu_controls_tests.cpp:131` | `const SemanticNode* delete_node = find_semantic(` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/menu_controls_tests.cpp:280` | `const SemanticNode* bar = find_semantic(snapshot.roots, "menustrip");` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/menu_controls_tests.cpp:281` | `const SemanticNode* file = find_semantic(snapshot.roots, "menustrip.file");` |
| `<file/function>` | `const char` | local/signature/use | `tests/png_registry_tests.cpp:25` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/range_controls_tests.cpp:18` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/retained_lifetime_tests.cpp:20` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/retained_lifetime_tests.cpp:480` | `void verify_ineligibility_cleanup(Mutation mutation, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/scrollable_control_tests.cpp:13` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/semantic_tests.cpp:17` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/semantic_tests.cpp:21` | `const SemanticNode* find_node(const std::vector<SemanticNode>& nodes,` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/semantic_tests.cpp:25` | `if (const SemanticNode* found = find_node(node.children, stable_id)) return found;` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/semantic_tests.cpp:96` | `const SemanticNode* group_node = find_node(first.roots, "semantics.group");` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/semantic_tests.cpp:97` | `const SemanticNode* field_node = find_node(first.roots, "semantics.field");` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/semantic_tests.cpp:98` | `const SemanticNode* check_node = find_node(first.roots, "semantics.check");` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/semantic_tests.cpp:99` | `const SemanticNode* progress_node = find_node(first.roots, "semantics.progress");` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/semantic_tests.cpp:199` | `const SemanticNode* list_node = find_node(before.roots, "virtual.list");` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/semantic_tests.cpp:200` | `const SemanticNode* item = find_node(before.roots, "virtual.list.item.1");` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/semantic_tests.cpp:210` | `const SemanticNode* selected = find_node(after.roots, "virtual.list.item.1");` |
| `<file/function>` | `const char` | local/signature/use | `tests/showcase_interaction_tests.cpp:18` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/showcase_interaction_tests.cpp:23` | `void require_eventually(Predicate predicate, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/split_container_tests.cpp:15` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/tab_control_tests.cpp:16` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/text_shaping_tests.cpp:16` | `void require(bool condition, const char *message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/text_store_tests.cpp:15` | `void require(bool condition, const char *message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/theme_tests.cpp:16` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/timer_tests.cpp:18` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/tooltip_tests.cpp:15` | `void require(bool condition, const char* message) {` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/tooltip_tests.cpp:19` | `const SemanticNode* find_role(const std::vector<SemanticNode>& nodes,` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/tooltip_tests.cpp:23` | `if (const SemanticNode* found = find_role(node.children, role)) return found;` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/tooltip_tests.cpp:104` | `const SemanticNode* tooltip = find_role(snapshot.roots, SemanticRole::tool_tip);` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/tooltip_tests.cpp:128` | `const SemanticNode* before =` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/tooltip_tests.cpp:136` | `const SemanticNode* after =` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/tooltip_tests.cpp:213` | `const SemanticNode* explicit_tip =` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/tooltip_tests.cpp:233` | `const SemanticNode* tip = find_role(snapshot.roots, SemanticRole::tool_tip);` |
| `<file/function>` | `const SemanticNode` | local/signature/use | `tests/tooltip_tests.cpp:256` | `const SemanticNode* tip = find_role(` |
| `<file/function>` | `const char` | local/signature/use | `tests/validation_tests.cpp:14` | `void require(bool condition, const char* message) {` |
| `AppendFocusTransition` | `const char` | class declaration | `tests/validation_tests.cpp:63` | `const char* label)` |
| `AppendFocusTransition` | `const char` | class declaration | `tests/validation_tests.cpp:75` | `const char* label_;` |
| `<file/function>` | `const char` | local/signature/use | `tests/windows_gdi_compat_shim_tests.cpp:27` | `Function entry(HMODULE module, const char* name) {` |
| `<file/function>` | `const char` | local/signature/use | `tests/windows_gdi_compat_shim_tests.cpp:35` | `gf_string_view text(const char* value) {` |
| `<file/function>` | `void` | local/signature/use | `tests/windows_gdi_compat_shim_tests.cpp:359` | `void* storage{};` |
| `FindingCollector` | `AutoType` | local/signature/use | `tools/house_policy_check/house_policy_check.cpp:222` | `const clang::AutoType* type = location.getTypePtr();` |
| `FindingCollector` | `NamedDecl` | local/signature/use | `tools/house_policy_check/house_policy_check.cpp:448` | `const clang::NamedDecl* named = parent.get<clang::NamedDecl>();` |
| `PolicyVisitor` | `VarDecl` | class declaration | `tools/house_policy_check/house_policy_check.cpp:523` | `bool VisitVarDecl(clang::VarDecl* declaration) {` |
| `PolicyVisitor` | `TypeSourceInfo` | local/signature/use | `tools/house_policy_check/house_policy_check.cpp:524` | `const clang::TypeSourceInfo* source_info =` |
| `PolicyVisitor` | `LambdaExpr` | class declaration | `tools/house_policy_check/house_policy_check.cpp:538` | `bool VisitLambdaExpr(clang::LambdaExpr* expression) {` |
| `PolicyVisitor` | `DecompositionDecl` | class declaration | `tools/house_policy_check/house_policy_check.cpp:544` | `bool VisitDecompositionDecl(clang::DecompositionDecl* declaration) {` |
| `PolicyVisitor` | `MemberExpr` | class declaration | `tools/house_policy_check/house_policy_check.cpp:550` | `bool VisitMemberExpr(clang::MemberExpr* expression) {` |
| `PolicyVisitor` | `CXXDependentScopeMemberExpr` | class declaration | `tools/house_policy_check/house_policy_check.cpp:558` | `clang::CXXDependentScopeMemberExpr* expression) {` |
| `PolicyVisitor` | `FunctionDecl` | class declaration | `tools/house_policy_check/house_policy_check.cpp:565` | `bool VisitFunctionDecl(clang::FunctionDecl* declaration) {` |
| `PolicyVisitor` | `TypeSourceInfo` | local/signature/use | `tools/house_policy_check/house_policy_check.cpp:567` | `const clang::TypeSourceInfo* source_info =` |
| `PolicyVisitor` | `FunctionProtoType` | local/signature/use | `tools/house_policy_check/house_policy_check.cpp:589` | `const clang::FunctionProtoType* prototype =` |
| `PolicyVisitor` | `IfStmt` | class declaration | `tools/house_policy_check/house_policy_check.cpp:617` | `bool VisitIfStmt(clang::IfStmt* statement) {` |
| `PolicyVisitor` | `RequiresExpr` | class declaration | `tools/house_policy_check/house_policy_check.cpp:626` | `bool VisitRequiresExpr(clang::RequiresExpr* expression) {` |
| `PolicyVisitor` | `CoroutineBodyStmt` | class declaration | `tools/house_policy_check/house_policy_check.cpp:638` | `bool VisitCoroutineBodyStmt(clang::CoroutineBodyStmt* statement) {` |
| `PolicyVisitor` | `CoawaitExpr` | class declaration | `tools/house_policy_check/house_policy_check.cpp:644` | `bool VisitCoawaitExpr(clang::CoawaitExpr* expression) {` |
| `PolicyVisitor` | `CoyieldExpr` | class declaration | `tools/house_policy_check/house_policy_check.cpp:650` | `bool VisitCoyieldExpr(clang::CoyieldExpr* expression) {` |
| `PolicyVisitor` | `CoreturnStmt` | class declaration | `tools/house_policy_check/house_policy_check.cpp:656` | `bool VisitCoreturnStmt(clang::CoreturnStmt* statement) {` |
| `PolicyVisitor` | `DesignatedInitExpr` | class declaration | `tools/house_policy_check/house_policy_check.cpp:662` | `bool VisitDesignatedInitExpr(clang::DesignatedInitExpr* expression) {` |
| `PolicyVisitor` | `ConceptSpecializationExpr` | class declaration | `tools/house_policy_check/house_policy_check.cpp:670` | `clang::ConceptSpecializationExpr* expression) {` |
| `PolicyVisitor` | `DeclRefExpr` | class declaration | `tools/house_policy_check/house_policy_check.cpp:700` | `bool VisitDeclRefExpr(clang::DeclRefExpr* expression) {` |
| `PolicyVisitor` | `NamedDecl` | local/signature/use | `tools/house_policy_check/house_policy_check.cpp:701` | `const clang::NamedDecl* declaration = expression->getFoundDecl();` |
| `<file/function>` | `const char` | local/signature/use | `tools/windows_automation_probe.cpp:13` | `std::wstring wide_from_utf8(const char* text) {` |
| `<file/function>` | `void` | local/signature/use | `tools/windows_automation_probe.cpp:219` | `void* captured_pixels{};` |
| `<file/function>` | `const char` | local/signature/use | `tools/windows_automation_probe.cpp:293` | `if (const char* requested_handle = std::getenv("GUI_FORMS_AUTOMATION_HANDLE");` |

## Lifetime-operation evidence

| Operation | Location | Evidence |
|---|---|---|
| `make_unique` | `demo/gallery.cpp:15` | `std::unique_ptr<gui_forms::Window> window = std::make_unique<Window>(std::move(tree.root), Size {900.0, 660.0});` |
| `make_unique` | `demo/showcase.cpp:150` | `std::unique_ptr<gui_forms::Window> window = std::make_unique<Window>(std::move(tree.root),` |
| `dynamic_pointer_cast` | `demo/showcase.cpp:165` | `if (const std::shared_ptr<gui_forms::PictureBox> picture = std::dynamic_pointer_cast<PictureBox>(` |
| `dynamic_pointer_cast` | `demo/showcase.cpp:172` | `if (const std::shared_ptr<gui_forms::PictureBox> picture = std::dynamic_pointer_cast<PictureBox>(` |
| `make_shared` | `demo/showcase.cpp:189` | `std::shared_ptr<gui_forms::ImageList> icons = std::make_shared<ImageList>(*window, Size{16.0, 16.0});` |
| `dynamic_pointer_cast` | `demo/showcase.cpp:195` | `if (const std::shared_ptr<gui_forms::Button> button = std::dynamic_pointer_cast<Button>(` |
| `dynamic_pointer_cast` | `demo/showcase.cpp:208` | `if (const std::shared_ptr<gui_forms::MaterialPanel> panel = std::dynamic_pointer_cast<MaterialPanel>(` |
| `dynamic_pointer_cast` | `demo/showcase.cpp:226` | `if (const std::shared_ptr<gui_forms::MaterialPanel> panel = std::dynamic_pointer_cast<MaterialPanel>(` |
| `shared_from_this` | `demo/vsync_lab.cpp:123` | `self = std::static_pointer_cast<WaterfallSurface>(shared_from_this());` |
| `static_pointer_cast` | `demo/vsync_lab.cpp:123` | `self = std::static_pointer_cast<WaterfallSurface>(shared_from_this());` |
| `weak_lock` | `demo/vsync_lab.cpp:152` | `const std::shared_ptr<WaterfallSurface> target = surface.lock();` |
| `weak_lock` | `demo/vsync_lab.cpp:169` | `const std::shared_ptr<WaterfallSurface> target = surface.lock();` |
| `shared_from_this` | `demo/vsync_lab.cpp:340` | `self = std::static_pointer_cast<WaterfallSurface>(shared_from_this());` |
| `static_pointer_cast` | `demo/vsync_lab.cpp:340` | `self = std::static_pointer_cast<WaterfallSurface>(shared_from_this());` |
| `shared_from_this` | `demo/vsync_lab.cpp:357` | `std::static_pointer_cast<WaterfallSurface>(shared_from_this());` |
| `static_pointer_cast` | `demo/vsync_lab.cpp:357` | `std::static_pointer_cast<WaterfallSurface>(shared_from_this());` |
| `subscription_token` | `demo/vsync_lab.cpp:403` | `std::vector<SubscriptionToken> subscriptions;` |
| `weak_lock` | `demo/vsync_lab.cpp:420` | `const std::shared_ptr<LabContext> state = context.lock();` |
| `weak_lock` | `demo/vsync_lab.cpp:445` | `const std::shared_ptr<LabContext> state = context.lock();` |
| `weak_lock` | `demo/vsync_lab.cpp:457` | `const std::shared_ptr<LabContext> state = context.lock();` |
| `weak_lock` | `demo/vsync_lab.cpp:470` | `const std::shared_ptr<LabContext> state = context.lock();` |
| `weak_lock` | `demo/vsync_lab.cpp:484` | `const std::shared_ptr<LabContext> state = context.lock();` |
| `make_shared` | `demo/vsync_lab.cpp:545` | `std::shared_ptr<LabContext> context = std::make_shared<LabContext>();` |
| `make_unique` | `demo/vsync_lab.cpp:717` | `std::unique_ptr<gui_forms::Window> window = std::make_unique<Window>(root, Size{1280.0, 780.0});` |
| `make_unique` | `demo/vsync_lab.cpp:718` | `(*context).telemetry_timer = std::make_unique<Timer>(*window, 250ms);` |
| `weak_lock` | `include/gui_forms/binding/binding/binding.hpp:27` | `return source_.lock();` |
| `subscription_token` | `include/gui_forms/binding/binding/binding.hpp:90` | `SubscriptionToken source_changed_;` |
| `subscription_token` | `include/gui_forms/binding/binding/binding.hpp:91` | `SubscriptionToken source_disposed_;` |
| `subscription_token` | `include/gui_forms/binding/binding/binding.hpp:92` | `SubscriptionToken target_changed_;` |
| `subscription_token` | `include/gui_forms/binding/binding/binding.hpp:93` | `SubscriptionToken target_validating_;` |
| `subscription_token` | `include/gui_forms/binding/binding_context/binding_context.hpp:39` | `SubscriptionToken disposed;` |
| `subscription_token` | `include/gui_forms/binding/value/binding_value.hpp:239` | `using ChangeConnector = std::function<SubscriptionToken(` |
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
| `weak_lock` | `include/gui_forms/components/error_provider/error_provider.hpp:114` | `return data_source_.lock();` |
| `subscription_token` | `include/gui_forms/components/error_provider/error_provider.hpp:175` | `SubscriptionToken availability_subscription_;` |
| `subscription_token` | `include/gui_forms/components/error_provider/error_provider.hpp:176` | `SubscriptionToken presentation_subscription_;` |
| `subscription_token` | `include/gui_forms/components/error_provider/error_provider.hpp:177` | `SubscriptionToken root_bounds_subscription_;` |
| `subscription_token` | `include/gui_forms/components/error_provider/error_provider.hpp:178` | `SubscriptionToken source_list_subscription_;` |
| `subscription_token` | `include/gui_forms/components/error_provider/error_provider.hpp:179` | `SubscriptionToken source_current_subscription_;` |
| `subscription_token` | `include/gui_forms/components/error_provider/error_provider.hpp:180` | `SubscriptionToken source_completion_subscription_;` |
| `subscription_token` | `include/gui_forms/components/error_provider/error_provider.hpp:181` | `SubscriptionToken source_disposed_subscription_;` |
| `subscription_token` | `include/gui_forms/components/tool_tip/tool_tip.hpp:116` | `SubscriptionToken timer_subscription_;` |
| `subscription_token` | `include/gui_forms/components/tool_tip/tool_tip.hpp:117` | `SubscriptionToken popup_subscription_;` |
| `weak_lock` | `include/gui_forms/control/control/control.hpp:341` | `[[nodiscard]] Ptr parent() const noexcept { return parent_.lock(); }` |
| `subscription_token` | `include/gui_forms/control/control/control.hpp:550` | `[[nodiscard]] SubscriptionToken subscribe_property_changed(` |
| `subscription_token` | `include/gui_forms/control/control/control.hpp:988` | `SubscriptionToken operator()(Component& owner,` |
| `make_shared` | `include/gui_forms/control/control/control.hpp:1017` | `std::shared_ptr<ControlType> control = std::make_shared<ControlType>(` |
| `subscription_token` | `include/gui_forms/controls/button_base/button_base.hpp:154` | `SubscriptionToken image_list_changed_;` |
| `subscription_token` | `include/gui_forms/controls/easing_preview/easing_preview.hpp:70` | `SubscriptionToken presentation_subscription_;` |
| `subscription_token` | `include/gui_forms/controls/menu_strip/menu_strip.hpp:102` | `SubscriptionToken popup_invoked_;` |
| `subscription_token` | `include/gui_forms/controls/menu_strip/menu_strip.hpp:103` | `SubscriptionToken popup_changed_;` |
| `weak_lock` | `include/gui_forms/controls/panel/anchored_popup_layer/anchored_popup_layer.hpp:27` | `[[nodiscard]] Control::Ptr anchor() const noexcept { return anchor_.lock(); }` |
| `subscription_token` | `include/gui_forms/controls/panel/color_value_editor/color_value_editor.hpp:50` | `SubscriptionToken committed_;` |
| `subscription_token` | `include/gui_forms/controls/panel/color_value_editor/color_value_editor.hpp:51` | `SubscriptionToken cancelled_;` |
| `subscription_token` | `include/gui_forms/controls/panel/combo_box/combo_box.hpp:98` | `SubscriptionToken popup_selection_;` |
| `subscription_token` | `include/gui_forms/controls/panel/combo_box/combo_box.hpp:99` | `SubscriptionToken popup_activation_;` |
| `subscription_token` | `include/gui_forms/controls/panel/combo_box/combo_box.hpp:100` | `SubscriptionToken popup_dismissal_;` |
| `subscription_token` | `include/gui_forms/controls/panel/combo_box/combo_box.hpp:101` | `SubscriptionToken popup_revocation_;` |
| `subscription_token` | `include/gui_forms/controls/panel/date_time_picker/date_time_picker.hpp:120` | `SubscriptionToken popup_commit_;` |
| `subscription_token` | `include/gui_forms/controls/panel/date_time_picker/date_time_picker.hpp:121` | `SubscriptionToken popup_cancel_;` |
| `subscription_token` | `include/gui_forms/controls/panel/date_time_picker/date_time_picker.hpp:122` | `SubscriptionToken popup_dismiss_;` |
| `subscription_token` | `include/gui_forms/controls/panel/date_time_picker/date_time_picker.hpp:123` | `SubscriptionToken popup_revocation_;` |
| `subscription_token` | `include/gui_forms/controls/panel/flags_value_editor/flags_value_editor.hpp:60` | `SubscriptionToken popup_check_;` |
| `subscription_token` | `include/gui_forms/controls/panel/flags_value_editor/flags_value_editor.hpp:61` | `SubscriptionToken popup_dismissal_;` |
| `subscription_token` | `include/gui_forms/controls/panel/flags_value_editor/flags_value_editor.hpp:62` | `SubscriptionToken popup_revocation_;` |
| `subscription_token` | `include/gui_forms/controls/panel/numeric_up_down/numeric_up_down.hpp:60` | `SubscriptionToken editor_change_;` |
| `subscription_token` | `include/gui_forms/controls/panel/numeric_up_down/numeric_up_down.hpp:61` | `SubscriptionToken spinner_step_;` |
| `subscription_token` | `include/gui_forms/controls/panel/object_view/object_view.hpp:183` | `SubscriptionToken image_list_changed_;` |
| `subscription_token` | `include/gui_forms/controls/panel/tree_view/tree_view.hpp:126` | `SubscriptionToken image_list_changed_;` |
| `subscription_token` | `include/gui_forms/controls/range_control/progress_bar/progress_bar.hpp:114` | `SubscriptionToken presentation_subscription_;` |
| `weak_lock` | `include/gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp:46` | `return selected_page_.lock();` |
| `subscription_token` | `include/gui_forms/detail/property_binding_adapters.hpp:125` | `SubscriptionToken operator()(Component& owner,` |
| `weak_lock` | `include/gui_forms/detail/weak_member_callback.hpp:23` | `const std::shared_ptr<Object> object = object_.lock();` |
| `weak_lock` | `include/gui_forms/detail/weak_member_callback.hpp:43` | `const std::shared_ptr<Object> object = object_.lock();` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:15` | `class SubscriptionToken final {` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:17` | `SubscriptionToken() = default;` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:18` | `~SubscriptionToken() { disconnect(); }` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:19` | `SubscriptionToken(SubscriptionToken&& other) noexcept` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:19` | `SubscriptionToken(SubscriptionToken&& other) noexcept` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:21` | `SubscriptionToken& operator=(SubscriptionToken&& other) noexcept {` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:21` | `SubscriptionToken& operator=(SubscriptionToken&& other) noexcept {` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:28` | `SubscriptionToken(const SubscriptionToken&) = delete;` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:28` | `SubscriptionToken(const SubscriptionToken&) = delete;` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:29` | `SubscriptionToken& operator=(const SubscriptionToken&) = delete;` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:29` | `SubscriptionToken& operator=(const SubscriptionToken&) = delete;` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:44` | `explicit SubscriptionToken(std::shared_ptr<detail::Revocable> revocable)` |
| `make_shared` | `include/gui_forms/event/event/event.hpp:65` | `Event() : state_(std::make_shared<State>()) {}` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:70` | `[[nodiscard]] SubscriptionToken subscribe(Callback callback) {` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:74` | `[[nodiscard]] SubscriptionToken subscribe(DelegateCallback callback) {` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:78` | `[[nodiscard]] SubscriptionToken subscribe(Component& owner, Callback callback) {` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:82` | `[[nodiscard]] SubscriptionToken subscribe(Component& owner,` |
| `weak_lock` | `include/gui_forms/event/event/event.hpp:129` | `const std::shared_ptr<State> event_state = state.lock();` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:166` | `[[nodiscard]] SubscriptionToken subscribe_impl(Component* owner, Callback callback) {` |
| `make_shared` | `include/gui_forms/event/event/event.hpp:171` | `std::make_shared<Slot>(state_, std::move(callback));` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:173` | `return SubscriptionToken(slot);` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:176` | `[[nodiscard]] SubscriptionToken subscribe_impl(Component* owner,` |
| `make_shared` | `include/gui_forms/event/event/event.hpp:182` | `std::make_shared<Slot>(state_, callback);` |
| `subscription_token` | `include/gui_forms/event/event/event.hpp:184` | `return SubscriptionToken(slot);` |
| `owner_revocable` | `include/gui_forms/event/event/event.hpp:191` | `(*owner).own_revocable(slot);` |
| `subscription_token` | `include/gui_forms/host/session/host_session.hpp:54` | `SubscriptionToken capture_observation_;` |
| `subscription_token` | `include/gui_forms/host/session/host_session.hpp:55` | `SubscriptionToken modal_observation_;` |
| `subscription_token` | `include/gui_forms/inspection/property_editor_registry/property_editor_registry.hpp:17` | `std::function<SubscriptionToken(` |
| `subscription_token` | `include/gui_forms/inspection/property_editor_registry/property_editor_registry.hpp:19` | `std::function<SubscriptionToken(` |
| `weak_lock` | `include/gui_forms/window/window.hpp:375` | `[[nodiscard]] Control::Ptr focused_control() const noexcept { return focused_.lock(); }` |
| `weak_lock` | `include/gui_forms/window/window.hpp:396` | `return accept_button_.lock();` |
| `weak_lock` | `include/gui_forms/window/window.hpp:399` | `return cancel_button_.lock();` |
| `weak_lock` | `include/gui_forms/window/window.hpp:414` | `[[nodiscard]] Control::Ptr captured_control() const noexcept { return captured_.lock(); }` |
| `weak_lock` | `include/gui_forms/window/window.hpp:434` | `[[nodiscard]] Control::Ptr pressed_control() const noexcept { return pressed_.lock(); }` |
| `subscription_token` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:25` | `gui_forms::SubscriptionToken operator()(` |
| `weak_lock` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:62` | `const std::shared_ptr<State> retained = state.lock();` |
| `weak_lock` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:77` | `const std::shared_ptr<State> retained = state.lock();` |
| `weak_lock` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:92` | `const std::shared_ptr<gui_forms::Button> editor = button.lock();` |
| `weak_lock` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:93` | `const std::shared_ptr<State> retained = state.lock();` |
| `weak_lock` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:126` | `const std::shared_ptr<State> retained = state.lock();` |
| `subscription_token` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:146` | `gui_forms::SubscriptionToken operator()(` |
| `weak_lock` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:149` | `const std::shared_ptr<gui_forms::Button> editor = button.lock();` |
| `subscription_token` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:161` | `gui_forms::SubscriptionToken operator()(` |
| `weak_lock` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:175` | `const std::shared_ptr<State> retained = state.lock();` |
| `make_shared` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:184` | `std::make_shared<gui_forms::BindingValue>(request.value);` |
| `make_shared` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:187` | `std::make_shared<gui_forms::Event<` |
| `make_shared` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:234` | `std::shared_ptr<gui_forms::abi::detail::AbiPropertyObjectControl::PropertyState> state = std::make_shared<PropertyState>();` |
| `make_shared` | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:573` | `std::make_shared<gui_forms::PropertyEnumDescriptor>();` |
| `static_pointer_cast` | `src/abi/control_adapters/raster_control/raster_control.hpp:159` | `std::static_pointer_cast<RasterControl>(` |
| `shared_from_this` | `src/abi/control_adapters/raster_control/raster_control.hpp:160` | `shared_from_this()),` |
| `weak_lock` | `src/abi/control_adapters/raster_control/raster_control.hpp:261` | `const std::shared_ptr<LiveWakeState> state = weak_state.lock();` |
| `weak_lock` | `src/abi/control_adapters/raster_control/raster_control.hpp:266` | `const std::shared_ptr<RasterControl> target = weak_target.lock();` |
| `weak_lock` | `src/abi/control_adapters/raster_control/raster_control.hpp:287` | `const std::shared_ptr<gui_forms::abi::detail::RasterControl::LiveWakeState> state = weak_state.lock();` |
| `weak_lock` | `src/abi/control_adapters/raster_control/raster_control.hpp:292` | `const std::shared_ptr<gui_forms::abi::detail::RasterControl> target = weak_target.lock();` |
| `static_pointer_cast` | `src/abi/control_adapters/raster_control/raster_control.hpp:308` | `const std::shared_ptr<gui_forms::abi::detail::RasterControl> self = std::static_pointer_cast<RasterControl>(` |
| `shared_from_this` | `src/abi/control_adapters/raster_control/raster_control.hpp:309` | `shared_from_this());` |
| `make_shared` | `src/abi/control_adapters/raster_control/raster_control.hpp:317` | `std::shared_ptr<gui_forms::abi::detail::RasterControl::LiveWakeState> wake_state = std::make_shared<LiveWakeState>();` |
| `dynamic_pointer_cast` | `src/abi/drawing_c_api.cpp:231` | `kind == 0U ? std::dynamic_pointer_cast<Object>((*record).object) :` |
| `static_pointer_cast` | `src/abi/drawing_c_api.cpp:232` | `std::static_pointer_cast<Object>((*record).object);` |
| `dynamic_pointer_cast` | `src/abi/drawing_c_api.cpp:267` | `std::dynamic_pointer_cast<Left>((*left_record).object) :` |
| `static_pointer_cast` | `src/abi/drawing_c_api.cpp:268` | `std::static_pointer_cast<Left>((*left_record).object);` |
| `dynamic_pointer_cast` | `src/abi/drawing_c_api.cpp:271` | `std::dynamic_pointer_cast<Right>((*right_record).object) :` |
| `static_pointer_cast` | `src/abi/drawing_c_api.cpp:272` | `std::static_pointer_cast<Right>((*right_record).object);` |
| `dynamic_pointer_cast` | `src/abi/drawing_c_api.cpp:314` | `std::shared_ptr<gui_drawing::GraphicsRecorder> recorder = std::dynamic_pointer_cast<gui_drawing::GraphicsRecorder>((*recorder_record).object);` |
| `dynamic_pointer_cast` | `src/abi/drawing_c_api.cpp:315` | `std::shared_ptr<gui_drawing::Font> font = std::dynamic_pointer_cast<gui_drawing::Font>((*font_record).object);` |
| `dynamic_pointer_cast` | `src/abi/drawing_c_api.cpp:316` | `std::shared_ptr<gui_drawing::SolidBrush> brush = std::dynamic_pointer_cast<gui_drawing::SolidBrush>((*brush_record).object);` |
| `dynamic_pointer_cast` | `src/abi/drawing_c_api.cpp:317` | `std::shared_ptr<gui_drawing::StringFormat> format = std::dynamic_pointer_cast<gui_drawing::StringFormat>((*format_record).object);` |
| `make_shared` | `src/abi/drawing_c_api.cpp:330` | `std::make_shared<ObjectRecord>();` |
| `make_shared` | `src/abi/drawing_c_api.cpp:332` | `std::make_shared<Object>(std::forward<Arguments>(arguments)...);` |
| `make_shared` | `src/abi/drawing_c_api.cpp:344` | `std::make_shared<ObjectRecord>();` |
| `weak_lock` | `src/abi/drawing_c_api.cpp:736` | `const gui_drawing::BitmapLockView native = bitmap.lock(` |
| `delete_expression` | `src/abi/drawing_c_api.cpp:1947` | `delete static_cast<gui_drawing::Bitmap*>(decoded);` |
| `make_unique` | `src/abi/drawing_platform_windows.cpp:194` | `auto bitmap = std::make_unique<Bitmap>(width, height,` |
| `weak_lock` | `src/abi/drawing_platform_windows.cpp:196` | `BitmapLockView lock = (*bitmap).lock(BitmapLockMode::write);` |
| `make_unique` | `src/abi/drawing_platform_windows.cpp:274` | `auto bitmap = std::make_unique<Bitmap>(width, height,` |
| `weak_lock` | `src/abi/drawing_platform_windows.cpp:276` | `BitmapLockView lock = (*bitmap).lock(BitmapLockMode::write);` |
| `weak_lock` | `src/abi/drawing_platform_windows.cpp:324` | `BitmapLockView lock = bitmap.lock(BitmapLockMode::write);` |
| `weak_lock` | `src/abi/drawing_platform_windows.cpp:482` | `BitmapLockView lock = bitmap.lock(BitmapLockMode::write);` |
| `weak_lock` | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:98` | `if (const std::shared_ptr<gui_forms::abi::detail::RasterControl> raster = target.lock()) {` |
| `weak_lock` | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:132` | `if (const std::shared_ptr<gui_forms::abi::detail::RasterControl> raster = binding.target.lock()) {` |
| `weak_lock` | `src/abi/paint_endpoint/windows_compatibility_paint_endpoint.cpp:232` | `if (const std::shared_ptr<gui_forms::abi::detail::RasterControl> raster = binding.target.lock()) {` |
| `subscription_token` | `src/abi/registry/registry.hpp:25` | `gui_forms::SubscriptionToken native_subscription;` |
| `make_shared` | `src/abi/registry/registry.hpp:108` | `std::shared_ptr<gui_forms::abi::detail::ControlRecord> record = std::make_shared<ControlRecord>();` |
| `make_shared` | `src/abi/registry/registry.hpp:112` | `(*record).control = std::make_shared<FormControl>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:115` | `(*record).control = std::make_shared<gui_forms::Panel>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:118` | `std::shared_ptr<gui_forms::Panel> panel = std::make_shared<gui_forms::Panel>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:127` | `(*record).control = std::make_shared<gui_forms::Button>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:130` | `(*record).control = std::make_shared<gui_forms::CheckBox>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:133` | `(*record).control = std::make_shared<gui_forms::Label>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:136` | `(*record).control = std::make_shared<FieldControl>(` |
| `make_shared` | `src/abi/registry/registry.hpp:140` | `(*record).control = std::make_shared<FieldControl>(` |
| `make_shared` | `src/abi/registry/registry.hpp:144` | `(*record).control = std::make_shared<FieldControl>(` |
| `make_shared` | `src/abi/registry/registry.hpp:148` | `(*record).control = std::make_shared<gui_forms::TrackBar>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:151` | `(*record).control = std::make_shared<gui_forms::RadioButton>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:154` | `(*record).control = std::make_shared<gui_forms::GroupBox>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:157` | `(*record).control = std::make_shared<gui_forms::ProgressBar>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:160` | `(*record).control = std::make_shared<gui_forms::LinkLabel>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:163` | `(*record).control = std::make_shared<FieldControl>(` |
| `make_shared` | `src/abi/registry/registry.hpp:167` | `(*record).control = std::make_shared<FieldControl>(` |
| `make_shared` | `src/abi/registry/registry.hpp:171` | `(*record).control = std::make_shared<FieldControl>(` |
| `make_shared` | `src/abi/registry/registry.hpp:175` | `(*record).control = std::make_shared<FieldControl>(` |
| `make_shared` | `src/abi/registry/registry.hpp:180` | `std::make_shared<InputTransparentControl>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:184` | `std::make_shared<RasterControl>(std::move(native_id), true);` |
| `make_shared` | `src/abi/registry/registry.hpp:188` | `std::make_shared<gui_forms::PropertyGrid>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:195` | `std::make_shared<AbiPropertyObjectControl>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:198` | `(*record).control = std::make_shared<RasterControl>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:202` | `(*record).control = std::make_shared<RasterControl>(std::move(native_id));` |
| `make_shared` | `src/abi/registry/registry.hpp:205` | `(*record).control = std::make_shared<Control>(std::move(native_id));` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:238` | `std::dynamic_pointer_cast<gui_forms::ButtonBase>((*record).control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:241` | `std::dynamic_pointer_cast<gui_forms::Label>((*record).control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:244` | `std::dynamic_pointer_cast<gui_forms::GroupBox>((*record).control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:247` | `std::dynamic_pointer_cast<FieldControl>((*record).control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:357` | `const std::shared_ptr<gui_forms::ScrollableControl> scrollable = std::dynamic_pointer_cast<` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:377` | `const std::shared_ptr<gui_forms::ScrollableControl> scrollable = std::dynamic_pointer_cast<` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:397` | `const std::shared_ptr<gui_forms::ScrollableControl> scrollable = std::dynamic_pointer_cast<` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:416` | `const std::shared_ptr<gui_forms::ScrollableControl> scrollable = std::dynamic_pointer_cast<` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:435` | `const std::shared_ptr<gui_forms::ScrollableControl> scrollable = std::dynamic_pointer_cast<` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:501` | `const std::shared_ptr<gui_forms::ScrollableControl> scrollable = std::dynamic_pointer_cast<` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:529` | `const std::shared_ptr<gui_forms::ScrollableControl> scrollable = std::dynamic_pointer_cast<` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:606` | `const std::shared_ptr<gui_forms::PropertyGrid> grid = std::dynamic_pointer_cast<gui_forms::PropertyGrid>(` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:635` | `std::dynamic_pointer_cast<AbiPropertyObjectControl>(control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:672` | `std::dynamic_pointer_cast<AbiPropertyObjectControl>((*record).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:694` | `std::dynamic_pointer_cast<AbiPropertyObjectControl>((*record).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:733` | `const std::shared_ptr<gui_forms::PropertyGrid> grid = std::dynamic_pointer_cast<gui_forms::PropertyGrid>(` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:763` | `const std::shared_ptr<gui_forms::PropertyGrid> grid = std::dynamic_pointer_cast<gui_forms::PropertyGrid>(` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:793` | `const std::shared_ptr<gui_forms::PropertyGrid> grid = std::dynamic_pointer_cast<gui_forms::PropertyGrid>(` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:814` | `const std::shared_ptr<gui_forms::PropertyGrid> grid = std::dynamic_pointer_cast<gui_forms::PropertyGrid>(` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:838` | `const std::shared_ptr<gui_forms::PropertyGrid> grid = std::dynamic_pointer_cast<gui_forms::PropertyGrid>(` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:856` | `const std::shared_ptr<gui_forms::PropertyGrid> grid = std::dynamic_pointer_cast<gui_forms::PropertyGrid>(` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:879` | `const std::shared_ptr<gui_forms::abi::detail::RasterControl> raster = std::dynamic_pointer_cast<RasterControl>((*record).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:910` | `const std::shared_ptr<gui_forms::abi::detail::RasterControl> raster = std::dynamic_pointer_cast<RasterControl>((*record).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:929` | `const std::shared_ptr<gui_forms::abi::detail::RasterControl> raster = std::dynamic_pointer_cast<RasterControl>((*record).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:955` | `const std::shared_ptr<gui_forms::abi::detail::RasterControl> raster = std::dynamic_pointer_cast<RasterControl>((*record).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1004` | `if (const std::shared_ptr<gui_forms::abi::detail::FieldControl> field = std::dynamic_pointer_cast<FieldControl>((*record).control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1007` | `std::dynamic_pointer_cast<gui_forms::Label>((*record).control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1010` | `std::dynamic_pointer_cast<gui_forms::ButtonBase>((*record).control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1018` | `std::dynamic_pointer_cast<gui_forms::Panel>((*record).control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1045` | `std::dynamic_pointer_cast<gui_forms::Label>((*record).control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1051` | `std::dynamic_pointer_cast<gui_forms::ButtonBase>((*record).control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1073` | `std::dynamic_pointer_cast<gui_forms::Button>((*record).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1095` | `std::dynamic_pointer_cast<gui_forms::Panel>((*record).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1119` | `const std::shared_ptr<gui_forms::abi::detail::FieldControl> field = std::dynamic_pointer_cast<FieldControl>((*record).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1142` | `const std::shared_ptr<gui_forms::abi::detail::FieldControl> field = std::dynamic_pointer_cast<FieldControl>((*record).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1164` | `const std::shared_ptr<gui_forms::abi::detail::FieldControl> field = std::dynamic_pointer_cast<FieldControl>((*record).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1267` | `const std::shared_ptr<gui_forms::abi::detail::FieldControl> field = std::dynamic_pointer_cast<FieldControl>((*record).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1296` | `const std::shared_ptr<gui_forms::abi::detail::FieldControl> field = std::dynamic_pointer_cast<FieldControl>((*record).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1319` | `const std::shared_ptr<gui_forms::abi::detail::FieldControl> field = std::dynamic_pointer_cast<FieldControl>((*record).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1337` | `const std::shared_ptr<gui_forms::abi::detail::FieldControl> field = std::dynamic_pointer_cast<FieldControl>((*record).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1356` | `std::dynamic_pointer_cast<gui_forms::CheckBox>((*record).control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1362` | `std::dynamic_pointer_cast<gui_forms::RadioButton>((*record).control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1386` | `std::dynamic_pointer_cast<gui_forms::CheckBox>((*record).control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1391` | `std::dynamic_pointer_cast<gui_forms::RadioButton>((*record).control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1407` | `std::dynamic_pointer_cast<gui_forms::RangeControl>((*record).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1426` | `std::dynamic_pointer_cast<gui_forms::RangeControl>((*record).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1442` | `std::dynamic_pointer_cast<gui_forms::RangeControl>((*record).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:1461` | `std::dynamic_pointer_cast<gui_forms::RangeControl>((*record).control);` |
| `make_unique` | `src/abi/registry/registry.hpp:1718` | `std::unique_ptr<gui_forms::Window> model = std::make_unique<Window>((*record).control, client_size);` |
| `weak_lock` | `src/abi/registry/registry.hpp:2131` | `if (const Control::Ptr control = weak.lock()) {` |
| `weak_lock` | `src/abi/registry/registry.hpp:2151` | `if (const Control::Ptr control = weak.lock()) (*control).set_paint_plane(plane);` |
| `make_shared` | `src/abi/registry/registry.hpp:2174` | `std::shared_ptr<gui_forms::abi::detail::SubscriptionRecord> record = std::make_shared<SubscriptionRecord>();` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2223` | `!std::dynamic_pointer_cast<gui_forms::ButtonBase>((*sender).control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2233` | `std::dynamic_pointer_cast<gui_forms::RangeControl>((*sender).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2234` | `const std::shared_ptr<gui_forms::ScrollableControl> scrollable = std::dynamic_pointer_cast<` |
| `make_shared` | `src/abi/registry/registry.hpp:2250` | `std::shared_ptr<gui_forms::abi::detail::SubscriptionRecord> record = std::make_shared<SubscriptionRecord>();` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2258` | `std::shared_ptr<gui_forms::ButtonBase> button = std::dynamic_pointer_cast<gui_forms::ButtonBase>((*sender).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2299` | `const std::shared_ptr<gui_forms::abi::detail::RasterControl> raster = std::dynamic_pointer_cast<RasterControl>((*sender).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2300` | `const std::shared_ptr<gui_forms::abi::detail::FieldControl> field = std::dynamic_pointer_cast<FieldControl>((*sender).control);` |
| `make_shared` | `src/abi/registry/registry.hpp:2301` | `std::shared_ptr<gui_forms::abi::detail::SubscriptionRecord> record = std::make_shared<SubscriptionRecord>();` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2333` | `const std::shared_ptr<gui_forms::abi::detail::FieldControl> field = std::dynamic_pointer_cast<FieldControl>((*sender).control);` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2334` | `const std::shared_ptr<gui_forms::abi::detail::RasterControl> raster = std::dynamic_pointer_cast<RasterControl>((*sender).control);` |
| `make_shared` | `src/abi/registry/registry.hpp:2339` | `std::shared_ptr<gui_forms::abi::detail::SubscriptionRecord> record = std::make_shared<SubscriptionRecord>();` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2365` | `const std::shared_ptr<gui_forms::abi::detail::FormControl> form = std::dynamic_pointer_cast<FormControl>((*sender).control);` |
| `make_shared` | `src/abi/registry/registry.hpp:2370` | `std::shared_ptr<gui_forms::abi::detail::SubscriptionRecord> record = std::make_shared<SubscriptionRecord>();` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2393` | `const std::shared_ptr<gui_forms::abi::detail::FieldControl> field = std::dynamic_pointer_cast<FieldControl>((*sender).control);` |
| `make_shared` | `src/abi/registry/registry.hpp:2398` | `std::shared_ptr<gui_forms::abi::detail::SubscriptionRecord> record = std::make_shared<SubscriptionRecord>();` |
| `weak_lock` | `src/abi/registry/registry.hpp:2630` | `const std::shared_ptr<Control> retained = control.lock();` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2719` | `if (std::dynamic_pointer_cast<FormControl>(control)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:2998` | `std::dynamic_pointer_cast<gui_forms::ButtonBase>(root)) {` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:3015` | `if (std::dynamic_pointer_cast<RasterControl>(root) \|\|` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:3016` | `std::dynamic_pointer_cast<FieldControl>(root) \|\|` |
| `dynamic_pointer_cast` | `src/abi/registry/registry.hpp:3017` | `std::dynamic_pointer_cast<gui_forms::RangeControl>(root)) {` |
| `make_shared` | `src/abi/registry/registry.hpp:3032` | `std::string, std::shared_ptr<Control>>> result = std::make_shared<` |
| `make_shared` | `src/controls/basic/basic_control_rendering.cpp:12` | `static const std::shared_ptr<const gui_forms::PropertyEnumDescriptor> value = std::make_shared<const PropertyEnumDescriptor>(` |
| `dynamic_pointer_cast` | `src/controls/button_base/radio_button/radio_button.cpp:50` | `std::shared_ptr<gui_forms::RadioButton> peer = std::dynamic_pointer_cast<RadioButton>(sibling);` |
| `dynamic_pointer_cast` | `src/controls/button_base/radio_button/radio_button.cpp:74` | `std::shared_ptr<gui_forms::RadioButton> peer = std::dynamic_pointer_cast<RadioButton>(sibling);` |
| `weak_lock` | `src/controls/commands/command_binding/command_binding.cpp:13` | `if (const std::shared_ptr<Command> retained = command.lock()) {` |
| `weak_lock` | `src/controls/commands/command_binding/command_binding.cpp:24` | `const std::shared_ptr<ButtonBase> retained = button.lock();` |
| `shared_from_this` | `src/controls/easing_preview/easing_preview.cpp:257` | `shared_from_this(), interval, FrameClock::now() + interval);` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:180` | `std::dynamic_pointer_cast<Label>(window.find(id))) {` |
| `subscription_token` | `src/controls/gallery_controls.cpp:238` | `SubscriptionToken click_;` |
| `subscription_token` | `src/controls/gallery_controls.cpp:277` | `SubscriptionToken click_;` |
| `subscription_token` | `src/controls/gallery_controls.cpp:307` | `SubscriptionToken click_;` |
| `subscription_token` | `src/controls/gallery_controls.cpp:331` | `SubscriptionToken click_;` |
| `subscription_token` | `src/controls/gallery_controls.cpp:361` | `SubscriptionToken value_changed_;` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:440` | `if (std::shared_ptr<gui_forms::Label> status = std::dynamic_pointer_cast<Label>(child)) {` |
| `subscription_token` | `src/controls/gallery_controls.cpp:467` | `SubscriptionToken initialized_;` |
| `subscription_token` | `src/controls/gallery_controls.cpp:468` | `SubscriptionToken load_;` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:525` | `if (std::shared_ptr<gui_forms::ButtonBase> button = std::dynamic_pointer_cast<ButtonBase>(control)) {` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:527` | `} else if (std::shared_ptr<gui_forms::RangeControl> range = std::dynamic_pointer_cast<RangeControl>(control)) {` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:531` | `if (std::shared_ptr<gui_forms::CheckBox> check = std::dynamic_pointer_cast<CheckBox>(` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:535` | `if (std::shared_ptr<gui_forms::RadioButton> classic = std::dynamic_pointer_cast<RadioButton>(` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:539` | `if (std::shared_ptr<gui_forms::RadioButton> quiet_radio = std::dynamic_pointer_cast<RadioButton>(` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:544` | `if (std::shared_ptr<gui_forms::CheckBox> tri = std::dynamic_pointer_cast<CheckBox>(` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:548` | `if (std::shared_ptr<gui_forms::LinkLabel> link = std::dynamic_pointer_cast<LinkLabel>(` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:553` | `if (std::shared_ptr<gui_forms::TrackBar> slider = std::dynamic_pointer_cast<TrackBar>(` |
| `dynamic_pointer_cast` | `src/controls/gallery_controls.cpp:557` | `if (std::shared_ptr<gui_forms::ProgressBar> progress = std::dynamic_pointer_cast<ProgressBar>(` |
| `make_shared` | `src/controls/gallery_controls.cpp:974` | `std::shared_ptr<gui_forms::gallery::GalleryContext> context = std::make_shared<GalleryContext>();` |
| `dynamic_pointer_cast` | `src/controls/guidance/error_glyph/error_glyph.cpp:137` | `std::shared_ptr<gui_forms::ErrorGlyph> self = std::dynamic_pointer_cast<ErrorGlyph>(shared_from_this());` |
| `shared_from_this` | `src/controls/guidance/error_glyph/error_glyph.cpp:137` | `std::shared_ptr<gui_forms::ErrorGlyph> self = std::dynamic_pointer_cast<ErrorGlyph>(shared_from_this());` |
| `subscription_token` | `src/controls/guidance/error_provider/error_provider.cpp:32` | `SubscriptionToken bounds_subscription;` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:40` | `const std::shared_ptr<Control> retained = target.lock();` |
| `make_unique` | `src/controls/guidance/error_provider/error_provider.cpp:54` | `: window_lifetime_(window.lifetime_), tool_tip_(std::make_unique<ToolTip>(window)),` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:110` | `const std::shared_ptr<gui_forms::detail::WindowLifetime> lifetime = window_lifetime_.lock();` |
| `make_unique` | `src/controls/guidance/error_provider/error_provider.cpp:151` | `std::unique_ptr<gui_forms::ErrorProvider::Entry> entry = std::make_unique<Entry>();` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:193` | `if (const std::shared_ptr<gui_forms::Control> target = (*entry).target.lock(); target && (*target).is_alive()) {` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:206` | `const std::shared_ptr<gui_forms::Control> target = entry.target.lock();` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:317` | `if (data_source_.lock() == source) {` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:411` | `if (const std::shared_ptr<gui_forms::Control> target = weak.lock(); target && (*target).is_alive()) {` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:418` | `if (!event.binding \|\| (*event.binding).source() != data_source_.lock()) return;` |
| `weak_from_this` | `src/controls/guidance/error_provider/error_provider.cpp:421` | `const std::shared_ptr<gui_forms::Control> target = (*raw).weak_from_this().lock();` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:421` | `const std::shared_ptr<gui_forms::Control> target = (*raw).weak_from_this().lock();` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:434` | `const std::shared_ptr<gui_forms::BindingSource> source = data_source_.lock();` |
| `weak_from_this` | `src/controls/guidance/error_provider/error_provider.cpp:451` | `const std::shared_ptr<gui_forms::Control> target = raw ? (*raw).weak_from_this().lock() : nullptr;` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:451` | `const std::shared_ptr<gui_forms::Control> target = raw ? (*raw).weak_from_this().lock() : nullptr;` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:489` | `if (const std::shared_ptr<gui_forms::Control> target = weak.lock(); target && (*target).is_alive()) {` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:495` | `const std::shared_ptr<Control> target = (*error).second.first.lock();` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:511` | `const std::shared_ptr<gui_forms::Control> target = entry.target.lock();` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:566` | `const std::shared_ptr<gui_forms::Control> target = entry.target.lock();` |
| `make_unique` | `src/controls/guidance/error_provider/error_provider.cpp:590` | `entry.popup = std::make_unique<PopupToken>(std::move(popup));` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:628` | `const std::shared_ptr<gui_forms::Control> target = (*entry).target.lock();` |
| `weak_lock` | `src/controls/guidance/error_provider/error_provider.cpp:664` | `if (const std::shared_ptr<gui_forms::Control> target = (*entry).target.lock(); target && (*target).is_alive()) {` |
| `make_unique` | `src/controls/guidance/help_provider/help_provider.cpp:41` | `accelerator_ = std::make_unique<AcceleratorHolder>(window.register_accelerator(` |
| `weak_lock` | `src/controls/guidance/help_provider/help_provider.cpp:58` | `const std::shared_ptr<gui_forms::detail::WindowLifetime> lifetime = window_lifetime_.lock();` |
| `make_unique` | `src/controls/guidance/help_provider/help_provider.cpp:99` | `std::unique_ptr<gui_forms::HelpProvider::Entry> entry = std::make_unique<Entry>();` |
| `weak_lock` | `src/controls/guidance/help_provider/help_provider.cpp:112` | `const std::shared_ptr<gui_forms::Control> target = entry.target.lock();` |
| `weak_lock` | `src/controls/guidance/help_provider/help_provider.cpp:210` | `if (const std::shared_ptr<gui_forms::Control> target = (*entry).target.lock(); target && (*target).is_alive()) {` |
| `weak_lock` | `src/controls/guidance/help_provider/help_provider.cpp:273` | `if (const std::shared_ptr<gui_forms::Control> target = entry.target.lock(); target && (*target).is_alive()) {` |
| `weak_lock` | `src/controls/guidance/help_provider/help_provider.cpp:286` | `const std::shared_ptr<Control> target = (*pair.second).target.lock();` |
| `weak_lock` | `src/controls/guidance/help_provider/help_provider.cpp:303` | `if (const std::shared_ptr<gui_forms::Control> target = (*entry).target.lock(); target && (*target).is_alive()) {` |
| `weak_lock` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:24` | `const std::shared_ptr<NumericUpDown> retained = editor.lock();` |
| `subscription_token` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:46` | `SubscriptionToken operator()(` |
| `weak_lock` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:49` | `const std::shared_ptr<NumericUpDown> retained = editor.lock();` |
| `subscription_token` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:54` | `: SubscriptionToken{};` |
| `weak_lock` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:63` | `const std::shared_ptr<FlagsValueEditor> retained = editor.lock();` |
| `subscription_token` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:85` | `SubscriptionToken operator()(` |
| `weak_lock` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:88` | `const std::shared_ptr<FlagsValueEditor> retained = editor.lock();` |
| `subscription_token` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:93` | `: SubscriptionToken{};` |
| `weak_lock` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:102` | `const std::shared_ptr<ColorValueEditor> retained = editor.lock();` |
| `subscription_token` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:124` | `SubscriptionToken operator()(` |
| `weak_lock` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:127` | `const std::shared_ptr<ColorValueEditor> retained = editor.lock();` |
| `subscription_token` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:132` | `: SubscriptionToken{};` |
| `subscription_token` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:139` | `SubscriptionToken operator()(` |
| `weak_lock` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:142` | `const std::shared_ptr<ColorValueEditor> retained = editor.lock();` |
| `subscription_token` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:145` | `: SubscriptionToken{};` |
| `make_shared` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:163` | `const std::shared_ptr<bool> synchronizing = std::make_shared<bool>(false);` |
| `make_shared` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:196` | `const std::shared_ptr<bool> synchronizing = std::make_shared<bool>(false);` |
| `make_shared` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:219` | `const std::shared_ptr<bool> synchronizing = std::make_shared<bool>(false);` |
| `make_shared` | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:312` | `std::shared_ptr<gui_forms::PropertyEditorRegistry> result = std::make_shared<PropertyEditorRegistry>();` |
| `make_shared` | `src/controls/inspection/property_value_converter_registry/property_value_converter_registry.cpp:194` | `std::shared_ptr<gui_forms::PropertyValueConverterRegistry> result = std::make_shared<PropertyValueConverterRegistry>();` |
| `shared_from_this` | `src/controls/menu/context_menu/context_menu.cpp:288` | `if (window()) return (*window()).request_focus(shared_from_this());` |
| `subscription_token` | `src/controls/menu/context_menu/context_menu.cpp:657` | `SubscriptionToken popup_revocation;` |
| `make_unique` | `src/controls/menu/context_menu/context_menu.cpp:666` | `: stable_id_(std::move(stable_id)), impl_(std::make_unique<Impl>(*this)) {` |
| `make_unique` | `src/controls/menu/menu_strip/menu_strip.cpp:46` | `popup_(std::make_unique<ContextMenu>(` |
| `shared_from_this` | `src/controls/menu/menu_strip/menu_strip.cpp:138` | `(*popup_).show(shared_from_this(),` |
| `shared_from_this` | `src/controls/menu/menu_strip/menu_strip.cpp:409` | `if (window()) static_cast<void>((*window()).request_focus(shared_from_this()));` |
| `shared_from_this` | `src/controls/menu/menu_strip/menu_strip.cpp:445` | `if (window() && !(*window()).request_focus(shared_from_this())) {` |
| `weak_lock` | `src/controls/panel/anchored_popup_layer/anchored_popup_layer.cpp:33` | `if (anchor_.lock() == anchor) return;` |
| `weak_lock` | `src/controls/panel/anchored_popup_layer/anchored_popup_layer.cpp:94` | `const Control::Ptr anchor = anchor_.lock();` |
| `shared_from_this` | `src/controls/panel/color_value_editor/color_value_editor.cpp:26` | `std::static_pointer_cast<ColorValueEditor>(shared_from_this());` |
| `static_pointer_cast` | `src/controls/panel/color_value_editor/color_value_editor.cpp:26` | `std::static_pointer_cast<ColorValueEditor>(shared_from_this());` |
| `shared_from_this` | `src/controls/panel/combo_box/combo_box.cpp:314` | `PopupToken popup_token = (*window()).open_popup(shared_from_this(), layer);` |
| `shared_from_this` | `src/controls/panel/combo_box/combo_box.cpp:320` | `std::static_pointer_cast<ComboBox>(shared_from_this());` |
| `static_pointer_cast` | `src/controls/panel/combo_box/combo_box.cpp:320` | `std::static_pointer_cast<ComboBox>(shared_from_this());` |
| `weak_lock` | `src/controls/panel/correspondence_view/correspondence_view.cpp:23` | `const std::shared_ptr<CorrespondenceView> retained = view.lock();` |
| `static_pointer_cast` | `src/controls/panel/correspondence_view/correspondence_view.cpp:446` | `const std::shared_ptr<gui_forms::CorrespondenceView> self = std::static_pointer_cast<CorrespondenceView>(` |
| `shared_from_this` | `src/controls/panel/correspondence_view/correspondence_view.cpp:447` | `shared_from_this());` |
| `shared_from_this` | `src/controls/panel/correspondence_view/correspondence_view.cpp:704` | `static_cast<void>((*window()).request_focus(shared_from_this()));` |
| `shared_from_this` | `src/controls/panel/correspondence_view/correspondence_view.cpp:907` | `static_cast<void>((*window()).request_focus(shared_from_this()));` |
| `shared_from_this` | `src/controls/panel/correspondence_view/correspondence_view.cpp:924` | `if (window()) static_cast<void>((*window()).request_focus(shared_from_this()));` |
| `shared_from_this` | `src/controls/panel/date_time_picker/date_time_picker.cpp:292` | `const Control::Ptr owner = shared_from_this();` |
| `shared_from_this` | `src/controls/panel/date_time_picker/date_time_picker.cpp:319` | `std::static_pointer_cast<DateTimePicker>(shared_from_this());` |
| `static_pointer_cast` | `src/controls/panel/date_time_picker/date_time_picker.cpp:319` | `std::static_pointer_cast<DateTimePicker>(shared_from_this());` |
| `make_shared` | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:54` | `std::make_shared<const PropertyEnumDescriptor>(descriptor);` |
| `make_shared` | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:78` | `std::make_shared<const PropertyEnumDescriptor>(descriptor_);` |
| `shared_from_this` | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:150` | `PopupToken token = (*window()).open_popup(shared_from_this(), layer);` |
| `shared_from_this` | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:156` | `std::static_pointer_cast<FlagsValueEditor>(shared_from_this());` |
| `static_pointer_cast` | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:156` | `std::static_pointer_cast<FlagsValueEditor>(shared_from_this());` |
| `make_shared` | `src/controls/panel/flags_value_editor/flags_value_editor.cpp:255` | `std::make_shared<const PropertyEnumDescriptor>(descriptor_);` |
| `subscription_token` | `src/controls/panel/instrument_rack/instrument_rack.cpp:103` | `SubscriptionToken changed;` |
| `subscription_token` | `src/controls/panel/instrument_rack/instrument_rack.cpp:104` | `SubscriptionToken committed;` |
| `subscription_token` | `src/controls/panel/instrument_rack/instrument_rack.cpp:105` | `SubscriptionToken cancelled;` |
| `subscription_token` | `src/controls/panel/instrument_rack/instrument_rack.cpp:106` | `SubscriptionToken focused;` |
| `subscription_token` | `src/controls/panel/instrument_rack/instrument_rack.cpp:118` | `SubscriptionToken toggled;` |
| `subscription_token` | `src/controls/panel/instrument_rack/instrument_rack.cpp:119` | `SubscriptionToken remove_clicked;` |
| `subscription_token` | `src/controls/panel/instrument_rack/instrument_rack.cpp:120` | `SubscriptionToken enable_focused;` |
| `subscription_token` | `src/controls/panel/instrument_rack/instrument_rack.cpp:121` | `SubscriptionToken remove_focused;` |
| `subscription_token` | `src/controls/panel/instrument_rack/instrument_rack.cpp:254` | `SubscriptionToken& token) {` |
| `weak_lock` | `src/controls/panel/instrument_rack/instrument_rack.cpp:265` | `const std::shared_ptr<ComboBox> control = weak_control.lock();` |
| `weak_lock` | `src/controls/panel/instrument_rack/instrument_rack.cpp:349` | `const std::shared_ptr<TextBox> control = weak_control.lock();` |
| `dynamic_pointer_cast` | `src/controls/panel/instrument_rack/instrument_rack.cpp:420` | `if (const std::shared_ptr<gui_forms::ComboBox> choice = std::dynamic_pointer_cast<ComboBox>(state.editor)) {` |
| `dynamic_pointer_cast` | `src/controls/panel/instrument_rack/instrument_rack.cpp:425` | `} else if (const std::shared_ptr<gui_forms::TextBox> text = std::dynamic_pointer_cast<TextBox>(state.editor)) {` |
| `dynamic_pointer_cast` | `src/controls/panel/instrument_rack/instrument_rack.cpp:523` | `if (const std::shared_ptr<gui_forms::Panel> panel = std::dynamic_pointer_cast<Panel>(` |
| `dynamic_pointer_cast` | `src/controls/panel/instrument_rack/instrument_rack.cpp:555` | `if (const std::shared_ptr<gui_forms::ComboBox> choice = std::dynamic_pointer_cast<ComboBox>(` |
| `dynamic_pointer_cast` | `src/controls/panel/instrument_rack/instrument_rack.cpp:565` | `} else if (const std::shared_ptr<gui_forms::TextBox> text = std::dynamic_pointer_cast<TextBox>(` |
| `make_unique` | `src/controls/panel/instrument_rack/instrument_rack.cpp:692` | `: Panel(std::move(stable_id)), impl_(std::make_unique<Impl>(*this)) {` |
| `dynamic_pointer_cast` | `src/controls/panel/instrument_rack/instrument_rack.cpp:875` | `if (const std::shared_ptr<gui_forms::TextBox> text = std::dynamic_pointer_cast<TextBox>(` |
| `dynamic_pointer_cast` | `src/controls/panel/instrument_rack/instrument_rack.cpp:878` | `} else if (const std::shared_ptr<gui_forms::ComboBox> choice = std::dynamic_pointer_cast<ComboBox>(` |
| `shared_from_this` | `src/controls/panel/list_box/checked_list_box/checked_list_box.cpp:233` | `static_cast<void>((*window()).request_focus(shared_from_this()));` |
| `shared_from_this` | `src/controls/panel/list_box/list_box.cpp:368` | `if (window() != nullptr) static_cast<void>((*window()).request_focus(shared_from_this()));` |
| `shared_from_this` | `src/controls/panel/list_box/list_box.cpp:487` | `static_cast<void>((*window()).request_focus(shared_from_this()));` |
| `weak_lock` | `src/controls/panel/numeric_up_down/numeric_up_down.cpp:34` | `const std::shared_ptr<NumericUpDown> numeric = target.lock();` |
| `weak_lock` | `src/controls/panel/numeric_up_down/numeric_up_down.cpp:41` | `const std::shared_ptr<NumericUpDown> numeric = target.lock();` |
| `shared_from_this` | `src/controls/panel/numeric_up_down/numeric_up_down.cpp:53` | `std::static_pointer_cast<NumericUpDown>(shared_from_this());` |
| `static_pointer_cast` | `src/controls/panel/numeric_up_down/numeric_up_down.cpp:53` | `std::static_pointer_cast<NumericUpDown>(shared_from_this());` |
| `dynamic_pointer_cast` | `src/controls/panel/numeric_up_down/numeric_up_down.cpp:56` | `std::shared_ptr<gui_forms::SpinButtons> spin = std::dynamic_pointer_cast<SpinButtons>(spinner_);` |
| `shared_from_this` | `src/controls/panel/object_view/object_view.cpp:568` | `if (window()) static_cast<void>((*window()).request_focus(shared_from_this()));` |
| `shared_from_this` | `src/controls/panel/object_view/object_view.cpp:748` | `if (window()) static_cast<void>((*window()).request_focus(shared_from_this()));` |

Detailed action rows were capped at 400; 674 additional rows remain represented in the summary counts.

## Required next evidence before any lifecycle change

1. Build an AST-backed member/parameter/return graph from the exact compilation database, resolving aliases and templates.
2. Trace retained Control child ownership, parent/observer weakness, Window attachment, Component disposal, Event slot/token revocation, dispatcher work ownership, and ABI subscription records as separate semantic graphs.
3. Instrument construction, attachment, detachment, disposal, revocation, final destruction, and cross-thread work with stable identities and sequence numbers.
4. Identify and reproduce cycles, delayed release, premature release, or allocation pressure before proposing a replacement.
5. Compare any unique ownership, generation-handle pool, intrusive link, or stable-pool candidate against the current shared/weak lifecycle traces and failure behavior.
6. Treat C ABI generational handles and native/platform ownership as fixed boundaries unless separately approved.

## Current decision

The present rewrite preserves shared/weak retained ownership. This inventory is intentionally banked for a future lifecycle-change round and must not be used to smuggle ownership changes into syntax, Delegate/Event, sorting, or storage batches.
