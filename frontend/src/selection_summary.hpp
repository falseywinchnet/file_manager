#pragma once

#include "file_manager/filesystem_model.hpp"

#include <limits>
#include <span>
#include <unordered_map>

namespace file_manager::detail {

// Direct, already observed entries only. No I/O, recursion, retained borrows or
// heap storage. Logical bytes count selected bindings, not unique disk extents.
struct SelectionSummary final {
    std::size_t selected{};
    std::size_t files{};
    std::size_t folders{};
    std::size_t other{};
    std::size_t unavailable_entries{};
    std::size_t known_sizes{};
    std::uint64_t logical_bytes{};
    bool size_overflow{};
    std::optional<EntryKind> common_kind{};
    bool mixed_kind{};
    std::optional<ObservedFileTime> common_modified{};
    bool mixed_modified{};
    bool modified_unavailable{};
};

[[nodiscard]] inline SelectionSummary summarize_selection(
    const std::unordered_map<std::string, DirectoryEntry>& entries,
    const std::span<const std::string> selected) {
    SelectionSummary summary{};
    summary.selected = selected.size();
    for (const std::string& id : selected) {
        const std::unordered_map<std::string, DirectoryEntry>::const_iterator found = entries.find(id);
        if (found == entries.end() || !(*found).second.identity.available()) {
            ++summary.unavailable_entries;
            summary.modified_unavailable = true;
            continue;
        }
        const DirectoryEntry& entry = (*found).second;
        if (entry.identity.type == std::filesystem::file_type::none ||
            entry.identity.type == std::filesystem::file_type::unknown) {
            ++summary.unavailable_entries;
            summary.modified_unavailable = true;
            continue;
        }
        if (!summary.common_kind) summary.common_kind = entry.kind;
        else if (*summary.common_kind != entry.kind) summary.mixed_kind = true;

        if (!entry.metadata.modified) {
            summary.modified_unavailable = true;
        } else if (!summary.common_modified) {
            summary.common_modified = entry.metadata.modified;
        } else if ((*summary.common_modified).unix_seconds != (*entry.metadata.modified).unix_seconds ||
                   (*summary.common_modified).nanoseconds != (*entry.metadata.modified).nanoseconds) {
            summary.mixed_modified = true;
        }

        if (entry.identity.type == std::filesystem::file_type::directory) {
            ++summary.folders;
        } else if (entry.identity.type == std::filesystem::file_type::regular) {
            ++summary.files;
            if (!entry.metadata.logical_size) continue;
            ++summary.known_sizes;
            const std::uint64_t bytes = *entry.metadata.logical_size;
            if (!summary.size_overflow) {
                if (bytes > std::numeric_limits<std::uint64_t>::max() - summary.logical_bytes) {
                    summary.size_overflow = true;
                } else {
                    summary.logical_bytes += bytes;
                }
            }
        } else {
            ++summary.other;
        }
    }
    return summary;
}

[[nodiscard]] inline std::string selection_size_text(const SelectionSummary& summary) {
    std::string text{};
    if (summary.size_overflow) {
        text = "File size total exceeds display range";
    } else if (summary.files == 0U) {
        if (summary.unavailable_entries > 0U) text = "Sizes unavailable";
        else if (summary.folders == 0U) text = "Not applicable";
    } else if (summary.known_sizes == 0U) {
        text = "File sizes unavailable";
    } else {
        text = format_bytes(summary.logical_bytes);
        text += summary.known_sizes == summary.files && summary.unavailable_entries == 0U
            ? " in files" : " in known files";
        if (summary.known_sizes != summary.files || summary.unavailable_entries > 0U) {
            text += " · some sizes unavailable";
        }
    }
    if (summary.folders > 0U) {
        if (!text.empty()) text += " · ";
        text += "Folder contents not counted";
    }
    return text;
}

[[nodiscard]] inline std::string selection_count_text(const SelectionSummary& summary) {
    std::string text{};
    if (summary.files > 0U) {
        text = std::to_string(summary.files);
        text += summary.files == 1U ? " file" : " files";
    }
    if (summary.folders > 0U) {
        if (!text.empty()) text += " · ";
        text += std::to_string(summary.folders);
        text += summary.folders == 1U ? " folder" : " folders";
    }
    if (summary.other > 0U) {
        if (!text.empty()) text += " · ";
        text += std::to_string(summary.other);
        text += summary.other == 1U ? " other object" : " other objects";
    }
    if (summary.unavailable_entries > 0U) {
        if (!text.empty()) text += " · ";
        text += std::to_string(summary.unavailable_entries);
        text += " unavailable";
    }
    return text;
}

} // namespace file_manager::detail
