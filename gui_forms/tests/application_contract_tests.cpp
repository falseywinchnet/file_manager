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
    ApplicationWindow entry;
    entry.stable_id = std::move(id);
    entry.owner_id = std::move(owner);
    entry.model = std::make_unique<Window>(make_control<Control>(StableId("root")), Size{320, 240});
    return entry;
}
struct ValidateOnWorker final {
    const std::vector<ApplicationWindow>* windows{};
    ApplicationResult* result{};
    void operator()() const { *result = Application::validate(*windows); }
};
struct RequestOnWorker final {
    ApplicationWindowHandle handle;
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
void validation() {
    std::vector<ApplicationWindow> windows;
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
    ApplicationResult worker;
    std::thread thread(ValidateOnWorker{&windows, &worker});
    thread.join();
    require(worker.error == ApplicationError::wrong_thread, "foreign model thread accepted");
}
void handles() {
    const std::shared_ptr<detail::ApplicationWindowState> state = std::make_shared<detail::ApplicationWindowState>();
    const ApplicationWindowHandle handle = detail::ApplicationHandleAccess::make(state);
    unsigned closes = 0U, full_screens = 0U;
    (*state).request_close = CountRequest{&closes};
    require(!handle.active() && handle.request_close().error == HostServiceError::backend_failure, "unready handle callable");
    (*state).ready = true;
    require(handle.active() && handle.request_close().accepted() && closes == 1U, "live handle did not dispatch");
    require(handle.toggle_full_screen().error == HostServiceError::unsupported, "missing full screen host must be explicit");
    (*state).toggle_full_screen = CountRequest{&full_screens};
    require(handle.toggle_full_screen().accepted() && full_screens == 1U, "full screen request did not dispatch");
    require(handle.hide().error == HostServiceError::invalid_argument, "primary could hide");
    HostServiceStatus worker;
    std::thread thread(RequestOnWorker{handle, &worker});
    thread.join();
    require(worker.error == HostServiceError::wrong_thread && closes == 1U, "foreign thread reached native request");
    (*state).closed = true;
    require(!handle.active() && handle.request_close().error == HostServiceError::after_shutdown, "closed handle callable");
    require(handle.toggle_full_screen().error == HostServiceError::after_shutdown && full_screens == 1U, "closed handle toggled full screen");
    const ApplicationWindowHandle empty;
    require(!empty.active() && empty.show().error == HostServiceError::after_shutdown, "empty handle callable");
}
void reusable_hide() {
    const std::shared_ptr<Control> root = make_control<Control>(StableId("reusable.root"));
    Window window(root, {320, 240});
    HostSession session(window, {HostCapabilities::current_protocol_version, "test", HostCapability::lifecycle});
    HostEvent event;
    event.sequence = 1U;
    event.payload = HostAttachEvent{{320, 240}, 1.0};
    require(session.dispatch(event).accepted(), "attach failed");
    SubscriptionToken token = session.closing().subscribe(ChangeHidePolicy{});
    for (unsigned index = 0U; index < 3U; ++index) {
        ++event.sequence;
        event.payload = HostCloseRequest{HostCloseReason::user, false, true};
        const HostDispatchResult hidden = session.dispatch(event);
        require(hidden.accepted() && hidden.close_allowed && session.snapshot().phase == HostLifecyclePhase::attached,
                "hide authorization terminated the attached session");
        ++event.sequence;
        event.payload = HostActivationEvent{true};
        require(session.dispatch(event).accepted(), "reopened session rejected activation");
    }
    ++event.sequence;
    event.payload = HostCloseRequest{HostCloseReason::user, false};
    require(session.dispatch(event).close_allowed && session.snapshot().phase == HostLifecyclePhase::close_authorized,
            "ordinary close did not authorize termination");
}
}
int main() {
    validation(); handles(); reusable_hide();
    std::cout << "application validation, weak handles, affinity, and reusable hide passed\n";
}
