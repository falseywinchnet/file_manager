#pragma once

#include "jpeg_pixels.hpp"
#include <cstdint>

namespace jpeg_research {

// Research-only stdout frame. Borrows the entire raster synchronously; validates
// geometry and opacity before writing. Throws on validation/output failure.
// No file path, runtime provider, authentication or retained borrow is supplied.
void write_preview_frame(const Pixels& raster, const std::uint64_t session,
                         const std::uint64_t nonce);

}
