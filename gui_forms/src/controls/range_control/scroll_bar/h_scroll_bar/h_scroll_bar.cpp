#include "gui_forms/controls/range_control/scroll_bar/h_scroll_bar/h_scroll_bar.hpp"

#include <utility>

namespace gui_forms {

HScrollBar::HScrollBar(StableId stable_id)
    : ScrollBar(std::move(stable_id), Orientation::horizontal) {}

} // namespace gui_forms
