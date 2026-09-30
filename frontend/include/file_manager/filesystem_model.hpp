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

    std::uint64_t inode_high{}; // Upper 64 bits of Windows FILE_ID_128; zero on POSIX.

    [[nodiscard]] bool available() const noexcept {
        const bool identified = inode != 0 || inode_high != 0;
        return identified;
    }
    [[nodiscard]] bool same_revision(const ObjectIdentity& other) const noexcept {
        const bool same = *this == other && size == other.size &&
            modified_nanoseconds == other.modified_nanoseconds;
        return same;
    }
    friend bool operator==(const ObjectIdentity& left,
                           const ObjectIdentity& right) noexcept {
        const bool same = left.device == right.device && left.inode == right.inode &&
            left.inode_high == right.inode_high &&
            left.type == right.type;
        return same;
    }
};

struct DirectoryEntry final {
    std::string stable_id{};
    std::filesystem::path path{};
    std::string name{};
    std::string secondary_text{};
    std::string modified_text{};
    ObjectIdentity identity{};
    EntryKind kind{EntryKind::other};
    bool directory{};
};

struct DirectorySnapshot final {
    std::filesystem::path root{};
    std::filesystem::path location{};
    std::vector<DirectoryEntry> entries{};
    std::string error{};
    std::uint64_t generation{};
    bool cancelled{};

    [[nodiscard]] bool available() const noexcept {
        const bool ready = !cancelled && error.empty();
        return ready;
    }
};

struct NavigationTarget final {
    std::filesystem::path root{};
    std::filesystem::path path{};
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
[[nodiscard]] std::string format_modified_time(std::filesystem::file_time_type time);

} // namespace file_manager
