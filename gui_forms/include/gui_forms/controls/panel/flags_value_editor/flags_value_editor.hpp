#pragma once

#include "gui_forms/inspection/inspection_types.hpp"

#include <cstddef>
#include <memory>
#include <vector>

namespace gui_forms {

class FlagsValueEditor final : public Panel {
public:
    FlagsValueEditor(StableId stable_id, PropertyEnumDescriptor descriptor,
                     PropertyEnumValue value);

    [[nodiscard]] const PropertyEnumDescriptor& descriptor() const noexcept {
        return descriptor_;
    }
    void set_descriptor(PropertyEnumDescriptor descriptor);
    [[nodiscard]] const PropertyEnumValue& value() const noexcept {
        return value_;
    }
    void set_value(PropertyEnumValue value);
    [[nodiscard]] bool dropped_down() const noexcept { return dropped_down_; }
    void set_dropped_down(bool dropped_down);
    [[nodiscard]] double popup_width() const noexcept { return popup_width_; }
    void set_popup_width(double width);
    [[nodiscard]] Event<const PropertyEnumValue&>& value_changed() noexcept {
        return value_changed_;
    }
    [[nodiscard]] Event<bool>& drop_down_changed() noexcept {
        return drop_down_changed_;
    }

    void on_paint(Painter& painter, Rect local_damage) override;
    void on_pointer(PointerEvent& event) override;
    void on_key(KeyEvent& event) override;
    void on_focus_changed(bool focused) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    bool on_semantic_action(SemanticAction action,
                            std::string_view value) override;

protected:
    void on_detached_from_window() noexcept override;

private:
    void open_drop_down();
    void close_drop_down();
    void on_popup_revoked();
    void synchronize_popup();
    void apply_popup_choice(std::size_t popup_index, CheckState state);

    PropertyEnumDescriptor descriptor_;
    PropertyEnumValue value_;
    std::shared_ptr<Panel> popup_layer_;
    std::shared_ptr<CheckedListBox> popup_list_;
    std::vector<std::size_t> popup_choice_indices_;
    std::uint64_t popup_scope_{};
    PopupToken popup_token_;
    SubscriptionToken popup_check_;
    SubscriptionToken popup_dismissal_;
    SubscriptionToken popup_revocation_;
    double popup_width_{240.0};
    bool dropped_down_{};
    bool focused_{};
    bool synchronizing_popup_{};
    bool closing_popup_{};
    Event<const PropertyEnumValue&> value_changed_;
    Event<bool> drop_down_changed_;
};

} // namespace gui_forms
