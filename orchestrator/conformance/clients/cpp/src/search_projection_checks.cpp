#include "search_projection.hpp"

#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
namespace orc = fileman::orchestrator;

void require(const bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

std::string response(const std::string_view annotations) {
    std::string result = R"({"id":"search-test","status":"partial","result":{"source":"catalogue","complete":false,"generation":17,"cursor":{"source":"catalogue","value":"opaque"},"results":[{"object":{"path":"/docs/example.txt"},"metadata":{"name":"example.txt","kind":"file","size":42},"unavailable":false}])";
    result.append(annotations);
    result += "}}";
    return result;
}

void test_owned_coverage() {
    std::string bytes = response(R"(,"stale_roots":["docs"],"unavailable_roots":["removable"],"warnings":["manual reconcile","coverage incomplete"])" );
    const orc::SearchPageInfo page = orc::detail::project_search_response(bytes, "search-test");
    bytes.clear();
    bytes.shrink_to_fit();
    require(page.terminal == "partial" && !page.complete && page.generation == 17U,
            "terminal, completeness and generation must survive projection");
    require(page.cursor && (*page.cursor).value == "opaque" && page.results.size() == 1U &&
                page.results[0].name == "example.txt" && page.results[0].size == 42U,
            "existing result and continuation projection must remain intact");
    require(page.coverage.stale_roots && (*page.coverage.stale_roots)[0] == "docs" &&
                page.coverage.unavailable_roots && (*page.coverage.unavailable_roots)[0] == "removable" &&
                page.coverage.warnings && (*page.coverage.warnings).size() == 2U &&
                (*page.coverage.warnings)[1] == "coverage incomplete",
            "provider coverage must own its ordered strings after response retirement");
    require(!page.coverage.unavailable_paths && !page.coverage.scan_id,
            "absent live-only fields must not become synthetic observations");
}

void test_live_and_unreported_coverage() {
    const std::string bytes = R"({"id":"live-test","status":"success","result":{"source":"live_filesystem","complete":true,"cursor":null,"scan_id":"scan-1","results":[],"unavailable_paths":["private"],"warnings":[]}})";
    const orc::SearchPageInfo live = orc::detail::project_search_response(bytes, "live-test");
    require(live.coverage.scan_id == "scan-1" && live.coverage.unavailable_paths &&
                (*live.coverage.unavailable_paths)[0] == "private" && live.coverage.warnings &&
                (*live.coverage.warnings).empty() && !live.coverage.stale_roots && !live.generation,
            "live fields and explicitly empty warnings must be distinguishable from absence");
    const std::string missing_bytes = response({});
    const orc::SearchPageInfo missing = orc::detail::project_search_response(missing_bytes, "search-test");
    require(!missing.coverage.warnings && !missing.coverage.stale_roots,
            "older missing fields must remain unreported, not be synthesized as empty");
    const std::string null_bytes = response(R"(,"warnings":null,"stale_roots":null,"scan_id":null)" );
    const orc::SearchPageInfo nulls = orc::detail::project_search_response(null_bytes, "search-test");
    require(!nulls.coverage.warnings && !nulls.coverage.stale_roots && !nulls.coverage.scan_id,
            "null optional observations must remain unreported");
}

void require_rejected(const std::string_view annotations) {
    const std::string bytes = response(annotations);
    bool rejected{};
    try {
        const orc::SearchPageInfo page = orc::detail::project_search_response(bytes, "search-test");
        static_cast<void>(page);
    } catch (const orc::ClientError&) {
        rejected = true;
    }
    require(rejected, "malformed coverage must fail without publishing a usable page");
}

void test_malformed_coverage() {
    require_rejected(R"(,"warnings":"not an array")");
    require_rejected(R"(,"warnings":["valid",42])");
    require_rejected(R"(,"stale_roots":[false])");
    require_rejected(R"(,"unavailable_roots":{})");
    require_rejected(R"(,"unavailable_paths":[null])");
    require_rejected(R"(,"scan_id":42)");
}

std::string source_response(const std::string_view identity,
    const std::string_view size, const std::string_view metadata,
    const std::string_view revision) {
    std::string bytes = R"({"id":"source-test","status":"success","result":{"source":"catalogue","complete":true,"generation":17,"results":[{"object":{"path":"/docs/example.txt")";
    bytes.append(identity);
    bytes += R"(},"metadata":{"name":"example.txt","kind":"file","size":)";
    bytes.append(size);
    bytes.append(metadata);
    bytes += R"(},"unavailable":false)";
    bytes.append(revision);
    bytes += "}]}}";
    return bytes;
}

void test_owned_source_identity_and_integer_domains() {
    std::string bytes = source_response(
        R"(,"root":"docs","id":"opaque-18446744073709551615","incarnation":"birth-1","platform_key":{"volume_serial":"ABCD","file_id":"00000000000000000000000000000001"})",
        "-9223372036854775808",
        R"(,"mode":4294967295,"modified_unix_nano":9223372036854775807)",
        R"(,"generation":18446744073709551615)");
    const orc::SearchPageInfo page = orc::detail::project_search_response(bytes, "source-test");
    bytes.clear();
    bytes.shrink_to_fit();
    const orc::SearchResultInfo& row = page.results.at(0U);
    require(row.object.root_id == "docs" && row.object.file_object_id == "opaque-18446744073709551615" &&
                row.object.incarnation == "birth-1" && row.object.platform_key &&
                (*row.object.platform_key).at("file_id") == "00000000000000000000000000000001",
            "source identity must own exact strings after response retirement");
    require(row.size == std::numeric_limits<std::int64_t>::min() &&
                row.mode == std::numeric_limits<std::uint32_t>::max() &&
                row.modified_unix_nanoseconds == std::numeric_limits<std::int64_t>::max() &&
                row.generation == std::numeric_limits<std::uint64_t>::max() && page.generation == 17U,
            "stored integer domains and row/page generations must remain distinct");
    const std::string alternate = source_response(
        R"(,"root_id":"docs","file_object_id":"semantic-id")", "9223372036854775807",
        R"(,"mode":0,"modified_unix_nano":-9223372036854775808)", R"(,"generation":0)");
    const orc::SearchPageInfo alternate_page = orc::detail::project_search_response(alternate, "source-test");
    const orc::SearchResultInfo& alternate_row = alternate_page.results.at(0U);
    require(alternate_row.object.root_id == "docs" && alternate_row.object.file_object_id == "semantic-id" &&
                alternate_row.size == std::numeric_limits<std::int64_t>::max() && alternate_row.mode == 0U &&
                alternate_row.modified_unix_nanoseconds == std::numeric_limits<std::int64_t>::min() &&
                alternate_row.generation == 0U,
            "semantic identity spellings and the opposite integer endpoints must project exactly");
}

void test_source_optional_fields_and_aliases() {
    const std::string absent = source_response({}, "0", {}, {});
    const orc::SearchPageInfo missing = orc::detail::project_search_response(absent, "source-test");
    const orc::SearchResultInfo& row = missing.results.at(0U);
    require(!row.object.root_id && !row.object.file_object_id && !row.object.incarnation &&
                !row.object.platform_key && !row.mode && !row.modified_unix_nanoseconds && !row.generation,
            "missing source fields must remain unreported despite a reported page generation");
    const std::string nulls = source_response(
        R"(,"root":null,"id":null,"incarnation":null,"platform_key":null)", "0",
        R"(,"mode":null,"modified_unix_nano":null)", R"(,"generation":null)");
    const orc::SearchPageInfo null_page = orc::detail::project_search_response(nulls, "source-test");
    const orc::SearchResultInfo& null_row = null_page.results.at(0U);
    require(!null_row.object.root_id && !null_row.object.file_object_id && !null_row.object.incarnation &&
                !null_row.object.platform_key && !null_row.mode && !null_row.modified_unix_nanoseconds &&
                !null_row.generation, "null source fields must remain unreported");
    const std::string empties = source_response(
        R"(,"root":"","root_id":"","id":null,"file_object_id":"","incarnation":"","platform_key":{})",
        "0", {}, {});
    const orc::SearchPageInfo empty_page = orc::detail::project_search_response(empties, "source-test");
    const orc::SearchObjectIdentityInfo& identity = empty_page.results.at(0U).object;
    require(identity.root_id == "" && identity.file_object_id == "" && identity.incarnation == "" &&
                identity.platform_key && (*identity.platform_key).empty(),
            "reported empty identity fields must survive agreeing and null aliases");
}

void reject_source(const std::string_view identity, const std::string_view size,
    const std::string_view metadata, const std::string_view revision) {
    const std::string bytes = source_response(identity, size, metadata, revision);
    bool rejected{};
    try {
        const orc::SearchPageInfo page = orc::detail::project_search_response(bytes, "source-test");
        static_cast<void>(page);
    } catch (const orc::ClientError&) { rejected = true; }
    require(rejected, "invalid source record must reject the whole page");
}

void test_invalid_source_records() {
    reject_source(R"(,"root":"a","root_id":"b")", "0", {}, {});
    reject_source(R"(,"id":"a","file_object_id":"b")", "0", {}, {});
    reject_source(R"(,"root":42,"root_id":"a")", "0", {}, {});
    reject_source(R"(,"file_object_id":[])" , "0", {}, {});
    reject_source(R"(,"incarnation":false)", "0", {}, {});
    reject_source(R"(,"platform_key":[])", "0", {}, {});
    reject_source(R"(,"platform_key":{"file_id":42})", "0", {}, {});
    reject_source({}, "9223372036854775808", {}, {});
    reject_source({}, "-9223372036854775809", {}, {});
    reject_source({}, "1.0", {}, {});
    reject_source({}, "1e2", {}, {});
    reject_source({}, "null", {}, {});
    reject_source({}, "0", R"(,"mode":4294967296)", {});
    reject_source({}, "0", R"(,"mode":-1)", {});
    reject_source({}, "0", R"(,"modified_unix_nano":9223372036854775808)", {});
    reject_source({}, "0", R"(,"modified_unix_nano":false)", {});
    reject_source({}, "0", {}, R"(,"generation":18446744073709551616)");
    reject_source({}, "0", {}, R"(,"generation":-1)");
}
} // namespace

int main() {
    try {
        test_owned_coverage();
        test_live_and_unreported_coverage();
        test_malformed_coverage();
        test_owned_source_identity_and_integer_domains();
        test_source_optional_fields_and_aliases();
        test_invalid_source_records();
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
