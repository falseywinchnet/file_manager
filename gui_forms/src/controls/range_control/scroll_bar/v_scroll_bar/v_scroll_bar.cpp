#include "gui_forms/controls/range_control/scroll_bar/v_scroll_bar/v_scroll_bar.hpp"

#include <utility>

namespace gui_forms {

VScrollBar::VScrollBar(StableId stable_id)
    : ScrollBar(std::move(stable_id), Orientation::vertical) {}

} // namespace gui_forms
