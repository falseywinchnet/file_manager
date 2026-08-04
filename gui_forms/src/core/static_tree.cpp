#include "gui_forms/static_tree.hpp"

#include <stdexcept>

namespace gui_forms {

void ControlFactory::register_type(std::string type, Creator creator) {
    if (type.empty() || !creator) {
        throw std::invalid_argument("GUI.Forms control registration requires type and creator");
    }
    const auto [iterator, inserted] = creators_.emplace(std::move(type), std::move(creator));
    if (!inserted) {
        throw std::logic_error("GUI.Forms control type is already registered: " + iterator->first);
    }
}

Control::Ptr ControlFactory::create(std::string_view type, StableId stable_id) const {
    const auto found = creators_.find(std::string(type));
    if (found == creators_.end()) {
        throw std::out_of_range("GUI.Forms unknown static control type: " + std::string(type));
    }
    Control::Ptr result = found->second(std::move(stable_id));
    if (!result) {
        throw std::logic_error("GUI.Forms control factory returned null");
    }
    return result;
}

Control::Ptr build_static_tree(const StaticNode& node, const ControlFactory& factory) {
    Control::Ptr control = factory.create(node.type, StableId(node.stable_id));
    control->set_requested_bounds(node.requested_bounds);
    for (const StaticNode& child_node : node.children) {
        control->add_child(build_static_tree(child_node, factory));
    }
    return control;
}

} // namespace gui_forms
