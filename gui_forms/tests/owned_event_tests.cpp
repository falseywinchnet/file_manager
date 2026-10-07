#include "gui_forms/event.hpp"
#include "gui_forms/basic_controls.hpp"
#include "gui_forms/timer.hpp"
#include "gui_forms/window.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>
#ifdef _WIN32
#include <malloc.h>
#endif

namespace owned_event_allocation_probe {
thread_local bool enabled{};
thread_local bool fired{};
thread_local std::size_t fail_at{};
thread_local std::size_t attempts{};

void visit() {
    if (!enabled) return;
    const std::size_t current = attempts;
    ++attempts;
    if (current == fail_at) {
        fired = true;
        throw std::bad_alloc();
    }
}

void* allocate(const std::size_t bytes) {
    visit();
    const std::size_t extent = bytes == 0U ? 1U : bytes;
    void* memory = std::malloc(extent);
    if (!memory) throw std::bad_alloc();
    return memory;
}

void* allocate_aligned(const std::size_t bytes, const std::size_t alignment) {
    visit();
    const std::size_t extent = bytes == 0U ? 1U : bytes;
    void* memory = nullptr;
#ifdef _WIN32
    memory = _aligned_malloc(extent, alignment);
#else
    const int status = posix_memalign(&memory, alignment, extent);
    if (status != 0) throw std::bad_alloc();
#endif
    if (!memory) throw std::bad_alloc();
    return memory;
}

void release_aligned(void* const memory) noexcept {
#ifdef _WIN32
    _aligned_free(memory);
#else
    std::free(memory);
#endif
}

class FaultScope final {
public:
    explicit FaultScope(const std::size_t index) {
        attempts = 0U;
        fired = false;
        fail_at = index;
        enabled = true;
    }
    ~FaultScope() { enabled = false; }
    FaultScope(const FaultScope&) = delete;
    FaultScope& operator=(const FaultScope&) = delete;
};
} // namespace owned_event_allocation_probe

// Replacement allocation is confined to this executable. Raw addresses are
// used only at the allocation boundary; injection never spans fixture setup,
// snapshot construction, assertions, logging, or fixture destruction.
void* operator new(const std::size_t bytes) {
    void* const memory = owned_event_allocation_probe::allocate(bytes);
    return memory;
}
void* operator new[](const std::size_t bytes) {
    void* const memory = owned_event_allocation_probe::allocate(bytes);
    return memory;
}
void operator delete(void* const memory) noexcept { std::free(memory); }
void operator delete[](void* const memory) noexcept { std::free(memory); }
void operator delete(void* const memory, std::size_t) noexcept { std::free(memory); }
void operator delete[](void* const memory, std::size_t) noexcept { std::free(memory); }
void* operator new(const std::size_t bytes, const std::align_val_t alignment) {
    void* memory = owned_event_allocation_probe::allocate_aligned(bytes, static_cast<std::size_t>(alignment));
    return memory;
}
void* operator new[](const std::size_t bytes, const std::align_val_t alignment) {
    void* memory = owned_event_allocation_probe::allocate_aligned(bytes, static_cast<std::size_t>(alignment));
    return memory;
}
void operator delete(void* const memory, const std::align_val_t) noexcept { owned_event_allocation_probe::release_aligned(memory); }
void operator delete[](void* const memory, const std::align_val_t) noexcept { owned_event_allocation_probe::release_aligned(memory); }
void operator delete(void* const memory, std::size_t, const std::align_val_t) noexcept { owned_event_allocation_probe::release_aligned(memory); }
void operator delete[](void* const memory, std::size_t, const std::align_val_t) noexcept { owned_event_allocation_probe::release_aligned(memory); }

namespace {
namespace gf = gui_forms;

void require(const bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

class Recorder : public gf::Component {
public:
    explicit Recorder(int& total) : total_(total) {}
    virtual void add(int value) { total_ += value; }
    void add_const(int value) const { total_ += value; }
    void add_noexcept(int value) noexcept { total_ += value; }
    void add_const_noexcept(int value) const noexcept { total_ += value; }
    void change(std::string& text, const int& value) { text += std::to_string(value); }
    void clicked(gf::ButtonBase&) { ++total_; }
    void text_changed(const std::string&) { ++total_; }
    void tick() { ++total_; }

private:
    int& total_;
};

class DerivedRecorder final : public Recorder {
public:
    explicit DerivedRecorder(int& total) : Recorder(total) {}
    void add(int value) override { Recorder::add(value * 2); }
};

class ShadowingOwner final : public gf::Component {
public:
    // These application members must not replace Component's lifetime protocol.
    [[nodiscard]] bool is_alive() const noexcept { return true; }
    void own_subscription(gf::SubscriptionToken) { ++intercepted; }
    void tick() { ++calls; }
    int calls{};
    int intercepted{};
};

class NestedOwner final : public gf::Component {
public:
    explicit NestedOwner(gf::Event<int>& event) : event_(event) {}
    void first(int depth) {
        order[count] = 10 + depth;
        ++count;
        if (depth == 0) {
            gf::on(event_, *this, &NestedOwner::third);
            event_.emit(1);
        }
    }
    void second(int depth) {
        order[count] = 20 + depth;
        ++count;
    }
    void third(int depth) {
        order[count] = 30 + depth;
        ++count;
    }
    std::array<int, 5U> order{};
    std::size_t count{};

private:
    gf::Event<int>& event_;
};

class DisposalOwner final : public gf::Component {
public:
    explicit DisposalOwner(gf::Event<>& event) : event_(event) {}
    void first() {
        ++calls;
        dispose();
        // Neither registration nor a nested emission may revive this owner.
        gf::on(event_, *this, &DisposalOwner::later);
        event_.emit();
        ++completed;
    }
    void later() { calls += 100; }
    int calls{};
    int completed{};
    bool revoked_before_hook{};

private:
    void on_dispose() noexcept override {
        const gf::EventStatistics statistics = event_.statistics();
        revoked_before_hook = statistics.subscriptions_connected ==
                              statistics.subscriptions_disconnected;
    }
    gf::Event<>& event_;
};

class PublisherDestruction final : public gf::Component {
public:
    std::unique_ptr<gf::Event<>> event{std::make_unique<gf::Event<>>()};
    int calls{};
    void destroy() {
        ++calls;
        event.reset();
        ++calls;
    }
    void later() { calls += 100; }
};

class SelfDestruction final : public gf::Component {
public:
    SelfDestruction(std::unique_ptr<SelfDestruction>& owner, int& calls)
        : owner_(owner), calls_(calls) {}
    void destroy() {
        ++calls_;
        owner_.reset();
        // No access to this after destruction. The event retains its slot.
    }
    void later() { calls_ += 100; }

private:
    std::unique_ptr<SelfDestruction>& owner_;
    int& calls_;
};

class ThrowingOwner final : public gf::Component {
public:
    void fail() {
        ++calls;
        if (calls == 1) {
            throw std::runtime_error("owned handler failure");
        }
    }
    int calls{};
};

struct RegistrationOnRelease final {
    gf::Component& owner;
    gf::Event<>& event;
    bool& observed_disposing;

    ~RegistrationOnRelease() {
        observed_disposing = !owner.is_alive();
        gf::on(event, owner, &gf::Component::dispose);
    }
};

struct RetainUntilDisconnected final {
    std::shared_ptr<RegistrationOnRelease> retained{};
    void operator()() const noexcept {}
};

void test_signatures_and_natural_destruction() {
    gf::Event<int> event{};
    gf::Event<std::string&, const int&> references{};
    int total = 0;
    std::string text{};
    {
        DerivedRecorder owner(total);
        gf::on(event, owner, &Recorder::add);
        gf::on(event, owner, &Recorder::add_const);
        gf::on(event, owner, &Recorder::add_noexcept);
        gf::on(event, owner, &Recorder::add_const_noexcept);
        gf::on(references, owner, &Recorder::change);
        event.emit(2);
        references.emit(text, total);
        require(total == 10 && text == "10",
                "deduction must preserve const, noexcept, virtual bases and references");
    }
    event.emit(50);
    require(total == 10, "natural owner destruction must revoke every handler");
    const gf::EventStatistics statistics = event.statistics();
    require(statistics.subscriptions_connected == 4U &&
                statistics.subscriptions_disconnected == 4U,
            "each owned subscription must disconnect exactly once");
}

void test_real_control_and_timer_signatures() {
    int total = 0;
    Recorder owner(total);
    const std::shared_ptr<gf::Button> button =
        gf::make_control<gf::Button>(gf::StableId("owned.button"), "Run");
    const std::shared_ptr<gf::Label> label =
        gf::make_control<gf::Label>(gf::StableId("owned.label"));
    const gf::Control::Ptr root =
        gf::make_control<gf::Control>(gf::StableId("owned.root"));
    gf::Window window(root, {100.0, 100.0});
    gf::Timer timer(window);
    gf::on((*button).clicked(), owner, &Recorder::clicked);
    gf::on((*label).text_changed(), owner, &Recorder::text_changed);
    gf::on(timer.tick(), owner, &Recorder::tick);
    (*button).perform_click();
    (*label).set_text("Changed");
    timer.tick().emit();
    require(total == 3 && !timer.enabled(),
            "control and timer handlers must wire without starting a timer");
    owner.dispose();
    (*button).perform_click();
    (*label).set_text("Again");
    timer.tick().emit();
    require(total == 3, "disposed owner must stop real control notifications");
}

void test_component_protocol_is_explicit() {
    gf::Event<> event{};
    ShadowingOwner owner{};
    gf::on(event, owner, &ShadowingOwner::tick);
    event.emit();
    owner.dispose();
    gf::on(event, owner, &ShadowingOwner::tick);
    event.emit();
    const gf::EventStatistics statistics = event.statistics();
    require(owner.calls == 1 && owner.intercepted == 0 &&
                statistics.subscriptions_connected == 1U,
            "derived method names must not select or replace lifecycle authority");
}

void test_disposal_during_emission() {
    gf::Event<> event{};
    DisposalOwner owner(event);
    gf::on(event, owner, &DisposalOwner::first);
    gf::on(event, owner, &DisposalOwner::later);
    event.emit();
    require(owner.calls == 1 && owner.completed == 1 && owner.revoked_before_hook,
            "disposal must cancel pending callbacks and retain the executing binding");
    const gf::EventStatistics statistics = event.statistics();
    require(statistics.subscriptions_connected == 2U &&
                statistics.subscriptions_disconnected == 2U,
            "registration on a dead owner must not create a slot");
}

void test_natural_teardown_cannot_resubscribe() {
    gf::Event<> event{};
    bool observed_disposing = false;
    {
        gf::Component owner{};
        const std::shared_ptr<RegistrationOnRelease> teardown =
            std::make_shared<RegistrationOnRelease>(owner, event, observed_disposing);
        owner.own_subscription(event.subscribe(RetainUntilDisconnected{teardown}));
    }
    const gf::EventStatistics statistics = event.statistics();
    require(observed_disposing && statistics.subscriptions_connected == 1U &&
                statistics.subscriptions_disconnected == 1U,
            "callback destruction must not register new work on a destructing Component");
    event.emit();
}

void test_nested_emission() {
    gf::Event<int> event{};
    NestedOwner owner(event);
    gf::on(event, owner, &NestedOwner::first);
    gf::on(event, owner, &NestedOwner::second);
    event.emit(0);
    const std::array<int, 5U> expected{10, 11, 21, 31, 20};
    require(owner.order == expected && owner.count == expected.size(),
            "nested events must keep independent registration boundaries");
}

void test_publisher_and_target_destruction() {
    PublisherDestruction owner{};
    gf::on(*owner.event, owner, &PublisherDestruction::destroy);
    gf::on(*owner.event, owner, &PublisherDestruction::later);
    (*owner.event).emit();
    require(owner.calls == 2, "publisher destruction must skip later slots");
    owner.dispose();

    gf::Event<> event{};
    int calls = 0;
    std::unique_ptr<SelfDestruction> target{};
    target = std::make_unique<SelfDestruction>(target, calls);
    gf::on(event, *target, &SelfDestruction::destroy);
    gf::on(event, *target, &SelfDestruction::later);
    event.emit();
    require(!target && calls == 1,
            "destroying a target during dispatch must cancel remaining handlers");
}

void test_exception_recovery_and_null_member() {
    gf::Event<> event{};
    ThrowingOwner owner{};
    gf::on(event, owner, &ThrowingOwner::fail);
    bool propagated = false;
    try {
        event.emit();
    } catch (const std::runtime_error&) {
        propagated = true;
    }
    event.emit();
    require(propagated && owner.calls == 2,
            "exception must propagate without corrupting the next emission");
    void (ThrowingOwner::*null_method)() = nullptr;
    bool rejected = false;
    try {
        gf::on(event, owner, null_method);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected, "null member pointers must be rejected in release builds");
}

void test_explicit_token_transfer_and_legacy_lifetime() {
    gf::Event<int> event{};
    int total = 0;
    Recorder owner(total);
    const gf::Delegate<int> handler =
        gf::Delegate<int>::bind<Recorder, &Recorder::add>(owner);
    {
        const gf::SubscriptionToken token = event.subscribe(owner, handler);
        event.emit(1);
    }
    event.emit(2);
    require(total == 1, "legacy subscribe still requires a surviving token");
    gf::SubscriptionToken token = event.subscribe(handler);
    owner.own_subscription(std::move(token));
    require(!token.connected(), "token transfer must leave the former owner empty");
    event.emit(4);
    owner.dispose();
    event.emit(8);
    require(total == 5, "explicit transfer must retain then revoke the connection");
    owner.own_subscription(event.subscribe(handler));
    event.emit(16);
    require(total == 5, "a dead owner must immediately disconnect a transferred token");
}

void test_registration_failure_rollback() {
    std::size_t failure_count = 0U;
    bool reached_success = false;
    // The bound stops a broken failure loop; no expected allocation count is
    // baked in. Every fresh registration allocation is failed in turn.
    for (std::size_t fail_at = 0U; fail_at < 32U; ++fail_at) {
        gf::Event<int> event{};
        int total = 0;
        Recorder owner(total);
        bool failed = false;
        try {
            owned_event_allocation_probe::FaultScope fault(fail_at);
            gf::on(event, owner, &Recorder::add);
        } catch (const std::bad_alloc&) {
            failed = true;
        }
        event.emit(1);
        const gf::EventStatistics statistics = event.statistics();
        if (!failed) {
            require(total == 1, "successful registration must publish one handler");
            reached_success = true;
            break;
        }
        ++failure_count;
        require(total == 0 && statistics.subscriptions_connected ==
                                  statistics.subscriptions_disconnected,
                "failed registration must leave no connected callback or stale statistic");
        gf::on(event, owner, &Recorder::add);
        event.emit(2);
        require(total == 2, "registration must remain usable after allocation failure");
    }
    require(reached_success && failure_count != 0U,
            "fault sweep must cover every registration allocation and terminate");
    std::cout << "registration allocation failures covered: " << failure_count << '\n';
}

void test_allocation_free_emission() {
    gf::Event<int> event{};
    int total = 0;
    Recorder owner(total);
    gf::on(event, owner, &Recorder::add);
    constexpr std::size_t iterations = 10000U;
    std::size_t allocations = 0U;
    {
        owned_event_allocation_probe::FaultScope probe(
            std::numeric_limits<std::size_t>::max());
        for (std::size_t index = 0U; index < iterations; ++index) {
            event.emit(1);
        }
        allocations = owned_event_allocation_probe::attempts;
    }
    require(total == static_cast<int>(iterations) && allocations == 0U,
            "steady-state owned member dispatch must not allocate");
    std::cout << "owned member emissions: " << iterations
              << ", allocations: " << allocations << '\n';
}

} // namespace

int main() {
    try {
        test_signatures_and_natural_destruction();
        test_real_control_and_timer_signatures();
        test_component_protocol_is_explicit();
        test_disposal_during_emission();
        test_natural_teardown_cannot_resubscribe();
        test_nested_emission();
        test_publisher_and_target_destruction();
        test_exception_recovery_and_null_member();
        test_explicit_token_transfer_and_legacy_lifetime();
        test_registration_failure_rollback();
        test_allocation_free_emission();
        std::cout << "owned event tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "owned event tests failed: " << error.what() << '\n';
        return 1;
    }
}
