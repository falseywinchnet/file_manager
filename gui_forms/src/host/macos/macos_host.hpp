#pragma once

#include "gui_forms/host.hpp"
#include "gui_forms/window.hpp"

#include <functional>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace gui_forms::host {

struct MacHostOptions {
    std::string title{"GUI.Forms Gallery"};
    Size initial_size{1120.0, 680.0};
    Size minimum_size{980.0, 620.0};
    bool print_metrics_on_close{true};
    bool close_after_launch_for_testing{};
    std::uint32_t close_attempts_for_testing{1};
    std::function<void(HostCloseRequest&)> close_request;
    std::function<void(std::function<void()> wake,
                       std::function<void()> request_close)> host_ready;
    std::function<void()> dispatch_pending;
    std::function<void()> closed;
    std::function<void(std::string_view metrics_json,
                       std::string_view host_json)> final_snapshot;
};

struct MacHostServiceOptions {
    // Empty selects the general pasteboard. Tests use a unique named pasteboard
    // so they never overwrite the user's clipboard.
    std::string clipboard_name;
    // Native tests can exercise real panel construction and nested-loop entry
    // without waiting for human input. Production callers leave this false.
    bool cancel_dialogs_for_testing{};
};

[[nodiscard]] HostCapabilities macos_capabilities();
[[nodiscard]] std::unique_ptr<HostServices> make_macos_host_services(
    MacHostServiceOptions options = {});
int run_macos(std::unique_ptr<Window> window, MacHostOptions options = {});

} // namespace gui_forms::host
