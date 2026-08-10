#include "gui_forms/host.hpp"

#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <array>
#include <sstream>
#include <unordered_set>

namespace gui_forms {
namespace {

struct HostEventNameVisitor final {
    const char* operator()(const HostAttachEvent&) const noexcept { return "attach"; }
    const char* operator()(const HostResizeEvent&) const noexcept { return "resize"; }
    const char* operator()(const HostScaleEvent&) const noexcept { return "scale"; }
    const char* operator()(const HostActivationEvent&) const noexcept { return "activation"; }
    const char* operator()(const HostOcclusionEvent&) const noexcept { return "occlusion"; }
    const char* operator()(const HostDisplayEvent&) const noexcept { return "display"; }
    const char* operator()(const DragEvent&) const noexcept { return "drag"; }
    const char* operator()(const PointerEvent&) const noexcept { return "pointer"; }
    const char* operator()(const KeyEvent&) const noexcept { return "key"; }
    const char* operator()(const TextInputEvent&) const noexcept { return "text"; }
    const char* operator()(const HostCloseRequest&) const noexcept { return "close_request"; }
    const char* operator()(const HostClosedEvent&) const noexcept { return "closed"; }
    const char* operator()(const HostShutdownEvent&) const noexcept { return "shutdown"; }
};

struct HostDialogKindVisitor final {
    const char* operator()(const HostMessageDialogRequest&) const noexcept {
        return "message";
    }
    const char* operator()(const HostOpenFileDialogRequest&) const noexcept {
        return "open_file";
    }
    const char* operator()(const HostSaveFileDialogRequest&) const noexcept {
        return "save_file";
    }
    const char* operator()(const HostFolderDialogRequest&) const noexcept {
        return "select_folder";
    }
    const char* operator()(const HostColorDialogRequest&) const noexcept {
        return "color";
    }
};

} // namespace

const char* host_dispatch_error_name(HostDispatchError error) noexcept {
    switch (error) {
    case HostDispatchError::none: return "none";
    case HostDispatchError::sequence_zero: return "sequence_zero";
    case HostDispatchError::non_monotonic_sequence: return "non_monotonic_sequence";
    case HostDispatchError::wrong_thread: return "wrong_thread";
    case HostDispatchError::after_shutdown: return "after_shutdown";
    case HostDispatchError::invalid_lifecycle: return "invalid_lifecycle";
    case HostDispatchError::invalid_geometry: return "invalid_geometry";
    case HostDispatchError::invalid_payload: return "invalid_payload";
    case HostDispatchError::callback_fault: return "callback_fault";
    }
    return "unknown";
}

const char* host_lifecycle_phase_name(HostLifecyclePhase phase) noexcept {
    switch (phase) {
    case HostLifecyclePhase::constructed: return "constructed";
    case HostLifecyclePhase::attached: return "attached";
    case HostLifecyclePhase::close_authorized: return "close_authorized";
    case HostLifecyclePhase::closed: return "closed";
    case HostLifecyclePhase::shutdown: return "shutdown";
    }
    return "unknown";
}

const char* host_event_name(const HostEventPayload& payload) noexcept {
    return std::visit(HostEventNameVisitor{}, payload);
}

const char* host_service_error_name(HostServiceError error) noexcept {
    switch (error) {
    case HostServiceError::none: return "none";
    case HostServiceError::unsupported: return "unsupported";
    case HostServiceError::wrong_thread: return "wrong_thread";
    case HostServiceError::invalid_argument: return "invalid_argument";
    case HostServiceError::invalid_utf8: return "invalid_utf8";
    case HostServiceError::too_large: return "too_large";
    case HostServiceError::modal_limit: return "modal_limit";
    case HostServiceError::backend_failure: return "backend_failure";
    case HostServiceError::after_shutdown: return "after_shutdown";
    }
    return "unknown";
}

const char* host_sound_cue_name(HostSoundCue cue) noexcept {
    switch (cue) {
    case HostSoundCue::notification: return "notification";
    case HostSoundCue::success: return "success";
    case HostSoundCue::warning: return "warning";
    case HostSoundCue::error: return "error";
    case HostSoundCue::operation_complete: return "operation_complete";
    }
    return "unknown";
}

const char* cursor_kind_name(CursorKind cursor) noexcept {
    switch (cursor) {
    case CursorKind::arrow: return "arrow";
    case CursorKind::text: return "text";
    case CursorKind::hand: return "hand";
    case CursorKind::crosshair: return "crosshair";
    case CursorKind::resize_horizontal: return "resize_horizontal";
    case CursorKind::resize_vertical: return "resize_vertical";
    case CursorKind::wait: return "wait";
    case CursorKind::forbidden: return "forbidden";
    }
    return "unknown";
}

const char* drag_effect_name(DragEffect effect) noexcept {
    switch (effect) {
    case DragEffect::none: return "none";
    case DragEffect::copy: return "copy";
    case DragEffect::move: return "move";
    case DragEffect::link: return "link";
    }
    return "invalid";
}

const char* host_dialog_kind_name(const HostDialogRequestPayload& payload) noexcept {
    return std::visit(HostDialogKindVisitor{}, payload);
}

const char* host_dialog_outcome_name(HostDialogOutcome outcome) noexcept {
    return outcome == HostDialogOutcome::accepted ? "accepted" : "cancelled";
}

const char* host_dialog_choice_name(HostDialogChoice choice) noexcept {
    switch (choice) {
    case HostDialogChoice::none: return "none";
    case HostDialogChoice::ok: return "ok";
    case HostDialogChoice::cancel: return "cancel";
    case HostDialogChoice::yes: return "yes";
    case HostDialogChoice::no: return "no";
    case HostDialogChoice::retry: return "retry";
    }
    return "unknown";
}

} // namespace gui_forms
