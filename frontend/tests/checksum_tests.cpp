#include "fixture_links.hpp"
#include "file_manager/checksum.hpp"

#include <cstdlib>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <unistd.h>

namespace {

class TestRoot final {
public:
    TestRoot() {
        path_ = std::filesystem::temp_directory_path() /
            ("file-manager-checksum-" + std::to_string(::getpid()));
        std::filesystem::create_directories(path_);
    }
    TestRoot(const TestRoot&) = delete;
    TestRoot& operator=(const TestRoot&) = delete;
    ~TestRoot() {
        std::error_code ignored{};
        std::filesystem::remove_all(path_, ignored);
    }
    [[nodiscard]] const std::filesystem::path& path() const noexcept {
        return path_;
    }
private:
    std::filesystem::path path_{};
};

void require(const bool condition, const std::string_view message) {
    if (!condition) {
        std::cerr << "checksum test failed: " << message << '\n';
        throw std::runtime_error(std::string(message));
    }
}

void write_bytes(const std::filesystem::path& path, const std::string_view bytes) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    stream.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
}

// These callables borrow main's counter only during synchronous hashing.
struct CancelAfterProgress final {
    const unsigned& calls;
    bool operator()() const {
        const bool cancelled = calls >= 1U;
        return cancelled;
    }
};

struct CountProgress final {
    unsigned& calls;
    void operator()(const file_manager::ChecksumProgress progress) const {
        require(progress.bytes_read <= progress.total_bytes,
                "progress must remain bounded by observed size");
        ++calls;
    }
};

} // namespace

int main() {
    try {
        TestRoot root{};
        const std::filesystem::path empty = root.path() / "empty.bin";
        write_bytes(empty, {});
        file_manager::ChecksumResult result = file_manager::checksum_sha256(
            root.path(), empty, file_manager::observe_identity(empty));
        require(result.succeeded() &&
                    result.digest_hex ==
                        "e3b0c44298fc1c149afbf4c8996fb924"
                        "27ae41e4649b934ca495991b7852b855" &&
                    result.bytes_read == 0,
                "empty-file known-answer vector must match");

        const std::filesystem::path abc = root.path() / "abc.txt";
        write_bytes(abc, "abc");
        result = file_manager::checksum_sha256(
            root.path(), abc, file_manager::observe_identity(abc));
        require(result.succeeded() &&
                    result.digest_hex ==
                        "ba7816bf8f01cfea414140de5dae2223"
                        "b00361a396177a9cb410ff61f20015ad" &&
                    result.bytes_read == 3,
                "abc known-answer vector must match");

        const std::filesystem::path large = root.path() / "large.bin";
        write_bytes(large, std::string(700'000, 'a'));
        unsigned progress_calls{};
        result = file_manager::checksum_sha256(
            root.path(), large, file_manager::observe_identity(large),
            CancelAfterProgress{progress_calls}, CountProgress{progress_calls});
        require(result.terminal == file_manager::ChecksumTerminal::cancelled &&
                    result.digest_hex.empty() && result.bytes_read == 256U * 1024U,
                "cancellation must publish no digest and retain exact progress");

        const file_manager::ObjectIdentity stale = file_manager::observe_identity(abc);
        write_bytes(abc, "different revision");
        result = file_manager::checksum_sha256(root.path(), abc, stale);
        require(result.terminal == file_manager::ChecksumTerminal::changed &&
                    result.code == "selection-changed" && result.digest_hex.empty(),
                "stale selected revisions must fail before reading");

        if (create_fixture_link("abc.txt", root.path() / "abc-link")) {

        result = file_manager::checksum_sha256(
            root.path(), root.path() / "abc-link",
            file_manager::observe_identity(root.path() / "abc-link"));
        require(result.terminal == file_manager::ChecksumTerminal::refused &&
                    result.code == "symlink-refused",
                "checksum must never follow the selected symbolic link");
        }

        result = file_manager::checksum_sha256(
            root.path(), root.path().parent_path(), {});
        require(result.terminal == file_manager::ChecksumTerminal::refused &&
                    result.code == "outside-protected-root",
                "out-of-root checksum paths must fail closed");

        std::cout << "checksum tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
