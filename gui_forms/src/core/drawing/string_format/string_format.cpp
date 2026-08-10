#include "../support/drawing_support.hpp"

StringFormat::StringFormat(std::uint32_t flags) { value_.flags = flags; }
StringFormat::StringFormat(const StringFormat& source) : value_(source.snapshot()) {}

void StringFormat::set_alignment(StringAlignment alignment) {
    require_alive();
    require_enum(alignment, 2U, "string alignment");
    value_.alignment = alignment;
}

void StringFormat::set_line_alignment(StringAlignment alignment) {
    require_alive();
    require_enum(alignment, 2U, "line alignment");
    value_.line_alignment = alignment;
}

void StringFormat::set_trimming(StringTrimming trimming) {
    require_alive();
    require_enum(trimming, 5U, "string trimming");
    value_.trimming = trimming;
}

void StringFormat::set_flags(std::uint32_t flags) {
    require_alive();
    value_.flags = flags;
}

StringFormatSnapshot StringFormat::snapshot() const {
    require_alive();
    return value_;
}


} // namespace gui_drawing

