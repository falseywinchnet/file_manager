#pragma once

#include "gui_forms/controls/button_base/button_base.hpp"

#include <string>

namespace gui_forms {

class RadioButton : public ButtonBase {
public:
    explicit RadioButton(StableId stable_id, std::string text = {});

    [[nodiscard]] bool checked() const noexcept { return checked_; }
    void set_checked(bool checked);
    [[nodiscard]] const std::string& group_name() const noexcept { return group_name_; }
    void set_group_name(std::string name);
    [[nodiscard]] bool auto_check() const noexcept { return auto_check_; }
    void set_auto_check(bool enabled);
    [[nodiscard]] ChoiceIndicatorStyle indicator_style() const noexcept {
        return indicator_style_;
    }
    void set_indicator_style(ChoiceIndicatorStyle style);
    [[nodiscard]] Event<bool>& checked_changed() noexcept {
        return checked_changed_;
    }

    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] Insets visual_outsets() const noexcept override;
    void on_activate() override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    void set_checked_without_exclusion(bool checked);

    std::string group_name_;
    Event<bool> checked_changed_;
    bool checked_{};
    bool auto_check_{true};
    ChoiceIndicatorStyle indicator_style_{ChoiceIndicatorStyle::classic};
};

} // namespace gui_forms
