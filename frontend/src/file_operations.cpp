#include "file_manager/platform_paths.hpp"
#include "file_manager/file_operations.hpp"

#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <system_error>

namespace file_manager {
namespace {

bool looks_like_repository_root(const std::filesystem::path& path) {
    std::error_code error;
    if (std::filesystem::exists(path / ".git", error)) return true;
    error.clear();
    const bool has_agents = std::filesystem::is_regular_file(path / "AGENTS.md", error);
    error.clear();
    const bool has_gitignore =
        std::filesystem::is_regular_file(path / ".gitignore", error);
    return has_agents && has_gitignore;
}

bool absolute_route_has_symlink(const std::filesystem::path& path) {
    auto cursor = path.root_path();
    for (const auto& component : path.relative_path()) {
        if (component == ".") continue;
        cursor /= component;
        std::error_code error;
        const auto status = std::filesystem::symlink_status(cursor, error);
        if (error) {
            throw std::invalid_argument(
                "operation root route is not fully observable: " + error.message());
        }
        if (std::filesystem::is_symlink(status)) return true;
    }
    return false;
}

bool broad_or_personal_root(const std::filesystem::path& path) {
    if (path == path.root_path()) return true;
    std::error_code error;
    const std::filesystem::path canonical_home =
        std::filesystem::canonical(user_home_directory(), error);
    if (!error && path == canonical_home) return true;
    return looks_like_repository_root(path);
}

std::string identity_changed_message() {
    return "the filesystem object changed after it was observed";
}

struct CopyOutcome final {
    bool success{};
    bool cancelled{};
    std::string code;
    std::string message;
};

CopyOutcome copy_node_no_follow(const std::filesystem::path& source,
                                const std::filesystem::path& destination,
                                const std::uint64_t source_device,
                                const CancellationCheck& cancelled,
                                const OperationFaultCheck& injected_fault) {
    if (cancelled && cancelled()) {
        return {false, true, "cancelled", "copy cancelled before publication"};
    }
    const auto identity = observe_identity(source);
    if (!identity.available()) {
        return {false, false, "source_disappeared",
                "copy source disappeared during traversal"};
    }
    if (identity.device != source_device) {
        return {false, false, "mount_boundary",
                "copy refuses to cross a nested volume boundary"};
    }
    if (injected_fault) {
        if (const auto fault = injected_fault(
                OperationFaultPoint::copy_before_node, source)) {
            return {false, false,
                    *fault == std::errc::no_space_on_device
                        ? "disk_full" : "copy_fault",
                    fault->message()};
        }
    }

    std::error_code error;
    switch (identity.type) {
        case std::filesystem::file_type::directory: {
            if (!std::filesystem::create_directory(destination, source, error) || error) {
                return {false, false, "stage_create_failed",
                        error ? error.message() : "copy stage directory was not created"};
            }
            std::filesystem::directory_iterator iterator(source, error);
            if (error) {
                return {false, false, "source_enumeration_failed", error.message()};
            }
            const std::filesystem::directory_iterator end;
            while (iterator != end) {
                const auto child = copy_node_no_follow(
                    iterator->path(), destination / iterator->path().filename(),
                    source_device, cancelled, injected_fault);
                if (!child.success) return child;
                iterator.increment(error);
                if (error) {
                    return {false, false, "source_enumeration_failed", error.message()};
                }
            }
            return {true, false, "copied", "directory copied to stage"};
        }
        case std::filesystem::file_type::regular:
            if (!std::filesystem::copy_file(source, destination, error) || error) {
                return {false, false, "file_copy_failed",
                        error ? error.message() : "file was not copied"};
            }
            return {true, false, "copied", "file copied to stage"};
        case std::filesystem::file_type::symlink: {
            const auto target = std::filesystem::read_symlink(source, error);
            if (error) return {false, false, "symlink_read_failed", error.message()};
            std::filesystem::create_symlink(target, destination, error);
            if (error) return {false, false, "symlink_copy_failed", error.message()};
            return {true, false, "copied", "symlink leaf copied to stage"};
        }
        default:
            return {false, false, "object_type_unsupported",
                    "copy supports regular files, directories, and symlink leaves"};
    }
}

bool cleanup_stage(const std::filesystem::path& stage,
                   const ObjectIdentity& expected) {
    if (!expected.available() || observe_identity(stage) != expected) return false;
    std::error_code error;
    std::filesystem::remove_all(stage, error);
    if (error) return false;
    return !observe_identity(stage).available();
}

} // namespace

FileOperationService::FileOperationService(
    std::filesystem::path protected_root,
    std::filesystem::path quarantine_root,
    const bool mutations_enabled,
    OperationFaultCheck injected_fault)
    : mutations_enabled_(mutations_enabled),
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
    const auto protected_identity = observe_identity(protected_root_);
    const auto quarantine_identity = observe_identity(quarantine_root_);
    if (!protected_identity.available() || !quarantine_identity.available() ||
        protected_identity.device != quarantine_identity.device) {
        throw std::invalid_argument(
            "protected root and quarantine must be on the same volume");
    }
}

std::string FileOperationService::next_operation_id() {
    std::ostringstream stream;
    stream << "fm.operation." << std::setw(8) << std::setfill('0') << next_id_++;
    return stream.str();
}

OperationResult FileOperationService::result(
    const OperationKind kind,
    const OperationTerminal terminal,
    std::string code,
    std::string message,
    std::filesystem::path original,
    std::filesystem::path resulting,
    const ObjectIdentity identity) const {
    OperationResult value;
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
        undo_.has_value() && undo_->kind == UndoKind::restore_quarantined;
    return value;
}

std::optional<std::string> FileOperationService::validate_parent(
    const std::filesystem::path& parent) const {
    if (!parent.is_absolute()) return "parent path must be absolute";
    const auto lexical = parent.lexically_normal();
    if (!path_is_within(protected_root_, lexical)) {
        return "parent path is outside the protected root";
    }
    if (path_route_has_symlink(protected_root_, lexical)) {
        return "parent path traverses a symbolic link";
    }
    try {
        const auto canonical = canonical_existing_directory(lexical);
        if (!path_is_within(protected_root_, canonical)) {
            return "parent path resolves outside the protected root";
        }
    } catch (const std::exception& error) {
        return error.what();
    }
    return std::nullopt;
}

std::optional<std::string> FileOperationService::validate_source(
    const std::filesystem::path& source,
    const ObjectIdentity& expected) const {
    if (!source.is_absolute()) return "source path must be absolute";
    const auto lexical = source.lexically_normal();
    if (lexical == protected_root_ || !path_is_within(protected_root_, lexical)) {
        return "source path is outside the mutable protected contents";
    }
    if (const auto parent_error = validate_parent(lexical.parent_path())) {
        return parent_error;
    }
    if (!expected.available() || observe_identity(lexical) != expected) {
        return identity_changed_message();
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

bool FileOperationService::destination_exists_no_follow(
    const std::filesystem::path& path) {
    std::error_code error;
    const auto status = std::filesystem::symlink_status(path, error);
    return !error && status.type() != std::filesystem::file_type::not_found;
}

std::filesystem::path FileOperationService::available_quarantine_path(
    const std::filesystem::path& source) {
    const auto basename = path_utf8(source.filename());
    for (std::uint64_t attempt = 0; attempt < 100'000; ++attempt) {
        std::ostringstream name;
        name << "fm-q-" << std::setw(8) << std::setfill('0') << next_id_ << '-'
             << std::setw(5) << attempt << '-' << basename;
        const auto candidate = quarantine_root_ / path_from_utf8(name.str());
        if (!destination_exists_no_follow(candidate)) return candidate;
    }
    throw std::runtime_error("quarantine name space is exhausted");
}

OperationResult FileOperationService::create_folder(
    const std::filesystem::path& parent) {
    const auto operation_id = next_operation_id();
    if (!mutations_enabled_) {
        auto value = result(OperationKind::create_folder,
                            OperationTerminal::unavailable,
                            "mutations_disabled",
                            "mutations require explicit protected-profile opt-in",
                            parent);
        value.operation_id = operation_id;
        return value;
    }
    if (const auto parent_error = validate_parent(parent)) {
        auto value = result(OperationKind::create_folder,
                            OperationTerminal::failed,
                            "invalid_parent", *parent_error, parent);
        value.operation_id = operation_id;
        return value;
    }
    std::filesystem::path destination;
    for (std::uint64_t suffix = 1; suffix <= 10'000; ++suffix) {
        const auto name = suffix == 1 ? "New folder"
                                      : "New folder " + std::to_string(suffix);
        const std::filesystem::path candidate = parent / path_from_utf8(name);
        if (!destination_exists_no_follow(candidate)) {
            destination = candidate;
            break;
        }
    }
    if (destination.empty()) {
        auto value = result(OperationKind::create_folder,
                            OperationTerminal::conflict,
                            "name_space_exhausted",
                            "no deterministic New folder name is available", parent);
        value.operation_id = operation_id;
        return value;
    }
    std::error_code error;
    if (injected_fault_) {
        if (const auto fault = injected_fault_(
                OperationFaultPoint::create_before_publication, destination)) {
            auto value = result(OperationKind::create_folder,
                                OperationTerminal::failed,
                                *fault == std::errc::permission_denied
                                    ? "permission_denied" : "create_fault",
                                fault->message(), parent, destination);
            value.operation_id = operation_id;
            return value;
        }
    }
    if (!std::filesystem::create_directory(destination, error) || error) {
        auto value = result(OperationKind::create_folder,
                            OperationTerminal::failed,
                            "create_failed",
                            error ? error.message() : "directory was not created",
                            parent, destination);
        value.operation_id = operation_id;
        return value;
    }
    const auto identity = observe_identity(destination);
    undo_ = UndoRecord{UndoKind::remove_created_directory,
                       destination, {}, identity};
    auto value = result(OperationKind::create_folder, OperationTerminal::success,
                        "created", "folder created", parent, destination, identity);
    value.operation_id = operation_id;
    return value;
}

OperationResult FileOperationService::rename_object(
    const std::filesystem::path& source,
    const ObjectIdentity& expected,
    const std::string_view new_basename) {
    const auto operation_id = next_operation_id();
    if (!mutations_enabled_) {
        auto value = result(OperationKind::rename_object,
                            OperationTerminal::unavailable,
                            "mutations_disabled",
                            "mutations require explicit protected-profile opt-in",
                            source);
        value.operation_id = operation_id;
        return value;
    }
    if (const auto name_error = validate_basename(new_basename)) {
        auto value = result(OperationKind::rename_object,
                            OperationTerminal::failed,
                            "invalid_name", *name_error, source);
        value.operation_id = operation_id;
        return value;
    }
    if (const auto source_error = validate_source(source, expected)) {
        const auto terminal = *source_error == identity_changed_message()
            ? OperationTerminal::conflict : OperationTerminal::failed;
        auto value = result(OperationKind::rename_object, terminal,
                            terminal == OperationTerminal::conflict
                                ? "identity_changed" : "invalid_source",
                            *source_error, source, {}, expected);
        value.operation_id = operation_id;
        return value;
    }
    const auto destination = source.parent_path() / path_from_utf8(new_basename);
    if (destination_exists_no_follow(destination)) {
        auto value = result(OperationKind::rename_object,
                            OperationTerminal::conflict,
                            "destination_exists",
                            "rename never overwrites an existing destination",
                            source, destination, expected);
        value.operation_id = operation_id;
        return value;
    }
    std::error_code error;
    std::filesystem::rename(source, destination, error);
    if (error) {
        auto value = result(OperationKind::rename_object,
                            OperationTerminal::failed,
                            "rename_failed", error.message(),
                            source, destination, expected);
        value.operation_id = operation_id;
        return value;
    }
    const auto observed = observe_identity(destination);
    if (observed != expected) {
        auto value = result(OperationKind::rename_object,
                            OperationTerminal::failed,
                            "postcondition_failed",
                            "renamed object identity did not match the commit",
                            source, destination, observed);
        value.operation_id = operation_id;
        return value;
    }
    undo_ = UndoRecord{UndoKind::rename_back, destination, source, observed};
    auto value = result(OperationKind::rename_object, OperationTerminal::success,
                        "renamed", "object renamed", source, destination, observed);
    value.operation_id = operation_id;
    return value;
}

OperationResult FileOperationService::quarantine_object(
    const std::filesystem::path& source,
    const ObjectIdentity& expected) {
    const auto operation_id = next_operation_id();
    if (!mutations_enabled_) {
        auto value = result(OperationKind::quarantine_object,
                            OperationTerminal::unavailable,
                            "mutations_disabled",
                            "mutations require explicit protected-profile opt-in",
                            source);
        value.operation_id = operation_id;
        return value;
    }
    if (const auto source_error = validate_source(source, expected)) {
        const auto terminal = *source_error == identity_changed_message()
            ? OperationTerminal::conflict : OperationTerminal::failed;
        auto value = result(OperationKind::quarantine_object, terminal,
                            terminal == OperationTerminal::conflict
                                ? "identity_changed" : "invalid_source",
                            *source_error, source, {}, expected);
        value.operation_id = operation_id;
        return value;
    }
    std::filesystem::path destination;
    try {
        destination = available_quarantine_path(source);
    } catch (const std::exception& error) {
        auto value = result(OperationKind::quarantine_object,
                            OperationTerminal::failed,
                            "quarantine_unavailable", error.what(),
                            source, {}, expected);
        value.operation_id = operation_id;
        return value;
    }
    std::error_code error;
    std::filesystem::rename(source, destination, error);
    if (error) {
        auto value = result(OperationKind::quarantine_object,
                            OperationTerminal::failed,
                            error == std::errc::cross_device_link
                                ? "cross_volume_unsupported" : "quarantine_failed",
                            error.message(), source, destination, expected);
        value.operation_id = operation_id;
        return value;
    }
    const auto observed = observe_identity(destination);
    if (observed != expected) {
        auto value = result(OperationKind::quarantine_object,
                            OperationTerminal::failed,
                            "postcondition_failed",
                            "quarantined object identity did not match the commit",
                            source, destination, observed);
        value.operation_id = operation_id;
        return value;
    }
    undo_ = UndoRecord{UndoKind::restore_quarantined,
                       destination, source, observed};
    auto value = result(OperationKind::quarantine_object,
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
    const CancellationCheck& cancelled) {
    const auto operation_id = next_operation_id();
    if (!mutations_enabled_) {
        auto value = result(OperationKind::copy_object,
                            OperationTerminal::unavailable,
                            "mutations_disabled",
                            "mutations require explicit protected-profile opt-in",
                            source, destination_parent);
        value.operation_id = operation_id;
        return value;
    }
    if (const auto source_error = validate_source(source, expected)) {
        const auto terminal = *source_error == identity_changed_message()
            ? OperationTerminal::conflict : OperationTerminal::failed;
        auto value = result(OperationKind::copy_object, terminal,
                            terminal == OperationTerminal::conflict
                                ? "identity_changed" : "invalid_source",
                            *source_error, source, destination_parent, expected);
        value.operation_id = operation_id;
        return value;
    }
    if (const auto parent_error = validate_parent(destination_parent)) {
        auto value = result(OperationKind::copy_object,
                            OperationTerminal::failed,
                            "invalid_destination", *parent_error,
                            source, destination_parent, expected);
        value.operation_id = operation_id;
        return value;
    }
    if (expected.type == std::filesystem::file_type::directory &&
        path_is_within(source, destination_parent.lexically_normal())) {
        auto value = result(OperationKind::copy_object,
                            OperationTerminal::conflict,
                            "recursive_destination",
                            "a directory cannot be copied into itself",
                            source, destination_parent, expected);
        value.operation_id = operation_id;
        return value;
    }
    const auto destination = destination_parent / source.filename();
    if (destination_exists_no_follow(destination)) {
        auto value = result(OperationKind::copy_object,
                            OperationTerminal::conflict,
                            "destination_exists",
                            "copy never overwrites an existing destination",
                            source, destination, expected);
        value.operation_id = operation_id;
        return value;
    }
    const auto stage = destination_parent /
        (".fm-stage-" + operation_id + '-' + path_utf8(source.filename()));
    if (destination_exists_no_follow(stage)) {
        auto value = result(OperationKind::copy_object,
                            OperationTerminal::conflict,
                            "stage_exists",
                            "the exact copy stage already exists",
                            source, stage, expected);
        value.operation_id = operation_id;
        return value;
    }

    const auto copied = copy_node_no_follow(
        source, stage, expected.device, cancelled, injected_fault_);
    const auto stage_identity = observe_identity(stage);
    if (!copied.success) {
        const bool cleaned = !stage_identity.available() ||
            cleanup_stage(stage, stage_identity);
        auto value = result(OperationKind::copy_object,
                            copied.cancelled ? OperationTerminal::cancelled
                                             : OperationTerminal::failed,
                            copied.code, copied.message,
                            source, stage, expected);
        value.operation_id = operation_id;
        value.recoverable_object_retained = !cleaned;
        return value;
    }
    if (cancelled && cancelled()) {
        const bool cleaned = cleanup_stage(stage, stage_identity);
        auto value = result(OperationKind::copy_object,
                            OperationTerminal::cancelled,
                            "cancelled", "copy cancelled before publication",
                            source, stage, expected);
        value.operation_id = operation_id;
        value.recoverable_object_retained = !cleaned;
        return value;
    }
    if (!observe_identity(source).same_revision(expected)) {
        const bool cleaned = cleanup_stage(stage, stage_identity);
        auto value = result(OperationKind::copy_object,
                            OperationTerminal::conflict,
                            "identity_changed", identity_changed_message(),
                            source, stage, expected);
        value.operation_id = operation_id;
        value.recoverable_object_retained = !cleaned;
        return value;
    }
    if (injected_fault_) {
        if (const auto fault = injected_fault_(
                OperationFaultPoint::copy_before_publication, destination)) {
            const bool cleaned = cleanup_stage(stage, stage_identity);
            auto value = result(OperationKind::copy_object,
                                OperationTerminal::failed,
                                *fault == std::errc::no_space_on_device
                                    ? "disk_full" : "publication_fault",
                                fault->message(), source, stage, expected);
            value.operation_id = operation_id;
            value.recoverable_object_retained = !cleaned;
            return value;
        }
    }
    std::error_code error;
    std::filesystem::rename(stage, destination, error);
    if (error) {
        const bool cleaned = cleanup_stage(stage, stage_identity);
        auto value = result(OperationKind::copy_object,
                            OperationTerminal::failed,
                            "publication_failed", error.message(),
                            source, stage, expected);
        value.operation_id = operation_id;
        value.recoverable_object_retained = !cleaned;
        return value;
    }
    const auto result_identity = observe_identity(destination);
    undo_.reset();
    auto value = result(OperationKind::copy_object, OperationTerminal::success,
                        "copied", "object copied with staged publication",
                        source, destination, result_identity);
    value.operation_id = operation_id;
    return value;
}

OperationResult FileOperationService::move_object(
    const std::filesystem::path& source,
    const ObjectIdentity& expected,
    const std::filesystem::path& destination_parent) {
    const auto operation_id = next_operation_id();
    if (!mutations_enabled_) {
        auto value = result(OperationKind::move_object,
                            OperationTerminal::unavailable,
                            "mutations_disabled",
                            "mutations require explicit protected-profile opt-in",
                            source, destination_parent);
        value.operation_id = operation_id;
        return value;
    }
    if (const auto source_error = validate_source(source, expected)) {
        const auto terminal = *source_error == identity_changed_message()
            ? OperationTerminal::conflict : OperationTerminal::failed;
        auto value = result(OperationKind::move_object, terminal,
                            terminal == OperationTerminal::conflict
                                ? "identity_changed" : "invalid_source",
                            *source_error, source, destination_parent, expected);
        value.operation_id = operation_id;
        return value;
    }
    if (const auto parent_error = validate_parent(destination_parent)) {
        auto value = result(OperationKind::move_object,
                            OperationTerminal::failed,
                            "invalid_destination", *parent_error,
                            source, destination_parent, expected);
        value.operation_id = operation_id;
        return value;
    }
    if (expected.type == std::filesystem::file_type::directory &&
        path_is_within(source, destination_parent.lexically_normal())) {
        auto value = result(OperationKind::move_object,
                            OperationTerminal::conflict,
                            "recursive_destination",
                            "a directory cannot be moved into itself",
                            source, destination_parent, expected);
        value.operation_id = operation_id;
        return value;
    }
    const auto destination = destination_parent / source.filename();
    if (destination == source) {
        auto value = result(OperationKind::move_object,
                            OperationTerminal::conflict,
                            "same_destination",
                            "source already has that destination",
                            source, destination, expected);
        value.operation_id = operation_id;
        return value;
    }
    if (destination_exists_no_follow(destination)) {
        auto value = result(OperationKind::move_object,
                            OperationTerminal::conflict,
                            "destination_exists",
                            "move never overwrites an existing destination",
                            source, destination, expected);
        value.operation_id = operation_id;
        return value;
    }
    const auto destination_identity = observe_identity(destination_parent);
    if (!destination_identity.available() ||
        destination_identity.device != expected.device) {
        auto value = result(OperationKind::move_object,
                            OperationTerminal::unavailable,
                            "cross_volume_unsupported",
                            "cross-volume move waits for staged copy and fault evidence",
                            source, destination, expected);
        value.operation_id = operation_id;
        return value;
    }
    std::error_code error;
    if (injected_fault_) {
        if (const auto fault = injected_fault_(
                OperationFaultPoint::move_before_publication, destination)) {
            auto value = result(
                OperationKind::move_object,
                *fault == std::errc::cross_device_link
                    ? OperationTerminal::unavailable
                    : OperationTerminal::failed,
                *fault == std::errc::cross_device_link
                    ? "cross_volume_unsupported" : "move_fault",
                fault->message(), source, destination, expected);
            value.operation_id = operation_id;
            return value;
        }
    }
    std::filesystem::rename(source, destination, error);
    if (error) {
        auto value = result(OperationKind::move_object,
                            OperationTerminal::failed,
                            "move_failed", error.message(),
                            source, destination, expected);
        value.operation_id = operation_id;
        return value;
    }
    const auto observed = observe_identity(destination);
    if (observed != expected) {
        auto value = result(OperationKind::move_object,
                            OperationTerminal::failed,
                            "postcondition_failed",
                            "moved object identity did not match the commit",
                            source, destination, observed);
        value.operation_id = operation_id;
        return value;
    }
    undo_ = UndoRecord{UndoKind::rename_back, destination, source, observed};
    auto value = result(OperationKind::move_object, OperationTerminal::success,
                        "moved", "object moved", source, destination, observed);
    value.operation_id = operation_id;
    return value;
}

OperationResult FileOperationService::undo_last() {
    const auto operation_id = next_operation_id();
    if (!mutations_enabled_) {
        auto value = result(OperationKind::undo, OperationTerminal::unavailable,
                            "mutations_disabled",
                            "mutations require explicit protected-profile opt-in");
        value.operation_id = operation_id;
        return value;
    }
    if (!undo_) {
        auto value = result(OperationKind::undo, OperationTerminal::unavailable,
                            "nothing_to_undo", "no operation is available to undo");
        value.operation_id = operation_id;
        return value;
    }
    const auto record = *undo_;
    if (observe_identity(record.current_path) != record.identity) {
        auto value = result(OperationKind::undo, OperationTerminal::conflict,
                            "identity_changed", identity_changed_message(),
                            record.current_path, record.original_path,
                            record.identity);
        value.operation_id = operation_id;
        return value;
    }
    std::error_code error;
    if (record.kind == UndoKind::remove_created_directory) {
        if (!std::filesystem::is_empty(record.current_path, error) || error) {
            auto value = result(OperationKind::undo, OperationTerminal::conflict,
                                "created_folder_not_empty",
                                error ? error.message()
                                      : "created folder is no longer empty",
                                record.current_path, {}, record.identity);
            value.operation_id = operation_id;
            return value;
        }
        if (!std::filesystem::remove(record.current_path, error) || error) {
            auto value = result(OperationKind::undo, OperationTerminal::failed,
                                "undo_remove_failed",
                                error ? error.message() : "created folder was not removed",
                                record.current_path, {}, record.identity);
            value.operation_id = operation_id;
            return value;
        }
        undo_.reset();
        auto value = result(OperationKind::undo, OperationTerminal::success,
                            "undone", "created folder removed",
                            record.current_path, {}, record.identity);
        value.operation_id = operation_id;
        return value;
    }
    if (destination_exists_no_follow(record.original_path)) {
        auto value = result(OperationKind::undo, OperationTerminal::conflict,
                            "destination_exists",
                            "undo never overwrites an occupied original path",
                            record.current_path, record.original_path,
                            record.identity);
        value.operation_id = operation_id;
        return value;
    }
    if (const auto parent_error = validate_parent(record.original_path.parent_path())) {
        auto value = result(OperationKind::undo, OperationTerminal::failed,
                            "invalid_restore_parent", *parent_error,
                            record.current_path, record.original_path,
                            record.identity);
        value.operation_id = operation_id;
        return value;
    }
    std::filesystem::rename(record.current_path, record.original_path, error);
    if (error) {
        auto value = result(OperationKind::undo, OperationTerminal::failed,
                            "undo_rename_failed", error.message(),
                            record.current_path, record.original_path,
                            record.identity);
        value.operation_id = operation_id;
        return value;
    }
    if (observe_identity(record.original_path) != record.identity) {
        auto value = result(OperationKind::undo, OperationTerminal::failed,
                            "postcondition_failed",
                            "restored object identity did not match the inverse",
                            record.current_path, record.original_path,
                            record.identity);
        value.operation_id = operation_id;
        return value;
    }
    undo_.reset();
    auto value = result(OperationKind::undo, OperationTerminal::success,
                        "undone", "operation undone",
                        record.current_path, record.original_path,
                        record.identity);
    value.operation_id = operation_id;
    return value;
}

} // namespace file_manager
