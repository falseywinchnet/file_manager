#include "gui_forms/gui_forms.hpp"
#include "headless_host.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
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

class DispatchPhaseProbe final : public Control {
public:
    DispatchPhaseProbe(StableId stable_id, std::vector<std::string>& trace)
        : Control(std::move(stable_id)), trace_(trace) {}

    Size measure(Size available) override {
        if (recording) trace_.push_back("measure");
        return Control::measure(available);
    }

    void arrange(Rect bounds) override {
        if (recording) trace_.push_back("arrange");
        Control::arrange(bounds);
    }

    void on_paint(Painter&, Rect) override {
        if (recording) trace_.push_back("paint");
    }

    void on_pointer(PointerEvent& event) override {
        if (!recording || event.action != PointerAction::down) return;
        trace_.push_back("input");
        const auto weak = weak_from_this();
        static_cast<void>(begin_invoke([weak, this] {
            trace_.push_back("dispatch");
            if (const auto control = weak.lock()) {
                control->set_requested_bounds({0.0, 0.0, 319.0, 179.0});
            }
        }));
        event.handled = true;
    }

    void on_frame(FrameTime) override {
        if (recording) trace_.push_back("frame");
    }

    bool recording{};

private:
    std::vector<std::string>& trace_;
};

template <typename Predicate>
void require_eventually(Predicate predicate, const char* message) {
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::seconds(2);
    while (!predicate()) {
        if (std::chrono::steady_clock::now() >= deadline) {
            throw std::runtime_error(message);
        }
        std::this_thread::yield();
    }
}

void test_fifo_snapshot_and_nested_deferral() {
    auto root = make_control<Control>(StableId("dispatcher.root"));
    Window window(root, {320.0, 180.0});
    std::uint64_t wake_count = 0U;
    window.set_dispatch_wake_handler([&wake_count] { ++wake_count; });

    std::string trace;
    DispatchOperation nested;
    DispatchOperation first = window.begin_invoke([&] {
        trace += 'A';
        nested = window.begin_invoke([&] { trace += 'D'; });
    });
    DispatchOperation second = window.begin_invoke([&] { trace += 'B'; });
    DispatchOperation third = window.begin_invoke([&] { trace += 'C'; });
    require(trace.empty() && wake_count == 1U,
            "BeginInvoke must remain posted and coalesce one host wake");
    const DispatcherSnapshot queued = window.dispatcher_snapshot();
    require(queued.posted == 3U && queued.pending == 3U &&
                queued.maximum_pending == 3U && queued.wake_requests == 1U &&
                queued.coalesced_wakes == 2U,
            "dispatcher telemetry must describe the pending FIFO batch");

    const DispatchDrainResult first_turn = window.drain_posted_work();
    require(trace == "ABC" && first_turn.invoked == 3U &&
                first_turn.remaining == 1U && first_turn.wake_requested &&
                wake_count == 2U && first.state() == DispatchOperationState::completed &&
                second.state() == DispatchOperationState::completed &&
                third.state() == DispatchOperationState::completed && nested.pending(),
            "a dispatch turn must use a FIFO snapshot and defer nested work");
    const DispatchDrainResult second_turn = window.drain_posted_work();
    require(trace == "ABCD" && second_turn.invoked == 1U &&
                second_turn.remaining == 0U &&
                nested.state() == DispatchOperationState::completed,
            "nested BeginInvoke work must run on the next explicit turn");
}

void test_cancellation_owner_lifetime_and_fault_isolation() {
    auto root = make_control<Control>(StableId("dispatcher.owner.root"));
    auto child = make_control<Control>(StableId("dispatcher.owner.child"));
    root->add_child(child);
    Window window(root, {320.0, 180.0});

    std::uint64_t callbacks = 0U;
    DispatchOperation explicit_cancel = window.begin_invoke([&] { ++callbacks; });
    require(explicit_cancel.cancel() && !explicit_cancel.cancel(),
            "posted cancellation must be explicit and idempotent");
    DispatchOperation owner_cancel = child->begin_invoke([&] { ++callbacks; });
    const Control::Ptr detached = root->remove_child(child->runtime_id());
    require(detached == child && !child->attached() && !child->invoke_required(),
            "detached controls must lose dispatcher affinity atomically");

    DispatchOperation fault = window.begin_invoke([] {
        throw std::runtime_error("dispatcher fixture fault");
    });
    DispatchOperation survivor = window.begin_invoke([&] { ++callbacks; });
    const DispatchDrainResult result = window.drain_posted_work();
    require(callbacks == 1U && result.invoked == 1U && result.cancelled == 2U &&
                result.faulted == 1U &&
                explicit_cancel.state() == DispatchOperationState::cancelled &&
                owner_cancel.state() == DispatchOperationState::cancelled &&
                fault.state() == DispatchOperationState::faulted &&
                fault.exception() != nullptr &&
                survivor.state() == DispatchOperationState::completed,
            "cancelled/ownerless/faulted work must not suppress later callbacks");

    bool detached_rejected = false;
    try {
        static_cast<void>(child->begin_invoke([] {}));
    } catch (const std::logic_error&) {
        detached_rejected = true;
    }
    require(detached_rejected,
            "BeginInvoke on a detached control must reject instead of orphaning work");
}

void test_cross_thread_post_and_headless_pump() {
    auto root = make_control<Control>(StableId("dispatcher.thread.root"));
    Window window(root, {320.0, 180.0});
    host::HeadlessHost host(window);
    std::atomic<bool> worker_saw_affinity{false};
    std::atomic<bool> callback_on_ui{false};
    DispatchOperation operation;
    const std::thread::id ui_thread = std::this_thread::get_id();
    std::thread worker([&] {
        worker_saw_affinity.store(window.invoke_required() &&
                                      root->invoke_required(),
                                  std::memory_order_release);
        operation = root->begin_invoke([&] {
            callback_on_ui.store(std::this_thread::get_id() == ui_thread,
                                 std::memory_order_release);
        });
    });
    worker.join();
    require(worker_saw_affinity.load(std::memory_order_acquire) &&
                operation.pending() &&
                !callback_on_ui.load(std::memory_order_acquire),
            "worker BeginInvoke must publish affinity without executing inline");
    const DispatchDrainResult result = host.pump_dispatcher();
    require(result.invoked == 1U &&
                callback_on_ui.load(std::memory_order_acquire) &&
                operation.state() == DispatchOperationState::completed,
            "headless pump must marshal worker work to the UI thread");
}

void test_synchronous_invoke_inline_marshal_fault_and_owner_cancel() {
    auto root = make_control<Control>(StableId("dispatcher.invoke.root"));
    auto child = make_control<Control>(StableId("dispatcher.invoke.child"));
    root->add_child(child);
    Window window(root, {320.0, 180.0});
    host::HeadlessHost host(window);

    std::string trace;
    window.invoke([&] { trace += 'A'; });
    child->invoke([&] { trace += 'B'; });
    bool inline_fault = false;
    try {
        window.invoke([] { throw std::runtime_error("inline fault"); });
    } catch (const std::runtime_error& error) {
        inline_fault = std::string(error.what()) == "inline fault";
    }
    require(trace == "AB" && inline_fault &&
                window.dispatcher_snapshot().inline_invocations == 3U,
            "UI-thread Invoke must execute inline and preserve exception identity");

    std::atomic<bool> marshalled_returned{false};
    std::atomic<bool> marshalled_on_ui{false};
    const std::thread::id ui_thread = std::this_thread::get_id();
    std::thread marshalled([&] {
        child->invoke([&] {
            trace += 'C';
            marshalled_on_ui.store(std::this_thread::get_id() == ui_thread,
                                   std::memory_order_release);
        });
        marshalled_returned.store(true, std::memory_order_release);
    });
    require_eventually(
        [&] {
            return host.dispatcher_wake_pending() &&
                   window.dispatcher_snapshot().pending == 1U;
        },
        "worker Invoke did not publish a bounded headless wake");
    require(!marshalled_returned.load(std::memory_order_acquire),
            "worker Invoke must block without pumping a nested loop");
    const DispatchDrainResult marshalled_turn = host.pump_dispatcher();
    marshalled.join();
    require(marshalled_turn.invoked == 1U && trace == "ABC" &&
                marshalled_on_ui.load(std::memory_order_acquire) &&
                marshalled_returned.load(std::memory_order_acquire),
            "headless host must release worker Invoke only after UI execution");

    std::string worker_fault;
    std::thread faulted([&] {
        try {
            window.invoke([] { throw std::runtime_error("worker fault"); });
        } catch (const std::runtime_error& error) {
            worker_fault = error.what();
        }
    });
    require_eventually(
        [&] { return window.dispatcher_snapshot().pending == 1U; },
        "faulting worker Invoke was not queued");
    const DispatchDrainResult fault_turn = host.pump_dispatcher();
    faulted.join();
    require(fault_turn.faulted == 1U && worker_fault == "worker fault",
            "worker Invoke must rethrow the original callback exception on its caller");

    std::atomic<bool> owner_cancelled{false};
    std::thread owner_waiter([&] {
        try {
            child->invoke([] {});
        } catch (const DispatchCancelledError&) {
            owner_cancelled.store(true, std::memory_order_release);
        }
    });
    require_eventually(
        [&] { return window.dispatcher_snapshot().pending == 1U; },
        "owned worker Invoke was not queued");
    const Control::Ptr detached = root->remove_child(child->runtime_id());
    const DispatchDrainResult cancelled_turn = host.pump_dispatcher();
    owner_waiter.join();
    const DispatcherSnapshot snapshot = window.dispatcher_snapshot();
    require(detached == child && cancelled_turn.cancelled == 1U &&
                owner_cancelled.load(std::memory_order_acquire) &&
                snapshot.synchronous_invocations == 6U &&
                snapshot.inline_invocations == 3U &&
                snapshot.marshalled_invocations == 3U,
            "owner detachment must release synchronous waiters with cancellation");
}

void test_synchronous_invoke_host_and_shutdown_guards() {
    auto unhosted_root = make_control<Control>(
        StableId("dispatcher.invoke.unhosted"));
    Window unhosted(unhosted_root, {200.0, 100.0});
    std::atomic<bool> unhosted_rejected{false};
    std::thread no_host([&] {
        try {
            unhosted.invoke([] {});
        } catch (const std::logic_error&) {
            unhosted_rejected.store(true, std::memory_order_release);
        }
    });
    no_host.join();
    require(unhosted_rejected.load(std::memory_order_acquire) &&
                unhosted.dispatcher_snapshot().pending == 0U,
            "worker Invoke without a running host must reject instead of deadlocking");

    auto root = make_control<Control>(StableId("dispatcher.invoke.shutdown"));
    Window window(root, {200.0, 100.0});
    host::HeadlessHost host(window);
    std::atomic<bool> shutdown_cancelled{false};
    std::thread waiter([&] {
        try {
            window.invoke([] {});
        } catch (const DispatchCancelledError&) {
            shutdown_cancelled.store(true, std::memory_order_release);
        }
    });
    require_eventually(
        [&] { return window.dispatcher_snapshot().pending == 1U; },
        "shutdown fixture worker Invoke was not queued");
    bool removal_rejected = false;
    try {
        window.set_dispatch_wake_handler({});
    } catch (const std::logic_error&) {
        removal_rejected = true;
    }
    window.shutdown_dispatcher();
    waiter.join();
    require(removal_rejected &&
                shutdown_cancelled.load(std::memory_order_acquire) &&
                !window.dispatcher_snapshot().accepting,
            "wake removal/shutdown must not strand a synchronous waiter");
}

void test_concurrent_producer_fifo_and_turn_bound() {
    auto root = make_control<Control>(StableId("dispatcher.stress.root"));
    Window window(root, {320.0, 180.0});
    struct Record final {
        std::uint64_t sequence{};
        std::size_t producer{};
        std::size_t index{};
    };
    constexpr std::size_t producer_count = 8U;
    constexpr std::size_t callbacks_per_producer = 129U;
    constexpr std::size_t callback_count =
        producer_count * callbacks_per_producer;
    std::mutex records_mutex;
    std::vector<Record> expected;
    expected.reserve(callback_count);
    std::vector<Record> observed;
    observed.reserve(callback_count);
    std::vector<std::thread> producers;
    producers.reserve(producer_count);
    for (std::size_t producer = 0U; producer < producer_count; ++producer) {
        producers.emplace_back([&, producer] {
            for (std::size_t index = 0U; index < callbacks_per_producer; ++index) {
                auto record = std::make_shared<Record>(
                    Record{0U, producer, index});
                DispatchOperation operation = window.begin_invoke([&, record] {
                    observed.push_back(*record);
                });
                record->sequence = operation.sequence();
                std::scoped_lock lock(records_mutex);
                expected.push_back(*record);
            }
        });
    }
    for (auto& producer : producers) producer.join();
    std::sort(expected.begin(), expected.end(), [](const Record& left,
                                                   const Record& right) {
        return left.sequence < right.sequence;
    });
    require(expected.size() == callback_count &&
                window.dispatcher_snapshot().pending == callback_count,
            "all concurrent producers must publish one bounded global FIFO");

    const DispatchDrainResult first = window.drain_posted_work();
    require(first.invoked == maximum_callbacks_per_dispatch_turn &&
                first.remaining == callback_count -
                    maximum_callbacks_per_dispatch_turn,
            "a saturated turn must stop at the public callback bound");
    const DispatchDrainResult second = window.drain_posted_work();
    require(second.invoked == callback_count -
                    maximum_callbacks_per_dispatch_turn &&
                second.remaining == 0U && observed.size() == callback_count,
            "the next turn must drain the exact saturated remainder");
    for (std::size_t index = 0U; index < callback_count; ++index) {
        require(observed[index].sequence == expected[index].sequence &&
                    observed[index].producer == expected[index].producer &&
                    observed[index].index == expected[index].index,
                "concurrent posting must execute in assigned global sequence order");
    }
    for (std::size_t producer = 0U; producer < producer_count; ++producer) {
        std::size_t prior = 0U;
        bool seen = false;
        for (const Record& record : observed) {
            if (record.producer != producer) continue;
            require(!seen || record.index == prior + 1U,
                    "each producer's program order must survive interleaving");
            prior = record.index;
            seen = true;
        }
    }
}

void test_dispatch_order_against_input_timer_layout_and_paint() {
    std::vector<std::string> trace;
    auto root = make_control<DispatchPhaseProbe>(
        StableId("dispatcher.phase.root"), trace);
    root->set_focusable(true);
    Window window(root, {320.0, 180.0});
    host::HeadlessHost host(window);
    NullPainter painter;
    window.flush();
    window.paint(painter);
    trace.clear();
    root->recording = true;

    require(window.dispatch_pointer(
                {PointerAction::down, PointerButton::primary, {20.0, 20.0}}) &&
                trace == std::vector<std::string>{"input"} &&
                window.dispatcher_snapshot().pending == 1U,
            "routed input must finish before work it posts can execute");
    static_cast<void>(host.pump_dispatcher());
    require(trace == std::vector<std::string>({"input", "dispatch"}),
            "posted input work must execute on the following dispatcher turn");
    window.flush();
    require(trace.size() >= 4U && trace[2] == "measure" &&
                trace[3] == "arrange",
            "dispatch mutations must enter layout only after the callback completes");
    window.paint(painter);
    require(!trace.empty() && trace.back() == "paint",
            "paint must consume the layout committed by posted work");

    trace.clear();
    Timer timer(window, std::chrono::milliseconds(1));
    SubscriptionToken tick = timer.tick().subscribe(timer, [&] {
        trace.push_back("timer");
        static_cast<void>(root->begin_invoke([&] {
            trace.push_back("timer-dispatch");
        }));
    });
    const FrameTime due = FrameClock::now() + std::chrono::milliseconds(1);
    timer.start_at(due);
    const FramePollResult timer_turn = window.poll_frame_schedule(due);
    require(timer_turn.ui_timer_ticks == 1U &&
                trace == std::vector<std::string>{"timer"} &&
                window.dispatcher_snapshot().pending == 1U,
            "timer callbacks must finish before their posted work runs");
    static_cast<void>(host.pump_dispatcher());
    require(trace == std::vector<std::string>({"timer", "timer-dispatch"}),
            "timer-posted work must run in a later dispatcher turn");
    timer.stop();
    tick.disconnect();

    trace.clear();
    FrameRequestToken animation = window.activate_surface(
        root, std::chrono::milliseconds(16), due + std::chrono::milliseconds(16));
    const FrameTime frame_due = due + std::chrono::milliseconds(16);
    const FramePollResult frame_turn = window.poll_frame_schedule(frame_due);
    require(frame_turn.active_surface_ticks == 1U &&
                trace == std::vector<std::string>{"frame"},
            "active-surface state must advance before its paint is presented");
    window.paint(painter);
    require(trace == std::vector<std::string>({"frame", "paint"}),
            "the retained frame callback must precede its visible paint");
    animation.disconnect();
}

void test_bounds_and_shutdown_revocation() {
    auto root = make_control<Control>(StableId("dispatcher.bound.root"));
    Window window(root, {320.0, 180.0});
    std::vector<DispatchOperation> operations;
    operations.reserve(maximum_posted_callbacks);
    for (std::size_t index = 0U; index < maximum_posted_callbacks; ++index) {
        operations.push_back(window.begin_invoke([] {}));
    }
    bool bound_rejected = false;
    try {
        static_cast<void>(window.begin_invoke([] {}));
    } catch (const std::length_error&) {
        bound_rejected = true;
    }
    require(bound_rejected &&
                window.dispatcher_snapshot().pending == maximum_posted_callbacks,
            "dispatcher must reject work beyond its declared queue bound");

    window.shutdown_dispatcher();
    const DispatcherSnapshot stopped = window.dispatcher_snapshot();
    require(!stopped.accepting && stopped.pending == 0U &&
                stopped.cancelled == maximum_posted_callbacks &&
                operations.front().state() == DispatchOperationState::cancelled &&
                operations.back().state() == DispatchOperationState::cancelled,
            "shutdown must synchronously revoke every pending operation");
    bool shutdown_rejected = false;
    try {
        static_cast<void>(window.begin_invoke([] {}));
    } catch (const std::logic_error&) {
        shutdown_rejected = true;
    }
    require(shutdown_rejected,
            "a shut-down dispatcher must reject new work deterministically");
}

} // namespace

int main() {
    try {
        test_fifo_snapshot_and_nested_deferral();
        test_cancellation_owner_lifetime_and_fault_isolation();
        test_cross_thread_post_and_headless_pump();
        test_synchronous_invoke_inline_marshal_fault_and_owner_cancel();
        test_synchronous_invoke_host_and_shutdown_guards();
        test_concurrent_producer_fifo_and_turn_bound();
        test_dispatch_order_against_input_timer_layout_and_paint();
        test_bounds_and_shutdown_revocation();
        std::cout << "dispatcher-tests=pass\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "dispatcher-tests=fail reason=" << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
