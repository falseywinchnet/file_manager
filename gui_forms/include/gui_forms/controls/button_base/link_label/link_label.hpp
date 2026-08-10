#pragma once

#include "gui_forms/controls/button_base/button_base.hpp"

namespace gui_forms {

class LinkLabel : public ButtonBase {
public:
    explicit LinkLabel(StableId stable_id, std::string text = {});

    [[nodiscard]] bool visited() const noexcept { return visited_; }
    void set_visited(bool visited);
    void on_paint(Painter& painter, Rect local_damage) override;
    void on_activate() override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    bool visited_{};
};

} // namespace gui_forms
