#include "gui_forms/controls/scrollable_control/container_control/tab_control/tab_page/tab_page.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>

namespace gui_forms {

TabPage::TabPage(StableId stable_id, std::string text)
    : Panel(std::move(stable_id)), text_(std::move(text)) {
    set_background(Color::rgba(255, 255, 255));
    set_border_style(BorderStyle::line);
}

void TabPage::set_text(std::string text) {
    require_mutable();
    if (text_ == text) return;
    text_ = std::move(text);
    invalidate(Dirty::paint | Dirty::semantics | Dirty::measure);
    if (const Control::Ptr owner = parent()) {
        owner->invalidate(Dirty::paint | Dirty::semantics | Dirty::measure);
    }
}

SemanticDescriptor TabPage::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::group;
    descriptor.name = accessible_name().empty() ? text_ : accessible_name();
    descriptor.description = accessible_description();
    descriptor.exposed = true;
    return descriptor;
}

} // namespace gui_forms
