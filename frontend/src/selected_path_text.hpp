#pragma once

#include "file_manager/filesystem_model.hpp"

#include <cstddef>
#include <span>
#include <string>
#include <unordered_map>

namespace file_manager::detail {

enum class PathTextStatus { ready, missing_entry, too_large, invalid_encoding };

struct PathText final {
    PathTextStatus status{PathTextStatus::missing_entry};
    std::string text{};
};

// Plain native paths separated by LF, without shell quoting or a final LF.
// This is readable clipboard text, not a reversible filename-list encoding:
// an embedded newline in a filename is preserved. No file is opened or resolved.
// Inputs are borrowed only for this synchronous call. Failure publishes no text.
[[nodiscard]] PathText selected_path_text(
    const std::unordered_map<std::string, DirectoryEntry>& entries,
    std::span<const std::string> selected,
    const std::filesystem::path& location, std::size_t maximum_bytes);

} // namespace file_manager::detail
