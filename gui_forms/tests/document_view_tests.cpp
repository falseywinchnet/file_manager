#include "gui_forms/document_view.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace {
using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}

SourceByteRange range(std::uint64_t begin, std::uint64_t end) {
    const SourceByteRange result{SourceByteOffset(begin), SourceByteOffset(end)};
    return result;
}

DocumentViewport viewport(std::uint64_t anchor) {
    const DocumentViewport result{.anchor = SourceByteOffset(anchor), .horizontal_dip = 0};
    return result;
}

void bind(DocumentViewState& state, std::uint64_t size, DocumentRevision revision = {1, 1}) {
    const DocumentViewStatus status = state.bind(revision, SourceByteOffset(size));
    require(status == DocumentViewStatus::success, "binding must succeed");
}

DocumentPageRequest request(DocumentViewState& state, std::uint64_t begin, std::uint64_t end) {
    const SourceByteRange permitted = range(begin, end);
    const DocumentViewport desired = viewport(begin);
    const DocumentRequestResult result = state.request_page(desired, permitted);
    require(result.status == DocumentViewStatus::success && result.request.has_value(), "request success");
    return *result.request;
}

DocumentPage identity_page(const DocumentPageRequest& token, std::string_view text) {
    DocumentPage page{};
    page.request = token;
    page.covered = token.permitted;
    page.display_utf8.assign(text);
    if (!page.display_utf8.empty()) {
        const std::uint32_t end = static_cast<std::uint32_t>(page.display_utf8.size());
        const DocumentMapSpan span{.source = page.covered, .begin = DisplayByteOffset(0),
            .end = DisplayByteOffset(end), .kind = DocumentMapKind::identity_utf8};
        page.mapping.push_back(span);
    }
    return page;
}

void publish(DocumentViewState& state, DocumentPage& page) {
    const DocumentViewStatus status = state.publish(std::move(page));
    require(status == DocumentViewStatus::success, "publication success");
    require(page.display_utf8.empty() && page.mapping.empty() && page.request.serial == 0,
            "publication consumes and empties former owner");
}

void finish(DocumentViewState& state, const DocumentPageRequest& token) {
    const DocumentViewStatus status = state.finish(token);
    require(status == DocumentViewStatus::success, "release acknowledgement success");
}

void test_empty_and_huge_offsets() {
    DocumentViewState state{};
    const SourceByteRange empty_range{};
    const DocumentRequestResult unbound = state.request_page(DocumentViewport{}, empty_range);
    require(unbound.status == DocumentViewStatus::unavailable, "unbound unavailable");
    bind(state, 0);
    const DocumentPageRequest empty_token = request(state, 0, 0);
    DocumentPage empty_page = identity_page(empty_token, "");
    publish(state, empty_page);
    const SourceMappingResult empty = state.source_position(empty_token, DisplayByteOffset(0));
    require(empty.status == DocumentViewStatus::success && (*empty.position).value == 0, "empty mapping");

    const std::uint64_t end = std::numeric_limits<std::uint64_t>::max();
    const std::uint64_t begin = end - 3;
    bind(state, end, {2, 1});
    const DocumentPageRequest token = request(state, begin, end);
    DocumentPage page = identity_page(token, "abc");
    publish(state, page);
    for (std::uint32_t index = 0; index <= 3; ++index) {
        const SourceMappingResult source = state.source_position(token, DisplayByteOffset(index));
        const std::uint64_t expected = begin + index;
        require(source.status == DocumentViewStatus::success && (*source.position).value == expected,
                "uint64 edge must remain exact, beyond double precision");
        const DisplayMappingResult display = state.display_position(token, SourceByteOffset(expected));
        require(display.status == DocumentViewStatus::success && (*display.position).value == index,
                "uint64 edge roundtrip");
    }
    const DocumentPageRequest eof_token = request(state, end, end);
    DocumentPage eof_page = identity_page(eof_token, "");
    publish(state, eof_page);
    const SourceMappingResult eof = state.source_position(eof_token, DisplayByteOffset(0));
    require(eof.status == DocumentViewStatus::success && (*eof.position).value == end, "EOF exact boundary");
}

void test_slots_and_validation_precedence() {
    DocumentViewState state{};
    bind(state, 100'000);
    const DocumentPageRequest first = request(state, 0, 3);
    const DocumentPageRequest second = request(state, 3, 6);
    const std::size_t occupied = state.pending_count();
    require(occupied == 2, "two slots reserved");
    const SourceByteRange three = range(0, 3);
    const DocumentRequestResult invalid = state.request_page(viewport(4), three);
    require(invalid.status == DocumentViewStatus::invalid_range, "invalid precedes busy");
    const SourceByteRange excessive = range(0, 65'537);
    const DocumentRequestResult budget = state.request_page(viewport(0), excessive);
    require(budget.status == DocumentViewStatus::budget_exceeded, "source budget precedes busy");
    const DocumentRequestResult busy = state.request_page(viewport(0), three);
    require(busy.status == DocumentViewStatus::busy && !busy.request.has_value(), "third request cannot start");
    const DocumentViewport retained = state.viewport();
    require(retained.anchor.value == 3, "busy and invalid preserve viewport");
    DocumentPage newest = identity_page(second, "def");
    publish(state, newest);
    DocumentPage older = identity_page(first, "abc");
    const DocumentViewStatus rejected = state.publish(std::move(older));
    require(rejected != DocumentViewStatus::success && older.display_utf8 == "abc",
            "old response cannot overwrite newest");
    const std::size_t after_reject = state.pending_count();
    require(after_reject == 1, "rejected payload retains slot until ownership release");
    older = DocumentPage{};
    finish(state, first);
    const DocumentViewStatus duplicate = state.finish(first);
    require(duplicate == DocumentViewStatus::stale, "duplicate ack refused");

    const DocumentPageRequest cancelled = request(state, 0, 3);
    state.cancel();
    DocumentPage cancelled_page = identity_page(cancelled, "abc");
    const DocumentViewStatus cancelled_status = state.publish(std::move(cancelled_page));
    require(cancelled_status == DocumentViewStatus::cancelled, "cancel revokes publication");
    const std::size_t cancelled_count = state.pending_count();
    require(cancelled_count == 1, "cancel does not release occupied slot");
    cancelled_page = DocumentPage{};
    finish(state, cancelled);
}

void test_full_token_and_binding() {
    DocumentViewState state{};
    bind(state, 6);
    const DocumentPageRequest token = request(state, 0, 3);
    DocumentPageRequest forged = token;
    forged.viewport.horizontal_dip = 4;
    DocumentPage forgery = identity_page(forged, "abc");
    const DocumentViewStatus forged_publish = state.publish(std::move(forgery));
    const DocumentViewStatus forged_finish = state.finish(forged);
    require(forged_publish == DocumentViewStatus::stale && forged_finish == DocumentViewStatus::stale,
            "same serial with changed full token refused");
    forgery = DocumentPage{};
    DocumentPage valid = identity_page(token, "abc");
    publish(state, valid);

    const DocumentPageRequest next = request(state, 3, 6);
    DocumentPage next_page = identity_page(next, "def");
    publish(state, next_page);
    const SourceMappingResult stale = state.source_position(token, DisplayByteOffset(1));
    const DisplayMappingResult stale_reverse = state.display_position(token, SourceByteOffset(1));
    const SourceMappingResult current = state.source_position(next, DisplayByteOffset(1));
    require(stale.status == DocumentViewStatus::stale && stale_reverse.status == DocumentViewStatus::stale,
            "same-revision page replacement invalidates BOTH old mapping directions");
    require(current.status == DocumentViewStatus::success && (*current.position).value == 4,
            "new page maps same display position differently");
    for (std::size_t index = 0; index < 3; ++index) {
        DocumentPageRequest forged_mapping = next;
        if (index == 0) { forged_mapping.viewport.horizontal_dip = 1; }
        if (index == 1) { forged_mapping.permitted.begin = SourceByteOffset(2); }
        if (index == 2) { ++forged_mapping.revision.revision; }
        const SourceMappingResult source = state.source_position(forged_mapping, DisplayByteOffset(1));
        const DisplayMappingResult display = state.display_position(forged_mapping, SourceByteOffset(4));
        require(source.status == DocumentViewStatus::stale && display.status == DocumentViewStatus::stale,
                "exact mapping rejects altered viewport, permitted interval and revision");
    }

    const DocumentPageRequest pending = request(state, 0, 3);
    const DocumentViewStatus invalid_bind = state.bind({1, 1}, SourceByteOffset(7));
    require(invalid_bind == DocumentViewStatus::invalid_revision, "same revision cannot change size");
    bind(state, 6, {1, 1});
    const std::optional<DocumentPage>& retained = state.page();
    require(retained.has_value(), "identical bind retains page");
    bind(state, 3, {2, 1});
    const std::size_t occupied = state.pending_count();
    const std::optional<DocumentPage>& cleared = state.page();
    require(occupied == 1 && !cleared.has_value(), "new identity clears page but not old slots");
    DocumentPage old_identity = identity_page(pending, "abc");
    const DocumentViewStatus stale_identity = state.publish(std::move(old_identity));
    require(stale_identity == DocumentViewStatus::stale, "old identity refused");
    old_identity = DocumentPage{};
    finish(state, pending);
    const DocumentPageRequest fresh = request(state, 0, 3);
    require(fresh.serial > pending.serial, "serial survives bind");
    state.cancel();
    finish(state, fresh);
}

void test_mapping_units() {
    DocumentViewState state{};
    bind(state, 16);
    const DocumentPageRequest token = request(state, 0, 16);
    DocumentPage page{};
    page.request = token;
    page.covered = token.permitted;
    // Literal label, multibyte e-acute, one illegal byte and one CRLF token.
    page.display_utf8 = "[BYTE FF] A\xc3\xa9[BYTE FF][CRLF]";
    const std::array<DocumentMapSpan, 3> units{{
        {.source = range(0, 13), .begin = DisplayByteOffset(0), .end = DisplayByteOffset(13),
         .kind = DocumentMapKind::identity_utf8},
        {.source = range(13, 14), .begin = DisplayByteOffset(13), .end = DisplayByteOffset(22),
         .kind = DocumentMapKind::atomic_token},
        {.source = range(14, 16), .begin = DisplayByteOffset(22), .end = DisplayByteOffset(28),
         .kind = DocumentMapKind::atomic_token}}};
    page.mapping.assign(units.begin(), units.end());
    publish(state, page);
    const SourceMappingResult literal = state.source_position(token, DisplayByteOffset(2));
    require(literal.status == DocumentViewStatus::success && (*literal.position).value == 2,
            "literal labels never reverse parsed");
    const SourceMappingResult scalar = state.source_position(token, DisplayByteOffset(12));
    const SourceMappingResult atomic = state.source_position(token, DisplayByteOffset(15));
    const DisplayMappingResult crlf = state.display_position(token, SourceByteOffset(15));
    require(scalar.status == DocumentViewStatus::invalid_boundary &&
            atomic.status == DocumentViewStatus::invalid_boundary &&
            crlf.status == DocumentViewStatus::invalid_boundary, "scalar, label and CRLF interiors refused");
    const std::array<std::uint32_t, 4> display_boundaries{0, 13, 22, 28};
    const std::array<std::uint64_t, 4> source_boundaries{0, 13, 14, 16};
    for (std::size_t index = 0; index < display_boundaries.size(); ++index) {
        const SourceMappingResult source = state.source_position(token, DisplayByteOffset(display_boundaries[index]));
        const DisplayMappingResult display = state.display_position(token, SourceByteOffset(source_boundaries[index]));
        require(source.status == DocumentViewStatus::success && (*source.position).value == source_boundaries[index],
                "source shared endpoints");
        require(display.status == DocumentViewStatus::success && (*display.position).value == display_boundaries[index],
                "display shared endpoints");
    }
}

void test_refusal_preserves_page_and_capacity() {
    DocumentViewState state{};
    bind(state, 3);
    const DocumentPageRequest first = request(state, 0, 3);
    DocumentPage original = identity_page(first, "abc");
    publish(state, original);
    const DocumentPageRequest token = request(state, 0, 3);
    DocumentPage malformed = identity_page(token, "abc");
    malformed.mapping[0].source.begin = SourceByteOffset(1);
    DocumentViewStatus status = state.publish(std::move(malformed));
    require(status == DocumentViewStatus::invalid_page, "mapping gap refused");
    malformed.mapping[0].source.begin = SourceByteOffset(0);
    malformed.display_utf8[0] = static_cast<char>(0xff);
    status = state.publish(std::move(malformed));
    require(status == DocumentViewStatus::invalid_page, "invalid display UTF8 refused");
    malformed.display_utf8[0] = 'a';
    const std::size_t excessive_display_capacity = DocumentViewLimits::display_capacity + 1;
    malformed.display_utf8.reserve(excessive_display_capacity);
    status = state.publish(std::move(malformed));
    require(status == DocumentViewStatus::budget_exceeded, "small live string with excessive capacity refused");
    DocumentPage replacement = identity_page(token, "abc");
    malformed = std::move(replacement);
    const std::size_t excessive_mapping_capacity = DocumentViewLimits::mapping_capacity + 1;
    malformed.mapping.reserve(excessive_mapping_capacity);
    status = state.publish(std::move(malformed));
    require(status == DocumentViewStatus::budget_exceeded, "mapping capacity separate from length");
    const std::optional<DocumentPage>& retained = state.page();
    require((*retained).display_utf8 == "abc" && (*retained).request.serial == first.serial,
            "all failures preserve prior page");
    malformed = DocumentPage{};
    finish(state, token);

    // Simulate terminal producer failures without installing a global allocator
    // hook. The model itself allocates no payloads during request/finish/publish.
    const DocumentPageRequest allocation_token = request(state, 0, 3);
    try { throw std::bad_alloc(); }
    catch (const std::bad_alloc&) { finish(state, allocation_token); }
    const DocumentPageRequest context_token = request(state, 0, 3);
    const DocumentViewStatus producer_context = DocumentViewStatus::context_required;
    require(producer_context != DocumentViewStatus::success, "context outcome is not empty success");
    finish(state, context_token);
    const std::size_t remaining = state.pending_count();
    require(remaining == 0 && (*retained).request.serial == first.serial, "failure acknowledgements do not publish");
}

void test_large_mapping_reference_and_reuse() {
    DocumentViewState state{};
    bind(state, 65'536);
    for (std::size_t cycle = 0; cycle < 3; ++cycle) {
        const DocumentPageRequest token = request(state, 0, 65'536);
        DocumentPage page{};
        page.request = token;
        page.covered = token.permitted;
        constexpr std::size_t display_capacity = 9 * 65'536;
        page.display_utf8.reserve(display_capacity);
        page.mapping.reserve(65'536);
        for (std::uint32_t index = 0; index < 65'536; ++index) {
            page.display_utf8.append("[BYTE FF]");
            const std::uint32_t display_begin = 9 * index;
            const std::uint32_t display_end = display_begin + 9;
            const std::uint64_t source_begin = index;
            const std::uint64_t source_end = source_begin + 1;
            const DocumentMapSpan span{.source = range(index, source_end),
                .begin = DisplayByteOffset(display_begin), .end = DisplayByteOffset(display_end),
                .kind = DocumentMapKind::atomic_token};
            page.mapping.push_back(span);
        }
        publish(state, page);
        for (std::uint32_t source = 0; source <= 65'536; source += 16) {
            const std::uint32_t expected = source * 9;
            const DisplayMappingResult actual = state.display_position(token, SourceByteOffset(source));
            require(actual.status == DocumentViewStatus::success && (*actual.position).value == expected,
                    "maximum mapping matches arithmetic reference");
            const SourceMappingResult reverse = state.source_position(token, DisplayByteOffset(expected));
            require(reverse.status == DocumentViewStatus::success && (*reverse.position).value == source,
                    "maximum mapping inverse");
        }
    }
}

void test_invalid_viewports_and_page_partitions() {
    DocumentViewState state{};
    bind(state, 20);
    const DocumentPageRequest token = request(state, 0, 3);
    const DocumentPageRequest later = request(state, 3, 6);
    const SourceByteRange permitted = range(0, 3);
    const double infinity = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const std::array<double, 3> invalid_offsets{-1.0, infinity, nan};
    for (std::size_t index = 0; index < invalid_offsets.size(); ++index) {
        DocumentViewport invalid = viewport(0);
        invalid.horizontal_dip = invalid_offsets[index];
        const DocumentRequestResult result = state.request_page(invalid, permitted);
        require(result.status == DocumentViewStatus::invalid_range, "invalid DIP precedes capacity");
    }
    const SourceByteRange reversed = range(3, 0);
    const SourceByteRange outside = range(0, 21);
    const DocumentViewport origin = viewport(0);
    const DocumentRequestResult reversed_result = state.request_page(origin, reversed);
    const DocumentRequestResult outside_result = state.request_page(origin, outside);
    require(reversed_result.status == DocumentViewStatus::invalid_range &&
            outside_result.status == DocumentViewStatus::invalid_range, "range validation before slot lookup");
    DocumentPage valid = identity_page(later, "def");
    publish(state, valid);
    finish(state, token);

    const DocumentPageRequest next = request(state, 0, 3);
    const std::array<SourceByteRange, 3> invalid_coverage{
        range(0, 0), range(0, 4), range(1, 3)};
    for (std::size_t index = 0; index < invalid_coverage.size(); ++index) {
        DocumentPage page = identity_page(next, "abc");
        page.covered = invalid_coverage[index];
        const DocumentViewStatus status = state.publish(std::move(page));
        require(status == DocumentViewStatus::invalid_page, "nonEOF empty, excessive, missing anchor coverage");
    }
    DocumentPage malformed = identity_page(next, "abc");
    malformed.mapping[0].end = DisplayByteOffset(2);
    DocumentViewStatus status = state.publish(std::move(malformed));
    require(status == DocumentViewStatus::invalid_page, "unequal identity lengths refused");
    malformed.mapping[0].end = DisplayByteOffset(4);
    status = state.publish(std::move(malformed));
    require(status == DocumentViewStatus::invalid_page, "display span beyond payload refused");
    malformed.mapping[0].end = DisplayByteOffset(3);
    malformed.mapping[0].kind = static_cast<DocumentMapKind>(255);
    status = state.publish(std::move(malformed));
    require(status == DocumentViewStatus::invalid_page, "invalid enum refused");
    malformed = DocumentPage{};

    DocumentPageRequest forged = next;
    forged.permitted.end = SourceByteOffset(2);
    const DocumentViewStatus changed_interval = state.finish(forged);
    forged = next;
    ++forged.revision.revision;
    const DocumentViewStatus changed_revision = state.finish(forged);
    require(changed_interval == DocumentViewStatus::stale && changed_revision == DocumentViewStatus::stale,
            "all token fields participate in release authority");
    finish(state, next);

    const DocumentViewStatus zero_identity = state.bind({0, 1}, SourceByteOffset(20));
    const DocumentViewStatus zero_revision = state.bind({1, 0}, SourceByteOffset(20));
    require(zero_identity == DocumentViewStatus::invalid_revision &&
            zero_revision == DocumentViewStatus::invalid_revision, "zero identities rejected");
    bind(state, 20, {1, 2});
    const DocumentViewStatus backward = state.bind({1, 1}, SourceByteOffset(20));
    require(backward == DocumentViewStatus::stale, "revision cannot decrease");
}

} // namespace

int main() {
    try {
        test_empty_and_huge_offsets();
        test_slots_and_validation_precedence();
        test_full_token_and_binding();
        test_mapping_units();
        test_refusal_preserves_page_and_capacity();
        test_large_mapping_reference_and_reuse();
        test_invalid_viewports_and_page_partitions();
        std::cout << "document view D1 fixtures passed; map span bytes=" << sizeof(DocumentMapSpan) << '\n';
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
