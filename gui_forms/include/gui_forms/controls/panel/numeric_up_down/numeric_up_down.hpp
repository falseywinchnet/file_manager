#pragma once

#include "gui_forms/controls/panel/text_box/text_box.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace gui_forms {

class NumericUpDown final : public Panel {
public:
    static constexpr bool initialize_tree_after_construction = true;
    explicit NumericUpDown(StableId stable_id);
    void initialize_control_tree();

    [[nodiscard]] double minimum() const noexcept { return minimum_; }
    [[nodiscard]] double maximum() const noexcept { return maximum_; }
    void set_range(double minimum, double maximum);
    [[nodiscard]] double value() const noexcept { return value_; }
    void set_value(double value);
    [[nodiscard]] double increment() const noexcept { return increment_; }
    void set_increment(double increment);
    [[nodiscard]] std::uint8_t decimal_places() const noexcept {
        return decimal_places_;
    }
    void set_decimal_places(std::uint8_t places);
    [[nodiscard]] bool hexadecimal() const noexcept { return hexadecimal_; }
    void set_hexadecimal(bool hexadecimal);
    [[nodiscard]] double button_width() const noexcept { return button_width_; }
    void set_button_width(double width);
    [[nodiscard]] std::shared_ptr<TextBox> editor() const noexcept { return editor_; }
    [[nodiscard]] Event<double>& value_changed() noexcept { return value_changed_; }

    void arrange(Rect final_bounds) override;
    void on_key_preview(KeyEvent& event) override;
    void on_pointer_preview(PointerEvent& event) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    bool on_semantic_action(SemanticAction action,
                            std::string_view value) override;

private:
    struct EditorChangeCallback final {
        std::weak_ptr<NumericUpDown> target;
        void operator()(const std::string&) const;
    };
    struct SpinnerStepCallback final {
        std::weak_ptr<NumericUpDown> target;
        void operator()(int direction) const;
    };

    void step(int direction);
    void commit_editor_text();
    void synchronize_editor();
    [[nodiscard]] std::string formatted_value() const;

    std::shared_ptr<TextBox> editor_;
    Control::Ptr spinner_;
    SubscriptionToken editor_change_;
    SubscriptionToken spinner_step_;
    double minimum_{0.0};
    double maximum_{100.0};
    double value_{};
    double increment_{1.0};
    double button_width_{18.0};
    std::uint8_t decimal_places_{};
    bool hexadecimal_{};
    bool synchronizing_{};
    Event<double> value_changed_;
};

} // namespace gui_forms
