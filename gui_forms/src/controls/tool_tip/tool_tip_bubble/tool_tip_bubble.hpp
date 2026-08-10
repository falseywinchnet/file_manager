#pragma once

#include "gui_forms/basic_controls.hpp"

namespace gui_forms {

class ToolTipBubble final : public Panel {
public:
    ToolTipBubble(StableId stable_id, std::string text);

    void initialize_control_tree();
    void set_content_size(Size size);
    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] bool hit_test_local(Point point) const override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    std::string text_;
    std::shared_ptr<Label> label_;
};

} // namespace gui_forms
