#include "gui_forms/controls/scrollable_control/container_control/split_container/splitter_panel/splitter_panel.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>

namespace gui_forms {

SplitterPanel::SplitterPanel(StableId stable_id)
    : ContainerControl(std::move(stable_id)) {}

void SplitterPanel::set_background(Color color) {
    require_mutable();
    if (background_ == color) return;
    background_ = color;
    invalidate(invalidation::style_only);
}

void SplitterPanel::on_paint(Painter& painter, Rect) {
    if (background_.alpha == 0U) return;
    const Rect bounds = committed_arranged_bounds();
    painter.fill_rect({0.0, 0.0, bounds.width, bounds.height}, background_);
}

} // namespace gui_forms
