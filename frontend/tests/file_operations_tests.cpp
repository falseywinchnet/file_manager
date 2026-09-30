#include "fixture_links.hpp"
#include "file_manager/file_operations.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>

namespace {

class TestArea final {
public:
    TestArea() {
        base_ = std::filesystem::temp_directory_path() /
            ("file-manager-operations-" + std::to_string(::getpid()));
        std::error_code ignored;
        std::filesystem::remove_all(base_, ignored);
        std::filesystem::create_directories(base_ / "source");
        std::filesystem::create_directories(base_ / "quarantine");
        base_ = std::filesystem::canonical(base_);
    }

    ~TestArea() {
        std::error_code ignored;
        std::filesystem::remove_all(base_, ignored);
    }

    [[nodiscard]] std::filesystem::path source() const {
        return base_ / "source";
    }
    [[nodiscard]] std::filesystem::path quarantine() const {
        return base_ / "quarantine";
    }
    [[nodiscard]] std::filesystem::path outside() const {
        return base_ / "outside.txt";
    }

private:
    std::filesystem::path base_;
};

bool require(const bool condition, const char* message) {
    if (!condition) std::cerr << message << '\n';
    return condition;
}

void write_file(const std::filesystem::path& path, const std::string& value) {
    std::ofstream(path) << value;
}

bool has_copy_stage(const std::filesystem::path& parent) {
    for (const auto& entry : std::filesystem::directory_iterator(parent)) {
        if (entry.path().filename().string().starts_with(".fm-stage-")) {
            return true;
        }
    }
    return false;
}

} // namespace

int main() {
    TestArea area;
    file_manager::FileOperationService disabled(
        area.source(), area.quarantine(), false);
    const auto disabled_create = disabled.create_folder(area.source());
    if (!require(disabled_create.terminal ==
                     file_manager::OperationTerminal::unavailable &&
                 !std::filesystem::exists(area.source() / "New folder"),
                 "read-only default must refuse folder creation")) return 1;

    file_manager::FileOperationService operations(
        area.source(), area.quarantine(), true);

    const auto linked_root = area.source().parent_path() / "source-link";
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
    const auto created = operations.create_folder(area.source());
    if (!require(created.succeeded() && created.undo_available &&
                 std::filesystem::is_directory(created.resulting_path),
                 "create folder must publish an undoable directory")) return 1;
    const auto undo_create = operations.undo_last();
    if (!require(undo_create.succeeded() && !undo_create.undo_available &&
                 !std::filesystem::exists(created.resulting_path),
                 "undo must remove an unchanged empty created folder")) return 1;

    const auto nonempty = operations.create_folder(area.source());
    write_file(nonempty.resulting_path / "child.txt", "child");
    const auto blocked_empty_undo = operations.undo_last();
    if (!require(blocked_empty_undo.terminal ==
                     file_manager::OperationTerminal::conflict &&
                 operations.undo_available(),
                 "undo must preserve a created folder that became nonempty")) return 1;
    std::filesystem::remove(nonempty.resulting_path / "child.txt");
    if (!require(operations.undo_last().succeeded(),
                 "undo must remain retryable after an occupancy conflict")) return 1;

    const auto alpha = area.source() / "alpha.txt";
    const auto beta = area.source() / "beta.txt";
    write_file(alpha, "alpha");
    const auto alpha_identity = file_manager::observe_identity(alpha);
    const auto renamed = operations.rename_object(alpha, alpha_identity, "beta.txt");
    if (!require(renamed.succeeded() && renamed.resulting_path == beta &&
                 file_manager::observe_identity(beta) == alpha_identity,
                 "rename must retain no-follow filesystem identity")) return 1;
    if (!require(operations.undo_last().succeeded() &&
                 file_manager::observe_identity(alpha) == alpha_identity,
                 "rename undo must restore the same object")) return 1;

    const auto copy_destination = area.source() / "copy-destination";
    const auto move_destination = area.source() / "move-destination";
    std::filesystem::create_directory(copy_destination);
    std::filesystem::create_directory(move_destination);
    const auto copied_file = operations.copy_object(
        alpha, alpha_identity, copy_destination);
    if (!require(copied_file.succeeded() && !copied_file.undo_available &&
                 std::filesystem::exists(copy_destination / "alpha.txt") &&
                 file_manager::observe_identity(copy_destination / "alpha.txt") !=
                     alpha_identity,
                 "file copy must stage and publish a distinct object")) return 1;
    const auto copy_collision = operations.copy_object(
        alpha, alpha_identity, copy_destination);
    if (!require(copy_collision.terminal ==
                     file_manager::OperationTerminal::conflict &&
                 copy_collision.code == "destination_exists",
                 "copy collision must not overwrite the published object")) return 1;

    const auto copy_tree = area.source() / "copy-tree";
    std::filesystem::create_directories(copy_tree / "nested");
    write_file(copy_tree / "nested" / "value.txt", "tree value");
    const bool copy_link_available = create_fixture_link(area.outside(), copy_tree / "outside-link");
    const auto copied_tree = operations.copy_object(
        copy_tree, file_manager::observe_identity(copy_tree), copy_destination);
    if (!require(copied_tree.succeeded() &&
                 std::filesystem::exists(copy_destination / "copy-tree" /
                                         "nested" / "value.txt") &&
                 (!copy_link_available || std::filesystem::is_symlink(copy_destination / "copy-tree" /
                                             "outside-link")),
                 "directory copy must preserve nested files and symlink leaves")) return 1;
    const auto recursive_copy = operations.copy_object(
        copy_tree, file_manager::observe_identity(copy_tree), copy_tree / "nested");
    if (!require(recursive_copy.terminal ==
                     file_manager::OperationTerminal::conflict &&
                 recursive_copy.code == "recursive_destination",
                 "copy must refuse a destination inside its source")) return 1;

    const auto cancelled_source = area.source() / "cancelled-source";
    std::filesystem::create_directory(cancelled_source);
    write_file(cancelled_source / "value.txt", "cancel");
    const auto cancelled_copy = operations.copy_object(
        cancelled_source, file_manager::observe_identity(cancelled_source),
        copy_destination, [] { return true; });
    if (!require(cancelled_copy.terminal ==
                     file_manager::OperationTerminal::cancelled &&
                 !std::filesystem::exists(copy_destination / "cancelled-source"),
                 "cancelled copy must not publish a destination")) return 1;

    const auto partial_cancel_source = area.source() / "partial-cancel-source";
    std::filesystem::create_directory(partial_cancel_source);
    write_file(partial_cancel_source / "a.txt", "a");
    write_file(partial_cancel_source / "b.txt", "b");
    write_file(partial_cancel_source / "c.txt", "c");
    std::size_t cancellation_checks = 0;
    const auto partial_cancel = operations.copy_object(
        partial_cancel_source,
        file_manager::observe_identity(partial_cancel_source), copy_destination,
        [&cancellation_checks] { return ++cancellation_checks >= 4; });
    if (!require(partial_cancel.terminal ==
                     file_manager::OperationTerminal::cancelled &&
                 cancellation_checks >= 4 &&
                 !std::filesystem::exists(copy_destination /
                                          "partial-cancel-source") &&
                 !has_copy_stage(copy_destination),
                 "mid-traversal cancellation must clean a partial stage")) return 1;

    file_manager::FileOperationService permission_fault(
        area.source(), area.quarantine(), true,
        [](const file_manager::OperationFaultPoint point,
           const std::filesystem::path&)
            -> std::optional<std::error_code> {
            if (point == file_manager::OperationFaultPoint::
                             create_before_publication) {
                return std::make_error_code(std::errc::permission_denied);
            }
            return std::nullopt;
        });
    const auto denied_create = permission_fault.create_folder(area.source());
    if (!require(denied_create.terminal ==
                     file_manager::OperationTerminal::failed &&
                 denied_create.code == "permission_denied" &&
                 !std::filesystem::exists(area.source() / "New folder"),
                 "injected permission denial must publish no folder")) return 1;

    const auto disk_full_source = area.source() / "disk-full-source";
    const auto disk_full_destination = area.source() / "disk-full-destination";
    std::filesystem::create_directory(disk_full_source);
    std::filesystem::create_directory(disk_full_destination);
    write_file(disk_full_source / "a.txt", "a");
    write_file(disk_full_source / "b.txt", "b");
    std::size_t copied_nodes = 0;
    file_manager::FileOperationService disk_full_fault(
        area.source(), area.quarantine(), true,
        [&copied_nodes](const file_manager::OperationFaultPoint point,
                        const std::filesystem::path&)
            -> std::optional<std::error_code> {
            if (point == file_manager::OperationFaultPoint::copy_before_node &&
                ++copied_nodes == 3) {
                return std::make_error_code(std::errc::no_space_on_device);
            }
            return std::nullopt;
        });
    const auto disk_full_copy = disk_full_fault.copy_object(
        disk_full_source, file_manager::observe_identity(disk_full_source),
        disk_full_destination);
    if (!require(disk_full_copy.terminal ==
                     file_manager::OperationTerminal::failed &&
                 disk_full_copy.code == "disk_full" && copied_nodes == 3 &&
                 !std::filesystem::exists(disk_full_destination /
                                          "disk-full-source") &&
                 !has_copy_stage(disk_full_destination),
                 "injected disk-full must clean a partially copied stage")) return 1;

    const auto move_source = area.source() / "move-me.txt";
    write_file(move_source, "move");
    const auto move_identity = file_manager::observe_identity(move_source);
    const auto moved = operations.move_object(
        move_source, move_identity, move_destination);
    if (!require(moved.succeeded() &&
                 file_manager::observe_identity(move_destination / "move-me.txt") ==
                     move_identity && operations.undo_available(),
                 "same-volume move must preserve identity and expose undo")) return 1;
    if (!require(operations.undo_last().succeeded() &&
                 file_manager::observe_identity(move_source) == move_identity,
                 "move undo must restore the same object")) return 1;

    const auto cross_volume_source = area.source() / "cross-volume.txt";
    write_file(cross_volume_source, "cross-volume");
    file_manager::FileOperationService cross_volume_fault(
        area.source(), area.quarantine(), true,
        [](const file_manager::OperationFaultPoint point,
           const std::filesystem::path&)
            -> std::optional<std::error_code> {
            if (point == file_manager::OperationFaultPoint::
                             move_before_publication) {
                return std::make_error_code(std::errc::cross_device_link);
            }
            return std::nullopt;
        });
    const auto cross_volume_move = cross_volume_fault.move_object(
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
    const auto collision = operations.rename_object(
        alpha, alpha_identity, "beta.txt");
    if (!require(collision.terminal == file_manager::OperationTerminal::conflict &&
                 collision.code == "destination_exists" &&
                 std::filesystem::exists(alpha) && std::filesystem::exists(beta),
                 "rename collision must not overwrite either object")) return 1;
    std::filesystem::remove(beta);

    const auto old_alpha = area.source() / "old-alpha.txt";
    std::filesystem::rename(alpha, old_alpha);
    write_file(alpha, "replacement");
    const auto replaced = operations.rename_object(
        alpha, alpha_identity, "should-not-exist.txt");
    if (!require(replaced.terminal == file_manager::OperationTerminal::conflict &&
                 replaced.code == "identity_changed" &&
                 std::filesystem::exists(alpha) && std::filesystem::exists(old_alpha),
                 "replacement between observation and commit must fail closed")) return 1;
    std::filesystem::remove(alpha);
    std::filesystem::rename(old_alpha, alpha);

    write_file(area.outside(), "outside target");
    const auto link = area.source() / "outside-link";
    if (create_fixture_link(area.outside(), link)) {
    const auto link_identity = file_manager::observe_identity(link);
    const auto quarantined_link = operations.quarantine_object(link, link_identity);
    if (!require(quarantined_link.succeeded() &&
                 quarantined_link.recoverable_object_retained &&
                 std::filesystem::exists(area.outside()) &&
                 !std::filesystem::exists(link),
                 "quarantine must move a symlink leaf without following its target")) return 1;
    if (!require(operations.undo_last().succeeded() &&
                 std::filesystem::is_symlink(link) &&
                 std::filesystem::exists(area.outside()),
                 "undo must restore the symlink leaf and preserve its target")) return 1;

    }
    const auto tree = area.source() / "tree";
    std::filesystem::create_directories(tree / "nested");
    write_file(tree / "nested" / "value.txt", "value");
    const auto tree_identity = file_manager::observe_identity(tree);
    const auto quarantined_tree = operations.quarantine_object(tree, tree_identity);
    if (!require(quarantined_tree.succeeded() &&
                 std::filesystem::exists(quarantined_tree.resulting_path /
                                         "nested" / "value.txt"),
                 "quarantine must retain a directory tree without traversal copying")) return 1;
    if (!require(operations.undo_last().succeeded() &&
                 std::filesystem::exists(tree / "nested" / "value.txt"),
                 "directory quarantine undo must restore the exact tree")) return 1;

    const auto conflict_file = area.source() / "conflict.txt";
    write_file(conflict_file, "original");
    const auto conflict_identity = file_manager::observe_identity(conflict_file);
    const auto quarantined_conflict = operations.quarantine_object(
        conflict_file, conflict_identity);
    write_file(conflict_file, "new occupant");
    const auto occupied_undo = operations.undo_last();
    if (!require(occupied_undo.terminal ==
                     file_manager::OperationTerminal::conflict &&
                 std::filesystem::exists(quarantined_conflict.resulting_path),
                 "undo must retain quarantine when the original path is occupied")) return 1;
    std::filesystem::remove(conflict_file);
    if (!require(operations.undo_last().succeeded() &&
                 file_manager::observe_identity(conflict_file) == conflict_identity,
                 "quarantine undo must remain retryable after collision removal")) return 1;

    const auto older = area.source() / "older.txt";
    write_file(older, "older");
    const auto older_quarantine = operations.quarantine_object(
        older, file_manager::observe_identity(older));
    const auto newer_create = operations.create_folder(area.source());
    if (!require(newer_create.succeeded() && operations.undo_last().succeeded() &&
                 std::filesystem::exists(older_quarantine.resulting_path) &&
                 !operations.undo_available(),
                 "a new success must replace, not silently extend, one-step undo")) return 1;

    return 0;
}
