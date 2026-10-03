#pragma once

#include "fileman_orchestrator/client.hpp"
#include <string_view>

namespace fileman::orchestrator::detail {

// Private projection seam used by the transport client and conformance tests.
// Borrows response/id only during parsing; returns an owned page or throws
// ClientError without publishing a partial page. No transport or filesystem I/O.
// Client supplies an already frame-bounded response; tests supply bounded fixtures.
[[nodiscard]] SearchPageInfo project_search_response(
    std::string_view response, std::string_view expected_id);

} // namespace fileman::orchestrator::detail
