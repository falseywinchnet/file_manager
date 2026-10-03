#include "../src/native_copy.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#if defined(__APPLE__)
#include <cerrno>
#include <sys/xattr.h>
#endif

namespace {
namespace fs = std::filesystem;
using file_manager::NativeCopyResult;
using file_manager::NativeCopyTerminal;
using file_manager::ObjectIdentity;

void require(const bool condition, const char* const message) {
    if (!condition) throw std::runtime_error(message);
}

void write_fixture(const fs::path& path, const std::size_t length) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    std::array<char, 4096> bytes{};
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        bytes[index] = static_cast<char>('A' + index % 23U);
    }
    std::size_t remaining = length;
    while (remaining != 0) {
        const std::size_t count = std::min(remaining, bytes.size());
        stream.write(bytes.data(), static_cast<std::streamsize>(count));
        require(static_cast<bool>(stream), "fixture write failed");
        remaining -= count;
    }
    stream.close();
    require(static_cast<bool>(stream), "fixture close failed");
}

class FixtureOwner final {
public:
    FixtureOwner() = default;
    FixtureOwner(const FixtureOwner&) = delete;
    FixtureOwner& operator=(const FixtureOwner&) = delete;
    ~FixtureOwner() {
        if (!owned_ || cleanup_attempted_) return;
        try { cleanup(); }
        catch (const std::exception& error) { std::cerr << "cleanup preserved scope: " << error.what() << '\n'; }
    }

    void create() {
        parent_ = fs::canonical(fs::temp_directory_path());
        std::random_device random{};
        for (unsigned int attempt = 0; attempt < 128U; ++attempt) {
            const std::string token = std::to_string(random());
            root_ = parent_ / ("file-manager-native-copy-" + token);
            std::error_code error{};
            owned_ = fs::create_directory(root_, error);
            if (owned_) {
                root_identity_ = file_manager::observe_identity(root_);
                require(root_identity_.available(), "fixture root identity unavailable");
                std::cout << "owned_temp=" << root_.generic_string() << '\n';
                return;
            }
            if (error && error != std::errc::file_exists) throw std::system_error(error);
        }
        throw std::runtime_error("fixture exclusive create attempts exhausted");
    }

    [[nodiscard]] fs::path file(const std::string& name) {
        const fs::path leaf(name);
        require(!leaf.empty() && leaf == leaf.filename() && leaf != "." && leaf != "..",
                "fixture name is not a direct child");
        const fs::path path = root_ / leaf;
        files_.push_back(path);
        return path;
    }

    void cleanup() {
        cleanup_attempted_ = true;
        require(root_.is_absolute() && root_.parent_path() == parent_ &&
                fs::canonical(root_) == root_, "fixture cleanup scope changed");
        const ObjectIdentity current = file_manager::observe_identity(root_);
        require(current == root_identity_ && current.type == fs::file_type::directory,
                "fixture cleanup root identity changed");
        for (const fs::path& path : files_) {
            require(path.parent_path() == root_, "fixture cleanup child escaped");
            std::error_code error{};
            const fs::file_status status = fs::symlink_status(path, error);
            if (error == std::errc::no_such_file_or_directory || status.type() == fs::file_type::not_found) continue;
            require(!error && status.type() == fs::file_type::regular, "unexpected fixture child type");
            fs::permissions(path, fs::perms::owner_write, fs::perm_options::add);
            require(fs::remove(path), "fixture file cleanup failed");
        }
        require(fs::remove(root_), "fixture root cleanup failed; unknown children preserved");
        require(!fs::exists(root_), "fixture root still exists");
        std::cout << "owned_cleanup=verified and removed\n";
    }
private:
    fs::path parent_{};
    fs::path root_{};
    ObjectIdentity root_identity_{};
    std::vector<fs::path> files_{};
    bool owned_{};
    bool cleanup_attempted_{};
};

void verify_contents(const fs::path& path, const std::size_t length) {
    require(fs::file_size(path) == length, "fixture size mismatch");
    std::ifstream stream(path, std::ios::binary);
    std::array<char, 4096> bytes{};
    std::size_t remaining = length;
    while (remaining != 0) {
        const std::size_t count = std::min(remaining, bytes.size());
        stream.read(bytes.data(), static_cast<std::streamsize>(count));
        require(stream.gcount() == static_cast<std::streamsize>(count) && !stream.bad(), "fixture read failed");
        for (std::size_t index = 0; index < count; ++index) {
            const char expected = static_cast<char>('A' + index % 23U);
            require(bytes[index] == expected, "fixture contents mismatch");
        }
        remaining -= count;
    }
}

void verify_closed(const NativeCopyResult& result) {
    require(!result.source_close_error && !result.stage_close_error, "native close failed");
}

void verify_owned(const NativeCopyResult& result, const fs::path& stage) {
    const ObjectIdentity current = file_manager::observe_identity(stage);
    require(result.stage_created && result.stage_identity.available() &&
            current == result.stage_identity, "returned stage ownership mismatch");
    verify_closed(result);
}

void successful_files(FixtureOwner& owner, file_manager::NativeCopyWorkspace& workspace) {
    const std::array<std::size_t, 3> lengths{0U, 37U, 3U * file_manager::NativeCopyWorkspace::capacity + 17U};
    unsigned int ordinal{};
    for (const std::size_t length : lengths) {
        const std::string prefix = "success-" + std::to_string(ordinal);
        ++ordinal;
        const fs::path source = owner.file(prefix + "-source");
        const fs::path stage = owner.file(prefix + "-stage");
        write_fixture(source, length);
        const ObjectIdentity expected = file_manager::observe_identity(source);
        const NativeCopyResult result = file_manager::copy_regular_file_to_stage(source, stage, expected, {}, workspace);
        require(result.terminal == NativeCopyTerminal::complete && !result.error &&
                result.copied_bytes == length, "successful copy result mismatch");
        verify_owned(result, stage);
        require(file_manager::observe_identity(source).same_revision(expected), "source revision changed");
        require(!(expected == result.stage_identity), "copy reused source identity");
        verify_contents(source, length);
        verify_contents(stage, length);
    }
    std::cout << "PASS empty/small/multichunk; contents and identities\n";
}

void existing_destination(FixtureOwner& owner, file_manager::NativeCopyWorkspace& workspace) {
    const fs::path source = owner.file("existing-source");
    const fs::path stage = owner.file("existing-stage");
    write_fixture(source, 37U);
    write_fixture(stage, 19U);
    const ObjectIdentity expected = file_manager::observe_identity(source);
    const ObjectIdentity occupied = file_manager::observe_identity(stage);
    const NativeCopyResult result = file_manager::copy_regular_file_to_stage(source, stage, expected, {}, workspace);
    require(result.terminal == NativeCopyTerminal::failed && result.error &&
            !result.stage_created && !result.stage_identity.available() && result.copied_bytes == 0,
            "create refusal claimed destination ownership");
    verify_closed(result);
    require(file_manager::observe_identity(stage).same_revision(occupied), "existing destination changed");
    verify_contents(stage, 19U);
    verify_contents(source, 37U);
    std::cout << "PASS existing destination preserved; no ownership\n";
}

bool cancel_before_creation() { return true; }

struct StageObserver final {
    fs::path stage{};
    bool throw_after_data{};
    bool observed_data{};
    bool operator()() {
        // Observe through a fresh native handle: the CRT path-size projection
        // on Windows can remain stale while the writer's handle is still open.
        const ObjectIdentity current = file_manager::observe_identity(stage);
        if (!current.available() || current.size == 0) return false;
        observed_data = true;
        if (throw_after_data) throw std::runtime_error("generated callback exception");
        return true;
    }
};

void cancellation_files(FixtureOwner& owner, file_manager::NativeCopyWorkspace& workspace) {
    const fs::path source = owner.file("cancel-source");
    const fs::path before_stage = owner.file("cancel-before-stage");
    constexpr std::size_t length = 16U * 1024U * 1024U;
    write_fixture(source, length);
    const ObjectIdentity expected = file_manager::observe_identity(source);
    const file_manager::CancellationCheck immediate{cancel_before_creation};
    const NativeCopyResult before = file_manager::copy_regular_file_to_stage(source, before_stage, expected, immediate, workspace);
    require(before.terminal == NativeCopyTerminal::cancelled && !before.stage_created &&
            !before.stage_identity.available() && before.copied_bytes == 0 && !fs::exists(before_stage),
            "early cancellation created destination");
    verify_closed(before);
    const std::array<bool, 2> exception_modes{false, true};
    for (const bool throw_after_data : exception_modes) {
        const std::string name = throw_after_data ? "exception-stage" : "cancel-after-stage";
        const fs::path stage = owner.file(name);
        StageObserver observer{stage, throw_after_data, false};
        const file_manager::CancellationCheck check{std::ref(observer)};
        const NativeCopyResult result = file_manager::copy_regular_file_to_stage(source, stage, expected, check, workspace);
        const NativeCopyTerminal terminal = throw_after_data ? NativeCopyTerminal::callback_failed : NativeCopyTerminal::cancelled;
        require(result.terminal == terminal && observer.observed_data && result.copied_bytes > 0 &&
                result.copied_bytes <= length, "post-write cancellation/exception not contained");
#if !defined(__APPLE__)
        require(result.copied_bytes <= file_manager::NativeCopyWorkspace::capacity,
                "bounded callback cancellation exceeded first chunk");
#endif
        verify_owned(result, stage);
        require(fs::file_size(stage) == result.copied_bytes, "cancelled byte count mismatch");
        const std::size_t copied = static_cast<std::size_t>(result.copied_bytes);
        verify_contents(stage, copied);
        require(file_manager::observe_identity(source).same_revision(expected), "cancel modified source");
    }
    verify_contents(source, length);
    std::cout << "PASS before-create cancellation; post-write cancellation and callback exception\n";
}

void source_mismatch(FixtureOwner& owner, file_manager::NativeCopyWorkspace& workspace) {
    const fs::path source = owner.file("mismatch-source");
    const fs::path stage = owner.file("mismatch-stage");
    write_fixture(source, 37U);
    ObjectIdentity expected = file_manager::observe_identity(source);
    ++expected.size;
    const NativeCopyResult result = file_manager::copy_regular_file_to_stage(source, stage, expected, {}, workspace);
    require(result.terminal == NativeCopyTerminal::source_changed && !result.stage_created &&
            !result.stage_identity.available() && result.copied_bytes == 0 && !fs::exists(stage),
            "source mismatch created stage");
    verify_closed(result);
    verify_contents(source, 37U);
    std::cout << "PASS source revision mismatch before creation\n";
}

void permissions(FixtureOwner& owner, file_manager::NativeCopyWorkspace& workspace) {
    const std::array<fs::perms, 2> modes{
        fs::perms::owner_read | fs::perms::owner_write | fs::perms::owner_exec | fs::perms::group_read,
        fs::perms::owner_read | fs::perms::group_read | fs::perms::others_read};
    unsigned int ordinal{};
    for (const fs::perms mode : modes) {
        const std::string prefix = "permissions-" + std::to_string(ordinal);
        ++ordinal;
        const fs::path source = owner.file(prefix + "-source");
        const fs::path stage = owner.file(prefix + "-stage");
        write_fixture(source, 37U);
        fs::permissions(source, mode);
        const fs::perms observed = fs::status(source).permissions();
        const ObjectIdentity expected = file_manager::observe_identity(source);
        const NativeCopyResult result = file_manager::copy_regular_file_to_stage(source, stage, expected, {}, workspace);
        require(result.terminal == NativeCopyTerminal::complete && !result.error, "permission copy failed");
        verify_owned(result, stage);
        require(fs::status(stage).permissions() == observed, "ordinary/read-only mode mismatch");
        verify_contents(stage, 37U);
    }
    std::cout << "PASS ordinary and read-only permissions\n";
}

#if defined(__APPLE__)
struct AttributeValue final {
    bool present{};
    std::string value{};
};

[[nodiscard]] AttributeValue read_attribute(const fs::path& path, const std::string& name) {
    std::array<char, 512> bytes{};
    const ssize_t count = getxattr(path.c_str(), name.c_str(), bytes.data(), bytes.size(), 0, XATTR_NOFOLLOW);
    if (count < 0) {
        require(errno == ENOATTR, "metadata equivalence read failed");
        return {};
    }
    const std::size_t size = static_cast<std::size_t>(count);
    const AttributeValue result{true, std::string(bytes.data(), size)};
    return result;
}

void mac_metadata_equivalence(FixtureOwner& owner, file_manager::NativeCopyWorkspace& workspace) {
    const std::array<std::string, 2> names{"org.filemanager.native-copy", "com.apple.quarantine"};
    const std::array<std::string, 2> values{"generated native copy xattr", "0081;60000000;FileManagerCopyBaseline;"};
    for (std::size_t index = 0; index < names.size(); ++index) {
        const std::string prefix = "metadata-" + std::to_string(index);
        const fs::path source = owner.file(prefix + "-source");
        const fs::path baseline = owner.file(prefix + "-baseline");
        const fs::path stage = owner.file(prefix + "-stage");
        write_fixture(source, 37U);
        const std::string& name = names[index];
        const std::string& value = values[index];
        const int set = setxattr(source.c_str(), name.c_str(), value.data(), value.size(), 0, XATTR_CREATE | XATTR_NOFOLLOW);
        if (set != 0) {
            const int error = errno;
            std::cout << "UNMEASURED metadata equivalence " << name << " fixture errno=" << error << '\n';
            continue;
        }
        const AttributeValue before = read_attribute(source, name);
        require(before.present && before.value == value, "source metadata readback failed");
        require(fs::copy_file(source, baseline), "metadata baseline copy failed");
        const ObjectIdentity expected = file_manager::observe_identity(source);
        const NativeCopyResult result = file_manager::copy_regular_file_to_stage(source, stage, expected, {}, workspace);
        require(result.terminal == NativeCopyTerminal::complete && !result.error, "metadata adapter copy failed");
        verify_owned(result, stage);
        const AttributeValue control = read_attribute(baseline, name);
        const AttributeValue candidate = read_attribute(stage, name);
        require(control.present == candidate.present && control.value == candidate.value,
                "native adapter differs from libc++ metadata baseline");
        verify_contents(stage, 37U);
        std::cout << "PASS metadata equivalence " << name << " present=" << candidate.present << '\n';
    }
}
#endif
} // namespace

int main() {
    try {
        FixtureOwner owner{};
        owner.create();
        file_manager::NativeCopyWorkspace workspace{};
        const std::byte* const initial_storage = workspace.bytes().data();
        successful_files(owner, workspace);
        existing_destination(owner, workspace);
        cancellation_files(owner, workspace);
        source_mismatch(owner, workspace);
        permissions(owner, workspace);
#if defined(__APPLE__)
        mac_metadata_equivalence(owner, workspace);
#endif
        require(workspace.bytes().data() == initial_storage, "workspace storage replaced");
        owner.cleanup();
        std::cout << "PASS native copy fixtures; workspace reused\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL " << error.what() << '\n';
        return 1;
    }
}
