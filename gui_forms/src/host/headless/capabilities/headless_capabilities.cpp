#include "headless_capabilities.hpp"

#include <sstream>

namespace gui_forms::host {

HostCapabilities headless_capabilities() {
    return {HostCapabilities::current_protocol_version,
            "headless-reference",
            HostCapability::lifecycle |
                HostCapability::scale_notifications |
                HostCapability::occlusion |
                HostCapability::scheduled_wake |
                HostCapability::pointer_input |
                HostCapability::keyboard_input |
                HostCapability::text_composition |
                HostCapability::monitor_geometry |
                HostCapability::pointer_capture |
                HostCapability::cursor |
                HostCapability::clipboard |
                HostCapability::clipboard_images |
                HostCapability::typed_drag_destination |
                HostCapability::dialogs |
                HostCapability::sound_cues};
}
} // namespace gui_forms::host
