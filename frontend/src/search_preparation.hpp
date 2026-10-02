#pragma once

#include "file_manager/filesystem_model.hpp"
#include "fileman_orchestrator/client.hpp"

namespace file_manager {

// Owned worker-to-UI value. Entries are path observations made during preparation,
// not proof that a cached provider match still has the same identity or metadata.
// Ordinary open/mutation operations must continue their own identity revalidation.
struct PreparedSearchPage final {
    fileman::orchestrator::SearchPageInfo page{};
    std::vector<DirectoryEntry> entries{};
    std::size_t rejected{};
    bool cancelled{};
};

// Root and cancellation target are borrowed only during this synchronous call.
// No GUI object is accessed. Cancellation publishes an empty cancelled value.
// A native filesystem call already in progress finishes before the next check.
// The 500-result ceiling matches the frontend's admitted page-size setting.
[[nodiscard]] PreparedSearchPage prepare_search_page(
    const std::filesystem::path& canonical_root,
    fileman::orchestrator::SearchPageInfo page,
    const CancellationCheck& cancelled = {});

} // namespace file_manager
