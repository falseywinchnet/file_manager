#include "fixture_links.hpp"
#include "file_manager/filesystem_model.hpp"
#include "../src/directory_name_order.hpp"
#include "../src/native_file.hpp"

#include <algorithm>
#include <array>

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
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

void check_metadata(const bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

bool same_time(const file_manager::ObservedFileTime& left,
               const file_manager::ObservedFileTime& right) {
    const bool same = left.unix_seconds == right.unix_seconds && left.nanoseconds == right.nanoseconds;
    return same;
}

void timestamp_decoder_cases() {
    struct WindowsTimeCase final {
        std::uint64_t ticks{};
        file_manager::ObservedFileTime expected{};
    };
    const std::array<WindowsTimeCase, 5U> cases{{
        {0U, {-11'644'473'600LL, 0U}},
        {116444735999999999ULL, {-1, 999999900U}},
        {116444736000000000ULL, {0, 0U}},
        {116444736012345678ULL, {1, 234567800U}},
        {std::numeric_limits<std::uint64_t>::max(), {1'833'029'933'770LL, 955161500U}},
    }};
    for (const WindowsTimeCase& example : cases) {
        const file_manager::ObservedFileTime time = file_manager::decode_windows_file_time(example.ticks);
        check_metadata(same_time(time, example.expected), "Windows factual time must not wrap before epoch or at max ticks");
    }
    const std::optional<file_manager::ObservedFileTime> negative = file_manager::decode_native_time(-1, 999999999);
    check_metadata(negative && same_time(*negative, {-1, 999999999U}), "negative POSIX fraction uses floor seconds");
    check_metadata(!file_manager::decode_native_time(0, -1) &&
        !file_manager::decode_native_time(0, 1'000'000'000), "invalid native fraction must be unavailable");
    const std::optional<file_manager::ObservedFileTime> minimum =
        file_manager::decode_native_time(std::numeric_limits<std::int64_t>::min(), 0);
    const std::optional<file_manager::ObservedFileTime> maximum =
        file_manager::decode_native_time(std::numeric_limits<std::int64_t>::max(), 999999999);
    check_metadata(minimum && maximum, "signed seconds must retain their full representation without multiplying");
    check_metadata(file_manager::format_modified_time(*minimum) == "Unavailable" &&
        file_manager::format_modified_time(*maximum) == "Unavailable", "unsupported calendar range must not clamp or wrap");
    check_metadata(file_manager::format_modified_time({0, 1'000'000'000U}) == "Unavailable",
        "formatter must validate public time fraction");
    const std::string negative_text = file_manager::format_modified_time(*negative);
    std::cout << "Signed pre-epoch calendar: " << negative_text << '\n';
    check_metadata(negative_text == "Unavailable" || negative_text.starts_with("1969-") || negative_text.starts_with("1970-"),
        "supported negative calendar instant must not become an unsigned future date");
}

void native_record_cases() {
#if defined(_WIN32)
    BY_HANDLE_FILE_INFORMATION basic{};
    std::array<unsigned char, 16U> identifier{};
    identifier[0U] = 1U;
    identifier[8U] = 2U;
    basic.nFileSizeLow = 123U;
    constexpr std::uint64_t ticks = 116444735999999999ULL;
    basic.ftLastWriteTime.dwHighDateTime = static_cast<DWORD>(ticks >> 32U);
    basic.ftLastWriteTime.dwLowDateTime = static_cast<DWORD>(ticks & 0xffffffffU);
    const file_manager::NativeObjectObservation regular = file_manager::observation_from_windows_information(basic, 7U, identifier);
    check_metadata(!regular.error && regular.identity.inode == 1U && regular.identity.inode_high == 2U &&
        regular.facts.logical_size == 123U && regular.facts.modified &&
        same_time(*regular.facts.modified, {-1, 999999900U}), "Windows decoder must preserve full ID and factual signed time");
    check_metadata(regular.identity.modified_nanoseconds == std::numeric_limits<std::uint64_t>::max() - 99U,
        "Windows legacy pre-epoch fingerprint must retain its exact modulo bits");
    basic.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
    const file_manager::NativeObjectObservation directory = file_manager::observation_from_windows_information(basic, 7U, identifier);
    check_metadata(directory.identity.size == 123U && !directory.facts.logical_size,
        "native directory bytes must not become factual logical/aggregate size");
    basic.dwFileAttributes |= FILE_ATTRIBUTE_REPARSE_POINT;
    const file_manager::NativeObjectObservation reparse = file_manager::observation_from_windows_information(basic, 7U, identifier);
    check_metadata(reparse.identity.type == std::filesystem::file_type::symlink && !reparse.facts.logical_size,
        "all reparse kinds must remain no-follow objects without regular-file size");
    identifier = {};
    const file_manager::NativeObjectObservation no_id = file_manager::observation_from_windows_information(basic, 7U, identifier);
    check_metadata(no_id.error && !no_id.identity.available() && !no_id.facts.logical_size && !no_id.facts.modified,
        "unavailable native identity cannot publish factual metadata");
    const file_manager::NativeObjectObservation invalid_handle = file_manager::observation_from_handle(INVALID_HANDLE_VALUE);
    check_metadata(invalid_handle.error && !invalid_handle.identity.available() && !invalid_handle.facts.modified,
        "native handle query failure must return a complete failure");
#else
    struct stat native{};
    native.st_dev = 7;
    native.st_ino = 1;
    native.st_mode = S_IFREG;
    native.st_size = 123;
#if defined(__APPLE__)
    native.st_mtimespec = {-1, 999999999};
#else
    native.st_mtim = {-1, 999999999};
#endif
    const file_manager::NativeObjectObservation regular = file_manager::observation_from_stat(
        native, file_manager::NativeIdentityProjection::path);
    check_metadata(regular.facts.logical_size == 123U && regular.facts.modified &&
        same_time(*regular.facts.modified, {-1, 999999999U}) &&
        regular.identity.modified_nanoseconds == std::numeric_limits<std::uint64_t>::max(),
        "POSIX factual signed time and legacy fingerprint must remain distinct");
    native.st_size = -1;
    const file_manager::NativeObjectObservation negative_size = file_manager::observation_from_stat(
        native, file_manager::NativeIdentityProjection::path);
    check_metadata(!negative_size.facts.logical_size && negative_size.facts.modified &&
        negative_size.identity.size == std::numeric_limits<std::uint64_t>::max(),
        "negative native size clears factual size without changing legacy bits or valid time");
    native.st_size = 123;
#if defined(__APPLE__)
    native.st_mtimespec.tv_nsec = 1'000'000'000;
#else
    native.st_mtim.tv_nsec = 1'000'000'000;
#endif
    const file_manager::NativeObjectObservation invalid_time = file_manager::observation_from_stat(
        native, file_manager::NativeIdentityProjection::path);
    check_metadata(!invalid_time.facts.modified && invalid_time.facts.logical_size == 123U && invalid_time.identity.available(),
        "invalid native fraction must not erase valid size or identity");
    native.st_mode = S_IFIFO;
    const file_manager::NativeObjectObservation path = file_manager::observation_from_stat(
        native, file_manager::NativeIdentityProjection::path);
    const file_manager::NativeObjectObservation descriptor = file_manager::observation_from_stat(
        native, file_manager::NativeIdentityProjection::read_descriptor);
    check_metadata(path.identity.type == std::filesystem::file_type::fifo &&
        descriptor.identity.type == std::filesystem::file_type::unknown && !path.facts.logical_size,
        "existing POSIX special-type projection difference must be retained");
#endif
}

void write_metadata_fixture(const std::filesystem::path& path, const std::string_view text) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    check_metadata(stream.is_open(), "metadata fixture must open");
    stream.write(text.data(), static_cast<std::streamsize>(text.size()));
    stream.close();
    check_metadata(!stream.fail(), "metadata fixture write/close must succeed");
}

void set_metadata_fixture_time(const std::filesystem::path& path, const std::int64_t unix_seconds) {
#if defined(_WIN32)
    constexpr std::uint64_t epoch_seconds = 11'644'473'600ULL;
    constexpr std::uint64_t ticks_per_second = 10'000'000ULL;
    constexpr std::uint64_t maximum_seconds = std::numeric_limits<std::uint64_t>::max() / ticks_per_second - epoch_seconds;
    check_metadata(unix_seconds >= 0 && static_cast<std::uint64_t>(unix_seconds) <= maximum_seconds,
        "native fixture timestamp must be a supported nonnegative test instant");
    const std::uint64_t native_seconds = static_cast<std::uint64_t>(unix_seconds) + epoch_seconds;
    const std::uint64_t ticks = native_seconds * ticks_per_second;
    const FILETIME modified{static_cast<DWORD>(ticks & 0xffffffffU), static_cast<DWORD>(ticks >> 32U)};
    const HANDLE handle = CreateFileW(path.c_str(), FILE_WRITE_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
        FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    check_metadata(handle != INVALID_HANDLE_VALUE, "open generated timestamp fixture");
    // No throwing work between acquisition and close; assertions run afterward.
    const BOOL changed = SetFileTime(handle, nullptr, nullptr, &modified);
    const BOOL closed = CloseHandle(handle);
    check_metadata(changed && closed, "native fixture timestamp update and close must succeed");
#else
    const std::chrono::system_clock::time_point instant{std::chrono::seconds{unix_seconds}};
    const std::filesystem::file_time_type native_time = std::chrono::file_clock::from_sys(instant);
    std::filesystem::last_write_time(path, native_time);
#endif
}

struct RemoveEnumeratedEntry final {
    const std::filesystem::path& path;
    std::size_t& checks;
    bool operator()() const {
        ++checks;
        if (checks == 2U) {
            const bool removed = std::filesystem::remove(path);
            check_metadata(removed, "remove generated entry after iterator acquisition");
        }
        return false;
    }
};

void metadata_observation_cases(const std::filesystem::path& root) {
    const std::filesystem::path folder = root / "metadata-fixtures";
    std::filesystem::create_directory(folder);
    const std::filesystem::path file = folder / "facts.txt";
    const std::filesystem::path empty = folder / "empty.txt";
    write_metadata_fixture(file, "owned observation");
    write_metadata_fixture(empty, "");
    const file_manager::NativeObjectObservation before = file_manager::observe_native_object(file);
    file_manager::NativeReadFile reader(file);
    const file_manager::NativeObjectObservation opened = reader.observation();
    check_metadata(reader.available() && !before.error && before.identity.same_revision(opened.identity) &&
        before.identity.same_revision(reader.identity()) && before.facts.logical_size == 17U &&
        before.facts.modified && opened.facts.modified && same_time(*before.facts.modified, *opened.facts.modified),
        "path and descriptor must agree on unchanged object identity and metadata");
    const file_manager::NativeObjectObservation zero = file_manager::observe_native_object(empty);
    const file_manager::NativeObjectObservation directory = file_manager::observe_native_object(folder);
    check_metadata(zero.facts.logical_size == 0U && !directory.facts.logical_size && directory.facts.modified,
        "zero regular-file bytes are present; directory aggregate is absent");
    const file_manager::DirectorySnapshot listing = file_manager::read_directory(root, folder, {}, 21U);
    check_metadata(listing.available() && listing.entries.size() == 2U, "metadata fixtures must enumerate");
    for (const file_manager::DirectoryEntry& entry : listing.entries) {
        check_metadata(entry.metadata.logical_size == entry.identity.size && entry.metadata.modified &&
            entry.modified_text == file_manager::format_modified_time(*entry.metadata.modified),
            "row identity and formatted facts must derive from one observation");
    }
    write_metadata_fixture(file, "changed size");
    const file_manager::NativeObjectObservation changed = reader.observation();
    check_metadata(before.facts.logical_size == 17U && changed.facts.logical_size == 12U &&
        !before.identity.same_revision(changed.identity), "owned observations remain unchanged; descriptor re-observes writes");
    const std::filesystem::path retained = folder / "retained.txt";
    std::filesystem::rename(file, retained);
    write_metadata_fixture(file, "new path object");
    const file_manager::NativeObjectObservation replaced = file_manager::observe_native_object(file);
    check_metadata(reader.identity() == changed.identity && !(replaced.identity == changed.identity),
        "path replacement cannot switch the retained read descriptor");

    std::size_t links_run = 0U;
    std::size_t links_skipped = 0U;
    const std::filesystem::path link = folder / "file-link";
    const bool link_available = create_fixture_link("facts.txt", link);
    if (link_available) {
        ++links_run;
        set_metadata_fixture_time(file, 946684800);
        const file_manager::NativeObjectObservation target = file_manager::observe_native_object(file);
        const file_manager::NativeObjectObservation link_observed = file_manager::observe_native_object(link);
        check_metadata(link_observed.identity.type == std::filesystem::file_type::symlink &&
            !link_observed.facts.logical_size && link_observed.facts.modified && target.facts.modified &&
            !same_time(*link_observed.facts.modified, *target.facts.modified), "link facts must not follow target time or size");
        const file_manager::DirectorySnapshot link_listing = file_manager::read_directory(root, folder, "file-link", 22U);
        check_metadata(link_listing.entries.size() == 1U && link_listing.entries[0U].kind == file_manager::EntryKind::symlink &&
            link_listing.entries[0U].metadata.modified &&
            same_time(*link_listing.entries[0U].metadata.modified, *link_observed.facts.modified),
            "listed Modified must describe the link itself");
    } else ++links_skipped;
    const std::filesystem::path broken = folder / "broken-link.txt";
    const bool broken_available = create_fixture_link("absent-target", broken);
    if (broken_available) {
        ++links_run;
        const file_manager::NativeObjectObservation observed = file_manager::observe_native_object(broken);
        check_metadata(!observed.error && observed.identity.type == std::filesystem::file_type::symlink &&
            !observed.facts.logical_size, "dangling link is an observable link, never a regular-file extension");
    } else ++links_skipped;
    std::cout << "Metadata link fixtures run=" << links_run << " skipped=" << links_skipped << '\n';

    const std::filesystem::path disappearing_folder = root / "metadata-disappearing";
    std::filesystem::create_directory(disappearing_folder);
    const std::filesystem::path disappearing = disappearing_folder / "vanished.png";
    write_metadata_fixture(disappearing, "fixture");
    std::size_t checks = 0U;
    const file_manager::DirectorySnapshot unavailable = file_manager::read_directory(root, disappearing_folder, {}, 23U,
        RemoveEnumeratedEntry{disappearing, checks});
    check_metadata(unavailable.available() && unavailable.entries.size() == 1U,
        "enumerated entry that disappears must remain an unavailable row");
    const file_manager::DirectoryEntry& row = unavailable.entries[0U];
    check_metadata(!row.identity.available() && !row.metadata.logical_size && !row.metadata.modified &&
        row.kind == file_manager::EntryKind::other && !row.directory && row.secondary_text == "Unavailable" &&
        row.modified_text == "Unavailable" && !row.stable_id.empty(),
        "unavailable png-named row must not acquire image or regular-file authority");
    const file_manager::NativeObjectObservation missing = file_manager::observe_native_object(disappearing);
    check_metadata(missing.error && !missing.identity.available() && !missing.facts.logical_size && !missing.facts.modified,
        "missing path produces a complete native observation failure");
}

} // namespace

int main() {
    timestamp_decoder_cases();
    native_record_cases();
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
    set_metadata_fixture_time(canonical / "renamed.txt", static_cast<std::int64_t>(local_seconds));
    const file_manager::DirectorySnapshot dated = file_manager::read_directory(
        canonical, canonical, "renamed.txt", 12);
    if (dated.entries.size() == 1U && dated.entries.front().modified_text != "2024-01-15 13:45") {
        const file_manager::DirectoryEntry& entry = dated.entries.front();
        std::cerr << "Modified diagnostic text=" << entry.modified_text << " expected_seconds=" << local_seconds;
        if (entry.metadata.modified) std::cerr << " observed_seconds=" << (*entry.metadata.modified).unix_seconds;
        std::cerr << '\n';
    }
    if (!require(dated.available() && dated.entries.size() == 1U &&
                 dated.entries.front().modified_text == "2024-01-15 13:45",
                 "Modified must show local calendar time rather than filesystem epoch ticks")) return 1;

    if (!require(file_manager::format_bytes(1536) == "1.5 KB",
                 "byte formatting must be stable")) return 1;
    metadata_observation_cases(canonical);
    return 0;
}
