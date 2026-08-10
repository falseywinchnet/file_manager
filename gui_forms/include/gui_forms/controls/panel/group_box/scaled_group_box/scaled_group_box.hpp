#pragma once

#include "gui_forms/controls/panel/group_box/group_box.hpp"

#include <optional>
#include <string>
#include <unordered_map>

namespace gui_forms {

class ScaledGroupBox final : public GroupBox {
public:
    explicit ScaledGroupBox(StableId stable_id, std::string text = {},
                            Size design_size = {1.0, 1.0});

    [[nodiscard]] Size design_size() const noexcept { return design_size_; }
    void set_design_size(Size size);
    void add_at(Control::Ptr child, Rect design_bounds);
    void set_design_bounds(const Control& child, Rect design_bounds);
    [[nodiscard]] std::optional<Rect> design_bounds(const Control& child) const;
    void arrange(Rect final_bounds) override;

private:
    void reconcile_slots();
    Size design_size_;
    std::unordered_map<std::uint64_t, Rect> slots_;
};

} // namespace gui_forms
