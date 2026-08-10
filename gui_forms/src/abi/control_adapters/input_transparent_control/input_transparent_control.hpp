#pragma once

#include "../support/abi_control_adapter_support.hpp"

namespace gui_forms::abi::detail {

// Some compatibility widgets participate in retained layout and painting order
// but are not interactive surfaces. In particular, DockPanelSuite creates an
// empty auto-hide strip covering its entire client area. Keeping this policy in
// a dedicated ABI kind avoids encoding third-party names in the portable core.
class InputTransparentControl final : public Control {
public:
    ~InputTransparentControl() override;

    explicit InputTransparentControl(StableId stable_id)
        : Control(std::move(stable_id)) {}

    [[nodiscard]] bool hit_test_local(gui_forms::Point) const override {
        return false;
    }
};

} // namespace gui_forms::abi::detail

