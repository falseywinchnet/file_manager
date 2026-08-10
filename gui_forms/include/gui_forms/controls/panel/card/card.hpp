#pragma once

#include "gui_forms/controls/panel/panel.hpp"

#include <cstdint>
#include <string_view>

namespace gui_forms {

struct CardLayout final {
    Insets padding{12.0, 10.0, 12.0, 10.0};
    double section_gap{8.0};
    double header_extent{30.0};
    double footer_extent{30.0};

    friend constexpr bool operator==(const CardLayout&,
                                     const CardLayout&) = default;
};

// Controls whether activation is purely notificational or also updates the
// retained selection state before Card::activated is published. The default
// preserves the original manual-selection behavior.
enum class CardSelectionBehavior : std::uint8_t {
    manual,
    select_on_activation,
    toggle_on_activation,
};

// General retained header/body/footer composition. Card owns each installed
// section as an ordinary visual child and returns the detached predecessor on
// replacement; it never hides demo-only layout or renderer code.
class Card : public Panel {
public:
    explicit Card(StableId stable_id);

    [[nodiscard]] Control::Ptr header() const noexcept { return header_; }
    [[nodiscard]] Control::Ptr body() const noexcept { return body_; }
    [[nodiscard]] Control::Ptr footer() const noexcept { return footer_; }
    [[nodiscard]] Control::Ptr set_header(Control::Ptr control);
    [[nodiscard]] Control::Ptr set_body(Control::Ptr control);
    [[nodiscard]] Control::Ptr set_footer(Control::Ptr control);

    [[nodiscard]] const CardLayout& card_layout() const noexcept { return layout_; }
    [[nodiscard]] bool uses_theme_layout() const noexcept {
        return uses_theme_layout_;
    }
    [[nodiscard]] CardLayout effective_card_layout() const noexcept;
    void set_card_layout(CardLayout layout);
    void reset_card_layout_to_theme();
    [[nodiscard]] bool interactive() const noexcept { return interactive_; }
    void set_interactive(bool interactive);
    [[nodiscard]] bool selected() const noexcept { return selected_; }
    void set_selected(bool selected);
    [[nodiscard]] CardSelectionBehavior selection_behavior() const noexcept {
        return selection_behavior_;
    }
    void set_selection_behavior(CardSelectionBehavior behavior);
    [[nodiscard]] bool hovered_visual() const noexcept { return hovered_; }
    [[nodiscard]] bool pressed_visual() const noexcept { return pressed_; }
    [[nodiscard]] bool focused_visual() const noexcept { return focused_; }
    [[nodiscard]] Event<Card&>& activated() noexcept { return activated_; }
    [[nodiscard]] Event<bool>& selected_changed() noexcept {
        return selected_changed_;
    }

    [[nodiscard]] Size measure(Size available) override;
    void arrange(Rect final_bounds) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] Insets visual_outsets() const noexcept override;
    [[nodiscard]] bool hit_test_local(Point local_point) const override;
    void on_pointer(PointerEvent& event) override;
    void on_key(KeyEvent& event) override;
    void on_focus_changed(bool focused) override;
    void on_activate() override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    bool on_semantic_action(SemanticAction action,
                            std::string_view value) override;

private:
    Control::Ptr replace_section(Control::Ptr& slot, Control::Ptr replacement);
    [[nodiscard]] ControlVisualContext current_context() const noexcept;

    Control::Ptr header_;
    Control::Ptr body_;
    Control::Ptr footer_;
    CardLayout layout_;
    Event<Card&> activated_;
    Event<bool> selected_changed_;
    CardSelectionBehavior selection_behavior_{CardSelectionBehavior::manual};
    std::uint32_t keyboard_key_{};
    bool interactive_{};
    bool selected_{};
    bool hovered_{};
    bool pressed_{};
    bool focused_{};
    bool uses_theme_layout_{true};
};

} // namespace gui_forms
