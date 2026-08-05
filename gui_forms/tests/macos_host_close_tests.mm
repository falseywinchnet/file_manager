#import <AppKit/AppKit.h>

#include "macos_host.hpp"

#include "gui_forms/gui_forms.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>

namespace {

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
    auto root = gui_forms::make_control<gui_forms::Control>(
        gui_forms::StableId("host.close.root"));
    auto window = std::make_unique<gui_forms::Window>(root,
                                                      gui_forms::Size{320.0, 180.0});
    auto active = window->activate_surface(
        root, std::chrono::milliseconds(10),
        gui_forms::FrameClock::now() + std::chrono::milliseconds(10));
    gui_forms::host::MacHostOptions options;
    options.title = "GUI.Forms Close Test";
    options.initial_size = {320.0, 180.0};
    options.minimum_size = {320.0, 180.0};
    options.print_metrics_on_close = false;
    options.close_after_launch_for_testing = true;
    options.close_attempts_for_testing = 2;
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
    const int result = gui_forms::host::run_macos(std::move(window), std::move(options));
    if (result != 0) {
        return result;
    }
    if (active.connected()) {
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
    std::cout << "macos_capabilities=" << capabilities.to_json()
              << " close_requests=" << close_requests << '\n';
    return 0;
}
