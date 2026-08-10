#pragma once

#include <cstdint>

namespace gui_forms {

enum class PropertyEditorKind : std::uint8_t {
    read_only,
    text,
    choice,
    boolean,
    custom,
};

} // namespace gui_forms
