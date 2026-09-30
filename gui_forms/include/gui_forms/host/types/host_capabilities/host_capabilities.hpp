#pragma once

#include "gui_forms/host/types/host_capability/host_capability.hpp"

#include <cstdint>
#include <string>

namespace gui_forms {

struct HostCapabilities final {
    static constexpr std::uint32_t current_protocol_version = 7;

    std::uint32_t protocol_version{current_protocol_version};
    std::string platform{"unknown"};
    HostCapability available{HostCapability::none};

    [[nodiscard]] bool supports(HostCapability capability) const noexcept {
        return has_capability(available, capability);
    }
    [[nodiscard]] std::string to_json() const;
};

} // namespace gui_forms
