#include "../support/drawing_support.hpp"

Font::Font(std::string family, double size, FontStyle style,
           GraphicsUnit unit, std::uint8_t charset) {
    require_finite(size, "font size");
    if (family.empty() || family.find('\0') != std::string::npos ||
        family.size() > 4096 || !valid_utf8(family)) {
        throw std::invalid_argument("font family must be bounded valid UTF-8 without NUL");
    }
    if (size <= 0.0 || size > 1'000'000.0) {
        throw std::invalid_argument("font size must be positive and bounded");
    }
    if ((static_cast<std::uint32_t>(style) & ~UINT32_C(0x0f)) != 0U) {
        throw std::invalid_argument("font style contains undeclared bits");
    }
    require_enum(unit, 6U, "graphics unit");
    value_ = {std::move(family), size, static_cast<std::uint32_t>(style), unit, charset};
}

Font::Font(const Font& source, FontStyle style) : value_(source.snapshot()) {
    value_.style = static_cast<std::uint32_t>(style);
}

FontSnapshot Font::snapshot() const {
    require_alive();
    return value_;
}

std::int32_t Font::deterministic_height() const {
    require_alive();
    return clamp_i32(static_cast<std::int64_t>(std::ceil(value_.size * 1.2)));
}


} // namespace gui_drawing

