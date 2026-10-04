#pragma once

#include <span>
#include <vector>

namespace jpeg_research {

struct Pixels final {
    int width{};
    int height{};
    int row_bytes{};
    // Owned contiguous top-down BGRA8; opaque alpha, no row padding.
    std::vector<unsigned char> bytes{};
};

#ifdef _WIN32
// Research control only: borrows encoded bytes synchronously, copies them into
// the WIC stream, and returns independent pixels. No file paths or retained
// callbacks. Requested unrotated dimensions must be positive, <=1024 per side
// and no larger than the source. Throws on failure; no partial output escapes.
// This has no process memory/deadline boundary and is not a product provider.
[[nodiscard]] Pixels decode_wic(const std::span<const unsigned char> encoded,
                               const int output_width, const int output_height);
#endif

} // namespace jpeg_research
