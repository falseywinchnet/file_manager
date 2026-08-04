#pragma once

#include "gui_forms/event.hpp"
#include "gui_forms/events.hpp"
#include "gui_forms/types.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <variant>
#include <vector>

namespace gui_forms {

class Window;

enum class HostCapability : std::uint64_t {
    none = 0,
    lifecycle = 1ULL << 0U,
    scale_notifications = 1ULL << 1U,
    monitor_geometry = 1ULL << 2U,
    occlusion = 1ULL << 3U,
    scheduled_wake = 1ULL << 4U,
    pointer_input = 1ULL << 5U,
    keyboard_input = 1ULL << 6U,
    text_composition = 1ULL << 7U,
    pointer_capture = 1ULL << 8U,
    cursor = 1ULL << 9U,
    clipboard = 1ULL << 10U,
    typed_drag_drop = 1ULL << 11U,
    dialogs = 1ULL << 12U,
    menus = 1ULL << 13U,
    font_discovery = 1ULL << 14U,
    accessibility = 1ULL << 15U,
};

[[nodiscard]] constexpr HostCapability operator|(HostCapability left,
                                                  HostCapability right) noexcept {
    return static_cast<HostCapability>(static_cast<std::uint64_t>(left) |
                                       static_cast<std::uint64_t>(right));
}

[[nodiscard]] constexpr bool has_capability(HostCapability available,
                                            HostCapability requested) noexcept {
    return (static_cast<std::uint64_t>(available) &
            static_cast<std::uint64_t>(requested)) ==
           static_cast<std::uint64_t>(requested);
}

struct HostCapabilities final {
    static constexpr std::uint32_t current_protocol_version = 4;

    std::uint32_t protocol_version{current_protocol_version};
    std::string platform{"unknown"};
    HostCapability available{HostCapability::none};

    [[nodiscard]] bool supports(HostCapability capability) const noexcept {
        return has_capability(available, capability);
    }
    [[nodiscard]] std::string to_json() const;
};

struct HostMonitor final {
    std::string id;
    Rect frame;
    Rect work_area;
    double scale{1.0};
    bool primary{};

    friend bool operator==(const HostMonitor&, const HostMonitor&) = default;
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

    friend bool operator==(const HostFileDialogFilter&,
                           const HostFileDialogFilter&) = default;
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

struct HostModalTransition final {
    std::uint64_t request_id{};
    std::uint32_t depth{};
    bool entering{};
};

struct HostServicesSnapshot final {
    HostCapabilities capabilities;
    CursorKind cursor{CursorKind::arrow};
    bool pointer_captured{};
    std::uint64_t captured_pointer_id{};
    std::uint64_t monitor_queries{};
    std::uint64_t cursor_updates{};
    std::uint64_t pointer_capture_updates{};
    std::uint64_t clipboard_reads{};
    std::uint64_t clipboard_writes{};
    std::uint64_t clipboard_generation{};
    std::uint64_t dialog_requests{};
    std::uint64_t dialog_completions{};
    std::uint64_t dialog_cancellations{};
    std::uint32_t modal_depth{};
    std::uint32_t maximum_modal_depth{};
    std::uint64_t rejected_requests{};
    bool shutdown{};

    [[nodiscard]] std::string to_json() const;
};

// Portable host-to-platform service seam. The base class owns thread,
// capability, UTF-8, size, accounting, and shutdown policy so adapters cannot
// silently disagree on those contracts.
class HostServices {
public:
    static constexpr std::size_t maximum_clipboard_text_bytes = 16U * 1024U * 1024U;
    static constexpr std::size_t maximum_dialog_text_bytes = 64U * 1024U;
    static constexpr std::size_t maximum_dialog_filters = 64U;
    static constexpr std::size_t maximum_dialog_extensions = 64U;
    static constexpr std::size_t maximum_dialog_paths = 64U;
    static constexpr std::uint32_t maximum_nested_modal_depth = 8U;

    explicit HostServices(HostCapabilities capabilities);
    virtual ~HostServices() = default;
    HostServices(const HostServices&) = delete;
    HostServices& operator=(const HostServices&) = delete;

    [[nodiscard]] HostMonitorResult query_monitors();
    [[nodiscard]] HostServiceStatus set_cursor(CursorKind cursor);
    [[nodiscard]] HostServiceStatus set_pointer_capture(bool captured,
                                                        std::uint64_t pointer_id = 1);
    [[nodiscard]] HostClipboardTextResult read_clipboard_text();
    [[nodiscard]] HostServiceStatus write_clipboard_text(std::string_view text_utf8);
    [[nodiscard]] HostDialogResult show_dialog(const HostDialogRequest& request);
    void shutdown() noexcept;

    [[nodiscard]] HostServicesSnapshot snapshot() const;
    [[nodiscard]] Event<const HostModalTransition&>& modal_changed() noexcept {
        return modal_changed_;
    }

protected:
    [[nodiscard]] virtual HostMonitorResult query_monitors_impl() = 0;
    [[nodiscard]] virtual HostServiceStatus set_cursor_impl(CursorKind cursor) = 0;
    [[nodiscard]] virtual HostServiceStatus set_pointer_capture_impl(
        bool captured, std::uint64_t pointer_id) = 0;
    [[nodiscard]] virtual HostClipboardTextResult read_clipboard_text_impl() = 0;
    [[nodiscard]] virtual HostServiceStatus write_clipboard_text_impl(
        std::string_view text_utf8) = 0;
    [[nodiscard]] virtual HostDialogResult show_dialog_impl(
        const HostDialogRequest& request) = 0;
    virtual void shutdown_impl() noexcept {}

private:
    [[nodiscard]] HostServiceStatus validate_request(HostCapability capability) noexcept;

    std::thread::id ui_thread_;
    HostServicesSnapshot snapshot_;
    Event<const HostModalTransition&> modal_changed_;
    std::vector<std::uint64_t> modal_stack_;
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
    invalid_geometry,
    invalid_payload,
};

struct HostDispatchResult final {
    bool handled{};
    bool close_allowed{true};
    DragEffect drag_effect{DragEffect::none};
    HostDispatchError error{HostDispatchError::none};

    [[nodiscard]] bool accepted() const noexcept {
        return error == HostDispatchError::none;
    }
};

struct HostSessionSnapshot final {
    HostCapabilities capabilities;
    std::uint64_t last_sequence{};
    std::uint64_t events_accepted{};
    std::uint64_t events_rejected{};
    std::uint64_t close_requests{};
    std::uint64_t close_cancellations{};
    std::uint64_t display_changes{};
    std::uint64_t monitor_count{};
    std::uint64_t modal_transitions{};
    std::uint64_t modal_input_suppressions{};
    std::uint64_t drag_events{};
    std::uint64_t drag_drops{};
    std::uint32_t modal_depth{};
    bool attached{};
    bool active{};
    bool occluded{};
    bool closed{};
    bool shutdown{};

    [[nodiscard]] std::string to_json() const;
};

// Experimental 0.x host seam. It normalizes host events into the retained
// Window without admitting any platform type into the portable API.
class HostSession final {
public:
    HostSession(Window& window,
                HostCapabilities capabilities,
                HostServices* services = nullptr);
    ~HostSession();
    HostSession(const HostSession&) = delete;
    HostSession& operator=(const HostSession&) = delete;

    [[nodiscard]] HostDispatchResult dispatch(HostEvent event);
    void shutdown() noexcept;

    [[nodiscard]] Event<HostCloseRequest&>& closing() noexcept { return closing_; }
    [[nodiscard]] Event<const HostEvent&, const HostDispatchResult&>& observed() noexcept {
        return observed_;
    }
    [[nodiscard]] HostSessionSnapshot snapshot() const;

private:
    Window* window_{};
    HostServices* services_{};
    std::thread::id ui_thread_;
    HostSessionSnapshot snapshot_;
    Event<HostCloseRequest&> closing_;
    Event<const HostEvent&, const HostDispatchResult&> observed_;
    SubscriptionToken capture_observation_;
    SubscriptionToken modal_observation_;
};

[[nodiscard]] const char* host_dispatch_error_name(HostDispatchError error) noexcept;
[[nodiscard]] const char* host_event_name(const HostEventPayload& payload) noexcept;
[[nodiscard]] const char* host_service_error_name(HostServiceError error) noexcept;
[[nodiscard]] const char* cursor_kind_name(CursorKind cursor) noexcept;
[[nodiscard]] const char* drag_effect_name(DragEffect effect) noexcept;
[[nodiscard]] const char* host_dialog_kind_name(
    const HostDialogRequestPayload& payload) noexcept;
[[nodiscard]] const char* host_dialog_outcome_name(HostDialogOutcome outcome) noexcept;
[[nodiscard]] const char* host_dialog_choice_name(HostDialogChoice choice) noexcept;

} // namespace gui_forms
