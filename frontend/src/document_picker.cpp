#include "file_manager/platform_paths.hpp"
#include "file_manager/document_picker.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <unordered_set>

namespace file_manager {
namespace {

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](const unsigned char character) {
                       return static_cast<char>(std::tolower(character));
                   });
    return value;
}

std::string normalize_extension(std::string extension) {
    if (!extension.empty() && extension.front() != '.') {
        extension.insert(extension.begin(), '.');
    }
    return lower(std::move(extension));
}


} // namespace

FileSelectionController::FileSelectionController(DocumentPickerRequest request)
    : request_(std::move(request)),
      filename_(request_.suggested_name),
      active_filter_id_(request_.active_filter_id),
      show_hidden_(request_.show_hidden),
      orchestrator_session_valid_(request_.orchestrator_session_valid ||
          request_.authority == DocumentPickerAuthority::trusted_local_host) {
    if (request_.authority == DocumentPickerAuthority::unavailable && request_.orchestrator_session_valid)
        request_.authority = DocumentPickerAuthority::orchestrator_session;
    if (request_.owner_application_id.empty()) {
        throw std::invalid_argument("picker owner application ID is empty");
    }
    if (request_.protected_root.empty()) throw std::invalid_argument("picker requires an explicit root");
    const auto supplied_root = std::filesystem::absolute(
        request_.protected_root).lexically_normal();
    auto supplied_initial = request_.initial_location.empty()
        ? supplied_root
        : std::filesystem::absolute(request_.initial_location).lexically_normal();
    request_.protected_root = canonical_existing_directory(supplied_root);
    if (path_is_within(supplied_root, supplied_initial)) {
        supplied_initial = request_.protected_root /
            supplied_initial.lexically_relative(supplied_root);
    }
    request_.initial_location = std::move(supplied_initial);
    if (request_.admitted_roots.size() > 32) throw std::invalid_argument("picker root bound is 32");
    for (auto& root : request_.admitted_roots) root = canonical_existing_directory(root);
    if (std::find(request_.admitted_roots.begin(), request_.admitted_roots.end(), request_.protected_root) == request_.admitted_roots.end()) {
        request_.admitted_roots.push_back(request_.protected_root);
    }
    if (request_.home_location.empty()) request_.home_location = user_home_directory();
    if (request_.maximum_selection == 0U ||
        request_.maximum_selection > 32U) {
        throw std::invalid_argument("picker selection bound must be 1..32");
    }
    if (!profile_accepts_multiple() && request_.maximum_selection != 1U) {
        throw std::invalid_argument(
            "single-selection picker profile requires a bound of one");
    }
    std::unordered_set<std::string> filter_ids;
    for (auto& filter : request_.filters) {
        if (filter.id.empty() || filter.label.empty() ||
            !filter_ids.insert(filter.id).second) {
            throw std::invalid_argument("picker filters require unique identities");
        }
        for (auto& extension : filter.extensions) {
            extension = normalize_extension(std::move(extension));
        }
    }
    if (active_filter_id_.empty() && !request_.filters.empty()) {
        active_filter_id_ = request_.filters.front().id;
    }
    if (!active_filter_id_.empty() &&
        std::none_of(request_.filters.begin(), request_.filters.end(),
                     [this](const DocumentTypeFilter& filter) {
                         return filter.id == active_filter_id_;
                     })) {
        throw std::invalid_argument("picker active filter is unknown");
    }
    if (!refresh()) {
        throw std::runtime_error(last_error_);
    }
}

const DocumentPickerRequest& FileSelectionController::request() const noexcept {
    return request_;
}

const DirectorySnapshot& FileSelectionController::browser() const noexcept {
    return browser_;
}

const std::vector<std::string>&
FileSelectionController::selected_ids() const noexcept {
    return selected_ids_;
}

const std::string& FileSelectionController::filename() const noexcept {
    return filename_;
}

const std::string& FileSelectionController::active_filter_id() const noexcept {
    return active_filter_id_;
}

const std::string& FileSelectionController::last_error() const noexcept {
    return last_error_;
}

bool FileSelectionController::session_valid() const noexcept {
    return orchestrator_session_valid_;
}

bool FileSelectionController::show_hidden() const noexcept {
    return show_hidden_;
}

bool FileSelectionController::profile_accepts_multiple() const noexcept {
    return request_.profile == DocumentPickerProfile::open_files ||
        request_.profile == DocumentPickerProfile::import_files;
}

bool FileSelectionController::profile_saves() const noexcept {
    return request_.profile == DocumentPickerProfile::save_as ||
        request_.profile == DocumentPickerProfile::export_file;
}

bool FileSelectionController::entry_visible(const DirectoryEntry& entry) const {
    if (entry.directory) return true;
    if (request_.profile == DocumentPickerProfile::select_folder) return false;
    if (!name_filter_.empty()) {
        // Bounded ASCII-insensitive glob, '*' and '?' only; directories stay navigable.
        const auto text = lower(entry.name);
        const auto pattern = lower(name_filter_);
        std::size_t t = 0, p = 0, star = std::string::npos, retry = 0;
        while (t < text.size()) {
            if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == text[t])) { ++p; ++t; }
            else if (p < pattern.size() && pattern[p] == '*') { star = p++; retry = t; }
            else if (star != std::string::npos) { p = star + 1; t = ++retry; }
            else return false;
        }
        while (p < pattern.size() && pattern[p] == '*') ++p;
        if (p != pattern.size()) return false;
    }
    if (active_filter_id_.empty()) return true;
    const auto filter = std::find_if(
        request_.filters.begin(), request_.filters.end(),
        [this](const DocumentTypeFilter& candidate) {
            return candidate.id == active_filter_id_;
        });
    if (filter == request_.filters.end() || filter->extensions.empty()) return true;
    const auto extension = normalize_extension(path_utf8(entry.path.extension()));
    return std::find(filter->extensions.begin(), filter->extensions.end(),
                     extension) != filter->extensions.end();
}

bool FileSelectionController::navigate(
    const std::filesystem::path& location) {
    const auto target = resolve_navigation_target(request_.admitted_roots,
        browser_.location, request_.home_location, location);
    if (!target) { last_error_ = "Location is outside the admitted roots"; return false; }
    const auto previous = request_.initial_location;
    const auto previous_root = request_.protected_root;
    request_.initial_location = target->path;
    request_.protected_root = target->root;
    if (!refresh()) {
        request_.initial_location = previous;
        request_.protected_root = previous_root;
        return false;
    }
    return true;
}

bool FileSelectionController::refresh() {
    auto snapshot = read_directory(
        request_.protected_root, request_.initial_location, {}, ++generation_,
        {}, show_hidden_);
    if (!snapshot.available()) {
        last_error_ = snapshot.cancelled
            ? "directory refresh was cancelled"
            : snapshot.error;
        return false;
    }
    last_error_.clear();
    request_.initial_location = snapshot.location;
    snapshot.entries.erase(
        std::remove_if(snapshot.entries.begin(), snapshot.entries.end(),
                       [this](const DirectoryEntry& entry) {
                           return !entry_visible(entry);
                       }),
        snapshot.entries.end());
    location_identity_ = observe_identity(snapshot.location);
    browser_ = std::move(snapshot);
    selected_ids_.erase(
        std::remove_if(selected_ids_.begin(), selected_ids_.end(),
                       [this](const std::string& id) {
                           return find_entry(id) == nullptr;
                       }),
        selected_ids_.end());
    return true;
}

const DirectoryEntry* FileSelectionController::find_entry(
    const std::string_view stable_id) const noexcept {
    const auto found = std::find_if(
        browser_.entries.begin(), browser_.entries.end(),
        [stable_id](const DirectoryEntry& entry) {
            return entry.stable_id == stable_id;
        });
    return found == browser_.entries.end() ? nullptr : &*found;
}

bool FileSelectionController::set_selection(
    std::vector<std::string> stable_ids) {
    if (stable_ids.size() > request_.maximum_selection ||
        (!profile_accepts_multiple() && stable_ids.size() > 1U)) {
        return false;
    }
    std::unordered_set<std::string> unique;
    for (const auto& id : stable_ids) {
        const auto* entry = find_entry(id);
        if (!entry || !unique.insert(id).second ||
            entry->kind == EntryKind::symlink) {
            return false;
        }
        if (request_.profile == DocumentPickerProfile::select_folder) {
            if (!entry->directory) return false;
        }
    }
    selected_ids_ = std::move(stable_ids);
    if (profile_saves() && selected_ids_.size() == 1U) {
        const auto* entry = find_entry(selected_ids_.front());
        if (entry && !entry->directory) filename_ = entry->name;
    }
    return true;
}

bool FileSelectionController::set_filename(std::string filename) {
    if (!profile_saves() || filename.size() > 255U) return false;
    filename_ = std::move(filename);
    return true;
}

bool FileSelectionController::set_active_filter(std::string filter_id) {
    if (!filter_id.empty() &&
        std::none_of(request_.filters.begin(), request_.filters.end(),
                     [&filter_id](const DocumentTypeFilter& filter) {
                         return filter.id == filter_id;
                     })) {
        return false;
    }
    auto previous = active_filter_id_;
    active_filter_id_ = std::move(filter_id);
    if (refresh()) return true;
    active_filter_id_ = std::move(previous);
    return false;
}

const std::string& FileSelectionController::name_filter() const noexcept { return name_filter_; }

bool FileSelectionController::set_name_filter(std::string pattern) {
    if (pattern.size() > 255) return false;
    const auto previous = name_filter_;
    name_filter_ = std::move(pattern);
    if (refresh()) return true;
    name_filter_ = previous;
    return false;
}

bool FileSelectionController::set_show_hidden(const bool show_hidden) {
    if (!request_.allow_hidden_toggle && show_hidden != show_hidden_) return false;
    const auto previous = show_hidden_;
    show_hidden_ = show_hidden;
    if (refresh()) return true;
    show_hidden_ = previous;
    return false;
}

void FileSelectionController::set_orchestrator_session_valid(
    const bool valid) noexcept {
    if (request_.authority != DocumentPickerAuthority::trusted_local_host) {
        request_.authority = DocumentPickerAuthority::orchestrator_session;
        orchestrator_session_valid_ = valid;
    }
}

void FileSelectionController::set_authority_valid(const bool valid) noexcept {
    orchestrator_session_valid_ = valid && request_.authority != DocumentPickerAuthority::unavailable;
}

DocumentPickerResult FileSelectionController::selection_error(
    std::string code, std::string message) const {
    return {DocumentPickerTerminal::validation_error, std::move(code),
            std::move(message), {}, request_.allow_native_fallback};
}

DocumentPickerResult FileSelectionController::accept(
    const bool overwrite_confirmed) const {
    if (!orchestrator_session_valid_) {
        return {DocumentPickerTerminal::unavailable, "session-unavailable",
                "Selection authority must be revalidated before acceptance",
                {}, request_.allow_native_fallback};
    }
    if (!browser_.available()) {
        return {DocumentPickerTerminal::unavailable, "location-unavailable",
                browser_.error, {}, request_.allow_native_fallback};
    }
    const auto current_location = observe_identity(browser_.location);
    if (!current_location.available() || current_location != location_identity_ ||
        path_route_has_symlink(request_.protected_root, browser_.location)) {
        return selection_error("location-changed", "Current folder changed; refresh before selecting");
    }
    if (profile_saves()) {
        if (!valid_platform_basename(filename_)) {
            return selection_error("invalid-filename",
                                   "save filename must be one valid basename");
        }
        if (!show_hidden_ && !filename_.empty() && filename_.front() == '.') {
            return selection_error("hidden-destination", "Enable Show hidden to select a hidden destination");
        }
        auto name = filename_;
        auto extension = normalize_extension(request_.default_extension);
        const auto filter = std::find_if(request_.filters.begin(), request_.filters.end(),
            [this](const DocumentTypeFilter& value) { return value.id == active_filter_id_; });
        if (filter != request_.filters.end() && !filter->extensions.empty() &&
            std::find(filter->extensions.begin(), filter->extensions.end(), extension) == filter->extensions.end()) {
            extension = filter->extensions.front();
        }
        if (!extension.empty() &&
            lower(path_utf8(path_from_utf8(name).extension())) != extension) {
            name += extension;
        }
        if (!valid_platform_basename(name)) {
            return selection_error("invalid-filename", "Final filename including extension is invalid");
        }
        const auto destination = (browser_.location / path_from_utf8(name)).lexically_normal();
        if (!path_is_within(request_.protected_root, destination) ||
            path_route_has_symlink(request_.protected_root, destination)) {
            return selection_error("destination-refused",
                                   "save destination failed protected-root policy");
        }
        std::error_code destination_error;
        const auto destination_status = std::filesystem::symlink_status(destination, destination_error);
        if (destination_error && destination_error != std::errc::no_such_file_or_directory) {
            return selection_error("destination-unavailable", destination_error.message());
        }
        const auto identity = observe_identity(destination);
        if (destination_status.type() != std::filesystem::file_type::not_found && !identity.available()) {
            return selection_error("destination-unavailable", "Destination identity is unavailable");
        }
        if (overwrite_confirmed && (!pending_overwrite_ || pending_overwrite_->path != destination ||
            !pending_overwrite_->identity.same_revision(identity))) {
            pending_overwrite_.reset();
            return selection_error("destination-changed", "Destination changed; request overwrite confirmation again");
        }
        if (identity.available()) {
            if (identity.type != std::filesystem::file_type::regular) {
                return selection_error("destination-not-file",
                                       "save destination is not a regular file");
            }
            if (!overwrite_confirmed) {
                pending_overwrite_ = DocumentSelectionObservation{destination, identity, true};
                return {DocumentPickerTerminal::overwrite_confirmation_required,
                        "overwrite-confirmation-required",
                        "existing destination requires an explicit overwrite decision",
                        {{destination, identity, true}},
                        request_.allow_native_fallback};
            }
        }
        return {DocumentPickerTerminal::accepted, "accepted",
                "picker returned a destination observation; the host still owns the write",
                {{destination, identity, identity.available()}},
                request_.allow_native_fallback};
    }

    if (request_.profile == DocumentPickerProfile::select_folder &&
        selected_ids_.empty()) {
        const auto identity = observe_identity(browser_.location);
        if (!identity.available() ||
            identity.type != std::filesystem::file_type::directory) {
            return selection_error("folder-unavailable",
                                   "current folder is no longer available");
        }
        return {DocumentPickerTerminal::accepted, "accepted",
                "current folder accepted", {{browser_.location, identity, true}},
                request_.allow_native_fallback};
    }
    if (selected_ids_.empty()) {
        return selection_error("selection-required", "select an admitted object");
    }
    std::vector<DocumentSelectionObservation> observations;
    observations.reserve(selected_ids_.size());
    for (const auto& id : selected_ids_) {
        const auto* entry = find_entry(id);
        if (!entry) {
            return selection_error("selection-stale",
                                   "selected object is no longer in the browser snapshot");
        }
        const auto current = observe_identity(entry->path);
        if (!current.available() || !entry->identity.same_revision(current) ||
            path_route_has_symlink(request_.protected_root, entry->path)) {
            return selection_error("selection-changed",
                                   "selected object changed before acceptance");
        }
        const bool want_folder =
            request_.profile == DocumentPickerProfile::select_folder;
        if (want_folder != entry->directory) {
            return selection_error("selection-kind",
                                   "selected object kind is not accepted by this profile");
        }
        observations.push_back({entry->path, current, true});
    }
    return {DocumentPickerTerminal::accepted, "accepted",
            "picker selections revalidated", std::move(observations),
            request_.allow_native_fallback};
}

DocumentPickerResult FileSelectionController::cancel() const {
    return {DocumentPickerTerminal::cancelled, "cancelled",
            "picker cancelled without a selection", {},
            request_.allow_native_fallback};
}

} // namespace file_manager
