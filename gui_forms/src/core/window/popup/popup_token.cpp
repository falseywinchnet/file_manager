#include "popup_attachment.hpp"

namespace gui_forms {

void PopupToken::disconnect() noexcept {
    if (attachment_) {
        attachment_->disconnect();
        attachment_.reset();
    }
}

bool PopupToken::connected() const noexcept {
    return attachment_ && attachment_->connected();
}

Event<>* PopupToken::closed_event() noexcept {
    return attachment_ ? &attachment_->closed() : nullptr;
}

} // namespace gui_forms
