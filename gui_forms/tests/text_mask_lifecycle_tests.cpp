#include "../src/core/text/text_mask/text_mask_state.hpp"
#include "prepared_text_test_support.hpp"

#include <iostream>
#include <limits>

namespace {
using namespace gui_forms;
using prepared_test::require;

class WorkerWake final : public PreparedTextWakeTarget {
public:
    const std::thread::id opening_executor{std::this_thread::get_id()};
    std::atomic<unsigned> count{0};
    std::atomic<bool> wrong_executor{false};
    void post_prepared_text_wake() noexcept override {
        if (std::this_thread::get_id() == opening_executor) wrong_executor.store(true, std::memory_order_release);
        count.fetch_add(1, std::memory_order_release);
    }
};

// Explicit barrier retains worker-side input until released. It provides
// deterministic cancellation/close tests without depending on native timing.
class FixtureBackend final : public detail::MaskBackend {
public:
    std::mutex mutex{};
    std::condition_variable condition{};
    bool blocked{false};
    bool entered{false};
    std::size_t calls{0};
    void block() {
        std::lock_guard<std::mutex> lock(mutex);
        blocked = true;
        entered = false;
    }
    void release() {
        std::lock_guard<std::mutex> lock(mutex);
        blocked = false;
        condition.notify_all();
    }
    void wait_entered() {
        std::unique_lock<std::mutex> lock(mutex);
        const std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (!entered) {
            const std::cv_status status = condition.wait_until(lock, deadline);
            require(status != std::cv_status::timeout, "worker enters bounded fixture");
        }
    }
    TextMaskResult execute(const std::shared_ptr<detail::MaskLedger>& ledger,
        const std::shared_ptr<const detail::MaskKey>& key,
        std::shared_ptr<const detail::TextMaskStorage>& output) override {
        {
            std::unique_lock<std::mutex> lock(mutex);
            ++calls;
            entered = true;
            condition.notify_all();
            while (blocked) condition.wait(lock);
        }
        const detail::MaskOptions& options = (*key).options;
        TextMaskMetrics metrics{};
        metrics.size_64 = options.size_64;
        metrics.wrap_width_64 = options.width_64;
        metrics.additional_gap_64 = options.gap_64;
        metrics.device_scale = options.scale;
        if (!(*key).text.empty() && options.width_64 != 64) {
            metrics.width_px = 1024;
            metrics.height_px = 1024;
            metrics.stride_bytes = 1024;
        }
        std::shared_ptr<detail::TextMaskStorage> candidate = detail::allocate_mask_storage(ledger, key, metrics, 1);
        (*candidate).lines[0].text_end = static_cast<std::uint32_t>((*key).text.size());
        (*candidate).lines[0].consumed_end = static_cast<std::uint32_t>((*key).text.size());
        if (!(*candidate).pixels.empty()) (*candidate).pixels[0] = 255;
        output = std::move(candidate);
        return {TextMaskStatus::success};
    }
};

// Always release barriers before destroying the session, even on failed checks.
struct Fixture final {
    FixtureBackend backend{};
    WorkerWake wake{};
    TextMaskService service{};
    EncodedFontLease bank{};
    std::unique_ptr<TextMaskSession> session{};
    explicit Fixture(const std::span<const std::byte> bytes) {
        (*detail::TextMaskAccess::state(service)).backend = &backend;
        const std::array<PreparedFontSource, 1> sources{{{.encoded = bytes}}};
        TextMaskResult result = service.create_font_bank(sources, bank);
        require(result.status == TextMaskStatus::success, "fixture bank");
        result = service.open_session(&wake, session);
        require(result.status == TextMaskStatus::success, "fixture session");
    }
    ~Fixture() {
        backend.release();
        if (session) (*session).join_and_release();
    }
};

TextMaskSessionSnapshot wait_completed(TextMaskSession& session, const std::size_t count) {
    const std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    for (;;) {
        const TextMaskSessionSnapshot snapshot = session.snapshot();
        if (snapshot.completed == count) return snapshot;
        require(std::chrono::steady_clock::now() < deadline, "completion deadline");
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
TextMaskResult prepare(Fixture& fixture, const TextMaskRequest& request, TextMaskLease& output) {
    TextMaskRequestId id{};
    TextMaskResult result = (*fixture.session).submit(fixture.bank, request, id);
    if (result.status != TextMaskStatus::success) return result;
    const TextMaskSessionSnapshot snapshot = wait_completed(*fixture.session, 1);
    require(snapshot.occupied == 1, "completed slot remains occupied");
    result = (*fixture.session).take(id, output);
    return result;
}

void test_queue(const std::span<const std::byte> bytes) {
    Fixture fixture(bytes);
    fixture.backend.block();
    std::array<TextMaskRequestId, 9> ids{};
    TextMaskResult result = (*fixture.session).submit(fixture.bank, {.utf8 = "running"}, ids[0]);
    require(result.status == TextMaskStatus::success, "running request admits");
    fixture.backend.wait_entered();
    for (std::size_t index = 1; index < ids.size(); ++index) {
        result = (*fixture.session).submit(fixture.bank, {.utf8 = "queued"}, ids[index]);
        require(result.status == TextMaskStatus::success, "eight queued admissions");
    }
    TextMaskSessionSnapshot snapshot = (*fixture.session).snapshot();
    require(snapshot.occupied == 9 && snapshot.assigned == 1 && snapshot.queued == 8, "one running plus eight queued");
    TextMaskRequestId unchanged{77, 88};
    result = (*fixture.session).submit(fixture.bank, {.utf8 = "overflow"}, unchanged);
    require(result.status == TextMaskStatus::busy && unchanged.session == 77 && unchanged.serial == 88, "busy preserves identity");
    TextMaskCancellation cancellation = (*fixture.session).cancel(ids[1]);
    require(cancellation.retirement == TextMaskRetirement::released, "queued cancel retires synchronously");
    cancellation = (*fixture.session).cancel(ids[0]);
    require(cancellation.retirement == TextMaskRetirement::pending_retirement, "native-running cancel retains slot");
    snapshot = (*fixture.session).snapshot();
    require(snapshot.occupied == 8 && snapshot.retiring == 1, "retiring job remains occupied");
    TextMaskBudgetSnapshot budget = fixture.service.budget_snapshot();
    require(budget.request_slots.live == 8 && budget.source_keys.live == 8, "cancelled running input still charged");
    fixture.backend.release();
    snapshot = wait_completed(*fixture.session, 7);
    require(snapshot.occupied == 7, "cancelled work automatically retires");
    TextMaskLease output{};
    result = (*fixture.session).take(ids[0], output);
    require(result.status == TextMaskStatus::stale, "cancelled identity cannot publish");
    result = (*fixture.session).lookup(fixture.bank, {.utf8 = "running"}, output);
    require(result.status == TextMaskStatus::cache_miss, "cancelled result never caches");
    for (std::size_t index = 2; index < ids.size(); ++index) {
        result = (*fixture.session).take(ids[index], output);
        require(result.status == TextMaskStatus::success, "completed result can be consumed");
    }
    snapshot = (*fixture.session).snapshot();
    require(snapshot.occupied == 0, "all slots consumed");
}

void test_retention_and_limits(const std::span<const std::byte> bytes) {
    Fixture fixture(bytes);
    std::array<TextMaskLease, 32> retained{};
    for (std::size_t index = 0; index < retained.size(); ++index) {
        const std::string text = "mask" + std::to_string(index);
        const TextMaskResult result = prepare(fixture, {.utf8 = text}, retained[index]);
        require(result.status == TextMaskStatus::success, "32 one-MiB masks admitted");
    }
    (*fixture.session).clear_cache();
    TextMaskBudgetSnapshot budget = fixture.service.budget_snapshot();
    require(budget.coverage.live == 32U*1024U*1024U && budget.cache_records.live == 0, "evicted leases still charged");
    TextMaskLease previous = retained[0];
    TextMaskResult result = prepare(fixture, {.utf8 = "late-refusal"}, previous);
    require(result.status == TextMaskStatus::limit_exceeded && result.limit == TextMaskLimit::live_mask_bytes, "late coverage refusal typed");
    require(previous.source_utf8() == "mask0", "late failure preserves previous mask");
    retained[1].reset();
    result = prepare(fixture, {.utf8 = "replacement"}, retained[1]);
    require(result.status == TextMaskStatus::success, "actual retirement permits replacement");
    (*fixture.session).join_and_release();
    result = fixture.service.open_session(&fixture.wake, fixture.session);
    require(result.status == TextMaskStatus::success, "reopen after join");
    result = prepare(fixture, {.utf8 = "reopen-refusal"}, previous);
    require(result.status == TextMaskStatus::limit_exceeded, "reopen cannot reset live coverage budget");
    require(previous.coverage()[0] == 255 && previous.source_utf8() == "mask0", "old immutable data survives close");
}

void test_empty_object_cap(const std::span<const std::byte> bytes) {
    Fixture fixture(bytes);
    std::array<TextMaskLease, 709> retained{};
    for (std::size_t index = 0; index < retained.size(); ++index) {
        const double size = 4.0 + static_cast<double>(index) / 64.0;
        const TextMaskResult result = prepare(fixture, {.size = size}, retained[index]);
        require(result.status == TextMaskStatus::success, "zero-ink masks admitted to object bound");
    }
    TextMaskBudgetSnapshot budget = fixture.service.budget_snapshot();
    require(budget.masks.live == 709 && budget.coverage.live == 0 && budget.cache_records.live == 700, "zero ink still bounded and cache evicts");
    require(retained[0].has_value() && retained[0].coverage().empty() && retained[0].lines().size() == 1, "valid empty mask differs from null handle");
    TextMaskLease output{};
    TextMaskResult result = prepare(fixture, {.size = 100.0}, output);
    require(result.status == TextMaskStatus::limit_exceeded && result.limit == TextMaskLimit::live_mask_objects, "zero-ink object exhaustion typed");
    retained[0].reset();
    result = prepare(fixture, {.size = 100.0}, output);
    require(result.status == TextMaskStatus::success, "last owner release frees object slot");
}

void test_validation(const std::span<const std::byte> bytes) {
    Fixture fixture(bytes);
    TextMaskLease lease{};
    TextMaskResult result = prepare(fixture, {.utf8 = "exact", .size = 16.0 + 1.0/128.0}, lease);
    require(result.status == TextMaskStatus::success && lease.metrics().size_64 == 1025 && lease.metrics().additional_gap_64 == 51, "size and gap round once");
    const TextMaskBudgetSnapshot before = fixture.service.budget_snapshot();
    TextMaskLease copy{};
    result = (*fixture.session).lookup(fixture.bank, {.utf8 = "exact", .size = 1025.0/64.0}, copy);
    require(result.status == TextMaskStatus::success && copy.coverage().data() == lease.coverage().data(), "normalized exact lookup shares storage");
    const TextMaskBudgetSnapshot after = fixture.service.budget_snapshot();
    require(before.metadata.live == after.metadata.live && before.coverage.live == after.coverage.live, "copy and lookup allocate no storage");
    TextMaskRequestId id{5, 6};
    result = (*fixture.session).submit(fixture.bank, {.utf8 = "tab\t"}, id);
    require(result.status == TextMaskStatus::unsupported_profile && id.session == 5, "tab unsupported and ID preserved");
    result = (*fixture.session).submit(fixture.bank, {.device_scale = 1.5, .raster = TextMaskRaster::true_mono}, id);
    require(result.status == TextMaskStatus::unsupported_profile, "fractional mono refused");
    result = (*fixture.session).submit(fixture.bank, {.wrap_width = 0.001}, id);
    require(result.status == TextMaskStatus::invalid_input, "positive width quantizing to zero refused");
    result = (*fixture.session).submit(fixture.bank, {.size = std::numeric_limits<double>::quiet_NaN()}, id);
    require(result.status == TextMaskStatus::invalid_input, "NaN refused");
    const std::string invalid("\xC0\x80", 2);
    result = (*fixture.session).submit(fixture.bank, {.utf8 = invalid}, id);
    require(result.status == TextMaskStatus::invalid_input, "overlong UTF8 refused");
    const std::string excess_lines(256, '\n');
    result = (*fixture.session).submit(fixture.bank, {.utf8 = excess_lines}, id);
    require(result.limit == TextMaskLimit::line_count, "empty trailing lines count toward bound");
    Fixture foreign(bytes);
    result = (*fixture.session).submit(foreign.bank, {}, id);
    require(result.status == TextMaskStatus::invalid_input, "foreign ledger bank refused");
}

void test_close(const std::span<const std::byte> bytes) {
    Fixture fixture(bytes);
    fixture.backend.block();
    TextMaskRequestId id{};
    TextMaskResult result = (*fixture.session).submit(fixture.bank, {.utf8 = "close"}, id);
    require(result.status == TextMaskStatus::success, "close fixture admitted");
    fixture.backend.wait_entered();
    (*fixture.session).begin_close();
    TextMaskSessionSnapshot snapshot = (*fixture.session).snapshot();
    require(snapshot.closing && !snapshot.joined && snapshot.retiring == 1, "begin_close returns with native job retained");
    std::unique_ptr<TextMaskSession> replacement{};
    result = fixture.service.open_session(nullptr, replacement);
    require(result.status == TextMaskStatus::busy, "replacement denied before join");
    fixture.backend.release();
    (*fixture.session).join_and_release();
    snapshot = (*fixture.session).snapshot();
    require(snapshot.joined && snapshot.occupied == 0, "join finishes retirement");
    const TextMaskBudgetSnapshot budget = fixture.service.budget_snapshot();
    require(budget.request_slots.live == 0 && budget.source_keys.live == 0 && budget.masks.live == 0 && budget.coverage.live == 0,
        "closed cancelled job releases all result resources");
    require(fixture.wake.count.load(std::memory_order_acquire) == 0, "close revokes completion delivery");
}

void test_source_capacity(const std::span<const std::byte> bytes) {
    Fixture fixture(bytes);
    std::array<TextMaskLease, 128> retained{};
    std::string text(16384, 'x');
    for (std::size_t index = 0; index < retained.size(); ++index) {
        const std::string prefix = std::to_string(index);
        text.replace(0, prefix.size(), prefix);
        const TextMaskResult result = prepare(fixture, {.utf8 = text, .wrap_width = 1.0}, retained[index]);
        require(result.status == TextMaskStatus::success, "exact source capacity admission");
    }
    (*fixture.session).clear_cache();
    (*fixture.session).join_and_release();
    TextMaskBudgetSnapshot budget = fixture.service.budget_snapshot();
    require(budget.utf8.live == 2U*1024U*1024U && budget.source_keys.live == 128, "source remains charged after eviction and close");
    TextMaskResult result = fixture.service.open_session(nullptr, fixture.session);
    require(result.status == TextMaskStatus::success, "source budget fixture reopens");
    TextMaskRequestId id{17, 18};
    result = (*fixture.session).submit(fixture.bank, {.utf8 = "another", .wrap_width = 1.0}, id);
    require(result.limit == TextMaskLimit::key_bytes && id.session == 17 && id.serial == 18, "UTF8 capacity refuses before publishing ID");
    budget = fixture.service.budget_snapshot();
    require(budget.request_slots.live == 0 && budget.source_keys.live == 128 && budget.metadata.reserved == 0, "failed key allocation rolls back slot and object");
    retained[0].reset();
    result = prepare(fixture, {.utf8 = "another", .wrap_width = 1.0}, retained[0]);
    require(result.status == TextMaskStatus::success, "last source owner frees UTF8 capacity");
}

void test_budget_rollback(const std::span<const std::byte> bytes) {
    Fixture fixture(bytes);
    const std::shared_ptr<detail::MaskLedger> ledger = (*detail::TextMaskAccess::state(fixture.service)).ledger;
    const TextMaskBudgetSnapshot before = fixture.service.budget_snapshot();
    detail::MaskCharge reservation{};
    reservation.reserve(ledger, detail::MaskResource::metadata, before.metadata.limit - before.metadata.live);
    TextMaskBudgetSnapshot budget = fixture.service.budget_snapshot();
    require(budget.metadata.live == before.metadata.live && budget.metadata.live + budget.metadata.reserved == budget.metadata.limit,
        "reservations distinguished from live allocations");
    TextMaskRequestId id{37, 38};
    TextMaskResult result = (*fixture.session).submit(fixture.bank, {}, id);
    require(result.limit == TextMaskLimit::metadata_bytes && id.serial == 38, "control block allocation fails with preserved ID");
    reservation.reset();
    budget = fixture.service.budget_snapshot();
    require(budget.metadata.live == before.metadata.live && budget.metadata.reserved == 0 && budget.request_slots.live == 0 && budget.source_keys.live == 0,
        "failed allocate_shared leaves no count or byte charge");
    TextMaskLease lease{};
    result = prepare(fixture, {}, lease);
    require(result.status == TextMaskStatus::success, "service usable after allocation rollback");
    std::array<TextMaskRequestId, 9> ids{};
    for (std::size_t index = 0; index < ids.size(); ++index) {
        result = (*fixture.session).submit(fixture.bank, {}, ids[index]);
        require(result.status == TextMaskStatus::success, "cache hit still admits a completed slot");
    }
    const TextMaskSessionSnapshot snapshot = (*fixture.session).snapshot();
    require(snapshot.completed == 9 && snapshot.occupied == 9, "cache hits hold all nine slots");
    result = (*fixture.session).submit(fixture.bank, {}, id);
    require(result.status == TextMaskStatus::busy, "unconsumed cached completions prevent admission");
    const TextMaskCancellation cancelled = (*fixture.session).cancel(ids[0]);
    require(cancelled.retirement == TextMaskRetirement::released, "completed cancellation releases synchronously");
    result = (*fixture.session).discard(ids[1]);
    require(result.status == TextMaskStatus::success, "completed discard releases slot");
}

struct WrongExecutor final {
    TextMaskSession& session;
    EncodedFontLease& bank;
    TextMaskResult result{};
    void operator()() {
        TextMaskRequestId id{};
        result = session.submit(bank, {}, id);
    }
};
void test_executor_and_default(const std::span<const std::byte> bytes) {
    Fixture fixture(bytes);
    WrongExecutor operation{*fixture.session, fixture.bank};
    std::thread worker(std::ref(operation));
    worker.join();
    require(operation.result.status == TextMaskStatus::wrong_executor, "control rejects wrong executor");
    TextMaskService service{};
    EncodedFontLease bank{};
    const std::array<PreparedFontSource, 1> sources{{{.encoded = bytes}}};
    TextMaskResult result = service.create_font_bank(sources, bank);
    require(result.status == TextMaskStatus::success, "default bank admitted");
    std::unique_ptr<TextMaskSession> session{};
    result = service.open_session(nullptr, session);
    require(result.status == TextMaskStatus::success, "default session opens");
    TextMaskRequestId id{};
    result = (*session).submit(bank, {}, id);
    require(result.status == TextMaskStatus::success, "default admission bounded");
    const TextMaskSessionSnapshot snapshot = wait_completed(*session, 1);
    require(snapshot.completed == 1, "default completion ready");
    TextMaskLease lease{};
    result = (*session).take(id, lease);
    require(result.status == TextMaskStatus::unsupported_profile && !lease.has_value(), "unfinished native backend never claims rendered success");
}

void test_all_resource_reservations() {
    const std::shared_ptr<detail::MaskLedger> ledger = detail::make_mask_ledger();
    for (std::size_t index = 0; index < static_cast<std::size_t>(detail::MaskResource::count); ++index) {
        const detail::MaskResource resource = static_cast<detail::MaskResource>(index);
        detail::MaskUsage baseline{};
        {
            std::lock_guard<std::mutex> lock((*ledger).mutex);
            baseline = (*ledger).usage[index];
        }
        detail::MaskCharge capacity{};
        capacity.reserve(ledger, resource, baseline.limit - baseline.live);
        bool refused = false;
        try {
            detail::MaskCharge excess{};
            excess.acquire(ledger, resource, 1);
        } catch (const detail::MaskBudgetFailure& failure) { refused = failure.resource == resource; }
        require(refused, "each resource refuses limit plus one without mutation");
        capacity.commit();
        {
            std::lock_guard<std::mutex> lock((*ledger).mutex);
            const detail::MaskUsage& usage = (*ledger).usage[index];
            require(usage.live == usage.limit && usage.reserved == 0 && usage.peak == usage.limit, "reservation transfers without double charge");
        }
        capacity.reset();
        {
            std::lock_guard<std::mutex> lock((*ledger).mutex);
            const detail::MaskUsage& usage = (*ledger).usage[index];
            require(usage.live == baseline.live && usage.reserved == 0, "each resource releases to exact baseline");
        }
    }
}
void test_service_lifetime(const std::span<const std::byte> bytes) {
    TextMaskLease survivor{};
    std::shared_ptr<detail::MaskLedger> ledger{};
    {
        Fixture fixture(bytes);
        ledger = (*detail::TextMaskAccess::state(fixture.service)).ledger;
        const TextMaskResult result = prepare(fixture, {.utf8 = "survivor"}, survivor);
        require(result.status == TextMaskStatus::success, "service lifetime fixture prepares");
    }
    TextMaskBudgetSnapshot budget = (*ledger).snapshot();
    require(budget.masks.live == 1 && budget.source_keys.live == 1 && budget.font_banks.live == 1 && budget.utf8.live == 8,
        "last lease retains source and font charges after service destruction");
    require(survivor.source_utf8() == "survivor" && survivor.coverage()[0] == 255, "lease bytes survive service destruction");
    survivor.reset();
    budget = (*ledger).snapshot();
    require(budget.masks.live == 0 && budget.source_keys.live == 0 && budget.font_banks.live == 0 && budget.utf8.live == 0 &&
        budget.coverage.live == 0 && budget.encoded_fonts.live == 0 && budget.request_slots.live == 0,
        "last owner destroys all retained resources");
    const std::shared_ptr<detail::MaskLedger> fresh = detail::make_mask_ledger();
    const TextMaskBudgetSnapshot baseline = (*fresh).snapshot();
    require(budget.metadata.live == baseline.metadata.live && budget.metadata.reserved == 0, "all metadata including control blocks returns to bootstrap baseline");
}
void test_service_close_and_cached_wake(const std::span<const std::byte> bytes) {
    Fixture fixture(bytes);
    TextMaskLease lease{};
    TextMaskResult result = prepare(fixture, {}, lease);
    require(result.status == TextMaskStatus::success, "cached wake fixture prepares");
    const std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (fixture.wake.count.load(std::memory_order_acquire) == 0) {
        require(std::chrono::steady_clock::now() < deadline, "initial wake delivered");
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    const TextMaskSessionSnapshot acknowledged = (*fixture.session).snapshot();
    require(acknowledged.occupied == 0, "initial wake acknowledged after consumption");
    const unsigned previous = fixture.wake.count.load(std::memory_order_acquire);
    TextMaskRequestId id{};
    result = (*fixture.session).submit(fixture.bank, {}, id);
    require(result.status == TextMaskStatus::success, "cached completion submitted");
    while (fixture.wake.count.load(std::memory_order_acquire) == previous) {
        require(std::chrono::steady_clock::now() < deadline, "cache hit posts a worker wake");
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    require(!fixture.wake.wrong_executor.load(std::memory_order_acquire), "cache completion never invokes target on caller executor");
    result = (*fixture.session).take(id, lease);
    require(result.status == TextMaskStatus::success, "cached completion consumed");
    fixture.backend.block();
    result = (*fixture.session).submit(fixture.bank, {.utf8 = "service-close"}, id);
    require(result.status == TextMaskStatus::success, "service close fixture admitted");
    fixture.backend.wait_entered();
    fixture.service.begin_close();
    const TextMaskSessionSnapshot snapshot = (*fixture.session).snapshot();
    require(snapshot.closing && !snapshot.joined && snapshot.retiring == 1, "service begin_close does not join blocked work");
    result = (*fixture.session).submit(fixture.bank, {}, id);
    require(result.status == TextMaskStatus::closing, "service close revokes session admission");
    fixture.backend.release();
    (*fixture.session).join_and_release();
}
}

int main(const int argc, char** const argv) {
    try {
        require(argc == 2, "pass approved Carlito fixture directory");
        const std::vector<std::byte> bytes = prepared_test::read_font(std::filesystem::path(argv[1]));
        test_queue(bytes);
        test_retention_and_limits(bytes);
        test_empty_object_cap(bytes);
        test_validation(bytes);
        test_close(bytes);
        test_source_capacity(bytes);
        test_budget_rollback(bytes);
        test_executor_and_default(bytes);
        test_all_resource_reservations();
        test_service_lifetime(bytes);
        test_service_close_and_cached_wake(bytes);
        std::cout << "text mask lifecycle: eleven groups passed (fixture backend only)\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
