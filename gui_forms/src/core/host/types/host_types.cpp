#include "gui_forms/host.hpp"

#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <array>
#include <sstream>
#include <type_traits>
#include <unordered_set>

namespace gui_forms {
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
    return std::visit([](const auto& value) -> const char* {
        using Payload = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<Payload, HostAttachEvent>) return "attach";
        if constexpr (std::is_same_v<Payload, HostResizeEvent>) return "resize";
        if constexpr (std::is_same_v<Payload, HostScaleEvent>) return "scale";
        if constexpr (std::is_same_v<Payload, HostActivationEvent>) return "activation";
        if constexpr (std::is_same_v<Payload, HostOcclusionEvent>) return "occlusion";
        if constexpr (std::is_same_v<Payload, HostDisplayEvent>) return "display";
        if constexpr (std::is_same_v<Payload, DragEvent>) return "drag";
        if constexpr (std::is_same_v<Payload, PointerEvent>) return "pointer";
        if constexpr (std::is_same_v<Payload, KeyEvent>) return "key";
        if constexpr (std::is_same_v<Payload, TextInputEvent>) return "text";
        if constexpr (std::is_same_v<Payload, HostCloseRequest>) return "close_request";
        if constexpr (std::is_same_v<Payload, HostClosedEvent>) return "closed";
        if constexpr (std::is_same_v<Payload, HostShutdownEvent>) return "shutdown";
        return "unknown";
    }, payload);
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
    return std::visit([](const auto& value) -> const char* {
        using Payload = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<Payload, HostMessageDialogRequest>) {
            return "message";
        } else if constexpr (std::is_same_v<Payload, HostOpenFileDialogRequest>) {
            return "open_file";
        } else if constexpr (std::is_same_v<Payload, HostSaveFileDialogRequest>) {
            return "save_file";
        } else if constexpr (std::is_same_v<Payload, HostFolderDialogRequest>) {
            return "select_folder";
        } else if constexpr (std::is_same_v<Payload, HostColorDialogRequest>) {
            return "color";
        }
        return "unknown";
    }, payload);
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
