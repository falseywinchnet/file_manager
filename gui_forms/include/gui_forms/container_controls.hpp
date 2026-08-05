#pragma once

#include "gui_forms/control.hpp"

namespace gui_forms {

// The first reusable focus-container identity. Validation, scaling, scrolling,
// and dialog-key routing remain separate contracts owned by later M5/M6 slices.
class ContainerControl : public Control {
public:
    explicit ContainerControl(StableId stable_id);

    [[nodiscard]] bool contains_descendant(const Control::Ptr& control) const noexcept;
    [[nodiscard]] Control::Ptr active_control() const noexcept;
    bool request_active_control(const Control::Ptr& control);
    bool clear_active_control();
};

// A retained composition root with a one-shot lifetime load notification and
// a successful whole-subtree attachment count.
class UserControl : public ContainerControl {
public:
    explicit UserControl(StableId stable_id);

    [[nodiscard]] Event<>& loaded() noexcept { return loaded_event_; }
    [[nodiscard]] bool is_loaded() const noexcept { return loaded_; }
    [[nodiscard]] bool is_attached() const noexcept { return attached_; }
    [[nodiscard]] std::uint64_t attachment_count() const noexcept {
        return attachment_count_;
    }

protected:
    void on_attached_to_window() override;
    void on_attachment_committed() noexcept override;
    void on_detached_from_window() noexcept override;

private:
    Event<> loaded_event_;
    std::uint64_t attachment_count_{};
    bool loaded_{};
    bool attached_{};
};

} // namespace gui_forms
