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
    std::function<void()> finish;
};

struct NativeWindowProbeState final {
    bool found{};
    bool moved{};
    bool resized{};
    bool zoomed{};
    bool minimized{};
    bool restored{};
};

void run_rearm_cycle(const std::shared_ptr<RearmProbeState>& state);

using VoidHostCallback = std::function<void()>;
using ShowDialogCallback = std::function<gui_forms::HostDialogResult(
    const gui_forms::HostDialogRequest&)>;
using ShowTooltipCallback = std::function<gui_forms::HostServiceStatus(
    const gui_forms::HostTooltipRequest&)>;
using ReadClipboardCallback =
    std::function<gui_forms::HostClipboardTextResult()>;
using WriteClipboardCallback =
    std::function<gui_forms::HostServiceStatus(std::string_view)>;

bool dialog_result_cancelled(const gui_forms::HostDialogResult& result) {
    if (!result.status.accepted()) return false;
    if (std::holds_alternative<gui_forms::HostMessageDialogResult>(
            result.payload)) {
        return std::get<gui_forms::HostMessageDialogResult>(result.payload).outcome ==
               gui_forms::HostDialogOutcome::cancelled;
    }
    if (std::holds_alternative<gui_forms::HostPathDialogResult>(result.payload)) {
        return std::get<gui_forms::HostPathDialogResult>(result.payload).outcome ==
               gui_forms::HostDialogOutcome::cancelled;
    }
    return std::get<gui_forms::HostColorDialogResult>(result.payload).outcome ==
           gui_forms::HostDialogOutcome::cancelled;
}

class RecordNestedDispatch final {
public:
    explicit RecordNestedDispatch(std::shared_ptr<RearmProbeState> state)
        : state_(std::move(state)) {}

    void operator()() const {
        (*state_).nested_ran_on_ui_thread =
            !(*(*state_).root).invoke_required();
        (*state_).dispatch_trace += 'B';
    }

private:
    std::shared_ptr<RearmProbeState> state_;
};

class RecordWorkerDispatch final {
public:
    explicit RecordWorkerDispatch(std::shared_ptr<RearmProbeState> state)
        : state_(std::move(state)) {}

    void operator()() const {
        (*state_).worker_ran_on_ui_thread =
            !(*(*state_).root).invoke_required();
        (*state_).dispatch_trace += 'A';
        (*state_).nested_dispatch =
            (*(*state_).root).begin_invoke(RecordNestedDispatch(state_));
    }

private:
    std::shared_ptr<RearmProbeState> state_;
};

class BeginWorkerDispatch final {
public:
    explicit BeginWorkerDispatch(std::shared_ptr<RearmProbeState> state)
        : state_(std::move(state)) {}

    void operator()() const {
        (*state_).worker_required_invoke =
            (*(*state_).root).invoke_required();
        (*state_).worker_dispatch =
            (*(*state_).root).begin_invoke(RecordWorkerDispatch(state_));
    }

private:
    std::shared_ptr<RearmProbeState> state_;
};

class RecordSynchronousDispatch final {
public:
    explicit RecordSynchronousDispatch(std::shared_ptr<RearmProbeState> state)
        : state_(std::move(state)) {}

    void operator()() const {
        (*state_).synchronous_ran_on_ui_thread =
            !(*(*state_).root).invoke_required();
    }

private:
    std::shared_ptr<RearmProbeState> state_;
};

class ThrowNativeSynchronousFault final {
public:
    void operator()() const {
        throw std::runtime_error("native synchronous fault");
    }
};

class RunSynchronousDispatch final {
public:
    explicit RunSynchronousDispatch(std::shared_ptr<RearmProbeState> state)
        : state_(std::move(state)) {}

    void operator()() const {
        (*state_).synchronous_worker_required_invoke =
            (*(*state_).root).invoke_required();
        (*(*state_).root).invoke(RecordSynchronousDispatch(state_));
        (*state_).synchronous_returned = true;
        try {
            (*(*state_).window).invoke(ThrowNativeSynchronousFault());
        } catch (const std::runtime_error& error) {
            (*state_).synchronous_fault = error.what();
        }
    }

private:
    std::shared_ptr<RearmProbeState> state_;
};

class CountCloseRequests final {
public:
    explicit CountCloseRequests(std::uint64_t& requests) noexcept
        : requests_(requests) {}

    void operator()(gui_forms::HostCloseRequest& request) const {
        ++requests_;
        request.cancel = requests_ == 1U;
    }

private:
    std::uint64_t& requests_;
};

class StoreFinalHostSnapshot final {
public:
    explicit StoreFinalHostSnapshot(std::string& snapshot) noexcept
        : snapshot_(snapshot) {}

    void operator()(std::string_view, std::string_view host) const {
        snapshot_.assign(host);
    }

private:
    std::string& snapshot_;
};

class PrepareCloseTestHost final {
public:
    PrepareCloseTestHost(
        std::shared_ptr<RearmProbeState>& rearm_state,
        const std::shared_ptr<FrameProbe>& surface,
        gui_forms::Window* live_window,
        gui_forms::FrameRequestToken& active,
        std::thread& synchronous_worker,
        const std::shared_ptr<NativeWindowProbeState>& native_window_probe) noexcept
        : rearm_state_(rearm_state),
          surface_(surface),
          live_window_(live_window),
          active_(active),
          synchronous_worker_(synchronous_worker),
          native_window_probe_(native_window_probe) {}

    void operator()(
        VoidHostCallback wake,
        VoidHostCallback request_close,
        ShowDialogCallback,
        ShowTooltipCallback,
        VoidHostCallback,
        ReadClipboardCallback,
        WriteClipboardCallback) const {
        // Exercise the exact transport used by pause/resume: disconnect the
        // last active lease, force the host to disarm, then publish a new
        // lease and wake it again. Repeating the transition catches a host
        // source that reports a logically live model but remains disarmed.
        rearm_state_ = std::make_shared<RearmProbeState>();
        (*rearm_state_).root = surface_;
        (*rearm_state_).window = live_window_;
        (*rearm_state_).initial = &active_;
        (*rearm_state_).wake = std::move(wake);
        (*rearm_state_).request_close = std::move(request_close);
        std::thread worker{BeginWorkerDispatch(rearm_state_)};
        worker.join();
        synchronous_worker_ =
            std::thread{RunSynchronousDispatch(rearm_state_)};
        const std::shared_ptr<NativeWindowProbeState> nativeWindowProbe =
            native_window_probe_;
        const std::function<void()> requestClose =
            (*rearm_state_).request_close;
        (*rearm_state_).finish = [nativeWindowProbe, requestClose] {
            NSWindow* nativeWindow = nil;
            for (NSWindow* candidate in NSApplication.sharedApplication.windows) {
                if ([candidate.title isEqualToString:@"GUI.Forms Close Test"]) {
                    nativeWindow = candidate;
                    break;
                }
            }
            if (nativeWindow == nil) return;
            nativeWindowProbe->found = true;
            [nativeWindow setAnimationBehavior:NSWindowAnimationBehaviorNone];
            const NSRect original = nativeWindow.frame;
            NSRect changed = original;
            changed.origin.x += 6.0;
            changed.origin.y -= 6.0;
            changed.size.width += 12.0;
            changed.size.height += 12.0;
            [nativeWindow setFrame:changed display:NO];
            const NSRect committed = nativeWindow.frame;
            nativeWindowProbe->moved =
                committed.origin.x != original.origin.x ||
                committed.origin.y != original.origin.y;
            nativeWindowProbe->resized =
                committed.size.width != original.size.width ||
                committed.size.height != original.size.height;
            [nativeWindow zoom:nil];
            nativeWindowProbe->zoomed = nativeWindow.isZoomed ||
                !NSEqualRects(nativeWindow.frame, committed);
            [nativeWindow zoom:nil];
            [nativeWindow miniaturize:nil];
            dispatch_after(
                dispatch_time(DISPATCH_TIME_NOW, 80 * NSEC_PER_MSEC),
                dispatch_get_main_queue(), ^{
                    nativeWindowProbe->minimized = nativeWindow.isMiniaturized;
                    [nativeWindow deminiaturize:nil];
                    dispatch_after(
                        dispatch_time(DISPATCH_TIME_NOW, 80 * NSEC_PER_MSEC),
                        dispatch_get_main_queue(), ^{
                            nativeWindowProbe->restored =
                                !nativeWindow.isMiniaturized;
                            requestClose();
                            requestClose();
                        });
                });
        };
        const std::shared_ptr<RearmProbeState> scheduled_state = rearm_state_;
        const std::shared_ptr<FrameProbe> scheduled_surface = surface_;
        dispatch_after(
            dispatch_time(DISPATCH_TIME_NOW, 80 * NSEC_PER_MSEC),
            dispatch_get_main_queue(), ^{
                (*scheduled_surface).set_visible(true);
                (*scheduled_state).wake();
                dispatch_after(
                    dispatch_time(DISPATCH_TIME_NOW, 80 * NSEC_PER_MSEC),
                    dispatch_get_main_queue(), ^{
                        run_rearm_cycle(scheduled_state);
                    });
            });
    }

private:
    std::shared_ptr<RearmProbeState>& rearm_state_;
    std::shared_ptr<FrameProbe> surface_;
    gui_forms::Window* live_window_;
    gui_forms::FrameRequestToken& active_;
    std::thread& synchronous_worker_;
    std::shared_ptr<NativeWindowProbeState> native_window_probe_;
};

void run_rearm_cycle(const std::shared_ptr<RearmProbeState>& state) {
    const std::size_t current = (*state).cycle;
    (*state).ticks_before[current] = (*(*state).root).ticks;
    if (current == 0U) {
        (*(*state).initial).disconnect();
    } else {
        (*state).replacement.disconnect();
    }
    (*state).wake();
    const std::shared_ptr<RearmProbeState> retained_state = state;
    dispatch_after(
        dispatch_time(DISPATCH_TIME_NOW, 12 * NSEC_PER_MSEC),
        dispatch_get_main_queue(), ^{
            (*retained_state).ticks_at_rearm[current] =
                (*(*retained_state).root).ticks;
            if ((*retained_state).ticks_at_rearm[current] !=
                (*retained_state).ticks_before[current]) {
                ++(*retained_state).quiescent_gap_failures;
            }
            (*retained_state).replacement =
                (*(*retained_state).window).activate_surface(
                    (*retained_state).root, std::chrono::milliseconds(10),
                    gui_forms::FrameClock::now() +
                        std::chrono::milliseconds(10));
            (*retained_state).wake();
            dispatch_after(
                dispatch_time(DISPATCH_TIME_NOW, 70 * NSEC_PER_MSEC),
                dispatch_get_main_queue(), ^{
                    (*retained_state).ticks_after[current] =
                        (*(*retained_state).root).ticks;
                    if ((*retained_state).ticks_after[current] >
                        (*retained_state).ticks_at_rearm[current]) {
                        ++(*retained_state).successful_cycles;
                    }
                    ++(*retained_state).cycle;
                    if ((*retained_state).cycle ==
                        (*retained_state).ticks_before.size()) {
                        if ((*retained_state).finish) {
                            (*retained_state).finish();
                        } else {
                            (*retained_state).request_close();
                            (*retained_state).request_close();
                        }
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
    std::unique_ptr<gui_forms::HostServices> services =
        gui_forms::host::make_macos_host_services(service_options);
    if (!services) {
        return false;
    }
    const gui_forms::HostMonitorResult monitors = (*services).query_monitors();
    if (!monitors.status.accepted() || monitors.monitors.empty()) {
        return false;
    }
    if (!(*services).set_cursor(gui_forms::CursorKind::hand).accepted() ||
        !(*services).set_cursor(gui_forms::CursorKind::arrow).accepted() ||
        !(*services).set_pointer_capture(true, 9).accepted() ||
        !(*services).set_pointer_capture(false).accepted() ||
        !(*services).write_clipboard_text("M3c AppKit Ω").accepted()) {
        return false;
    }
    const gui_forms::HostClipboardTextResult clipboard =
        (*services).read_clipboard_text();
    if (!clipboard.status.accepted() || !clipboard.has_text ||
        clipboard.text_utf8 != "M3c AppKit Ω") {
        return false;
    }
    gui_forms::HostMessageDialogRequest message_request;
    message_request.title = "M3d message";
    message_request.message = "Native cancellation smoke";
    message_request.buttons = gui_forms::HostMessageButtons::ok_cancel;
    const gui_forms::HostDialogResult message = (*services).show_dialog(
        {101, "test.window", std::move(message_request)});
    gui_forms::HostOpenFileDialogRequest open_request;
    open_request.title = "M3d open";
    open_request.filters = {{"Text", {"txt"}}};
    const gui_forms::HostDialogResult open = (*services).show_dialog(
        {102, "test.window", std::move(open_request)});
    gui_forms::HostSaveFileDialogRequest save_request;
    save_request.title = "M3d save";
    save_request.suggested_name = "evidence";
    save_request.default_extension = "txt";
    const gui_forms::HostDialogResult save = (*services).show_dialog(
        {103, "test.window", std::move(save_request)});
    const gui_forms::HostDialogResult folder = (*services).show_dialog(
        {104, "test.window",
         gui_forms::HostFolderDialogRequest{"M3d folder", {}}});
    const gui_forms::HostDialogResult color = (*services).show_dialog(
        {105, "test.window",
         gui_forms::HostColorDialogRequest{"M3d color", 0x3366CCFFU, false}});
    if (!dialog_result_cancelled(message) || !dialog_result_cancelled(open) ||
        !dialog_result_cancelled(save) || !dialog_result_cancelled(folder) ||
        !dialog_result_cancelled(color) ||
        (*services).snapshot().dialog_requests != 5 ||
        (*services).snapshot().dialog_completions != 5 ||
        (*services).snapshot().dialog_cancellations != 5 ||
        (*services).snapshot().maximum_modal_depth != 1) {
        return false;
    }
    (*services).shutdown();
    if ((*services).query_monitors().status.error !=
        gui_forms::HostServiceError::after_shutdown) {
        return false;
    }
    std::cout << "appkit_services monitor_count=" << monitors.monitors.size()
              << " primary=" << monitors.monitors.front().id
              << " clipboard_generation=" << clipboard.generation
              << " snapshot=" << (*services).snapshot().to_json() << '\n';
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
    std::shared_ptr<FrameProbe> root = gui_forms::make_control<FrameProbe>(
        gui_forms::StableId("host.close.root"));
    std::shared_ptr<FrameProbe> surface = gui_forms::make_control<FrameProbe>(
        gui_forms::StableId("host.close.surface"));
    (*surface).set_visible(false);
    (*surface).set_requested_bounds({0.0, 0.0, 320.0, 180.0});
    (*root).add_child(surface);
    std::unique_ptr<gui_forms::Window> window = std::make_unique<gui_forms::Window>(root,
                                                      gui_forms::Size{320.0, 180.0});
    gui_forms::Window* const live_window = window.get();
    gui_forms::FrameRequestToken active = (*window).activate_surface(
        surface, std::chrono::milliseconds(10),
        gui_forms::FrameClock::now() + std::chrono::milliseconds(10));
    std::shared_ptr<RearmProbeState> rearm_state;
    const std::shared_ptr<NativeWindowProbeState> native_window_probe =
        std::make_shared<NativeWindowProbeState>();
    std::thread synchronous_worker;
    gui_forms::host::MacHostOptions options;
    options.title = "GUI.Forms Close Test";
    options.initial_size = {320.0, 180.0};
    options.minimum_size = {320.0, 180.0};
    options.titlebar_presentation =
        gui_forms::host::MacTitlebarPresentation::
            transparent_full_size_content;
    options.window_drag_region_id = "host.close.root";
    options.print_metrics_on_close = false;
    std::uint64_t close_requests = 0;
    std::string final_host_snapshot;
    options.close_request = CountCloseRequests(close_requests);
    options.final_snapshot = StoreFinalHostSnapshot(final_host_snapshot);
    options.host_ready = PrepareCloseTestHost(
        rearm_state, surface, live_window, active, synchronous_worker,
        native_window_probe);
    const int result = gui_forms::host::run_macos(std::move(window), std::move(options));
    if (synchronous_worker.joinable()) synchronous_worker.join();
    if (result != 0) {
        return result;
    }
    if (!rearm_state || active.connected() || (*rearm_state).replacement.connected()) {
        return 3;
    }
    if (!native_window_probe->found || !native_window_probe->moved ||
        !native_window_probe->resized || !native_window_probe->zoomed ||
        !native_window_probe->minimized || !native_window_probe->restored) {
        std::cerr << "native_window_probe found=" << native_window_probe->found
                  << " moved=" << native_window_probe->moved
                  << " resized=" << native_window_probe->resized
                  << " zoomed=" << native_window_probe->zoomed
                  << " minimized=" << native_window_probe->minimized
                  << " restored=" << native_window_probe->restored << '\n';
        return 9;
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
    if (final_host_snapshot.find(
            "\"titlebar_presentation\":\"transparent_full_size_content\"") ==
            std::string::npos ||
        final_host_snapshot.find("\"native_full_size_content\":true") ==
            std::string::npos ||
        final_host_snapshot.find("\"native_title_hidden\":true") ==
            std::string::npos ||
        final_host_snapshot.find("\"native_titlebar_transparent\":true") ==
            std::string::npos ||
        final_host_snapshot.find("\"native_close_button_present\":true") ==
            std::string::npos ||
        final_host_snapshot.find("\"native_minimize_button_present\":true") ==
            std::string::npos ||
        final_host_snapshot.find("\"native_zoom_button_present\":true") ==
            std::string::npos ||
        final_host_snapshot.find("\"native_system_title_present\":true") ==
            std::string::npos ||
        final_host_snapshot.find("\"window_drag_region_configured\":true") ==
            std::string::npos ||
        final_host_snapshot.find("\"window_drag_region_resolved\":true") ==
            std::string::npos ||
        final_host_snapshot.find("\"window_drag_region_count\":1") ==
            std::string::npos ||
        final_host_snapshot.find("\"window_drag_region_resolved_count\":1") ==
            std::string::npos) {
        return 8;
    }
    if ((*rearm_state).ticks_before.front() == 0 ||
        (*rearm_state).ticks_after.back() <= (*rearm_state).ticks_before.front() + 2U ||
        (*(*rearm_state).root).paints < 4U ||
        (*rearm_state).successful_cycles != (*rearm_state).ticks_before.size() ||
        (*rearm_state).quiescent_gap_failures != 0U ||
        !(*rearm_state).worker_required_invoke ||
        !(*rearm_state).worker_ran_on_ui_thread ||
        !(*rearm_state).nested_ran_on_ui_thread ||
        !(*rearm_state).synchronous_worker_required_invoke ||
        !(*rearm_state).synchronous_ran_on_ui_thread ||
        !(*rearm_state).synchronous_returned ||
        (*rearm_state).synchronous_fault != "native synchronous fault" ||
        (*rearm_state).dispatch_trace != "AB" ||
        (*rearm_state).worker_dispatch.state() !=
            gui_forms::DispatchOperationState::completed ||
        (*rearm_state).nested_dispatch.state() !=
            gui_forms::DispatchOperationState::completed) {
        std::cerr << "AppKit scheduled wake did not autonomously advance after "
                     "repeated active-surface replacement: before="
                  << (*rearm_state).ticks_before.front()
                  << " after=" << (*rearm_state).ticks_after.back()
                  << " paints=" << (*(*rearm_state).root).paints
                  << " successful_cycles=" << (*rearm_state).successful_cycles
                  << " quiescent_gap_failures="
                  << (*rearm_state).quiescent_gap_failures << '\n';
        for (std::size_t cycle = 0; cycle < (*rearm_state).ticks_before.size();
             ++cycle) {
            std::cerr << "  cycle=" << cycle
                      << " before=" << (*rearm_state).ticks_before[cycle]
                      << " rearm=" << (*rearm_state).ticks_at_rearm[cycle]
                      << " after=" << (*rearm_state).ticks_after[cycle] << '\n';
        }
        return 8;
    }
    std::cout << "macos_capabilities=" << capabilities.to_json()
              << " close_requests=" << close_requests << '\n';
    return 0;
}
