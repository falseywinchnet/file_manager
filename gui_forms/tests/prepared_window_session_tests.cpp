#include "../src/render/text/prepared_window_session.hpp"
#include "prepared_text_test_support.hpp"

#include <cstdlib>
#include <iostream>
#include <new>
#include <system_error>

namespace gf = gui_forms;
namespace detail = gui_forms::detail;
namespace text = gui_forms::render::text;
using prepared_test::require;

namespace allocation_probe {
thread_local bool fail_next = false;
void* allocate(const std::size_t bytes) {
    if (fail_next) { fail_next = false; throw std::bad_alloc(); }
    const std::size_t requested = bytes == 0U ? 1U : bytes;
    void* const memory = std::malloc(requested);
    if (memory == nullptr) throw std::bad_alloc();
    return memory;
}
}
void* operator new(const std::size_t bytes) { void* const memory = allocation_probe::allocate(bytes); return memory; }
void* operator new[](const std::size_t bytes) { void* const memory = allocation_probe::allocate(bytes); return memory; }
void operator delete(void* const memory) noexcept { std::free(memory); }
void operator delete[](void* const memory) noexcept { std::free(memory); }
void operator delete(void* const memory, std::size_t) noexcept { std::free(memory); }
void operator delete[](void* const memory, std::size_t) noexcept { std::free(memory); }

namespace {
class Gate final {
public:
    void enter() {
        std::unique_lock<std::mutex> lock(mutex_);
        entered_ = true;
        condition_.notify_all();
        while (!released_) condition_.wait(lock);
    }
    void wait_entered() {
        std::unique_lock<std::mutex> lock(mutex_);
        const std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
        while (!entered_) {
            const std::cv_status result = condition_.wait_until(lock, deadline);
            require(result != std::cv_status::timeout || entered_, "gate deadline");
        }
    }
    void release() {
        std::lock_guard<std::mutex> lock(mutex_);
        released_ = true;
        condition_.notify_all();
    }
private:
    std::mutex mutex_{};
    std::condition_variable condition_{};
    bool entered_{};
    bool released_{};
};
// Declare after session: assertion unwinding opens the gate before session join.
struct ReleaseGate final {
    Gate& gate;
    ~ReleaseGate() { gate.release(); }
};

class Wake final : public text::PreparedWindowWakeConnection {
public:
    explicit Wake(const text::PreparedWindowWakeResult outcome = text::PreparedWindowWakeResult::signalled)
        : result_(outcome) {}
    text::PreparedWindowWakeResult signal() noexcept override {
        std::lock_guard<std::mutex> lock(mutex_);
        ++count_;
        condition_.notify_all();
        return result_;
    }
    void wait_count(const std::size_t expected) {
        std::unique_lock<std::mutex> lock(mutex_);
        const std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
        while (count_ < expected) {
            const std::cv_status result = condition_.wait_until(lock, deadline);
            require(result != std::cv_status::timeout || count_ >= expected, "wake deadline");
        }
    }
private:
    std::mutex mutex_{};
    std::condition_variable condition_{};
    std::size_t count_{};
    const text::PreparedWindowWakeResult result_;
};

// Deliberately blocks ONLY to prove in-flight target lifetime. A real host
// connection must satisfy the nonblocking signal contract.
class BlockingWake final : public text::PreparedWindowWakeConnection {
public:
    explicit BlockingWake(std::shared_ptr<Gate> gate) : gate_(std::move(gate)) {}
    text::PreparedWindowWakeResult signal() noexcept override {
        (*gate_).enter();
        return text::PreparedWindowWakeResult::signalled;
    }
private:
    std::shared_ptr<Gate> gate_{};
};

enum class HookMode { shape_gate, initialization_gate, start_failure, initialization_failure, shape_failure };
class Hook final : public text::PreparedWindowSessionTestHook {
public:
    explicit Hook(const HookMode mode) : mode_(mode) {}
    Gate gate{};
    void before_thread_start() override {
        if (mode_ == HookMode::start_failure)
            throw std::system_error(std::make_error_code(std::errc::resource_unavailable_try_again));
    }
    void before_initialize() override {
        if (mode_ == HookMode::initialization_gate) gate.enter();
        if (mode_ == HookMode::initialization_failure) throw std::bad_alloc();
    }
    void before_shape() override {
        if (mode_ == HookMode::shape_gate) gate.enter();
        if (mode_ == HookMode::shape_failure) throw std::runtime_error("injected worker failure");
    }
private:
    const HookMode mode_;
};

struct Fixture final {
    gf::PreparedTextService service{};
    gf::EncodedFontLease lease{};
    std::shared_ptr<const detail::PreparedFontBank> fonts{};
    std::shared_ptr<detail::PreparedLedger> ledger{};
    std::shared_ptr<detail::PreparedWindowBatchAuthority> authority{std::make_shared<detail::PreparedWindowBatchAuthority>()};
    detail::PreparedWindowKey key{};
    explicit Fixture(const std::span<const std::byte> encoded) {
        const std::array<gf::PreparedFontSource, 1U> sources{{{.encoded = encoded, .role = gf::FontRole::content}}};
        require(service.create_font_bank(sources, lease) == gf::PreparedTextStatus::success, "font bank");
        fonts = detail::PreparedTextAccess::fonts(lease);
        ledger = (*fonts).ledger;
        key.text = prepared_test::make_key(service, lease, "abc");
        key.controller_instance = 1U;
        key.projection_generation = 1U;
        key.authority = {1U, 1U};
    }
    gf::PreparedTextStatus admit(std::unique_ptr<detail::PreparedWindowBatchStorage>& batch) {
        const gf::DocumentMapSpan map{key.text.source, key.text.display_begin, key.text.display_end,
            gf::DocumentMapKind::identity_utf8};
        const std::array<gf::PreparedSourceEndpoint, 2U> endpoints{{
            {key.text.source.begin, key.text.display_begin}, {key.text.source.end, key.text.display_end}}};
        const detail::PreparedWindowParagraph row{key.text.source, key.text.display_begin, key.text.display_end,
            {true, true}, detail::PreparedWindowSeparator::none, {key.text.source.end, key.text.source.end},
            key.text.display_end, key.text.display_end};
        const detail::PreparedWindowView view{key, "abc", {&map, 1U}, endpoints, {&row, 1U}, true};
        detail::PreparedWindowInput input{};
        require(detail::own_prepared_window(view, ledger, input) == gf::PreparedTextStatus::success, "certified input");
        const gf::PreparedTextStatus result = detail::admit_prepared_window(key, authority, ledger, fonts, input, batch);
        return result;
    }
    std::size_t generations() const {
        std::lock_guard<std::mutex> lock((*ledger).mutex);
        return (*ledger).usage.payload_generations;
    }
};

text::PreparedWindowSessionSnapshot inspect(text::PreparedWindowSession& session) {
    text::PreparedWindowSessionSnapshot result{};
    require(session.inspect(result) == gf::PreparedTextStatus::success, "owner inspection");
    return result;
}

void use_session_identity(Fixture& fixture, text::PreparedWindowSession& session) {
    const text::PreparedWindowSessionSnapshot state = inspect(session);
    require(state.session_instance != 0U && state.controller_instance != 0U, "assigned session/controller identities");
    fixture.key.authority = {state.session_instance, 1U};
    fixture.key.controller_instance = state.controller_instance;
}

void success_and_retention(const std::span<const std::byte> font) {
    Fixture fixture(font);
    const std::shared_ptr<Wake> wake = std::make_shared<Wake>();
    text::PreparedWindowSession session(fixture.service.instance(), fixture.fonts, fixture.authority, wake);
    require(session.start() == gf::PreparedTextStatus::success, "session thread started");
    use_session_identity(fixture, session);
    (*wake).wait_count(1U);
    require(inspect(session).initialization == gf::PreparedTextStatus::success, "worker initialized");
    require(session.start() == gf::PreparedTextStatus::busy, "repeated start preserves worker");
    detail::PreparedWindowKey foreign_provider = fixture.key;
    ++foreign_provider.text.provider_instance;
    require(session.desire(foreign_provider) == gf::PreparedTextStatus::stale && !inspect(session).desired,
        "foreign provider cannot mutate authority");
    {
        const std::shared_ptr<detail::PreparedWindowBatchAuthority> fresh = std::make_shared<detail::PreparedWindowBatchAuthority>();
        text::PreparedWindowSession competing(fixture.service.instance(), fixture.fonts, fresh, wake);
        require(competing.start() == gf::PreparedTextStatus::busy, "ledger excludes second private session");
    }
    {
        Fixture foreign(font);
        require(detail::desire_prepared_window(*foreign.authority, foreign.key) == gf::PreparedTextStatus::success,
            "foreign certified authority");
        std::unique_ptr<detail::PreparedWindowBatchStorage> foreign_batch{};
        require(foreign.admit(foreign_batch) == gf::PreparedTextStatus::success, "foreign certified batch");
        const detail::PreparedWindowBatchStorage* const original = foreign_batch.get();
        require(session.submit(foreign_batch) == gf::PreparedTextStatus::invalid_input && foreign_batch.get() == original,
            "foreign bank ledger and authority refused without ownership loss");
    }
    std::array<std::shared_ptr<const detail::PreparedWindowBatchStorage>, 3U> retained{};
    for (std::size_t index = 0U; index < retained.size(); ++index) {
        require(session.desire(fixture.key) == gf::PreparedTextStatus::success, "new desired metadata");
        std::unique_ptr<detail::PreparedWindowBatchStorage> batch{};
        require(fixture.admit(batch) == gf::PreparedTextStatus::success, "already admitted batch");
        const detail::PreparedWindowBatchStorage* const original = batch.get();
        require(session.submit(batch) == gf::PreparedTextStatus::success && !batch, "submit transfers");
        require(fixture.generations() == index + 1U, "submit adds no reservation");
        (*wake).wait_count(index + 2U);
        detail::PreparedWindowKey stale = fixture.key;
        ++stale.projection_generation;
        require(session.adopt(stale, retained[index]) == gf::PreparedTextStatus::stale && !retained[index],
            "wrong identity preserves ready ownership");
        allocation_probe::fail_next = true;
        const gf::PreparedTextStatus failed = session.adopt(fixture.key, retained[index]);
        allocation_probe::fail_next = false;
        require(failed == gf::PreparedTextStatus::resource_failure && !retained[index] &&
            inspect(session).slot == gf::PreparedTextSlot::ready, "control-block failure preserves ready unique owner");
        require(session.adopt(fixture.key, retained[index]) == gf::PreparedTextStatus::success &&
            retained[index].get() == original && (*retained[index]).geometry, "complete immutable owner adoption");
        require(session.adopt(fixture.key, retained[index]) == gf::PreparedTextStatus::busy,
            "occupied output preserved");
        ++fixture.key.authority.epoch;
    }
    require(session.desire(fixture.key) == gf::PreparedTextStatus::success, "fourth metadata desire");
    std::unique_ptr<detail::PreparedWindowBatchStorage> fourth{};
    require(fixture.admit(fourth) == gf::PreparedTextStatus::busy && !fourth && fixture.generations() == 3U,
        "retained leases refuse fourth reservation");
    require(session.join_and_release() == gf::PreparedTextStatus::success, "confirmed join");
    const text::PreparedWindowSessionSnapshot joined = inspect(session);
    require(joined.exited && joined.joined && fixture.generations() == 3U, "join does not retire external leases");
    retained[0].reset();
    retained[1].reset();
    retained[2].reset();
    require(fixture.generations() == 0U, "final retained owner retirement");
}

void stale_and_busy(const std::span<const std::byte> font) {
    Fixture fixture(font);
    const std::shared_ptr<Wake> wake = std::make_shared<Wake>();
    const std::shared_ptr<Hook> hook = std::make_shared<Hook>(HookMode::shape_gate);
    text::PreparedWindowSession session(fixture.service.instance(), fixture.fonts, fixture.authority, wake, hook);
    ReleaseGate release{(*hook).gate};
    require(session.start() == gf::PreparedTextStatus::success, "start gated worker");
    use_session_identity(fixture, session);
    (*wake).wait_count(1U);
    require(session.desire(fixture.key) == gf::PreparedTextStatus::success, "first desire");
    std::unique_ptr<detail::PreparedWindowBatchStorage> first{};
    require(fixture.admit(first) == gf::PreparedTextStatus::success, "first admission");
    require(session.submit(first) == gf::PreparedTextStatus::success, "first submit");
    (*hook).gate.wait_entered();
    ++fixture.key.authority.epoch;
    require(session.desire(fixture.key) == gf::PreparedTextStatus::success, "new desire revokes running work");
    ++fixture.key.authority.epoch;
    require(session.desire(fixture.key) == gf::PreparedTextStatus::success, "latest metadata replaces intermediate desire");
    std::unique_ptr<detail::PreparedWindowBatchStorage> latest{};
    require(fixture.admit(latest) == gf::PreparedTextStatus::success, "latest admitted input");
    const detail::PreparedWindowBatchStorage* const original = latest.get();
    require(session.submit(latest) == gf::PreparedTextStatus::busy && latest.get() == original,
        "running occupied slot preserves refused submit");
    require(inspect(session).slot == gf::PreparedTextSlot::running && fixture.generations() == 2U,
        "revocation does not retire running reservation");
    (*hook).gate.release();
    (*wake).wait_count(2U);
    std::shared_ptr<const detail::PreparedWindowBatchStorage> output{};
    require(session.adopt(fixture.key, output) == gf::PreparedTextStatus::stale && !output,
        "late stale completion cannot publish");
    require(session.discard() == gf::PreparedTextStatus::success && fixture.generations() == 1U,
        "explicit discard retires stale slot");
    require(session.submit(latest) == gf::PreparedTextStatus::success, "latest work after retirement");
    (*wake).wait_count(3U);
    require(session.adopt(fixture.key, output) == gf::PreparedTextStatus::success, "latest request adopted");
}

struct WrongExecutor final {
    text::PreparedWindowSession& session;
    detail::PreparedWindowKey key{};
    bool passed{};
    void run() {
        text::PreparedWindowSessionSnapshot snapshot{};
        snapshot.joined = true;
        std::unique_ptr<detail::PreparedWindowBatchStorage> input{};
        std::shared_ptr<const detail::PreparedWindowBatchStorage> output{};
        std::array<gf::PreparedTextStatus, 9U> results{};
        results[0] = session.start();
        results[1] = session.desire(key);
        results[2] = session.submit(input);
        results[3] = session.inspect(snapshot);
        results[4] = session.adopt(key, output);
        results[5] = session.discard();
        results[6] = session.cancel();
        results[7] = session.begin_close();
        results[8] = session.join_and_release();
        passed = snapshot.joined;
        for (const gf::PreparedTextStatus status : results) {
            if (status != gf::PreparedTextStatus::wrong_executor) passed = false;
        }
    }
};

void close_race(const std::span<const std::byte> font) {
    Fixture fixture(font);
    const std::shared_ptr<Wake> wake = std::make_shared<Wake>();
    const std::shared_ptr<Hook> hook = std::make_shared<Hook>(HookMode::shape_gate);
    text::PreparedWindowSession session(fixture.service.instance(), fixture.fonts, fixture.authority, wake, hook);
    ReleaseGate release{(*hook).gate};
    require(session.start() == gf::PreparedTextStatus::success, "close-race start");
    use_session_identity(fixture, session);
    (*wake).wait_count(1U);
    WrongExecutor wrong{session, fixture.key};
    std::thread thread(&WrongExecutor::run, &wrong);
    thread.join();
    require(wrong.passed, "wrong executor refuses all owner operations without mutation");
    require(session.desire(fixture.key) == gf::PreparedTextStatus::success, "close-race desire");
    std::unique_ptr<detail::PreparedWindowBatchStorage> batch{};
    require(fixture.admit(batch) == gf::PreparedTextStatus::success, "close-race admission");
    require(session.submit(batch) == gf::PreparedTextStatus::success, "close-race submit");
    (*hook).gate.wait_entered();
    require(session.cancel() == gf::PreparedTextStatus::success && !inspect(session).desired,
        "cancel revokes metadata without releasing running slot");
    require(session.begin_close() == gf::PreparedTextStatus::success, "close returns while worker gated");
    require(inspect(session).slot == gf::PreparedTextSlot::running && fixture.generations() == 1U,
        "close retains running ownership");
    (*hook).gate.release();
    require(session.join_and_release() == gf::PreparedTextStatus::success && fixture.generations() == 0U,
        "confirmed worker exit retires unpublished batch");
    require(inspect(session).exited && inspect(session).joined, "exit and join recorded separately");
}

void startup_failures(const std::span<const std::byte> font) {
    Fixture fixture(font);
    const std::shared_ptr<Wake> wake = std::make_shared<Wake>();
    const std::shared_ptr<Hook> hook = std::make_shared<Hook>(HookMode::start_failure);
    text::PreparedWindowSession failed(fixture.service.instance(), fixture.fonts, fixture.authority, wake, hook);
    require(failed.start() == gf::PreparedTextStatus::resource_failure && !inspect(failed).started,
        "simulated pre-thread-start failure leaves no worker");
    const text::PreparedWindowSessionSnapshot burned = inspect(failed);
    require(burned.session_instance != 0U && burned.controller_instance != 0U,
        "failed start reports consumed identities");
    detail::PreparedSessionClaim available{};
    require(available.acquire(fixture.ledger) == gf::PreparedTextStatus::success, "failed start releases workspace claim");
    available.release();
    const std::shared_ptr<detail::PreparedWindowBatchAuthority> authority = std::make_shared<detail::PreparedWindowBatchAuthority>();
    const std::shared_ptr<Hook> initialization = std::make_shared<Hook>(HookMode::initialization_failure);
    text::PreparedWindowSession session(fixture.service.instance(), fixture.fonts, authority, wake, initialization);
    require(session.start() == gf::PreparedTextStatus::success, "worker launched before initialization failure");
    (*wake).wait_count(1U);
    const text::PreparedWindowSessionSnapshot state = inspect(session);
    require(state.session_instance != burned.session_instance && state.controller_instance != burned.controller_instance,
        "new start cannot recycle failed-start identities");
    require(state.initialization == gf::PreparedTextStatus::resource_failure && state.exited,
        "initialization failure reports typed exit with no slot");
    require(available.acquire(fixture.ledger) == gf::PreparedTextStatus::busy,
        "worker exit without join retains workspace claim");
    require(session.join_and_release() == gf::PreparedTextStatus::success, "initialization failure join");
    require(available.acquire(fixture.ledger) == gf::PreparedTextStatus::success, "join releases claim");
}

void worker_failure(const std::span<const std::byte> font) {
    Fixture fixture(font);
    const std::shared_ptr<Wake> wake = std::make_shared<Wake>();
    const std::shared_ptr<Hook> hook = std::make_shared<Hook>(HookMode::shape_failure);
    text::PreparedWindowSession session(fixture.service.instance(), fixture.fonts, fixture.authority, wake, hook);
    require(session.start() == gf::PreparedTextStatus::success, "failure-worker start");
    use_session_identity(fixture, session);
    (*wake).wait_count(1U);
    require(session.desire(fixture.key) == gf::PreparedTextStatus::success, "failure desire");
    std::unique_ptr<detail::PreparedWindowBatchStorage> batch{};
    require(fixture.admit(batch) == gf::PreparedTextStatus::success, "failure admission");
    require(session.submit(batch) == gf::PreparedTextStatus::success, "failure submit");
    (*wake).wait_count(2U);
    std::shared_ptr<const detail::PreparedWindowBatchStorage> output{};
    require(session.adopt(fixture.key, output) == gf::PreparedTextStatus::native_failure && !output &&
        fixture.generations() == 1U, "worker exception completes but retains occupied slot");
    require(session.discard() == gf::PreparedTextStatus::success && fixture.generations() == 0U,
        "failure drain releases reservation");
}

void close_during_notification(const std::span<const std::byte> font) {
    Fixture fixture(font);
    const std::shared_ptr<Gate> gate = std::make_shared<Gate>();
    std::shared_ptr<BlockingWake> connection = std::make_shared<BlockingWake>(gate);
    const std::weak_ptr<BlockingWake> lifetime = connection;
    text::PreparedWindowSession session(fixture.service.instance(), fixture.fonts, fixture.authority, connection);
    ReleaseGate release{*gate};
    require(session.start() == gf::PreparedTextStatus::success, "blocked-notification start");
    (*gate).wait_entered();
    connection.reset();
    require(session.begin_close() == gf::PreparedTextStatus::success && !lifetime.expired(),
        "close returns during callback and retains its target");
    const text::PreparedWindowSessionSnapshot closing = inspect(session);
    require(closing.closing && !closing.exited, "callback remains in flight after begin_close");
    (*gate).release();
    require(session.join_and_release() == gf::PreparedTextStatus::success && lifetime.expired(),
        "target retires only after notification and worker join");
}

void close_during_initialization(const std::span<const std::byte> font) {
    Fixture fixture(font);
    const std::shared_ptr<Wake> wake = std::make_shared<Wake>();
    const std::shared_ptr<Hook> hook = std::make_shared<Hook>(HookMode::initialization_gate);
    text::PreparedWindowSession session(fixture.service.instance(), fixture.fonts, fixture.authority, wake, hook);
    ReleaseGate release{(*hook).gate};
    require(session.start() == gf::PreparedTextStatus::success, "initialization gate start");
    (*hook).gate.wait_entered();
    use_session_identity(fixture, session);
    require(detail::desire_prepared_window(*fixture.authority, fixture.key) == gf::PreparedTextStatus::success,
        "pending-initialization admission uses allocated identity");
    std::unique_ptr<detail::PreparedWindowBatchStorage> batch{};
    require(fixture.admit(batch) == gf::PreparedTextStatus::success, "caller admission while initialization pending");
    const detail::PreparedWindowBatchStorage* const original = batch.get();
    require(session.submit(batch) == gf::PreparedTextStatus::pending && batch.get() == original,
        "initialization pending does not strand or consume caller input");
    require(session.begin_close() == gf::PreparedTextStatus::success, "close during initialization");
    (*hook).gate.release();
    require(session.join_and_release() == gf::PreparedTextStatus::success && batch.get() == original &&
        fixture.generations() == 1U, "failed admission ownership survives close");
}

void wake_failure(const std::span<const std::byte> font, const text::PreparedWindowWakeResult outcome) {
    Fixture fixture(font);
    const std::shared_ptr<Wake> wake = std::make_shared<Wake>(outcome);
    text::PreparedWindowSession session(fixture.service.instance(), fixture.fonts, fixture.authority, wake);
    require(session.start() == gf::PreparedTextStatus::success, "wake-failure start");
    (*wake).wait_count(1U);
    require(session.join_and_release() == gf::PreparedTextStatus::success, "wake failure retained until join");
    const text::PreparedWindowSessionSnapshot state = inspect(session);
    require(state.notification == outcome && state.closing && state.exited,
        "explicit sticky failed or closed notification outcome");
}

void ledger_closing(const std::span<const std::byte> font) {
    Fixture fixture(font);
    const std::shared_ptr<Wake> wake = std::make_shared<Wake>();
    text::PreparedWindowSession session(fixture.service.instance(), fixture.fonts, fixture.authority, wake);
    require(session.start() == gf::PreparedTextStatus::success, "ledger closing start");
    use_session_identity(fixture, session);
    (*wake).wait_count(1U);
    require(session.desire(fixture.key) == gf::PreparedTextStatus::success, "ledger closing desire");
    std::unique_ptr<detail::PreparedWindowBatchStorage> batch{};
    require(fixture.admit(batch) == gf::PreparedTextStatus::success, "ledger closing admission");
    const detail::PreparedWindowBatchStorage* const original = batch.get();
    fixture.service.begin_close();
    ++fixture.key.authority.epoch;
    require(session.desire(fixture.key) == gf::PreparedTextStatus::closing, "closed service cannot advance desire");
    require(session.submit(batch) == gf::PreparedTextStatus::closing && batch.get() == original,
        "closed ledger refuses transfer unchanged");
    require(session.join_and_release() == gf::PreparedTextStatus::success,
        "application explicitly closes private session outside old service registry");
}

void replacement_identities(const std::span<const std::byte> font) {
    Fixture fixture(font);
    std::shared_ptr<const detail::PreparedWindowBatchStorage> old_layout{};
    detail::PreparedWindowKey old_key{};
    {
        const std::shared_ptr<Wake> wake = std::make_shared<Wake>();
        text::PreparedWindowSession first(fixture.service.instance(), fixture.fonts, fixture.authority, wake);
        require(first.start() == gf::PreparedTextStatus::success, "first identity lifetime starts");
        use_session_identity(fixture, first);
        (*wake).wait_count(1U);
        require(first.desire(fixture.key) == gf::PreparedTextStatus::success, "first identity desire");
        std::unique_ptr<detail::PreparedWindowBatchStorage> batch{};
        require(fixture.admit(batch) == gf::PreparedTextStatus::success, "first identity admission");
        require(first.submit(batch) == gf::PreparedTextStatus::success, "first identity submit");
        (*wake).wait_count(2U);
        require(first.adopt(fixture.key, old_layout) == gf::PreparedTextStatus::success, "first retained layout");
        old_key = fixture.key;
        require(first.join_and_release() == gf::PreparedTextStatus::success, "first lifetime joins");
    }
    require(fixture.generations() == 1U, "old layout outlives first session");
    fixture.authority = std::make_shared<detail::PreparedWindowBatchAuthority>();
    const std::shared_ptr<Wake> wake = std::make_shared<Wake>();
    text::PreparedWindowSession second(fixture.service.instance(), fixture.fonts, fixture.authority, wake);
    require(second.start() == gf::PreparedTextStatus::success, "replacement on same service and font bank");
    use_session_identity(fixture, second);
    (*wake).wait_count(1U);
    require(fixture.key.authority.session != old_key.authority.session &&
        fixture.key.controller_instance != old_key.controller_instance, "replacement identities are independently nonreused");
    require(second.desire(old_key) == gf::PreparedTextStatus::stale && !inspect(second).desired,
        "old complete key cannot become replacement desire");
    require(second.desire(fixture.key) == gf::PreparedTextStatus::success, "replacement desire accepted");
    detail::PreparedWindowKey foreign = fixture.key;
    ++foreign.authority.epoch;
    foreign.authority.session = old_key.authority.session;
    require(second.desire(foreign) == gf::PreparedTextStatus::stale, "foreign session identity rejected");
    foreign = fixture.key;
    ++foreign.authority.epoch;
    foreign.controller_instance = old_key.controller_instance;
    require(second.desire(foreign) == gf::PreparedTextStatus::stale, "foreign controller identity rejected");
    const text::PreparedWindowSessionSnapshot unchanged = inspect(second);
    require(unchanged.desired && detail::same_prepared_window_key(*unchanged.desired, fixture.key),
        "identity refusals preserve accepted metadata and epoch");
    require(detail::prepared_window_worker_current(*old_layout) == gf::PreparedTextStatus::closing,
        "retained old geometry remains revoked");
    std::unique_ptr<detail::PreparedWindowBatchStorage> batch{};
    require(fixture.admit(batch) == gf::PreparedTextStatus::success, "replacement admission");
    require(second.submit(batch) == gf::PreparedTextStatus::success, "replacement submit");
    (*wake).wait_count(2U);
    std::shared_ptr<const detail::PreparedWindowBatchStorage> replacement{};
    require(second.adopt(old_key, replacement) == gf::PreparedTextStatus::stale && !replacement,
        "old identity cannot adopt replacement result");
    require(second.adopt(fixture.key, replacement) == gf::PreparedTextStatus::success && fixture.generations() == 2U,
        "both retained lifetimes remain independently charged");
}

void reject_preadmitted_authority(const std::span<const std::byte> font) {
    Fixture fixture(font);
    require(detail::desire_prepared_window(*fixture.authority, fixture.key) == gf::PreparedTextStatus::success,
        "existing authority desire");
    std::unique_ptr<detail::PreparedWindowBatchStorage> batch{};
    require(fixture.admit(batch) == gf::PreparedTextStatus::success, "existing admitted input");
    const detail::PreparedWindowBatchStorage* const original = batch.get();
    const std::shared_ptr<Wake> wake = std::make_shared<Wake>();
    {
        text::PreparedWindowSession session(fixture.service.instance(), fixture.fonts, fixture.authority, wake);
        require(session.start() == gf::PreparedTextStatus::invalid_input, "nonfresh authority refused before rewrite");
        const text::PreparedWindowSessionSnapshot state = inspect(session);
        require(!state.started && state.session_instance == 0U && state.controller_instance == 0U,
            "freshness refusal does not assign replacement identities");
    }
    require(batch.get() == original && detail::prepared_window_worker_current(*batch) == gf::PreparedTextStatus::success,
        "rejected session destruction preserves existing input and authority");
}
} // namespace

int main(const int argc, char** argv) {
    try {
        require(argc == 2, "font directory required");
        const std::filesystem::path directory(argv[1]);
        const std::vector<std::byte> font = prepared_test::read_font(directory);
        success_and_retention(font);
        stale_and_busy(font);
        close_race(font);
        startup_failures(font);
        worker_failure(font);
        close_during_notification(font);
        close_during_initialization(font);
        wake_failure(font, text::PreparedWindowWakeResult::failed);
        wake_failure(font, text::PreparedWindowWakeResult::closed);
        ledger_closing(font);
        replacement_identities(font);
        reject_preadmitted_authority(font);
        std::cout << "Prepared window session fixtures passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
