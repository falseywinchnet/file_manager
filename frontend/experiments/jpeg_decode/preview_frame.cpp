#include "preview_frame.hpp"

#include <array>
#include <cstdio>
#include <stdexcept>
#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#endif

namespace jpeg_research {
namespace {

void require_frame(const bool condition, const char* const message) {
    if (!condition) { throw std::runtime_error(message); }
}

void write_u32(std::array<unsigned char, 64>& header, const std::size_t offset,
               const std::uint32_t value) noexcept {
    for (unsigned index = 0; index < 4; ++index) {
        const unsigned shift = index * 8;
        header[offset + index] = static_cast<unsigned char>((value >> shift) & 255U);
    }
}

void write_u64(std::array<unsigned char, 64>& header, const std::size_t offset,
               const std::uint64_t value) noexcept {
    for (unsigned index = 0; index < 8; ++index) {
        const unsigned shift = index * 8;
        header[offset + index] = static_cast<unsigned char>((value >> shift) & 255U);
    }
}

}

void write_preview_frame(const Pixels& raster, const std::uint64_t session,
                         const std::uint64_t nonce) {
    require_frame(session != 0 && nonce != 0, "frame ticket must be nonzero");
    require_frame(raster.width > 0 && raster.height > 0 &&
                  raster.width <= 1024 && raster.height <= 1024, "frame dimensions");
    const int expected_stride = raster.width * 4;
    require_frame(raster.row_bytes == expected_stride, "frame stride");
    const std::size_t extent = static_cast<std::size_t>(expected_stride) *
                               static_cast<std::size_t>(raster.height);
    require_frame(raster.bytes.size() == extent, "frame payload extent");
    for (std::size_t offset = 3; offset < extent; offset += 4) {
        require_frame(raster.bytes[offset] == 255, "frame alpha must be opaque");
    }
    std::array<unsigned char, 64> header{};
    constexpr std::array<unsigned char, 8> magic{'F', 'M', 'P', 'R', 'E', 'V', '0', '1'};
    for (std::size_t index = 0; index < magic.size(); ++index) { header[index] = magic[index]; }
    header[8] = 1;
    header[10] = 64;
    write_u64(header, 16, session);
    write_u64(header, 24, nonce);
    write_u32(header, 32, static_cast<std::uint32_t>(raster.width));
    write_u32(header, 36, static_cast<std::uint32_t>(raster.height));
    write_u32(header, 40, static_cast<std::uint32_t>(expected_stride));
    write_u32(header, 44, static_cast<std::uint32_t>(extent));
    write_u32(header, 48, 1);
#if defined(_WIN32)
    const int descriptor = _fileno(stdout);
    const int previous = _setmode(descriptor, _O_BINARY);
    require_frame(previous != -1, "cannot select binary frame stream");
#endif
    const std::size_t header_written = std::fwrite(header.data(), 1, header.size(), stdout);
    require_frame(header_written == header.size(), "frame header write failed");
    const std::size_t body_written = std::fwrite(raster.bytes.data(), 1, extent, stdout);
    require_frame(body_written == extent, "frame payload write failed");
    const int flushed = std::fflush(stdout);
    require_frame(flushed == 0, "frame stream flush failed");
}

}
