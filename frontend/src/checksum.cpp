#include "file_manager/checksum.hpp"
#include "native_file.hpp"

#include <array>
#include <cerrno>
#include <cstring>
#include <iomanip>
#include <sstream>

namespace file_manager {
namespace {

constexpr std::array<std::uint32_t, 64> round_constants{
    0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U,
    0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
    0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U,
    0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
    0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
    0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
    0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U,
    0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
    0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U,
    0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
    0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U,
    0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
    0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U,
    0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
    0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
    0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U,
};

constexpr std::uint32_t rotate_right(const std::uint32_t value,
                                     const unsigned count) noexcept {
    const std::uint32_t result = (value >> count) | (value << (32U - count));
    return result;
}

class Sha256 final {
public:
    // Borrows count initialized bytes for this call; fixed block storage is reused.
    void update(const std::byte* bytes, std::size_t count) noexcept {
        total_bytes_ += count;
        while (count != 0U) {
            const std::size_t available = block_.size() - block_size_;
            const std::size_t consumed = std::min(count, available);
            std::memcpy(block_.data() + block_size_, bytes, consumed);
            block_size_ += consumed;
            bytes += consumed;
            count -= consumed;
            if (block_size_ == block_.size()) {
                transform(block_.data());
                block_size_ = 0U;
            }
        }
    }

    [[nodiscard]] std::array<std::uint8_t, 32> finish() noexcept {
        const std::uint64_t bit_count = static_cast<std::uint64_t>(total_bytes_) * 8U;
        block_[block_size_++] = std::byte{0x80};
        if (block_size_ > 56U) {
            std::fill(block_.begin() + static_cast<std::ptrdiff_t>(block_size_),
                      block_.end(), std::byte{});
            transform(block_.data());
            block_size_ = 0U;
        }
        std::fill(block_.begin() + static_cast<std::ptrdiff_t>(block_size_),
                  block_.begin() + 56, std::byte{});
        for (std::size_t index = 0; index < 8U; ++index) {
            block_[63U - index] = static_cast<std::byte>(
                (bit_count >> (index * 8U)) & 0xffU);
        }
        transform(block_.data());

        std::array<std::uint8_t, 32> digest{};
        for (std::size_t word = 0; word < state_.size(); ++word) {
            for (std::size_t byte = 0; byte < 4U; ++byte) {
                digest[word * 4U + byte] = static_cast<std::uint8_t>(
                    state_[word] >> ((3U - byte) * 8U));
            }
        }
        return digest;
    }

private:
    void transform(const std::byte* block) noexcept {
        std::array<std::uint32_t, 64> words{};
        for (std::size_t index = 0; index < 16U; ++index) {
            const std::size_t offset = index * 4U;
            words[index] =
                (static_cast<std::uint32_t>(block[offset]) << 24U) |
                (static_cast<std::uint32_t>(block[offset + 1U]) << 16U) |
                (static_cast<std::uint32_t>(block[offset + 2U]) << 8U) |
                static_cast<std::uint32_t>(block[offset + 3U]);
        }
        for (std::size_t index = 16U; index < words.size(); ++index) {
            const std::uint32_t a = words[index - 15U];
            const std::uint32_t b = words[index - 2U];
            const std::uint32_t sigma0 = rotate_right(a, 7U) ^ rotate_right(a, 18U) ^
                (a >> 3U);
            const std::uint32_t sigma1 = rotate_right(b, 17U) ^ rotate_right(b, 19U) ^
                (b >> 10U);
            words[index] = words[index - 16U] + sigma0 +
                words[index - 7U] + sigma1;
        }

        std::uint32_t a = state_[0];
        std::uint32_t b = state_[1];
        std::uint32_t c = state_[2];
        std::uint32_t d = state_[3];
        std::uint32_t e = state_[4];
        std::uint32_t f = state_[5];
        std::uint32_t g = state_[6];
        std::uint32_t h = state_[7];
        for (std::size_t index = 0; index < words.size(); ++index) {
            const std::uint32_t choose = (e & f) ^ (~e & g);
            const std::uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
            const std::uint32_t sum0 = rotate_right(a, 2U) ^ rotate_right(a, 13U) ^
                rotate_right(a, 22U);
            const std::uint32_t sum1 = rotate_right(e, 6U) ^ rotate_right(e, 11U) ^
                rotate_right(e, 25U);
            const std::uint32_t first = h + sum1 + choose + round_constants[index] +
                words[index];
            const std::uint32_t second = sum0 + majority;
            h = g;
            g = f;
            f = e;
            e = d + first;
            d = c;
            c = b;
            b = a;
            a = first + second;
        }
        state_[0] += a;
        state_[1] += b;
        state_[2] += c;
        state_[3] += d;
        state_[4] += e;
        state_[5] += f;
        state_[6] += g;
        state_[7] += h;
    }

    std::array<std::uint32_t, 8> state_{
        0x6a09e667U, 0xbb67ae85U, 0x3c6ef372U, 0xa54ff53aU,
        0x510e527fU, 0x9b05688cU, 0x1f83d9abU, 0x5be0cd19U,
    };
    std::array<std::byte, 64> block_{};
    std::size_t block_size_{};
    std::size_t total_bytes_{};
};

ChecksumResult failure(const ChecksumTerminal terminal, std::string code,
                       std::string message,
                       const std::filesystem::path& path) {
    ChecksumResult result{};
    result.terminal = terminal;
    result.code = std::move(code);
    result.message = std::move(message);
    result.path = path;
    return result;
}

std::string hex_digest(const std::array<std::uint8_t, 32>& bytes) {
    std::ostringstream stream{};
    stream << std::hex << std::setfill('0');
    for (const std::uint8_t byte : bytes) {
        stream << std::setw(2) << static_cast<unsigned>(byte);
    }
    const std::string result = stream.str();
    return result;
}

} // namespace

ChecksumResult checksum_sha256(
    const std::filesystem::path& protected_root,
    const std::filesystem::path& selected_path,
    const ObjectIdentity& expected_identity,
    const CancellationCheck& cancelled,
    const ChecksumProgressCallback& progress) {
    std::filesystem::path root{};
    try {
        root = canonical_existing_directory(protected_root);
    } catch (const std::exception& error) {
        const ChecksumResult result = failure(ChecksumTerminal::unavailable, "root-unavailable",
                       error.what(), selected_path);
        return result;
    }
    std::error_code absolute_error{};
    const std::filesystem::path supplied_root = std::filesystem::absolute(
        protected_root, absolute_error).lexically_normal();
    const std::filesystem::path supplied_path = (selected_path.is_absolute()
        ? selected_path
        : supplied_root / selected_path).lexically_normal();
    const std::filesystem::path path = (!absolute_error &&
                       path_is_within(supplied_root, supplied_path))
        ? (root / supplied_path.lexically_relative(supplied_root))
              .lexically_normal()
        : supplied_path;
    if (!path_is_within(root, path)) {
        const ChecksumResult result = failure(ChecksumTerminal::refused, "outside-protected-root",
                       "checksum path is outside the protected root", path);
        return result;
    }
    if (path_route_has_symlink(root, path)) {
        const ChecksumResult result = failure(ChecksumTerminal::refused, "symlink-refused",
                       "checksum never follows a symbolic link", path);
        return result;
    }

    NativeReadFile descriptor(path);
    if (!descriptor.available()) {
        const ChecksumResult result = failure(ChecksumTerminal::unavailable, "open-failed",
                       std::string("cannot open file: ") + descriptor.error_message(),
                       path);
        return result;
    }
    const ObjectIdentity before = descriptor.identity();
    if (!before.available()) {
        const ChecksumResult result = failure(ChecksumTerminal::unavailable, "inspect-failed",
                       "cannot inspect opened file", path);
        return result;
    }
    if (before.type != std::filesystem::file_type::regular) {
        const ChecksumResult result = failure(ChecksumTerminal::refused, "not-regular-file",
                       "checksum accepts one regular file", path);
        return result;
    }
    if (expected_identity.available() &&
        !expected_identity.same_revision(before)) {
        const ChecksumResult result = failure(ChecksumTerminal::changed, "selection-changed",
                       "selected file changed before checksum began", path);
        return result;
    }

    constexpr std::size_t buffer_size = 256U * 1024U;
    std::array<std::byte, buffer_size> buffer{};
    Sha256 hash{};
    std::uint64_t bytes_read{};
    for (;;) {
        if (cancelled && cancelled()) {
            ChecksumResult result = failure(ChecksumTerminal::cancelled, "cancelled",
                                  "checksum cancelled", path);
            result.identity = before;
            result.bytes_read = bytes_read;
            return result;
        }
        const std::ptrdiff_t count = descriptor.read(buffer.data(), buffer.size());
        if (count < 0) {
            ChecksumResult result = failure(ChecksumTerminal::unavailable, "read-failed",
                std::string("cannot read file: ") + descriptor.error_message(), path);
            result.identity = before;
            result.bytes_read = bytes_read;
            return result;
        }
        if (count == 0) break;
        const std::size_t size = static_cast<std::size_t>(count);
        hash.update(buffer.data(), size);
        bytes_read += static_cast<std::uint64_t>(size);
        if (progress) {
            progress({bytes_read, before.size});
        }
    }

    const ObjectIdentity after = descriptor.identity();
    if (!after.available()) {
        ChecksumResult result = failure(ChecksumTerminal::unavailable,
            "post-inspect-failed", "cannot revalidate opened file", path);
        result.identity = before;
        result.bytes_read = bytes_read;
        return result;
    }
    const ObjectIdentity path_after = observe_identity(path);
    if (!before.same_revision(after) || !after.same_revision(path_after) ||
        bytes_read != after.size) {
        ChecksumResult result = failure(ChecksumTerminal::changed,
            "changed-during-read",
            "file changed or was replaced while SHA-256 was reading it", path);
        result.identity = after;
        result.bytes_read = bytes_read;
        return result;
    }

    ChecksumResult result{};
    result.terminal = ChecksumTerminal::completed;
    result.code = "ok";
    result.message = "SHA-256 completed over a stable file revision";
    const std::array<std::uint8_t, 32> digest = hash.finish();
    result.digest_hex = hex_digest(digest);
    result.path = path;
    result.identity = after;
    result.bytes_read = bytes_read;
    return result;
}

} // namespace file_manager
