#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#if defined(_WIN32)
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#include <winioctl.h>
#else
#include <cerrno>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <sys/xattr.h>
#endif

namespace {
namespace fs = std::filesystem;
constexpr std::uint64_t sparse_size = 8U * 1024U * 1024U;
constexpr std::string_view payload = "File Manager generated copy baseline\n";

void report(const std::string_view fixture, const std::string_view fact,
            const std::string_view value) {
    std::cout << std::quoted(std::string(fixture)) << '\t'
              << std::quoted(std::string(fact)) << '\t'
              << std::quoted(std::string(value)) << '\n';
}

void require(const bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

void write_bytes(const fs::path& path, const std::string_view bytes) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    stream.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    stream.close();
    require(static_cast<bool>(stream), "cannot write generated fixture");
}

enum class MetadataState { absent, present, read_error };

[[nodiscard]] const char* metadata_state_name(const MetadataState state) {
    switch (state) {
    case MetadataState::absent: return "absent";
    case MetadataState::present: return "present";
    case MetadataState::read_error: return "read-error";
    }
    return "invalid";
}

struct MetadataValue final {
    MetadataState state = MetadataState::absent;
    std::string value{};
};

// Each run owns an exclusively created directory. Cleanup is deliberately
// nonrecursive: validate the marker and exact direct children, restore write
// access, and remove only names registered by this owner.
class FixtureArea final {
public:
    FixtureArea() = default;
    ~FixtureArea() {
        if (!owned_ || cleanup_attempted_) return;
        try {
            cleanup();
        } catch (const std::exception& error) {
            report("run", "cleanup_failed_preserved_scope", error.what());
        }
    }

    void create() {
        require(!owned_, "fixture area already created");
        parent_ = fs::canonical(fs::temp_directory_path());
        std::random_device random{};
        for (unsigned int attempt = 0; attempt < 128U; ++attempt) {
            const std::string first = std::to_string(random());
            const std::string second = std::to_string(random());
            token_ = first + "-" + second;
            root_ = parent_ / ("file-manager-copy-metadata-" + token_);
            std::error_code error{};
            const bool created = fs::create_directory(root_, error);
            if (created) {
                owned_ = true;
                report("run", "owned_temp", root_.generic_string());
                write_bytes(root_ / "owner.txt", token_);
                return;
            }
            if (error && error != std::errc::file_exists) {
                throw fs::filesystem_error("create fixture directory", root_, error);
            }
        }
        throw std::runtime_error("exclusive temporary directory attempts exhausted");
    }
    FixtureArea(const FixtureArea&) = delete;
    FixtureArea& operator=(const FixtureArea&) = delete;

    [[nodiscard]] fs::path file(const std::string& name) {
        const fs::path leaf(name);
        require(leaf == leaf.filename() && leaf != "." && leaf != "..",
                "fixture name must be a direct child");
        const fs::path result = root_ / leaf;
        files_.push_back(result);
        return result;
    }

    void cleanup() {
        cleanup_attempted_ = true;
        require(root_.is_absolute() && root_.parent_path() == parent_ &&
                    root_.filename() == "file-manager-copy-metadata-" + token_,
                "cleanup scope mismatch");
        require(fs::symlink_status(root_).type() == fs::file_type::directory &&
                    fs::canonical(root_) == root_, "cleanup root changed");
        const fs::path marker = root_ / "owner.txt";
        require(fs::symlink_status(marker).type() == fs::file_type::regular,
                "cleanup marker type changed");
        std::ifstream stream(marker, std::ios::binary);
        std::string marker_token{};
        std::getline(stream, marker_token);
        stream.close();
        require(marker_token == token_, "cleanup marker mismatch");
        for (const fs::path& path : files_) {
            require(path.parent_path() == root_, "cleanup child escaped root");
            std::error_code error{};
            const fs::file_status status = fs::symlink_status(path, error);
            if (error == std::errc::no_such_file_or_directory ||
                status.type() == fs::file_type::not_found) continue;
            require(!error && status.type() == fs::file_type::regular,
                    "cleanup child is not an owned regular file");
            fs::permissions(path, fs::perms::owner_write, fs::perm_options::add);
            require(fs::remove(path), "cleanup file removal failed");
        }
        require(fs::remove(marker), "cleanup marker removal failed");
        require(fs::remove(root_), "cleanup root not empty; preserved unexpected contents");
        report("run", "cleanup", "verified exact owned scope; removed");
    }

private:
    fs::path parent_{};
    fs::path root_{};
    std::string token_{};
    std::vector<fs::path> files_{};
    bool owned_ = false;
    bool cleanup_attempted_ = false;
};

[[nodiscard]] bool equal_contents(const fs::path& source, const fs::path& destination) {
    std::ifstream left(source, std::ios::binary);
    std::ifstream right(destination, std::ios::binary);
    require(left.is_open() && right.is_open(), "content comparison open failed");
    std::vector<char> left_bytes(64U * 1024U, '\0');
    std::vector<char> right_bytes(64U * 1024U, '\0');
    for (;;) {
        left.read(left_bytes.data(), static_cast<std::streamsize>(left_bytes.size()));
        right.read(right_bytes.data(), static_cast<std::streamsize>(right_bytes.size()));
        require(!left.bad() && !right.bad(), "content comparison read failed");
        const std::streamsize count = left.gcount();
        if (count != right.gcount()) return false;
        for (std::streamsize index = 0; index < count; ++index) {
            const std::size_t position = static_cast<std::size_t>(index);
            if (left_bytes[position] != right_bytes[position]) return false;
        }
        if (left.eof() || right.eof()) {
            const bool both_ended = left.eof() && right.eof();
            return both_ended;
        }
        require(static_cast<bool>(left) && static_cast<bool>(right), "content read did not advance");
    }
}

void copy_and_report(const std::string& name, const fs::path& source,
                     const fs::path& destination) {
    const fs::perms source_permissions = fs::status(source).permissions();
    const fs::file_time_type source_time = fs::last_write_time(source);
    std::error_code error{};
    const bool copied = fs::copy_file(source, destination, error);
    report(name, "copy_error", error.message());
    require(copied && !error, "baseline copy_file failed: " + name);
    const bool contents_equal = equal_contents(source, destination);
    report(name, "contents_equal", contents_equal ? "yes" : "no");
    require(contents_equal, "baseline contents mismatch: " + name);
    const fs::perms destination_permissions = fs::status(destination).permissions();
    report(name, "source_permissions_decimal", std::to_string(static_cast<unsigned int>(source_permissions)));
    report(name, "destination_permissions_decimal", std::to_string(static_cast<unsigned int>(destination_permissions)));
    report(name, "permissions_equal", source_permissions == destination_permissions ? "yes" : "no");
    const fs::file_time_type destination_time = fs::last_write_time(destination);
    report(name, "source_mtime_clock_ticks", std::to_string(source_time.time_since_epoch().count()));
    report(name, "destination_mtime_clock_ticks", std::to_string(destination_time.time_since_epoch().count()));
    report(name, "mtime_equal", source_time == destination_time ? "yes" : "no");
}

#if defined(_WIN32)
[[nodiscard]] fs::path stream_path(const fs::path& path) {
    fs::path result = path;
    result += L":fm-copy-baseline";
    return result;
}

[[nodiscard]] MetadataValue read_metadata(const fs::path& path, const std::string&) {
    const fs::path alternate = stream_path(path);
    const HANDLE handle = CreateFileW(alternate.c_str(), GENERIC_READ,
        FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        const DWORD error = GetLastError();
        const bool absent = error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND;
        const MetadataValue result{absent ? MetadataState::absent : MetadataState::read_error, std::to_string(error)};
        return result;
    }
    std::array<char, 512> bytes{};
    DWORD count{};
    const BOOL read = ReadFile(handle, bytes.data(), static_cast<DWORD>(bytes.size()), &count, nullptr);
    const DWORD error = read ? ERROR_SUCCESS : GetLastError();
    const BOOL closed = CloseHandle(handle);
    require(closed != 0, "named-stream read handle close failed");
    if (!read || count == bytes.size()) {
        const MetadataValue result{MetadataState::read_error, std::to_string(error)};
        return result;
    }
    const MetadataValue result{MetadataState::present, std::string(bytes.data(), count)};
    return result;
}

[[nodiscard]] bool set_metadata(const fs::path& path, const std::string& name,
                                const std::string& value) {
    require(value.size() <= std::numeric_limits<DWORD>::max(), "named-stream payload too large");
    const fs::path alternate = stream_path(path);
    const HANDLE handle = CreateFileW(alternate.c_str(), GENERIC_WRITE, 0, nullptr,
        CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        report(name, "fixture_unavailable_windows_error", std::to_string(GetLastError()));
        return false;
    }
    DWORD count{};
    const BOOL written = WriteFile(handle, value.data(), static_cast<DWORD>(value.size()), &count, nullptr);
    const DWORD error = written ? ERROR_SUCCESS : GetLastError();
    const BOOL closed = CloseHandle(handle);
    require(closed != 0, "named-stream write handle close failed");
    require(written && count == value.size(), "named-stream fixture write failed: " + std::to_string(error));
    return true;
}
#else
[[nodiscard]] MetadataValue read_metadata(const fs::path& path, const std::string& name) {
    std::array<char, 512> bytes{};
#if defined(__APPLE__)
    const ssize_t count = getxattr(path.c_str(), name.c_str(), bytes.data(), bytes.size(), 0, XATTR_NOFOLLOW);
#else
    const ssize_t count = lgetxattr(path.c_str(), name.c_str(), bytes.data(), bytes.size());
#endif
    if (count < 0) {
        const int error = errno;
#if defined(__APPLE__)
        const bool absent = error == ENOATTR;
#else
        const bool absent = error == ENODATA;
#endif
        const MetadataValue result{absent ? MetadataState::absent : MetadataState::read_error, std::to_string(error)};
        return result;
    }
    const MetadataValue result{MetadataState::present, std::string(bytes.data(), static_cast<std::size_t>(count))};
    return result;
}

[[nodiscard]] bool set_metadata(const fs::path& path, const std::string& name,
                                const std::string& value) {
#if defined(__APPLE__)
    const int status = setxattr(path.c_str(), name.c_str(), value.data(), value.size(), 0, XATTR_CREATE | XATTR_NOFOLLOW);
#else
    const int status = lsetxattr(path.c_str(), name.c_str(), value.data(), value.size(), XATTR_CREATE);
#endif
    if (status != 0) {
        report(name, "fixture_unavailable_errno", std::to_string(errno));
        return false;
    }
    return true;
}
#endif

void metadata_fixture(FixtureArea& area, const std::string& label,
                      const std::string& attribute, const std::string& value) {
    const fs::path source = area.file(label + "-source");
    const fs::path destination = area.file(label + "-copy");
    write_bytes(source, payload);
    if (!set_metadata(source, attribute, value)) {
        report(label, "preservation", "not-measured: fixture capability unavailable");
        return;
    }
    const MetadataValue before = read_metadata(source, attribute);
    require(before.state == MetadataState::present && before.value == value, "metadata fixture readback failed");
    copy_and_report(label, source, destination);
    const MetadataValue after = read_metadata(destination, attribute);
    report(label, "source_metadata", before.value);
    report(label, "destination_metadata_state", metadata_state_name(after.state));
    report(label, "destination_metadata", after.value);
    if (after.state == MetadataState::read_error) throw std::runtime_error("destination metadata unreadable");
    const bool equal = after.state == MetadataState::present && before.value == after.value;
    report(label, "metadata_equal", equal ? "yes" : "no");
}

void allocation_fact(const std::string& label, const fs::path& path) {
    report(label, "logical_bytes", std::to_string(fs::file_size(path)));
#if defined(_WIN32)
    DWORD high{};
    SetLastError(ERROR_SUCCESS);
    const DWORD low = GetCompressedFileSizeW(path.c_str(), &high);
    const DWORD error = GetLastError();
    if (low == INVALID_FILE_SIZE && error != ERROR_SUCCESS) {
        report(label, "allocation_unavailable_windows_error", std::to_string(error));
        return;
    }
    const std::uint64_t allocated = (static_cast<std::uint64_t>(high) << 32U) | low;
#else
    struct stat status{};
    if (::stat(path.c_str(), &status) != 0) {
        report(label, "allocation_unavailable_errno", std::to_string(errno));
        return;
    }
    require(status.st_blocks >= 0, "negative allocation block count");
    const std::uint64_t blocks = static_cast<std::uint64_t>(status.st_blocks);
    require(blocks <= std::numeric_limits<std::uint64_t>::max() / 512U,
            "allocation block count overflow");
    const std::uint64_t allocated = blocks * 512U;
#endif
    report(label, "allocated_bytes", std::to_string(allocated));
    report(label, "allocation_less_than_logical", allocated < sparse_size ? "yes" : "no");
}

void sparse_fixture(FixtureArea& area) {
    const fs::path source = area.file("sparse-source");
    const fs::path destination = area.file("sparse-copy");
    write_bytes(source, "A");
#if defined(_WIN32)
    const HANDLE handle = CreateFileW(source.c_str(), GENERIC_WRITE, 0, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    require(handle != INVALID_HANDLE_VALUE, "sparse fixture open failed");
    DWORD returned{};
    const BOOL marked = DeviceIoControl(handle, FSCTL_SET_SPARSE, nullptr, 0,
        nullptr, 0, &returned, nullptr);
    const DWORD error = marked ? ERROR_SUCCESS : GetLastError();
    const BOOL closed = CloseHandle(handle);
    require(closed != 0, "sparse fixture handle close failed");
    if (!marked) {
        report("sparse", "fixture_unavailable_windows_error", std::to_string(error));
        return;
    }
#endif
    std::fstream stream(source, std::ios::in | std::ios::out | std::ios::binary);
    stream.seekp(static_cast<std::streamoff>(sparse_size - 1U));
    stream.put('Z');
    stream.close();
    require(static_cast<bool>(stream), "sparse fixture write failed");
    allocation_fact("sparse-source", source);
    copy_and_report("sparse", source, destination);
    allocation_fact("sparse-copy", destination);
}

void run(FixtureArea& area) {
    const std::array<std::string, 3> names{"ordinary", "readonly", "timestamp"};
    for (const std::string& name : names) {
        const fs::path source = area.file(name + "-source");
        const fs::path destination = area.file(name + "-copy");
        write_bytes(source, payload);
        if (name == "readonly") {
            fs::permissions(source, fs::perms::owner_read | fs::perms::group_read | fs::perms::others_read);
        } else {
            fs::permissions(source, fs::perms::owner_read | fs::perms::owner_write |
                fs::perms::owner_exec | fs::perms::group_read);
        }
        if (name == "timestamp") {
            const fs::file_time_type requested = fs::file_time_type::clock::now() - std::chrono::hours(24 * 365 * 5);
            fs::last_write_time(source, requested);
        }
        copy_and_report(name, source, destination);
    }
#if defined(_WIN32)
    metadata_fixture(area, "named-stream", "named-stream", "generated ADS payload");
#elif defined(__APPLE__)
    metadata_fixture(area, "xattr", "org.filemanager.copy-baseline", "generated xattr payload");
    metadata_fixture(area, "quarantine", "com.apple.quarantine", "0081;60000000;FileManagerCopyBaseline;");
#else
    metadata_fixture(area, "xattr", "user.filemanager_copy_baseline", "generated xattr payload");
#endif
    sparse_fixture(area);
}

void environment() {
    report("run", "probe", "copy-metadata-baseline-v1");
#if defined(_WIN32)
    report("run", "platform", "Windows");
#else
    struct utsname system{};
    if (uname(&system) == 0) {
        report("run", "platform", system.sysname);
        report("run", "kernel", system.release);
        report("run", "machine", system.machine);
    }
#endif
#if defined(__VERSION__)
    report("run", "compiler", __VERSION__);
#endif
#if defined(__GLIBCXX__)
    report("run", "libstdcxx_date", std::to_string(__GLIBCXX__));
#elif defined(_LIBCPP_VERSION)
    report("run", "libcxx_version", std::to_string(_LIBCPP_VERSION));
#endif
    report("run", "pointer_bytes", std::to_string(sizeof(void*)));
    report("run", "file_clock_period_numerator", std::to_string(fs::file_time_type::period::num));
    report("run", "file_clock_period_denominator", std::to_string(fs::file_time_type::period::den));
}
} // namespace

int main() {
    environment();
    try {
        FixtureArea area{};
        area.create();
        int result = 0;
        try {
            run(area);
        } catch (const std::exception& error) {
            report("run", "failure", error.what());
            result = 1;
        }
        area.cleanup();
        report("run", "result", result == 0 ? "completed" : "failed");
        return result;
    } catch (const std::exception& error) {
        report("run", "failure_preserved_scope", error.what());
        return 1;
    }
}
