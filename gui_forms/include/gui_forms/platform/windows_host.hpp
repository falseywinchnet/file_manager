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

struct WindowsHostOptions final {
    std::string title{"GUI.Forms Application"};
    Size initial_size{1120.0, 680.0};
    Size minimum_size{150.0, 150.0};
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

struct WindowsApplicationWindow final {
    std::string stable_id;
    std::string owner_id;
    std::unique_ptr<Window> model;
    WindowsHostOptions options;
    bool tool_window{};
};

// A virtual WinForms-handle/HDC endpoint for compatibility frontends whose
// callers retain GetDC(Control.Handle). The compatibility handle is only a
// registry token understood by the GDI shim; it is deliberately not an HWND
// and can never enter native window, input, focus, capture, z-order, or cursor
// state. The retained renderer samples the durable live raster directly;
// endpoint writes never call back into the control or enqueue UI work.
class WindowsCompatibilityPaintEndpoint final {
public:
    static std::shared_ptr<WindowsCompatibilityPaintEndpoint> acquire(
        std::uint32_t width, std::uint32_t height);

    ~WindowsCompatibilityPaintEndpoint();

    WindowsCompatibilityPaintEndpoint(
        const WindowsCompatibilityPaintEndpoint&) = delete;
    WindowsCompatibilityPaintEndpoint& operator=(
        const WindowsCompatibilityPaintEndpoint&) = delete;

    [[nodiscard]] std::uintptr_t compatibility_handle() const noexcept;
    [[nodiscard]] std::uintptr_t device_context() const noexcept;
    [[nodiscard]] std::shared_ptr<LiveSurface> live_surface() const noexcept;
    [[nodiscard]] bool publish_device_context(
        std::uintptr_t device_context) noexcept;
    // A compatibility writer may retain the HDC for the control lifetime, but
    // each destination operation must take this short lease. It prevents a
    // resize or release from replacing/deleting the selected DIB while GDI is
    // writing it. The producer's private DC is deliberately outside the lease.
    [[nodiscard]] bool begin_device_context_write(
        std::uintptr_t device_context) noexcept;
    [[nodiscard]] bool end_device_context_write(
        std::uintptr_t device_context, bool publish) noexcept;
    [[nodiscard]] bool configure(std::uint32_t width,
                                 std::uint32_t height) noexcept;
    [[nodiscard]] bool submit_bgra32_premultiplied(
        std::uint32_t width, std::uint32_t height, std::uint64_t row_bytes,
        std::span<const std::byte> pixels) noexcept;
    void touch(bool explicit_boundary = false) noexcept;
    [[nodiscard]] bool drain_now() noexcept;
    [[nodiscard]] std::string snapshot() const;
    void release() noexcept;

private:
    struct Implementation;
    explicit WindowsCompatibilityPaintEndpoint(
        std::unique_ptr<Implementation> implementation) noexcept;

    std::unique_ptr<Implementation> implementation_;
};

[[nodiscard]] HostCapabilities windows_capabilities();
int run_windows(std::unique_ptr<Window> window, WindowsHostOptions options = {});
int run_windows_application(std::vector<WindowsApplicationWindow> windows);

} // namespace gui_forms::host
