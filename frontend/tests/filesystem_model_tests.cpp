#include "fixture_links.hpp"
#include "file_manager/filesystem_model.hpp"
#include "../src/directory_name_order.hpp"

#include <algorithm>
#include <array>

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

namespace {

class TestRoot final {
public:
    TestRoot() {
        path_ = std::filesystem::temp_directory_path() /
            ("file-manager-frontend-model-" + std::to_string(::getpid()));
        alias_ = path_.parent_path() / (path_.filename().string() + "-alias");
        std::filesystem::create_directories(path_ / "Folder" / "Nested");
        std::ofstream(path_ / "alpha.txt") << "alpha";
        std::ofstream(path_ / "zeta.png") << "not an image";
        std::ofstream(path_ / "Folder" / "child.cpp") << "int child;";
        link_available = create_fixture_link("Folder", path_ / "Folder link", true);
        alias_available = create_fixture_link(path_, alias_, true);
    }

    TestRoot(const TestRoot&) = delete;
    TestRoot& operator=(const TestRoot&) = delete;

    bool link_available{};
    bool alias_available{};

    ~TestRoot() {
        std::error_code ignored{};
        std::filesystem::remove(alias_, ignored);
        std::filesystem::remove_all(path_, ignored);
    }

    [[nodiscard]] const std::filesystem::path& path() const noexcept {
        return path_;
    }

    [[nodiscard]] const std::filesystem::path& alias() const noexcept {
        return alias_;
    }

private:
    std::filesystem::path path_{};
    std::filesystem::path alias_{};
};

bool require(const bool condition, const char* message) {
    if (!condition) std::cerr << message << '\n';
    return condition;
}

// Synchronous enumeration borrows the counter; it never retains this predicate.
struct CancelAfterFirstCheck final {
    int& checks;
    bool operator()() const {
        ++checks;
        const bool cancelled = checks > 1;
        return cancelled;
    }
};

bool directory_order_cases() {
    // In-memory names permit equal-folded ties on case-insensitive filesystems.
    std::array<file_manager::DirectoryEntry, 9> entries{};
    const std::array<std::string_view, 9> input{
        "Ω.txt", "alpha", "beta", "é.txt", "Alpha", "Alph", "alpha-long", "z-folder", "A-folder"};
    for (std::size_t index = 0; index < entries.size(); ++index) {
        entries[index].name = input[index];
        entries[index].directory = index >= 7;
    }
    const std::array<std::string_view, 9> expected{
        "A-folder", "z-folder", "Alph", "Alpha", "alpha", "alpha-long", "beta", "é.txt", "Ω.txt"};
    std::sort(entries.begin(), entries.end(), file_manager::detail::DirectoryNameOrder{});
    for (std::size_t index = 0; index < entries.size(); ++index) {
        if (!require(entries[index].name == expected[index],
                     "name order must preserve folder priority, byte folding, exact ties, prefixes and UTF-8 bytes")) {
            return false;
        }
    }
    return true;
}

} // namespace

int main() {
    if (!directory_order_cases()) return 1;
    TestRoot root{};
    const std::filesystem::path canonical = file_manager::canonical_existing_directory(root.path());
    if (!require(file_manager::path_is_within(canonical, canonical / "Folder"),
                 "child path must be inside root")) return 1;
    if (!require(!file_manager::path_is_within(canonical, canonical.parent_path()),
                 "parent path must be outside root")) return 1;
    if (root.alias_available) {
    const std::optional<std::filesystem::path> rebased = file_manager::rebase_path_from_equivalent_root(
        canonical, root.alias() / "Folder" / "child.cpp");
    if (!require(rebased && *rebased == canonical / "Folder" / "child.cpp",
                 "an equivalent root spelling must rebase onto the protected root")) {
        return 1;
    }
    }
    if (!require(!file_manager::rebase_path_from_equivalent_root(
                     canonical, canonical.parent_path()),
                 "an unrelated ancestor must not rebase into the protected root")) {
        return 1;
    }

    const std::filesystem::path sibling_root = canonical.parent_path() /
        (canonical.filename().string() + "-sibling");
    std::filesystem::create_directories(sibling_root / "Mounted");
    const std::vector<std::filesystem::path> navigation_roots{
        canonical, sibling_root};
    const std::optional<file_manager::NavigationTarget> relative_target = file_manager::resolve_navigation_target(
        navigation_roots, canonical, canonical, "Folder/Nested");
    if (!require(relative_target && (*relative_target).root == canonical &&
                 (*relative_target).path == canonical / "Folder" / "Nested",
                 "relative navigation must resolve within the current admitted root")) {
        return 1;
    }
    const std::optional<file_manager::NavigationTarget> home_target = file_manager::resolve_navigation_target(
        navigation_roots, sibling_root, canonical, "~/Folder");
    if (!require(home_target && (*home_target).root == canonical &&
                 (*home_target).path == canonical / "Folder",
                 "tilde navigation must resolve against the admitted Home root")) {
        return 1;
    }
    const std::optional<file_manager::NavigationTarget> sibling_target = file_manager::resolve_navigation_target(
        navigation_roots, canonical, canonical, sibling_root / "Mounted");
    if (!require(sibling_target && (*sibling_target).root == sibling_root,
                 "navigation must switch to another explicitly admitted root")) {
        return 1;
    }
    if (!require(!file_manager::resolve_navigation_target(
                     navigation_roots, canonical, canonical,
                     canonical.parent_path()),
                 "navigation outside every admitted root must fail closed")) {
        return 1;
    }
    std::error_code cleanup_error{};
    std::filesystem::remove_all(sibling_root, cleanup_error);

    const file_manager::DirectorySnapshot snapshot = file_manager::read_directory(canonical, canonical, {}, 7);
    if (!require(snapshot.available(), "fixture root must enumerate")) return 1;
    if (!require(snapshot.generation == 7, "generation must be retained")) return 1;
    if (!require(snapshot.entries.size() == (root.link_available ? 4U : 3U), "fixture must expose four objects")) return 1;
    if (!require(snapshot.entries.front().name == "Folder" &&
                 snapshot.entries.front().directory,
                 "directories must sort before files")) return 1;

    const file_manager::DirectorySnapshot filtered = file_manager::read_directory(
        canonical, canonical, "ZETA", 8);
    if (!require(filtered.available() && filtered.entries.size() == 1 &&
                 filtered.entries.front().kind == file_manager::EntryKind::image,
                 "filter and extension classification must be deterministic")) return 1;

    const file_manager::DirectorySnapshot rejected = file_manager::read_directory(
        canonical, canonical.parent_path(), {}, 9);
    if (!require(!rejected.available() &&
                 rejected.error.find("outside the protected root") != std::string::npos,
                 "out-of-root navigation must fail closed")) return 1;

    if (root.link_available) {
    const file_manager::DirectorySnapshot symlink_rejected = file_manager::read_directory(
        canonical, canonical / "Folder link", {}, 10);
    if (!require(!symlink_rejected.available() &&
                 symlink_rejected.error.find("symbolic link") != std::string::npos,
                 "direct navigation must not traverse a symlink")) return 1;

    }
    int cancellation_checks = 0;
    const file_manager::DirectorySnapshot cancelled = file_manager::read_directory(
        canonical, canonical, {}, 11, CancelAfterFirstCheck{cancellation_checks});
    if (!require(cancelled.cancelled && !cancelled.available() &&
                 cancelled.entries.empty(),
                 "cancelled enumeration must not publish a partial snapshot")) return 1;

    const file_manager::ObjectIdentity identity_before = file_manager::observe_identity(canonical / "alpha.txt");
    std::filesystem::rename(canonical / "alpha.txt", canonical / "renamed.txt");
    const file_manager::ObjectIdentity identity_after = file_manager::observe_identity(canonical / "renamed.txt");
    if (!require(identity_before.available() && identity_before == identity_after,
                 "no-follow filesystem identity must survive rename")) return 1;

    std::tm local_calendar{};
    local_calendar.tm_year = 124;
    local_calendar.tm_mon = 0;
    local_calendar.tm_mday = 15;
    local_calendar.tm_hour = 13;
    local_calendar.tm_min = 45;
    local_calendar.tm_sec = 30;
    local_calendar.tm_isdst = -1;
    const std::time_t local_seconds = std::mktime(&local_calendar);
    const std::filesystem::file_time_type modified = std::chrono::file_clock::from_sys(
        std::chrono::system_clock::from_time_t(local_seconds));
    std::filesystem::last_write_time(canonical / "renamed.txt", modified);
    const file_manager::DirectorySnapshot dated = file_manager::read_directory(
        canonical, canonical, "renamed.txt", 12);
    if (!require(dated.available() && dated.entries.size() == 1U &&
                 dated.entries.front().modified_text == "2024-01-15 13:45",
                 "Modified must show local calendar time rather than filesystem epoch ticks")) return 1;

    if (!require(file_manager::format_bytes(1536) == "1.5 KB",
                 "byte formatting must be stable")) return 1;
    return 0;
}
