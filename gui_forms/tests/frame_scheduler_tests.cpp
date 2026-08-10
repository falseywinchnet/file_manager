#include "gui_forms/gui_forms.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

namespace {

using namespace gui_forms;
using namespace std::chrono_literals;

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

class NullPainter final : public Painter {
public:
    void save() override {}
    void restore() override {}
    void translate(Point) override {}
    void clip_rect(Rect) override {}
    void fill_rect(Rect, Color) override {}
    void stroke_rect(Rect, Color, double) override {}
    void draw_line(Point, Point, Color, double) override {}
    void draw_text_utf8(Point, std::string_view, FontSpec, Color) override {}
    void draw_image(ImageId, Rect, double) override {}
};

class ThrowingFrameControl final : public Control {
public:
    explicit ThrowingFrameControl(StableId id) : Control(std::move(id)) {}

    void on_frame(FrameTime) override {
        ++frame_calls;
        throw std::runtime_error("intentional frame callback fault");
    }

    std::uint64_t frame_calls{};
};

class ReentrantFrameControl final : public Control {
public:
    explicit ReentrantFrameControl(StableId id) : Control(std::move(id)) {}

    void on_frame(FrameTime now) override {
        ++frame_calls;
        if (callback) callback(now);
    }

    std::function<void(FrameTime)> callback;
    std::uint64_t frame_calls{};
};

struct Fixture {
    Fixture() {
        (*root).set_requested_bounds({0.0, 0.0, 160.0, 90.0});
        window = std::make_unique<Window>(root, Size{160.0, 90.0});
        (*window).perform_layout();
        DamageRegion initial = (*window).take_damage();
        (*window).paint(painter, initial.bounds());
        (*window).reset_activity_metrics();
    }

    void consume_frame() {
        DamageRegion damage = (*window).take_damage();
        if (!damage.empty()) {
            (*window).paint(painter, damage.bounds());
        }
    }

    Control::Ptr root = make_control<Control>(StableId("scheduler.root"));
    std::unique_ptr<Window> window;
    NullPainter painter;
};

class ScheduleDuringFrameCallback final {
public:
    ScheduleDuringFrameCallback(FrameRequestToken& cancelled_request,
                                FrameRequestToken& next_request,
                                Window& window,
                                const std::shared_ptr<ReentrantFrameControl>& admitted_next,
                                FramePollResult& nested)
        : cancelled_request_(cancelled_request), next_request_(next_request),
          window_(window), admitted_next_(admitted_next), nested_(nested) {}

    void operator()(FrameTime now) const {
        cancelled_request_.disconnect();
        next_request_ = window_.schedule_paint(admitted_next_, now);
        nested_ = window_.poll_frame_schedule(now);
    }

private:
    FrameRequestToken& cancelled_request_;
    FrameRequestToken& next_request_;
    Window& window_;
    const std::shared_ptr<ReentrantFrameControl>& admitted_next_;
    FramePollResult& nested_;
};

class SchedulePaintOffThread final {
public:
    SchedulePaintOffThread(Window& window, const Control::Ptr& root,
                           std::atomic<bool>& rejected)
        : window_(window), root_(root), rejected_(rejected) {}

    void operator()() const {
        try {
            static_cast<void>(window_.schedule_paint(root_, FrameTime{}));
        } catch (const std::logic_error&) {
            rejected_.store(true, std::memory_order_relaxed);
        }
    }

private:
    Window& window_;
    const Control::Ptr& root_;
    std::atomic<bool>& rejected_;
};

void test_deadline_is_exact_and_one_shot() {
    Fixture fixture;
    const FrameTime base{};
    FrameRequestToken deadline =
        (*fixture.window).schedule_paint(fixture.root, base + 50ms);
    require(deadline.connected() && (*fixture.window).next_wake() == base + 50ms &&
                !(*fixture.window).needs_frame(),
            "a future deadline must request a wake without immediate paint");

    FramePollResult early = (*fixture.window).poll_frame_schedule(base + 49ms);
    require(early.deadlines_fired == 0 && !early.damage_pending &&
                early.next_wake == base + 50ms && deadline.connected(),
            "polling before a deadline must preserve it exactly");

    FramePollResult due = (*fixture.window).poll_frame_schedule(base + 50ms);
    require(due.deadlines_fired == 1 && due.damage_pending &&
                !due.next_wake.has_value() && !deadline.connected(),
            "a due one-shot deadline must invalidate once and disconnect");
    fixture.consume_frame();
    require(!(*fixture.window).needs_frame(),
            "painting deadline damage must return the scheduler to idle");
}

void test_same_target_deadlines_coalesce() {
    Fixture fixture;
    const FrameTime due = FrameTime{} + 10ms;
    FrameRequestToken first = (*fixture.window).schedule_paint(fixture.root, due);
    FrameRequestToken second = (*fixture.window).schedule_paint(fixture.root, due);
    const FramePollResult poll = (*fixture.window).poll_frame_schedule(due);
    require(poll.deadlines_fired == 2 && poll.coalesced_requests == 1,
            "same-target deadlines must produce one retained invalidation");
    require(!first.connected() && !second.connected(),
            "all fired one-shot deadline tokens must disconnect");
    const MetricsSnapshot metrics = (*fixture.window).metrics_snapshot();
    require(metrics.scheduled_frame_requests == 2 && metrics.scheduler_wakes == 1 &&
                metrics.frame_deadlines_fired == 2 &&
                metrics.frame_requests_coalesced == 1,
            "deadline coalescing metrics must remain truthful");
}

void test_active_surface_skips_catch_up_bursts() {
    Fixture fixture;
    const FrameTime first = FrameTime{} + 100ms;
    constexpr FrameInterval interval = 33ms;
    FrameRequestToken active =
        (*fixture.window).activate_surface(fixture.root, interval, first);
    require(active.connected() && (*fixture.window).next_wake() == first &&
                (*fixture.window).metrics_snapshot().active_surface_count == 1,
            "active surface must expose one bounded wake");

    FramePollResult initial = (*fixture.window).poll_frame_schedule(first);
    require(initial.active_surface_ticks == 1 && initial.coalesced_requests == 0 &&
                initial.next_wake == first + interval,
            "first active tick must advance by one interval");
    fixture.consume_frame();

    const FrameTime late = first + interval * 10;
    FramePollResult delayed = (*fixture.window).poll_frame_schedule(late);
    require(delayed.active_surface_ticks == 1 && delayed.coalesced_requests == 9 &&
                delayed.next_wake == late + interval,
            "late active polling must emit one frame and skip missed intervals");
    fixture.consume_frame();
    active.disconnect();
    static_cast<void>((*fixture.window).poll_frame_schedule(late));
    require(!(*fixture.window).next_wake().has_value() &&
                (*fixture.window).metrics_snapshot().active_surface_count == 0 &&
                !(*fixture.window).needs_frame(),
            "disconnecting the active lease must restore quiescence");
}

void test_throwing_frame_callback_isolated_and_disconnected() {
    std::shared_ptr<gui_forms::Control> root = make_control<Control>(StableId("scheduler.fault.root"));
    std::shared_ptr<ThrowingFrameControl> throwing =
        make_control<ThrowingFrameControl>(StableId("scheduler.fault.throwing"));
    std::shared_ptr<gui_forms::Control> healthy = make_control<Control>(StableId("scheduler.fault.healthy"));
    (*root).set_requested_bounds({0.0, 0.0, 160.0, 90.0});
    (*throwing).set_requested_bounds({0.0, 0.0, 70.0, 40.0});
    (*healthy).set_requested_bounds({80.0, 0.0, 70.0, 40.0});
    (*root).add_child(throwing);
    (*root).add_child(healthy);
    Window window(root, {160.0, 90.0});
    NullPainter painter;
    window.paint(painter, window.take_damage().bounds());
    window.reset_activity_metrics();
    const FrameTime due = FrameTime{} + 33ms;
    FrameRequestToken bad = window.activate_surface(throwing, 33ms, due);
    FrameRequestToken good = window.activate_surface(healthy, 33ms, due);

    const FramePollResult result = window.poll_frame_schedule(due);
    const MetricsSnapshot metrics = window.metrics_snapshot();
    require(result.active_surface_ticks == 2U &&
                result.callback_faults == 1U &&
                (*throwing).frame_calls == 1U && !bad.connected() &&
                good.connected() && metrics.frame_callback_faults == 1U &&
                metrics.active_surface_count == 1U && result.damage_pending,
            "one throwing animation callback must disconnect itself, report the fault, and preserve a healthy peer");
    window.paint(painter, window.take_damage().bounds());
    const FramePollResult later = window.poll_frame_schedule(due + 33ms);
    require(later.callback_faults == 0U && later.active_surface_ticks == 1U &&
                (*throwing).frame_calls == 1U && good.connected(),
            "a faulted frame request must not crash or retry on later native timer turns");
}

void test_frame_callbacks_use_fixed_due_set_and_defer_nested_poll() {
    std::shared_ptr<gui_forms::Control> root = make_control<Control>(StableId("scheduler.reentry.root"));
    std::shared_ptr<ReentrantFrameControl> controller =
        make_control<ReentrantFrameControl>(
        StableId("scheduler.reentry.controller"));
    std::shared_ptr<ReentrantFrameControl> cancelled =
        make_control<ReentrantFrameControl>(
        StableId("scheduler.reentry.cancelled"));
    std::shared_ptr<ReentrantFrameControl> admitted_next =
        make_control<ReentrantFrameControl>(
        StableId("scheduler.reentry.next"));
    (*root).add_child(controller);
    (*root).add_child(cancelled);
    (*root).add_child(admitted_next);
    Window window(root, {160.0, 90.0});
    const FrameTime due = FrameTime{} + 20ms;
    FrameRequestToken cancelled_request;
    FrameRequestToken next_request;
    FramePollResult nested;
    (*controller).callback = ScheduleDuringFrameCallback(
        cancelled_request, next_request, window, admitted_next, nested);
    FrameRequestToken controller_request = window.schedule_paint(controller, due);
    cancelled_request = window.schedule_paint(cancelled, due);

    const FramePollResult first = window.poll_frame_schedule(due);
    require(first.deadlines_fired == 1U && (*controller).frame_calls == 1U &&
                (*cancelled).frame_calls == 0U && (*admitted_next).frame_calls == 0U &&
                nested.reentrant_poll_deferred && next_request.connected() &&
                first.next_wake == due,
            "frame callbacks must cancel later peers and admit new due work only to the next poll");
    const FramePollResult second = window.poll_frame_schedule(due);
    require(second.deadlines_fired == 1U && (*admitted_next).frame_calls == 1U &&
                !next_request.connected() &&
                window.metrics_snapshot().reentrant_frame_polls_deferred == 1U,
            "a deferred nested poll must preserve one next-turn delivery and diagnostic");
}

void test_owner_disposal_revokes_active_surface() {
    Fixture fixture;
    FrameRequestToken active = (*fixture.window).activate_surface(
        fixture.root, 33ms, FrameTime{} + 33ms);
    (*fixture.root).dispose();
    require(!active.connected(),
            "component disposal must synchronously revoke its active surface");
    static_cast<void>((*fixture.window).poll_frame_schedule(FrameTime{} + 33ms));
    require(!(*fixture.window).next_wake().has_value() &&
                (*fixture.window).metrics_snapshot().active_surface_count == 0,
            "disposed active surfaces must leave no latent wake");
}

void test_occlusion_pauses_and_rebases_active_surface() {
    Fixture fixture;
    const FrameTime base{};
    constexpr FrameInterval interval = 33ms;
    FrameRequestToken active = (*fixture.window).activate_surface(
        fixture.root, interval, base + 100ms);

    (*fixture.window).set_occluded(true, base + 90ms);
    require((*fixture.window).occluded() && !(*fixture.window).next_wake().has_value() &&
                active.connected(),
            "occlusion must suppress wakes without revoking the active surface");
    const FramePollResult hidden =
        (*fixture.window).poll_frame_schedule(base + 500ms);
    require(hidden.suppressed_by_occlusion && hidden.active_surface_ticks == 0 &&
                !hidden.next_wake.has_value() && active.connected(),
            "polling while occluded must not fire or disconnect retained work");

    (*fixture.window).set_occluded(false, base + 500ms);
    require(!(*fixture.window).occluded() &&
                (*fixture.window).next_wake() == base + 500ms,
            "resume must rebase one overdue active tick to the transition time");
    const FramePollResult resumed =
        (*fixture.window).poll_frame_schedule(base + 500ms);
    require(resumed.active_surface_ticks == 1 && resumed.coalesced_requests == 0 &&
                resumed.next_wake == base + 500ms + interval,
            "resume must emit one frame and schedule from now without catch-up bursts");
    const MetricsSnapshot metrics = (*fixture.window).metrics_snapshot();
    require(metrics.occlusion_suspensions == 1 && metrics.occlusion_resumes == 1 &&
                metrics.occluded_frame_polls == 1 &&
                metrics.active_surface_ticks == 1,
            "occlusion and resumed work must remain explicitly accounted");
}

void test_thirty_tick_band_stays_localized() {
    std::shared_ptr<gui_forms::Control> root = make_control<Control>(StableId("band.root"));
    std::shared_ptr<gui_forms::Control> band = make_control<Control>(StableId("band.active"));
    std::shared_ptr<gui_forms::Control> sibling = make_control<Control>(StableId("band.sibling"));
    (*root).set_requested_bounds({0.0, 0.0, 200.0, 100.0});
    (*band).set_requested_bounds({10.0, 20.0, 60.0, 10.0});
    (*sibling).set_requested_bounds({120.0, 20.0, 60.0, 10.0});
    (*root).add_child(band);
    (*root).add_child(sibling);
    Window window(root, {200.0, 100.0});
    NullPainter painter;
    window.perform_layout();
    DamageRegion initial = window.take_damage();
    window.paint(painter, initial.bounds());
    window.reset_activity_metrics();

    constexpr FrameInterval interval = std::chrono::nanoseconds(33'333'333);
    const FrameTime base{};
    FrameRequestToken active =
        window.activate_surface(band, interval, base + interval);
    for (int tick = 1; tick <= 30; ++tick) {
        const FramePollResult poll =
            window.poll_frame_schedule(base + interval * tick);
        require(poll.active_surface_ticks == 1 && poll.coalesced_requests == 0,
                "on-cadence active band must emit exactly one request");
        DamageRegion damage = window.take_damage();
        require(damage.bounds() == Rect{10.0, 20.0, 60.0, 10.0},
                "active band damage must remain at its exact arranged bounds");
        window.paint(painter, damage.bounds());
    }

    const MetricsSnapshot metrics = window.metrics_snapshot();
    require(metrics.active_surface_ticks == 30 &&
                metrics.display_chunks_rebuilt == 30 &&
                metrics.paint_invalidations_consumed == 30,
            "30 active ticks must account for 30 localized chunk rebuilds");
    require(metrics.partial_paints == 30 && metrics.full_window_paints == 0 &&
                metrics.painted_damage_area == 18'000.0,
            "30 active ticks must never expand the 600-pixel band damage");
    active.disconnect();
    static_cast<void>(window.poll_frame_schedule(base + interval * 30));
    require(!window.next_wake().has_value() && !window.needs_frame(),
            "completed 30-tick workload must return to idle");
}

void test_active_surface_bounds_and_thread_affinity() {
    Fixture fixture;
    bool cadence_rejected = false;
    try {
        static_cast<void>((*fixture.window).activate_surface(
            fixture.root, minimum_active_surface_interval - 1ns, FrameTime{}));
    } catch (const std::invalid_argument&) {
        cadence_rejected = true;
    }
    require(cadence_rejected, "sub-bound active cadence must be rejected");

    std::vector<FrameRequestToken> leases;
    leases.reserve(maximum_active_surfaces);
    for (std::size_t index = 0; index < maximum_active_surfaces; ++index) {
        leases.push_back((*fixture.window).activate_surface(
            fixture.root, 33ms, FrameTime{} + 33ms));
    }
    bool quota_rejected = false;
    try {
        static_cast<void>((*fixture.window).activate_surface(
            fixture.root, 33ms, FrameTime{} + 33ms));
    } catch (const std::length_error&) {
        quota_rejected = true;
    }
    require(quota_rejected &&
                (*fixture.window).metrics_snapshot().maximum_active_surface_count ==
                    maximum_active_surfaces,
            "active surface quota must reject unbounded registrations");

    for (gui_forms::FrameRequestToken& lease : leases) {
        lease.disconnect();
    }
    static_cast<void>((*fixture.window).poll_frame_schedule(FrameTime{}));

    std::atomic<bool> rejected{false};
    std::thread worker(SchedulePaintOffThread(
        *fixture.window, fixture.root, rejected));
    worker.join();
    require(rejected.load(std::memory_order_relaxed) &&
                (*fixture.window).metrics_snapshot().rejected_wrong_thread_operations == 1,
            "wrong-thread scheduling must reject without adding a request");
}

} // namespace

int main() {
    try {
        test_deadline_is_exact_and_one_shot();
        test_same_target_deadlines_coalesce();
        test_active_surface_skips_catch_up_bursts();
        test_throwing_frame_callback_isolated_and_disconnected();
        test_frame_callbacks_use_fixed_due_set_and_defer_nested_poll();
        test_owner_disposal_revokes_active_surface();
        test_occlusion_pauses_and_rebases_active_surface();
        test_thirty_tick_band_stays_localized();
        test_active_surface_bounds_and_thread_affinity();
        std::cout << "gui_forms_frame_scheduler_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_frame_scheduler_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
