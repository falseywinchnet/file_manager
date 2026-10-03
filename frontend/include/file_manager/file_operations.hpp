#pragma once

#include "file_manager/filesystem_model.hpp"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

namespace file_manager {

enum class OperationKind : std::uint8_t {
    create_folder,
    rename_object,
    quarantine_object,
    copy_object,
    move_object,
    undo,
};

enum class OperationTerminal : std::uint8_t {
    success,
    cancelled,
    unavailable,
    conflict,
    failed,
};

struct OperationResult final {
    std::string operation_id{};
    OperationKind kind{OperationKind::create_folder};
    OperationTerminal terminal{OperationTerminal::failed};
    std::string code{};
    std::string message{};
    std::filesystem::path protected_root{};
    std::filesystem::path original_path{};
    std::filesystem::path resulting_path{};
    ObjectIdentity identity{};
    bool undo_available{};
    bool recoverable_object_retained{};

    [[nodiscard]] bool succeeded() const noexcept {
        const bool success = terminal == OperationTerminal::success;
        return success;
    }
};

enum class OperationFaultPoint : std::uint8_t {
    create_before_publication,
    copy_before_node,
    copy_before_publication,
    move_before_publication,
};

// Deterministic fault injection is constructor-owned and absent in product
// launches. It exists so the protected adapter's terminal/cleanup law can be
// proven without filling a physical volume or changing ambient permissions.
using OperationFaultCheck = std::function<std::optional<std::error_code>(
    OperationFaultPoint, const std::filesystem::path&)>;

class FileOperationService final {
public:
    FileOperationService(std::filesystem::path protected_root,
                         std::filesystem::path quarantine_root,
                         bool mutations_enabled,
                         OperationFaultCheck injected_fault = {});

    FileOperationService(const FileOperationService&) = delete;
    FileOperationService& operator=(const FileOperationService&) = delete;

    [[nodiscard]] const std::filesystem::path& protected_root() const noexcept {
        return protected_root_;
    }
    [[nodiscard]] const std::filesystem::path& quarantine_root() const noexcept {
        return quarantine_root_;
    }
    [[nodiscard]] bool mutations_enabled() const noexcept {
        return mutations_enabled_;
    }
    [[nodiscard]] bool undo_available() const noexcept {
        const bool available = undo_.has_value();
        return available;
    }

    [[nodiscard]] OperationResult create_folder(
        const std::filesystem::path& parent);
    [[nodiscard]] OperationResult rename_object(
        const std::filesystem::path& source,
        const ObjectIdentity& expected,
        std::string_view new_basename);
    [[nodiscard]] OperationResult quarantine_object(
        const std::filesystem::path& source,
        const ObjectIdentity& expected);
    [[nodiscard]] OperationResult copy_object(
        const std::filesystem::path& source,
        const ObjectIdentity& expected,
        const std::filesystem::path& destination_parent,
        const CancellationCheck& cancelled = {});
    [[nodiscard]] OperationResult move_object(
        const std::filesystem::path& source,
        const ObjectIdentity& expected,
        const std::filesystem::path& destination_parent);
    [[nodiscard]] OperationResult undo_last();

private:
    enum class UndoKind : std::uint8_t {
        rename_back,
        restore_quarantined,
        remove_created_directory,
    };

    struct UndoRecord final {
        UndoKind kind{UndoKind::rename_back};
        std::filesystem::path current_path{};
        std::filesystem::path original_path{};
        ObjectIdentity identity{};
    };

    [[nodiscard]] std::string next_operation_id();
    [[nodiscard]] OperationResult result(OperationKind kind,
                                         OperationTerminal terminal,
                                         std::string code,
                                         std::string message,
                                         std::filesystem::path original = {},
                                         std::filesystem::path resulting = {},
                                         ObjectIdentity identity = {}) const;
    [[nodiscard]] std::optional<std::string> validate_parent(
        const std::filesystem::path& parent) const;
    [[nodiscard]] std::optional<std::string> validate_source(
        const std::filesystem::path& source,
        const ObjectIdentity& expected) const;
    [[nodiscard]] static std::optional<std::string> validate_basename(
        std::string_view basename);
    [[nodiscard]] std::filesystem::path available_quarantine_path(
        const std::filesystem::path& source);

    std::filesystem::path protected_root_{};
    std::filesystem::path quarantine_root_{};
    bool mutations_enabled_{};
    OperationFaultCheck injected_fault_{};
    std::uint64_t next_id_{1};
    std::optional<UndoRecord> undo_{};
};

} // namespace file_manager
