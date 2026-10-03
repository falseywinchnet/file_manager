#include "file_manager/platform_paths.hpp"
#include "file_manager/file_operations.hpp"
#include "native_file.hpp"
#include "native_copy.hpp"
#include "native_publication.hpp"

#include <cstdlib>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <system_error>

namespace file_manager {
namespace {

bool looks_like_repository_root(const std::filesystem::path& path) {
    std::error_code error{};
    if (std::filesystem::exists(path / ".git", error)) return true;
    error.clear();
    const bool has_agents = std::filesystem::is_regular_file(path / "AGENTS.md", error);
    error.clear();
    const bool has_gitignore =
        std::filesystem::is_regular_file(path / ".gitignore", error);
    const bool repository = has_agents && has_gitignore;
    return repository;
}

bool absolute_route_has_symlink(const std::filesystem::path& path) {
    std::filesystem::path cursor = path.root_path();
    for (const std::filesystem::path& component : path.relative_path()) {
        if (component == ".") continue;
        cursor /= component;
        const ObjectIdentity identity = observe_identity(cursor);
        if (!identity.available()) {
            throw std::invalid_argument(
                "operation root route is not fully observable");
        }
        // The native no-follow identity treats Windows reparse points as
        // links, including directory links not reported by MinGW's status.
        if (identity.type == std::filesystem::file_type::symlink) return true;
    }
    return false;
}

bool broad_or_personal_root(const std::filesystem::path& path) {
    if (path == path.root_path()) return true;
    std::error_code error{};
    const std::filesystem::path canonical_home =
        std::filesystem::canonical(user_home_directory(), error);
    if (!error && path == canonical_home) return true;
    const bool repository = looks_like_repository_root(path);
    return repository;
}

std::string identity_changed_message() {
    return "the filesystem object changed after it was observed";
}

struct CopyOutcome final {
    bool success{};
    bool cancelled{};
    std::string code{};
    std::string message{};
};

// Set only after this traversal successfully creates its own stage root.
// Failure to observe that root's identity leaves it retained for inspection.
struct CopyStageIdentity final {
    bool created{};
    ObjectIdentity identity{};
};

struct CopyObserverFailure final {};

// The traversal owns one snapshot. Path replacement happens per node, never
// per byte-buffer write. Observer and native callback borrows are synchronous.
struct CopyProgressState final {
    const CopyProgressObserver& observer;
    CopyProgress snapshot{};

    void publish() const {
        if (!observer) return;
        try { observer(snapshot); }
        catch (...) { throw CopyObserverFailure{}; }
    }

    void begin_node(const std::filesystem::path& source) {
        if (!observer) return;
        snapshot.current_source = source;
        publish();
    }

    void complete_node(const std::filesystem::path& source) {
        if (!observer) return;
        if (snapshot.completed_objects == std::numeric_limits<std::uint64_t>::max()) {
            throw std::overflow_error("copy progress object count overflow");
        }
        snapshot.current_source = source;
        ++snapshot.completed_objects;
        publish();
    }
};

struct CopyLeafProgress final {
    CopyProgressState& state;
    std::uint64_t prior_bytes{};

    void operator()(const NativeCopyProgress progress) const {
        if (progress.copied_bytes > std::numeric_limits<std::uint64_t>::max() - prior_bytes) {
            throw std::overflow_error("copy progress byte count overflow");
        }
        state.snapshot.copied_bytes = prior_bytes + progress.copied_bytes;
        state.publish();
    }
};

CopyOutcome regular_copy_outcome(const NativeCopyResult& native) {
    CopyOutcome result{};
    switch (native.terminal) {
        case NativeCopyTerminal::complete:
            result = {true, false, "copied", "file copied to stage"};
            break;
        case NativeCopyTerminal::cancelled:
            result = {false, true, "cancelled", "copy cancelled before publication"};
            break;
        case NativeCopyTerminal::source_changed:
            result = {false, false, "identity_changed", identity_changed_message()};
            break;
        case NativeCopyTerminal::callback_failed:
            result = {false, false, "copy_callback_failed", "copy callback failed"};
            break;
        case NativeCopyTerminal::failed:
            result = {false, false, "file_copy_failed",
                native.error ? native.error.message() : "file copy failed"};
            break;
    }
    if (native.source_close_error) {
        result.message += " · source close: " + native.source_close_error.message();
    }
    if (native.stage_close_error) {
        result.message += " · stage close: " + native.stage_close_error.message();
    }
    return result;
}

OperationTerminal publication_failure_terminal(const std::error_code& error) {
    if (error == std::errc::file_exists) return OperationTerminal::conflict;
    if (error == std::errc::cross_device_link ||
        error == std::errc::not_supported ||
        error == std::errc::operation_not_supported ||
        error == std::errc::function_not_supported) return OperationTerminal::unavailable;
    return OperationTerminal::failed;
}

std::string publication_failure_code(const std::error_code& error,
                                     const char* fallback) {
    if (error == std::errc::file_exists) return "destination_exists";
    if (error == std::errc::cross_device_link) return "cross_volume_unsupported";
    if (error == std::errc::not_supported || error == std::errc::operation_not_supported ||
        error == std::errc::function_not_supported) return "no_replace_unsupported";
    const std::string code{fallback};
    return code;
}

CopyOutcome copy_node_no_follow(const std::filesystem::path& source,
                                const std::filesystem::path& destination,
                                const std::uint64_t source_device,
                                NativeCopyWorkspace& workspace,
                                CopyStageIdentity& created_stage,
                                const CancellationCheck& cancelled,
                                const OperationFaultCheck& injected_fault,
                                CopyProgressState& progress,
                                const std::optional<ObjectIdentity>& requested_revision = std::nullopt) {
    if (cancelled && cancelled()) {
        const CopyOutcome outcome{false, true, "cancelled", "copy cancelled before publication"};
        return outcome;
    }
    const ObjectIdentity identity = observe_identity(source);
    if (!identity.available()) {
        const CopyOutcome outcome{false, false, "source_disappeared",
                "copy source disappeared during traversal"};
        return outcome;
    }
    if (identity.device != source_device) {
        const CopyOutcome outcome{false, false, "mount_boundary",
                "copy refuses to cross a nested volume boundary"};
        return outcome;
    }
    if (requested_revision && !identity.same_revision(*requested_revision)) {
        const CopyOutcome outcome{false, false, "identity_changed", identity_changed_message()};
        return outcome;
    }
    progress.begin_node(source);
    if (injected_fault) {
        if (const std::optional<std::error_code> fault = injected_fault(
                OperationFaultPoint::copy_before_node, source)) {
            const CopyOutcome outcome{false, false,
                    *fault == std::errc::no_space_on_device
                        ? "disk_full" : "copy_fault",
                    (*fault).message()};
            return outcome;
        }
    }

    std::error_code error{};
    switch (identity.type) {
        case std::filesystem::file_type::directory: {
            error = create_copy_directory_stage(destination);
            if (error) {
                const CopyOutcome outcome{false, false, "stage_create_failed",
                        error ? error.message() : "copy stage directory was not created"};
                return outcome;
            }
            created_stage.created = true;
            created_stage.identity = observe_identity(destination);
            if (!created_stage.identity.available()) {
                const CopyOutcome outcome{false, false, "stage_identity_unavailable",
                    "created copy stage cannot be identified; it was retained"};
                return outcome;
            }
            std::filesystem::directory_iterator iterator(source, error);
            if (error) {
                const CopyOutcome outcome{false, false, "source_enumeration_failed", error.message()};
                return outcome;
            }
            const std::filesystem::directory_iterator end{};
            while (iterator != end) {
                const std::filesystem::path child_source = (*iterator).path();
                const std::filesystem::path child_name = child_source.filename();
                const std::filesystem::path child_destination = destination / child_name;
                CopyStageIdentity child_stage{};
                const CopyOutcome child = copy_node_no_follow(
                    child_source, child_destination, source_device, workspace,
                    child_stage, cancelled, injected_fault, progress);
                if (!child.success) return child;
                iterator.increment(error);
                if (error) {
                    const CopyOutcome outcome{false, false, "source_enumeration_failed", error.message()};
                    return outcome;
                }
            }
            if (cancelled && cancelled()) {
                const CopyOutcome outcome{false, true, "cancelled", "copy cancelled before directory metadata"};
                return outcome;
            }
            const DirectoryCopyMetadataResult metadata = finish_copy_directory_metadata(
                source, destination, identity, created_stage.identity);
            if (!metadata.succeeded()) {
                std::string message = metadata.identity_changed ? identity_changed_message()
                    : metadata.error ? metadata.error.message() : "directory metadata finalization failed";
                if (metadata.source_close_error) message += " · source close: " + metadata.source_close_error.message();
                if (metadata.stage_close_error) message += " · stage close: " + metadata.stage_close_error.message();
                const CopyOutcome outcome{false, false,
                    metadata.identity_changed ? "identity_changed" : "directory_metadata_failed", std::move(message)};
                return outcome;
            }
            progress.complete_node(source);
            const CopyOutcome outcome{true, false, "copied", "directory copied to stage"};
            return outcome;
        }
        case std::filesystem::file_type::regular: {
            NativeCopyProgressObserver native_progress{};
            if (progress.observer) {
                native_progress = CopyLeafProgress{progress, progress.snapshot.copied_bytes};
            }
            const NativeCopyResult copied = copy_regular_file_to_stage(
                source, destination, identity, cancelled, workspace, native_progress);
            created_stage.created = copied.stage_created;
            created_stage.identity = copied.stage_identity;
            if (copied.terminal == NativeCopyTerminal::complete) progress.complete_node(source);
            const CopyOutcome outcome = regular_copy_outcome(copied);
            return outcome;
        }
        case std::filesystem::file_type::symlink: {
            const NativeSymlink link = read_native_symlink(source, error);
            if (error) {
                const CopyOutcome outcome{false, false, "symlink_read_failed", error.message()};
                return outcome;
            }
#if defined(_WIN32)
            // The link itself retains its directory kind even when its target
            // is missing. Do not follow the target to infer that kind.
            DWORD flags = link.directory
                ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0;
            flags |= SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
            if (!CreateSymbolicLinkW(destination.c_str(), link.target.c_str(), flags)) {
                error = std::error_code(static_cast<int>(GetLastError()),
                                        std::system_category());
            }
#else
            std::filesystem::create_symlink(link.target, destination, error);
#endif
            if (error) {
                const CopyOutcome outcome{false, false, "symlink_copy_failed", error.message()};
                return outcome;
            }
            created_stage.created = true;
            created_stage.identity = observe_identity(destination);
            if (!created_stage.identity.available()) {
                const CopyOutcome outcome{false, false, "stage_identity_unavailable",
                    "created copy link cannot be identified; it was retained"};
                return outcome;
            }
            progress.complete_node(source);
            const CopyOutcome outcome{true, false, "copied", "symlink leaf copied to stage"};
            return outcome;
        }
        default:
            const CopyOutcome outcome{false, false, "object_type_unsupported",
                    "copy supports regular files, directories, and symlink leaves"};
            return outcome;
    }
}

bool cleanup_stage(const std::filesystem::path& stage,
                   const ObjectIdentity& expected) {
    if (!expected.available() || observe_identity(stage) != expected) return false;
    std::error_code error{};
    std::filesystem::remove_all(stage, error);
    if (error) return false;
    const ObjectIdentity remaining = observe_identity(stage);
    const bool removed = !remaining.available();
    return removed;
}

} // namespace

FileOperationService::FileOperationService(const OperationPolicy policy)
    : policy_(policy) {
    if (policy == OperationPolicy::protected_profile) {
        throw std::invalid_argument("protected operations require explicit roots");
    }
}

FileOperationService::FileOperationService(
    std::filesystem::path protected_root,
    std::filesystem::path quarantine_root,
    const bool mutations_enabled,
    OperationFaultCheck injected_fault)
    : policy_(mutations_enabled ? OperationPolicy::protected_profile : OperationPolicy::read_only),
      mutations_enabled_(mutations_enabled),
      injected_fault_(std::move(injected_fault)) {
    if (!protected_root.is_absolute() || !quarantine_root.is_absolute()) {
        throw std::invalid_argument("operation roots must be absolute");
    }
    if (absolute_route_has_symlink(protected_root.lexically_normal()) ||
        absolute_route_has_symlink(quarantine_root.lexically_normal())) {
        throw std::invalid_argument(
            "operation roots must not traverse symbolic links");
    }
    protected_root_ = canonical_existing_directory(protected_root);
    quarantine_root_ = canonical_existing_directory(quarantine_root);
    if (broad_or_personal_root(protected_root_)) {
        throw std::invalid_argument(
            "mutation refuses a root, home directory, or repository root");
    }
    if (path_is_within(protected_root_, quarantine_root_) ||
        path_is_within(quarantine_root_, protected_root_)) {
        throw std::invalid_argument(
            "quarantine must be separate from the protected root");
    }
    const ObjectIdentity protected_identity = observe_identity(protected_root_);
    const ObjectIdentity quarantine_identity = observe_identity(quarantine_root_);
    if (!protected_identity.available() || !quarantine_identity.available() ||
        protected_identity.device != quarantine_identity.device) {
        throw std::invalid_argument(
            "protected root and quarantine must be on the same volume");
    }
}

std::string FileOperationService::next_operation_id() {
    std::ostringstream stream{};
    stream << "fm.operation." << std::setw(8) << std::setfill('0') << next_id_;
    ++next_id_;
    const std::string operation_id = stream.str();
    return operation_id;
}

OperationResult FileOperationService::result(
    const OperationKind kind,
    const OperationTerminal terminal,
    std::string code,
    std::string message,
    std::filesystem::path original,
    std::filesystem::path resulting,
    const ObjectIdentity identity) const {
    OperationResult value{};
    value.kind = kind;
    value.terminal = terminal;
    value.code = std::move(code);
    value.message = std::move(message);
    value.protected_root = protected_root_;
    value.original_path = std::move(original);
    value.resulting_path = std::move(resulting);
    value.identity = identity;
    value.undo_available = undo_.has_value();
    value.recoverable_object_retained =
        undo_.has_value() && (*undo_).kind == UndoKind::restore_quarantined;
    return value;
}

std::optional<std::string> FileOperationService::validate_parent(
    const std::filesystem::path& parent) const {
    if (!parent.is_absolute()) return "parent path must be absolute";
    const std::filesystem::path lexical = parent.lexically_normal();
    if (policy_ == OperationPolicy::ordinary_local) {
        if (parent != lexical) return "parent path must be lexically normalized";
        try {
            if (absolute_route_has_symlink(lexical)) {
                return "parent route traverses a symbolic link or reparse point";
            }
        } catch (const std::exception& error) {
            const std::string message = error.what();
            return message;
        }
        const ObjectIdentity parent_identity = observe_identity(lexical);
        if (!parent_identity.available() ||
            parent_identity.type != std::filesystem::file_type::directory) {
            return "parent must be an observable directory";
        }
        return std::nullopt;
    }
    if (!path_is_within(protected_root_, lexical)) {
        return "parent path is outside the protected root";
    }
    if (path_route_has_symlink(protected_root_, lexical)) {
        return "parent path traverses a symbolic link";
    }
    try {
        const std::filesystem::path canonical = canonical_existing_directory(lexical);
        if (!path_is_within(protected_root_, canonical)) {
            return "parent path resolves outside the protected root";
        }
    } catch (const std::exception& error) {
        const std::string message = error.what();
        return message;
    }
    return std::nullopt;
}

std::optional<std::string> FileOperationService::validate_source(
    const std::filesystem::path& source,
    const ObjectIdentity& expected) const {
    if (!source.is_absolute()) return "source path must be absolute";
    const std::filesystem::path lexical = source.lexically_normal();
    if (policy_ == OperationPolicy::ordinary_local && source != lexical) {
        return "source path must be lexically normalized";
    }
    if (lexical == lexical.root_path()) return "filesystem root cannot be renamed";
    if (policy_ != OperationPolicy::ordinary_local &&
        (lexical == protected_root_ || !path_is_within(protected_root_, lexical))) {
        return "source path is outside the mutable protected contents";
    }
    if (const std::optional<std::string> parent_error = validate_parent(lexical.parent_path())) {
        return parent_error;
    }
    if (!expected.available() || observe_identity(lexical) != expected) {
        const std::string message = identity_changed_message();
        return message;
    }
    return std::nullopt;
}

std::optional<std::string> FileOperationService::validate_basename(
    const std::string_view basename) {
    if (basename.empty()) return "name must not be empty";
    if (basename.size() > 255) return "name exceeds the 255-byte protected bound";
    if (basename == "." || basename == "..") return "reserved path component";
    if (!valid_platform_basename(basename)) {
        return "name must be one filesystem component";
    }
    return std::nullopt;
}

std::filesystem::path FileOperationService::available_quarantine_path(
    const std::filesystem::path& source) {
    const std::string basename = path_utf8(source.filename());
    for (std::uint64_t attempt = 0; attempt < 100'000; ++attempt) {
        std::ostringstream name{};
        name << "fm-q-" << std::setw(8) << std::setfill('0') << next_id_ << '-'
             << std::setw(5) << attempt << '-' << basename;
        const std::filesystem::path candidate = quarantine_root_ / path_from_utf8(name.str());
        const std::error_code vacancy = check_destination_vacant(candidate);
        if (!vacancy) return candidate;
        if (vacancy != std::errc::file_exists) {
            throw std::system_error(vacancy, "cannot observe quarantine destination");
        }
    }
    throw std::runtime_error("quarantine name space is exhausted");
}

OperationResult FileOperationService::create_folder(
    const std::filesystem::path& parent,
    const ObjectIdentity& expected_parent) {
    const std::string operation_id = next_operation_id();
    if (!mutations_enabled_ && policy_ != OperationPolicy::ordinary_local) {
        OperationResult value = result(OperationKind::create_folder,
                            OperationTerminal::unavailable,
                            "mutations_disabled",
                            "mutations require explicit protected-profile opt-in",
                            parent);
        value.operation_id = operation_id;
        return value;
    }
    if (const std::optional<std::string> parent_error = validate_parent(parent)) {
        OperationResult value = result(OperationKind::create_folder,
                            OperationTerminal::failed,
                            "invalid_parent", *parent_error, parent);
        value.operation_id = operation_id;
        return value;
    }
    if (policy_ == OperationPolicy::ordinary_local &&
        (!expected_parent.available() || observe_identity(parent) != expected_parent)) {
        OperationResult value = result(OperationKind::create_folder,
            OperationTerminal::conflict, "parent_identity_changed",
            "parent directory changed after the command was requested", parent);
        value.operation_id = operation_id;
        return value;
    }
    std::filesystem::path destination{};
    for (std::uint64_t suffix = 1; suffix <= 10'000; ++suffix) {
        const std::string name = suffix == 1 ? "New folder"
                                      : "New folder " + std::to_string(suffix);
        const std::filesystem::path candidate = parent / path_from_utf8(name);
        const std::error_code vacancy = check_destination_vacant(candidate);
        if (!vacancy) {
            destination = candidate;
            break;
        }
        if (vacancy != std::errc::file_exists) {
            OperationResult value = result(OperationKind::create_folder,
                OperationTerminal::failed, "destination_unavailable", vacancy.message(),
                parent, candidate);
            value.operation_id = operation_id;
            return value;
        }
    }
    if (destination.empty()) {
        OperationResult value = result(OperationKind::create_folder,
                            OperationTerminal::conflict,
                            "name_space_exhausted",
                            "no deterministic New folder name is available", parent);
        value.operation_id = operation_id;
        return value;
    }
    std::error_code error{};
    if (injected_fault_) {
        if (const std::optional<std::error_code> fault = injected_fault_(
                OperationFaultPoint::create_before_publication, destination)) {
            OperationResult value = result(OperationKind::create_folder,
                                OperationTerminal::failed,
                                *fault == std::errc::permission_denied
                                    ? "permission_denied" : "create_fault",
                                (*fault).message(), parent, destination);
            value.operation_id = operation_id;
            return value;
        }
    }
    if (policy_ == OperationPolicy::ordinary_local &&
        (validate_parent(parent) || observe_identity(parent) != expected_parent)) {
        OperationResult value = result(OperationKind::create_folder,
            OperationTerminal::conflict, "parent_identity_changed",
            "parent directory changed before folder creation", parent, destination);
        value.operation_id = operation_id;
        return value;
    }
    if (!std::filesystem::create_directory(destination, error) || error) {
        // create_directory reports false without an error for an existing directory.
        if (!error) error = std::make_error_code(std::errc::file_exists);
        OperationResult value = result(OperationKind::create_folder,
                            publication_failure_terminal(error),
                            publication_failure_code(error, "create_failed"),
                            error.message(),
                            parent, destination);
        value.operation_id = operation_id;
        return value;
    }
    const ObjectIdentity identity = observe_identity(destination);
    if (!identity.available() || identity.type != std::filesystem::file_type::directory) {
        undo_.reset();
        OperationResult value = result(OperationKind::create_folder,
            OperationTerminal::failed, "postcondition_failed",
            "folder was created but its identity is unavailable; it may remain",
            parent, destination, identity);
        value.operation_id = operation_id;
        value.recoverable_object_retained = true;
        return value;
    }
    undo_ = UndoRecord{UndoKind::remove_created_directory,
                       destination, {}, identity, expected_parent};
    OperationResult value = result(OperationKind::create_folder, OperationTerminal::success,
                        "created", "folder created", parent, destination, identity);
    value.operation_id = operation_id;
    return value;
}

OperationResult FileOperationService::rename_object(
    const std::filesystem::path& source,
    const ObjectIdentity& expected,
    const std::string_view new_basename,
    const ObjectIdentity& expected_parent) {
    const std::string operation_id = next_operation_id();
    if (!mutations_enabled_ && policy_ != OperationPolicy::ordinary_local) {
        OperationResult value = result(OperationKind::rename_object,
                            OperationTerminal::unavailable,
                            "mutations_disabled",
                            "mutations require explicit protected-profile opt-in",
                            source);
        value.operation_id = operation_id;
        return value;
    }
    if (const std::optional<std::string> name_error = validate_basename(new_basename)) {
        OperationResult value = result(OperationKind::rename_object,
                            OperationTerminal::failed,
                            "invalid_name", *name_error, source);
        value.operation_id = operation_id;
        return value;
    }
    if (const std::optional<std::string> source_error = validate_source(source, expected)) {
        const OperationTerminal terminal = *source_error == identity_changed_message()
            ? OperationTerminal::conflict : OperationTerminal::failed;
        OperationResult value = result(OperationKind::rename_object, terminal,
                            terminal == OperationTerminal::conflict
                                ? "identity_changed" : "invalid_source",
                            *source_error, source, {}, expected);
        value.operation_id = operation_id;
        return value;
    }
    const std::filesystem::path parent = source.parent_path();
    if (policy_ == OperationPolicy::ordinary_local &&
        (!expected_parent.available() || observe_identity(parent) != expected_parent)) {
        OperationResult value = result(OperationKind::rename_object,
            OperationTerminal::conflict, "parent_identity_changed",
            "parent directory changed after the command was requested", source);
        value.operation_id = operation_id;
        return value;
    }
    const std::filesystem::path basename = path_from_utf8(new_basename);
    const std::filesystem::path destination = parent / basename;
    if (destination == source) {
        OperationResult value = result(OperationKind::rename_object,
            OperationTerminal::success, "name_unchanged", "name unchanged",
            source, source, expected);
        value.operation_id = operation_id;
        return value;
    }
    const std::error_code vacancy = check_destination_vacant(destination);
    if (vacancy) {
        OperationResult value = result(OperationKind::rename_object,
                            vacancy == std::errc::file_exists
                                ? OperationTerminal::conflict : OperationTerminal::failed,
                            vacancy == std::errc::file_exists
                                ? "destination_exists" : "destination_unavailable",
                            vacancy.message(),
                            source, destination, expected);
        value.operation_id = operation_id;
        return value;
    }
    if (policy_ == OperationPolicy::ordinary_local &&
        (validate_parent(parent) || observe_identity(parent) != expected_parent)) {
        OperationResult value = result(OperationKind::rename_object,
            OperationTerminal::conflict, "parent_identity_changed",
            "parent directory changed before rename", source, destination, expected);
        value.operation_id = operation_id;
        return value;
    }
    const std::error_code error = rename_no_replace(source, destination);
    if (error) {
        OperationResult value = result(OperationKind::rename_object,
                            publication_failure_terminal(error),
                            publication_failure_code(error, "rename_failed"), error.message(),
                            source, destination, expected);
        value.operation_id = operation_id;
        return value;
    }
    const ObjectIdentity observed = observe_identity(destination);
    if (observed != expected) {
        undo_.reset();
        OperationResult value = result(OperationKind::rename_object,
                            OperationTerminal::failed,
                            "postcondition_failed",
                            "object was renamed but its identity did not match; it may remain at the new path",
                            source, destination, observed);
        value.operation_id = operation_id;
        value.recoverable_object_retained = true;
        return value;
    }
    undo_ = UndoRecord{UndoKind::rename_back, destination, source, observed, expected_parent};
    OperationResult value = result(OperationKind::rename_object, OperationTerminal::success,
                        "renamed", "object renamed", source, destination, observed);
    value.operation_id = operation_id;
    return value;
}

OperationResult FileOperationService::quarantine_object(
    const std::filesystem::path& source,
    const ObjectIdentity& expected) {
    const std::string operation_id = next_operation_id();
    if (!mutations_enabled_) {
        OperationResult value = result(OperationKind::quarantine_object,
                            OperationTerminal::unavailable,
                            "mutations_disabled",
                            "mutations require explicit protected-profile opt-in",
                            source);
        value.operation_id = operation_id;
        return value;
    }
    if (const std::optional<std::string> source_error = validate_source(source, expected)) {
        const OperationTerminal terminal = *source_error == identity_changed_message()
            ? OperationTerminal::conflict : OperationTerminal::failed;
        OperationResult value = result(OperationKind::quarantine_object, terminal,
                            terminal == OperationTerminal::conflict
                                ? "identity_changed" : "invalid_source",
                            *source_error, source, {}, expected);
        value.operation_id = operation_id;
        return value;
    }
    std::filesystem::path destination{};
    try {
        destination = available_quarantine_path(source);
    } catch (const std::exception& error) {
        OperationResult value = result(OperationKind::quarantine_object,
                            OperationTerminal::failed,
                            "quarantine_unavailable", error.what(),
                            source, {}, expected);
        value.operation_id = operation_id;
        return value;
    }
    const std::error_code error = rename_no_replace(source, destination);
    if (error) {
        OperationResult value = result(OperationKind::quarantine_object,
                            publication_failure_terminal(error),
                            publication_failure_code(error, "quarantine_failed"),
                            error.message(), source, destination, expected);
        value.operation_id = operation_id;
        return value;
    }
    const ObjectIdentity observed = observe_identity(destination);
    if (observed != expected) {
        OperationResult value = result(OperationKind::quarantine_object,
                            OperationTerminal::failed,
                            "postcondition_failed",
                            "quarantined object identity did not match the commit",
                            source, destination, observed);
        value.operation_id = operation_id;
        return value;
    }
    undo_ = UndoRecord{UndoKind::restore_quarantined,
                       destination, source, observed};
    OperationResult value = result(OperationKind::quarantine_object,
                        OperationTerminal::success,
                        "quarantined", "object moved to recoverable quarantine",
                        source, destination, observed);
    value.operation_id = operation_id;
    return value;
}

OperationResult FileOperationService::copy_object(
    const std::filesystem::path& source,
    const ObjectIdentity& expected,
    const std::filesystem::path& destination_parent,
    const CancellationCheck& cancelled,
    const CopyProgressObserver& progress) {
    const std::string operation_id = next_operation_id();
    if (!mutations_enabled_) {
        OperationResult value = result(OperationKind::copy_object,
                            OperationTerminal::unavailable,
                            "mutations_disabled",
                            "mutations require explicit protected-profile opt-in",
                            source, destination_parent);
        value.operation_id = operation_id;
        return value;
    }
    if (const std::optional<std::string> source_error = validate_source(source, expected)) {
        const OperationTerminal terminal = *source_error == identity_changed_message()
            ? OperationTerminal::conflict : OperationTerminal::failed;
        OperationResult value = result(OperationKind::copy_object, terminal,
                            terminal == OperationTerminal::conflict
                                ? "identity_changed" : "invalid_source",
                            *source_error, source, destination_parent, expected);
        value.operation_id = operation_id;
        return value;
    }
    if (const std::optional<std::string> parent_error = validate_parent(destination_parent)) {
        OperationResult value = result(OperationKind::copy_object,
                            OperationTerminal::failed,
                            "invalid_destination", *parent_error,
                            source, destination_parent, expected);
        value.operation_id = operation_id;
        return value;
    }
    if (expected.type == std::filesystem::file_type::directory &&
        path_is_within(source, destination_parent.lexically_normal())) {
        OperationResult value = result(OperationKind::copy_object,
                            OperationTerminal::conflict,
                            "recursive_destination",
                            "a directory cannot be copied into itself",
                            source, destination_parent, expected);
        value.operation_id = operation_id;
        return value;
    }
    const std::filesystem::path destination = destination_parent / source.filename();
    const std::error_code vacancy = check_destination_vacant(destination);
    if (vacancy) {
        OperationResult value = result(OperationKind::copy_object,
                            vacancy == std::errc::file_exists
                                ? OperationTerminal::conflict : OperationTerminal::failed,
                            vacancy == std::errc::file_exists
                                ? "destination_exists" : "destination_unavailable",
                            vacancy.message(),
                            source, destination, expected);
        value.operation_id = operation_id;
        return value;
    }
    const std::filesystem::path stage = destination_parent /
        (".fm-stage-" + operation_id + '-' + path_utf8(source.filename()));
    const std::error_code stage_vacancy = check_destination_vacant(stage);
    if (stage_vacancy) {
        OperationResult value = result(OperationKind::copy_object,
                            stage_vacancy == std::errc::file_exists
                                ? OperationTerminal::conflict : OperationTerminal::failed,
                            stage_vacancy == std::errc::file_exists
                                ? "stage_exists" : "stage_unavailable",
                            stage_vacancy.message(),
                            source, stage, expected);
        value.operation_id = operation_id;
        return value;
    }

    // Allocate the reusable byte workspace before creating any stage objects.
    NativeCopyWorkspace workspace{};
    CopyStageIdentity created_stage{};
    CopyOutcome copied{};
    CopyProgressState progress_state{progress};
    const std::optional<ObjectIdentity> requested_revision{expected};
    if (expected.type == std::filesystem::file_type::regular) {
        progress_state.snapshot.total_bytes = expected.size;
    }
    try {
        copied = copy_node_no_follow(source, stage, expected.device, workspace,
            created_stage, cancelled, injected_fault_, progress_state, requested_revision);
        if (copied.success) {
            progress_state.snapshot.phase = CopyPhase::finalizing;
            progress_state.publish();
        }
        if (copied.success && cancelled && cancelled()) {
            copied = {false, true, "cancelled", "copy cancelled before publication"};
        }
    } catch (const CopyObserverFailure&) {
        copied = {false, false, "copy_callback_failed", "copy progress observer failed"};
    } catch (const std::exception& failure) {
        copied = {false, false, "copy_failed", failure.what()};
    } catch (...) {
        copied = {false, false, "copy_failed", "copy failed before publication"};
    }
    const ObjectIdentity stage_identity = created_stage.identity;
    if (!copied.success) {
        const bool cleaned = !created_stage.created ||
            cleanup_stage(stage, stage_identity);
        OperationTerminal terminal = OperationTerminal::failed;
        if (copied.cancelled) terminal = OperationTerminal::cancelled;
        else if (copied.code == "identity_changed") terminal = OperationTerminal::conflict;
        OperationResult value = result(OperationKind::copy_object,
                            terminal,
                            copied.code, copied.message,
                            source, stage, expected);
        value.operation_id = operation_id;
        value.recoverable_object_retained = !cleaned;
        return value;
    }
    if (!observe_identity(source).same_revision(expected)) {
        const bool cleaned = cleanup_stage(stage, stage_identity);
        OperationResult value = result(OperationKind::copy_object,
                            OperationTerminal::conflict,
                            "identity_changed", identity_changed_message(),
                            source, stage, expected);
        value.operation_id = operation_id;
        value.recoverable_object_retained = !cleaned;
        return value;
    }
    if (injected_fault_) {
        if (const std::optional<std::error_code> fault = injected_fault_(
                OperationFaultPoint::copy_before_publication, destination)) {
            const bool cleaned = cleanup_stage(stage, stage_identity);
            OperationResult value = result(OperationKind::copy_object,
                                OperationTerminal::failed,
                                *fault == std::errc::no_space_on_device
                                    ? "disk_full" : "publication_fault",
                                (*fault).message(), source, stage, expected);
            value.operation_id = operation_id;
            value.recoverable_object_retained = !cleaned;
            return value;
        }
    }
    bool publication_cancelled{};
    bool publication_callback_failed{};
    try {
        progress_state.snapshot.phase = CopyPhase::publishing;
        progress_state.publish();
        if (cancelled) publication_cancelled = cancelled();
    } catch (...) {
        publication_callback_failed = true;
    }
    if (publication_cancelled || publication_callback_failed) {
        const bool cleaned = cleanup_stage(stage, stage_identity);
        const OperationTerminal terminal = publication_callback_failed
            ? OperationTerminal::failed : OperationTerminal::cancelled;
        const std::string code = publication_callback_failed ? "copy_callback_failed" : "cancelled";
        const std::string message = publication_callback_failed
            ? "copy callback failed before publication" : "copy cancelled before publication";
        OperationResult value = result(OperationKind::copy_object, terminal, code, message,
            source, stage, expected);
        value.operation_id = operation_id;
        value.recoverable_object_retained = !cleaned;
        return value;
    }
    const std::error_code error = rename_no_replace(stage, destination);
    if (error) {
        const bool cleaned = cleanup_stage(stage, stage_identity);
        std::filesystem::path reported_path = cleaned ? destination : stage;
        OperationResult value = result(OperationKind::copy_object,
                            publication_failure_terminal(error),
                            publication_failure_code(error, "publication_failed"), error.message(),
                            source, std::move(reported_path), expected);
        value.operation_id = operation_id;
        value.recoverable_object_retained = !cleaned;
        return value;
    }
    const ObjectIdentity result_identity = observe_identity(destination);
    undo_.reset();
    OperationResult value = result(OperationKind::copy_object, OperationTerminal::success,
                        "copied", "object copied with staged publication",
                        source, destination, result_identity);
    value.operation_id = operation_id;
    return value;
}

OperationResult FileOperationService::move_object(
    const std::filesystem::path& source,
    const ObjectIdentity& expected,
    const std::filesystem::path& destination_parent) {
    const std::string operation_id = next_operation_id();
    if (!mutations_enabled_) {
        OperationResult value = result(OperationKind::move_object,
                            OperationTerminal::unavailable,
                            "mutations_disabled",
                            "mutations require explicit protected-profile opt-in",
                            source, destination_parent);
        value.operation_id = operation_id;
        return value;
    }
    if (const std::optional<std::string> source_error = validate_source(source, expected)) {
        const OperationTerminal terminal = *source_error == identity_changed_message()
            ? OperationTerminal::conflict : OperationTerminal::failed;
        OperationResult value = result(OperationKind::move_object, terminal,
                            terminal == OperationTerminal::conflict
                                ? "identity_changed" : "invalid_source",
                            *source_error, source, destination_parent, expected);
        value.operation_id = operation_id;
        return value;
    }
    if (const std::optional<std::string> parent_error = validate_parent(destination_parent)) {
        OperationResult value = result(OperationKind::move_object,
                            OperationTerminal::failed,
                            "invalid_destination", *parent_error,
                            source, destination_parent, expected);
        value.operation_id = operation_id;
        return value;
    }
    if (expected.type == std::filesystem::file_type::directory &&
        path_is_within(source, destination_parent.lexically_normal())) {
        OperationResult value = result(OperationKind::move_object,
                            OperationTerminal::conflict,
                            "recursive_destination",
                            "a directory cannot be moved into itself",
                            source, destination_parent, expected);
        value.operation_id = operation_id;
        return value;
    }
    const std::filesystem::path destination = destination_parent / source.filename();
    if (destination == source) {
        OperationResult value = result(OperationKind::move_object,
                            OperationTerminal::conflict,
                            "same_destination",
                            "source already has that destination",
                            source, destination, expected);
        value.operation_id = operation_id;
        return value;
    }
    const std::error_code vacancy = check_destination_vacant(destination);
    if (vacancy) {
        OperationResult value = result(OperationKind::move_object,
                            vacancy == std::errc::file_exists
                                ? OperationTerminal::conflict : OperationTerminal::failed,
                            vacancy == std::errc::file_exists
                                ? "destination_exists" : "destination_unavailable",
                            vacancy.message(),
                            source, destination, expected);
        value.operation_id = operation_id;
        return value;
    }
    const ObjectIdentity destination_identity = observe_identity(destination_parent);
    if (!destination_identity.available() ||
        destination_identity.device != expected.device) {
        OperationResult value = result(OperationKind::move_object,
                            OperationTerminal::unavailable,
                            "cross_volume_unsupported",
                            "cross-volume move waits for staged copy and fault evidence",
                            source, destination, expected);
        value.operation_id = operation_id;
        return value;
    }
    std::error_code error{};
    if (injected_fault_) {
        if (const std::optional<std::error_code> fault = injected_fault_(
                OperationFaultPoint::move_before_publication, destination)) {
            OperationResult value = result(
                OperationKind::move_object,
                *fault == std::errc::cross_device_link
                    ? OperationTerminal::unavailable
                    : OperationTerminal::failed,
                *fault == std::errc::cross_device_link
                    ? "cross_volume_unsupported" : "move_fault",
                (*fault).message(), source, destination, expected);
            value.operation_id = operation_id;
            return value;
        }
    }
    error = rename_no_replace(source, destination);
    if (error) {
        OperationResult value = result(OperationKind::move_object,
                            publication_failure_terminal(error),
                            publication_failure_code(error, "move_failed"), error.message(),
                            source, destination, expected);
        value.operation_id = operation_id;
        return value;
    }
    const ObjectIdentity observed = observe_identity(destination);
    if (observed != expected) {
        OperationResult value = result(OperationKind::move_object,
                            OperationTerminal::failed,
                            "postcondition_failed",
                            "moved object identity did not match the commit",
                            source, destination, observed);
        value.operation_id = operation_id;
        return value;
    }
    undo_ = UndoRecord{UndoKind::rename_back, destination, source, observed};
    OperationResult value = result(OperationKind::move_object, OperationTerminal::success,
                        "moved", "object moved", source, destination, observed);
    value.operation_id = operation_id;
    return value;
}

OperationResult FileOperationService::undo_last() {
    const std::string operation_id = next_operation_id();
    if (!mutations_enabled_ && policy_ != OperationPolicy::ordinary_local) {
        OperationResult value = result(OperationKind::undo, OperationTerminal::unavailable,
                            "mutations_disabled",
                            "mutations require explicit protected-profile opt-in");
        value.operation_id = operation_id;
        return value;
    }
    if (!undo_) {
        OperationResult value = result(OperationKind::undo, OperationTerminal::unavailable,
                            "nothing_to_undo", "no operation is available to undo");
        value.operation_id = operation_id;
        return value;
    }
    const UndoRecord record = *undo_;
    if (policy_ == OperationPolicy::ordinary_local) {
        const std::filesystem::path parent = record.current_path.parent_path();
        const std::optional<std::string> parent_error = validate_parent(parent);
        const ObjectIdentity observed_parent = observe_identity(parent);
        if (parent_error || !record.parent_identity.available() ||
            observed_parent != record.parent_identity) {
            OperationResult value = result(OperationKind::undo,
                OperationTerminal::conflict, "parent_identity_changed",
                parent_error ? *parent_error : "undo parent identity changed",
                record.current_path, record.original_path, record.identity);
            value.operation_id = operation_id;
            return value;
        }
    }
    if (observe_identity(record.current_path) != record.identity) {
        OperationResult value = result(OperationKind::undo, OperationTerminal::conflict,
                            "identity_changed", identity_changed_message(),
                            record.current_path, record.original_path,
                            record.identity);
        value.operation_id = operation_id;
        return value;
    }
    std::error_code error{};
    if (record.kind == UndoKind::remove_created_directory) {
        if (!std::filesystem::is_empty(record.current_path, error) || error) {
            OperationResult value = result(OperationKind::undo, OperationTerminal::conflict,
                                "created_folder_not_empty",
                                error ? error.message()
                                      : "created folder is no longer empty",
                                record.current_path, {}, record.identity);
            value.operation_id = operation_id;
            return value;
        }
        if (policy_ == OperationPolicy::ordinary_local) {
            const std::filesystem::path parent = record.current_path.parent_path();
            if (validate_parent(parent) || observe_identity(parent) != record.parent_identity) {
                OperationResult value = result(OperationKind::undo,
                    OperationTerminal::conflict, "parent_identity_changed",
                    "parent directory changed before folder removal", record.current_path);
                value.operation_id = operation_id;
                return value;
            }
        }
        if (!std::filesystem::remove(record.current_path, error) || error) {
            OperationResult value = result(OperationKind::undo, OperationTerminal::failed,
                                "undo_remove_failed",
                                error ? error.message() : "created folder was not removed",
                                record.current_path, {}, record.identity);
            value.operation_id = operation_id;
            return value;
        }
        undo_.reset();
        OperationResult value = result(OperationKind::undo, OperationTerminal::success,
                            "undone", "created folder removed",
                            record.current_path, {}, record.identity);
        value.operation_id = operation_id;
        return value;
    }
    const std::error_code vacancy = check_destination_vacant(record.original_path);
    if (vacancy) {
        OperationResult value = result(OperationKind::undo,
                            vacancy == std::errc::file_exists
                                ? OperationTerminal::conflict : OperationTerminal::failed,
                            vacancy == std::errc::file_exists
                                ? "destination_exists" : "destination_unavailable",
                            vacancy.message(),
                            record.current_path, record.original_path,
                            record.identity);
        value.operation_id = operation_id;
        return value;
    }
    if (const std::optional<std::string> parent_error = validate_parent(record.original_path.parent_path())) {
        OperationResult value = result(OperationKind::undo, OperationTerminal::failed,
                            "invalid_restore_parent", *parent_error,
                            record.current_path, record.original_path,
                            record.identity);
        value.operation_id = operation_id;
        return value;
    }
    if (policy_ == OperationPolicy::ordinary_local) {
        const std::filesystem::path parent = record.current_path.parent_path();
        const std::filesystem::path original_parent = record.original_path.parent_path();
        if (parent != original_parent || validate_parent(parent) ||
            observe_identity(parent) != record.parent_identity) {
            OperationResult value = result(OperationKind::undo,
                OperationTerminal::conflict, "parent_identity_changed",
                "parent directory changed before rename undo",
                record.current_path, record.original_path, record.identity);
            value.operation_id = operation_id;
            return value;
        }
    }
    error = rename_no_replace(record.current_path, record.original_path);
    if (error) {
        OperationResult value = result(OperationKind::undo, publication_failure_terminal(error),
                            publication_failure_code(error, "undo_rename_failed"), error.message(),
                            record.current_path, record.original_path,
                            record.identity);
        value.operation_id = operation_id;
        return value;
    }
    if (observe_identity(record.original_path) != record.identity) {
        undo_.reset();
        OperationResult value = result(OperationKind::undo, OperationTerminal::failed,
                            "postcondition_failed",
                            "object was restored but its identity did not match; it may remain at the original path",
                            record.current_path, record.original_path,
                            record.identity);
        value.operation_id = operation_id;
        value.recoverable_object_retained = true;
        return value;
    }
    undo_.reset();
    OperationResult value = result(OperationKind::undo, OperationTerminal::success,
                        "undone", "operation undone",
                        record.current_path, record.original_path,
                        record.identity);
    value.operation_id = operation_id;
    return value;
}

} // namespace file_manager
