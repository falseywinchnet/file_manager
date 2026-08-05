#pragma once

#include "gui_forms/host.hpp"
#include "gui_forms/window.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace gui_forms::host {

struct WindowsHostOptions final {
    std::string title{"GUI.Forms Gallery"};
    Size initial_size{1120.0, 680.0};
    Size minimum_size{980.0, 620.0};
    bool print_metrics_on_close{true};
    bool automation_enabled{};
    bool close_after_launch_for_testing{};
    bool quit_thread_on_close{true};
    bool popup_window{};
    Point initial_position{};
    std::function<std::shared_ptr<Control>(std::string_view)> automation_resolve;
    std::function<void(HostCloseRequest&)> close_request;
    std::function<void(std::function<void()> wake,
                       std::function<void()> request_close,
                       std::function<HostDialogResult(const HostDialogRequest&)> show_dialog,
                       std::function<HostServiceStatus(const HostTooltipRequest&)> show_tooltip,
                       std::function<void()> hide_tooltip,
                       std::function<HostClipboardTextResult()> read_clipboard_text,
                       std::function<HostServiceStatus(std::string_view)> write_clipboard_text)> host_ready;
    std::function<void()> dispatch_pending;
    std::function<void()> closed;
    std::function<void(std::string_view metrics_json,
                       std::string_view host_json)> final_snapshot;
};

[[nodiscard]] HostCapabilities windows_capabilities();
int run_windows(std::unique_ptr<Window> window,
                WindowsHostOptions options = {});

} // namespace gui_forms::host
