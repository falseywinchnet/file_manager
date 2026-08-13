#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace file_manager {

enum class EntryKind : std::uint8_t {
    folder,
    document,
    image,
    archive,
    audio,
    code,
    symlink,
    other,
};

struct ObjectIdentity final {
    std::uint64_t device{};
    std::uint64_t inode{};
    std::uint64_t size{};
    std::uint64_t modified_nanoseconds{};
    std::filesystem::file_type type{std::filesystem::file_type::none};

    [[nodiscard]] bool available() const noexcept { return inode != 0; }
    [[nodiscard]] bool same_revision(const ObjectIdentity& other) const noexcept {
        return *this == other && size == other.size &&
            modified_nanoseconds == other.modified_nanoseconds;
    }
    friend bool operator==(const ObjectIdentity& left,
                           const ObjectIdentity& right) noexcept {
        return left.device == right.device && left.inode == right.inode &&
            left.type == right.type;
    }
};

struct DirectoryEntry final {
    std::string stable_id;
    std::filesystem::path path;
    std::string name;
    std::string secondary_text;
    std::string modified_text;
    ObjectIdentity identity;
    EntryKind kind{EntryKind::other};
    bool directory{};
};

struct DirectorySnapshot final {
    std::filesystem::path root;
    std::filesystem::path location;
    std::vector<DirectoryEntry> entries;
    std::string error;
    std::uint64_t generation{};
    bool cancelled{};

    [[nodiscard]] bool available() const noexcept {
        return !cancelled && error.empty();
    }
};

struct NavigationTarget final {
    std::filesystem::path root;
    std::filesystem::path path;
};

using CancellationCheck = std::function<bool()>;

[[nodiscard]] std::filesystem::path canonical_existing_directory(
    const std::filesystem::path& path);
[[nodiscard]] bool path_is_within(const std::filesystem::path& root,
                                  const std::filesystem::path& candidate);
[[nodiscard]] std::optional<std::filesystem::path>
rebase_path_from_equivalent_root(
    const std::filesystem::path& canonical_root,
    const std::filesystem::path& candidate);
[[nodiscard]] bool path_route_has_symlink(
    const std::filesystem::path& canonical_root,
    const std::filesystem::path& candidate);
[[nodiscard]] std::optional<NavigationTarget> resolve_navigation_target(
    const std::vector<std::filesystem::path>& admitted_roots,
    const std::filesystem::path& current_location,
    const std::filesystem::path& home_root,
    const std::filesystem::path& requested);
[[nodiscard]] ObjectIdentity observe_identity(
    const std::filesystem::path& path);
[[nodiscard]] DirectorySnapshot read_directory(
    const std::filesystem::path& root,
    const std::filesystem::path& requested,
    std::string_view filter,
    std::uint64_t generation,
    const CancellationCheck& cancelled = {},
    bool show_hidden = false);
[[nodiscard]] std::string format_bytes(std::uintmax_t bytes);

} // namespace file_manager
