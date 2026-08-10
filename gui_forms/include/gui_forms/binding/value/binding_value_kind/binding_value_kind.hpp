#pragma once

#include <cstdint>

namespace gui_forms {

enum class BindingValueKind : std::uint8_t {
    null,
    boolean,
    signed_integer,
    unsigned_integer,
    number,
    text,
    point,
    size,
    rectangle,
    insets,
    color,
    font,
    image,
    enumeration,
    object,
    collection,
};

} // namespace gui_forms
