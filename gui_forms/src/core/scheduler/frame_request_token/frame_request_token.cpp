#include "gui_forms/scheduler/frame_request_token/frame_request_token.hpp"

#include <utility>

namespace gui_forms {

FrameRequestToken::~FrameRequestToken() { disconnect(); }

FrameRequestToken::FrameRequestToken(FrameRequestToken&& other) noexcept
    : revocable_(std::move(other.revocable_)) {}

FrameRequestToken& FrameRequestToken::operator=(
    FrameRequestToken&& other) noexcept {
    if (this != &other) {
        disconnect();
        revocable_ = std::move(other.revocable_);
    }
    return *this;
}

FrameRequestToken::FrameRequestToken(
    std::shared_ptr<detail::Revocable> revocable)
    : revocable_(std::move(revocable)) {}

void FrameRequestToken::disconnect() noexcept {
    if (revocable_) {
        (*revocable_).disconnect();
        revocable_.reset();
    }
}

bool FrameRequestToken::connected() const noexcept {
    return revocable_ != nullptr && (*revocable_).connected();
}

} // namespace gui_forms
