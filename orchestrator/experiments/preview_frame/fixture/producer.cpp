#include <array>
#include <cstdio>
#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#endif

int main() {
#if defined(_WIN32)
    const int descriptor = _fileno(stdout);
    const int previous = _setmode(descriptor, _O_BINARY);
    if (previous == -1) { return 1; }
#endif
    // Independent literal producer. No Rust encoder or native record layout.
    std::array<unsigned char, 64> header{};
    constexpr std::array<unsigned char, 8> magic{'F', 'M', 'P', 'R', 'E', 'V', '0', '1'};
    for (std::size_t index = 0; index < magic.size(); ++index) {
        header[index] = magic[index];
    }
    header[8] = 1;
    header[10] = 64;
    header[16] = 7;
    header[24] = 11;
    header[32] = 2;
    header[36] = 3;
    header[40] = 8;
    header[44] = 24;
    header[48] = 1;
    constexpr std::array<unsigned char, 24> pixels{
        0, 0, 255, 255, 0, 255, 0, 255,
        255, 0, 0, 255, 255, 255, 255, 255,
        0, 0, 0, 255, 20, 40, 60, 255};
    const std::size_t header_written = std::fwrite(header.data(), 1, header.size(), stdout);
    if (header_written != header.size()) { return 1; }
    const std::size_t pixels_written = std::fwrite(pixels.data(), 1, pixels.size(), stdout);
    if (pixels_written != pixels.size()) { return 1; }
    const int flushed = std::fflush(stdout);
    if (flushed != 0) { return 1; }
    return 0;
}
