#include "search_preparation.hpp"
#include "native_observation.hpp"
#include "file_manager/platform_paths.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace file_manager {
namespace {

char lower_extension_byte(const char value) {
    const unsigned char input = static_cast<unsigned char>(value);
    const int lowered = std::tolower(input);
    const char result = static_cast<char>(lowered);
    return result;
}

EntryKind observed_entry_kind(const std::filesystem::file_type type,
                             const std::filesystem::path& path) {
    if (type == std::filesystem::file_type::directory) return EntryKind::folder;
    if (type == std::filesystem::file_type::symlink) return EntryKind::symlink;
    if (type != std::filesystem::file_type::regular) return EntryKind::other;
    std::string extension = path_utf8(path.extension());
    std::transform(extension.begin(), extension.end(), extension.begin(), lower_extension_byte);
    if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" ||
        extension == ".gif" || extension == ".webp") return EntryKind::image;
    if (extension == ".zip" || extension == ".tar" || extension == ".gz" ||
        extension == ".7z") return EntryKind::archive;
    if (extension == ".mp3" || extension == ".wav" || extension == ".flac") return EntryKind::audio;
    if (extension == ".cpp" || extension == ".hpp" || extension == ".c" ||
        extension == ".h" || extension == ".go" || extension == ".rs" ||
        extension == ".py" || extension == ".html" || extension == ".css") return EntryKind::code;
    return EntryKind::document;
}

std::string search_stable_id(const std::filesystem::path& path, const ObjectIdentity& identity) {
    constexpr std::uint64_t offset{14695981039346656037ULL};
    constexpr std::uint64_t prime{1099511628211ULL};
    std::uint64_t hash{offset};
    const std::string address = path_generic_utf8(path);
    for (const unsigned char value : address) {
        hash ^= value;
        hash *= prime;
    }
    std::ostringstream stream{};
    stream << "engine-result-" << std::hex << identity.device << '-'
           << identity.inode << '-' << hash;
    const std::string result = stream.str();
    return result;
}

DirectoryEntry observed_search_entry(const std::filesystem::path& path,
                                     const NativeObjectObservation& observed) {
    DirectoryEntry entry{};
    entry.stable_id = search_stable_id(path, observed.identity);
    entry.path = path;
    entry.name = path_utf8(path.filename());
    entry.identity = observed.identity;
    entry.metadata = observed.facts;
    entry.kind = observed_entry_kind(entry.identity.type, path);
    entry.directory = entry.kind == EntryKind::folder;
    if (entry.directory) entry.secondary_text = "Folder";
    else if (entry.metadata.logical_size) entry.secondary_text = format_bytes(*entry.metadata.logical_size);
    else if (entry.kind == EntryKind::symlink) entry.secondary_text = "Symbolic link · not followed";
    else entry.secondary_text = "Unavailable";
    entry.modified_text = entry.metadata.modified ? format_modified_time(*entry.metadata.modified) : "Unavailable";
    return entry;
}

} // namespace

void SearchCoverageSummary::observe(const fileman::orchestrator::SearchPageInfo& page,
                                    const std::size_t rejected, const bool append) {
    if (!append) *this = SearchCoverageSummary{};
    const fileman::orchestrator::SearchCoverageInfo& coverage = page.coverage;
    if (coverage.stale_roots && !(*coverage.stale_roots).empty()) stale = true;
    if (coverage.unavailable_roots && !(*coverage.unavailable_roots).empty()) unavailable = true;
    if (coverage.unavailable_paths && !(*coverage.unavailable_paths).empty()) unavailable = true;
    if (coverage.warnings && !(*coverage.warnings).empty()) warnings = true;
    if (!coverage.warnings) unreported = true;
    if (page.source == "catalogue") {
        if (!coverage.stale_roots || !coverage.unavailable_roots) unreported = true;
    } else if (page.source == "live_filesystem") {
        if (!coverage.unavailable_paths) unreported = true;
    } else {
        unreported = true;
    }
    if (rejected != 0U) omitted = true;
}

std::string SearchCoverageSummary::describe(const fileman::orchestrator::SearchPageInfo& page) const {
    std::string description{};
    if (page.source == "catalogue") description += " · indexed matches may have changed";
    if (page.terminal == "partial") description += " · partial search results";
    if (!page.complete) {
        if (page.cursor) description += " · more results available";
        else description += " · search incomplete; no more results available";
    }
    if (stale) description += " · index needs refresh";
    if (unavailable) description += " · locations unavailable";
    if (warnings) description += " · search reported warnings";
    if (unreported) description += " · search coverage unavailable";
    if (omitted) description += " · some returned paths could not be displayed";
    return description;
}

std::string SearchCoverageSummary::result_status(const std::size_t count, const bool criteria,
    const fileman::orchestrator::SearchPageInfo& page) const {
    std::string status{};
    if (count == 0U) {
        status = criteria ? "No criteria matches shown" : "No matches shown";
    } else {
        status = std::to_string(count);
        if (criteria) status += count == 1U ? " criteria match shown" : " criteria matches shown";
        else status += count == 1U ? " match shown" : " matches shown";
    }
    // Keep the most consequential coverage state in the primary status field.
    // The secondary description retains all states, including earlier pages.
    if (!page.complete || page.terminal == "partial") status += " · partial search";
    else if (stale) status += " · index needs refresh";
    else if (unavailable) status += " · some locations unavailable";
    else if (warnings) status += " · search warnings";
    else if (unreported) status += " · coverage unavailable";
    else if (omitted) status += " · some results omitted";
    else if (page.source == "catalogue") status += " · indexed results";
    return status;
}

PreparedSearchPage prepare_search_page(const std::filesystem::path& canonical_root,
    fileman::orchestrator::SearchPageInfo page, const CancellationCheck& cancelled) {
    PreparedSearchPage cancelled_result{};
    cancelled_result.cancelled = true;
    if (cancelled && cancelled()) return cancelled_result;
    constexpr std::size_t maximum_results{500U};
    if (page.results.size() > maximum_results) {
        throw std::length_error("search provider exceeded the frontend's 500-result page limit");
    }
    PreparedSearchPage prepared{};
    prepared.entries.reserve(page.results.size());
    for (const fileman::orchestrator::SearchResultInfo& result : page.results) {
        if (cancelled && cancelled()) return cancelled_result;
        std::filesystem::path path = result.path.lexically_normal();
        if (!path_is_within(canonical_root, path)) {
            const std::optional<std::filesystem::path> rebased =
                rebase_path_from_equivalent_root(canonical_root, path);
            if (rebased) path = *rebased;
        }
        if (result.unavailable || !path_is_within(canonical_root, path) ||
            path_route_has_symlink(canonical_root, path.parent_path())) {
            ++prepared.rejected;
            continue;
        }
        if (cancelled && cancelled()) return cancelled_result;
        const NativeObjectObservation observed = observe_native_object(path);
        if (cancelled && cancelled()) return cancelled_result;
        if (observed.error || !observed.identity.available()) {
            ++prepared.rejected;
            continue;
        }
        DirectoryEntry entry = observed_search_entry(path, observed);
        prepared.entries.push_back(std::move(entry));
    }
    if (cancelled && cancelled()) return cancelled_result;
    // Raw addresses are no longer needed by the consumer; retain only provenance
    // and cursor metadata alongside the independently owned observations.
    std::vector<fileman::orchestrator::SearchResultInfo> retired_results{};
    retired_results.swap(page.results);
    std::vector<std::string> retired_names{};
    retired_names.swap(page.names);
    prepared.page = std::move(page);
    return prepared;
}

} // namespace file_manager
