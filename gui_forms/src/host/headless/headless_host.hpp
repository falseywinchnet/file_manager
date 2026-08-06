#pragma once

#include "gui_forms/host.hpp"
#include "gui_forms/window.hpp"

#include <cstdint>
#include <deque>
#include <functional>
#include <atomic>
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
    [[nodiscard]] HostDialogResult show_dialog_impl(
        const HostDialogRequest& request) override;
    [[nodiscard]] HostServiceStatus play_sound_cue_impl(
        const HostSoundCueRequest& request) override;
    void shutdown_impl() noexcept override;

private:
    std::vector<HostMonitor> monitors_;
    std::string clipboard_text_;
    std::uint64_t clipboard_generation_{};
    bool clipboard_has_text_{};
    std::deque<HostDialogResult> dialog_results_;
    DialogHandler dialog_handler_;
    std::string dialog_trace_;
    std::string sound_trace_;
};

// Deterministic reference adapter for host-event conformance tests. It owns no
// clock and performs no background work; the caller supplies every timestamp.
class HeadlessHost final {
public:
    explicit HeadlessHost(Window& window);
    ~HeadlessHost() = default;
    HeadlessHost(const HeadlessHost&) = delete;
    HeadlessHost& operator=(const HeadlessHost&) = delete;

    [[nodiscard]] HostDispatchResult dispatch(HostEventPayload payload,
                                              std::uint64_t timestamp_nanoseconds);
    [[nodiscard]] DispatchDrainResult pump_dispatcher(
        std::size_t maximum_callbacks = maximum_callbacks_per_dispatch_turn);
    [[nodiscard]] bool dispatcher_wake_pending() const noexcept {
        return dispatcher_wake_pending_.load(std::memory_order_acquire);
    }
    [[nodiscard]] HostSession& session() noexcept { return session_; }
    [[nodiscard]] const HostSession& session() const noexcept { return session_; }
    [[nodiscard]] HostServices& services() noexcept { return services_; }
    [[nodiscard]] const HostServices& services() const noexcept { return services_; }
    [[nodiscard]] const std::string& trace() const noexcept { return trace_; }

private:
    Window* window_{};
    std::atomic<bool> dispatcher_wake_pending_{};
    HeadlessHostServices services_;
    HostSession session_;
    SubscriptionToken observation_;
    std::uint64_t next_sequence_{1};
    std::string trace_;
};

[[nodiscard]] HostCapabilities headless_capabilities();

} // namespace gui_forms::host
