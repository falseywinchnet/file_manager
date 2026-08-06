#pragma once

#include "gui_forms/host.hpp"
#include "gui_forms/window.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace gui_forms::host {

struct MacHostOptions {
    std::string title{"GUI.Forms Application"};
    Size initial_size{1120.0, 680.0};
    Size minimum_size{150.0, 150.0};
    bool print_metrics_on_close{true};
    bool close_after_launch_for_testing{};
    std::uint32_t close_attempts_for_testing{1};
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

struct MacHostServiceOptions {
    std::string clipboard_name;
    bool cancel_dialogs_for_testing{};
};

// A native application may present several independent retained roots without
// merging their models or starting a second process. Ownership is expressed by
// stable host IDs, never native handles; adapters map it to their platform's
// owned/tool-window relationship.
struct MacApplicationWindow final {
    std::string stable_id;
    std::string owner_id;
    std::unique_ptr<Window> model;
    MacHostOptions options;
    bool tool_window{};
};

[[nodiscard]] HostCapabilities macos_capabilities();
[[nodiscard]] std::unique_ptr<HostServices> make_macos_host_services(
    MacHostServiceOptions options = {});
int run_macos(std::unique_ptr<Window> window, MacHostOptions options = {});
int run_macos_application(std::vector<MacApplicationWindow> windows);

} // namespace gui_forms::host
