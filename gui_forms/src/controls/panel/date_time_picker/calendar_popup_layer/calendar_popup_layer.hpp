#pragma once

#include "gui_forms/controls/panel/panel.hpp"

namespace gui_forms {

class CalendarPopupLayer final : public Panel {
public:
    explicit CalendarPopupLayer(StableId stable_id);
    [[nodiscard]] Event<>& dismissed() noexcept { return dismissed_; }
    void on_pointer(PointerEvent& event) override;

private:
    Event<> dismissed_;
};

} // namespace gui_forms
