#pragma once

#include "gui_forms/host.hpp"
#include "gui_forms/window.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace gui_forms::host {

struct LinuxHostOptions final {
  std::string title{"GUI.Forms Application"};
  Size initial_size{1120.0, 680.0};
  Size minimum_size{150.0, 150.0};
  bool print_metrics_on_close{true};
  bool initially_visible{true};
  bool hide_on_close{};
  bool minimizable{true};
  std::function<void(std::function<void()> show, std::function<void()> hide)>
      visibility_ready;
  std::function<void(std::function<void()> toggle)> full_screen_ready;
  // Published on the host UI thread during initialization. The returned title
  // operation borrows its UTF-8 input only through the call; empty is allowed,
  // NUL/malformed UTF-8/>65536 bytes are invalid_argument. Invoke on the host
  // UI thread; wrong-thread calls are refused before native mutable access.
  // Copies do not own the host/window and may be discarded after run returns.
  // Calls on the UI thread after native lifetime ends return after_shutdown.
  // Adapter/conversion exceptions become backend_failure; accepted means the
  // native request was submitted, not that title pixels were presented.
  std::function<void(std::function<HostServiceStatus(std::string_view)>)> title_ready;
  std::function<void(HostCloseRequest &)> close_request;
  std::function<void(
      std::function<void()> wake, std::function<void()> request_close,
      std::function<HostDialogResult(const HostDialogRequest &)> show_dialog,
      std::function<HostServiceStatus(const HostTooltipRequest &)> show_tooltip,
      std::function<void()> hide_tooltip,
      std::function<HostClipboardTextResult()> read_clipboard_text,
      std::function<HostServiceStatus(std::string_view)> write_clipboard_text)>
      host_ready;
  std::function<void()> dispatch_pending;
  std::function<void()> closed;
};

struct LinuxApplicationWindow final {
  std::string stable_id;
  std::string owner_id;
  std::unique_ptr<Window> model;
  LinuxHostOptions options;
  bool tool_window{};
};

[[nodiscard]] HostCapabilities linux_capabilities();
int run_linux(std::unique_ptr<Window> window, LinuxHostOptions options = {});
int run_linux_application(std::vector<LinuxApplicationWindow> linux);

} // namespace gui_forms::host
