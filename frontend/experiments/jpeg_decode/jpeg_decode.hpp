#pragma once

#include "jpeg_pixels.hpp"
#include <cstddef>
#include <span>

namespace jpeg_research {

inline constexpr std::size_t encoded_limit{16U * 1024U * 1024U};

// Research-private functions, no exported/install ABI. Borrow encoded bytes only
// for the call; return owned opaque BGRA or throw without publishing a raster.
[[nodiscard]] Pixels decode(const std::span<const unsigned char> encoded,
                            const unsigned orientation);
[[nodiscard]] Pixels decode_exif(const std::span<const unsigned char> encoded);

// Takes an already validated tight opaque raster, <=1024 per axis, and an EXIF
// orientation in 1..8. Private codec/WIC fixture callers establish these bounds.
[[nodiscard]] Pixels orient(Pixels source, const unsigned orientation);

}
