#include "gui_forms/control/stable_id/stable_id.hpp"

#include <stdexcept>
#include <utility>

namespace gui_forms {

StableId::StableId(std::string value) : value_(std::move(value)) {
    if (value_.empty()) {
        throw std::invalid_argument("GUI.Forms stable IDs may not be empty");
    }
}

} // namespace gui_forms
