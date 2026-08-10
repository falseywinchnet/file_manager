#pragma once

#include "gui_forms/controls/scrollable_control/scrollable_control.hpp"

namespace gui_forms {

class ContainerControl : public ScrollableControl {
public:
    explicit ContainerControl(StableId stable_id);

    [[nodiscard]] bool contains_descendant(const Control::Ptr& control) const noexcept;
    [[nodiscard]] Control::Ptr active_control() const noexcept;
    bool request_active_control(const Control::Ptr& control);
    bool clear_active_control();
    [[nodiscard]] AutoValidate auto_validate() const noexcept {
        return auto_validate_;
    }
    [[nodiscard]] AutoValidate effective_auto_validate() const noexcept;
    void set_auto_validate(AutoValidate value);
    [[nodiscard]] Event<AutoValidate>& auto_validate_changed() noexcept {
        return auto_validate_changed_;
    }
    bool validate(bool check_auto_validate = false);
    bool validate_children(
        ValidationConstraints constraints = ValidationConstraints::selectable);
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    [[nodiscard]] AutoValidate authored_auto_validate() const noexcept override {
        return auto_validate_;
    }
    AutoValidate auto_validate_{AutoValidate::inherit};
    Event<AutoValidate> auto_validate_changed_;
};

} // namespace gui_forms
