#pragma once

#include "gui_forms/controls/panel/panel.hpp"
#include "gui_forms/controls/panel/list_box/list_box.hpp"
#include "gui_forms/controls/range_control/scroll_bar/scroll_bar.hpp"

namespace gui_forms {

class DropDownList final : public ListBox {
public:
    static constexpr bool initialize_tree_after_construction = true;
    explicit DropDownList(StableId stable_id);
    void initialize_control_tree();
    void arrange(Rect bounds) override;
    void on_pointer(PointerEvent& event) override;
    void on_key(KeyEvent& event) override;
    bool on_semantic_child_action(std::string_view id, SemanticAction action,
                                  std::string_view value) override;
private:
    void synchronize_scroll();
    void scroll_changed(double value);
    std::shared_ptr<ScrollBar> scrollbar_;
    SubscriptionToken scroll_subscription_;
    bool synchronizing_ = false;
};

class DropDownLayer final : public Panel {
public:
    explicit DropDownLayer(StableId stable_id);
    [[nodiscard]] Event<>& dismissed() noexcept { return dismissed_; }
    void on_pointer(PointerEvent& event) override;
    void on_key_preview(KeyEvent& event) override;

private:
    Event<> dismissed_;
};

} // namespace gui_forms
