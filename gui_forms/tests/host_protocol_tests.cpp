#include "gui_forms/gui_forms.hpp"
#include "headless_host.hpp"

#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

class InputProbe final : public Control {
public:
    explicit InputProbe(StableId id) : Control(std::move(id)) {
        set_focusable(true);
        set_allow_drop(true);
    }

    void on_pointer(PointerEvent& event) override {
        ++pointer_events;
        event.handled = true;
    }

    void on_key(KeyEvent& event) override {
        ++key_events;
        event.handled = true;
    }

    void on_text_input(TextInputEvent& event) override {
        ++text_events;
        last_text = event.text_utf8;
        event.handled = true;
    }

    void on_drag(DragEvent& event) override {
        drag_actions.push_back(event.action);
        if (!event.items.empty()) {
            last_drag_items = event.items;
        }
        event.accepted_effect = drag_effect;
        event.handled = true;
    }

    std::uint64_t pointer_events{};
    std::uint64_t key_events{};
    std::uint64_t text_events{};
    std::string last_text;
    std::vector<DragAction> drag_actions;
    std::vector<DragDataItem> last_drag_items;
    DragEffect drag_effect{DragEffect::copy};
};

struct Fixture final {
    Fixture()
        : probe(make_control<InputProbe>(StableId("host.probe"))),
          window(probe, {100.0, 80.0}),
          host(window) {
        probe->set_requested_bounds({0.0, 0.0, 100.0, 80.0});
    }

    std::shared_ptr<InputProbe> probe;
    Window window;
    host::HeadlessHost host;
};

HostMonitor reference_monitor() {
    return {"headless.primary",
            {0.0, 0.0, 1920.0, 1080.0},
            {0.0, 0.0, 1920.0, 1040.0},
            1.0,
            true};
}

HostDialogRequest message_request(std::uint64_t request_id,
                                  HostMessageButtons buttons = HostMessageButtons::ok_cancel) {
    HostMessageDialogRequest message;
    message.title = "Portable dialog";
    message.message = "Renderer-free modal contract Ω";
    message.buttons = buttons;
    message.icon = HostMessageIcon::information;
    message.default_choice = buttons == HostMessageButtons::yes_no
        ? HostDialogChoice::yes : HostDialogChoice::ok;
    return {request_id, "gallery.window", std::move(message)};
}

std::string canonical_host_trace() {
    Fixture fixture;
    std::uint64_t time = 1'000;
    require(fixture.host.dispatch(HostAttachEvent{{320.0, 180.0}, 2.0}, time++).accepted(),
            "headless attach must be accepted");
    static_cast<void>(fixture.host.dispatch(
        HostDisplayEvent{{reference_monitor()}}, time++));
    static_cast<void>(fixture.host.dispatch(HostActivationEvent{true}, time++));
    static_cast<void>(fixture.host.dispatch(HostOcclusionEvent{true}, time++));
    static_cast<void>(fixture.host.dispatch(HostOcclusionEvent{false}, time++));
    PointerEvent pointer;
    pointer.action = PointerAction::down;
    pointer.button = PointerButton::primary;
    pointer.position = {20.0, 20.0};
    pointer.pointer_id = 1;
    static_cast<void>(fixture.host.dispatch(pointer, time++));
    KeyEvent key;
    key.action = KeyAction::down;
    key.physical_key = 36;
    static_cast<void>(fixture.host.dispatch(key, time++));
    TextInputEvent text;
    text.text_utf8 = "host-text";
    static_cast<void>(fixture.host.dispatch(std::move(text), time++));
    static_cast<void>(fixture.host.dispatch(HostCloseRequest{HostCloseReason::test, false},
                                            time++));
    static_cast<void>(fixture.host.dispatch(HostClosedEvent{HostCloseReason::test}, time++));
    static_cast<void>(fixture.host.dispatch(HostShutdownEvent{}, time++));
    return fixture.host.trace();
}

std::string canonical_service_trace() {
    Fixture fixture;
    HostServices& services = fixture.host.services();
    const HostMonitorResult monitors = services.query_monitors();
    std::string trace = "service=monitors error=";
    trace += host_service_error_name(monitors.status.error);
    trace += " count=" + std::to_string(monitors.monitors.size());
    if (!monitors.monitors.empty()) {
        trace += " primary=" + monitors.monitors.front().id;
    }
    trace += '\n';
    const HostServiceStatus cursor = services.set_cursor(CursorKind::text);
    trace += "service=cursor error=";
    trace += host_service_error_name(cursor.error);
    trace += " value=";
    trace += cursor_kind_name(services.snapshot().cursor);
    trace += '\n';
    const HostServiceStatus capture = services.set_pointer_capture(true, 7);
    trace += "service=capture error=";
    trace += host_service_error_name(capture.error);
    trace += " pointer=7\n";
    const HostServiceStatus release = services.set_pointer_capture(false);
    trace += "service=release error=";
    trace += host_service_error_name(release.error);
    trace += '\n';
    const HostServiceStatus write = services.write_clipboard_text("service Ω");
    trace += "service=clipboard_write error=";
    trace += host_service_error_name(write.error);
    trace += '\n';
    const HostClipboardTextResult read = services.read_clipboard_text();
    trace += "service=clipboard_read error=";
    trace += host_service_error_name(read.status.error);
    trace += " generation=" + std::to_string(read.generation);
    trace += " text=" + read.text_utf8 + '\n';
    auto& headless = static_cast<host::HeadlessHostServices&>(services);
    headless.queue_dialog_result(
        {{}, 40, HostMessageDialogResult{HostDialogOutcome::cancelled,
                                         HostDialogChoice::cancel}});
    const HostDialogResult dialog = services.show_dialog(message_request(40));
    trace += headless.dialog_trace();
    trace += "service=dialog error=";
    trace += host_service_error_name(dialog.status.error);
    trace += '\n';
    trace += "snapshot=" + services.snapshot().to_json() + '\n';
    return trace;
}

std::string canonical_modal_trace() {
    Fixture fixture;
    auto& services = static_cast<host::HeadlessHostServices&>(
        fixture.host.services());
    std::string trace;
    auto observation = services.modal_changed().subscribe(
        [&trace](const HostModalTransition& transition) {
            trace += transition.entering ? "modal=enter" : "modal=leave";
            trace += " request=" + std::to_string(transition.request_id);
            trace += " depth=" + std::to_string(transition.depth) + '\n';
        });
    services.set_dialog_handler(
        [](const HostDialogRequest& request, host::HeadlessHostServices& adapter) {
            if (request.request_id == 70) {
                static_cast<void>(adapter.show_dialog(message_request(71)));
                return HostDialogResult{
                    {}, request.request_id,
                    HostMessageDialogResult{HostDialogOutcome::cancelled,
                                            HostDialogChoice::cancel}};
            }
            return HostDialogResult{
                {}, request.request_id,
                HostMessageDialogResult{HostDialogOutcome::accepted,
                                        HostDialogChoice::ok}};
        });
    static_cast<void>(services.show_dialog(message_request(70)));
    trace += services.dialog_trace();
    trace += "services=" + services.snapshot().to_json() + '\n';
    trace += "session=" + fixture.host.session().snapshot().to_json() + '\n';
    return trace;
}

void test_capabilities_and_normalized_dispatch() {
    Fixture fixture;
    const HostCapabilities capabilities = host::headless_capabilities();
    require(capabilities.supports(HostCapability::lifecycle |
                                      HostCapability::scale_notifications |
                                      HostCapability::pointer_input |
                                      HostCapability::keyboard_input |
                                      HostCapability::text_composition |
                                      HostCapability::monitor_geometry |
                                      HostCapability::pointer_capture |
                                      HostCapability::cursor |
                                      HostCapability::clipboard |
                                      HostCapability::dialogs),
            "headless host must report every implemented normalized input capability");
    require(capabilities.to_json().find("\"scale_notifications\"") !=
                std::string::npos,
            "capability report must expose readable names, not only a bit field");

    require(fixture.host.dispatch(HostAttachEvent{{640.0, 360.0}, 2.0}, 1).accepted(),
            "valid attach event must be accepted");
    require(fixture.window.client_size() == Size{640.0, 360.0} &&
                fixture.window.scale() == 2.0,
            "host attach must update retained size and scale through one transaction");

    PointerEvent pointer;
    pointer.action = PointerAction::down;
    pointer.button = PointerButton::primary;
    pointer.position = {10.0, 10.0};
    const HostDispatchResult pointer_result = fixture.host.dispatch(pointer, 2);
    require(pointer_result.handled && fixture.probe->pointer_events == 1,
            "normalized pointer input must reach the retained target");
    require(fixture.window.focused_control() == fixture.probe,
            "pointer-down through the host protocol must preserve focus behavior");

    KeyEvent key;
    key.physical_key = 48;
    require(fixture.host.dispatch(key, 3).handled && fixture.probe->key_events == 1,
            "normalized key input must reach the retained focus target");
    TextInputEvent text;
    text.text_utf8 = "é";
    require(fixture.host.dispatch(std::move(text), 4).handled &&
                fixture.probe->last_text == "é",
            "committed UTF-8 host text must retain bytes and handling result");
}

void test_typed_dialog_requests_and_results() {
    Fixture fixture;
    auto& services = static_cast<host::HeadlessHostServices&>(
        fixture.host.services());

    services.queue_dialog_result(
        {{}, 10, HostMessageDialogResult{HostDialogOutcome::accepted,
                                         HostDialogChoice::yes}});
    const HostDialogResult message = services.show_dialog(
        message_request(10, HostMessageButtons::yes_no));
    require(message.status.accepted() &&
                std::get<HostMessageDialogResult>(message.payload).choice ==
                    HostDialogChoice::yes,
            "typed message dialog must preserve its accepted choice");

    HostOpenFileDialogRequest open;
    open.title = "Open recording";
    open.initial_directory = "/fixtures";
    open.suggested_name = "capture.wav";
    open.filters = {{"Recordings", {"wav", "iq"}}};
    open.allow_multiple = true;
    services.queue_dialog_result(
        {{}, 11, HostPathDialogResult{HostDialogOutcome::accepted,
                                      {"/fixtures/a.wav", "/fixtures/b.iq"}}});
    const HostDialogResult opened = services.show_dialog(
        {11, "gallery.window", std::move(open)});
    require(opened.status.accepted() &&
                std::get<HostPathDialogResult>(opened.payload).paths.size() == 2,
            "open-file result must preserve bounded multiple selections");

    HostSaveFileDialogRequest save;
    save.title = "Save profile";
    save.initial_directory = "/profiles";
    save.suggested_name = "receiver";
    save.default_extension = "json";
    save.filters = {{"Profiles", {"json"}}};
    services.queue_dialog_result(
        {{}, 12, HostPathDialogResult{HostDialogOutcome::cancelled, {}}});
    const HostDialogResult saved = services.show_dialog(
        {12, "gallery.window", std::move(save)});
    require(saved.status.accepted() &&
                std::get<HostPathDialogResult>(saved.payload).outcome ==
                    HostDialogOutcome::cancelled &&
                std::get<HostPathDialogResult>(saved.payload).paths.empty(),
            "cancelled save must carry no value for a facade to commit");

    services.queue_dialog_result(
        {{}, 13, HostPathDialogResult{HostDialogOutcome::accepted,
                                      {"/fixtures"}}});
    const HostDialogResult folder = services.show_dialog(
        {13, "gallery.window", HostFolderDialogRequest{"Select folder", "/"}});
    require(folder.status.accepted() &&
                std::get<HostPathDialogResult>(folder.payload).paths.front() ==
                    "/fixtures",
            "folder result must retain its selected UTF-8 path");

    services.queue_dialog_result(
        {{}, 14, HostColorDialogResult{HostDialogOutcome::accepted, 0x3366CCFFU}});
    const HostDialogResult color = services.show_dialog(
        {14, "gallery.window",
         HostColorDialogRequest{"Select accent", 0x112233FFU, false}});
    require(color.status.accepted() &&
                std::get<HostColorDialogResult>(color.payload).rgba == 0x3366CCFFU,
            "color result must retain portable RGBA bytes");

    HostDialogResult wrong_thread;
    std::thread worker([&services, &wrong_thread] {
        wrong_thread = services.show_dialog(message_request(15));
    });
    worker.join();
    require(wrong_thread.status.error == HostServiceError::wrong_thread,
            "dialog calls must reject the wrong thread without entering modal state");

    HostDialogRequest invalid = message_request(0);
    require(services.show_dialog(invalid).status.error ==
                HostServiceError::invalid_argument,
            "zero dialog identity must be rejected before adapter entry");
    invalid = message_request(16);
    std::get<HostMessageDialogRequest>(invalid.payload).message =
        std::string{"\xF0\x28\x8C\x28", 4};
    require(services.show_dialog(invalid).status.error ==
                HostServiceError::invalid_argument,
            "malformed dialog UTF-8 must be rejected before adapter entry");

    services.queue_dialog_result(
        {{}, 17, HostPathDialogResult{HostDialogOutcome::accepted, {"/wrong"}}});
    require(services.show_dialog(message_request(17)).status.error ==
                HostServiceError::backend_failure,
            "an adapter result whose type does not match its request must fail closed");
}

DragEvent drag_event(DragAction action, std::uint64_t session_id, Point position) {
    DragEvent event;
    event.action = action;
    event.session_id = session_id;
    event.position = position;
    event.allowed_effects = DragEffect::copy | DragEffect::move;
    event.items = {
        DragTextData{"receiver Ω"},
        DragFileListData{{"/captures/a.iq", "/captures/b.wav"}},
        DragBinaryData{"application/x-gui-forms-probe", {0x00, 0x7f, 0xff}},
    };
    return event;
}

void test_typed_drag_destination_routing_and_bounds() {
    Fixture fixture;
    require(fixture.host.dispatch(drag_event(DragAction::enter, 90, {10.0, 10.0}), 1)
                .drag_effect == DragEffect::copy,
            "drag enter must return the target's allowed portable effect");
    require(fixture.host.dispatch(drag_event(DragAction::over, 90, {20.0, 10.0}), 2)
                .drag_effect == DragEffect::copy,
            "drag over must route to the retained opt-in target");
    const HostDispatchResult drop = fixture.host.dispatch(
        drag_event(DragAction::drop, 90, {20.0, 10.0}), 3);
    require(drop.handled && drop.drag_effect == DragEffect::copy &&
                fixture.probe->drag_actions ==
                    std::vector<DragAction>{DragAction::enter, DragAction::over,
                                            DragAction::drop} &&
                fixture.probe->last_drag_items.size() == 3 &&
                std::get<DragTextData>(fixture.probe->last_drag_items[0]).text_utf8 ==
                    "receiver Ω" &&
                std::get<DragFileListData>(fixture.probe->last_drag_items[1])
                    .paths_utf8.size() == 2 &&
                std::get<DragBinaryData>(fixture.probe->last_drag_items[2]).bytes.back() ==
                    0xff,
            "typed drop must preserve variant order, UTF-8, file lists, and opaque bytes");
    require(fixture.host.session().snapshot().drag_events == 3 &&
                fixture.host.session().snapshot().drag_drops == 1,
            "drag accounting must distinguish sequenced events from completed drops");

    DragEvent invalid = drag_event(DragAction::enter, 0, {10.0, 10.0});
    require(fixture.host.dispatch(std::move(invalid), 4).error ==
                HostDispatchError::invalid_payload,
            "zero drag identity must be rejected before retained routing");
    invalid = drag_event(DragAction::enter, 91, {10.0, 10.0});
    std::get<DragTextData>(invalid.items[0]).text_utf8 =
        std::string{"\xF0\x28\x8C\x28", 4};
    require(fixture.host.dispatch(std::move(invalid), 5).error ==
                HostDispatchError::invalid_payload,
            "malformed drag UTF-8 must fail at the host boundary");
    invalid = drag_event(DragAction::enter, 92, {10.0, 10.0});
    std::get<DragBinaryData>(invalid.items[2]).bytes.resize(16U * 1024U * 1024U);
    require(fixture.host.dispatch(std::move(invalid), 6).error ==
                HostDispatchError::invalid_payload,
            "aggregate drag payloads beyond 16 MiB must fail before callbacks");

    auto root = make_control<Control>(StableId("drag.root"));
    root->set_requested_bounds({0.0, 0.0, 100.0, 40.0});
    auto left = make_control<InputProbe>(StableId("drag.left"));
    auto right = make_control<InputProbe>(StableId("drag.right"));
    left->set_requested_bounds({0.0, 0.0, 50.0, 40.0});
    right->set_requested_bounds({50.0, 0.0, 50.0, 40.0});
    root->add_child(left);
    root->add_child(right);
    Window transition_window(root, {100.0, 40.0});
    host::HeadlessHost transition_host(transition_window);
    static_cast<void>(transition_host.dispatch(
        drag_event(DragAction::enter, 93, {10.0, 10.0}), 1));
    static_cast<void>(transition_host.dispatch(
        drag_event(DragAction::over, 93, {60.0, 10.0}), 2));
    require(left->drag_actions ==
                std::vector<DragAction>{DragAction::enter, DragAction::leave} &&
                right->drag_actions ==
                std::vector<DragAction>{DragAction::enter, DragAction::over},
            "target crossing must deterministically leave old, enter new, then deliver over");
    right->set_enabled(false);
    right->set_enabled(true);
    static_cast<void>(transition_host.dispatch(
        drag_event(DragAction::leave, 93, {60.0, 10.0}), 3));
    require(right->drag_actions.size() == 2,
            "eligibility revocation must clear drag ownership without a stale callback");
}

void test_nested_modal_order_owner_suppression_and_limit() {
    Fixture fixture;
    auto& services = static_cast<host::HeadlessHostServices&>(
        fixture.host.services());
    PointerEvent down;
    down.action = PointerAction::down;
    down.button = PointerButton::primary;
    down.position = {10.0, 10.0};
    down.pointer_id = 31;
    require(fixture.host.dispatch(down, 1).handled &&
                fixture.window.captured_control() == fixture.probe,
            "modal fixture must begin with focused captured owner input");

    std::string transitions;
    auto observation = services.modal_changed().subscribe(
        [&transitions](const HostModalTransition& transition) {
            transitions += transition.entering ? "enter:" : "leave:";
            transitions += std::to_string(transition.request_id) + ":" +
                std::to_string(transition.depth) + '\n';
        });
    HostDialogResult nested;
    services.set_dialog_handler(
        [&fixture, &nested](const HostDialogRequest& request,
                           host::HeadlessHostServices& adapter) {
            if (request.request_id == 100) {
                require(adapter.snapshot().modal_depth == 1 &&
                            fixture.host.session().snapshot().modal_depth == 1 &&
                            !fixture.window.captured_control(),
                        "modal entry must publish depth and release owner capture first");
                PointerEvent blocked;
                blocked.action = PointerAction::move;
                blocked.position = {20.0, 20.0};
                const std::uint64_t before = fixture.probe->pointer_events;
                const HostDispatchResult blocked_result =
                    fixture.host.dispatch(blocked, 2);
                require(blocked_result.accepted() && !blocked_result.handled &&
                            fixture.probe->pointer_events == before,
                        "owner input must be sequenced but suppressed during a modal call");
                nested = adapter.show_dialog(message_request(200));
                const HostDialogResult duplicate = adapter.show_dialog(request);
                require(duplicate.status.error == HostServiceError::invalid_argument,
                        "an active dialog identity must not reenter itself");
                return HostDialogResult{
                    {}, request.request_id,
                    HostMessageDialogResult{HostDialogOutcome::cancelled,
                                            HostDialogChoice::cancel}};
            }
            return HostDialogResult{
                {}, request.request_id,
                HostMessageDialogResult{HostDialogOutcome::accepted,
                                        HostDialogChoice::ok}};
        });
    const HostDialogResult outer = services.show_dialog(message_request(100));
    require(outer.status.accepted() && nested.status.accepted() &&
                transitions == "enter:100:1\nenter:200:2\nleave:200:1\nleave:100:0\n",
            "nested modal transitions must use one deterministic stack order");
    const HostServicesSnapshot service_snapshot = services.snapshot();
    const HostSessionSnapshot session_snapshot = fixture.host.session().snapshot();
    require(service_snapshot.dialog_requests == 2 &&
                service_snapshot.dialog_completions == 2 &&
                service_snapshot.dialog_cancellations == 1 &&
                service_snapshot.maximum_modal_depth == 2 &&
                service_snapshot.modal_depth == 0 &&
                session_snapshot.modal_transitions == 4 &&
                session_snapshot.modal_input_suppressions == 1 &&
                session_snapshot.modal_depth == 0 &&
                fixture.window.focused_control() == fixture.probe,
            "modal exit must restore owner eligibility without losing focus");
    observation.disconnect();

    Fixture limit_fixture;
    auto& limit_services = static_cast<host::HeadlessHostServices&>(
        limit_fixture.host.services());
    HostServiceError terminal_error = HostServiceError::none;
    limit_services.set_dialog_handler(
        [&terminal_error](const HostDialogRequest& request,
                          host::HeadlessHostServices& adapter) {
            if (request.request_id <= HostServices::maximum_nested_modal_depth) {
                const HostDialogResult child = adapter.show_dialog(
                    message_request(request.request_id + 1));
                if (!child.status.accepted()) {
                    terminal_error = child.status.error;
                }
            }
            return HostDialogResult{
                {}, request.request_id,
                HostMessageDialogResult{HostDialogOutcome::cancelled,
                                        HostDialogChoice::cancel}};
        });
    require(limit_services.show_dialog(message_request(1)).status.accepted() &&
                terminal_error == HostServiceError::modal_limit &&
                limit_services.snapshot().maximum_modal_depth ==
                    HostServices::maximum_nested_modal_depth &&
                limit_services.snapshot().modal_depth == 0,
            "nested modal depth must fail closed at the portable bound and unwind");

    Fixture shutdown_fixture;
    auto& shutdown_services = static_cast<host::HeadlessHostServices&>(
        shutdown_fixture.host.services());
    shutdown_services.set_dialog_handler(
        [](const HostDialogRequest& request, host::HeadlessHostServices& adapter) {
            adapter.shutdown();
            return HostDialogResult{
                {}, request.request_id,
                HostMessageDialogResult{HostDialogOutcome::accepted,
                                        HostDialogChoice::ok}};
        });
    const HostDialogResult interrupted =
        shutdown_services.show_dialog(message_request(300));
    require(interrupted.status.error == HostServiceError::after_shutdown &&
                shutdown_services.snapshot().shutdown &&
                shutdown_services.snapshot().modal_depth == 0,
            "shutdown during a modal adapter call must unwind and reject its stale result");
}

void test_display_capture_and_occlusion_synchronization() {
    Fixture fixture;
    require(fixture.host.dispatch(HostAttachEvent{{640.0, 360.0}, 1.0}, 1).accepted(),
            "host synchronization fixture must attach");
    require(fixture.host.dispatch(
                HostDisplayEvent{{reference_monitor()}}, 2).accepted(),
            "valid display topology must enter the sequenced host trace");
    HostDisplayEvent invalid_display;
    require(fixture.host.dispatch(std::move(invalid_display), 3).error ==
                HostDispatchError::invalid_geometry,
            "empty display topology must be rejected before snapshot mutation");
    const HostSessionSnapshot display_snapshot = fixture.host.session().snapshot();
    require(display_snapshot.display_changes == 1 &&
                display_snapshot.monitor_count == 1,
            "accepted display topology must update structured session state once");

    PointerEvent down;
    down.action = PointerAction::down;
    down.button = PointerButton::primary;
    down.position = {10.0, 10.0};
    down.pointer_id = 7;
    require(fixture.host.dispatch(down, 4).handled &&
                fixture.window.captured_control() == fixture.probe &&
                fixture.host.services().snapshot().pointer_captured &&
                fixture.host.services().snapshot().captured_pointer_id == 7,
            "retained pointer-down capture must synchronize to the host service");
    PointerEvent up = down;
    up.action = PointerAction::up;
    require(fixture.host.dispatch(up, 5).handled &&
                !fixture.window.captured_control() &&
                !fixture.host.services().snapshot().pointer_captured &&
                fixture.host.services().snapshot().pointer_capture_updates == 2,
            "physical release must synchronously release retained and host capture");

    down.pointer_id = 8;
    require(fixture.host.dispatch(down, 6).handled &&
                fixture.host.services().snapshot().pointer_captured,
            "a second retained capture must synchronize before revocation");
    fixture.probe->set_enabled(false);
    require(!fixture.window.captured_control() &&
                !fixture.host.services().snapshot().pointer_captured &&
                fixture.host.services().snapshot().pointer_capture_updates == 4 &&
                fixture.window.metrics_snapshot().capture_revocations == 1,
            "eligibility revocation must release retained and host capture once");
    fixture.probe->set_enabled(true);

    const FrameTime base{};
    auto active = fixture.window.activate_surface(
        fixture.probe, std::chrono::milliseconds(10),
        base + std::chrono::nanoseconds(10));
    require(fixture.host.dispatch(HostOcclusionEvent{true}, 7).accepted() &&
                fixture.window.occluded() && !fixture.window.next_wake().has_value() &&
                active.connected(),
            "host occlusion must pause rather than revoke active retained work");
    require(fixture.host.dispatch(HostOcclusionEvent{false}, 20).accepted() &&
                !fixture.window.occluded() &&
                fixture.window.next_wake() == base + std::chrono::nanoseconds(20),
            "host resume must rebase overdue active work to one immediate tick");
    const FramePollResult resumed =
        fixture.window.poll_frame_schedule(base + std::chrono::nanoseconds(20));
    require(resumed.active_surface_ticks == 1,
            "resumed host work must not replay missed active frames");
}

void test_environment_services_are_bounded_and_deterministic() {
    Fixture fixture;
    HostServices& services = fixture.host.services();
    const HostMonitorResult monitors = services.query_monitors();
    require(monitors.status.accepted() && monitors.monitors.size() == 1 &&
                monitors.monitors.front().id == "headless.primary" &&
                monitors.monitors.front().work_area == Rect{0.0, 0.0, 1920.0, 1040.0} &&
                monitors.monitors.front().primary,
            "headless monitor query must return the stable reference desktop");

    require(services.set_cursor(CursorKind::hand).accepted() &&
                services.snapshot().cursor == CursorKind::hand,
            "cursor service must retain the last accepted portable cursor");
    require(services.set_pointer_capture(true, 0).error ==
                HostServiceError::invalid_argument,
            "host capture must reject the reserved zero pointer identity");
    require(services.write_clipboard_text("M3b Ω clipboard").accepted(),
            "headless clipboard must accept bounded UTF-8 text");
    const HostClipboardTextResult clipboard = services.read_clipboard_text();
    require(clipboard.status.accepted() && clipboard.has_text &&
                clipboard.text_utf8 == "M3b Ω clipboard" && clipboard.generation == 1,
            "headless clipboard read must reproduce text and generation");

    const std::string invalid_utf8{"\xF0\x28\x8C\x28", 4};
    require(services.write_clipboard_text(invalid_utf8).error ==
                HostServiceError::invalid_utf8,
            "clipboard must reject malformed UTF-8 before reaching the adapter");
    const std::string too_large(HostServices::maximum_clipboard_text_bytes + 1U, 'x');
    require(services.write_clipboard_text(too_large).error ==
                HostServiceError::too_large,
            "clipboard must reject text beyond the declared host-service bound");

    HostMonitorResult wrong_thread;
    std::thread worker([&services, &wrong_thread] {
        wrong_thread = services.query_monitors();
    });
    worker.join();
    require(wrong_thread.status.error == HostServiceError::wrong_thread,
            "host services must reject calls outside the owning UI thread");

    const HostServicesSnapshot before_shutdown = services.snapshot();
    require(before_shutdown.monitor_queries == 1 &&
                before_shutdown.cursor_updates == 1 &&
                before_shutdown.clipboard_writes == 1 &&
                before_shutdown.clipboard_reads == 1 &&
                before_shutdown.rejected_requests == 3,
            "service accounting must reject malformed work without mutating state on a wrong thread");
    services.shutdown();
    require(services.read_clipboard_text().status.error ==
                HostServiceError::after_shutdown,
            "service requests after shutdown must be rejected deterministically");
    require(services.snapshot().to_json().find("\"shutdown\":true") !=
                std::string::npos,
            "service snapshot must expose shutdown state");

    require(canonical_service_trace() == canonical_service_trace(),
            "headless service traces must be byte-identical across fresh adapters");
}

void test_sequence_geometry_and_shutdown_guards() {
    Fixture fixture;
    HostDispatchResult wrong_thread;
    std::thread worker([&fixture, &wrong_thread] {
        HostEvent event{1, 1, HostActivationEvent{true}};
        wrong_thread = fixture.host.session().dispatch(std::move(event));
    });
    worker.join();
    require(wrong_thread.error == HostDispatchError::wrong_thread,
            "host session must reject delivery outside its owning UI thread");

    HostEvent zero{0, 1, HostActivationEvent{true}};
    require(fixture.host.session().dispatch(std::move(zero)).error ==
                HostDispatchError::sequence_zero,
            "zero host sequence must be rejected");

    HostEvent first{4, 2, HostAttachEvent{{320.0, 180.0}, 1.0}};
    require(fixture.host.session().dispatch(std::move(first)).accepted(),
            "first explicit monotonic host sequence must be accepted");
    HostEvent repeated{4, 3, HostActivationEvent{true}};
    require(fixture.host.session().dispatch(std::move(repeated)).error ==
                HostDispatchError::non_monotonic_sequence,
            "replayed host sequence must be rejected");
    HostEvent invalid{5, 4, HostResizeEvent{{-1.0, 10.0}}};
    require(fixture.host.session().dispatch(std::move(invalid)).error ==
                HostDispatchError::invalid_geometry,
            "invalid host geometry must be rejected before mutating the window");

    auto active = fixture.window.activate_surface(
        fixture.probe, std::chrono::milliseconds(10),
        FrameTime{} + std::chrono::milliseconds(10));
    fixture.window.capture_pointer(fixture.probe, 11);
    require(fixture.host.services().snapshot().pointer_captured,
            "explicit retained capture must reach host services before shutdown");
    HostEvent shutdown{6, 5, HostShutdownEvent{}};
    require(fixture.host.session().dispatch(std::move(shutdown)).accepted() &&
                !active.connected() &&
                !fixture.host.services().snapshot().pointer_captured,
            "host shutdown must synchronously revoke scheduled work and capture");
    HostEvent after{7, 6, HostActivationEvent{true}};
    require(fixture.host.session().dispatch(std::move(after)).error ==
                HostDispatchError::after_shutdown,
            "events after shutdown must be rejected deterministically");
}

void test_close_cancellation_and_closed_cleanup() {
    Fixture fixture;
    static_cast<void>(fixture.host.dispatch(HostAttachEvent{{320.0, 180.0}, 1.0}, 1));
    auto cancellation = fixture.host.session().closing().subscribe(
        [](HostCloseRequest& request) { request.cancel = true; });
    const HostDispatchResult cancelled = fixture.host.dispatch(
        HostCloseRequest{HostCloseReason::user, false}, 2);
    require(!cancelled.close_allowed,
            "host close request must expose synchronous cancellation");
    cancellation.disconnect();
    const HostDispatchResult allowed = fixture.host.dispatch(
        HostCloseRequest{HostCloseReason::user, false}, 3);
    require(allowed.close_allowed,
            "disconnecting close policy must restore default allow behavior");

    auto active = fixture.window.activate_surface(
        fixture.probe, std::chrono::milliseconds(10),
        FrameTime{} + std::chrono::milliseconds(10));
    static_cast<void>(fixture.host.dispatch(HostClosedEvent{HostCloseReason::user}, 4));
    const HostSessionSnapshot snapshot = fixture.host.session().snapshot();
    require(snapshot.close_requests == 2 && snapshot.close_cancellations == 1 &&
                snapshot.closed && !active.connected(),
            "closed lifecycle must account cancellation and revoke scheduled work");
}

void test_headless_trace_is_byte_deterministic() {
    const std::string first = canonical_host_trace();
    const std::string second = canonical_host_trace();
    require(first == second, "host event replay trace must be byte-identical");
    require(first.find("event=attach accepted=1") != std::string::npos &&
                first.find("event=shutdown") != std::string::npos,
            "host trace must expose lifecycle endpoints");
    require(canonical_modal_trace() == canonical_modal_trace(),
            "nested modal trace must be byte-identical across fresh adapters");
}

} // namespace

int main() {
    try {
        test_capabilities_and_normalized_dispatch();
        test_typed_dialog_requests_and_results();
        test_typed_drag_destination_routing_and_bounds();
        test_nested_modal_order_owner_suppression_and_limit();
        test_display_capture_and_occlusion_synchronization();
        test_environment_services_are_bounded_and_deterministic();
        test_sequence_geometry_and_shutdown_guards();
        test_close_cancellation_and_closed_cleanup();
        test_headless_trace_is_byte_deterministic();
        std::cout << "headless_capabilities="
                  << host::headless_capabilities().to_json() << '\n';
        std::cout << canonical_host_trace();
        std::cout << canonical_service_trace();
        std::cout << canonical_modal_trace();
        std::cout << "gui_forms_host_protocol_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_host_protocol_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
