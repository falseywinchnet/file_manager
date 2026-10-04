#pragma once

#include <span>
#include <vector>

namespace jpeg_research {

// Research input envelope: fixed magic, little-endian byte length, JPEG bytes,
// then EOF. No file path, source identity or authentication is conveyed.
// Both calls are synchronous and throw on errors; reader owns its returned bytes.
void write_encoded_stream(const std::span<const unsigned char> encoded);
[[nodiscard]] std::vector<unsigned char> read_encoded_stream();

}
