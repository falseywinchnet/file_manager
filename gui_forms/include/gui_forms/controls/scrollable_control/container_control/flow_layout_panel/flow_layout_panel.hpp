#pragma once

#include "gui_forms/controls/scrollable_control/container_control/container_control.hpp"

#include <cstdint>
#include <unordered_map>

namespace gui_forms {

enum class FlowDirection : std::uint8_t {
    left_to_right,
    right_to_left,
    top_down,
    bottom_up,
};

class FlowLayoutPanel final : public ContainerControl {
public:
    explicit FlowLayoutPanel(StableId stable_id);

    [[nodiscard]] FlowDirection flow_direction() const noexcept {
        return flow_direction_;
    }
    void set_flow_direction(FlowDirection direction);
    [[nodiscard]] bool wrap_contents() const noexcept { return wrap_contents_; }
    void set_wrap_contents(bool wrap);
    [[nodiscard]] Size item_spacing() const noexcept { return item_spacing_; }
    void set_item_spacing(Size spacing);
    [[nodiscard]] bool auto_size() const noexcept override {
        return Control::auto_size();
    }
    void set_auto_size(bool auto_size) override;
    void set_flow_break(const Control& child, bool flow_break);
    [[nodiscard]] bool flow_break(const Control& child) const;

    [[nodiscard]] Size measure(Size available) override;
    void arrange(Rect final_bounds) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    using FlowBreakMap = std::unordered_map<std::uint64_t, bool>;
    [[nodiscard]] Size layout_children(Size available, bool assign);
    void reconcile_flow_breaks();

    FlowBreakMap flow_breaks_;
    FlowDirection flow_direction_{FlowDirection::left_to_right};
    Size item_spacing_{};
    bool wrap_contents_{true};
};

} // namespace gui_forms
