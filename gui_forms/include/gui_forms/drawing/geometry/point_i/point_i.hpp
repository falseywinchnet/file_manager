#pragma once

#include <cstdint>

namespace gui_drawing {

struct PointI final {
    std::int32_t x{};
    std::int32_t y{};
    void offset(std::int32_t dx, std::int32_t dy) noexcept;
    friend constexpr bool operator==(const PointI&, const PointI&) = default;
};

} // namespace gui_drawing
