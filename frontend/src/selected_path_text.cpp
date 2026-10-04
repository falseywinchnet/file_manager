#include "selected_path_text.hpp"

#include <algorithm>
#include <limits>
#include <utility>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace file_manager::detail {
namespace {

struct PathExtent final {
    PathTextStatus status{PathTextStatus::ready};
    std::size_t bytes{};
};

[[nodiscard]] PathExtent path_extent(const std::filesystem::path& path) {
    PathExtent extent{};
#if defined(_WIN32)
    const std::wstring& native = path.native();
    if (native.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        extent.status = PathTextStatus::too_large;
        return extent;
    }
    const int units = static_cast<int>(native.size());
    const int bytes = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
        native.data(), units, nullptr, 0, nullptr, nullptr);
    if (bytes == 0) extent.status = PathTextStatus::invalid_encoding;
    else extent.bytes = static_cast<std::size_t>(bytes);
#else
    extent.bytes = path.native().size();
#endif
    return extent;
}

// Observes map storage only during this synchronous preparation. No callback,
// mutation, or owner transfer can invalidate a returned path before its use.
[[nodiscard]] const std::filesystem::path* selected_path(
    const std::unordered_map<std::string, DirectoryEntry>& entries,
    const std::span<const std::string> selected,
    const std::filesystem::path& location, const std::size_t index) {
    if (selected.empty()) return &location;
    const std::unordered_map<std::string, DirectoryEntry>::const_iterator found = entries.find(selected[index]);
    if (found == entries.end()) return nullptr;
    return &(*found).second.path;
}

} // namespace

PathText selected_path_text(const std::unordered_map<std::string, DirectoryEntry>& entries,
    const std::span<const std::string> selected,
    const std::filesystem::path& location, const std::size_t maximum_bytes) {
    PathText result{};
    const std::size_t count = selected.empty() ? 1U : selected.size();
    std::size_t total = count - 1U;
    if (total > maximum_bytes) {
        result.status = PathTextStatus::too_large;
        return result;
    }
    for (std::size_t index = 0U; index < count; ++index) {
        const std::filesystem::path* const path = selected_path(entries, selected, location, index);
        if (path == nullptr || (*path).empty()) return result;
        const PathExtent extent = path_extent(*path);
        if (extent.status != PathTextStatus::ready) {
            result.status = extent.status;
            return result;
        }
        if (extent.bytes > maximum_bytes - total) {
            result.status = PathTextStatus::too_large;
            return result;
        }
        total += extent.bytes;
    }
    // One owned destination, established before conversion. No per-path string
    // allocation or buffer growth occurs in the publication kernel.
    std::string text(total, '\0');
    std::size_t offset{};
    for (std::size_t index = 0U; index < count; ++index) {
        const std::filesystem::path& path = *selected_path(entries, selected, location, index);
        if (index > 0U) {
            text[offset] = '\n';
            ++offset;
        }
#if defined(_WIN32)
        const std::wstring& native = path.native();
        const int units = static_cast<int>(native.size()); // Validated in the sizing pass.
        const std::size_t available = std::min(total - offset,
            static_cast<std::size_t>(std::numeric_limits<int>::max()));
        const int capacity = static_cast<int>(available);
        // Both source and destination borrows end with this foreign call.
        const int written = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
            native.data(), units, text.data() + offset, capacity, nullptr, nullptr);
        if (written == 0) {
            result.status = PathTextStatus::invalid_encoding;
            return result;
        }
        offset += static_cast<std::size_t>(written);
#else
        const std::string& native = path.native();
        std::copy_n(native.data(), native.size(), text.data() + offset);
        offset += native.size();
#endif
    }
    result.text = std::move(text);
    result.status = PathTextStatus::ready;
    return result;
}

} // namespace file_manager::detail
