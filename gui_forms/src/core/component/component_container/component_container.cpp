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
    ComponentList::iterator found = components_.begin();
    while (found != components_.end() && (*found).get() != &component) {
        ++found;
    }
    if (found == components_.end()) {
        return {};
    }
    std::shared_ptr<Component> removed = std::move(*found);
    components_.erase(found);
    return removed;
}

bool ComponentContainer::contains(const Component& component) const noexcept {
    for (const Component::Ptr& item : components_) {
        if (item.get() == &component) return true;
    }
    return false;
}

void ComponentContainer::dispose() {
    if (disposed_) {
        return;
    }
    disposed_ = true;
    std::vector<std::shared_ptr<gui_forms::Component>> owned = std::exchange(components_, {});
    for (const std::shared_ptr<gui_forms::Component>& component : owned) {
        (*component).dispose();
    }
}

} // namespace gui_forms
