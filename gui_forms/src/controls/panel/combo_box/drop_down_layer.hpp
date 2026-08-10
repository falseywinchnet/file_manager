#pragma once

#include "gui_forms/controls/panel/panel.hpp"

namespace gui_forms {

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
