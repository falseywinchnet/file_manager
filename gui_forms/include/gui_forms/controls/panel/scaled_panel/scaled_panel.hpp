#pragma once

#include "gui_forms/controls/panel/panel.hpp"

#include <optional>
#include <unordered_map>

namespace gui_forms {

class ScaledPanel : public Panel {
public:
    explicit ScaledPanel(StableId stable_id, Size design_size = {1.0, 1.0});

    [[nodiscard]] Size design_size() const noexcept { return design_size_; }
    void set_design_size(Size size);
    void add_at(Control::Ptr child, Rect design_bounds);
    void set_design_bounds(const Control& child, Rect design_bounds);
    [[nodiscard]] std::optional<Rect> design_bounds(const Control& child) const;
    void arrange(Rect final_bounds) override;

private:
    using SlotMap = std::unordered_map<std::uint64_t, Rect>;
    void reconcile_slots();
    Size design_size_;
    SlotMap slots_;
};

} // namespace gui_forms
