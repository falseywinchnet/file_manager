#include "file_manager/filesystem_model.hpp"

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
        std::filesystem::create_directory_symlink("Folder", path_ / "Folder link");
        std::filesystem::create_directory_symlink(path_, alias_);
    }

    ~TestRoot() {
        std::error_code ignored;
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
    std::filesystem::path path_;
    std::filesystem::path alias_;
};

bool require(const bool condition, const char* message) {
    if (!condition) std::cerr << message << '\n';
    return condition;
}

} // namespace

int main() {
    TestRoot root;
    const auto canonical = file_manager::canonical_existing_directory(root.path());
    if (!require(file_manager::path_is_within(canonical, canonical / "Folder"),
                 "child path must be inside root")) return 1;
    if (!require(!file_manager::path_is_within(canonical, canonical.parent_path()),
                 "parent path must be outside root")) return 1;
    const auto rebased = file_manager::rebase_path_from_equivalent_root(
        canonical, root.alias() / "Folder" / "child.cpp");
    if (!require(rebased && *rebased == canonical / "Folder" / "child.cpp",
                 "an equivalent root spelling must rebase onto the protected root")) {
        return 1;
    }
    if (!require(!file_manager::rebase_path_from_equivalent_root(
                     canonical, canonical.parent_path()),
                 "an unrelated ancestor must not rebase into the protected root")) {
        return 1;
    }

    const auto sibling_root = canonical.parent_path() /
        (canonical.filename().string() + "-sibling");
    std::filesystem::create_directories(sibling_root / "Mounted");
    const std::vector<std::filesystem::path> navigation_roots{
        canonical, sibling_root};
    const auto relative_target = file_manager::resolve_navigation_target(
        navigation_roots, canonical, canonical, "Folder/Nested");
    if (!require(relative_target && relative_target->root == canonical &&
                 relative_target->path == canonical / "Folder" / "Nested",
                 "relative navigation must resolve within the current admitted root")) {
        return 1;
    }
    const auto home_target = file_manager::resolve_navigation_target(
        navigation_roots, sibling_root, canonical, "~/Folder");
    if (!require(home_target && home_target->root == canonical &&
                 home_target->path == canonical / "Folder",
                 "tilde navigation must resolve against the admitted Home root")) {
        return 1;
    }
    const auto sibling_target = file_manager::resolve_navigation_target(
        navigation_roots, canonical, canonical, sibling_root / "Mounted");
    if (!require(sibling_target && sibling_target->root == sibling_root,
                 "navigation must switch to another explicitly admitted root")) {
        return 1;
    }
    if (!require(!file_manager::resolve_navigation_target(
                     navigation_roots, canonical, canonical,
                     canonical.parent_path()),
                 "navigation outside every admitted root must fail closed")) {
        return 1;
    }
    std::error_code cleanup_error;
    std::filesystem::remove_all(sibling_root, cleanup_error);

    const auto snapshot = file_manager::read_directory(canonical, canonical, {}, 7);
    if (!require(snapshot.available(), "fixture root must enumerate")) return 1;
    if (!require(snapshot.generation == 7, "generation must be retained")) return 1;
    if (!require(snapshot.entries.size() == 4, "fixture must expose four objects")) return 1;
    if (!require(snapshot.entries.front().name == "Folder" &&
                 snapshot.entries.front().directory,
                 "directories must sort before files")) return 1;

    const auto filtered = file_manager::read_directory(
        canonical, canonical, "ZETA", 8);
    if (!require(filtered.available() && filtered.entries.size() == 1 &&
                 filtered.entries.front().kind == file_manager::EntryKind::image,
                 "filter and extension classification must be deterministic")) return 1;

    const auto rejected = file_manager::read_directory(
        canonical, canonical.parent_path(), {}, 9);
    if (!require(!rejected.available() &&
                 rejected.error.find("outside the protected root") != std::string::npos,
                 "out-of-root navigation must fail closed")) return 1;

    const auto symlink_rejected = file_manager::read_directory(
        canonical, canonical / "Folder link", {}, 10);
    if (!require(!symlink_rejected.available() &&
                 symlink_rejected.error.find("symbolic link") != std::string::npos,
                 "direct navigation must not traverse a symlink")) return 1;

    int cancellation_checks = 0;
    const auto cancelled = file_manager::read_directory(
        canonical, canonical, {}, 11, [&cancellation_checks] {
            return ++cancellation_checks > 1;
        });
    if (!require(cancelled.cancelled && !cancelled.available() &&
                 cancelled.entries.empty(),
                 "cancelled enumeration must not publish a partial snapshot")) return 1;

    const auto identity_before = file_manager::observe_identity(canonical / "alpha.txt");
    std::filesystem::rename(canonical / "alpha.txt", canonical / "renamed.txt");
    const auto identity_after = file_manager::observe_identity(canonical / "renamed.txt");
    if (!require(identity_before.available() && identity_before == identity_after,
                 "no-follow filesystem identity must survive rename")) return 1;

    if (!require(file_manager::format_bytes(1536) == "1.5 KB",
                 "byte formatting must be stable")) return 1;
    return 0;
}
