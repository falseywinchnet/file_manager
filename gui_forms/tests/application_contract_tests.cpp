#include "gui_forms/application.hpp"
#include "gui_forms/control.hpp"
#include "gui_forms/host/session/host_session.hpp"
#include "../src/core/application/application_state.hpp"

#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>

namespace {
using namespace gui_forms;
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
ApplicationWindow make_window(std::string id, std::string owner = {}) {
    ApplicationWindow entry{};
    entry.stable_id = std::move(id);
    entry.owner_id = std::move(owner);
    const std::shared_ptr<Control> root = make_control<Control>(StableId("root"));
    entry.model = std::make_unique<Window>(root, Size{320.0, 240.0});
    return entry;
}
struct ValidateOnWorker final {
    const std::vector<ApplicationWindow>* windows{};
    ApplicationResult* result{};
    void operator()() const { *result = Application::validate(*windows); }
};
struct RequestOnWorker final {
    ApplicationWindowHandle handle{};
    HostServiceStatus* result{};
    void operator()() const { *result = handle.request_close(); }
};
struct CountRequest final {
    unsigned* count{};
    void operator()() const { ++*count; }
};
struct ChangeHidePolicy final {
    void operator()(HostCloseRequest& request) const { request.hide_on_accept = false; }
};
struct CallbackLifetime final {
    bool& destroyed;
    explicit CallbackLifetime(bool& observed) : destroyed(observed) {}
    ~CallbackLifetime() { destroyed = true; }
    CallbackLifetime(const CallbackLifetime&) = delete;
    CallbackLifetime& operator=(const CallbackLifetime&) = delete;
};
struct CloseAndContinue final {
    std::weak_ptr<detail::ApplicationWindowState> state{};
    std::shared_ptr<CallbackLifetime> lifetime{};
    bool* destroyed{};
    bool* completed{};

    void operator()() const {
        // Cache observations before revoking the source callback. This fixture
        // detects premature destruction without accessing an expired functor.
        bool& was_destroyed = *destroyed;
        bool& did_complete = *completed;
        const std::shared_ptr<detail::ApplicationWindowState> active = state.lock();
        require(static_cast<bool>(active), "callback state expired");
        (*active).closed = true;
        (*active).request_close = {};
        require(!was_destroyed, "active callable destroyed during its own close");
        did_complete = true;
    }
};
void synchronous_close_lifetime() {
    bool destroyed = false;
    bool completed = false;
    const std::shared_ptr<detail::ApplicationWindowState> state =
        std::make_shared<detail::ApplicationWindowState>();
    (*state).ready = true;
    std::shared_ptr<CallbackLifetime> lifetime = std::make_shared<CallbackLifetime>(destroyed);
    (*state).request_close = CloseAndContinue{state, lifetime, &destroyed, &completed};
    lifetime.reset();
    const ApplicationWindowHandle handle = detail::ApplicationHandleAccess::make(state);
    const HostServiceStatus result = handle.request_close();
    require(result.accepted() && completed && destroyed && !handle.active(),
            "synchronous close did not retain and then release the active callable");
}
void validation() {
    std::vector<ApplicationWindow> windows{};
    require(!Application::validate(windows).accepted(), "empty application accepted");
    windows.push_back(make_window("main"));
    windows.push_back(make_window("tools", "main"));
    windows[1].options.initially_visible = false;
    windows[1].options.hide_on_close = true;
    windows[1].tool_window = true;
    require(Application::validate(windows).accepted(), "valid owned window rejected");
    windows[1].owner_id = "missing";
    require(Application::validate(windows).window_index == 1U, "missing owner not identified");
    windows[1].owner_id = "main";
    windows[0].owner_id = "tools";
    require(!Application::validate(windows).accepted(), "owner cycle accepted");
    windows[0].owner_id.clear();
    windows[1].stable_id = "main";
    require(!Application::validate(windows).accepted(), "duplicate ID accepted");
    windows[1].stable_id = "tools";
    windows[0].options.title.assign("bad\0title", 9U);
    require(!Application::validate(windows).accepted(), "embedded NUL accepted");
    windows[0].options.title = "\xc0\xaf";
    require(!Application::validate(windows).accepted(), "invalid UTF-8 accepted");
    windows[0].options.title = "Application";
    windows[0].options.initial_size.width = std::numeric_limits<double>::infinity();
    require(!Application::validate(windows).accepted(), "infinite geometry accepted");
    windows[0].options.initial_size.width = 320;
    windows[0].options.hide_on_close = true;
    require(!Application::validate(windows).accepted(), "hidden primary accepted");
    windows[0].options.hide_on_close = false;
    ApplicationResult worker{};
    std::thread thread(ValidateOnWorker{&windows, &worker});
    thread.join();
    require(worker.error == ApplicationError::wrong_thread, "foreign model thread accepted");
}
void handles() {
    const std::shared_ptr<detail::ApplicationWindowState> state = std::make_shared<detail::ApplicationWindowState>();
    const ApplicationWindowHandle handle = detail::ApplicationHandleAccess::make(state);
    unsigned closes = 0U, full_screens = 0U;
    (*state).request_close = CountRequest{&closes};
    const HostServiceStatus unready = handle.request_close();
    require(!handle.active() && unready.error == HostServiceError::backend_failure, "unready handle callable");
    (*state).ready = true;
    const HostServiceStatus close_requested = handle.request_close();
    require(handle.active() && close_requested.accepted() && closes == 1U, "live handle did not dispatch");
    const HostServiceStatus unsupported = handle.toggle_full_screen();
    require(unsupported.error == HostServiceError::unsupported, "missing full screen host must be explicit");
    (*state).toggle_full_screen = CountRequest{&full_screens};
    const HostServiceStatus toggled = handle.toggle_full_screen();
    require(toggled.accepted() && full_screens == 1U, "full screen request did not dispatch");
    const HostServiceStatus hidden = handle.hide();
    require(hidden.error == HostServiceError::invalid_argument, "primary could hide");
    HostServiceStatus worker{};
    std::thread thread(RequestOnWorker{handle, &worker});
    thread.join();
    require(worker.error == HostServiceError::wrong_thread && closes == 1U, "foreign thread reached native request");
    (*state).closed = true;
    const HostServiceStatus closed_request = handle.request_close();
    require(!handle.active() && closed_request.error == HostServiceError::after_shutdown, "closed handle callable");
    const HostServiceStatus closed_toggle = handle.toggle_full_screen();
    require(closed_toggle.error == HostServiceError::after_shutdown && full_screens == 1U, "closed handle toggled full screen");
    const ApplicationWindowHandle empty{};
    const HostServiceStatus empty_shown = empty.show();
    require(!empty.active() && empty_shown.error == HostServiceError::after_shutdown, "empty handle callable");
}
void reusable_hide() {
    const std::shared_ptr<Control> root = make_control<Control>(StableId("reusable.root"));
    Window window(root, {320, 240});
    HostSession session(window, {HostCapabilities::current_protocol_version, "test", HostCapability::lifecycle});
    HostEvent event{};
    event.sequence = 1U;
    event.payload = HostAttachEvent{{320, 240}, 1.0};
    const HostDispatchResult attached = session.dispatch(event);
    require(attached.accepted(), "attach failed");
    SubscriptionToken token = session.closing().subscribe(ChangeHidePolicy{});
    for (unsigned index = 0U; index < 3U; ++index) {
        ++event.sequence;
        event.payload = HostCloseRequest{HostCloseReason::user, false, true};
        const HostDispatchResult hidden = session.dispatch(event);
        require(hidden.accepted() && hidden.close_allowed && session.snapshot().phase == HostLifecyclePhase::attached,
                "hide authorization terminated the attached session");
        ++event.sequence;
        event.payload = HostActivationEvent{true};
        const HostDispatchResult activated = session.dispatch(event);
        require(activated.accepted(), "reopened session rejected activation");
    }
    ++event.sequence;
    event.payload = HostCloseRequest{HostCloseReason::user, false};
    const HostDispatchResult closed = session.dispatch(event);
    require(closed.close_allowed && session.snapshot().phase == HostLifecyclePhase::close_authorized,
            "ordinary close did not authorize termination");
}
}
int main() {
    validation();
    handles();
    reusable_hide();
    synchronous_close_lifetime();
    std::cout << "application validation, weak handles, affinity, and reusable hide passed\n";
}
