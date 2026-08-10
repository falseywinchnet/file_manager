# GUI.Forms `std::function` audit

Status: **OBSERVED lexical inventory plus measured M4 allocation evidence; retained uses are classified, not banned**.

## Snapshot

Generated UTC: `2026-08-10T21:20:02+00:00`. Scope: first-party GUI.Forms C++/Objective-C++ outside third-party, experiments, and build products. Explicit spellings: **210**.

One spelling is not necessarily one callback object: nested host signatures contain several spellings, while aliases can create many runtime objects from one spelling. This file is a review map, not an allocation profiler.

## Role classification

| Role | Spellings | Rewrite decision |
|---|---:|---|
| Binding, inspection, and property registries | 31 | Retain: heterogeneous property/value registries intentionally own erased operations. |
| Commands and input routing | 8 | Retain unless a concrete member binding can use Delegate without changing result/lifetime semantics. |
| Dispatch, scheduling, and cancellation | 32 | Retain: ownership, cancellation, wake, thread transfer, and exception transport differ from Event. |
| Event compatibility/storage | 1 | Retain the legacy owning overload; direct member subscriptions use Delegate first. |
| First-party support and dogfooding | 48 | Retain where it exercises the owning API; support code does not redesign production. |
| Host, platform, and C ABI adapters | 66 | Retain: these callbacks cross explicit host/provider lifetime boundaries; C ABI remains function pointer plus context. |
| Other retained owning callbacks | 20 | Reviewed individually; no allocation-heavy production target was proven by spelling alone. |
| Rendering and live content | 4 | Retain: owning paint/wake work has distinct surface and cancellation lifetime. |

## Quantified decision

`CALLBACK_LAB_M4_RESULTS.csv` is the reproducible Release-build control on the M4 host. On that libc++ configuration:

- Delegate binding is 16 bytes and performs zero allocation.
- A small `std::function<void(int)>` binding fits its implementation's small buffer and performs zero allocation; the object is 32 bytes.
- A 200-byte named owning target performs one allocation at `std::function` binding.
- Event subscription performs two allocations for Delegate and three for the large owning target.
- Snapshot Event emission performs one allocation for Delegate and two for the large owning target because the owning callable is copied to preserve callback lifetime during revocation.

The invocation microtiming is retained as negative/diagnostic evidence, not a winner: an atomic observable dominates the tiny call path and the three generated call sites optimize differently. Delegate is selected for explicit non-ownership and zero binding allocation, not from a claimed nanosecond advantage.

The conspicuous large-target Event case is now documented, but no matching first-party production subscription was proven. Replacing Event snapshot/copy semantics speculatively would violate O-015. If a future profiler identifies such a target, reduce its captured state or open the measured slot-storage round.

## Complete spelling inventory

| Role | Location | Source line |
|---|---|---|
| First-party support and dogfooding | `file_manager_demoboard/src/product_window.cpp:101` | `std::function<bool(std::string)> review_surface_selector;` |
| First-party support and dogfooding | `file_manager_demoboard/src/product_window.cpp:1180` | `std::function<void()> location_feedback)` |
| First-party support and dogfooding | `file_manager_demoboard/src/product_window.cpp:1291` | `std::function<void()> location_feedback_;` |
| First-party support and dogfooding | `file_manager_demoboard/src/product_window.cpp:1299` | `std::function<void(KeyEvent&)> key_preview;` |
| First-party support and dogfooding | `file_manager_demoboard/src/product_window.cpp:1310` | `std::function<void(std::string)> status)` |
| First-party support and dogfooding | `file_manager_demoboard/src/product_window.cpp:1736` | `std::function<void(std::string)> status_;` |
| First-party support and dogfooding | `file_manager_demoboard/src/product_window.cpp:1762` | `std::function<void(std::string)> status,` |
| First-party support and dogfooding | `file_manager_demoboard/src/product_window.cpp:1763` | `std::function<void(SemanticFeedbackKind)> feedback)` |
| First-party support and dogfooding | `file_manager_demoboard/src/product_window.cpp:2079` | `std::function<void(std::string)> status_;` |
| First-party support and dogfooding | `file_manager_demoboard/src/product_window.cpp:2080` | `std::function<void(SemanticFeedbackKind)> feedback_;` |
| First-party support and dogfooding | `file_manager_demoboard/src/product_window.cpp:2108` | `std::function<void(std::string)> status,` |
| First-party support and dogfooding | `file_manager_demoboard/src/product_window.cpp:2109` | `std::function<void(SemanticFeedbackKind)> feedback)` |
| First-party support and dogfooding | `file_manager_demoboard/src/product_window.cpp:2569` | `std::function<void(std::string)> status_;` |
| First-party support and dogfooding | `file_manager_demoboard/src/product_window.cpp:2570` | `std::function<void(SemanticFeedbackKind)> feedback_;` |
| Binding, inspection, and property registries | `include/gui_forms/binding/value/binding_value.hpp:237` | `using Getter = std::function<BindingValue()>;` |
| Binding, inspection, and property registries | `include/gui_forms/binding/value/binding_value.hpp:238` | `using Setter = std::function<void(const BindingValue&)>;` |
| Binding, inspection, and property registries | `include/gui_forms/binding/value/binding_value.hpp:239` | `using ChangeConnector = std::function<SubscriptionToken(` |
| Binding, inspection, and property registries | `include/gui_forms/binding/value/binding_value.hpp:240` | `Component&, std::function<void()>)>;` |
| Binding, inspection, and property registries | `include/gui_forms/binding/value/binding_value.hpp:241` | `using Resetter = std::function<void()>;` |
| Binding, inspection, and property registries | `include/gui_forms/binding/value/binding_value.hpp:242` | `using ShouldSerialize = std::function<bool()>;` |
| Binding, inspection, and property registries | `include/gui_forms/binding/value/binding_value.hpp:243` | `using OriginProvider = std::function<PropertyValueOrigin()>;` |
| Commands and input routing | `include/gui_forms/components/context_menu/context_menu.hpp:95` | `void set_root_navigation_handler(std::function<bool(int)> handler);` |
| Commands and input routing | `include/gui_forms/components/context_menu/context_menu.hpp:97` | `std::function<bool(const PointerEvent&)> handler);` |
| Dispatch, scheduling, and cancellation | `include/gui_forms/control/control/control.hpp:412` | `[[nodiscard]] DispatchOperation begin_invoke(std::function<void()> callback);` |
| Other retained owning callbacks | `include/gui_forms/control/control/control.hpp:416` | `void invoke(std::function<void()> callback);` |
| Other retained owning callbacks | `include/gui_forms/control/control/control.hpp:551` | `std::string_view name, Component& owner, std::function<void()> changed);` |
| Other retained owning callbacks | `include/gui_forms/control/control/control.hpp:689` | `std::function<void()> publication);` |
| Other retained owning callbacks | `include/gui_forms/control/control/control.hpp:713` | `std::function<void()> publication;` |
| Other retained owning callbacks | `include/gui_forms/control/control/control.hpp:976` | `std::function<void()> changed;` |
| Other retained owning callbacks | `include/gui_forms/control/control/control.hpp:989` | `std::function<void()> changed) const {` |
| Other retained owning callbacks | `include/gui_forms/control/static_tree/control_factory/control_factory.hpp:23` | `using Creator = std::function<Control::Ptr(StableId)>;` |
| Rendering and live content | `include/gui_forms/controls/drawing_surface/drawing_surface.hpp:13` | `using PaintCallback = std::function<void(Painter&, Rect, Rect)>;` |
| Other retained owning callbacks | `include/gui_forms/detail/bound_member_function.hpp:7` | `// Explicit adapter for APIs that deliberately retain std::function but bind a` |
| Binding, inspection, and property registries | `include/gui_forms/detail/property_binding_adapters.hpp:15` | `// already-authored decision for std::function-based registration seams.` |
| Binding, inspection, and property registries | `include/gui_forms/detail/property_binding_adapters.hpp:110` | `explicit PropertyChangeRelay(std::function<void()> changed)` |
| Binding, inspection, and property registries | `include/gui_forms/detail/property_binding_adapters.hpp:116` | `std::function<void()> changed_;` |
| Binding, inspection, and property registries | `include/gui_forms/detail/property_binding_adapters.hpp:126` | `std::function<void()> changed) const {` |
| Event compatibility/storage | `include/gui_forms/event/event/event.hpp:62` | `using Callback = std::function<void(Arguments...)>;` |
| Other retained owning callbacks | `include/gui_forms/feedback/semantic_feedback/semantic_feedback.hpp:20` | `using Clock = std::function<std::uint64_t()>;` |
| Binding, inspection, and property registries | `include/gui_forms/inspection/property_editor_registry/property_editor_registry.hpp:16` | `std::function<void(const BindingValue&)> synchronize;` |
| Binding, inspection, and property registries | `include/gui_forms/inspection/property_editor_registry/property_editor_registry.hpp:17` | `std::function<SubscriptionToken(` |
| Binding, inspection, and property registries | `include/gui_forms/inspection/property_editor_registry/property_editor_registry.hpp:18` | `Component&, std::function<void(BindingValue)>)> connect_committed;` |
| Binding, inspection, and property registries | `include/gui_forms/inspection/property_editor_registry/property_editor_registry.hpp:19` | `std::function<SubscriptionToken(` |
| Binding, inspection, and property registries | `include/gui_forms/inspection/property_editor_registry/property_editor_registry.hpp:20` | `Component&, std::function<void(const PropertyEditorInputError&)>)>` |
| Binding, inspection, and property registries | `include/gui_forms/inspection/property_editor_registry/property_editor_registry.hpp:24` | `using PropertyEditorFactory = std::function<std::optional<PropertyEditorBinding>(` |
| Binding, inspection, and property registries | `include/gui_forms/inspection/property_value_converter_registry/property_value_converter_registry.hpp:33` | `using Formatter = std::function<std::string(` |
| Binding, inspection, and property registries | `include/gui_forms/inspection/property_value_converter_registry/property_value_converter_registry.hpp:35` | `using Parser = std::function<std::optional<BindingValue>(` |
| Binding, inspection, and property registries | `include/gui_forms/inspection/property_value_converter_registry/property_value_converter_registry.hpp:37` | `using ContextFormatter = std::function<std::string(` |
| Binding, inspection, and property registries | `include/gui_forms/inspection/property_value_converter_registry/property_value_converter_registry.hpp:40` | `using ContextParser = std::function<std::optional<BindingValue>(` |
| Rendering and live content | `include/gui_forms/live_surface/surface/live_surface.hpp:35` | `std::function<void()> wake);` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/macos_host.hpp:22` | `std::function<void(HostCloseRequest&)> close_request;` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/macos_host.hpp:23` | `std::function<void(std::function<void()> wake,` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/macos_host.hpp:23` | `std::function<void(std::function<void()> wake,` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/macos_host.hpp:24` | `std::function<void()> request_close,` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/macos_host.hpp:25` | `std::function<HostDialogResult(const HostDialogRequest&)> show_dialog,` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/macos_host.hpp:26` | `std::function<HostServiceStatus(const HostTooltipRequest&)> show_tooltip,` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/macos_host.hpp:27` | `std::function<void()> hide_tooltip,` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/macos_host.hpp:28` | `std::function<HostClipboardTextResult()> read_clipboard_text,` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/macos_host.hpp:29` | `std::function<HostServiceStatus(std::string_view)> write_clipboard_text)> host_ready;` |
| Dispatch, scheduling, and cancellation | `include/gui_forms/platform/macos_host.hpp:30` | `std::function<void()> dispatch_pending;` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/macos_host.hpp:31` | `std::function<void()> closed;` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/macos_host.hpp:32` | `std::function<void(std::string_view metrics_json,` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/windows_host.hpp:28` | `std::function<std::shared_ptr<Control>(std::string_view)> automation_resolve;` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/windows_host.hpp:29` | `std::function<void(HostCloseRequest&)> close_request;` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/windows_host.hpp:30` | `std::function<void(std::function<void()> wake,` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/windows_host.hpp:30` | `std::function<void(std::function<void()> wake,` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/windows_host.hpp:31` | `std::function<void()> request_close,` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/windows_host.hpp:32` | `std::function<HostDialogResult(const HostDialogRequest&)> show_dialog,` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/windows_host.hpp:33` | `std::function<HostServiceStatus(const HostTooltipRequest&)> show_tooltip,` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/windows_host.hpp:34` | `std::function<void()> hide_tooltip,` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/windows_host.hpp:35` | `std::function<HostClipboardTextResult()> read_clipboard_text,` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/windows_host.hpp:36` | `std::function<HostServiceStatus(std::string_view)> write_clipboard_text)> host_ready;` |
| Dispatch, scheduling, and cancellation | `include/gui_forms/platform/windows_host.hpp:37` | `std::function<void()> dispatch_pending;` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/windows_host.hpp:38` | `std::function<void()> closed;` |
| Host, platform, and C ABI adapters | `include/gui_forms/platform/windows_host.hpp:39` | `std::function<void(std::string_view metrics_json,` |
| Dispatch, scheduling, and cancellation | `include/gui_forms/window/window.hpp:313` | `std::function<void(FrameTime)> callback);` |
| Dispatch, scheduling, and cancellation | `include/gui_forms/window/window.hpp:321` | `[[nodiscard]] DispatchOperation begin_invoke(std::function<void()> callback);` |
| Other retained owning callbacks | `include/gui_forms/window/window.hpp:323` | `const Control::Ptr& owner, std::function<void()> callback);` |
| Other retained owning callbacks | `include/gui_forms/window/window.hpp:326` | `void invoke(std::function<void()> callback);` |
| Other retained owning callbacks | `include/gui_forms/window/window.hpp:327` | `void invoke(const Control::Ptr& owner, std::function<void()> callback);` |
| Dispatch, scheduling, and cancellation | `include/gui_forms/window/window.hpp:333` | `void set_dispatch_wake_handler(std::function<void()> wake);` |
| Other retained owning callbacks | `include/gui_forms/window/window.hpp:339` | `void set_paint_wake_handler(std::function<void()> wake);` |
| Other retained owning callbacks | `include/gui_forms/window/window.hpp:432` | `Component& owner, KeyGesture gesture, std::function<bool()> callback,` |
| Other retained owning callbacks | `include/gui_forms/window/window.hpp:781` | `std::function<void()> paint_wake_handler_;` |
| Host, platform, and C ABI adapters | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:26` | `gui_forms::Component& owner, std::function<void()> changed) const {` |
| Host, platform, and C ABI adapters | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:123` | `std::function<void(gui_forms::BindingValue)> committed;` |
| Host, platform, and C ABI adapters | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:148` | `std::function<void(gui_forms::BindingValue)> committed) const {` |
| Host, platform, and C ABI adapters | `src/abi/control_adapters/abi_property_object_control/abi_property_object_control.hpp:163` | `std::function<void(` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:56` | `std::function<void()> host_wake;` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:57` | `std::function<void()> host_close;` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:58` | `std::function<gui_forms::HostDialogResult(` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:60` | `std::function<gui_forms::HostServiceStatus(` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:62` | `std::function<void()> host_tooltip_hide;` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:63` | `std::function<gui_forms::HostClipboardTextResult()> host_clipboard_read;` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:64` | `std::function<gui_forms::HostServiceStatus(std::string_view)> host_clipboard_write;` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:1181` | `std::function<gui_forms::HostServiceStatus(std::string_view)> provider;` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:1217` | `std::function<gui_forms::HostClipboardTextResult()> provider;` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:1530` | `std::function<gui_forms::HostDialogResult(` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:1641` | `std::function<gui_forms::HostServiceStatus(` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:1668` | `std::function<void()> hide;` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:2416` | `std::function<void()> wake;` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:2450` | `std::function<void()> request;` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:2536` | `std::function<void()> wake,` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:2537` | `std::function<void()> close,` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:2538` | `std::function<gui_forms::HostDialogResult(` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:2540` | `std::function<gui_forms::HostServiceStatus(` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:2542` | `std::function<void()> tooltip_hide,` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:2543` | `std::function<gui_forms::HostClipboardTextResult()> clipboard_read,` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:2544` | `std::function<gui_forms::HostServiceStatus(` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:2746` | `std::function<void()> wake;` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:2763` | `std::function<void()> wake;` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:2863` | `std::function<void()> wake,` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:2864` | `std::function<void()> request_close,` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:2865` | `std::function<gui_forms::HostDialogResult(` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:2867` | `std::function<gui_forms::HostServiceStatus(` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:2869` | `std::function<void()> tooltip_hide = {},` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:2870` | `std::function<gui_forms::HostClipboardTextResult()>` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:2872` | `std::function<gui_forms::HostServiceStatus(std::string_view)>` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:2876` | `std::function<void()> published_wake;` |
| Host, platform, and C ABI adapters | `src/abi/registry/registry.hpp:2877` | `std::function<void()> published_close;` |
| Binding, inspection, and property registries | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:35` | `std::function<void(BindingValue)> committed;` |
| Binding, inspection, and property registries | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:48` | `std::function<void(BindingValue)> committed) const {` |
| Binding, inspection, and property registries | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:74` | `std::function<void(BindingValue)> committed;` |
| Binding, inspection, and property registries | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:87` | `std::function<void(BindingValue)> committed) const {` |
| Binding, inspection, and property registries | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:113` | `std::function<void(BindingValue)> committed;` |
| Binding, inspection, and property registries | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:126` | `std::function<void(BindingValue)> committed) const {` |
| Binding, inspection, and property registries | `src/controls/inspection/property_editor_registry/property_editor_registry.cpp:141` | `std::function<void(const PropertyEditorInputError&)> failed) const {` |
| Commands and input routing | `src/controls/menu/context_menu/context_menu.cpp:661` | `std::function<bool(int)> root_navigation_handler;` |
| Commands and input routing | `src/controls/menu/context_menu/context_menu.cpp:662` | `std::function<bool(const PointerEvent&)> outside_pointer_handler;` |
| Commands and input routing | `src/controls/menu/context_menu/context_menu.cpp:716` | `std::function<bool(int)> handler) {` |
| Commands and input routing | `src/controls/menu/context_menu/context_menu.cpp:721` | `std::function<bool(const PointerEvent&)> handler) {` |
| Other retained owning callbacks | `src/controls/panel/instrument_rack/rack_module_panel/rack_module_panel.hpp:17` | `std::function<void(bool)> move_request;` |
| Other retained owning callbacks | `src/core/control/control/control.cpp:760` | `std::string_view name, Component& owner, std::function<void()> changed) {` |
| Other retained owning callbacks | `src/core/control/control/control.cpp:1861` | `std::function<void()> publication) {` |
| Dispatch, scheduling, and cancellation | `src/core/control/dispatcher/control_dispatcher.cpp:34` | `DispatchOperation Control::begin_invoke(std::function<void()> callback) {` |
| Dispatch, scheduling, and cancellation | `src/core/control/dispatcher/control_dispatcher.cpp:44` | `void Control::invoke(std::function<void()> callback) {` |
| Dispatch, scheduling, and cancellation | `src/core/dispatcher/state/dispatcher_state.hpp:20` | `std::function<void()> callback;` |
| Dispatch, scheduling, and cancellation | `src/core/dispatcher/state/dispatcher_state.hpp:38` | `std::function<void()> wake;` |
| Dispatch, scheduling, and cancellation | `src/core/dispatcher/state/dispatcher_state.hpp:59` | `std::function<void()> callback,` |
| Dispatch, scheduling, and cancellation | `src/core/dispatcher/state/post_dispatch.cpp:16` | `std::function<void()> callback,` |
| Dispatch, scheduling, and cancellation | `src/core/dispatcher/state/post_dispatch.cpp:26` | `std::function<void()> wake;` |
| Rendering and live content | `src/core/live_surface/state/live_surface_state.hpp:22` | `std::function<void()> callback;` |
| Rendering and live content | `src/core/live_surface/surface/live_surface.cpp:115` | `std::function<void()> wake) {` |
| Dispatch, scheduling, and cancellation | `src/core/scheduler/request/scheduled_frame_request.cpp:15` | `std::function<void(FrameTime)> request_callback) noexcept` |
| Dispatch, scheduling, and cancellation | `src/core/scheduler/request/scheduled_frame_request.hpp:24` | `std::function<void(FrameTime)> request_callback) noexcept;` |
| Dispatch, scheduling, and cancellation | `src/core/scheduler/request/scheduled_frame_request.hpp:33` | `std::function<void(FrameTime)> callback;` |
| Commands and input routing | `src/core/window/accelerator/accelerator_attachment.hpp:10` | `std::function<bool()> callback,` |
| Commands and input routing | `src/core/window/accelerator/accelerator_attachment.hpp:34` | `std::function<bool()> callback_;` |
| Dispatch, scheduling, and cancellation | `src/core/window/dispatcher/window_dispatcher.cpp:25` | `DispatchOperation Window::begin_invoke(std::function<void()> callback) {` |
| Dispatch, scheduling, and cancellation | `src/core/window/dispatcher/window_dispatcher.cpp:31` | `std::function<void()> callback) {` |
| Dispatch, scheduling, and cancellation | `src/core/window/dispatcher/window_dispatcher.cpp:40` | `void Window::invoke(std::function<void()> callback) {` |
| Dispatch, scheduling, and cancellation | `src/core/window/dispatcher/window_dispatcher.cpp:55` | `std::function<void()> callback) {` |
| Dispatch, scheduling, and cancellation | `src/core/window/dispatcher/window_dispatcher.cpp:122` | `std::function<void()> callback = std::move((*work).callback);` |
| Dispatch, scheduling, and cancellation | `src/core/window/dispatcher/window_dispatcher.cpp:142` | `std::function<void()> wake;` |
| Dispatch, scheduling, and cancellation | `src/core/window/dispatcher/window_dispatcher.cpp:198` | `void Window::set_dispatch_wake_handler(std::function<void()> wake_handler) {` |
| Dispatch, scheduling, and cancellation | `src/core/window/dispatcher/window_dispatcher.cpp:200` | `std::function<void()> wake;` |
| Other retained owning callbacks | `src/core/window/presentation/window_presentation.cpp:449` | `void Window::set_paint_wake_handler(std::function<void()> wake) {` |
| Dispatch, scheduling, and cancellation | `src/core/window/scheduler/window_scheduler.cpp:87` | `std::function<void(FrameTime)> callback) {` |
| Dispatch, scheduling, and cancellation | `src/core/window/scheduler/window_scheduler.cpp:148` | `const std::function<void(FrameTime)> callback = (*request).callback;` |
| Other retained owning callbacks | `src/core/window/window.cpp:321` | `Component& owner, KeyGesture gesture, std::function<bool()> callback,` |
| Host, platform, and C ABI adapters | `src/host/headless/services/headless_host_services.hpp:14` | `using DialogHandler = std::function<HostDialogResult(` |
| Host, platform, and C ABI adapters | `src/host/macos/application/macos_host.mm:461` | `- (void)installCloseRequestHandler:(std::function<void(HostCloseRequest&)>)handler;` |
| Host, platform, and C ABI adapters | `src/host/macos/application/macos_host.mm:873` | `- (void)installCloseRequestHandler:(std::function<void(HostCloseRequest&)>)handler {` |
| Host, platform, and C ABI adapters | `src/host/macos/application/macos_host.mm:1709` | `std::function<void()> _closedHandler;` |
| Host, platform, and C ABI adapters | `src/host/macos/application/macos_host.mm:1713` | `closedHandler:(std::function<void()>)closedHandler;` |
| Host, platform, and C ABI adapters | `src/host/macos/application/macos_host.mm:1719` | `closedHandler:(std::function<void()>)closedHandler {` |
| Host, platform, and C ABI adapters | `src/host/macos/application/macos_host.mm:1734` | `std::function<void()> callback = std::move(_closedHandler);` |
| Dispatch, scheduling, and cancellation | `src/host/macos/application/macos_host.mm:1806` | `std::function<void()> dispatch_pending) noexcept` |
| Dispatch, scheduling, and cancellation | `src/host/macos/application/macos_host.mm:1812` | `const std::function<void()> dispatch_pending = dispatch_pending_;` |
| Dispatch, scheduling, and cancellation | `src/host/macos/application/macos_host.mm:1821` | `std::function<void()> dispatch_pending_;` |
| Dispatch, scheduling, and cancellation | `src/host/macos/application/macos_host.mm:1960` | `const std::function<void()> dispatchPending = options.dispatch_pending;` |
| Dispatch, scheduling, and cancellation | `src/host/macos/application/macos_host.mm:2118` | `const std::function<void()> dispatchPending =` |
| First-party support and dogfooding | `tests/core_tests.cpp:101` | `std::function<void()> release_callback;` |
| First-party support and dogfooding | `tests/core_tests.cpp:138` | `std::function<void()> measure_callback;` |
| First-party support and dogfooding | `tests/core_tests.cpp:139` | `std::function<void()> arrange_callback;` |
| First-party support and dogfooding | `tests/core_tests.cpp:176` | `std::function<void()> paint_callback;` |
| First-party support and dogfooding | `tests/core_tests.cpp:177` | `std::function<void()> paint_overlay_callback;` |
| First-party support and dogfooding | `tests/core_tests.cpp:178` | `mutable std::function<void()> hit_test_callback;` |
| First-party support and dogfooding | `tests/core_tests.cpp:179` | `mutable std::function<void()> semantic_callback;` |
| First-party support and dogfooding | `tests/delegate_event_tests.cpp:193` | `"the legacy std::function subscription overload must remain compatible");` |
| Dispatch, scheduling, and cancellation | `tests/frame_scheduler_tests.cpp:59` | `std::function<void(FrameTime)> callback;` |
| First-party support and dogfooding | `tests/host_protocol_tests.cpp:112` | `std::function<HostDispatchResult()> inject;` |
| Binding, inspection, and property registries | `tests/inspection_controls_tests.cpp:414` | `std::function<void(BindingValue)> committed) noexcept` |
| Binding, inspection, and property registries | `tests/inspection_controls_tests.cpp:424` | `std::function<void(BindingValue)> committed_;` |
| Binding, inspection, and property registries | `tests/inspection_controls_tests.cpp:436` | `std::function<void(BindingValue)> committed) const {` |
| First-party support and dogfooding | `tests/invalidation_damage_tests.cpp:54` | `std::function<void()> on_first_command;` |
| First-party support and dogfooding | `tests/layout_panel_tests.cpp:34` | `std::function<void()> callback;` |
| First-party support and dogfooding | `tests/macos_host_close_tests.mm:34` | `std::function<void()> wake;` |
| First-party support and dogfooding | `tests/macos_host_close_tests.mm:35` | `std::function<void()> request_close;` |
| First-party support and dogfooding | `tests/macos_host_close_tests.mm:56` | `using VoidHostCallback = std::function<void()>;` |
| First-party support and dogfooding | `tests/macos_host_close_tests.mm:57` | `using ShowDialogCallback = std::function<gui_forms::HostDialogResult(` |
| First-party support and dogfooding | `tests/macos_host_close_tests.mm:59` | `using ShowTooltipCallback = std::function<gui_forms::HostServiceStatus(` |
| First-party support and dogfooding | `tests/macos_host_close_tests.mm:62` | `std::function<gui_forms::HostClipboardTextResult()>;` |
| First-party support and dogfooding | `tests/macos_host_close_tests.mm:64` | `std::function<gui_forms::HostServiceStatus(std::string_view)>;` |
| First-party support and dogfooding | `tests/macos_multi_window_tests.mm:14` | `using VoidHostCallback = std::function<void()>;` |
| First-party support and dogfooding | `tests/macos_multi_window_tests.mm:15` | `using ShowDialogCallback = std::function<gui_forms::HostDialogResult(` |
| First-party support and dogfooding | `tests/macos_multi_window_tests.mm:17` | `using ShowTooltipCallback = std::function<gui_forms::HostServiceStatus(` |
| First-party support and dogfooding | `tests/macos_multi_window_tests.mm:20` | `std::function<gui_forms::HostClipboardTextResult()>;` |
| First-party support and dogfooding | `tests/macos_multi_window_tests.mm:22` | `std::function<gui_forms::HostServiceStatus(std::string_view)>;` |
| First-party support and dogfooding | `tests/macos_multi_window_tests.mm:44` | `explicit CaptureCloseRequest(std::function<void()>& close_request) noexcept` |
| First-party support and dogfooding | `tests/macos_multi_window_tests.mm:59` | `std::function<void()>& close_request_;` |
| First-party support and dogfooding | `tests/macos_multi_window_tests.mm:78` | `const std::function<void()>& close_product) noexcept` |
| First-party support and dogfooding | `tests/macos_multi_window_tests.mm:89` | `const std::function<void()> primary_close = close_product_;` |
| First-party support and dogfooding | `tests/macos_multi_window_tests.mm:98` | `const std::function<void()>& close_product_;` |
| First-party support and dogfooding | `tests/macos_multi_window_tests.mm:112` | `std::function<void()> close_product;` |
| First-party support and dogfooding | `tests/retained_lifetime_tests.cpp:70` | `std::function<void()> pointer_action;` |
| First-party support and dogfooding | `tests/support/named_callbacks.hpp:132` | `explicit IgnoreArgumentsThenInvoke(std::function<void()> callback) noexcept` |
| First-party support and dogfooding | `tests/support/named_callbacks.hpp:138` | `std::function<void()> callback_;` |
| First-party support and dogfooding | `tools/callback_lab.cpp:135` | `std::function<void(int)> small_callback = SmallOwningCallback(small_target);` |
| First-party support and dogfooding | `tools/callback_lab.cpp:140` | `std::function<void(int)> large_callback = LargeOwningCallback(large_target);` |
