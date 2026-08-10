#pragma once

#include "gui_forms/input_controls.hpp"

namespace gui_forms::detail {

class PropertyEditorDropDownLayer final : public Panel {
public:
    explicit PropertyEditorDropDownLayer(StableId stable_id);

    [[nodiscard]] Event<>& dismissed() noexcept { return dismissed_; }
    void on_pointer(PointerEvent& event) override;
    void on_key_preview(KeyEvent& event) override;

private:
    Event<> dismissed_;
};

} // namespace gui_forms::detail
