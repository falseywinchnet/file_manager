#pragma once

#include "gui_forms/gui_forms.hpp"

#include <memory>
#include <string>
#include <vector>

namespace gui_forms::showcase {

struct ShowcaseTree final {
    Control::Ptr root;
};

[[nodiscard]] ShowcaseTree build_showcase_tree();
void initialize_showcase_runtime(Window& window);

} // namespace gui_forms::showcase
