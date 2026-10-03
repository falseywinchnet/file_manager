#include "fixture_links.hpp"
#include "copy_time_fixture.hpp"
#include "file_manager/file_operations.hpp"
#include "../src/native_file.hpp"
#include "../src/native_publication.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

class TestArea final {
public:
    TestArea() {
        parent_ = std::filesystem::canonical(std::filesystem::temp_directory_path());
        std::random_device random{};
        for (unsigned int attempt = 0; attempt < 128U; ++attempt) {
            const unsigned int token = random();
            base_ = parent_ / ("file-manager-operations-" + std::to_string(token));
            std::error_code error{};
            if (!std::filesystem::create_directory(base_, error)) {
                if (error && error != std::errc::file_exists) throw std::system_error(error);
                continue;
            }
            identity_ = file_manager::observe_identity(base_);
            if (!identity_.available()) throw std::runtime_error("fixture identity unavailable; directory retained");
            try {
                std::filesystem::create_directory(base_ / "source");
                std::filesystem::create_directory(base_ / "quarantine");
            } catch (...) {
                cleanup();
                throw;
            }
            return;
        }
        throw std::runtime_error("exclusive operation fixture creation exhausted");
    }

    TestArea(const TestArea&) = delete;
    TestArea& operator=(const TestArea&) = delete;

    ~TestArea() { cleanup(); }

    [[nodiscard]] std::filesystem::path source() const {
        const std::filesystem::path result = base_ / "source";
        return result;
    }
    [[nodiscard]] std::filesystem::path quarantine() const {
        const std::filesystem::path result = base_ / "quarantine";
        return result;
    }
    [[nodiscard]] std::filesystem::path outside() const {
        const std::filesystem::path result = base_ / "outside.txt";
        return result;
    }

private:
    void cleanup() noexcept {
        try {
            if (!identity_.available() || !base_.is_absolute() || base_.parent_path() != parent_ ||
                std::filesystem::canonical(base_) != base_ ||
                file_manager::observe_identity(base_) != identity_) {
                std::cerr << "Operation fixture cleanup scope changed; retained\n";
                return;
            }
            std::error_code error{};
            std::filesystem::remove_all(base_, error);
            if (error) std::cerr << "Operation fixture cleanup failed: " << error.message() << '\n';
        } catch (...) { std::cerr << "Operation fixture cleanup failed; retained\n"; }
    }
    std::filesystem::path parent_{};
    std::filesystem::path base_{};
    file_manager::ObjectIdentity identity_{};
};

bool require(const bool condition, const char* message) {
    if (!condition) std::cerr << message << '\n';
    return condition;
}

void write_file(const std::filesystem::path& path, const std::string& value) {
    std::ofstream output(path);
    output << value;
}

std::filesystem::path link_target(const std::filesystem::path& path) {
    std::error_code error{};
    const file_manager::NativeSymlink link = file_manager::read_native_symlink(path, error);
    if (error) throw std::system_error(error, "read fixture link target");
    return link.target;
}

bool has_copy_stage(const std::filesystem::path& parent) {
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(parent)) {
        if (entry.path().filename().string().starts_with(".fm-stage-")) {
            return true;
        }
    }
    return false;
}

bool cancel_immediately() { return true; }

std::string read_fixture_text(const std::filesystem::path& path) {
    std::ifstream input{path};
    if (!input) throw std::runtime_error("cannot open publication fixture");
    std::string text{};
    std::getline(input, text);
    if (input.bad()) throw std::runtime_error("cannot read publication fixture");
    return text;
}

// Directly verify the primitive's no-replace contract with owned fixture data.
bool test_native_publication(const std::filesystem::path& root) {
    const std::filesystem::path parent = root / "native-publication";
    std::filesystem::create_directory(parent);
    const std::filesystem::path source = parent / "source.txt";
    const std::filesystem::path destination = parent / "destination.txt";
    write_file(source, "source contents");
    write_file(destination, "destination contents");
    const file_manager::ObjectIdentity source_identity = file_manager::observe_identity(source);
    const file_manager::ObjectIdentity destination_identity = file_manager::observe_identity(destination);
    const std::error_code occupied = file_manager::check_destination_vacant(destination);
    if (!require(occupied == std::errc::file_exists, "existing destination must be occupied")) return false;
    const std::error_code refused = file_manager::rename_no_replace(source, destination);
    if (!require(refused == std::errc::file_exists &&
        file_manager::observe_identity(source) == source_identity &&
        file_manager::observe_identity(destination) == destination_identity &&
        read_fixture_text(source) == "source contents" &&
        read_fixture_text(destination) == "destination contents",
        "native no-replace publication must preserve both occupied regular files")) return false;

    const std::filesystem::path vacant = parent / "published.txt";
    const std::error_code vacancy = file_manager::check_destination_vacant(vacant);
    if (!require(!vacancy, "missing leaf under an existing parent must be vacant")) return false;
    const std::error_code published = file_manager::rename_no_replace(source, vacant);
    if (!require(!published && !std::filesystem::exists(source) &&
        file_manager::observe_identity(vacant) == source_identity &&
        read_fixture_text(vacant) == "source contents",
        "native no-replace publication must move the same file to a vacant leaf")) return false;

    const std::filesystem::path directory = parent / "source-directory";
    const std::filesystem::path occupied_directory = parent / "occupied-directory";
    const std::filesystem::path moved_directory = parent / "moved-directory";
    std::filesystem::create_directory(directory);
    std::filesystem::create_directory(occupied_directory);
    const std::filesystem::path marker = directory / "marker.txt";
    write_file(marker, "directory contents");
    const file_manager::ObjectIdentity occupied_directory_identity =
        file_manager::observe_identity(occupied_directory);
    const std::error_code directory_refusal = file_manager::rename_no_replace(directory, occupied_directory);
    if (!require(static_cast<bool>(directory_refusal) &&
        file_manager::observe_identity(occupied_directory) == occupied_directory_identity &&
        read_fixture_text(marker) == "directory contents",
        "native publication must not replace even an empty destination directory")) return false;
    const std::error_code directory_move = file_manager::rename_no_replace(directory, moved_directory);
    const std::filesystem::path moved_marker = moved_directory / "marker.txt";
    if (!require(!directory_move && !std::filesystem::exists(directory) &&
        read_fixture_text(moved_marker) == "directory contents",
        "directory publication must preserve the child without traversal copying")) return false;

    const std::filesystem::path invalid_child = destination / "child.txt";
    const std::error_code invalid_parent = file_manager::check_destination_vacant(invalid_child);
    if (!require(static_cast<bool>(invalid_parent) && invalid_parent != std::errc::file_exists,
        "a file used as a destination parent must be an observation failure, not vacancy")) return false;
    const std::filesystem::path missing_source = parent / "missing-source";
    const std::filesystem::path missing_result = parent / "missing-result";
    const std::error_code missing = file_manager::rename_no_replace(missing_source, missing_result);
    if (!require(static_cast<bool>(missing) && !std::filesystem::exists(missing_result),
        "failed publication must not fabricate a destination")) return false;

    const std::filesystem::path link = parent / "dangling-link";
    const std::filesystem::path link_target_path = parent / "absent-link-target";
    if (create_fixture_link(link_target_path, link, false)) {
        const file_manager::ObjectIdentity link_identity = file_manager::observe_identity(link);
        // The native reader preserves Windows extended-namespace spelling.
        // Compare the stored target across the move, not with its input spelling.
        const std::filesystem::path stored_link_target = link_target(link);
        if (!require(link_identity.available() &&
            link_identity.type == std::filesystem::file_type::symlink,
            "the source link fixture must have an observable leaf identity")) return false;
        const std::error_code link_occupied = file_manager::check_destination_vacant(link);
        const std::error_code link_refusal = file_manager::rename_no_replace(vacant, link);
        if (!require(link_occupied == std::errc::file_exists && static_cast<bool>(link_refusal) &&
            file_manager::observe_identity(link) == link_identity &&
            file_manager::observe_identity(vacant) == source_identity,
            "a dangling destination link must remain occupied and untouched")) return false;
        const std::filesystem::path moved_link = parent / "moved-link";
        const std::error_code link_move = file_manager::rename_no_replace(link, moved_link);
        if (link_move) {
            std::cerr << "source symlink publication failed: " << link_move.message() << '\n';
            return false;
        }
        if (!require(file_manager::observe_identity(moved_link) == link_identity,
            "publishing a source symlink must retain its leaf identity")) return false;
        const std::filesystem::path moved_link_target = link_target(moved_link);
        if (!require(moved_link_target == stored_link_target,
            "publishing a source symlink must preserve its stored target")) return false;
        const std::error_code old_link_status = file_manager::check_destination_vacant(link);
        if (!require(!old_link_status && !std::filesystem::exists(link_target_path),
            "source link publication must vacate the old name and leave its target absent")) return false;
    }
    return true;
}

// Service-owned callables borrow counters that are declared before their service.
struct CancelAfterFourChecks final {
    std::size_t& checks;
    bool operator()() const {
        ++checks;
        const bool cancelled = checks >= 4;
        return cancelled;
    }
};

// Synchronous cancellation observes only this test's newly created copy stage.
// Native identity observation sees an open Windows writer's current extent.
struct CancelAfterStageBytes final {
    const std::filesystem::path& parent;
    std::uint64_t& observed_bytes;
    bool operator()() const {
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(parent)) {
            const std::filesystem::path path = entry.path();
            const std::filesystem::path leaf = path.filename();
            const std::string name = leaf.string();
            if (!name.starts_with(".fm-stage-")) continue;
            const file_manager::ObjectIdentity identity = file_manager::observe_identity(path);
            if (identity.available() && identity.type == std::filesystem::file_type::regular && identity.size > 0) {
                observed_bytes = identity.size;
                return true;
            }
        }
        return false;
    }
};

bool test_copy_cancellation_after_bytes(const TestArea& area) {
    const std::filesystem::path root = area.source();
    const std::filesystem::path source = root / "cancel-after-bytes.txt";
    const std::filesystem::path destination = root / "cancel-after-bytes-destination";
    std::filesystem::create_directory(destination);
    const std::string contents(16U * 1024U * 1024U, 'x');
    write_file(source, contents);
    const file_manager::ObjectIdentity before = file_manager::observe_identity(source);
    file_manager::FileOperationService operations(root, area.quarantine(), true);
    std::uint64_t observed_bytes{};
    const file_manager::OperationResult cancelled = operations.copy_object(
        source, before, destination, CancelAfterStageBytes{destination, observed_bytes});
    if (!require(cancelled.terminal == file_manager::OperationTerminal::cancelled &&
        observed_bytes > 0 && !cancelled.recoverable_object_retained &&
        !std::filesystem::exists(destination / source.filename()) && !has_copy_stage(destination) &&
        file_manager::observe_identity(source).same_revision(before) && read_fixture_text(source) == contents,
        "cancellation after file bytes must clean the stage and preserve the source")) return false;
#if !defined(__APPLE__)
    if (!require(observed_bytes <= 256U * 1024U,
        "Windows/Linux cancellation must stop after the first bounded chunk")) return false;
#endif
    const file_manager::OperationResult retry = operations.copy_object(source, before, destination);
    const std::filesystem::path published = destination / source.filename();
    if (!require(retry.succeeded() && read_fixture_text(published) == contents &&
        !has_copy_stage(destination), "cancelled file copy must remain retryable")) return false;
    return true;
}

struct RecordCopyProgress final {
    std::vector<file_manager::CopyProgress>& records;
    void operator()(const file_manager::CopyProgress& progress) const {
        records.push_back(progress);
    }
};

enum class ObserverFailurePoint { bytes, finalizing, publishing };

struct FailCopyObserver final {
    ObserverFailurePoint point{ObserverFailurePoint::bytes};
    void operator()(const file_manager::CopyProgress& progress) const {
        const bool fail_bytes = point == ObserverFailurePoint::bytes && progress.copied_bytes > 0U;
        const bool fail_finalizing = point == ObserverFailurePoint::finalizing &&
            progress.phase == file_manager::CopyPhase::finalizing;
        const bool fail_publishing = point == ObserverFailurePoint::publishing &&
            progress.phase == file_manager::CopyPhase::publishing;
        if (fail_bytes || fail_finalizing || fail_publishing) throw std::runtime_error("fixture observer failure");
    }
};

struct CancelAtPublication final {
    bool& requested;
    void operator()(const file_manager::CopyProgress& progress) const {
        if (progress.phase == file_manager::CopyPhase::publishing) requested = true;
    }
};

struct CopyCancellationFlag final {
    const bool& requested;
    bool operator()() const { return requested; }
};

bool test_copy_progress(const TestArea& area) {
    using file_manager::CopyPhase;
    using file_manager::CopyProgress;
    const std::filesystem::path root = area.source();
    const std::filesystem::path tree = root / "progress-tree";
    const std::filesystem::path nested = tree / "nested";
    const std::filesystem::path destination = root / "progress-destination";
    std::filesystem::create_directories(nested);
    std::filesystem::create_directory(destination);
    const std::filesystem::path first = tree / "first.txt";
    const std::filesystem::path second = nested / "second.txt";
    const std::string first_contents(600U * 1024U, 'p');
    write_file(first, first_contents);
    write_file(second, "abc");
    file_manager::FileOperationService operations(root, area.quarantine(), true);
    std::vector<CopyProgress> records{};
    const file_manager::ObjectIdentity tree_identity = file_manager::observe_identity(tree);
    const file_manager::OperationResult copied = operations.copy_object(
        tree, tree_identity, destination, {}, RecordCopyProgress{records});
    if (!require(copied.succeeded() && !records.empty(), "tree copy must report progress and publish")) return false;
    std::uint64_t previous_bytes{};
    std::uint64_t previous_objects{};
    bool saw_finalizing{};
    for (const CopyProgress& progress : records) {
        if (!require(!progress.total_bytes && progress.copied_bytes >= previous_bytes &&
            progress.completed_objects >= previous_objects &&
            file_manager::path_is_within(tree, progress.current_source),
            "tree progress must be monotone with unknown total and a source inside its traversal")) return false;
        previous_bytes = progress.copied_bytes;
        previous_objects = progress.completed_objects;
        if (progress.phase == CopyPhase::finalizing) saw_finalizing = true;
    }
    const CopyProgress& complete = records.back();
    const std::uint64_t expected_bytes = static_cast<std::uint64_t>(first_contents.size()) + 3U;
    if (!require(complete.copied_bytes == expected_bytes && complete.completed_objects == 4U &&
        complete.phase == CopyPhase::publishing && saw_finalizing,
        "tree progress must count both files and both directories without a preliminary scan")) return false;

    records.clear();
    const file_manager::ObjectIdentity first_identity = file_manager::observe_identity(first);
    const file_manager::OperationResult file_copy = operations.copy_object(
        first, first_identity, destination, {}, RecordCopyProgress{records});
    if (!require(file_copy.succeeded() && !records.empty(), "regular copy must report progress")) return false;
    for (const CopyProgress& progress : records) {
        if (!require(progress.total_bytes == first_identity.size &&
            progress.copied_bytes <= first_identity.size,
            "regular-file progress must retain its known extent")) return false;
    }
    const std::filesystem::path published_file = destination / "first.txt";
    if (!require(read_fixture_text(published_file) == first_contents,
        "progress reporting must preserve copied contents")) return false;

    const std::filesystem::path failure_destination = root / "progress-failure-destination";
    std::filesystem::create_directory(failure_destination);
    for (const ObserverFailurePoint point : {ObserverFailurePoint::bytes,
            ObserverFailurePoint::finalizing, ObserverFailurePoint::publishing}) {
        const file_manager::OperationResult failed = operations.copy_object(
            first, first_identity, failure_destination, {}, FailCopyObserver{point});
        if (!require(failed.terminal == file_manager::OperationTerminal::failed &&
            failed.code == "copy_callback_failed" && !failed.recoverable_object_retained &&
            !has_copy_stage(failure_destination) &&
            !std::filesystem::exists(failure_destination / "first.txt"),
            "observer failure must report failure and clean owned stages before publication")) return false;
    }
    bool cancel_requested{};
    const file_manager::OperationResult cancelled = operations.copy_object(
        first, first_identity, failure_destination, CopyCancellationFlag{cancel_requested},
        CancelAtPublication{cancel_requested});
    if (!require(cancelled.terminal == file_manager::OperationTerminal::cancelled &&
        !cancelled.recoverable_object_retained && !has_copy_stage(failure_destination) &&
        !std::filesystem::exists(failure_destination / "first.txt"),
        "cancellation observed at publication progress must still prevent publication")) return false;
    records.clear();
    const std::string changed_contents(first_contents.size() + 1U, 'q');
    write_file(first, changed_contents);
    const file_manager::OperationResult changed = operations.copy_object(
        first, first_identity, failure_destination, {}, RecordCopyProgress{records});
    if (!require(changed.terminal == file_manager::OperationTerminal::conflict &&
        changed.code == "identity_changed" && records.empty() && !has_copy_stage(failure_destination) &&
        !std::filesystem::exists(failure_destination / "first.txt"),
        "changed request revision must be refused before progress advertises a stale total")) return false;
    return true;
}

struct PublicationFault final {
    file_manager::OperationFaultPoint point;
    std::errc error;
    std::optional<std::error_code> operator()(
        const file_manager::OperationFaultPoint requested,
        const std::filesystem::path&) const {
        if (requested != point) return std::nullopt;
        const std::error_code result = std::make_error_code(error);
        return result;
    }
};

struct DiskFullAfterThirdNode final {
    std::size_t& copied_nodes;
    std::optional<std::error_code> operator()(
        const file_manager::OperationFaultPoint point,
        const std::filesystem::path&) const {
        if (point != file_manager::OperationFaultPoint::copy_before_node) return std::nullopt;
        ++copied_nodes;
        if (copied_nodes != 3) return std::nullopt;
        const std::error_code result = std::make_error_code(std::errc::no_space_on_device);
        return result;
    }
};

bool test_ordinary_actions(const TestArea& area) {
    using file_manager::ObjectIdentity;
    using file_manager::OperationResult;
    using file_manager::OperationTerminal;
    const std::filesystem::path parent = area.source() / "ordinary";
    std::filesystem::create_directory(parent);
    // A generated repository-category location must not need a protected grant.
    const std::filesystem::path git_marker = parent / ".git";
    std::filesystem::create_directory(git_marker);
    const ObjectIdentity parent_identity = file_manager::observe_identity(parent);
    file_manager::FileOperationService ordinary(file_manager::OperationPolicy::ordinary_local);
    file_manager::FileOperationService readonly(file_manager::OperationPolicy::read_only);
    const OperationResult disabled = readonly.create_folder(parent, parent_identity);
    const OperationResult missing_parent = ordinary.create_folder(parent);
    if (!require(disabled.terminal == OperationTerminal::unavailable &&
                 missing_parent.code == "parent_identity_changed",
                 "ordinary requests must carry parent identity; explicit read-only refuses")) return false;
    const OperationResult created = ordinary.create_folder(parent, parent_identity);
    if (!require(created.succeeded() && created.undo_available && created.identity.available(),
                 "ordinary create must grant identity-checked undo without quarantine")) return false;
    const std::filesystem::path child = created.resulting_path / "child.txt";
    write_file(child, "retained");
    const OperationResult nonempty = ordinary.undo_last();
    if (!require(nonempty.code == "created_folder_not_empty" && nonempty.undo_available,
                 "ordinary undo must retain nonempty folders and retry authority")) return false;
    std::filesystem::remove(child);
    const OperationResult removed = ordinary.undo_last();
    if (!require(removed.succeeded(), "ordinary empty-folder undo must succeed")) return false;

    const std::filesystem::path source = parent / "source.txt";
    const std::filesystem::path renamed = parent / "renamed.txt";
    write_file(source, "ordinary source");
    const ObjectIdentity source_identity = file_manager::observe_identity(source);
    const OperationResult unchanged = ordinary.rename_object(source, source_identity, "source.txt", parent_identity);
    const OperationResult invalid = ordinary.rename_object(source, source_identity, "../escape", parent_identity);
    const OperationResult rename = ordinary.rename_object(source, source_identity, "renamed.txt", parent_identity);
    if (!require(unchanged.code == "name_unchanged" && invalid.code == "invalid_name" &&
                 rename.succeeded() && rename.identity == source_identity,
                 "ordinary rename must validate basename and preserve identity")) return false;
    write_file(source, "occupied");
    const OperationResult occupied = ordinary.undo_last();
    if (!require(occupied.code == "destination_exists" && occupied.undo_available,
                 "ordinary rename undo must refuse an occupied original name")) return false;
    std::filesystem::remove(source);
    const OperationResult restored = ordinary.undo_last();
    if (!require(restored.succeeded(), "ordinary rename undo must remain retryable")) return false;
    write_file(renamed, "occupied destination");
    const OperationResult collision = ordinary.rename_object(source, source_identity, "renamed.txt", parent_identity);
    const OperationResult copy = ordinary.copy_object(source, source_identity, area.quarantine());
    const OperationResult move = ordinary.move_object(source, source_identity, area.quarantine());
    const OperationResult erase = ordinary.quarantine_object(source, source_identity);
    if (!require(collision.code == "destination_exists" &&
                 copy.terminal == OperationTerminal::unavailable &&
                 move.terminal == OperationTerminal::unavailable &&
                 erase.terminal == OperationTerminal::unavailable,
                 "ordinary policy must not admit protected transfers or overwrite collisions")) return false;
    std::filesystem::remove(renamed);
    std::filesystem::rename(source, renamed);
    write_file(source, "replacement");
    const OperationResult stale_source = ordinary.rename_object(source, source_identity, "wrong.txt", parent_identity);
    if (!require(stale_source.code == "identity_changed", "ordinary rename must reject source replacement")) return false;

    const std::filesystem::path held_parent = area.source() / "ordinary-held";
    std::filesystem::rename(parent, held_parent);
    std::filesystem::create_directory(parent);
    const OperationResult stale_parent = ordinary.create_folder(parent, parent_identity);
    if (!require(stale_parent.code == "parent_identity_changed",
                 "ordinary create must reject parent replacement before execution")) return false;
    std::filesystem::remove(parent);
    std::filesystem::rename(held_parent, parent);
    const OperationResult undo_target = ordinary.create_folder(parent, parent_identity);
    if (!require(undo_target.succeeded(), "parent-undo fixture must create")) return false;
    std::filesystem::rename(parent, held_parent);
    std::filesystem::create_directory(parent);
    const OperationResult stale_undo = ordinary.undo_last();
    if (!require(stale_undo.code == "parent_identity_changed" && stale_undo.undo_available,
                 "ordinary folder undo must reject replaced parent before removal")) return false;
    std::filesystem::remove(parent);
    std::filesystem::rename(held_parent, parent);
    const OperationResult retry_undo = ordinary.undo_last();
    if (!require(retry_undo.succeeded(), "restoring the exact parent must permit undo retry")) return false;

    const std::filesystem::path root = parent.root_path();
    const ObjectIdentity root_identity = file_manager::observe_identity(root);
    const OperationResult root_rename = ordinary.rename_object(root, root_identity, "forbidden", root_identity);
    if (!require(root_rename.code == "invalid_source", "filesystem root rename must be refused")) return false;
    const std::filesystem::path linked_parent = area.source() / "ordinary-link";
    if (create_fixture_link(parent, linked_parent, true)) {
        const ObjectIdentity link_identity = file_manager::observe_identity(linked_parent);
        const OperationResult linked_create = ordinary.create_folder(linked_parent, link_identity);
        if (!require(linked_create.code == "invalid_parent", "link parent routes must be refused")) return false;
        const std::filesystem::path source_parent = area.source();
        const ObjectIdentity source_parent_identity = file_manager::observe_identity(source_parent);
        const OperationResult leaf_rename = ordinary.rename_object(linked_parent, link_identity,
            "ordinary-link-renamed", source_parent_identity);
        const OperationResult leaf_undo = ordinary.undo_last();
        if (!require(leaf_rename.succeeded() && leaf_undo.succeeded() && std::filesystem::exists(parent),
                     "ordinary link-leaf rename and undo must preserve the target")) return false;
    }
    return true;
}

} // namespace

int main() {
    TestArea area{};
    if (!test_ordinary_actions(area)) return 1;
    if (!test_native_publication(area.source())) return 1;
    if (!test_copy_cancellation_after_bytes(area)) return 1;
    if (!test_copy_progress(area)) return 1;
    file_manager::FileOperationService disabled(
        area.source(), area.quarantine(), false);
    const file_manager::OperationResult disabled_create = disabled.create_folder(area.source());
    if (!require(disabled_create.terminal ==
                     file_manager::OperationTerminal::unavailable &&
                 !std::filesystem::exists(area.source() / "New folder"),
                 "read-only default must refuse folder creation")) return 1;

    file_manager::FileOperationService operations(
        area.source(), area.quarantine(), true);

    const std::filesystem::path linked_root = area.source().parent_path() / "source-link";
    if (create_fixture_link(area.source(), linked_root, true)) {
    bool rejected_link_route = false;
    try {
        file_manager::FileOperationService rejected(
            linked_root, area.quarantine(), true);
    } catch (const std::invalid_argument&) {
        rejected_link_route = true;
    }
    if (!require(rejected_link_route,
                 "mutation profile must reject a symlink-routed root")) return 1;

    }
    const file_manager::OperationResult created = operations.create_folder(area.source());
    if (!require(created.succeeded() && created.undo_available &&
                 std::filesystem::is_directory(created.resulting_path),
                 "create folder must publish an undoable directory")) return 1;
    const file_manager::OperationResult undo_create = operations.undo_last();
    if (!require(undo_create.succeeded() && !undo_create.undo_available &&
                 !std::filesystem::exists(created.resulting_path),
                 "undo must remove an unchanged empty created folder")) return 1;

    const file_manager::OperationResult nonempty = operations.create_folder(area.source());
    write_file(nonempty.resulting_path / "child.txt", "child");
    const file_manager::OperationResult blocked_empty_undo = operations.undo_last();
    if (!require(blocked_empty_undo.terminal ==
                     file_manager::OperationTerminal::conflict &&
                 operations.undo_available(),
                 "undo must preserve a created folder that became nonempty")) return 1;
    std::filesystem::remove(nonempty.resulting_path / "child.txt");
    if (!require(operations.undo_last().succeeded(),
                 "undo must remain retryable after an occupancy conflict")) return 1;

    const std::filesystem::path alpha = area.source() / "alpha.txt";
    const std::filesystem::path beta = area.source() / "beta.txt";
    write_file(alpha, "alpha");
    if (!require(copy_time_fixture::set_modified(alpha), "set old regular-file fixture time")) return 1;
    const file_manager::ObjectIdentity alpha_identity = file_manager::observe_identity(alpha);
    const file_manager::OperationResult renamed = operations.rename_object(alpha, alpha_identity, "beta.txt");
    if (!require(renamed.succeeded() && renamed.resulting_path == beta &&
                 file_manager::observe_identity(beta) == alpha_identity,
                 "rename must retain no-follow filesystem identity")) return 1;
    if (!require(operations.undo_last().succeeded() &&
                 file_manager::observe_identity(alpha) == alpha_identity,
                 "rename undo must restore the same object")) return 1;

    const std::filesystem::path copy_destination = area.source() / "copy-destination";
    const std::filesystem::path move_destination = area.source() / "move-destination";
    std::filesystem::create_directory(copy_destination);
    std::filesystem::create_directory(move_destination);
    const file_manager::OperationResult copied_file = operations.copy_object(
        alpha, alpha_identity, copy_destination);
    if (!require(copied_file.succeeded() && !copied_file.undo_available &&
                 std::filesystem::exists(copy_destination / "alpha.txt") &&
                 file_manager::observe_identity(copy_destination / "alpha.txt") !=
                     alpha_identity,
                 "file copy must stage and publish a distinct object")) return 1;
    if (!require(copy_time_fixture::same_modified(alpha, copied_file.resulting_path),
                 "published regular copy must retain modification date")) return 1;
    const file_manager::OperationResult copy_collision = operations.copy_object(
        alpha, alpha_identity, copy_destination);
    if (!require(copy_collision.terminal ==
                     file_manager::OperationTerminal::conflict &&
                 copy_collision.code == "destination_exists",
                 "copy collision must not overwrite the published object")) return 1;

    const std::filesystem::path link_collision_parent = area.source() / "link-collision";
    std::filesystem::create_directory(link_collision_parent);
    const std::filesystem::path link_collision = link_collision_parent / alpha.filename();
    if (create_fixture_link(area.outside(), link_collision)) {
        const file_manager::ObjectIdentity before_link = file_manager::observe_identity(link_collision);
        const file_manager::OperationResult collision_result = operations.copy_object(
            alpha, alpha_identity, link_collision_parent);
        if (!require(collision_result.code == "destination_exists" &&
                     file_manager::observe_identity(link_collision) == before_link &&
                     !std::filesystem::exists(area.outside()),
                     "copy must preserve an occupied dangling-link destination")) return 1;
    }

    const std::filesystem::path copy_tree = area.source() / "copy-tree";
    std::filesystem::create_directories(copy_tree / "nested");
    write_file(copy_tree / "nested" / "value.txt", "tree value");
    const std::filesystem::path nested_source = copy_tree / "nested" / "value.txt";
    if (!require(copy_time_fixture::set_modified(nested_source), "set nested fixture time")) return 1;
    const bool copy_link_available = create_fixture_link(area.outside(), copy_tree / "outside-link");
    const bool directory_link_available = create_fixture_link(
        std::filesystem::path("missing-directory"), copy_tree / "directory-link", true);
    const file_manager::OperationResult copied_tree = operations.copy_object(
        copy_tree, file_manager::observe_identity(copy_tree), copy_destination);
    if (!copied_tree.succeeded()) {
        std::cerr << "directory copy failed: code=" << copied_tree.code
                  << " message=" << copied_tree.message << '\n';
    }
    if (!require(copied_tree.succeeded() &&
                 std::filesystem::exists(copy_destination / "copy-tree" /
                                         "nested" / "value.txt") &&
                 (!copy_link_available || file_manager::observe_identity(
                     copy_destination / "copy-tree" / "outside-link").type ==
                     std::filesystem::file_type::symlink),
                 "directory copy must preserve nested files and symlink leaves")) return 1;
    const std::filesystem::path nested_copy = copied_tree.resulting_path / "nested" / "value.txt";
    if (!require(copy_time_fixture::same_modified(nested_source, nested_copy),
                 "recursive copy must retain nested file modification dates")) return 1;
    if (copy_link_available && !require(
            link_target(copy_destination / "copy-tree" / "outside-link") ==
                link_target(copy_tree / "outside-link") &&
            !std::filesystem::exists(area.outside()),
            "copy must preserve a dangling file link target without creating it")) return 1;
    if (directory_link_available) {
        const std::filesystem::path copied_link = copy_destination / "copy-tree" / "directory-link";
        if (!require(file_manager::observe_identity(copied_link).type ==
                         std::filesystem::file_type::symlink &&
                     link_target(copied_link) == "missing-directory",
                     "copy must preserve a dangling relative directory link")) return 1;
#if defined(_WIN32)
        const DWORD attributes = GetFileAttributesW(copied_link.c_str());
        if (!require(attributes != INVALID_FILE_ATTRIBUTES &&
                     (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0,
                     "copied Windows directory link must retain its directory kind")) return 1;
#endif
    }
    const file_manager::OperationResult recursive_copy = operations.copy_object(
        copy_tree, file_manager::observe_identity(copy_tree), copy_tree / "nested");
    if (!require(recursive_copy.terminal ==
                     file_manager::OperationTerminal::conflict &&
                 recursive_copy.code == "recursive_destination",
                 "copy must refuse a destination inside its source")) return 1;

    const std::filesystem::path cancelled_source = area.source() / "cancelled-source";
    std::filesystem::create_directory(cancelled_source);
    write_file(cancelled_source / "value.txt", "cancel");
    const file_manager::OperationResult cancelled_copy = operations.copy_object(
        cancelled_source, file_manager::observe_identity(cancelled_source),
        copy_destination, cancel_immediately);
    if (!require(cancelled_copy.terminal ==
                     file_manager::OperationTerminal::cancelled &&
                 !std::filesystem::exists(copy_destination / "cancelled-source"),
                 "cancelled copy must not publish a destination")) return 1;

    const std::filesystem::path partial_cancel_source = area.source() / "partial-cancel-source";
    std::filesystem::create_directory(partial_cancel_source);
    write_file(partial_cancel_source / "a.txt", "a");
    write_file(partial_cancel_source / "b.txt", "b");
    write_file(partial_cancel_source / "c.txt", "c");
    std::size_t cancellation_checks = 0;
    const file_manager::OperationResult partial_cancel = operations.copy_object(
        partial_cancel_source,
        file_manager::observe_identity(partial_cancel_source), copy_destination,
        CancelAfterFourChecks{cancellation_checks});
    if (!require(partial_cancel.terminal ==
                     file_manager::OperationTerminal::cancelled &&
                 cancellation_checks >= 4 &&
                 !std::filesystem::exists(copy_destination /
                                          "partial-cancel-source") &&
                 !has_copy_stage(copy_destination),
                 "mid-traversal cancellation must clean a partial stage")) return 1;

    file_manager::FileOperationService permission_fault(
        area.source(), area.quarantine(), true,
        PublicationFault{file_manager::OperationFaultPoint::create_before_publication,
                         std::errc::permission_denied});
    const file_manager::OperationResult denied_create = permission_fault.create_folder(area.source());
    if (!require(denied_create.terminal ==
                     file_manager::OperationTerminal::failed &&
                 denied_create.code == "permission_denied" &&
                 !std::filesystem::exists(area.source() / "New folder"),
                 "injected permission denial must publish no folder")) return 1;

    const std::filesystem::path disk_full_source = area.source() / "disk-full-source";
    const std::filesystem::path disk_full_destination = area.source() / "disk-full-destination";
    std::filesystem::create_directory(disk_full_source);
    std::filesystem::create_directory(disk_full_destination);
    write_file(disk_full_source / "a.txt", "a");
    write_file(disk_full_source / "b.txt", "b");
    std::size_t copied_nodes = 0;
    file_manager::FileOperationService disk_full_fault(
        area.source(), area.quarantine(), true,
        DiskFullAfterThirdNode{copied_nodes});
    const file_manager::OperationResult disk_full_copy = disk_full_fault.copy_object(
        disk_full_source, file_manager::observe_identity(disk_full_source),
        disk_full_destination);
    if (!require(disk_full_copy.terminal ==
                     file_manager::OperationTerminal::failed &&
                 disk_full_copy.code == "disk_full" && copied_nodes == 3 &&
                 !std::filesystem::exists(disk_full_destination /
                                          "disk-full-source") &&
                 !has_copy_stage(disk_full_destination),
                 "injected disk-full must clean a partially copied stage")) return 1;

    const std::filesystem::path move_source = area.source() / "move-me.txt";
    write_file(move_source, "move");
    const file_manager::ObjectIdentity move_identity = file_manager::observe_identity(move_source);
    const file_manager::OperationResult moved = operations.move_object(
        move_source, move_identity, move_destination);
    if (!require(moved.succeeded() &&
                 file_manager::observe_identity(move_destination / "move-me.txt") ==
                     move_identity && operations.undo_available(),
                 "same-volume move must preserve identity and expose undo")) return 1;
    if (!require(operations.undo_last().succeeded() &&
                 file_manager::observe_identity(move_source) == move_identity,
                 "move undo must restore the same object")) return 1;

    const std::filesystem::path cross_volume_source = area.source() / "cross-volume.txt";
    write_file(cross_volume_source, "cross-volume");
    file_manager::FileOperationService cross_volume_fault(
        area.source(), area.quarantine(), true,
        PublicationFault{file_manager::OperationFaultPoint::move_before_publication,
                         std::errc::cross_device_link});
    const file_manager::OperationResult cross_volume_move = cross_volume_fault.move_object(
        cross_volume_source, file_manager::observe_identity(cross_volume_source),
        move_destination);
    if (!require(cross_volume_move.terminal ==
                     file_manager::OperationTerminal::unavailable &&
                 cross_volume_move.code == "cross_volume_unsupported" &&
                 std::filesystem::exists(cross_volume_source) &&
                 !std::filesystem::exists(move_destination /
                                          "cross-volume.txt"),
                 "cross-volume move refusal must preserve the source")) return 1;

    write_file(beta, "occupied");
    const file_manager::OperationResult collision = operations.rename_object(
        alpha, alpha_identity, "beta.txt");
    if (!require(collision.terminal == file_manager::OperationTerminal::conflict &&
                 collision.code == "destination_exists" &&
                 std::filesystem::exists(alpha) && std::filesystem::exists(beta),
                 "rename collision must not overwrite either object")) return 1;
    std::filesystem::remove(beta);

    const std::filesystem::path old_alpha = area.source() / "old-alpha.txt";
    std::filesystem::rename(alpha, old_alpha);
    write_file(alpha, "replacement");
    const file_manager::OperationResult replaced = operations.rename_object(
        alpha, alpha_identity, "should-not-exist.txt");
    if (!require(replaced.terminal == file_manager::OperationTerminal::conflict &&
                 replaced.code == "identity_changed" &&
                 std::filesystem::exists(alpha) && std::filesystem::exists(old_alpha),
                 "replacement between observation and commit must fail closed")) return 1;
    std::filesystem::remove(alpha);
    std::filesystem::rename(old_alpha, alpha);

    write_file(area.outside(), "outside target");
    const std::filesystem::path link = area.source() / "outside-link";
    if (create_fixture_link(area.outside(), link)) {
    const file_manager::ObjectIdentity link_identity = file_manager::observe_identity(link);
    const file_manager::OperationResult quarantined_link = operations.quarantine_object(link, link_identity);
    if (!require(quarantined_link.succeeded() &&
                 quarantined_link.recoverable_object_retained &&
                 std::filesystem::exists(area.outside()) &&
                 !std::filesystem::exists(link),
                 "quarantine must move a symlink leaf without following its target")) return 1;
    if (!require(operations.undo_last().succeeded() &&
                 file_manager::observe_identity(link) == link_identity &&
                 std::filesystem::exists(area.outside()),
                 "undo must restore the symlink leaf and preserve its target")) return 1;

    }
    const std::filesystem::path tree = area.source() / "tree";
    std::filesystem::create_directories(tree / "nested");
    write_file(tree / "nested" / "value.txt", "value");
    const file_manager::ObjectIdentity tree_identity = file_manager::observe_identity(tree);
    const file_manager::OperationResult quarantined_tree = operations.quarantine_object(tree, tree_identity);
    if (!require(quarantined_tree.succeeded() &&
                 std::filesystem::exists(quarantined_tree.resulting_path /
                                         "nested" / "value.txt"),
                 "quarantine must retain a directory tree without traversal copying")) return 1;
    if (!require(operations.undo_last().succeeded() &&
                 std::filesystem::exists(tree / "nested" / "value.txt"),
                 "directory quarantine undo must restore the exact tree")) return 1;

    const std::filesystem::path conflict_file = area.source() / "conflict.txt";
    write_file(conflict_file, "original");
    const file_manager::ObjectIdentity conflict_identity = file_manager::observe_identity(conflict_file);
    const file_manager::OperationResult quarantined_conflict = operations.quarantine_object(
        conflict_file, conflict_identity);
    write_file(conflict_file, "new occupant");
    const file_manager::OperationResult occupied_undo = operations.undo_last();
    if (!require(occupied_undo.terminal ==
                     file_manager::OperationTerminal::conflict &&
                 std::filesystem::exists(quarantined_conflict.resulting_path),
                 "undo must retain quarantine when the original path is occupied")) return 1;
    std::filesystem::remove(conflict_file);
    if (!require(operations.undo_last().succeeded() &&
                 file_manager::observe_identity(conflict_file) == conflict_identity,
                 "quarantine undo must remain retryable after collision removal")) return 1;

    const std::filesystem::path older = area.source() / "older.txt";
    write_file(older, "older");
    const file_manager::OperationResult older_quarantine = operations.quarantine_object(
        older, file_manager::observe_identity(older));
    const file_manager::OperationResult newer_create = operations.create_folder(area.source());
    const file_manager::OperationResult newer_undo = operations.undo_last();
    if (!require(newer_create.succeeded() && newer_undo.succeeded() &&
                 std::filesystem::exists(older_quarantine.resulting_path) &&
                 !operations.undo_available(),
                 "a new success must replace, not silently extend, one-step undo")) return 1;

    return 0;
}
