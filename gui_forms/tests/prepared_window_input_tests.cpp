#include "../src/core/text/prepared_window/prepared_window_input.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstdlib>
#include <new>
#include <limits>
#include <type_traits>

namespace allocation_probe {
thread_local bool enabled = false;
thread_local std::size_t fail_at = 0;
thread_local std::size_t attempts = 0;
void* allocate(const std::size_t bytes) {
    if (enabled) {
        const std::size_t current_attempt = attempts;
        ++attempts;
        if (current_attempt == fail_at) throw std::bad_alloc();
    }
    void* memory = std::malloc(bytes ? bytes : 1);
    if (!memory) throw std::bad_alloc();
    return memory;
}
class Scope final {
public:
    explicit Scope(const std::size_t index) {
        attempts = 0;
        fail_at = index;
        enabled = true;
    }
    ~Scope() { enabled = false; }
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
};
}
// Test-executable-only replacement allocation boundary. Injection is enabled
// solely around own_prepared_window; fixture/assertion allocations are excluded.
void* operator new(const std::size_t bytes) {
    void* memory = allocation_probe::allocate(bytes);
    return memory;
}
void* operator new[](const std::size_t bytes) {
    void* memory = allocation_probe::allocate(bytes);
    return memory;
}
void operator delete(void* const memory) noexcept { std::free(memory); }
void operator delete[](void* const memory) noexcept { std::free(memory); }
void operator delete(void* const memory, std::size_t) noexcept { std::free(memory); }
void operator delete[](void* const memory, std::size_t) noexcept { std::free(memory); }

namespace gf = gui_forms;
namespace detail = gui_forms::detail;
namespace {
struct PublishedInputTraits final {
    using DisplayElement = decltype(std::declval<const detail::PreparedWindowInput&>().display[0]);
    using MappingElement = decltype(std::declval<const detail::PreparedWindowInput&>().mappings[0]);
    using EndpointElement = decltype(std::declval<const detail::PreparedWindowInput&>().endpoints[0]);
    using ParagraphElement = decltype(std::declval<const detail::PreparedWindowInput&>().paragraphs[0]);
};
static_assert(std::is_same_v<PublishedInputTraits::DisplayElement, const char&>);
static_assert(std::is_same_v<PublishedInputTraits::MappingElement, const gf::DocumentMapSpan&>);
static_assert(std::is_same_v<PublishedInputTraits::EndpointElement, const gf::PreparedSourceEndpoint&>);
static_assert(std::is_same_v<PublishedInputTraits::ParagraphElement, const detail::PreparedWindowParagraph&>);
}
void require(const bool value, const char* const message) {
    if (!value) throw std::runtime_error(message);
}
bool same_budget(const gf::PreparedTextBudgetSnapshot& left, const gf::PreparedTextBudgetSnapshot& right) {
    const bool equal = left.payload_generations == right.payload_generations &&
        left.payload_reserved_bytes == right.payload_reserved_bytes && left.font_banks == right.font_banks &&
        left.font_bytes == right.font_bytes && left.input_owners == right.input_owners &&
        left.input_bytes == right.input_bytes && left.mask_owners == right.mask_owners && left.mask_bytes == right.mask_bytes;
    return equal;
}
void allocation_failures(const detail::PreparedWindowView& view, const std::shared_ptr<detail::PreparedLedger>& ledger) {
    detail::PreparedReservation unrelated{};
    require(unrelated.acquire(ledger, detail::PreparedResource::font_bank, 64) == gf::PreparedTextStatus::success,
        "Unrelated ledger reservation");
    const gf::PreparedTextBudgetSnapshot original = (*ledger).usage;
    std::unique_ptr<const detail::PreparedWindowInput> owned{};
    gf::PreparedTextStatus status{};
    std::size_t allocations = 0;
    {
        allocation_probe::Scope scope(std::numeric_limits<std::size_t>::max());
        status = detail::own_prepared_window(view, ledger, owned);
        allocations = allocation_probe::attempts;
    }
    require(status == gf::PreparedTextStatus::success && allocations == 5,
        "Populated input allocates one owner and four arrays");
    owned.reset();
    require(same_budget(original, (*ledger).usage), "Successful owner retirement preserves unrelated accounting");
    for (std::size_t failure = 0; failure < allocations; ++failure) {
        std::size_t attempts = 0;
        {
            allocation_probe::Scope scope(failure);
            status = detail::own_prepared_window(view, ledger, owned);
            attempts = allocation_probe::attempts;
        }
        require(status == gf::PreparedTextStatus::resource_failure && !owned && attempts == failure + 1,
            "Every failed allocation refuses before immutable publication");
        require(same_budget(original, (*ledger).usage), "Every failed allocation releases reservation and owners");
    }
}
detail::PreparedWindowKey key_for(const std::size_t source, const std::size_t display) {
    detail::PreparedWindowKey key{};
    key.controller_instance = 1;
    key.projection_generation = 1;
    key.authority = {1, 1};
    key.text.page.revision = {1, 1};
    key.text.page.serial = 1;
    key.text.page.permitted = {gf::SourceByteOffset(0), gf::SourceByteOffset(source)};
    key.text.source = key.text.page.permitted;
    key.text.display_end = gf::DisplayByteOffset(static_cast<std::uint32_t>(display));
    key.text.layout_serial = 1;
    key.text.provider_instance = 1;
    key.text.font_set = 1;
    key.text.font_generation = 1;
    key.text.context_generation = 1;
    key.text.font.size = 14;
    return key;
}
detail::PreparedWindowParagraph paragraph(const std::uint32_t begin, const std::uint32_t end,
    const detail::PreparedWindowSeparator kind, const std::uint32_t after) {
    detail::PreparedWindowParagraph row{};
    row.content = {gf::SourceByteOffset(begin), gf::SourceByteOffset(end)};
    row.display_begin = gf::DisplayByteOffset(begin);
    row.display_end = gf::DisplayByteOffset(end);
    row.proof = {true, true};
    row.separator = kind;
    row.separator_source = {gf::SourceByteOffset(end), gf::SourceByteOffset(after)};
    row.separator_begin = gf::DisplayByteOffset(end);
    row.separator_end = gf::DisplayByteOffset(after);
    return row;
}
void projection_tests() {
    using Separator = detail::PreparedWindowSeparator;
    const std::string text = "[CRLF]";
    gf::DocumentMapSpan map{{gf::SourceByteOffset(0), gf::SourceByteOffset(2)},
        gf::DisplayByteOffset(0), gf::DisplayByteOffset(6), gf::DocumentMapKind::atomic_token};
    std::vector<gf::PreparedSourceEndpoint> endpoints{
        {gf::SourceByteOffset(0), gf::DisplayByteOffset(0)},
        {gf::SourceByteOffset(2), gf::DisplayByteOffset(6)}};
    std::vector<detail::PreparedWindowParagraph> rows{
        paragraph(0, 0, Separator::crlf, 2), paragraph(2, 2, Separator::none, 2)};
    rows[0].separator_end.value = 6;
    rows[1].display_begin.value = 6;
    rows[1].display_end.value = 6;
    rows[1].separator_begin.value = 6;
    rows[1].separator_end.value = 6;
    detail::PreparedWindowView view{key_for(2, 6), text, {&map, 1}, endpoints, rows, true};
    std::size_t bytes = 0;
    require(detail::validate_prepared_window(view, bytes) == gf::PreparedTextStatus::success,
        "Explicit atomic separator projection has unequal source/display extents");
    map.source.end.value = 6;
    map.kind = gf::DocumentMapKind::identity_utf8;
    endpoints.back().source.value = 6;
    const detail::PreparedWindowParagraph literal = paragraph(0, 6, Separator::none, 6);
    view.key = key_for(6, 6);
    view.paragraphs = {&literal, 1};
    require(detail::validate_prepared_window(view, bytes) == gf::PreparedTextStatus::success,
        "Literal separator-label lookalike stays one content paragraph");
    const std::string unicode = "\xc3\xa9";
    map.source.end.value = 2;
    map.end.value = 2;
    endpoints.back() = {gf::SourceByteOffset(2), gf::DisplayByteOffset(2)};
    detail::PreparedWindowParagraph row = paragraph(0, 2, Separator::none, 2);
    view = {key_for(2, 2), unicode, {&map, 1}, endpoints, {&row, 1}, true};
    require(detail::validate_prepared_window(view, bytes) == gf::PreparedTextStatus::success, "Complete UTF-8");
    endpoints.insert(endpoints.begin() + 1, {gf::SourceByteOffset(1), gf::DisplayByteOffset(1)});
    view.endpoints = endpoints;
    require(detail::validate_prepared_window(view, bytes) == gf::PreparedTextStatus::invalid_input, "Endpoint cannot split UTF-8");
    row.proof.complete_end = false;
    view.display = std::string_view(unicode).substr(0, 1);
    require(detail::validate_prepared_window(view, bytes) == gf::PreparedTextStatus::context_required,
        "Incomplete paragraph context refuses before classifying split UTF-8");
    std::string lines(511, '\n');
    endpoints.clear();
    rows.clear();
    for (std::uint32_t index = 0; index <= 511; ++index) {
        endpoints.push_back({gf::SourceByteOffset(index), gf::DisplayByteOffset(index)});
        rows.push_back(paragraph(index, index, index == 511 ? Separator::none : Separator::lf,
            index == 511 ? index : index + 1));
    }
    map = {{gf::SourceByteOffset(0), gf::SourceByteOffset(511)}, gf::DisplayByteOffset(0),
        gf::DisplayByteOffset(511), gf::DocumentMapKind::identity_utf8};
    view = {key_for(511, 511), lines, {&map, 1}, endpoints, rows, true};
    require(detail::validate_prepared_window(view, bytes) == gf::PreparedTextStatus::success, "512 empty rows consume bounded metadata");
    view.key.text.display_end.value = 16385;
    require(detail::validate_prepared_window(view, bytes) == gf::PreparedTextStatus::budget_exceeded, "Display budget is aggregate");
    view.key = key_for(65537, 511);
    require(detail::validate_prepared_window(view, bytes) == gf::PreparedTextStatus::budget_exceeded, "Permission budget is aggregate");
}
void run_tests() {
    using Separator = detail::PreparedWindowSeparator;
    std::string text = "a\r\n\nb\r";
    std::vector<gf::DocumentMapSpan> mappings{{{gf::SourceByteOffset(0), gf::SourceByteOffset(6)},
        gf::DisplayByteOffset(0), gf::DisplayByteOffset(6), gf::DocumentMapKind::identity_utf8}};
    std::vector<gf::PreparedSourceEndpoint> endpoints{};
    for (const std::uint32_t offset : {0U, 1U, 3U, 4U, 5U, 6U})
        endpoints.push_back({gf::SourceByteOffset(offset), gf::DisplayByteOffset(offset)});
    std::vector<detail::PreparedWindowParagraph> rows{
        paragraph(0, 1, Separator::crlf, 3), paragraph(3, 3, Separator::lf, 4),
        paragraph(4, 5, Separator::cr, 6), paragraph(6, 6, Separator::none, 6)};
    detail::PreparedWindowView view{key_for(6, 6), text, mappings, endpoints, rows, true};
    std::size_t bytes = 0;
    require(detail::validate_prepared_window(view, bytes) == gf::PreparedTextStatus::success, "Mixed separators and empty EOF");
    const std::size_t admitted = bytes;
    const std::shared_ptr<detail::PreparedLedger> ledger = std::make_shared<detail::PreparedLedger>();
    std::unique_ptr<const detail::PreparedWindowInput> owned{};
    require(detail::own_prepared_window(view, ledger, owned) == gf::PreparedTextStatus::success, "Owned batch");
    require((*ledger).usage.input_owners == 1 && (*ledger).usage.input_bytes == bytes, "Aggregate ledger charged");
    text[0] = 'z';
    require((*owned).display[0] == 'a', "Input owns its immutable copy");
    mappings[0].kind = gf::DocumentMapKind::atomic_token;
    endpoints[0].source.value = 99;
    rows[0].proof.complete_begin = false;
    require((*owned).mappings[0].kind == gf::DocumentMapKind::identity_utf8 &&
        (*owned).endpoints[0].source.value == 0 && (*owned).paragraphs[0].proof.complete_begin,
        "All metadata arrays are owned independently of producer mutations");
    mappings[0].kind = gf::DocumentMapKind::identity_utf8;
    endpoints[0].source.value = 0;
    rows[0].proof.complete_begin = true;
    std::unique_ptr<const detail::PreparedWindowInput> second{};
    require(detail::own_prepared_window(view, ledger, second) == gf::PreparedTextStatus::busy && !second, "One input owner per ledger");
    const std::shared_ptr<detail::PreparedLedger> independent = std::make_shared<detail::PreparedLedger>();
    require(detail::own_prepared_window(view, independent, second) == gf::PreparedTextStatus::success,
        "An independent ledger admits its own input owner");
    require((*independent).usage.input_owners == 1 && (*ledger).usage.input_owners == 1,
        "Separate services retain separate accounting");
    second.reset();
    require((*independent).usage.input_owners == 0 && (*independent).usage.input_bytes == 0 &&
        (*ledger).usage.input_owners == 1 && (*ledger).usage.input_bytes == admitted,
        "Independent retirement cannot release the first ledger's owner");
    rows.back().proof.complete_end = false;
    bytes = 999;
    require(detail::validate_prepared_window(view, bytes) == gf::PreparedTextStatus::context_required && bytes == 999,
        "Late incomplete row preserves reported output");
    require(detail::own_prepared_window(view, ledger, owned) == gf::PreparedTextStatus::busy && (*owned).display[0] == 'a',
        "Occupied output preserved");
    rows.back().proof.complete_end = true;
    owned.reset();
    require((*ledger).usage.input_owners == 0 && (*ledger).usage.input_bytes == 0, "Owner retirement releases charge");
    view.complete_eof = false;
    require(detail::validate_prepared_window(view, bytes) == gf::PreparedTextStatus::context_required, "No invented empty EOF");
    view.complete_eof = true;
    endpoints.insert(endpoints.begin() + 2, {gf::SourceByteOffset(2), gf::DisplayByteOffset(2)});
    view.endpoints = endpoints;
    require(detail::validate_prepared_window(view, bytes) == gf::PreparedTextStatus::context_required, "CRLF cannot split");
    endpoints.erase(endpoints.begin() + 2);
    view.endpoints = endpoints;
    text[1] = 'x';
    require(detail::validate_prepared_window(view, bytes) == gf::PreparedTextStatus::invalid_input, "Identity separator spelling certified");
    text[1] = '\r';
    rows[1].content.begin.value = 2;
    require(detail::validate_prepared_window(view, bytes) == gf::PreparedTextStatus::invalid_input, "No overlapping paragraphs");
    rows[1].content.begin.value = 3;
    std::vector<detail::PreparedWindowParagraph> excess(513, rows.back());
    view.paragraphs = excess;
    require(detail::validate_prepared_window(view, bytes) == gf::PreparedTextStatus::budget_exceeded, "Aggregate row ceiling");
    view.paragraphs = rows;
    std::vector<gf::PreparedSourceEndpoint> excess_endpoints(gf::PreparedTextLimits::metadata_records);
    view.endpoints = excess_endpoints;
    require(detail::validate_prepared_window(view, bytes) == gf::PreparedTextStatus::budget_exceeded, "Combined metadata ceiling");
    view.endpoints = endpoints;
    require(detail::validate_prepared_window(view, bytes) == gf::PreparedTextStatus::success && bytes == admitted, "Restored complete batch");
    allocation_failures(view, ledger);
    detail::PreparedWindowKey changed = view.key;
    require(detail::same_prepared_window_key(changed, view.key), "Exact identity matches");
    ++changed.projection_generation;
    require(!detail::same_prepared_window_key(changed, view.key), "Projection identity differs");
    changed = view.key;
    ++changed.authority.epoch;
    require(!detail::same_prepared_window_key(changed, view.key), "Authority epoch differs");
    changed = view.key;
    ++changed.text.page.serial;
    require(!detail::same_prepared_window_key(changed, view.key), "Full D1 token differs");
    // Equality must distinguish every field even when a changed key would be
    // refused by admission. This verifies identity, not validity of variants.
    std::vector<detail::PreparedWindowKey> variants(27, view.key);
    ++variants[0].controller_instance;
    ++variants[1].authority.session;
    ++variants[2].text.page.revision.document;
    ++variants[3].text.page.revision.revision;
    ++variants[4].text.page.permitted.begin.value;
    ++variants[5].text.page.permitted.end.value;
    ++variants[6].text.page.viewport.anchor.value;
    variants[7].text.page.viewport.horizontal_dip += 1;
    ++variants[8].text.source.begin.value;
    ++variants[9].text.source.end.value;
    ++variants[10].text.display_begin.value;
    ++variants[11].text.display_end.value;
    ++variants[12].text.layout_serial;
    ++variants[13].text.provider_instance;
    ++variants[14].text.provider_generation;
    ++variants[15].text.font_set;
    ++variants[16].text.font_generation;
    ++variants[17].text.context_generation;
    variants[18].text.font.role = gf::FontRole::content;
    variants[19].text.font.size += 1;
    ++variants[20].text.font.weight;
    variants[21].text.font.italic = true;
    variants[22].text.font.letter_spacing += 1;
    variants[23].text.scale += 1;
    variants[24].text.wrap_width += 1;
    ++variants[25].text.tab_columns;
    variants[26].text.raster = static_cast<gf::PreparedRasterProfile>(255);
    for (const detail::PreparedWindowKey& variant : variants) {
        require(!detail::same_prepared_window_key(variant, view.key) &&
            !detail::same_prepared_window_key(view.key, variant), "Every complete key field participates symmetrically");
    }
    view.key.text.page.viewport.anchor.value = 1;
    require(detail::validate_prepared_window(view, bytes) == gf::PreparedTextStatus::context_required, "Interior anchor refuses");
    const gf::PreparedSourceEndpoint eof{gf::SourceByteOffset(0), gf::DisplayByteOffset(0)};
    const detail::PreparedWindowParagraph empty = paragraph(0, 0, Separator::none, 0);
    detail::PreparedWindowView blank{key_for(0, 0), {}, {}, {&eof, 1}, {&empty, 1}, true};
    require(detail::own_prepared_window(blank, ledger, owned) == gf::PreparedTextStatus::success && (*owned).paragraph_count == 1,
        "Empty document owns one row and endpoint, no zero mapping");
    owned.reset();
    (*ledger).closing = true;
    require(detail::own_prepared_window(blank, ledger, owned) == gf::PreparedTextStatus::closing && !owned, "Closing ledger refuses");
}
int main() {
    try {
        run_tests();
        projection_tests();
        std::cout << "Prepared-window private input tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
