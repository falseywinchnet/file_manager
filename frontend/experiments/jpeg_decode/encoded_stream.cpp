#include "encoded_stream.hpp"
#include "jpeg_decode.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#endif

namespace jpeg_research {
namespace {
constexpr std::array<unsigned char, 8> input_magic{'F', 'M', 'J', 'P', 'E', 'G', '0', '1'};

void require_stream(const bool condition, const char* const message) {
    if (!condition) { throw std::runtime_error(message); }
}

void read_exact(const std::span<unsigned char> destination) {
    std::size_t offset = 0;
    while (offset < destination.size()) {
        const std::size_t count = std::fread(destination.data() + offset, 1,
                                             destination.size() - offset, stdin);
        if (count == 0) {
            const int error = std::ferror(stdin);
            require_stream(error == 0, "input stream read failed");
            throw std::runtime_error("input stream truncated");
        }
        offset += count;
    }
}
}

void write_encoded_stream(const std::span<const unsigned char> encoded) {
    require_stream(!encoded.empty() && encoded.size() <= encoded_limit, "input encoded extent");
    std::array<unsigned char, 16> header{};
    for (std::size_t index = 0; index < input_magic.size(); ++index) { header[index] = input_magic[index]; }
    const std::uint64_t length = static_cast<std::uint64_t>(encoded.size());
    for (unsigned index = 0; index < 8; ++index) {
        const unsigned shift = index * 8;
        header[8 + index] = static_cast<unsigned char>((length >> shift) & 255U);
    }
#if defined(_WIN32)
    const int descriptor = _fileno(stdout);
    const int previous = _setmode(descriptor, _O_BINARY);
    require_stream(previous != -1, "cannot select binary encoded output");
#endif
    const std::size_t header_written = std::fwrite(header.data(), 1, header.size(), stdout);
    require_stream(header_written == header.size(), "encoded header write failed");
    const std::size_t body_written = std::fwrite(encoded.data(), 1, encoded.size(), stdout);
    require_stream(body_written == encoded.size(), "encoded body write failed");
    const int flushed = std::fflush(stdout);
    require_stream(flushed == 0, "encoded output flush failed");
}

std::vector<unsigned char> read_encoded_stream() {
#if defined(_WIN32)
    const int descriptor = _fileno(stdin);
    const int previous = _setmode(descriptor, _O_BINARY);
    require_stream(previous != -1, "cannot select binary encoded input");
#endif
    std::array<unsigned char, 16> header{};
    read_exact(header);
    const bool magic_valid = std::equal(input_magic.begin(), input_magic.end(), header.begin());
    require_stream(magic_valid, "encoded input magic");
    std::uint64_t length = 0;
    for (unsigned index = 0; index < 8; ++index) {
        const unsigned shift = index * 8;
        length |= static_cast<std::uint64_t>(header[8 + index]) << shift;
    }
    require_stream(length > 0 && length <= encoded_limit, "encoded input byte limit");
    // Length is bounded before conversion/allocation. No per-read growth.
    std::vector<unsigned char> encoded(static_cast<std::size_t>(length), 0);
    read_exact(encoded);
    const int tail = std::fgetc(stdin);
    const int error = std::ferror(stdin);
    require_stream(error == 0, "input EOF read failed");
    require_stream(tail == EOF, "encoded input trailing bytes");
    return encoded;
}

}
