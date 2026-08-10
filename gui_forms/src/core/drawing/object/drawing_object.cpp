#include "../support/drawing_support.hpp"

DrawingObject::DrawingObject() : owner_thread_(std::this_thread::get_id()) {}
DrawingObject::~DrawingObject() = default;

void DrawingObject::dispose() {
    verify_access();
    if (state_ == ObjectState::disposed) return;
    state_ = ObjectState::disposing;
    on_dispose();
    state_ = ObjectState::disposed;
}

ObjectState DrawingObject::state() const {
    verify_access();
    return state_;
}

bool DrawingObject::is_disposed() const {
    verify_access();
    return state_ == ObjectState::disposed;
}

void DrawingObject::verify_access() const {
    if (std::this_thread::get_id() != owner_thread_) {
        throw std::logic_error("GUI.Drawing object accessed from a non-owner thread");
    }
}

void DrawingObject::require_alive() const {
    verify_access();
    if (state_ != ObjectState::alive) {
        throw std::logic_error("GUI.Drawing object is disposed");
    }
}

void DrawingObject::handoff_to_current_thread() {
    owner_thread_ = std::this_thread::get_id();
}

void DrawingObject::on_dispose() noexcept {}


} // namespace gui_drawing

