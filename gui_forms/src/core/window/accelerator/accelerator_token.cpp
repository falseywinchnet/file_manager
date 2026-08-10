#include "accelerator_attachment.hpp"

namespace gui_forms {

void AcceleratorToken::disconnect() noexcept {
    if (attachment_) {
        (*attachment_).disconnect();
        attachment_.reset();
    }
}

bool AcceleratorToken::connected() const noexcept {
    return attachment_ && (*attachment_).connected();
}

} // namespace gui_forms
