#include "gui_forms/host/types/host_capabilities/host_capabilities.hpp"

#include <array>
#include <sstream>
#include <utility>

namespace gui_forms {
namespace {

std::string json_escape(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    for (const char character : text) {
        switch (character) {
        case '\\': result += "\\\\"; break;
        case '"': result += "\\\""; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default: result += character; break;
        }
    }
    return result;
}

} // namespace

std::string HostCapabilities::to_json() const {
    constexpr std::array<std::pair<HostCapability, const char*>, 18> names{{
        {HostCapability::lifecycle, "lifecycle"},
        {HostCapability::scale_notifications, "scale_notifications"},
        {HostCapability::monitor_geometry, "monitor_geometry"},
        {HostCapability::occlusion, "occlusion"},
        {HostCapability::scheduled_wake, "scheduled_wake"},
        {HostCapability::pointer_input, "pointer_input"},
        {HostCapability::keyboard_input, "keyboard_input"},
        {HostCapability::text_composition, "text_composition"},
        {HostCapability::pointer_capture, "pointer_capture"},
        {HostCapability::cursor, "cursor"},
        {HostCapability::clipboard, "clipboard"},
        {HostCapability::typed_drag_destination, "typed_drag_destination"},
        {HostCapability::dialogs, "dialogs"},
        {HostCapability::menus, "menus"},
        {HostCapability::font_discovery, "font_discovery"},
        {HostCapability::accessibility, "accessibility"},
        {HostCapability::typed_drag_source, "typed_drag_source"},
        {HostCapability::sound_cues, "sound_cues"},
    }};
    std::ostringstream output;
    output << "{\"protocol_version\":" << protocol_version
           << ",\"platform\":\"" << json_escape(platform)
           << "\",\"available_bits\":"
           << static_cast<std::uint64_t>(available) << ",\"capabilities\":[";
    bool first = true;
    for (const std::pair<HostCapability, const char*>& named_capability : names) {
        const HostCapability capability = named_capability.first;
        const char* const name = named_capability.second;
        if (!supports(capability)) continue;
        output << (first ? "\"" : ",\"") << name << '"';
        first = false;
    }
    output << "]}";
    return output.str();
}

} // namespace gui_forms
