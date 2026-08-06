#import <AppKit/AppKit.h>

#include "macos_host.hpp"

#include "gui_forms/gui_forms.hpp"

#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

namespace {

class FrameProbe final : public gui_forms::Control {
public:
    explicit FrameProbe(gui_forms::StableId stable_id)
        : Control(std::move(stable_id)) {}

    void on_frame(gui_forms::FrameTime) override { ++ticks; }
    void on_paint(gui_forms::Painter&, gui_forms::Rect) override { ++paints; }

    std::uint64_t ticks{};
    std::uint64_t paints{};
};

struct RearmProbeState final {
    std::shared_ptr<FrameProbe> root;
    gui_forms::Window* window{};
    gui_forms::FrameRequestToken* initial{};
    gui_forms::FrameRequestToken replacement;
    std::function<void()> wake;
    std::function<void()> request_close;
    std::array<std::uint64_t, 8> ticks_before{};
    std::array<std::uint64_t, 8> ticks_at_rearm{};
    std::array<std::uint64_t, 8> ticks_after{};
    std::size_t cycle{};
    std::uint64_t successful_cycles{};
    std::uint64_t quiescent_gap_failures{};
    gui_forms::DispatchOperation worker_dispatch;
    gui_forms::DispatchOperation nested_dispatch;
    std::string dispatch_trace;
    bool worker_required_invoke{};
    bool worker_ran_on_ui_thread{};
    bool nested_ran_on_ui_thread{};
    bool synchronous_worker_required_invoke{};
    bool synchronous_ran_on_ui_thread{};
    bool synchronous_returned{};
    std::string synchronous_fault;
};

void run_rearm_cycle(const std::shared_ptr<RearmProbeState>& state) {
    const std::size_t current = state->cycle;
    state->ticks_before[current] = state->root->ticks;
    if (current == 0U) {
        state->initial->disconnect();
    } else {
        state->replacement.disconnect();
    }
    state->wake();
    const std::shared_ptr<RearmProbeState> retained_state = state;
    dispatch_after(
        dispatch_time(DISPATCH_TIME_NOW, 12 * NSEC_PER_MSEC),
        dispatch_get_main_queue(), ^{
            retained_state->ticks_at_rearm[current] =
                retained_state->root->ticks;
            if (retained_state->ticks_at_rearm[current] !=
                retained_state->ticks_before[current]) {
                ++retained_state->quiescent_gap_failures;
            }
            retained_state->replacement =
                retained_state->window->activate_surface(
                    retained_state->root, std::chrono::milliseconds(10),
                    gui_forms::FrameClock::now() +
                        std::chrono::milliseconds(10));
            retained_state->wake();
            dispatch_after(
                dispatch_time(DISPATCH_TIME_NOW, 70 * NSEC_PER_MSEC),
                dispatch_get_main_queue(), ^{
                    retained_state->ticks_after[current] =
                        retained_state->root->ticks;
                    if (retained_state->ticks_after[current] >
                        retained_state->ticks_at_rearm[current]) {
                        ++retained_state->successful_cycles;
                    }
                    ++retained_state->cycle;
                    if (retained_state->cycle ==
                        retained_state->ticks_before.size()) {
                        retained_state->request_close();
                        retained_state->request_close();
                        return;
                    }
                    dispatch_after(
                        dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_MSEC),
                        dispatch_get_main_queue(), ^{
                            run_rearm_cycle(retained_state);
                        });
                });
        });
}

bool test_isolated_macos_services() {
    constexpr const char* pasteboard_name = "local.gui_forms.m3b.test";
    gui_forms::host::MacHostServiceOptions service_options;
    service_options.clipboard_name = pasteboard_name;
    service_options.cancel_dialogs_for_testing = true;
    auto services = gui_forms::host::make_macos_host_services(service_options);
    if (!services) {
        return false;
    }
    const gui_forms::HostMonitorResult monitors = services->query_monitors();
    if (!monitors.status.accepted() || monitors.monitors.empty()) {
        return false;
    }
    if (!services->set_cursor(gui_forms::CursorKind::hand).accepted() ||
        !services->set_cursor(gui_forms::CursorKind::arrow).accepted() ||
        !services->set_pointer_capture(true, 9).accepted() ||
        !services->set_pointer_capture(false).accepted() ||
        !services->write_clipboard_text("M3c AppKit Ω").accepted()) {
        return false;
    }
    const gui_forms::HostClipboardTextResult clipboard =
        services->read_clipboard_text();
    if (!clipboard.status.accepted() || !clipboard.has_text ||
        clipboard.text_utf8 != "M3c AppKit Ω") {
        return false;
    }
    gui_forms::HostMessageDialogRequest message_request;
    message_request.title = "M3d message";
    message_request.message = "Native cancellation smoke";
    message_request.buttons = gui_forms::HostMessageButtons::ok_cancel;
    const gui_forms::HostDialogResult message = services->show_dialog(
        {101, "test.window", std::move(message_request)});
    gui_forms::HostOpenFileDialogRequest open_request;
    open_request.title = "M3d open";
    open_request.filters = {{"Text", {"txt"}}};
    const gui_forms::HostDialogResult open = services->show_dialog(
        {102, "test.window", std::move(open_request)});
    gui_forms::HostSaveFileDialogRequest save_request;
    save_request.title = "M3d save";
    save_request.suggested_name = "evidence";
    save_request.default_extension = "txt";
    const gui_forms::HostDialogResult save = services->show_dialog(
        {103, "test.window", std::move(save_request)});
    const gui_forms::HostDialogResult folder = services->show_dialog(
        {104, "test.window",
         gui_forms::HostFolderDialogRequest{"M3d folder", {}}});
    const gui_forms::HostDialogResult color = services->show_dialog(
        {105, "test.window",
         gui_forms::HostColorDialogRequest{"M3d color", 0x3366CCFFU, false}});
    const auto cancelled = [](const gui_forms::HostDialogResult& result) {
        return result.status.accepted() && std::visit(
            [](const auto& payload) {
                return payload.outcome == gui_forms::HostDialogOutcome::cancelled;
            }, result.payload);
    };
    if (!cancelled(message) || !cancelled(open) || !cancelled(save) ||
        !cancelled(folder) || !cancelled(color) ||
        services->snapshot().dialog_requests != 5 ||
        services->snapshot().dialog_completions != 5 ||
        services->snapshot().dialog_cancellations != 5 ||
        services->snapshot().maximum_modal_depth != 1) {
        return false;
    }
    services->shutdown();
    if (services->query_monitors().status.error !=
        gui_forms::HostServiceError::after_shutdown) {
        return false;
    }
    std::cout << "appkit_services monitor_count=" << monitors.monitors.size()
              << " primary=" << monitors.monitors.front().id
              << " clipboard_generation=" << clipboard.generation
              << " snapshot=" << services->snapshot().to_json() << '\n';
    NSString* name = [NSString stringWithUTF8String:pasteboard_name];
    [[NSPasteboard pasteboardWithName:name] releaseGlobally];
    return true;
}

} // namespace

int main() {
    @autoreleasepool {
        [NSApplication sharedApplication];
        if (!test_isolated_macos_services()) {
            return 6;
        }
    }
    const gui_forms::HostCapabilities capabilities =
        gui_forms::host::macos_capabilities();
    if (!capabilities.supports(
            gui_forms::HostCapability::lifecycle |
            gui_forms::HostCapability::scale_notifications |
            gui_forms::HostCapability::occlusion |
            gui_forms::HostCapability::pointer_input |
            gui_forms::HostCapability::keyboard_input |
            gui_forms::HostCapability::text_composition |
            gui_forms::HostCapability::monitor_geometry |
            gui_forms::HostCapability::pointer_capture |
            gui_forms::HostCapability::cursor |
            gui_forms::HostCapability::clipboard |
            gui_forms::HostCapability::typed_drag_destination |
            gui_forms::HostCapability::dialogs)) {
        return 4;
    }
    auto root = gui_forms::make_control<FrameProbe>(
        gui_forms::StableId("host.close.root"));
    auto surface = gui_forms::make_control<FrameProbe>(
        gui_forms::StableId("host.close.surface"));
    surface->set_visible(false);
    surface->set_requested_bounds({0.0, 0.0, 320.0, 180.0});
    root->add_child(surface);
    auto window = std::make_unique<gui_forms::Window>(root,
                                                      gui_forms::Size{320.0, 180.0});
    gui_forms::Window* const live_window = window.get();
    auto active = window->activate_surface(
        surface, std::chrono::milliseconds(10),
        gui_forms::FrameClock::now() + std::chrono::milliseconds(10));
    std::shared_ptr<RearmProbeState> rearm_state;
    std::thread synchronous_worker;
    gui_forms::host::MacHostOptions options;
    options.title = "GUI.Forms Close Test";
    options.initial_size = {320.0, 180.0};
    options.minimum_size = {320.0, 180.0};
    options.print_metrics_on_close = false;
    std::uint64_t close_requests = 0;
    std::string final_host_snapshot;
    options.close_request = [&close_requests](gui_forms::HostCloseRequest& request) {
        ++close_requests;
        request.cancel = close_requests == 1;
    };
    options.final_snapshot = [&final_host_snapshot](std::string_view,
                                                    std::string_view host) {
        final_host_snapshot.assign(host);
    };
    options.host_ready = [&](auto wake, auto request_close, auto, auto, auto,
                             auto, auto) {
        // Exercise the exact transport used by pause/resume: disconnect the
        // last active lease, force the host to disarm, then publish a new
        // lease and wake it again. Repeating the transition catches a host
        // source that reports a logically live model but remains disarmed.
        rearm_state = std::make_shared<RearmProbeState>();
        rearm_state->root = surface;
        rearm_state->window = live_window;
        rearm_state->initial = &active;
        rearm_state->wake = std::move(wake);
        rearm_state->request_close = std::move(request_close);
        std::thread worker([state = rearm_state] {
            state->worker_required_invoke = state->root->invoke_required();
            state->worker_dispatch = state->root->begin_invoke([state] {
                state->worker_ran_on_ui_thread =
                    !state->root->invoke_required();
                state->dispatch_trace += 'A';
                state->nested_dispatch = state->root->begin_invoke([state] {
                    state->nested_ran_on_ui_thread =
                        !state->root->invoke_required();
                    state->dispatch_trace += 'B';
                });
            });
        });
        worker.join();
        synchronous_worker = std::thread([state = rearm_state] {
            state->synchronous_worker_required_invoke =
                state->root->invoke_required();
            state->root->invoke([state] {
                state->synchronous_ran_on_ui_thread =
                    !state->root->invoke_required();
            });
            state->synchronous_returned = true;
            try {
                state->window->invoke([] {
                    throw std::runtime_error("native synchronous fault");
                });
            } catch (const std::runtime_error& error) {
                state->synchronous_fault = error.what();
            }
        });
        const std::shared_ptr<RearmProbeState> scheduled_state = rearm_state;
        dispatch_after(
            dispatch_time(DISPATCH_TIME_NOW, 80 * NSEC_PER_MSEC),
            dispatch_get_main_queue(), ^{
                surface->set_visible(true);
                scheduled_state->wake();
                dispatch_after(
                    dispatch_time(DISPATCH_TIME_NOW, 80 * NSEC_PER_MSEC),
                    dispatch_get_main_queue(), ^{
                        run_rearm_cycle(scheduled_state);
                    });
            });
    };
    const int result = gui_forms::host::run_macos(std::move(window), std::move(options));
    if (synchronous_worker.joinable()) synchronous_worker.join();
    if (result != 0) {
        return result;
    }
    if (!rearm_state || active.connected() || rearm_state->replacement.connected()) {
        return 3;
    }
    if (close_requests != 2) {
        return 5;
    }
    if (final_host_snapshot.find("\"display_changes\":") == std::string::npos ||
        final_host_snapshot.find("\"display_changes\":0") != std::string::npos ||
        final_host_snapshot.find("\"monitor_count\":1") == std::string::npos ||
        final_host_snapshot.find("\"pointer_capture\"") == std::string::npos) {
        return 7;
    }
    if (rearm_state->ticks_before.front() == 0 ||
        rearm_state->ticks_after.back() <= rearm_state->ticks_before.front() + 2U ||
        rearm_state->root->paints < 4U ||
        rearm_state->successful_cycles != rearm_state->ticks_before.size() ||
        rearm_state->quiescent_gap_failures != 0U ||
        !rearm_state->worker_required_invoke ||
        !rearm_state->worker_ran_on_ui_thread ||
        !rearm_state->nested_ran_on_ui_thread ||
        !rearm_state->synchronous_worker_required_invoke ||
        !rearm_state->synchronous_ran_on_ui_thread ||
        !rearm_state->synchronous_returned ||
        rearm_state->synchronous_fault != "native synchronous fault" ||
        rearm_state->dispatch_trace != "AB" ||
        rearm_state->worker_dispatch.state() !=
            gui_forms::DispatchOperationState::completed ||
        rearm_state->nested_dispatch.state() !=
            gui_forms::DispatchOperationState::completed) {
        std::cerr << "AppKit scheduled wake did not autonomously advance after "
                     "repeated active-surface replacement: before="
                  << rearm_state->ticks_before.front()
                  << " after=" << rearm_state->ticks_after.back()
                  << " paints=" << rearm_state->root->paints
                  << " successful_cycles=" << rearm_state->successful_cycles
                  << " quiescent_gap_failures="
                  << rearm_state->quiescent_gap_failures << '\n';
        for (std::size_t cycle = 0; cycle < rearm_state->ticks_before.size();
             ++cycle) {
            std::cerr << "  cycle=" << cycle
                      << " before=" << rearm_state->ticks_before[cycle]
                      << " rearm=" << rearm_state->ticks_at_rearm[cycle]
                      << " after=" << rearm_state->ticks_after[cycle] << '\n';
        }
        return 8;
    }
    std::cout << "macos_capabilities=" << capabilities.to_json()
              << " close_requests=" << close_requests << '\n';
    return 0;
}
