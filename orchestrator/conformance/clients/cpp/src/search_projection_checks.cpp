#include "search_projection.hpp"

#include <iostream>
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
} // namespace

int main() {
    try {
        test_owned_coverage();
        test_live_and_unreported_coverage();
        test_malformed_coverage();
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
