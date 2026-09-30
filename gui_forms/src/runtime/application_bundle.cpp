#include "gui_forms/application.hpp"

namespace gui_forms::host {

// This translation unit gives the installed native application boundary a
// stable owner. Public implementation is force-loaded from GUI.Forms' private
// archives by CMake; no browser or framework runtime crosses this boundary.
void application_bundle_link_anchor() noexcept {}

} // namespace gui_forms::host
