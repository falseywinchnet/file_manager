#include "file_manager/platform_paths.hpp"
#include "file_manager/document_picker.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <unordered_set>

namespace file_manager {
namespace {

char lower_character(const char value) {
    const unsigned char byte = static_cast<unsigned char>(value);
    const int lowered = std::tolower(byte);
    const char result = static_cast<char>(lowered);
    return result;
}

std::string lower(std::string value) {
    for (std::size_t index = 0; index < value.size(); ++index) {
        value[index] = lower_character(value[index]);
    }
    return value;
}

std::string normalize_extension(std::string extension) {
    if (!extension.empty() && extension.front() != '.') {
        extension.insert(extension.begin(), '.');
    }
    extension = lower(std::move(extension));
    return extension;
}

// Borrows the request's filter storage until that request is modified.
const DocumentTypeFilter* find_filter(const std::vector<DocumentTypeFilter>& filters,
                                      const std::string_view id) {
    for (const DocumentTypeFilter& filter : filters) {
        if (filter.id == id)
            return &filter;
    }
    return nullptr;
}

// Resolves navigation only. The caller still uses the contained no-link reader
// and returns observations of the canonical target, never of the alias.
std::optional<NavigationTarget> trusted_directory_target(
    const DocumentPickerRequest& request, const std::filesystem::path& location) {
    std::error_code error{};
    const std::filesystem::path canonical = std::filesystem::canonical(location, error);
    if (error) return {};
    const bool directory = std::filesystem::is_directory(canonical, error);
    if (error || !directory) return {};
    const std::optional<NavigationTarget> target = resolve_navigation_target(
        request.admitted_roots, canonical, request.home_location, canonical);
    return target;
}

// Byte-oriented glob; case conversion preserves the existing C-locale behavior.
// No allocation, retained input, or writes to either borrowed string.
bool name_matches(const std::string_view text, const std::string_view pattern) {
    std::size_t text_index = 0;
    std::size_t pattern_index = 0;
    std::size_t star_index = std::string_view::npos;
    std::size_t retry_index = 0;
    while (text_index < text.size()) {
        if (pattern_index < pattern.size() &&
            (pattern[pattern_index] == '?' ||
             lower_character(pattern[pattern_index]) == lower_character(text[text_index]))) {
            ++pattern_index;
            ++text_index;
        } else if (pattern_index < pattern.size() && pattern[pattern_index] == '*') {
            star_index = pattern_index;
            ++pattern_index;
            retry_index = text_index;
        } else if (star_index != std::string_view::npos) {
            pattern_index = star_index + 1;
            ++retry_index;
            text_index = retry_index;
        } else {
            return false;
        }
    }
    while (pattern_index < pattern.size() && pattern[pattern_index] == '*') {
        ++pattern_index;
    }
    const bool matched = pattern_index == pattern.size();
    return matched;
}

bool entry_matches(const DirectoryEntry& entry, const DocumentPickerProfile profile,
                   const std::string_view name_pattern, const DocumentTypeFilter* filter) {
    if (entry.directory)
        return true;
    if (profile == DocumentPickerProfile::select_folder)
        return false;
    if (!name_pattern.empty() && !name_matches(entry.name, name_pattern))
        return false;
    if (filter == nullptr || (*filter).extensions.empty())
        return true;
    // DirectoryEntry.name is the filesystem model's UTF-8 basename. Its final
    // dot has std::filesystem extension semantics: a leading dot alone is not
    // an extension, while a trailing dot is the one-byte extension ".".
    const std::string_view name = entry.name;
    const std::size_t dot = name.rfind('.');
    std::string_view suffix{};
    if (dot != std::string_view::npos && dot != 0U)
        suffix = name.substr(dot);
    for (const std::string& extension : (*filter).extensions) {
        if (suffix.size() != extension.size())
            continue;
        bool equal = true;
        for (std::size_t index = 0; index < suffix.size(); ++index) {
            if (lower_character(suffix[index]) != extension[index]) {
                equal = false;
                break;
            }
        }
        if (equal)
            return true;
    }
    return false;
}

// Semantic factories keep terminal status and result payloads consistent.
DocumentPickerResult accepted_selection(std::string message,
                                        std::vector<DocumentSelectionObservation> observations,
                                        const bool native_fallback_permitted) {
    DocumentPickerResult result{
        .terminal = DocumentPickerTerminal::accepted,
        .code = "accepted",
        .message = std::move(message),
        .selections = std::move(observations),
        .native_fallback_permitted = native_fallback_permitted,
    };
    return result;
}

DocumentPickerResult unavailable_selection(std::string code, std::string message,
                                           const bool native_fallback_permitted) {
    DocumentPickerResult result{
        .terminal = DocumentPickerTerminal::unavailable,
        .code = std::move(code),
        .message = std::move(message),
        .selections = {},
        .native_fallback_permitted = native_fallback_permitted,
    };
    return result;
}

DocumentPickerResult overwrite_confirmation(const DocumentSelectionObservation& observation,
                                            const bool native_fallback_permitted) {
    DocumentPickerResult result{
        .terminal = DocumentPickerTerminal::overwrite_confirmation_required,
        .code = "overwrite-confirmation-required",
        .message = "existing destination requires an explicit overwrite decision",
        .selections = {observation},
        .native_fallback_permitted = native_fallback_permitted,
    };
    return result;
}

} // namespace

FileSelectionController::FileSelectionController(DocumentPickerRequest request)
    : request_(std::move(request)), filename_(request_.suggested_name),
      active_filter_id_(request_.active_filter_id), show_hidden_(request_.show_hidden),
      orchestrator_session_valid_(request_.orchestrator_session_valid ||
                                  request_.authority ==
                                      DocumentPickerAuthority::trusted_local_host) {
    if (request_.authority == DocumentPickerAuthority::unavailable &&
        request_.orchestrator_session_valid)
        request_.authority = DocumentPickerAuthority::orchestrator_session;
    if (request_.owner_application_id.empty()) {
        throw std::invalid_argument("picker owner application ID is empty");
    }
    if (request_.protected_root.empty())
        throw std::invalid_argument("picker requires an explicit root");
    const std::filesystem::path absolute_root = std::filesystem::absolute(request_.protected_root);
    const std::filesystem::path supplied_root = absolute_root.lexically_normal();
    std::filesystem::path supplied_initial = supplied_root;
    if (!request_.initial_location.empty()) {
        const std::filesystem::path absolute_initial =
            std::filesystem::absolute(request_.initial_location);
        supplied_initial = absolute_initial.lexically_normal();
    }
    request_.protected_root = canonical_existing_directory(supplied_root);
    if (path_is_within(supplied_root, supplied_initial)) {
        supplied_initial =
            request_.protected_root / supplied_initial.lexically_relative(supplied_root);
    }
    request_.initial_location = std::move(supplied_initial);
    if (request_.admitted_roots.size() > 32)
        throw std::invalid_argument("picker root bound is 32");
    for (std::filesystem::path& root : request_.admitted_roots)
        root = canonical_existing_directory(root);
    if (std::find(request_.admitted_roots.begin(), request_.admitted_roots.end(),
                  request_.protected_root) == request_.admitted_roots.end()) {
        request_.admitted_roots.push_back(request_.protected_root);
    }
    if (request_.home_location.empty())
        request_.home_location = user_home_directory();
    if (request_.maximum_selection == 0U || request_.maximum_selection > 32U) {
        throw std::invalid_argument("picker selection bound must be 1..32");
    }
    if (!profile_accepts_multiple() && request_.maximum_selection != 1U) {
        throw std::invalid_argument("single-selection picker profile requires a bound of one");
    }
    std::unordered_set<std::string> filter_ids{};
    for (DocumentTypeFilter& filter : request_.filters) {
        if (filter.id.empty() || filter.label.empty()) {
            throw std::invalid_argument("picker filters require unique identities");
        }
        const std::pair<std::unordered_set<std::string>::iterator, bool> insertion =
            filter_ids.insert(filter.id);
        if (!insertion.second) {
            throw std::invalid_argument("picker filters require unique identities");
        }
        for (std::string& extension : filter.extensions) {
            extension = normalize_extension(std::move(extension));
        }
    }
    if (active_filter_id_.empty() && !request_.filters.empty()) {
        active_filter_id_ = request_.filters.front().id;
    }
    const DocumentTypeFilter* active_filter = find_filter(request_.filters, active_filter_id_);
    if (!active_filter_id_.empty() && active_filter == nullptr) {
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

const std::vector<std::string>& FileSelectionController::selected_ids() const noexcept {
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
    const bool multiple = request_.profile == DocumentPickerProfile::open_files ||
                          request_.profile == DocumentPickerProfile::import_files;
    return multiple;
}

bool FileSelectionController::profile_saves() const noexcept {
    const bool saves = request_.profile == DocumentPickerProfile::save_as ||
                       request_.profile == DocumentPickerProfile::export_file;
    return saves;
}

bool FileSelectionController::navigate(const std::filesystem::path& location) {
    const std::optional<NavigationTarget> target = resolve_navigation_target(
        request_.admitted_roots, browser_.location, request_.home_location, location);
    if (!target) {
        last_error_ = "Location is outside the admitted roots";
        return false;
    }
    const std::filesystem::path previous = request_.initial_location;
    const std::filesystem::path previous_root = request_.protected_root;
    request_.initial_location = (*target).path;
    request_.protected_root = (*target).root;
    if (!refresh()) {
        request_.initial_location = previous;
        request_.protected_root = previous_root;
        return false;
    }
    return true;
}

bool FileSelectionController::refresh() {
    ++generation_;
    std::filesystem::path root = request_.protected_root;
    std::filesystem::path location = request_.initial_location;
    const bool trusted_navigation = orchestrator_session_valid_ &&
        request_.authority == DocumentPickerAuthority::trusted_local_host;
    if (trusted_navigation) {
        const std::optional<NavigationTarget> target = trusted_directory_target(request_, location);
        if (!target) {
            last_error_ = "Folder is unavailable or outside the admitted roots";
            return false;
        }
        root = (*target).root;
        location = (*target).path;
    }
    DirectorySnapshot snapshot = read_directory(root, location,
                                                {}, generation_, {}, show_hidden_);
    if (!snapshot.available()) {
        last_error_ = snapshot.cancelled ? "directory refresh was cancelled" : snapshot.error;
        return false;
    }
    last_error_.clear();
    request_.protected_root = snapshot.root;
    request_.initial_location = snapshot.location;
    // Compact live entries in place. Preserve order and move only surviving rows.
    // Filter selection is invariant for this refresh and resolved before traversal.
    const DocumentTypeFilter* filter = find_filter(request_.filters, active_filter_id_);
    std::size_t visible_count = 0;
    for (std::size_t index = 0; index < snapshot.entries.size(); ++index) {
        DirectoryEntry& entry = snapshot.entries[index];
        if (trusted_navigation && entry.kind == EntryKind::symlink) {
            const std::optional<NavigationTarget> target = trusted_directory_target(request_, entry.path);
            if (target) {
                // Keep the alias identity/path. It may be entered, but direct
                // acceptance still refuses a link leaf and requires navigation.
                entry.directory = true;
                entry.secondary_text = "Folder link";
            }
        }
        if (!entry_matches(snapshot.entries[index], request_.profile, name_filter_, filter))
            continue;
        if (visible_count != index)
            snapshot.entries[visible_count] = std::move(snapshot.entries[index]);
        ++visible_count;
    }
    snapshot.entries.resize(visible_count);
    location_identity_ = observe_identity(snapshot.location);
    browser_ = std::move(snapshot);
    std::size_t selected_count = 0;
    for (std::size_t index = 0; index < selected_ids_.size(); ++index) {
        if (find_entry(selected_ids_[index]) == nullptr)
            continue;
        if (selected_count != index)
            selected_ids_[selected_count] = std::move(selected_ids_[index]);
        ++selected_count;
    }
    selected_ids_.resize(selected_count);
    return true;
}

const DirectoryEntry*
FileSelectionController::find_entry(const std::string_view stable_id) const noexcept {
    for (const DirectoryEntry& entry : browser_.entries) {
        if (entry.stable_id == stable_id)
            return &entry;
    }
    return nullptr;
}

bool FileSelectionController::set_selection(std::vector<std::string> stable_ids) {
    if (stable_ids.size() > request_.maximum_selection ||
        (!profile_accepts_multiple() && stable_ids.size() > 1U)) {
        return false;
    }
    // Cardinality was bounded to 32 before traversal. Compare the supplied
    // prefix directly, avoiding per-selection hash-node allocation.
    for (std::size_t index = 0; index < stable_ids.size(); ++index) {
        const std::string& id = stable_ids[index];
        const DirectoryEntry* entry = find_entry(id);
        if (entry == nullptr)
            return false;
        const bool navigable_link = orchestrator_session_valid_ &&
            request_.authority == DocumentPickerAuthority::trusted_local_host && (*entry).directory;
        if ((*entry).kind == EntryKind::symlink && !navigable_link)
            return false;
        for (std::size_t prior = 0; prior < index; ++prior) {
            if (stable_ids[prior] == id)
                return false;
        }
        if (request_.profile == DocumentPickerProfile::select_folder && !(*entry).directory)
            return false;
    }
    selected_ids_ = std::move(stable_ids);
    if (profile_saves() && selected_ids_.size() == 1U) {
        const DirectoryEntry* entry = find_entry(selected_ids_.front());
        if (entry && !(*entry).directory)
            filename_ = (*entry).name;
    }
    return true;
}

bool FileSelectionController::set_filename(std::string filename) {
    if (!profile_saves() || filename.size() > 255U)
        return false;
    filename_ = std::move(filename);
    return true;
}

bool FileSelectionController::set_active_filter(std::string filter_id) {
    const DocumentTypeFilter* filter = find_filter(request_.filters, filter_id);
    if (!filter_id.empty() && filter == nullptr)
        return false;
    std::string previous = active_filter_id_;
    active_filter_id_ = std::move(filter_id);
    if (refresh())
        return true;
    active_filter_id_ = std::move(previous);
    return false;
}

const std::string& FileSelectionController::name_filter() const noexcept {
    return name_filter_;
}

bool FileSelectionController::set_name_filter(std::string pattern) {
    if (pattern.size() > 255)
        return false;
    const std::string previous = name_filter_;
    name_filter_ = std::move(pattern);
    if (refresh())
        return true;
    name_filter_ = previous;
    return false;
}

bool FileSelectionController::set_show_hidden(const bool show_hidden) {
    if (!request_.allow_hidden_toggle && show_hidden != show_hidden_)
        return false;
    const bool previous = show_hidden_;
    show_hidden_ = show_hidden;
    if (refresh())
        return true;
    show_hidden_ = previous;
    return false;
}

void FileSelectionController::set_orchestrator_session_valid(const bool valid) noexcept {
    if (request_.authority != DocumentPickerAuthority::trusted_local_host) {
        request_.authority = DocumentPickerAuthority::orchestrator_session;
        orchestrator_session_valid_ = valid;
    }
}

void FileSelectionController::set_authority_valid(const bool valid) noexcept {
    orchestrator_session_valid_ =
        valid && request_.authority != DocumentPickerAuthority::unavailable;
}

DocumentPickerResult FileSelectionController::selection_error(std::string code,
                                                              std::string message) const {
    const DocumentPickerResult result{
        .terminal = DocumentPickerTerminal::validation_error,
        .code = std::move(code),
        .message = std::move(message),
        .selections = {},
        .native_fallback_permitted = request_.allow_native_fallback,
    };
    return result;
}

DocumentPickerResult FileSelectionController::accept(const bool overwrite_confirmed) const {
    if (!orchestrator_session_valid_) {
        const DocumentPickerResult result = unavailable_selection(
            "session-unavailable", "Selection authority must be revalidated before acceptance",
            request_.allow_native_fallback);
        return result;
    }
    if (!browser_.available()) {
        const DocumentPickerResult result = unavailable_selection(
            "location-unavailable", browser_.error, request_.allow_native_fallback);
        return result;
    }
    const ObjectIdentity current_location = observe_identity(browser_.location);
    if (!current_location.available() || current_location != location_identity_ ||
        path_route_has_symlink(request_.protected_root, browser_.location)) {
        const DocumentPickerResult result =
            selection_error("location-changed", "Current folder changed; refresh before selecting");
        return result;
    }
    if (profile_saves()) {
        if (!valid_platform_basename(filename_)) {
            const DocumentPickerResult result =
                selection_error("invalid-filename", "save filename must be one valid basename");
            return result;
        }
        if (!show_hidden_ && !filename_.empty() && filename_.front() == '.') {
            const DocumentPickerResult result = selection_error(
                "hidden-destination", "Enable Show hidden to select a hidden destination");
            return result;
        }
        std::string name = filename_;
        std::string extension = normalize_extension(request_.default_extension);
        const DocumentTypeFilter* filter = find_filter(request_.filters, active_filter_id_);
        if (filter != nullptr && !(*filter).extensions.empty() &&
            std::find((*filter).extensions.begin(), (*filter).extensions.end(), extension) ==
                (*filter).extensions.end()) {
            extension = (*filter).extensions.front();
        }
        if (!extension.empty()) {
            const std::filesystem::path name_path = path_from_utf8(name);
            const std::filesystem::path suffix_path = name_path.extension();
            const std::string suffix_text = path_utf8(suffix_path);
            const std::string suffix = lower(suffix_text);
            if (suffix != extension)
                name += extension;
        }
        if (!valid_platform_basename(name)) {
            const DocumentPickerResult result = selection_error(
                "invalid-filename", "Final filename including extension is invalid");
            return result;
        }
        const std::filesystem::path name_path = path_from_utf8(name);
        const std::filesystem::path combined = browser_.location / name_path;
        const std::filesystem::path destination = combined.lexically_normal();
        if (!path_is_within(request_.protected_root, destination) ||
            path_route_has_symlink(request_.protected_root, destination)) {
            const DocumentPickerResult result = selection_error(
                "destination-refused", "save destination failed protected-root policy");
            return result;
        }
        std::error_code destination_error{};
        const std::filesystem::file_status destination_status =
            std::filesystem::symlink_status(destination, destination_error);
        if (destination_error && destination_error != std::errc::no_such_file_or_directory) {
            const DocumentPickerResult result =
                selection_error("destination-unavailable", destination_error.message());
            return result;
        }
        const ObjectIdentity identity = observe_identity(destination);
        if (destination_status.type() != std::filesystem::file_type::not_found &&
            !identity.available()) {
            const DocumentPickerResult result =
                selection_error("destination-unavailable", "Destination identity is unavailable");
            return result;
        }
        if (overwrite_confirmed &&
            (!pending_overwrite_ || (*pending_overwrite_).path != destination ||
             !(*pending_overwrite_).identity.same_revision(identity))) {
            pending_overwrite_.reset();
            const DocumentPickerResult result = selection_error(
                "destination-changed", "Destination changed; request overwrite confirmation again");
            return result;
        }
        if (identity.available()) {
            if (identity.type != std::filesystem::file_type::regular) {
                const DocumentPickerResult result = selection_error(
                    "destination-not-file", "save destination is not a regular file");
                return result;
            }
            if (!overwrite_confirmed) {
                pending_overwrite_ = DocumentSelectionObservation{
                    .path = destination, .identity = identity, .existing = true};
                const DocumentPickerResult result =
                    overwrite_confirmation(*pending_overwrite_, request_.allow_native_fallback);
                return result;
            }
        }
        const DocumentSelectionObservation observation{
            .path = destination, .identity = identity, .existing = identity.available()};
        std::vector<DocumentSelectionObservation> observations{observation};
        const DocumentPickerResult result = accepted_selection(
            "picker returned a destination observation; the host still owns the write",
            std::move(observations), request_.allow_native_fallback);
        return result;
    }

    if (request_.profile == DocumentPickerProfile::select_folder && selected_ids_.empty()) {
        const ObjectIdentity identity = observe_identity(browser_.location);
        if (!identity.available() || identity.type != std::filesystem::file_type::directory) {
            const DocumentPickerResult result =
                selection_error("folder-unavailable", "current folder is no longer available");
            return result;
        }
        const DocumentSelectionObservation observation{
            .path = browser_.location, .identity = identity, .existing = true};
        std::vector<DocumentSelectionObservation> observations{observation};
        const DocumentPickerResult result = accepted_selection(
            "current folder accepted", std::move(observations), request_.allow_native_fallback);
        return result;
    }
    if (selected_ids_.empty()) {
        const DocumentPickerResult result =
            selection_error("selection-required", "select an admitted object");
        return result;
    }
    std::vector<DocumentSelectionObservation> observations{};
    observations.reserve(selected_ids_.size());
    for (const std::string& id : selected_ids_) {
        const DirectoryEntry* entry = find_entry(id);
        if (!entry) {
            const DocumentPickerResult result = selection_error(
                "selection-stale", "selected object is no longer in the browser snapshot");
            return result;
        }
        const ObjectIdentity current = observe_identity((*entry).path);
        if (!current.available() || !(*entry).identity.same_revision(current) ||
            path_route_has_symlink(request_.protected_root, (*entry).path)) {
            const DocumentPickerResult result =
                selection_error("selection-changed", "selected object changed before acceptance");
            return result;
        }
        const bool want_folder = request_.profile == DocumentPickerProfile::select_folder;
        if (want_folder != (*entry).directory) {
            const DocumentPickerResult result = selection_error(
                "selection-kind", "selected object kind is not accepted by this profile");
            return result;
        }
        DocumentSelectionObservation observation{
            .path = (*entry).path, .identity = current, .existing = true};
        observations.push_back(std::move(observation));
    }
    const DocumentPickerResult result = accepted_selection(
        "picker selections revalidated", std::move(observations), request_.allow_native_fallback);
    return result;
}

DocumentPickerResult FileSelectionController::cancel() const {
    const DocumentPickerResult result{
        .terminal = DocumentPickerTerminal::cancelled,
        .code = "cancelled",
        .message = "picker cancelled without a selection",
        .selections = {},
        .native_fallback_permitted = request_.allow_native_fallback,
    };
    return result;
}

} // namespace file_manager
