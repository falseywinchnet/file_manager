#pragma once

#include "gui_forms/input_controls.hpp"

#include <functional>
#include <memory>
#include <vector>

namespace gui_forms::detail {

// Source-private retained module shell. InstrumentRack owns model authority;
// this type owns one module's responsive child layout and reorder semantics.
class RackModulePanel final : public Panel {
public:
    explicit RackModulePanel(StableId stable_id);

    std::function<void(bool)> move_request;
    std::shared_ptr<CheckBox> enable;
    std::vector<Control::Ptr> fields;
    std::vector<double> field_weights;
    std::shared_ptr<Label> status;
    std::shared_ptr<Button> remove;

    void set_compact(bool compact);
    [[nodiscard]] bool compact() const noexcept { return compact_; }

    void arrange(Rect final_bounds) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    bool on_semantic_action(SemanticAction action,
                            std::string_view value) override;

private:
    bool compact_{};
};

} // namespace gui_forms::detail
