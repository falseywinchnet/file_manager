#pragma once

#include "gui_forms/event.hpp"
#include "gui_forms/host/image/host_image.hpp"
#include "gui_forms/events.hpp"
#include "gui_forms/host/types/host_capabilities/host_capabilities.hpp"
#include "gui_forms/host/types/host_lifecycle_phase/host_lifecycle_phase.hpp"
#include "gui_forms/host/types/host_services_snapshot/host_services_snapshot.hpp"
#include "gui_forms/host/types/host_session_snapshot/host_session_snapshot.hpp"
#include "gui_forms/types.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace gui_forms {

class Window;

// Portable result for a host's non-client drag decision. The retained target
// remains authoritative: an authored drag backdrop is draggable only while it
// is the exact hit-test winner, so an interactive descendant keeps ordinary
// pointer input without a platform-specific exclusion rectangle.
enum class WindowChromeHitRole : std::uint8_t {
    client,
    drag_region,
};

struct WindowChromeHit final {
    WindowChromeHitRole role{WindowChromeHitRole::client};
    std::string target_stable_id;
    std::string matched_drag_region_id;

    [[nodiscard]] bool begins_native_drag() const noexcept {
        return role == WindowChromeHitRole::drag_region;
    }
};

enum class WindowChromeRegionError : std::uint8_t {
    none,
    empty_id,
    duplicate_id,
    unresolved_id,
};

struct WindowChromeRegionValidation final {
    WindowChromeRegionError error{WindowChromeRegionError::none};
    std::size_t index{};
    std::string stable_id;

    [[nodiscard]] bool accepted() const noexcept {
        return error == WindowChromeRegionError::none;
    }
};

[[nodiscard]] WindowChromeRegionValidation validate_window_chrome_drag_regions(
    const Window& window, std::span<const std::string> drag_region_ids);

[[nodiscard]] WindowChromeHit resolve_window_chrome_hit(
    Window& window, Point position,
    std::span<const std::string> drag_region_ids);

struct HostMonitor final {
    std::string id;
    Rect frame;
    Rect work_area;
    double scale{1.0};
    bool primary{};

    friend bool operator==(const HostMonitor& left,
                           const HostMonitor& right) noexcept(noexcept(
        left.id == right.id && left.frame == right.frame &&
        left.work_area == right.work_area && left.scale == right.scale &&
        left.primary == right.primary)) {
        return left.id == right.id && left.frame == right.frame &&
               left.work_area == right.work_area && left.scale == right.scale &&
               left.primary == right.primary;
    }
};

enum class HostServiceError : std::uint8_t {
    none,
    unsupported,
    wrong_thread,
    invalid_argument,
    invalid_utf8,
    too_large,
    modal_limit,
    backend_failure,
    after_shutdown,
};

struct HostServiceStatus final {
    HostServiceError error{HostServiceError::none};

    [[nodiscard]] bool accepted() const noexcept {
        return error == HostServiceError::none;
    }
};

// Semantic one-shot cues. Controls never select native files or platform
// sound identifiers; adapters map these meanings to their local presentation.
enum class HostSoundCue : std::uint8_t {
    notification,
    success,
    warning,
    error,
    operation_complete,
};

struct HostSoundCueRequest final {
    HostSoundCue cue{HostSoundCue::notification};
    double gain{1.0};
    // Caller-supplied monotonic time makes coalescing reproducible headlessly.
    std::uint64_t timestamp_nanoseconds{};
};

struct HostMonitorResult final {
    HostServiceStatus status;
    std::vector<HostMonitor> monitors;
};

struct HostClipboardTextResult final {
    HostServiceStatus status;
    std::string text_utf8;
    std::uint64_t generation{};
    bool has_text{};
};

struct HostClipboardImageResult final {
    HostServiceStatus status;
    HostImage image;
    std::uint64_t generation{};
    bool has_image{};
};

// File-manager copies carry local file references as well as optional icon
// bitmaps. Consumers choose their own codecs and must prefer these references
// when their command imports the file's contents.
struct HostClipboardFilesResult final {
    HostServiceStatus status;
    std::vector<std::string> paths_utf8;
    std::uint64_t generation{};
};

enum class HostDialogOutcome : std::uint8_t {
    accepted,
    cancelled,
};

enum class HostMessageButtons : std::uint8_t {
    ok,
    ok_cancel,
    yes_no,
    yes_no_cancel,
    retry_cancel,
};

enum class HostMessageIcon : std::uint8_t {
    none,
    information,
    warning,
    error,
    question,
};

enum class HostDialogChoice : std::uint8_t {
    none,
    ok,
    cancel,
    yes,
    no,
    retry,
};

struct HostFileDialogFilter final {
    std::string label;
    std::vector<std::string> extensions;

    friend bool operator==(const HostFileDialogFilter& left,
                           const HostFileDialogFilter& right) noexcept(noexcept(
        left.label == right.label && left.extensions == right.extensions)) {
        return left.label == right.label &&
               left.extensions == right.extensions;
    }
};

struct HostMessageDialogRequest final {
    std::string title;
    std::string message;
    HostMessageButtons buttons{HostMessageButtons::ok};
    HostMessageIcon icon{HostMessageIcon::none};
    HostDialogChoice default_choice{HostDialogChoice::ok};
};

struct HostOpenFileDialogRequest final {
    std::string title;
    std::string initial_directory;
    std::string suggested_name;
    std::vector<HostFileDialogFilter> filters;
    bool allow_multiple{};
};

struct HostSaveFileDialogRequest final {
    std::string title;
    std::string initial_directory;
    std::string suggested_name;
    std::string default_extension;
    std::vector<HostFileDialogFilter> filters;
    bool confirm_overwrite{true};
};

struct HostFolderDialogRequest final {
    std::string title;
    std::string initial_directory;
};

struct HostColorDialogRequest final {
    std::string title;
    std::uint32_t initial_rgba{0x000000FFU};
    bool allow_alpha{};
};

using HostDialogRequestPayload = std::variant<
    HostMessageDialogRequest,
    HostOpenFileDialogRequest,
    HostSaveFileDialogRequest,
    HostFolderDialogRequest,
    HostColorDialogRequest>;

struct HostDialogRequest final {
    std::uint64_t request_id{};
    // Portable stable identity for the logical owner. An empty value means the
    // application's active top-level window; it is never a native handle.
    std::string owner_id;
    HostDialogRequestPayload payload;
};

struct HostMessageDialogResult final {
    HostDialogOutcome outcome{HostDialogOutcome::cancelled};
    HostDialogChoice choice{HostDialogChoice::none};
};

struct HostPathDialogResult final {
    HostDialogOutcome outcome{HostDialogOutcome::cancelled};
    std::vector<std::string> paths;
};

struct HostColorDialogResult final {
    HostDialogOutcome outcome{HostDialogOutcome::cancelled};
    std::uint32_t rgba{};
};

using HostDialogResultPayload = std::variant<
    HostMessageDialogResult,
    HostPathDialogResult,
    HostColorDialogResult>;

struct HostDialogResult final {
    HostServiceStatus status;
    std::uint64_t request_id{};
    HostDialogResultPayload payload;
};

// A tooltip request is expressed in root-client logical coordinates. Platform
// adapters own screen conversion, work-area clamping, native window lifetime,
// and non-activation; no platform handle enters the portable contract.
struct HostTooltipRequest final {
    std::string text;
    Point anchor;
    std::uint32_t duration_milliseconds{};
};

struct HostModalTransition final {
    std::uint64_t request_id{};
    std::uint32_t depth{};
    bool entering{};
};

enum class HostCloseReason : std::uint8_t {
    user,
    application,
    session_end,
    test,
};

struct HostAttachEvent final {
    Size client_size{};
    double scale{1.0};
};

struct HostResizeEvent final {
    Size client_size{};
};

struct HostScaleEvent final {
    double scale{1.0};
};

struct HostActivationEvent final {
    bool active{};
};

struct HostOcclusionEvent final {
    bool occluded{};
};

struct HostDisplayEvent final {
    std::vector<HostMonitor> monitors;
};

struct HostCloseRequest final {
    HostCloseReason reason{HostCloseReason::user};
    bool cancel{};
    // Adapter policy: an accepted request hides a reusable window while its
    // session remains attached. Observers can cancel; changing this policy
    // during notification does not change the adapter's action.
    bool hide_on_accept{};
};

struct HostClosedEvent final {
    HostCloseReason reason{HostCloseReason::user};
};

struct HostShutdownEvent final {};

using HostEventPayload = std::variant<
    HostAttachEvent,
    HostResizeEvent,
    HostScaleEvent,
    HostActivationEvent,
    HostOcclusionEvent,
    HostDisplayEvent,
    DragEvent,
    PointerEvent,
    KeyEvent,
    TextInputEvent,
    HostCloseRequest,
    HostClosedEvent,
    HostShutdownEvent>;

struct HostEvent final {
    std::uint64_t sequence{};
    std::uint64_t timestamp_nanoseconds{};
    HostEventPayload payload;
};

enum class HostDispatchError : std::uint8_t {
    none,
    sequence_zero,
    non_monotonic_sequence,
    wrong_thread,
    after_shutdown,
    invalid_lifecycle,
    invalid_geometry,
    invalid_payload,
    callback_fault,
};

// Portable presentation lifecycle. Native adapters may have private allocation
// and handle-binding steps, but no portable callback or input may escape before
// attached. A cancelled close remains attached; an allowed close becomes
// close_authorized and admits only terminal host notifications.
struct HostDispatchResult final {
    bool handled{};
    bool input_deferred{};
    bool input_capacity_rejected{};
    bool close_allowed{true};
    DragEffect drag_effect{DragEffect::none};
    HostDispatchError error{HostDispatchError::none};

    [[nodiscard]] bool accepted() const noexcept {
        return error == HostDispatchError::none;
    }
};

[[nodiscard]] const char* host_dispatch_error_name(HostDispatchError error) noexcept;
[[nodiscard]] const char* host_lifecycle_phase_name(HostLifecyclePhase phase) noexcept;
[[nodiscard]] const char* host_event_name(const HostEventPayload& payload) noexcept;
[[nodiscard]] const char* host_service_error_name(HostServiceError error) noexcept;
[[nodiscard]] const char* cursor_kind_name(CursorKind cursor) noexcept;
[[nodiscard]] const char* drag_effect_name(DragEffect effect) noexcept;
[[nodiscard]] const char* host_dialog_kind_name(
    const HostDialogRequestPayload& payload) noexcept;
[[nodiscard]] const char* host_dialog_outcome_name(HostDialogOutcome outcome) noexcept;
[[nodiscard]] const char* host_dialog_choice_name(HostDialogChoice choice) noexcept;
[[nodiscard]] const char* host_sound_cue_name(HostSoundCue cue) noexcept;

} // namespace gui_forms
