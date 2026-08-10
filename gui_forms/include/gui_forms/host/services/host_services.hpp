#pragma once

#include "gui_forms/host/types/host_types.hpp"

#include <chrono>
#include <optional>
#include <thread>

namespace gui_forms {

// Portable host-to-platform service seam. The base class owns thread,
// capability, UTF-8, size, accounting, modal-depth, and shutdown policy so
// adapters cannot silently disagree on those contracts.
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
    [[nodiscard]] HostServiceStatus set_pointer_capture(
        bool captured, std::uint64_t pointer_id = 1);
    [[nodiscard]] HostClipboardTextResult read_clipboard_text();
    [[nodiscard]] HostServiceStatus write_clipboard_text(
        std::string_view text_utf8);
    [[nodiscard]] HostDialogResult show_dialog(
        const HostDialogRequest& request);
    [[nodiscard]] HostServiceStatus play_sound_cue(
        const HostSoundCueRequest& request);
    [[nodiscard]] std::chrono::nanoseconds
    sound_cue_coalescing_window() const noexcept {
        return std::chrono::nanoseconds(
            snapshot_.sound_coalescing_window_nanoseconds);
    }
    // A zero interval disables burst coalescing. The upper bound prevents a
    // theme or consumer setting from suppressing a semantic cue indefinitely.
    [[nodiscard]] HostServiceStatus set_sound_cue_coalescing_window(
        std::chrono::nanoseconds window);
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
    [[nodiscard]] virtual HostServiceStatus play_sound_cue_impl(
        const HostSoundCueRequest& request) = 0;
    virtual void shutdown_impl() noexcept {}

private:
    [[nodiscard]] HostServiceStatus validate_request(
        HostCapability capability) noexcept;

    std::thread::id ui_thread_;
    HostServicesSnapshot snapshot_;
    Event<const HostModalTransition&> modal_changed_;
    std::vector<std::uint64_t> modal_stack_;
    std::optional<HostSoundCue> last_sound_cue_;
    std::uint64_t last_sound_timestamp_{};
};

} // namespace gui_forms
