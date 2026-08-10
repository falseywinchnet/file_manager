#pragma once

#include <cstdint>

namespace gui_forms {

enum class HostLifecyclePhase : std::uint8_t {
    constructed,
    attached,
    close_authorized,
    closed,
    shutdown,
};

} // namespace gui_forms
