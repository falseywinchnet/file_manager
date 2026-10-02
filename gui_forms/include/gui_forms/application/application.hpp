#pragma once

#include "gui_forms/host/types/host_types.hpp"
#include "gui_forms/window.hpp"

#include <cstddef>
#include <exception>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace gui_forms {
namespace detail {
struct ApplicationWindowState;
struct ApplicationHandleAccess;
}

enum class ApplicationError : std::uint8_t {
    none,
    invalid_argument,
    wrong_thread,
    already_running,
    unsupported,
    backend_failure,
    callback_failure,
};

struct ApplicationResult final {
    ApplicationError error{ApplicationError::none};
    int native_exit_code{};
    std::size_t window_index{std::numeric_limits<std::size_t>::max()};
    std::exception_ptr callback_exception{};

    [[nodiscard]] bool accepted() const noexcept {
        const bool accepted = error == ApplicationError::none;
        return accepted;
    }
};

/// A weak, thread-affine handle to one native application window.
/// Requests after close or after Application::run return after_shutdown.
class ApplicationWindowHandle final {
public:
    ApplicationWindowHandle() noexcept = default;
    [[nodiscard]] bool active() const noexcept;
    [[nodiscard]] HostServiceStatus request_close() const;
    [[nodiscard]] HostServiceStatus show() const;
    [[nodiscard]] HostServiceStatus hide() const;
    [[nodiscard]] HostServiceStatus toggle_full_screen() const;
    // UI-thread request; title is borrowed only for this call. Empty is allowed;
    // malformed UTF-8, embedded NUL and more than 65536 bytes are refused.
    // Acceptance is a native update request, not a compositor presentation receipt.
    [[nodiscard]] HostServiceStatus set_title(const std::string_view title) const;

private:
    friend struct detail::ApplicationHandleAccess;
    explicit ApplicationWindowHandle(
        std::weak_ptr<detail::ApplicationWindowState> state) noexcept;
    std::weak_ptr<detail::ApplicationWindowState> state_{};
};

struct ApplicationWindowOptions final {
    std::string title{"GUI.Forms Application"};
    Size initial_size{1120.0, 680.0};
    Size minimum_size{150.0, 150.0};
    bool initially_visible{true};
    bool hide_on_close{};
    bool minimizable{true};
    bool print_metrics_on_close{};
    // Runs after this model has an attached host and services. Other windows
    // may not be ready yet. Keep a weak handle for native window requests.
    std::function<void(Window&, ApplicationWindowHandle)> ready{};
    // Published on the UI thread before ready. The supplied wake may be called
    // by workers while this window is alive; workers must stop before closed.
    // Wake schedules work only. dispatch_pending executes on the UI thread.
    std::function<void(std::function<void()>)> wake_ready{};
    std::function<void()> dispatch_pending{};
    std::function<void(HostCloseRequest&)> closing{};
    std::function<void()> closed{};
};

struct ApplicationWindow final {
    std::string stable_id{};
    std::string owner_id{};
    std::unique_ptr<Window> model{};
    ApplicationWindowOptions options{};
    bool tool_window{};
};

/// Runs the selected native host through one portable C++ entry point.
/// Link GUIForms::Application for run/capabilities; validation and handles
/// are part of GUIForms::Core. Window models are consumed even on failure.
class Application final {
public:
    Application() = delete;
    static constexpr std::size_t maximum_windows = 64U;
    [[nodiscard]] static ApplicationResult validate(
        const std::vector<ApplicationWindow>& windows);
    [[nodiscard]] static HostCapabilities capabilities();
    [[nodiscard]] static ApplicationResult run(
        std::unique_ptr<Window> window, ApplicationWindowOptions options = {});
    [[nodiscard]] static ApplicationResult run(
        std::vector<ApplicationWindow> windows);
};

} // namespace gui_forms
