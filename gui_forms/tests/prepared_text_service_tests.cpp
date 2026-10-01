#include "prepared_text_test_support.hpp"
#include "../src/render/text/prepared_storage.hpp"

#include <cstdlib>
#include <iostream>

namespace {
using namespace prepared_test;

void test_generations(std::span<const std::byte> bytes) {
    PreparedTextService service{};
    EncodedFontLease bank = make_bank(service, bytes);
    Wake wake{};
    std::unique_ptr<PreparedTextSession> session{};
    PreparedTextStatus status = service.open_session(bank, &wake, session);
    require(status == PreparedTextStatus::success, "session opens");
    const PreparedTextKey key = make_key(service, bank, "hello");
    std::array<PreparedTextLayout, 3> layouts{};
    for (std::size_t index = 0; index < layouts.size(); ++index) prepare(service, *session, key, "hello", layouts[index]);
    PreparedTextBudgetSnapshot budget = service.budget_snapshot();
    require(budget.payload_generations == 3 && budget.payload_reserved_bytes == 24U * 1024U * 1024U,
        "all old generations remain charged");
    // A retained command holds the typed immutable payload after its wrapper dies.
    std::shared_ptr<const detail::PreparedTextStorage> retained = detail::PreparedTextAccess::layout(layouts[0]);
    layouts[0] = PreparedTextLayout{};
    LayoutAuthority authority{};
    status = (*session).desire(key, authority);
    require(status == PreparedTextStatus::success, "fourth desire");
    PrepareInput input{};
    status = make_input(service, key, "hello", input);
    require(status == PreparedTextStatus::success, "fourth input admitted separately");
    status = (*session).submit(authority, input);
    require(status == PreparedTextStatus::busy && !input.empty(), "fourth payload denied and input preserved");
    retained.reset();
    status = (*session).submit(authority, input);
    require(status == PreparedTextStatus::success && input.empty(), "actual retirement permits admission");
    const PreparedTextSessionSnapshot ready = wait_ready(*session);
    require(ready.completion == PreparedTextStatus::success, "fourth completes");
    status = (*session).adopt_ready(authority, layouts[0]);
    require(status == PreparedTextStatus::success, "fourth adopts");
    (*session).join_and_release();
    std::unique_ptr<PreparedTextSession> reopened{};
    status = service.open_session(bank, nullptr, reopened);
    require(status == PreparedTextStatus::success, "new session after join");
    status = (*reopened).desire(key, authority);
    require(status == PreparedTextStatus::success, "new session desire");
    status = make_input(service, key, "hello", input);
    require(status == PreparedTextStatus::success, "new session input");
    status = (*reopened).submit(authority, input);
    require(status == PreparedTextStatus::busy && !input.empty(), "session reopening cannot evade payload ledger");
    require(wake.count.load(std::memory_order_acquire) == 4, "one wake per observed completion");
}

void test_authority(std::span<const std::byte> bytes) {
    PreparedTextService service{};
    EncodedFontLease bank = make_bank(service, bytes);
    Wake wake{};
    std::unique_ptr<PreparedTextSession> session{};
    PreparedTextStatus status = service.open_session(bank, &wake, session);
    require(status == PreparedTextStatus::success, "session opens");
    const PreparedTextKey key = make_key(service, bank, "hello");
    PreparedTextLayout old{};
    prepare(service, *session, key, "hello", old);
    const LayoutAuthority old_authority = old.authority();
    LayoutAuthority next{};
    status = (*session).desire(key, next);
    require(status == PreparedTextStatus::success && next.epoch != old_authority.epoch, "same key advances epoch");
    PrepareInput input{};
    status = make_input(service, key, "hello", input);
    require(status == PreparedTextStatus::success, "input created");
    status = (*session).submit(old_authority, input);
    require(status == PreparedTextStatus::stale && !input.empty(), "old authority cannot submit");
    PreparedTextKey invalid = key;
    invalid.scale = 0;
    LayoutAuthority unchanged = next;
    status = (*session).desire(invalid, unchanged);
    require(status == PreparedTextStatus::unsupported_profile && same_layout_authority(unchanged, next), "invalid desire preserves authority");
    status = (*session).submit(next, input);
    require(status == PreparedTextStatus::success && input.empty(), "valid authority preserved internally");
    const PreparedTextSessionSnapshot ready = wait_ready(*session);
    require(ready.completion == PreparedTextStatus::success, "completion");
    (*session).cancel();
    status = (*session).adopt_ready(next, old);
    require(status == PreparedTextStatus::stale && same_layout_authority(old.authority(), old_authority), "cancelled adoption preserves output");
    status = (*session).desire(key, unchanged);
    require(status == PreparedTextStatus::success, "same key after cancel");
    status = (*session).adopt_ready(unchanged, old);
    require(status == PreparedTextStatus::stale, "same key cannot revive cancelled result");
    PreparedTextBudgetSnapshot budget = service.budget_snapshot();
    require(budget.payload_generations == 2, "cancel is not retirement");
    status = (*session).discard_ready();
    require(status == PreparedTextStatus::success, "discard ready");
    budget = service.budget_snapshot();
    require(budget.payload_generations == 1, "discard retires completed owner");
    status = make_input(service, key, "hello", input);
    require(status == PreparedTextStatus::success, "close input");
    status = (*session).submit(unchanged, input);
    require(status == PreparedTextStatus::success, "close job admitted");
    (*session).begin_close();
    const unsigned revoked_wakes = wake.count.load(std::memory_order_acquire);
    (*session).join_and_release();
    require(wake.count.load(std::memory_order_acquire) == revoked_wakes, "close revokes subsequent posts");
    const PreparedTextSessionSnapshot closed = (*session).inspect_ready();
    require(closed.closing && closed.joined && closed.slot == PreparedTextSlot::empty, "joined slot cleared");
    budget = service.budget_snapshot();
    require(budget.payload_generations == 1, "join retains only caller old layout");
}

void test_validation_and_fonts(std::span<const std::byte> bytes) {
    PreparedTextService service{};
    EncodedFontLease first = make_bank(service, bytes);
    EncodedFontLease shared = first;
    EncodedFontLease second = make_bank(service, bytes);
    const std::array<PreparedFontSource, 1> sources{{{.encoded = bytes, .role = FontRole::content}}};
    EncodedFontLease denied{};
    PreparedTextStatus status = service.create_font_bank(sources, denied);
    require(status == PreparedTextStatus::busy && denied.empty(), "third font bank denied");
    first = EncodedFontLease{};
    status = service.create_font_bank(sources, denied);
    require(status == PreparedTextStatus::busy, "shared font lease extends charge");
    shared = EncodedFontLease{};
    status = service.create_font_bank(sources, denied);
    require(status == PreparedTextStatus::success, "font lease retirement permits bank");
    const PreparedTextKey key = make_key(service, second, "hello");
    PrepareInput input{};
    status = make_input(service, key, "hello", input, {false, true});
    require(status == PreparedTextStatus::context_required && input.empty(), "requires paragraph proof");
    status = make_input(service, key, "hello", input);
    require(status == PreparedTextStatus::success, "valid input");
    const PreparedTextKey tab = make_key(service, second, "a\tb");
    status = make_input(service, tab, "a\tb", input);
    require(status == PreparedTextStatus::unsupported_profile && !input.empty(), "unsupported checked before busy preserves old input");
    service.begin_close();
    PrepareInput closed{};
    status = make_input(service, key, "hello", closed);
    require(status == PreparedTextStatus::closing && closed.empty(), "closed service refuses input");
    const PreparedTextBudgetSnapshot budget = service.budget_snapshot();
    require(budget.input_owners == 1 && budget.font_banks == 2, "close does not uncharge external owners");
}

void test_failed_slot(std::span<const std::byte> bytes) {
    PreparedTextService service{};
    const std::array<std::byte, 8> invalid_font{};
    const std::array<PreparedFontSource, 1> sources{{{.encoded = invalid_font, .role = FontRole::content}}};
    EncodedFontLease bank{};
    PreparedTextStatus status = service.create_font_bank(sources, bank);
    require(status == PreparedTextStatus::incompatible_font && bank.empty(), "invalid native font refused before session");
    bank = make_bank(service, bytes);
    std::unique_ptr<PreparedTextSession> session{};
    status = service.open_session(bank, nullptr, session);
    require(status == PreparedTextStatus::success, "valid font session");
    const std::string_view missing = "\xf4\x8f\xbf\xbf";
    const PreparedTextKey key = make_key(service, bank, missing);
    LayoutAuthority authority{};
    status = (*session).desire(key, authority);
    require(status == PreparedTextStatus::success, "failed slot desire");
    PrepareInput input{};
    status = make_input(service, key, missing, input);
    require(status == PreparedTextStatus::success, "failed slot input");
    status = (*session).submit(authority, input);
    require(status == PreparedTextStatus::success, "failed slot submit");
    const PreparedTextSessionSnapshot ready = wait_ready(*session);
    require(ready.completion == PreparedTextStatus::missing_font_coverage, "missing coverage reported");
    const PreparedTextBudgetSnapshot before = service.budget_snapshot();
    require(before.payload_generations == 1, "failure retains reservation");
    PreparedTextLayout output{};
    status = (*session).adopt_ready(authority, output);
    require(status == PreparedTextStatus::missing_font_coverage && output.empty(), "failure cannot adopt");
    status = (*session).discard_ready();
    require(status == PreparedTextStatus::success, "failed slot discarded");
    const PreparedTextBudgetSnapshot after = service.budget_snapshot();
    require(after.payload_generations == 0, "failed slot retired");
}

void test_font_byte_budget(std::span<const std::byte> bytes) {
    PreparedTextService service{};
    std::vector<std::byte> padded(PreparedTextLimits::font_face_bytes);
    std::copy(bytes.begin(), bytes.end(), padded.begin());
    const std::array<PreparedFontSource, 2> sources{{
        {.encoded = padded, .role = FontRole::content}, {.encoded = padded}}};
    EncodedFontLease full{};
    PreparedTextStatus status = service.create_font_bank(sources, full);
    require(status == PreparedTextStatus::success, "exact aggregate encoded font budget");
    const PreparedTextBudgetSnapshot charged = service.budget_snapshot();
    require(charged.font_banks == 1 && charged.font_bytes == PreparedTextLimits::font_bytes, "eight MiB actual aggregate charge");
    const std::array<PreparedFontSource, 1> small{{{.encoded = bytes, .role = FontRole::content}}};
    EncodedFontLease replacement{};
    status = service.create_font_bank(small, replacement);
    require(status == PreparedTextStatus::busy && replacement.empty(), "second bank cannot exceed aggregate bytes");
    full = EncodedFontLease{};
    status = service.create_font_bank(small, replacement);
    require(status == PreparedTextStatus::success, "retired bank releases actual bytes");
}

void test_mapping(std::span<const std::byte> bytes) {
    PreparedTextService service{};
    EncodedFontLease bank = make_bank(service, bytes);
    PreparedTextKey key = make_key(service, bank, "[x]a");
    key.source.end = SourceByteOffset(102);
    key.page.permitted = key.source;
    const std::array<DocumentMapSpan, 2> mapping{{
        {.source = {SourceByteOffset(100), SourceByteOffset(101)}, .begin = DisplayByteOffset(7), .end = DisplayByteOffset(10), .kind = DocumentMapKind::atomic_token},
        {.source = {SourceByteOffset(101), SourceByteOffset(102)}, .begin = DisplayByteOffset(10), .end = DisplayByteOffset(11)}}};
    std::array<PreparedSourceEndpoint, 3> endpoints{{
        {SourceByteOffset(100), DisplayByteOffset(7)}, {SourceByteOffset(101), DisplayByteOffset(10)},
        {SourceByteOffset(102), DisplayByteOffset(11)}}};
    PrepareInput input{};
    PreparedTextStatus status = service.create_input(key, "[x]a", mapping, endpoints, {true, true}, input);
    require(status == PreparedTextStatus::success, "atomic and identity mapping admitted");
    endpoints[1].display = DisplayByteOffset(8);
    status = service.create_input(key, "[x]a", mapping, endpoints, {true, true}, input);
    require(status == PreparedTextStatus::invalid_input && !input.empty(), "interior atomic endpoint invalid before capacity check");
    const std::array<std::string_view, 6> separators{"a\nb", "a\rb", "a\vb", "a\xc2\x85" "b", "a\xe2\x80\xa8" "b", "a\xe2\x80\xa9" "b"};
    for (std::size_t index = 0; index < separators.size(); ++index) {
        key = make_key(service, bank, separators[index]);
        status = make_input(service, key, separators[index], input);
        require(status == PreparedTextStatus::unsupported_profile, "paragraph separator refused before busy");
    }
}

void test_cross_service_authority(std::span<const std::byte> bytes) {
    PreparedTextService first_service{};
    PreparedTextService second_service{};
    EncodedFontLease first_bank = make_bank(first_service, bytes);
    EncodedFontLease second_bank = make_bank(second_service, bytes);
    std::unique_ptr<PreparedTextSession> first_session{};
    std::unique_ptr<PreparedTextSession> second_session{};
    PreparedTextStatus status = first_service.open_session(first_bank, nullptr, first_session);
    require(status == PreparedTextStatus::success, "first independent session");
    status = second_service.open_session(second_bank, nullptr, second_session);
    require(status == PreparedTextStatus::success, "second independent session");
    const PreparedTextKey first_key = make_key(first_service, first_bank, "same");
    const PreparedTextKey second_key = make_key(second_service, second_bank, "same");
    PreparedTextLayout first_layout{};
    PreparedTextLayout second_layout{};
    prepare(first_service, *first_session, first_key, "same", first_layout);
    prepare(second_service, *second_session, second_key, "same", second_layout);
    const LayoutAuthority first_authority = first_layout.authority();
    const LayoutAuthority second_authority = second_layout.authority();
    require(first_authority.epoch == second_authority.epoch && first_authority.session != second_authority.session,
        "session identity unique across independent services at equal epoch");
    GrayTextMask output{};
    status = rasterize_prepared_text(first_layout, first_authority, output);
    require(status == PreparedTextStatus::success, "valid initial mask");
    const std::uint8_t* pixels = output.pixels().data();
    status = rasterize_prepared_text(first_layout, second_authority, output);
    require(status == PreparedTextStatus::stale && output.pixels().data() == pixels, "foreign authority cannot rasterize another service layout");
    LayoutAuthority first_next{};
    LayoutAuthority second_next{};
    status = (*first_session).desire(first_key, first_next);
    require(status == PreparedTextStatus::success, "first replacement desire");
    status = (*second_session).desire(second_key, second_next);
    require(status == PreparedTextStatus::success && first_next.epoch == second_next.epoch, "equal replacement epochs");
    PrepareInput input{};
    status = make_input(first_service, first_key, "same", input);
    require(status == PreparedTextStatus::success, "replacement input");
    status = (*first_session).submit(first_next, input);
    require(status == PreparedTextStatus::success, "replacement submit");
    const PreparedTextSessionSnapshot ready = wait_ready(*first_session);
    require(ready.completion == PreparedTextStatus::success, "replacement ready");
    status = (*first_session).adopt_ready(second_next, first_layout);
    require(status == PreparedTextStatus::stale && same_layout_authority(first_layout.authority(), first_authority),
        "foreign authority adoption preserves old layout");
    const PreparedTextSessionSnapshot preserved = (*first_session).inspect_ready();
    require(preserved.slot == PreparedTextSlot::ready, "foreign authority preserves ready slot");
    status = (*first_session).adopt_ready(first_next, first_layout);
    require(status == PreparedTextStatus::success, "correct authority still adopts ready result");
}
} // namespace

int main(int argc, char** argv) {
    try {
        require(argc == 2, "font directory required");
        const std::vector<std::byte> bytes = read_font(std::filesystem::path(argv[1]));
        test_generations(bytes);
        test_authority(bytes);
        test_validation_and_fonts(bytes);
        test_failed_slot(bytes);
        test_font_byte_budget(bytes);
        test_mapping(bytes);
        test_cross_service_authority(bytes);
        std::cout << "prepared service checks passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
