#include "../src/core/text/prepared_window/prepared_window_batch.hpp"
#include <array>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <stdexcept>

namespace probe {
thread_local bool enabled = false;
thread_local std::size_t fail_at = 0;
thread_local std::size_t attempts = 0;
thread_local gui_forms::detail::PreparedWindowBatchAuthority* close_on_rows = nullptr;
void* allocate(const std::size_t bytes) {
    if (enabled) {
        const std::size_t index = attempts;
        ++attempts;
        if (index == fail_at) throw std::bad_alloc();
        if (index == 1 && close_on_rows) (*close_on_rows).closing = true;
    }
    void* memory = std::malloc(bytes ? bytes : 1);
    if (!memory) throw std::bad_alloc();
    return memory;
}
class Scope final {
public:
    explicit Scope(const std::size_t failure) { attempts = 0; fail_at = failure; enabled = true; }
    ~Scope() { enabled = false; }
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
};
}
void* operator new(const std::size_t bytes) { void* memory = probe::allocate(bytes); return memory; }
void* operator new[](const std::size_t bytes) { void* memory = probe::allocate(bytes); return memory; }
void operator delete(void* const memory) noexcept { std::free(memory); }
void operator delete[](void* const memory) noexcept { std::free(memory); }
void operator delete(void* const memory, std::size_t) noexcept { std::free(memory); }
void operator delete[](void* const memory, std::size_t) noexcept { std::free(memory); }

namespace gf = gui_forms;
namespace detail = gui_forms::detail;
void require(const bool value, const char* const message) {
    if (!value) throw std::runtime_error(message);
}
struct Fixture final {
    std::shared_ptr<detail::PreparedLedger> ledger{std::make_shared<detail::PreparedLedger>()};
    std::shared_ptr<detail::PreparedWindowBatchAuthority> authority{std::make_shared<detail::PreparedWindowBatchAuthority>()};
    std::shared_ptr<detail::PreparedFontBank> fonts{std::make_shared<detail::PreparedFontBank>()};
    detail::PreparedWindowKey key{};
    detail::PreparedWindowInput input{};
    Fixture() {
        (*fonts).ledger = ledger;
        (*fonts).identity = 1;
        (*fonts).generation = 1;
        // A private storage fixture, not an actual validated font/shaping bank.
        require((*fonts).reservation.acquire(ledger, detail::PreparedResource::font_bank, 64) ==
            gf::PreparedTextStatus::success, "Fixture font charge");
        key.controller_instance = 1;
        key.projection_generation = 1;
        key.authority = {1, 1};
        key.text.page.revision = {1, 1};
        key.text.page.serial = 1;
        key.text.page.permitted = {gf::SourceByteOffset(0), gf::SourceByteOffset(3)};
        key.text.source = key.text.page.permitted;
        key.text.display_end = gf::DisplayByteOffset(3);
        key.text.layout_serial = 1;
        key.text.provider_instance = 1;
        key.text.font_set = 1;
        key.text.font_generation = 1;
        key.text.context_generation = 1;
        key.text.font.size = 14;
        require(detail::desire_prepared_window(*authority, key) == gf::PreparedTextStatus::success, "Initial desire");
        own();
    }
    void own() {
        const gf::DocumentMapSpan map{key.text.source, gf::DisplayByteOffset(0), gf::DisplayByteOffset(3),
            gf::DocumentMapKind::identity_utf8};
        std::array<gf::PreparedSourceEndpoint, 4> endpoints{};
        for (std::uint32_t index = 0; index < endpoints.size(); ++index)
            endpoints[index] = {gf::SourceByteOffset(index), gf::DisplayByteOffset(index)};
        std::array<detail::PreparedWindowParagraph, 2> rows{};
        rows[0] = {{gf::SourceByteOffset(0), gf::SourceByteOffset(1)},
            gf::DisplayByteOffset(0), gf::DisplayByteOffset(1), {true, true}, detail::PreparedWindowSeparator::lf,
            {gf::SourceByteOffset(1), gf::SourceByteOffset(2)}, gf::DisplayByteOffset(1), gf::DisplayByteOffset(2)};
        rows[1] = {{gf::SourceByteOffset(2), gf::SourceByteOffset(3)},
            gf::DisplayByteOffset(2), gf::DisplayByteOffset(3), {true, true}, detail::PreparedWindowSeparator::none,
            {gf::SourceByteOffset(3), gf::SourceByteOffset(3)}, gf::DisplayByteOffset(3), gf::DisplayByteOffset(3)};
        const detail::PreparedWindowView view{key, "a\nb", {&map, 1}, endpoints, rows, true};
        require(detail::own_prepared_window(view, ledger, input) == gf::PreparedTextStatus::success, "Fixture owned input");
    }
    gf::PreparedTextStatus admit(std::unique_ptr<detail::PreparedWindowBatchStorage>& output) {
        const gf::PreparedTextStatus status = detail::admit_prepared_window(key, authority, ledger, fonts, input, output);
        return status;
    }
};
void require_input_unchanged(const Fixture& fixture, const char* const original,
                             const std::size_t bytes) {
    require(fixture.input && fixture.input.data().display.get() == original &&
        (*fixture.ledger).usage.input_owners == 1 && (*fixture.ledger).usage.input_bytes == bytes &&
        (*fixture.ledger).usage.payload_generations == 0 && (*fixture.ledger).usage.payload_reserved_bytes == 0 &&
        (*fixture.ledger).usage.font_banks == 1 && (*fixture.ledger).usage.font_bytes == 64,
        "Failure preserves input allocation and unrelated ledger accounting");
}
void failure_tests() {
    Fixture fixture{};
    const char* const original = fixture.input.data().display.get();
    const std::size_t bytes = fixture.input.data().charged_bytes;
    std::unique_ptr<detail::PreparedWindowBatchStorage> output{};
    detail::PreparedWindowInput empty{};
    require(detail::admit_prepared_window(fixture.key, fixture.authority, fixture.ledger, fixture.fonts, empty, output) ==
        gf::PreparedTextStatus::invalid_input && !output, "Empty uncertified input refused");
    const std::shared_ptr<detail::PreparedLedger> foreign = std::make_shared<detail::PreparedLedger>();
    require(detail::admit_prepared_window(fixture.key, fixture.authority, foreign, fixture.fonts, fixture.input, output) ==
        gf::PreparedTextStatus::invalid_input && !output, "Wrong ledger refused");
    require_input_unchanged(fixture, original, bytes);
    require(detail::desire_prepared_window(*fixture.authority, fixture.key) == gf::PreparedTextStatus::stale,
        "Same epoch cannot replace desire");
    detail::PreparedWindowKey changed = fixture.key;
    ++changed.projection_generation;
    require(detail::admit_prepared_window(changed, fixture.authority, fixture.ledger, fixture.fonts, fixture.input, output) ==
        gf::PreparedTextStatus::stale, "Full window key checked before transfer");
    (*fixture.fonts).generation = 2;
    require(fixture.admit(output) == gf::PreparedTextStatus::incompatible_font, "Font generation checked");
    (*fixture.fonts).generation = 1;
    (*fixture.authority).executor = std::thread::id{};
    require(fixture.admit(output) == gf::PreparedTextStatus::wrong_executor, "Executor mismatch refused");
    (*fixture.authority).executor = std::this_thread::get_id();
    (*fixture.authority).closing = true;
    require(fixture.admit(output) == gf::PreparedTextStatus::closing, "Authority closing refused");
    (*fixture.authority).closing = false;
    (*fixture.ledger).closing = true;
    require(fixture.admit(output) == gf::PreparedTextStatus::closing, "Ledger closing refused");
    (*fixture.ledger).closing = false;
    require_input_unchanged(fixture, original, bytes);
    for (std::size_t failure = 0; failure < 2; ++failure) {
        gf::PreparedTextStatus status{};
        {
            probe::Scope scope(failure);
            status = fixture.admit(output);
        }
        require(status == gf::PreparedTextStatus::resource_failure && !output && probe::attempts == failure + 1,
            "Each aggregate allocation failure rolls back");
        require_input_unchanged(fixture, original, bytes);
    }
    gf::PreparedTextStatus closed_at_commit{};
    {
        probe::Scope scope(std::numeric_limits<std::size_t>::max());
        probe::close_on_rows = fixture.authority.get();
        closed_at_commit = fixture.admit(output);
        probe::close_on_rows = nullptr;
    }
    require(closed_at_commit == gf::PreparedTextStatus::closing && !output,
        "Closing after allocation is rechecked before taking caller input");
    (*fixture.authority).closing = false;
    require_input_unchanged(fixture, original, bytes);
    gf::PreparedTextStatus success{};
    {
        probe::Scope scope(std::numeric_limits<std::size_t>::max());
        success = fixture.admit(output);
    }
    require(success == gf::PreparedTextStatus::success && probe::attempts == 2 && !fixture.input,
        "One owner and one row-array allocation, no text copies");
    require((*(*output).input).display.get() == original && (*output).row_count == 2 &&
        (*output).rows[1].paragraph_index == 1 && (*output).requested_bytes ==
            bytes + sizeof(detail::PreparedWindowBatchStorage) + 2 * sizeof(detail::PreparedWindowRowStorage),
        "Original arrays transfer and exact controlled allocations are counted");
    require((*fixture.ledger).usage.input_owners == 0 && (*fixture.ledger).usage.input_bytes == 0 &&
        (*fixture.ledger).usage.payload_generations == 1 &&
        (*fixture.ledger).usage.payload_reserved_bytes == gf::PreparedTextLimits::payload_bytes,
        "One aggregate payload replaces the input-slot charge");
    fixture.own();
    const detail::PreparedWindowBatchStorage* const retained = output.get();
    require(fixture.admit(output) == gf::PreparedTextStatus::busy && output.get() == retained && fixture.input,
        "Occupied output preserves both owners");
}
void retirement_tests() {
    Fixture fixture{};
    std::array<std::shared_ptr<const detail::PreparedWindowBatchStorage>, 3> leases{};
    for (std::size_t index = 0; index < leases.size(); ++index) {
        if (index) {
            ++fixture.key.authority.epoch;
            require(detail::desire_prepared_window(*fixture.authority, fixture.key) == gf::PreparedTextStatus::success,
                "New desire advances authority");
            fixture.own();
        }
        std::unique_ptr<detail::PreparedWindowBatchStorage> output{};
        require(fixture.admit(output) == gf::PreparedTextStatus::success, "Admit retained generation");
        leases[index] = std::shared_ptr<const detail::PreparedWindowBatchStorage>(std::move(output));
    }
    require(!detail::prepared_window_batch_current(*leases[0]) && detail::prepared_window_batch_current(*leases[2]),
        "Identical text with newer authority revokes old batch without releasing its memory");
    fixture.own();
    std::unique_ptr<detail::PreparedWindowBatchStorage> fourth{};
    require(fixture.admit(fourth) == gf::PreparedTextStatus::busy && fixture.input && !fourth,
        "Fourth generation refused with original input retained");
    std::shared_ptr<const detail::PreparedWindowBatchStorage> recorded = leases[0];
    leases[0].reset();
    require(fixture.admit(fourth) == gf::PreparedTextStatus::busy, "Dropping wrapper does not retire recorded lease");
    recorded.reset();
    require(fixture.admit(fourth) == gf::PreparedTextStatus::success && !fixture.input,
        "Final lease retirement restores capacity");
    fourth.reset();
    leases[1].reset();
    leases[2].reset();
    require((*fixture.ledger).usage.payload_generations == 0 && (*fixture.ledger).usage.payload_reserved_bytes == 0,
        "All retired generations release exact charges");
    (*fixture.authority).desired.reset();
    require(detail::desire_prepared_window(*fixture.authority, fixture.key) == gf::PreparedTextStatus::stale,
        "Revoking desire does not allow the same epoch to revive");
    detail::PreparedWindowKey foreign = fixture.key;
    ++foreign.authority.epoch;
    ++foreign.controller_instance;
    require(detail::desire_prepared_window(*fixture.authority, foreign) == gf::PreparedTextStatus::invalid_input,
        "Revocation retains controller identity");
    foreign = fixture.key;
    ++foreign.authority.epoch;
    ++foreign.authority.session;
    require(detail::desire_prepared_window(*fixture.authority, foreign) == gf::PreparedTextStatus::invalid_input,
        "Revocation retains session identity");
    fixture.key.authority.epoch = std::numeric_limits<std::uint64_t>::max();
    require(detail::desire_prepared_window(*fixture.authority, fixture.key) == gf::PreparedTextStatus::success,
        "Last epoch may be desired");
    (*fixture.authority).desired.reset();
    fixture.key.authority.epoch = 1;
    require(detail::desire_prepared_window(*fixture.authority, fixture.key) == gf::PreparedTextStatus::generation_exhausted,
        "Exhaustion cannot recycle an earlier authority");
}
struct WorkerObservation final {
    Fixture& fixture;
    const detail::PreparedWindowBatchStorage& batch;
    gf::PreparedTextStatus observed{gf::PreparedTextStatus::pending};
    gf::PreparedTextStatus desired{gf::PreparedTextStatus::pending};
    gf::PreparedTextStatus admitted{gf::PreparedTextStatus::pending};
    bool owner_current{};
    void run() {
        observed = detail::prepared_window_worker_current(batch);
        owner_current = detail::prepared_window_batch_current(batch);
        detail::PreparedWindowKey next = fixture.key;
        ++next.authority.epoch;
        desired = detail::desire_prepared_window(*fixture.authority, next);
        std::unique_ptr<detail::PreparedWindowBatchStorage> output{};
        admitted = fixture.admit(output);
    }
};
void worker_observation_tests() {
    Fixture fixture{};
    std::unique_ptr<detail::PreparedWindowBatchStorage> batch{};
    require(fixture.admit(batch) == gf::PreparedTextStatus::success, "Worker observation admission");
    fixture.own();
    WorkerObservation observation{fixture, *batch};
    std::thread worker(&WorkerObservation::run, &observation);
    worker.join();
    require(observation.observed == gf::PreparedTextStatus::success && !observation.owner_current,
        "Worker observation does not require owner executor");
    require(observation.desired == gf::PreparedTextStatus::wrong_executor &&
        observation.admitted == gf::PreparedTextStatus::wrong_executor && fixture.input,
        "Read-only observation grants no mutation authority");
    {
        std::lock_guard<std::mutex> lock((*fixture.authority).mutex);
        ++(*(*fixture.authority).desired).projection_generation;
    }
    require(detail::prepared_window_worker_current(*batch) == gf::PreparedTextStatus::stale,
        "Worker checks full identity even when epoch is unchanged");
    {
        std::lock_guard<std::mutex> lock((*fixture.ledger).mutex);
        (*fixture.ledger).closing = true;
    }
    require(detail::prepared_window_worker_current(*batch) == gf::PreparedTextStatus::closing,
        "Ledger closing takes precedence over stale work");
    require((*fixture.ledger).usage.payload_generations == 1U,
        "Revocation and closing preserve retained generation charge");
}
int main() {
    try {
        failure_tests();
        retirement_tests();
        worker_observation_tests();
        std::cout << "Prepared-window batch admission tests passed\n";
        return 0;
    } catch (const std::exception& failure) {
        std::cerr << failure.what() << '\n';
        return 1;
    }
}
