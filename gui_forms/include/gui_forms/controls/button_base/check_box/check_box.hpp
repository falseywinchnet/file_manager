#pragma once

#include "gui_forms/controls/button_base/button_base.hpp"

#include <cstdint>

namespace gui_forms {

enum class CheckState : std::uint8_t {
    unchecked,
    checked,
    indeterminate,
};

enum class CheckBoxAppearance : std::uint8_t {
    normal,
    button,
};

class CheckBox : public ButtonBase {
public:
    explicit CheckBox(StableId stable_id, std::string text = {});

    [[nodiscard]] CheckState check_state() const noexcept { return check_state_; }
    void set_check_state(CheckState state);
    [[nodiscard]] bool checked() const noexcept {
        return check_state_ == CheckState::checked;
    }
    void set_checked(bool checked);
    [[nodiscard]] bool three_state() const noexcept { return three_state_; }
    void set_three_state(bool enabled);
    [[nodiscard]] bool auto_check() const noexcept { return auto_check_; }
    void set_auto_check(bool enabled);
    [[nodiscard]] CheckBoxAppearance appearance() const noexcept {
        return appearance_;
    }
    // Button appearance changes only the visual projection. CheckBox retains
    // its check-box role, checked state, command path, focus, and hit target.
    void set_appearance(CheckBoxAppearance appearance);
    [[nodiscard]] ChoiceIndicatorStyle indicator_style() const noexcept {
        return indicator_style_;
    }
    void set_indicator_style(ChoiceIndicatorStyle style);
    [[nodiscard]] Event<CheckState>& check_state_changed() noexcept {
        return check_state_changed_;
    }
    [[nodiscard]] Event<bool>& checked_changed() noexcept {
        return checked_changed_;
    }

    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] Insets visual_outsets() const noexcept override;
    void on_activate() override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    CheckState check_state_{CheckState::unchecked};
    Event<CheckState> check_state_changed_;
    Event<bool> checked_changed_;
    bool three_state_{};
    bool auto_check_{true};
    CheckBoxAppearance appearance_{CheckBoxAppearance::normal};
    ChoiceIndicatorStyle indicator_style_{ChoiceIndicatorStyle::classic};
};

} // namespace gui_forms
