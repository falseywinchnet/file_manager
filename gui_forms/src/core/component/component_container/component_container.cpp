#include "gui_forms/component/component_container/component_container.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace gui_forms {

ComponentContainer::~ComponentContainer() {
    dispose();
}

void ComponentContainer::add(Component::Ptr component) {
    if (disposed_) {
        throw std::logic_error(
            "GUI.Forms cannot add to a disposed component container");
    }
    if (!component) {
        throw std::invalid_argument("GUI.Forms cannot own a null component");
    }
    if (contains(*component)) {
        return;
    }
    components_.push_back(std::move(component));
}

Component::Ptr ComponentContainer::remove(const Component& component) {
    const auto found = std::find_if(
        components_.begin(), components_.end(),
        [&](const auto& item) { return item.get() == &component; });
    if (found == components_.end()) {
        return {};
    }
    auto removed = std::move(*found);
    components_.erase(found);
    return removed;
}

bool ComponentContainer::contains(const Component& component) const noexcept {
    return std::any_of(components_.begin(), components_.end(),
                       [&](const auto& item) {
                           return item.get() == &component;
                       });
}

void ComponentContainer::dispose() {
    if (disposed_) {
        return;
    }
    disposed_ = true;
    auto owned = std::exchange(components_, {});
    for (const auto& component : owned) {
        component->dispose();
    }
}

} // namespace gui_forms
