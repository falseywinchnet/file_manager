#pragma once

#include <cstdint>

namespace gui_drawing {

enum class PixelFormat : std::uint8_t {
    bgra32_premultiplied,
    rgba32_premultiplied,
};

} // namespace gui_drawing
