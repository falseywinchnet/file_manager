#pragma once

namespace gui_drawing {

struct SizeF final {
    double width{};
    double height{};
    friend constexpr bool operator==(const SizeF&, const SizeF&) = default;
};

} // namespace gui_drawing
