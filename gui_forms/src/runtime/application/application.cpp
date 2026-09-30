#include "gui_forms/application.hpp"
#include "../../core/application/application_state.hpp"

#include <atomic>
#include <utility>

#if defined(GUI_FORMS_APPLICATION_MACOS)
#include "gui_forms/platform/macos_host.hpp"
#include <pthread.h>
#elif defined(GUI_FORMS_APPLICATION_WINDOWS)
#include "gui_forms/platform/windows_host.hpp"
#elif defined(GUI_FORMS_APPLICATION_LINUX)
#include "gui_forms/platform/linux_host.hpp"
#endif

namespace gui_forms {
namespace {
std::atomic_flag running = ATOMIC_FLAG_INIT;

class RunningScope final {
public:
    RunningScope() noexcept : acquired_(!running.test_and_set()) {}
    ~RunningScope() { if (acquired_) running.clear(); }
    [[nodiscard]] bool acquired() const noexcept { return acquired_; }
private:
    bool acquired_;
};
struct CallbackFailure final {
    std::exception_ptr exception;
    std::weak_ptr<detail::ApplicationWindowState> primary;

    void stop() noexcept {
        const std::shared_ptr<detail::ApplicationWindowState> state = primary.lock();
        if (!state || (*state).closed || !(*state).request_close) return;
        try { (*state).request_close(); } catch (...) {}
    }
    void capture() noexcept {
        if (!exception) exception = std::current_exception();
        stop();
    }
};
struct WindowBridge final {
    std::shared_ptr<detail::ApplicationWindowState> state;
    std::shared_ptr<CallbackFailure> failure;
    Window* model{};
    ApplicationWindowOptions options;
    bool host_ready{};
    bool visibility_ready{};
    bool notified{};

    void notify() noexcept {
        if (notified || !host_ready || !visibility_ready) return;
        notified = true;
        (*state).ready = true;
        if ((*failure).exception) { (*failure).stop(); return; }
        try {
            if (options.ready) options.ready(*model, detail::ApplicationHandleAccess::make(state));
        } catch (...) { (*failure).capture(); }
    }
};
struct HostReady final {
    std::shared_ptr<WindowBridge> bridge;
    void operator()(std::function<void()> wake, std::function<void()> request_close,
                    std::function<HostDialogResult(const HostDialogRequest&)>,
                    std::function<HostServiceStatus(const HostTooltipRequest&)>,
                    std::function<void()>, std::function<HostClipboardTextResult()>,
                    std::function<HostServiceStatus(std::string_view)>) const noexcept {
        (*(*bridge).state).request_close = std::move(request_close);
        (*bridge).host_ready = true;
        try {
            if ((*bridge).options.wake_ready) {
                (*bridge).options.wake_ready(std::move(wake));
            }
        } catch (...) { (*(*bridge).failure).capture(); }
        (*bridge).notify();
    }
};
struct DispatchPending final {
    std::shared_ptr<WindowBridge> bridge;
    void operator()() const noexcept {
        if ((*(*bridge).state).closed || (*(*bridge).failure).exception) return;
        try {
            if ((*bridge).options.dispatch_pending) (*bridge).options.dispatch_pending();
        } catch (...) { (*(*bridge).failure).capture(); }
    }
};
struct FullScreenReady final {
    std::shared_ptr<WindowBridge> bridge;
    void operator()(std::function<void()> toggle) const noexcept {
        (*(*bridge).state).toggle_full_screen = std::move(toggle);
    }
};
struct VisibilityReady final {
    std::shared_ptr<WindowBridge> bridge;
    void operator()(std::function<void()> show, std::function<void()> hide) const noexcept {
        (*(*bridge).state).show = std::move(show);
        (*(*bridge).state).hide = std::move(hide);
        (*bridge).visibility_ready = true;
        (*bridge).notify();
    }
};
struct Closing final {
    std::shared_ptr<WindowBridge> bridge;
    void operator()(HostCloseRequest& request) const noexcept {
        if ((*(*bridge).failure).exception) { request.cancel = false; return; }
        try {
            if ((*bridge).options.closing) (*bridge).options.closing(request);
        } catch (...) {
            request.cancel = false;
            (*(*bridge).failure).capture();
        }
    }
};
struct Closed final {
    std::shared_ptr<WindowBridge> bridge;
    void operator()() const noexcept {
        if ((*(*bridge).state).closed) return;
        (*(*bridge).state).closed = true;
        (*(*bridge).state).ready = false;
        (*(*bridge).state).request_close = {};
        (*(*bridge).state).show = {};
        (*(*bridge).state).hide = {};
        (*(*bridge).state).toggle_full_screen = {};
        try {
            if ((*bridge).options.closed) (*bridge).options.closed();
        } catch (...) { (*(*bridge).failure).capture(); }
    }
};
class BridgeScope final {
public:
    ~BridgeScope() {
        for (std::size_t index = 0U; index < windows.size(); ++index) {
            detail::ApplicationWindowState& state = *(*windows[index]).state;
            state.closed = true;
            state.ready = false;
            state.request_close = {};
            state.show = {};
            state.hide = {};
            state.toggle_full_screen = {};
        }
    }
    std::vector<std::shared_ptr<WindowBridge>> windows;
};

template <typename NativeWindow>
std::vector<NativeWindow> prepare(std::vector<ApplicationWindow>& windows, BridgeScope& scope,
                                  const std::shared_ptr<CallbackFailure>& failure) {
    std::vector<NativeWindow> native;
    native.reserve(windows.size());
    scope.windows.reserve(windows.size());
    for (std::size_t index = 0U; index < windows.size(); ++index) {
        ApplicationWindow& source = windows[index];
        const bool primary = source.owner_id.empty() && !source.tool_window;
        const std::shared_ptr<WindowBridge> bridge = std::make_shared<WindowBridge>();
        (*bridge).state = std::make_shared<detail::ApplicationWindowState>();
        (*bridge).failure = failure;
        (*bridge).model = source.model.get();
        (*bridge).options = std::move(source.options);
        (*(*bridge).state).can_hide = !primary;
        if (primary) (*failure).primary = (*bridge).state;
        scope.windows.push_back(bridge);
        NativeWindow entry;
        entry.stable_id = std::move(source.stable_id);
        entry.owner_id = std::move(source.owner_id);
        entry.tool_window = source.tool_window;
        entry.model = std::move(source.model);
        entry.options.title = (*bridge).options.title;
        entry.options.initial_size = (*bridge).options.initial_size;
        entry.options.minimum_size = (*bridge).options.minimum_size;
        entry.options.initially_visible = (*bridge).options.initially_visible;
        entry.options.hide_on_close = (*bridge).options.hide_on_close;
        entry.options.minimizable = (*bridge).options.minimizable;
        entry.options.print_metrics_on_close = (*bridge).options.print_metrics_on_close;
        entry.options.host_ready = HostReady{bridge};
        entry.options.dispatch_pending = DispatchPending{bridge};
        entry.options.visibility_ready = VisibilityReady{bridge};
        entry.options.full_screen_ready = FullScreenReady{bridge};
        entry.options.close_request = Closing{bridge};
        entry.options.closed = Closed{bridge};
        native.push_back(std::move(entry));
    }
    return native;
}
}

HostCapabilities Application::capabilities() {
#if defined(GUI_FORMS_APPLICATION_MACOS)
    return host::macos_capabilities();
#elif defined(GUI_FORMS_APPLICATION_WINDOWS)
    return host::windows_capabilities();
#elif defined(GUI_FORMS_APPLICATION_LINUX)
    return host::linux_capabilities();
#else
    return {HostCapabilities::current_protocol_version, "unsupported", HostCapability::none};
#endif
}

ApplicationResult Application::run(std::unique_ptr<Window> window, ApplicationWindowOptions options) {
    std::vector<ApplicationWindow> windows;
    ApplicationWindow entry;
    entry.stable_id = "main";
    entry.model = std::move(window);
    entry.options = std::move(options);
    windows.push_back(std::move(entry));
    return run(std::move(windows));
}

ApplicationResult Application::run(std::vector<ApplicationWindow> windows) {
#if defined(GUI_FORMS_APPLICATION_MACOS)
    if (pthread_main_np() == 0) return {ApplicationError::wrong_thread};
#endif
    const RunningScope running_scope;
    if (!running_scope.acquired()) return {ApplicationError::already_running};
    ApplicationResult result = validate(windows);
    if (!result.accepted()) return result;
    BridgeScope bridges;
    const std::shared_ptr<CallbackFailure> failure = std::make_shared<CallbackFailure>();
#if defined(GUI_FORMS_APPLICATION_MACOS)
    result.native_exit_code = host::run_macos_application(prepare<host::MacApplicationWindow>(windows, bridges, failure));
#elif defined(GUI_FORMS_APPLICATION_WINDOWS)
    result.native_exit_code = host::run_windows_application(prepare<host::WindowsApplicationWindow>(windows, bridges, failure));
#elif defined(GUI_FORMS_APPLICATION_LINUX)
    result.native_exit_code = host::run_linux_application(prepare<host::LinuxApplicationWindow>(windows, bridges, failure));
#else
    result.error = ApplicationError::unsupported;
#endif
    if ((*failure).exception) {
        result.error = ApplicationError::callback_failure;
        result.callback_exception = (*failure).exception;
    } else if (result.native_exit_code != 0) {
        result.error = ApplicationError::backend_failure;
    }
    return result;
}
} // namespace gui_forms
