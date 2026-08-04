#include "headless_host.hpp"

#include <sstream>

namespace gui_forms::host {

HostCapabilities headless_capabilities() {
    return {HostCapabilities::current_protocol_version,
            "headless-reference",
            HostCapability::lifecycle |
                HostCapability::scale_notifications |
                HostCapability::occlusion |
                HostCapability::scheduled_wake |
                HostCapability::pointer_input |
                HostCapability::keyboard_input |
                HostCapability::text_composition |
                HostCapability::monitor_geometry |
                HostCapability::pointer_capture |
                HostCapability::cursor |
                HostCapability::clipboard |
                HostCapability::typed_drag_drop |
                HostCapability::dialogs};
}

HeadlessHostServices::HeadlessHostServices()
    : HostServices(headless_capabilities()),
      monitors_{{"headless.primary",
                 {0.0, 0.0, 1920.0, 1080.0},
                 {0.0, 0.0, 1920.0, 1040.0},
                 1.0,
                 true}} {}

HostMonitorResult HeadlessHostServices::query_monitors_impl() {
    return {{}, monitors_};
}

HostServiceStatus HeadlessHostServices::set_cursor_impl(CursorKind) {
    return {};
}

HostServiceStatus HeadlessHostServices::set_pointer_capture_impl(
    bool, std::uint64_t) {
    return {};
}

HostClipboardTextResult HeadlessHostServices::read_clipboard_text_impl() {
    return {{}, clipboard_text_, clipboard_generation_, clipboard_has_text_};
}

HostServiceStatus HeadlessHostServices::write_clipboard_text_impl(
    std::string_view text_utf8) {
    clipboard_text_.assign(text_utf8);
    clipboard_has_text_ = true;
    ++clipboard_generation_;
    return {};
}

void HeadlessHostServices::queue_dialog_result(HostDialogResult result) {
    dialog_results_.push_back(std::move(result));
}

void HeadlessHostServices::set_dialog_handler(DialogHandler handler) {
    dialog_handler_ = std::move(handler);
}

HostDialogResult HeadlessHostServices::show_dialog_impl(
    const HostDialogRequest& request) {
    HostDialogResult result;
    if (dialog_handler_) {
        result = dialog_handler_(request, *this);
    } else if (!dialog_results_.empty()) {
        result = std::move(dialog_results_.front());
        dialog_results_.pop_front();
    } else {
        result.status.error = HostServiceError::backend_failure;
        result.request_id = request.request_id;
    }
    std::ostringstream line;
    line << "dialog=" << host_dialog_kind_name(request.payload)
         << " request=" << request.request_id
         << " depth=" << snapshot().modal_depth
         << " error=" << host_service_error_name(result.status.error);
    if (result.status.accepted()) {
        line << " outcome=" << host_dialog_outcome_name(std::visit(
            [](const auto& value) { return value.outcome; }, result.payload));
        if (const auto* message =
                std::get_if<HostMessageDialogResult>(&result.payload)) {
            line << " choice=" << host_dialog_choice_name(message->choice);
        } else if (const auto* paths =
                       std::get_if<HostPathDialogResult>(&result.payload)) {
            line << " paths=" << paths->paths.size();
        }
    }
    dialog_trace_ += line.str() + '\n';
    return result;
}

void HeadlessHostServices::shutdown_impl() noexcept {
    clipboard_text_.clear();
    clipboard_has_text_ = false;
    dialog_results_.clear();
}

HeadlessHost::HeadlessHost(Window& window)
    : services_(), session_(window, headless_capabilities(), &services_) {
    observation_ = session_.observed().subscribe(
        [this](const HostEvent& event, const HostDispatchResult& result) {
            const HostSessionSnapshot snapshot = session_.snapshot();
            const HostServicesSnapshot services = services_.snapshot();
            std::ostringstream line;
            line << "seq=" << event.sequence
                 << " time_ns=" << event.timestamp_nanoseconds
                 << " event=" << host_event_name(event.payload)
                 << " accepted=" << (result.accepted() ? 1 : 0)
                 << " handled=" << (result.handled ? 1 : 0)
                 << " drag_effect=" << drag_effect_name(result.drag_effect)
                 << " close_allowed=" << (result.close_allowed ? 1 : 0)
                 << " error=" << host_dispatch_error_name(result.error)
                 << " attached=" << (snapshot.attached ? 1 : 0)
                 << " active=" << (snapshot.active ? 1 : 0)
                 << " occluded=" << (snapshot.occluded ? 1 : 0)
                 << " display_changes=" << snapshot.display_changes
                 << " monitors=" << snapshot.monitor_count
                 << " captured=" << (services.pointer_captured ? 1 : 0)
                 << " pointer_id=" << services.captured_pointer_id
                 << " modal_depth=" << snapshot.modal_depth
                 << " modal_suppressed=" << snapshot.modal_input_suppressions
                 << " drag_events=" << snapshot.drag_events
                 << " drag_drops=" << snapshot.drag_drops
                 << " closed=" << (snapshot.closed ? 1 : 0)
                 << " shutdown=" << (snapshot.shutdown ? 1 : 0) << '\n';
            trace_ += line.str();
        });
}

HostDispatchResult HeadlessHost::dispatch(HostEventPayload payload,
                                          std::uint64_t timestamp_nanoseconds) {
    HostEvent event;
    event.sequence = next_sequence_++;
    event.timestamp_nanoseconds = timestamp_nanoseconds;
    event.payload = std::move(payload);
    return session_.dispatch(std::move(event));
}

} // namespace gui_forms::host
