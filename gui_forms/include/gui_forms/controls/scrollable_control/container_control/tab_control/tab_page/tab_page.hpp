#pragma once

#include "gui_forms/controls/panel/panel.hpp"

#include <string>

namespace gui_forms {

class TabPage final : public Panel {
public:
    explicit TabPage(StableId stable_id, std::string text = {});

    [[nodiscard]] const std::string& text() const noexcept { return text_; }
    void set_text(std::string text);
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    std::string text_;
};

} // namespace gui_forms
