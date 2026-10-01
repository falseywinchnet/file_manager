#pragma once
#include "gui_forms/types/cursor_image/cursor_image.hpp"

#include "gui_forms/host/types/host_types.hpp"
#include "gui_forms/host/cursor_interaction/cursor_interaction.hpp"

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
    virtual ~HostServices();
    HostServices(const HostServices&) = delete;
    HostServices& operator=(const HostServices&) = delete;

    [[nodiscard]] HostMonitorResult query_monitors();
    [[nodiscard]] CursorCapabilities cursor_capabilities() const noexcept;
    [[nodiscard]] CursorLeaseResult begin_cursor_hidden(Window& window);
    [[nodiscard]] CursorStatus warp_cursor(Window& window, CursorMetrics metrics, Point target);
    // Host lifecycle hooks: revoke before focus/capture loss, detach or close.
    // Cleanup failures remain visible on outstanding leases and this snapshot.
    void revoke_cursor_interaction(CursorError cause = CursorError::revoked) noexcept;
    [[nodiscard]] CursorLeaseSnapshot cursor_interaction_snapshot() const noexcept;
    [[nodiscard]] bool cursor_hidden() const noexcept;
    [[nodiscard]] HostServiceStatus set_cursor(CursorKind cursor);
    // Native failure/unsupported images immediately install fallback. Success
    // then means a usable pointer; it does not promise custom-image support.
    [[nodiscard]] HostServiceStatus set_custom_cursor(CursorImagesPtr images,
        double device_scale, CursorKind fallback = CursorKind::arrow);
    [[nodiscard]] HostServiceStatus set_pointer_capture(
        bool captured, std::uint64_t pointer_id = 1);
    [[nodiscard]] HostClipboardTextResult read_clipboard_text();
    [[nodiscard]] HostServiceStatus write_clipboard_text(
        std::string_view text_utf8);
    /// Reads an owned straight-alpha RGBA8 image on the creating UI thread.
    /// Successful absence is distinguished by has_image == false.
    [[nodiscard]] HostClipboardImageResult read_clipboard_image();
    /// Reads local file references without opening or decoding their contents.
    [[nodiscard]] HostClipboardFilesResult read_clipboard_files();
    /// Copies an image to the native clipboard before returning. Does not retain
    /// the borrowed view. Replaces clipboard contents on successful publication.
    [[nodiscard]] HostServiceStatus write_clipboard_image(HostImageView image);
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
    [[nodiscard]] virtual CursorCapabilities cursor_capabilities_impl() const noexcept;
    [[nodiscard]] virtual CursorStatus cursor_authority_impl(const Window& window, bool require_pointer) const noexcept;
    [[nodiscard]] virtual CursorStatus hide_cursor_impl() noexcept;
    [[nodiscard]] virtual CursorStatus restore_cursor_impl() noexcept;
    [[nodiscard]] virtual CursorStatus warp_cursor_impl(int client_x, int client_y) noexcept;
    [[nodiscard]] virtual HostMonitorResult query_monitors_impl() = 0;
    [[nodiscard]] virtual HostServiceStatus set_cursor_impl(CursorKind cursor) = 0;
    [[nodiscard]] virtual HostServiceStatus set_custom_cursor_impl(
        const CursorImagesPtr& images, const CursorImage& image);
    [[nodiscard]] virtual HostServiceStatus set_pointer_capture_impl(
        bool captured, std::uint64_t pointer_id) = 0;
    [[nodiscard]] virtual HostClipboardTextResult read_clipboard_text_impl() = 0;
    [[nodiscard]] virtual HostServiceStatus write_clipboard_text_impl(
        std::string_view text_utf8) = 0;
    [[nodiscard]] virtual HostClipboardImageResult read_clipboard_image_impl();
    [[nodiscard]] virtual HostClipboardFilesResult read_clipboard_files_impl();
    [[nodiscard]] virtual HostServiceStatus write_clipboard_image_impl(HostImageView image);
    [[nodiscard]] virtual HostDialogResult show_dialog_impl(
        const HostDialogRequest& request) = 0;
    [[nodiscard]] virtual HostServiceStatus play_sound_cue_impl(
        const HostSoundCueRequest& request) = 0;
    virtual void shutdown_impl() noexcept {}

private:
    friend class CursorHiddenLease;
    [[nodiscard]] CursorStatus release_cursor_lease(CursorLeaseState& state) noexcept;
    [[nodiscard]] CursorStatus validate_cursor_request(const Window& window) const noexcept;
    std::shared_ptr<CursorLeaseState> cursor_lease_{};
    [[nodiscard]] HostServiceStatus validate_request(
        HostCapability capability) noexcept;

    CursorImagesPtr prepared_cursor_images_;
    CursorImage prepared_cursor_;
    double prepared_request_scale_{};
    CursorImagesPtr active_cursor_images_;
    CursorImagesPtr failed_cursor_images_;
    double active_cursor_scale_{};
    double failed_cursor_scale_{};
    bool cursor_applied_{};
    std::thread::id ui_thread_;
    HostServicesSnapshot snapshot_;
    Event<const HostModalTransition&> modal_changed_;
    std::vector<std::uint64_t> modal_stack_;
    std::optional<HostSoundCue> last_sound_cue_;
    std::uint64_t last_sound_timestamp_{};
};

} // namespace gui_forms
