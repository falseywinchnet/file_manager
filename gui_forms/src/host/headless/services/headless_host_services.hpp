#pragma once

#include "../capabilities/headless_capabilities.hpp"

#include <cstdint>
#include <deque>
#include <functional>
#include <string>

namespace gui_forms::host {

class HeadlessHostServices final : public HostServices {
public:
    using DialogHandler = std::function<HostDialogResult(
        const HostDialogRequest&, HeadlessHostServices&)>;

    HeadlessHostServices();
    ~HeadlessHostServices() override { shutdown(); }

    void queue_dialog_result(HostDialogResult result);
    void set_dialog_handler(DialogHandler handler);
    [[nodiscard]] const std::string& dialog_trace() const noexcept {
        return dialog_trace_;
    }
    [[nodiscard]] const std::string& sound_trace() const noexcept {
        return sound_trace_;
    }

protected:
    [[nodiscard]] HostMonitorResult query_monitors_impl() override;
    [[nodiscard]] HostServiceStatus set_cursor_impl(CursorKind cursor) override;
    [[nodiscard]] HostServiceStatus set_pointer_capture_impl(
        bool captured, std::uint64_t pointer_id) override;
    [[nodiscard]] HostClipboardTextResult read_clipboard_text_impl() override;
    [[nodiscard]] HostServiceStatus write_clipboard_text_impl(
        std::string_view text_utf8) override;
    [[nodiscard]] HostClipboardImageResult read_clipboard_image_impl() override;
    [[nodiscard]] HostServiceStatus write_clipboard_image_impl(HostImageView image) override;
    [[nodiscard]] HostDialogResult show_dialog_impl(
        const HostDialogRequest& request) override;
    [[nodiscard]] HostServiceStatus play_sound_cue_impl(
        const HostSoundCueRequest& request) override;
    void shutdown_impl() noexcept override;

private:
    std::vector<HostMonitor> monitors_;
    HostImage clipboard_image_;
    bool clipboard_has_image_{};
    std::string clipboard_text_;
    std::uint64_t clipboard_generation_{};
    bool clipboard_has_text_{};
    std::deque<HostDialogResult> dialog_results_;
    DialogHandler dialog_handler_;
    std::string dialog_trace_;
    std::string sound_trace_;
};

} // namespace gui_forms::host
