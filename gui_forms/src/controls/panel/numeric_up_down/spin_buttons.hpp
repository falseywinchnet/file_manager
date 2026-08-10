#pragma once

#include "gui_forms/control.hpp"

namespace gui_forms {

class SpinButtons final : public Control {
public:
    explicit SpinButtons(StableId stable_id);
    [[nodiscard]] Event<int>& stepped() noexcept { return stepped_; }
    void on_paint(Painter& painter, Rect local_damage) override;
    void on_pointer(PointerEvent& event) override;

private:
    Event<int> stepped_;
};

} // namespace gui_forms
