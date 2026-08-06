#include "gui_forms/host.hpp"

#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <array>
#include <sstream>
#include <type_traits>
#include <unordered_set>

namespace gui_forms {
namespace {

std::string json_escape(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    for (const char character : text) {
        switch (character) {
        case '\\': result += "\\\\"; break;
        case '"': result += "\\\""; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default: result += character; break;
        }
    }
    return result;
}

bool valid_size(Size size) noexcept {
    return std::isfinite(size.width) && std::isfinite(size.height) &&
           size.width > 0.0 && size.height > 0.0;
}

bool valid_scale(double scale) noexcept {
    return std::isfinite(scale) && scale > 0.0;
}

bool valid_utf8(std::string_view text) noexcept {
    const auto* bytes = reinterpret_cast<const unsigned char*>(text.data());
    std::size_t index = 0;
    while (index < text.size()) {
        const unsigned char lead = bytes[index];
        if (lead <= 0x7FU) {
            ++index;
            continue;
        }
        std::size_t length = 0;
        std::uint32_t codepoint = 0;
        if (lead >= 0xC2U && lead <= 0xDFU) {
            length = 2;
            codepoint = lead & 0x1FU;
        } else if (lead >= 0xE0U && lead <= 0xEFU) {
            length = 3;
            codepoint = lead & 0x0FU;
        } else if (lead >= 0xF0U && lead <= 0xF4U) {
            length = 4;
            codepoint = lead & 0x07U;
        } else {
            return false;
        }
        if (index + length > text.size()) {
            return false;
        }
        for (std::size_t offset = 1; offset < length; ++offset) {
            const unsigned char continuation = bytes[index + offset];
            if ((continuation & 0xC0U) != 0x80U) {
                return false;
            }
            codepoint = (codepoint << 6U) | (continuation & 0x3FU);
        }
        if ((length == 3 && codepoint < 0x800U) ||
            (length == 4 && codepoint < 0x10000U) ||
            (codepoint >= 0xD800U && codepoint <= 0xDFFFU) ||
            codepoint > 0x10FFFFU) {
            return false;
        }
        index += length;
    }
    return true;
}

bool valid_monitor(const HostMonitor& monitor) noexcept {
    const auto valid_rect = [](Rect rect) {
        return std::isfinite(rect.x) && std::isfinite(rect.y) &&
               valid_size({rect.width, rect.height});
    };
    return !monitor.id.empty() && valid_rect(monitor.frame) &&
           valid_rect(monitor.work_area) && valid_scale(monitor.scale);
}

bool valid_monitor_set(const std::vector<HostMonitor>& monitors) {
    if (monitors.empty() || monitors.size() > 64U) {
        return false;
    }
    std::unordered_set<std::string> identifiers;
    std::size_t primary_count = 0;
    for (const HostMonitor& monitor : monitors) {
        if (!valid_monitor(monitor) || !identifiers.insert(monitor.id).second) {
            return false;
        }
        primary_count += monitor.primary ? 1U : 0U;
    }
    return primary_count == 1U;
}

bool valid_drag_event(const DragEvent& event) noexcept {
    constexpr std::uint8_t known_effects =
        static_cast<std::uint8_t>(DragEffect::copy) |
        static_cast<std::uint8_t>(DragEffect::move) |
        static_cast<std::uint8_t>(DragEffect::link);
    const auto effects = static_cast<std::uint8_t>(event.allowed_effects);
    if (event.session_id == 0 || !std::isfinite(event.position.x) ||
        !std::isfinite(event.position.y) || (effects & ~known_effects) != 0 ||
        event.accepted_effect != DragEffect::none ||
        event.items.size() > DragLimits::maximum_items) {
        return false;
    }
    if (event.action != DragAction::leave &&
        (event.items.empty() || event.allowed_effects == DragEffect::none)) {
        return false;
    }
    std::size_t total = 0;
    const auto add_size = [&total](std::size_t value) {
        if (value > DragLimits::maximum_total_bytes - total) {
            return false;
        }
        total += value;
        return true;
    };
    for (const DragDataItem& item : event.items) {
        const bool valid = std::visit([&add_size](const auto& data) {
            using Data = std::decay_t<decltype(data)>;
            if constexpr (std::is_same_v<Data, DragTextData>) {
                return data.text_utf8.size() <= DragLimits::maximum_text_bytes &&
                       data.text_utf8.find('\0') == std::string::npos &&
                       valid_utf8(data.text_utf8) && add_size(data.text_utf8.size());
            } else if constexpr (std::is_same_v<Data, DragFileListData>) {
                if (data.paths_utf8.empty() ||
                    data.paths_utf8.size() > DragLimits::maximum_paths) {
                    return false;
                }
                for (const std::string& path : data.paths_utf8) {
                    if (path.empty() || path.size() > DragLimits::maximum_path_bytes ||
                        path.find('\0') != std::string::npos || !valid_utf8(path) ||
                        !add_size(path.size())) {
                        return false;
                    }
                }
                return true;
            } else if constexpr (std::is_same_v<Data, DragBinaryData>) {
                return !data.media_type.empty() &&
                       data.media_type.size() <= DragLimits::maximum_media_type_bytes &&
                       data.media_type.find('\0') == std::string::npos &&
                       valid_utf8(data.media_type) && add_size(data.bytes.size());
            }
            return false;
        }, item);
        if (!valid) {
            return false;
        }
    }
    return true;
}

bool valid_dialog_string(std::string_view text) noexcept {
    return text.size() <= HostServices::maximum_dialog_text_bytes &&
           text.find('\0') == std::string_view::npos && valid_utf8(text);
}

bool valid_dialog_filter(const HostFileDialogFilter& filter) noexcept {
    if (filter.extensions.empty() ||
        filter.extensions.size() > HostServices::maximum_dialog_extensions ||
        !valid_dialog_string(filter.label)) {
        return false;
    }
    return std::all_of(filter.extensions.begin(), filter.extensions.end(),
                       [](const std::string& extension) {
                           return !extension.empty() && valid_dialog_string(extension);
                       });
}

bool choice_allowed(HostMessageButtons buttons, HostDialogChoice choice) noexcept {
    switch (buttons) {
    case HostMessageButtons::ok:
        return choice == HostDialogChoice::ok;
    case HostMessageButtons::ok_cancel:
        return choice == HostDialogChoice::ok || choice == HostDialogChoice::cancel;
    case HostMessageButtons::yes_no:
        return choice == HostDialogChoice::yes || choice == HostDialogChoice::no;
    case HostMessageButtons::yes_no_cancel:
        return choice == HostDialogChoice::yes || choice == HostDialogChoice::no ||
               choice == HostDialogChoice::cancel;
    case HostMessageButtons::retry_cancel:
        return choice == HostDialogChoice::retry || choice == HostDialogChoice::cancel;
    }
    return false;
}

bool valid_dialog_request(const HostDialogRequest& request) noexcept {
    if (request.request_id == 0 || !valid_dialog_string(request.owner_id)) {
        return false;
    }
    return std::visit([](const auto& payload) {
        using Payload = std::decay_t<decltype(payload)>;
        if constexpr (std::is_same_v<Payload, HostMessageDialogRequest>) {
            return valid_dialog_string(payload.title) &&
                   valid_dialog_string(payload.message) &&
                   choice_allowed(payload.buttons, payload.default_choice);
        } else if constexpr (std::is_same_v<Payload, HostOpenFileDialogRequest>) {
            return valid_dialog_string(payload.title) &&
                   valid_dialog_string(payload.initial_directory) &&
                   valid_dialog_string(payload.suggested_name) &&
                   payload.filters.size() <= HostServices::maximum_dialog_filters &&
                   std::all_of(payload.filters.begin(), payload.filters.end(),
                               valid_dialog_filter);
        } else if constexpr (std::is_same_v<Payload, HostSaveFileDialogRequest>) {
            return valid_dialog_string(payload.title) &&
                   valid_dialog_string(payload.initial_directory) &&
                   valid_dialog_string(payload.suggested_name) &&
                   valid_dialog_string(payload.default_extension) &&
                   payload.filters.size() <= HostServices::maximum_dialog_filters &&
                   std::all_of(payload.filters.begin(), payload.filters.end(),
                               valid_dialog_filter);
        } else if constexpr (std::is_same_v<Payload, HostFolderDialogRequest>) {
            return valid_dialog_string(payload.title) &&
                   valid_dialog_string(payload.initial_directory);
        } else if constexpr (std::is_same_v<Payload, HostColorDialogRequest>) {
            return valid_dialog_string(payload.title);
        }
        return false;
    }, request.payload);
}

HostDialogOutcome dialog_outcome(const HostDialogResultPayload& payload) noexcept {
    return std::visit([](const auto& value) { return value.outcome; }, payload);
}

bool valid_dialog_result(const HostDialogRequest& request,
                         const HostDialogResult& result) noexcept {
    if (!result.status.accepted() || result.request_id != request.request_id) {
        return false;
    }
    return std::visit([&result](const auto& request_payload) {
        using Request = std::decay_t<decltype(request_payload)>;
        if constexpr (std::is_same_v<Request, HostMessageDialogRequest>) {
            const auto* value = std::get_if<HostMessageDialogResult>(&result.payload);
            if (value == nullptr) {
                return false;
            }
            return value->outcome == HostDialogOutcome::accepted
                ? choice_allowed(request_payload.buttons, value->choice)
                : value->choice == HostDialogChoice::none ||
                      value->choice == HostDialogChoice::cancel;
        } else if constexpr (std::is_same_v<Request, HostOpenFileDialogRequest> ||
                             std::is_same_v<Request, HostSaveFileDialogRequest> ||
                             std::is_same_v<Request, HostFolderDialogRequest>) {
            const auto* value = std::get_if<HostPathDialogResult>(&result.payload);
            if (value == nullptr || value->paths.size() >
                    HostServices::maximum_dialog_paths) {
                return false;
            }
            if (value->outcome == HostDialogOutcome::cancelled) {
                return value->paths.empty();
            }
            bool multiple = false;
            if constexpr (std::is_same_v<Request, HostOpenFileDialogRequest>) {
                multiple = request_payload.allow_multiple;
            }
            if (value->paths.empty() || (!multiple && value->paths.size() != 1U)) {
                return false;
            }
            return std::all_of(value->paths.begin(), value->paths.end(),
                               valid_dialog_string);
        } else if constexpr (std::is_same_v<Request, HostColorDialogRequest>) {
            return std::holds_alternative<HostColorDialogResult>(result.payload);
        }
        return false;
    }, request.payload);
}

} // namespace

std::string HostCapabilities::to_json() const {
    constexpr std::array<std::pair<HostCapability, const char*>, 18> names{{
        {HostCapability::lifecycle, "lifecycle"},
        {HostCapability::scale_notifications, "scale_notifications"},
        {HostCapability::monitor_geometry, "monitor_geometry"},
        {HostCapability::occlusion, "occlusion"},
        {HostCapability::scheduled_wake, "scheduled_wake"},
        {HostCapability::pointer_input, "pointer_input"},
        {HostCapability::keyboard_input, "keyboard_input"},
        {HostCapability::text_composition, "text_composition"},
        {HostCapability::pointer_capture, "pointer_capture"},
        {HostCapability::cursor, "cursor"},
        {HostCapability::clipboard, "clipboard"},
        {HostCapability::typed_drag_destination, "typed_drag_destination"},
        {HostCapability::dialogs, "dialogs"},
        {HostCapability::menus, "menus"},
        {HostCapability::font_discovery, "font_discovery"},
        {HostCapability::accessibility, "accessibility"},
        {HostCapability::typed_drag_source, "typed_drag_source"},
        {HostCapability::sound_cues, "sound_cues"},
    }};
    std::ostringstream output;
    output << "{\"protocol_version\":" << protocol_version
           << ",\"platform\":\"" << json_escape(platform)
           << "\",\"available_bits\":"
           << static_cast<std::uint64_t>(available) << ",\"capabilities\":[";
    bool first = true;
    for (const auto& [capability, name] : names) {
        if (!supports(capability)) {
            continue;
        }
        output << (first ? "\"" : ",\"") << name << '\"';
        first = false;
    }
    output << "]}";
    return output.str();
}

std::string HostSessionSnapshot::to_json() const {
    std::ostringstream output;
    output << "{\"capabilities\":" << capabilities.to_json()
           << ",\"phase\":\"" << host_lifecycle_phase_name(phase) << '\"'
           << ",\"last_sequence\":" << last_sequence
           << ",\"events_accepted\":" << events_accepted
           << ",\"events_rejected\":" << events_rejected
           << ",\"close_requests\":" << close_requests
           << ",\"close_cancellations\":" << close_cancellations
           << ",\"display_changes\":" << display_changes
           << ",\"monitor_count\":" << monitor_count
           << ",\"modal_transitions\":" << modal_transitions
           << ",\"modal_input_suppressions\":" << modal_input_suppressions
           << ",\"drag_events\":" << drag_events
           << ",\"drag_drops\":" << drag_drops
           << ",\"modal_depth\":" << modal_depth
           << ",\"attached\":" << (attached ? "true" : "false")
           << ",\"active\":" << (active ? "true" : "false")
           << ",\"occluded\":" << (occluded ? "true" : "false")
           << ",\"closed\":" << (closed ? "true" : "false")
           << ",\"shutdown\":" << (shutdown ? "true" : "false") << '}';
    return output.str();
}

std::string HostServicesSnapshot::to_json() const {
    std::ostringstream output;
    output << "{\"capabilities\":" << capabilities.to_json()
           << ",\"cursor\":\"" << cursor_kind_name(cursor) << '\"'
           << ",\"pointer_captured\":" << (pointer_captured ? "true" : "false")
           << ",\"captured_pointer_id\":" << captured_pointer_id
           << ",\"monitor_queries\":" << monitor_queries
           << ",\"cursor_updates\":" << cursor_updates
           << ",\"pointer_capture_updates\":" << pointer_capture_updates
           << ",\"clipboard_reads\":" << clipboard_reads
           << ",\"clipboard_writes\":" << clipboard_writes
           << ",\"clipboard_generation\":" << clipboard_generation
           << ",\"dialog_requests\":" << dialog_requests
           << ",\"dialog_completions\":" << dialog_completions
           << ",\"dialog_cancellations\":" << dialog_cancellations
           << ",\"sound_requests\":" << sound_requests
           << ",\"sound_playbacks\":" << sound_playbacks
           << ",\"sound_coalesced\":" << sound_coalesced
           << ",\"sound_muted\":" << sound_muted
           << ",\"modal_depth\":" << modal_depth
           << ",\"maximum_modal_depth\":" << maximum_modal_depth
           << ",\"rejected_requests\":" << rejected_requests
           << ",\"shutdown\":" << (shutdown ? "true" : "false") << '}';
    return output.str();
}

HostServices::HostServices(HostCapabilities capabilities)
    : ui_thread_(std::this_thread::get_id()) {
    snapshot_.capabilities = std::move(capabilities);
}

HostServiceStatus HostServices::validate_request(HostCapability capability) noexcept {
    HostServiceStatus status;
    if (std::this_thread::get_id() != ui_thread_) {
        status.error = HostServiceError::wrong_thread;
        return status;
    } else if (snapshot_.shutdown) {
        status.error = HostServiceError::after_shutdown;
    } else if (!snapshot_.capabilities.supports(capability)) {
        status.error = HostServiceError::unsupported;
    }
    if (!status.accepted()) {
        ++snapshot_.rejected_requests;
    }
    return status;
}

HostMonitorResult HostServices::query_monitors() {
    HostMonitorResult result;
    result.status = validate_request(HostCapability::monitor_geometry);
    if (!result.status.accepted()) {
        return result;
    }
    ++snapshot_.monitor_queries;
    result = query_monitors_impl();
    if (result.status.accepted() && !valid_monitor_set(result.monitors)) {
        result.status.error = HostServiceError::backend_failure;
        result.monitors.clear();
    }
    if (!result.status.accepted()) {
        ++snapshot_.rejected_requests;
    }
    return result;
}

HostServiceStatus HostServices::set_cursor(CursorKind cursor) {
    HostServiceStatus status = validate_request(HostCapability::cursor);
    if (!status.accepted() || snapshot_.cursor == cursor) {
        return status;
    }
    status = set_cursor_impl(cursor);
    if (status.accepted()) {
        snapshot_.cursor = cursor;
        ++snapshot_.cursor_updates;
    } else {
        ++snapshot_.rejected_requests;
    }
    return status;
}

HostServiceStatus HostServices::set_pointer_capture(bool captured,
                                                    std::uint64_t pointer_id) {
    HostServiceStatus status = validate_request(HostCapability::pointer_capture);
    if (!status.accepted()) {
        return status;
    }
    if (captured && pointer_id == 0) {
        status.error = HostServiceError::invalid_argument;
        ++snapshot_.rejected_requests;
        return status;
    }
    if (snapshot_.pointer_captured == captured &&
        (!captured || snapshot_.captured_pointer_id == pointer_id)) {
        return status;
    }
    status = set_pointer_capture_impl(captured, captured ? pointer_id : 0);
    if (status.accepted()) {
        snapshot_.pointer_captured = captured;
        snapshot_.captured_pointer_id = captured ? pointer_id : 0;
        ++snapshot_.pointer_capture_updates;
    } else {
        ++snapshot_.rejected_requests;
    }
    return status;
}

HostClipboardTextResult HostServices::read_clipboard_text() {
    HostClipboardTextResult result;
    result.status = validate_request(HostCapability::clipboard);
    if (!result.status.accepted()) {
        return result;
    }
    ++snapshot_.clipboard_reads;
    result = read_clipboard_text_impl();
    if (result.status.accepted() && result.has_text &&
        (result.text_utf8.size() > maximum_clipboard_text_bytes ||
         !valid_utf8(result.text_utf8))) {
        result.status.error = result.text_utf8.size() > maximum_clipboard_text_bytes
                                  ? HostServiceError::too_large
                                  : HostServiceError::invalid_utf8;
        result.text_utf8.clear();
        result.has_text = false;
    }
    if (result.status.accepted()) {
        snapshot_.clipboard_generation = result.generation;
    } else {
        ++snapshot_.rejected_requests;
    }
    return result;
}

HostServiceStatus HostServices::write_clipboard_text(std::string_view text_utf8) {
    HostServiceStatus status = validate_request(HostCapability::clipboard);
    if (!status.accepted()) {
        return status;
    }
    if (text_utf8.size() > maximum_clipboard_text_bytes) {
        status.error = HostServiceError::too_large;
    } else if (!valid_utf8(text_utf8)) {
        status.error = HostServiceError::invalid_utf8;
    }
    if (!status.accepted()) {
        ++snapshot_.rejected_requests;
        return status;
    }
    status = write_clipboard_text_impl(text_utf8);
    if (status.accepted()) {
        ++snapshot_.clipboard_writes;
        ++snapshot_.clipboard_generation;
    } else {
        ++snapshot_.rejected_requests;
    }
    return status;
}

HostDialogResult HostServices::show_dialog(const HostDialogRequest& request) {
    HostDialogResult result;
    result.request_id = request.request_id;
    result.status = validate_request(HostCapability::dialogs);
    if (!result.status.accepted()) {
        return result;
    }
    if (!valid_dialog_request(request) ||
        std::find(modal_stack_.begin(), modal_stack_.end(), request.request_id) !=
            modal_stack_.end()) {
        result.status.error = HostServiceError::invalid_argument;
        ++snapshot_.rejected_requests;
        return result;
    }
    if (modal_stack_.size() >= maximum_nested_modal_depth) {
        result.status.error = HostServiceError::modal_limit;
        ++snapshot_.rejected_requests;
        return result;
    }

    ++snapshot_.dialog_requests;
    modal_stack_.push_back(request.request_id);
    snapshot_.modal_depth = static_cast<std::uint32_t>(modal_stack_.size());
    snapshot_.maximum_modal_depth =
        std::max(snapshot_.maximum_modal_depth, snapshot_.modal_depth);
    modal_changed_.emit(HostModalTransition{request.request_id,
                                            snapshot_.modal_depth, true});
    try {
        result = show_dialog_impl(request);
    } catch (...) {
        result = {{HostServiceError::backend_failure}, request.request_id,
                  HostMessageDialogResult{}};
    }

    modal_stack_.pop_back();
    snapshot_.modal_depth = static_cast<std::uint32_t>(modal_stack_.size());
    modal_changed_.emit(HostModalTransition{request.request_id,
                                            snapshot_.modal_depth, false});

    if (snapshot_.shutdown) {
        result.status.error = HostServiceError::after_shutdown;
        result.request_id = request.request_id;
    } else if (result.status.accepted() && !valid_dialog_result(request, result)) {
        result.status.error = HostServiceError::backend_failure;
        result.request_id = request.request_id;
    }
    if (result.status.accepted()) {
        ++snapshot_.dialog_completions;
        snapshot_.dialog_cancellations +=
            dialog_outcome(result.payload) == HostDialogOutcome::cancelled ? 1U : 0U;
    } else {
        ++snapshot_.rejected_requests;
    }
    return result;
}

HostServiceStatus HostServices::play_sound_cue(
    const HostSoundCueRequest& request) {
    HostServiceStatus status = validate_request(HostCapability::sound_cues);
    if (!status.accepted()) return status;
    ++snapshot_.sound_requests;
    if (!std::isfinite(request.gain) || request.gain < 0.0 ||
        request.gain > 1.0 || request.timestamp_nanoseconds == 0U ||
        (last_sound_timestamp_ != 0U &&
         request.timestamp_nanoseconds < last_sound_timestamp_)) {
        status.error = HostServiceError::invalid_argument;
        ++snapshot_.rejected_requests;
        return status;
    }
    constexpr std::uint64_t coalescing_window_nanoseconds = 50'000'000U;
    const bool coalesced = last_sound_cue_ == request.cue &&
        last_sound_timestamp_ != 0U &&
        request.timestamp_nanoseconds - last_sound_timestamp_ <=
            coalescing_window_nanoseconds;
    last_sound_cue_ = request.cue;
    last_sound_timestamp_ = request.timestamp_nanoseconds;
    if (request.gain == 0.0) {
        ++snapshot_.sound_muted;
        return status;
    }
    if (coalesced) {
        ++snapshot_.sound_coalesced;
        return status;
    }
    try {
        status = play_sound_cue_impl(request);
    } catch (...) {
        status.error = HostServiceError::backend_failure;
    }
    if (status.accepted()) ++snapshot_.sound_playbacks;
    else ++snapshot_.rejected_requests;
    return status;
}

void HostServices::shutdown() noexcept {
    if (snapshot_.shutdown || std::this_thread::get_id() != ui_thread_) {
        return;
    }
    snapshot_.shutdown = true;
    shutdown_impl();
}

HostServicesSnapshot HostServices::snapshot() const {
    return snapshot_;
}

HostSession::HostSession(Window& window,
                         HostCapabilities capabilities,
                         HostServices* services)
    : window_(&window), services_(services), ui_thread_(std::this_thread::get_id()) {
    snapshot_.capabilities = std::move(capabilities);
    if (services_ != nullptr) {
        if (window.host_services_ != nullptr && window.host_services_ != services_) {
            throw std::logic_error("GUI.Forms Window already has an active host service seam");
        }
        window.host_services_ = services_;
        capture_observation_ = window.pointer_capture_changed().subscribe(
            [this](const PointerCaptureChange& change) {
                if (!snapshot_.shutdown && services_ != nullptr) {
                    static_cast<void>(services_->set_pointer_capture(
                        change.captured, change.pointer_id));
                }
            });
        modal_observation_ = services_->modal_changed().subscribe(
            [this](const HostModalTransition& transition) {
                if (snapshot_.shutdown) {
                    return;
                }
                snapshot_.modal_depth = transition.depth;
                ++snapshot_.modal_transitions;
                if (transition.entering && window_ != nullptr) {
                    window_->release_pointer();
                }
            });
    }
}

HostSession::~HostSession() {
    shutdown();
}

HostDispatchResult HostSession::dispatch(HostEvent event) {
    HostDispatchResult result;
    if (std::this_thread::get_id() != ui_thread_) {
        result.error = HostDispatchError::wrong_thread;
        return result;
    } else if (event.sequence == 0) {
        result.error = HostDispatchError::sequence_zero;
    } else if (event.sequence <= snapshot_.last_sequence) {
        result.error = HostDispatchError::non_monotonic_sequence;
    } else if (snapshot_.shutdown) {
        result.error = HostDispatchError::after_shutdown;
    }
    if (result.accepted()) {
        const bool attach = std::holds_alternative<HostAttachEvent>(event.payload);
        const bool shutdown = std::holds_alternative<HostShutdownEvent>(event.payload);
        const bool closed = std::holds_alternative<HostClosedEvent>(event.payload);
        const bool passive_terminal = std::visit([](const auto& payload) {
            using Payload = std::decay_t<decltype(payload)>;
            if constexpr (std::is_same_v<Payload, HostActivationEvent>) {
                return !payload.active;
            } else if constexpr (std::is_same_v<Payload, HostOcclusionEvent>) {
                return payload.occluded;
            }
            return false;
        }, event.payload);
        switch (snapshot_.phase) {
        case HostLifecyclePhase::constructed:
            if (!attach && !shutdown) result.error = HostDispatchError::invalid_lifecycle;
            break;
        case HostLifecyclePhase::attached:
            if (attach) result.error = HostDispatchError::invalid_lifecycle;
            break;
        case HostLifecyclePhase::close_authorized:
            if (!closed && !shutdown && !passive_terminal) {
                result.error = HostDispatchError::invalid_lifecycle;
            }
            break;
        case HostLifecyclePhase::closed:
            if (!shutdown) result.error = HostDispatchError::invalid_lifecycle;
            break;
        case HostLifecyclePhase::shutdown:
            result.error = HostDispatchError::after_shutdown;
            break;
        }
    }
    if (!result.accepted()) {
        ++snapshot_.events_rejected;
        observed_.emit(event, result);
        return result;
    }

    const bool invalid_geometry = std::visit(
        [](const auto& payload) {
            using Payload = std::decay_t<decltype(payload)>;
            if constexpr (std::is_same_v<Payload, HostAttachEvent>) {
                return !valid_size(payload.client_size) || !valid_scale(payload.scale);
            } else if constexpr (std::is_same_v<Payload, HostResizeEvent>) {
                return !valid_size(payload.client_size);
            } else if constexpr (std::is_same_v<Payload, HostScaleEvent>) {
                return !valid_scale(payload.scale);
            } else if constexpr (std::is_same_v<Payload, HostDisplayEvent>) {
                return !valid_monitor_set(payload.monitors);
            }
            return false;
        }, event.payload);
    if (invalid_geometry) {
        result.error = HostDispatchError::invalid_geometry;
        ++snapshot_.events_rejected;
        observed_.emit(event, result);
        return result;
    }
    if (const auto* drag = std::get_if<DragEvent>(&event.payload);
        drag != nullptr && !valid_drag_event(*drag)) {
        result.error = HostDispatchError::invalid_payload;
        ++snapshot_.events_rejected;
        observed_.emit(event, result);
        return result;
    }

    snapshot_.last_sequence = event.sequence;
    ++snapshot_.events_accepted;
    const bool modal_input = snapshot_.modal_depth != 0 &&
        (std::holds_alternative<PointerEvent>(event.payload) ||
         std::holds_alternative<KeyEvent>(event.payload) ||
         std::holds_alternative<TextInputEvent>(event.payload) ||
         std::holds_alternative<DragEvent>(event.payload));
    if (modal_input) {
        ++snapshot_.modal_input_suppressions;
        observed_.emit(event, result);
        return result;
    }
    std::visit(
        [this, &result, &event](auto& payload) {
            using Payload = std::decay_t<decltype(payload)>;
            if constexpr (std::is_same_v<Payload, HostAttachEvent>) {
                UpdateScope update = window_->begin_update();
                window_->resize(payload.client_size);
                window_->set_scale(payload.scale);
                update.close();
                snapshot_.attached = true;
                snapshot_.phase = HostLifecyclePhase::attached;
            } else if constexpr (std::is_same_v<Payload, HostResizeEvent>) {
                window_->resize(payload.client_size);
            } else if constexpr (std::is_same_v<Payload, HostScaleEvent>) {
                window_->set_scale(payload.scale);
            } else if constexpr (std::is_same_v<Payload, HostActivationEvent>) {
                snapshot_.active = payload.active;
            } else if constexpr (std::is_same_v<Payload, HostOcclusionEvent>) {
                snapshot_.occluded = payload.occluded;
                window_->set_occluded(
                    payload.occluded,
                    FrameTime{std::chrono::nanoseconds(event.timestamp_nanoseconds)});
            } else if constexpr (std::is_same_v<Payload, HostDisplayEvent>) {
                ++snapshot_.display_changes;
                snapshot_.monitor_count = payload.monitors.size();
            } else if constexpr (std::is_same_v<Payload, DragEvent>) {
                ++snapshot_.drag_events;
                snapshot_.drag_drops += payload.action == DragAction::drop ? 1U : 0U;
                const DragDispatchResult drag = window_->dispatch_drag(std::move(payload));
                result.handled = drag.handled;
                result.drag_effect = drag.accepted_effect;
            } else if constexpr (std::is_same_v<Payload, PointerEvent>) {
                result.handled = window_->dispatch_pointer(std::move(payload));
            } else if constexpr (std::is_same_v<Payload, KeyEvent>) {
                result.handled = window_->dispatch_key(std::move(payload));
            } else if constexpr (std::is_same_v<Payload, TextInputEvent>) {
                result.handled = window_->dispatch_text(std::move(payload));
            } else if constexpr (std::is_same_v<Payload, HostCloseRequest>) {
                ++snapshot_.close_requests;
                closing_.emit(payload);
                result.close_allowed = !payload.cancel;
                snapshot_.close_cancellations += payload.cancel ? 1U : 0U;
                if (!payload.cancel) {
                    snapshot_.phase = HostLifecyclePhase::close_authorized;
                }
            } else if constexpr (std::is_same_v<Payload, HostClosedEvent>) {
                snapshot_.phase = HostLifecyclePhase::closed;
                snapshot_.attached = false;
                snapshot_.closed = true;
                snapshot_.active = false;
                snapshot_.occluded = false;
                window_->shutdown_dispatcher();
                window_->cancel_frame_requests();
                window_->release_pointer();
                window_->cancel_drag();
            } else if constexpr (std::is_same_v<Payload, HostShutdownEvent>) {
                shutdown();
            }
        }, event.payload);
    observed_.emit(event, result);
    return result;
}

void HostSession::shutdown() noexcept {
    if (snapshot_.shutdown || std::this_thread::get_id() != ui_thread_) {
        return;
    }
    snapshot_.active = false;
    snapshot_.attached = false;
    snapshot_.occluded = false;
    if (window_ != nullptr) {
        window_->shutdown_dispatcher();
        window_->cancel_frame_requests();
        window_->release_pointer();
        window_->cancel_drag();
        if (window_->host_services_ == services_) window_->host_services_ = nullptr;
    }
    snapshot_.shutdown = true;
    snapshot_.phase = HostLifecyclePhase::shutdown;
}

HostSessionSnapshot HostSession::snapshot() const {
    return snapshot_;
}

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
