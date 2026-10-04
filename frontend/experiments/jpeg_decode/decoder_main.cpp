#include "encoded_stream.hpp"
#include "jpeg_decode.hpp"
#include "preview_frame.hpp"

#include <exception>
#include <cstdio>
#include <vector>

int main(const int argc, char** const argv) {
    static_cast<void>(argv);
    if (argc != 1) { return 2; }
    try {
        const std::vector<unsigned char> encoded = jpeg_research::read_encoded_stream();
        const jpeg_research::Pixels raster = jpeg_research::decode_exif(encoded);
        // Inert fixture ticket; no runtime request identity/source grant implied.
        jpeg_research::write_preview_frame(raster, 7, 11);
    } catch (const std::exception& error) {
        std::fprintf(stderr, "decoder fixture: %s\n", error.what());
        return 1;
    }
    return 0;
}
