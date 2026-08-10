#pragma once

#include "gui_forms/control.hpp"

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace gui_forms {

struct StaticNode {
    std::string type;
    std::string stable_id;
    Rect requested_bounds{};
    std::vector<StaticNode> children;
};

class ControlFactory {
public:
    using Creator = std::function<Control::Ptr(StableId)>;

    void register_type(std::string type, Creator creator);
    [[nodiscard]] Control::Ptr create(std::string_view type, StableId stable_id) const;

private:
    using CreatorMap = std::unordered_map<std::string, Creator>;
    CreatorMap creators_;
};

[[nodiscard]] Control::Ptr build_static_tree(const StaticNode& node,
                                             const ControlFactory& factory);

} // namespace gui_forms
