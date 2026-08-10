#pragma once

#include "gui_forms/controls/range_control/scroll_bar/scroll_bar.hpp"

namespace gui_forms {

class HScrollBar final : public ScrollBar {
public:
    explicit HScrollBar(StableId stable_id);
};

} // namespace gui_forms
