#include "search_projection.hpp"

#include <iostream>
#include <array>
#include <cmath>
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

void test_owned_match_evidence() {
    const std::string_view annotations = R"(,"rank":18446744073709551615,"certainty":-2.5e0,"evidence":[{"kind":"future-kind","channel":"unknown/channel","score":1.25e+2,"exact":false,"inferred":true,"calibration":"scale-v2","anchor":"a/b","observed_at":"provider-clock","details":{"nested":[null,true,false,"quote\" slash\\ newline\n nul\u0000 music\uD834\uDD1E",{"large":184467440737095516160001,"tiny":1.2300e-900,"negative_zero":-0.00,"empty":{}}],"empty":[]}},{"kind":"exact_path","channel":"exact","score":-0.0,"exact":true,"inferred":false}])";
    std::string bytes = source_response({}, "42", {}, annotations);
    const orc::SearchPageInfo page = orc::detail::project_search_response(bytes, "source-test");
    bytes.assign(bytes.size(), 'x');
    bytes.clear();
    bytes.shrink_to_fit();
    const orc::SearchResultInfo& row = page.results.at(0U);
    require(row.rank == std::numeric_limits<std::uint64_t>::max() && row.certainty == -2.5,
            "owned rank and certainty must preserve their domains without clamping");
    require(row.evidence && (*row.evidence).size() == 2U, "ordered evidence must survive response retirement");
    const orc::SearchEvidenceInfo& first = (*row.evidence)[0U];
    const orc::SearchEvidenceInfo& second = (*row.evidence)[1U];
    require(first.kind == "future-kind" && first.channel == "unknown/channel" && first.score == 125.0 &&
                !first.exact && first.inferred && first.calibration == "scale-v2" && first.anchor == "a/b" &&
                first.observed_at == "provider-clock", "unknown evidence and optional strings must remain owned observations");
    require(second.kind == "exact_path" && second.channel == "exact" && second.score == 0.0 &&
                std::signbit(second.score) && second.exact && !second.inferred && !second.calibration &&
                !second.anchor && !second.observed_at && !second.details_json,
            "ordered second evidence must retain signed zero and unreported fields");
    // Canonical key ordering is permitted. Number spellings and every nested
    // value/array position must survive, including values beyond binary64.
    const std::string expected_details = R"({"empty":[],"nested":[null,true,false,"quote\" slash\\ newline\n nul\u0000 music)"
        "\xF0\x9D\x84\x9E"
        R"(",{"empty":{},"large":184467440737095516160001,"negative_zero":-0.00,"tiny":1.2300e-900}]})";
    require(first.details_json == expected_details, "nested inert details lost values or numeric spelling");
    const std::string expected_first = *first.details_json;
    std::string replay_annotations = R"(,"evidence":[{"kind":"replay","channel":"inert","score":0,"exact":false,"inferred":false,"details":)";
    replay_annotations += expected_first;
    replay_annotations += "}]";
    const std::string replay_bytes = source_response({}, "0", {}, replay_annotations);
    const orc::SearchPageInfo replay = orc::detail::project_search_response(replay_bytes, "source-test");
    const orc::SearchResultInfo& replay_row = replay.results.at(0U);
    require(replay_row.evidence && (*replay_row.evidence)[0U].details_json == expected_first,
            "retained details must remain valid JSON after response/tree destruction");
}

void test_optional_match_fields() {
    const std::string missing_bytes = source_response({}, "0", {}, {});
    const orc::SearchPageInfo missing = orc::detail::project_search_response(missing_bytes, "source-test");
    const orc::SearchResultInfo& missing_row = missing.results.at(0U);
    require(!missing_row.rank && !missing_row.certainty && !missing_row.evidence,
            "missing match fields must not be synthesized");
    const std::string null_bytes = source_response({}, "0", {}, R"(,"rank":null,"certainty":null,"evidence":null)");
    const orc::SearchPageInfo nulls = orc::detail::project_search_response(null_bytes, "source-test");
    const orc::SearchResultInfo& null_row = nulls.results.at(0U);
    require(!null_row.rank && !null_row.certainty && !null_row.evidence, "null match fields must remain unreported");
    const std::string empty_bytes = source_response({}, "0", {}, R"(,"rank":0,"certainty":0,"evidence":[])");
    const orc::SearchPageInfo empty = orc::detail::project_search_response(empty_bytes, "source-test");
    const orc::SearchResultInfo& empty_row = empty.results.at(0U);
    require(empty_row.rank == 0U && empty_row.certainty == 0.0 && empty_row.evidence && (*empty_row.evidence).empty(),
            "reported zero and empty evidence must remain distinguishable from absence");
    const std::string evidence_bytes = source_response({}, "0", {}, R"(,"evidence":[{"kind":"","channel":"","score":0,"exact":false,"inferred":false,"calibration":"","anchor":"","observed_at":"","details":{}},{"kind":"k","channel":"c","score":0,"exact":true,"inferred":false,"calibration":null,"anchor":null,"observed_at":null,"details":null}])");
    const orc::SearchPageInfo evidence_page = orc::detail::project_search_response(evidence_bytes, "source-test");
    const orc::SearchResultInfo& row = evidence_page.results.at(0U);
    require(row.evidence && (*row.evidence).size() == 2U, "optional evidence fixture missing");
    const orc::SearchEvidenceInfo& reported = (*row.evidence)[0U];
    const orc::SearchEvidenceInfo& unreported = (*row.evidence)[1U];
    require(reported.kind.empty() && reported.channel.empty() && reported.calibration == "" &&
                reported.anchor == "" && reported.observed_at == "" && reported.details_json == "{}",
            "reported empty evidence strings and object must survive");
    require(!unreported.calibration && !unreported.anchor && !unreported.observed_at && !unreported.details_json,
            "null optional evidence values must remain unreported");
}

void test_match_numeric_boundaries() {
    const std::string bytes = source_response({}, "0", {}, R"(,"certainty":1.7976931348623157e308,"evidence":[{"kind":"small","channel":"c","score":4.9406564584124654e-324,"exact":false,"inferred":false},{"kind":"negative","channel":"c","score":-1.7976931348623157e308,"exact":false,"inferred":true}])");
    const orc::SearchPageInfo page = orc::detail::project_search_response(bytes, "source-test");
    const orc::SearchResultInfo& row = page.results.at(0U);
    require(row.certainty == std::numeric_limits<double>::max() && row.evidence && (*row.evidence).size() == 2U,
            "finite double maximum must be accepted");
    require((*row.evidence)[0U].score == std::numeric_limits<double>::denorm_min() &&
                (*row.evidence)[1U].score == -std::numeric_limits<double>::max(),
            "representable subnormal and negative maximum must survive");
    const std::array<std::string_view, 8> ranks{{"-1", "-0", "1.0", "1e0", "18446744073709551616", "true", "[]", "\"1\""}};
    for (const std::string_view rank : ranks) {
        std::string revision = ",\"rank\":";
        revision.append(rank);
        reject_source({}, "0", {}, revision);
    }
    const std::array<std::string_view, 10> invalid{{"1e309", "-1e309", "1e-999", "NaN", "Infinity", "\"NaN\"", "\"0.5\"", "false", "[]", "{}"}};
    for (const std::string_view number : invalid) {
        std::string certainty = ",\"certainty\":";
        certainty.append(number);
        reject_source({}, "0", {}, certainty);
        std::string evidence = R"(,"evidence":[{"kind":"k","channel":"c","score":)";
        evidence.append(number);
        evidence += R"(,"exact":true,"inferred":false}])";
        reject_source({}, "0", {}, evidence);
    }
}

void test_malformed_match_evidence() {
    const std::array<std::string_view, 15> invalid{{
        "null", "[]", "{}",
        R"({"kind":0,"channel":"c","score":0,"exact":true,"inferred":false})",
        R"({"kind":"k","channel":false,"score":0,"exact":true,"inferred":false})",
        R"({"kind":"k","channel":"c","score":null,"exact":true,"inferred":false})",
        R"({"kind":"k","channel":"c","score":0,"exact":1,"inferred":false})",
        R"({"kind":"k","channel":"c","score":0,"exact":true,"inferred":"false"})",
        R"({"kind":"k","channel":"c","score":0,"exact":true})",
        R"({"kind":"k","channel":"c","score":0,"exact":true,"inferred":false,"calibration":3})",
        R"({"kind":"k","channel":"c","score":0,"exact":true,"inferred":false,"anchor":[]})",
        R"({"kind":"k","channel":"c","score":0,"exact":true,"inferred":false,"observed_at":{}})",
        R"({"kind":"k","channel":"c","score":0,"exact":true,"inferred":false,"details":[]})",
        R"({"kind":"k","channel":"c","score":0,"exact":true,"inferred":false,"details":1})",
        R"({"kind":"k","channel":"c","score":0,"exact":true,"inferred":false,"details":"{}"})"
    }};
    for (const std::string_view entry : invalid) {
        // A valid first entry ensures failure happens after useful local work.
        std::string evidence = R"(,"evidence":[{"kind":"valid","channel":"c","score":1,"exact":true,"inferred":false},)";
        evidence.append(entry);
        evidence += ']';
        reject_source({}, "0", {}, evidence);
    }
    reject_source({}, "0", {}, R"(,"evidence":{})");
    reject_source({}, "0", {}, R"(,"evidence":false)");
    // The first row is complete; a malformed later row must prevent assignment
    // of the whole new page, retaining the caller's prior owned page unchanged.
    std::string late = source_response({}, "0", {}, R"(,"rank":1,"evidence":[])");
    late.resize(late.size() - 3U);
    late += R"(,{"object":{"path":"/later"},"metadata":{"name":"later","kind":"file","size":0},"unavailable":false,"evidence":[{"kind":"bad","channel":"c","score":1e999,"exact":true,"inferred":false}]}]}})";
    orc::SearchPageInfo prior{};
    prior.source = "prior-page";
    bool rejected = false;
    try { prior = orc::detail::project_search_response(late, "source-test"); }
    catch (const orc::ClientError&) { rejected = true; }
    require(rejected && prior.source == "prior-page" && prior.results.empty(),
            "late malformed evidence must not publish the preceding valid row");
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
        test_owned_match_evidence();
        test_optional_match_fields();
        test_match_numeric_boundaries();
        test_malformed_match_evidence();
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
