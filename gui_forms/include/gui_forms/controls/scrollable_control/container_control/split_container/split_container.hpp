#pragma once

#include "gui_forms/controls/scrollable_control/container_control/container_control.hpp"
#include "gui_forms/controls/scrollable_control/container_control/split_container/splitter_panel/splitter_panel.hpp"
#include "gui_forms/range_controls.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>

namespace gui_forms {

enum class SplitFixedPanel : std::uint8_t {
    none,
    first,
    second,
};

enum class SplitChangeReason : std::uint8_t {
    programmatic,
    pointer,
    keyboard,
    collapse,
    container_resize,
};

enum class SplitCollapseOrigin : std::uint8_t {
    none,
    programmatic,
    user,
    automatic_accommodation,
};

struct SplitChangeEvent final {
    double old_distance{};
    double new_distance{};
    SplitChangeReason reason{SplitChangeReason::programmatic};
    SplitCollapseOrigin collapse_origin{SplitCollapseOrigin::none};
};

class SplitContainer final : public ContainerControl {
public:
    explicit SplitContainer(StableId stable_id);
    void initialize_control_tree();

    [[nodiscard]] std::shared_ptr<SplitterPanel> first_panel() const noexcept {
        return first_panel_;
    }
    [[nodiscard]] std::shared_ptr<SplitterPanel> second_panel() const noexcept {
        return second_panel_;
    }
    [[nodiscard]] Control::Ptr splitter_control() const noexcept {
        return splitter_;
    }

    [[nodiscard]] Orientation orientation() const noexcept { return orientation_; }
    void set_orientation(Orientation orientation);
    [[nodiscard]] double splitter_distance() const noexcept {
        return effective_distance_;
    }
    void set_splitter_distance(double distance);
    [[nodiscard]] double splitter_width() const noexcept { return splitter_width_; }
    void set_splitter_width(double width);
    [[nodiscard]] double splitter_hit_width() const noexcept {
        return splitter_hit_width_;
    }
    void set_splitter_hit_width(double width);
    [[nodiscard]] double first_minimum() const noexcept { return first_minimum_; }
    void set_first_minimum(double extent);
    [[nodiscard]] double second_minimum() const noexcept { return second_minimum_; }
    void set_second_minimum(double extent);
    [[nodiscard]] std::optional<double> first_maximum() const noexcept {
        return first_maximum_;
    }
    void set_first_maximum(std::optional<double> extent);
    [[nodiscard]] std::optional<double> second_maximum() const noexcept {
        return second_maximum_;
    }
    void set_second_maximum(std::optional<double> extent);
    [[nodiscard]] bool first_collapsed() const noexcept { return first_collapsed_; }
    [[nodiscard]] SplitCollapseOrigin first_collapse_origin() const noexcept {
        return first_collapse_origin_;
    }
    void set_first_collapsed(
        bool collapsed,
        SplitCollapseOrigin origin = SplitCollapseOrigin::programmatic);
    [[nodiscard]] bool second_collapsed() const noexcept { return second_collapsed_; }
    [[nodiscard]] SplitCollapseOrigin second_collapse_origin() const noexcept {
        return second_collapse_origin_;
    }
    void set_second_collapsed(
        bool collapsed,
        SplitCollapseOrigin origin = SplitCollapseOrigin::programmatic);
    [[nodiscard]] bool splitter_fixed() const noexcept { return splitter_fixed_; }
    void set_splitter_fixed(bool fixed);
    [[nodiscard]] SplitFixedPanel fixed_panel() const noexcept { return fixed_panel_; }
    void set_fixed_panel(SplitFixedPanel panel);
    // Optional seam-tab authority. Unlike fixed_panel(), this identifies the
    // pane toggled by a compact splitter tab and may remain operable while
    // ordinary splitter resizing is fixed.
    [[nodiscard]] SplitFixedPanel collapse_panel() const noexcept {
        return collapse_panel_;
    }
    void set_collapse_panel(SplitFixedPanel panel);
    // Consumer-authored content threshold. Crossing below it automatically
    // collapses collapse_panel(); crossing back restores only an automatically
    // collapsed pane. A user restore below the threshold is honored until the
    // composition next crosses above it.
    [[nodiscard]] double automatic_collapse_threshold() const noexcept {
        return automatic_collapse_threshold_;
    }
    void set_automatic_collapse_threshold(double extent);
    [[nodiscard]] double keyboard_increment() const noexcept {
        return keyboard_increment_;
    }
    void set_keyboard_increment(double increment);

    [[nodiscard]] Event<const SplitChangeEvent&>& splitter_changed() noexcept {
        return splitter_changed_;
    }

    [[nodiscard]] Size measure(Size available) override;
    void arrange(Rect final_bounds) override;
    void on_pointer_preview(PointerEvent& event) override;
    void on_key_preview(KeyEvent& event) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    bool on_semantic_action(SemanticAction action,
                            std::string_view value) override;

private:
    [[nodiscard]] double axis_extent(Rect bounds) const noexcept;
    [[nodiscard]] double pointer_axis(Point point) const noexcept;
    [[nodiscard]] double constrained_distance(double requested,
                                              double total_extent) const noexcept;
    [[nodiscard]] Rect collapse_tab_bounds() const noexcept;
    [[nodiscard]] bool collapse_target_is_collapsed() const noexcept;
    [[nodiscard]] SplitCollapseOrigin collapse_target_origin() const noexcept;
    void toggle_collapse_target(SplitCollapseOrigin origin);
    void reconcile_automatic_collapse(double total_extent);
    void set_distance(double distance, SplitChangeReason reason);
    void transfer_focus_from(const std::shared_ptr<SplitterPanel>& panel);
    void update_splitter_cursor();

    std::shared_ptr<SplitterPanel> first_panel_;
    std::shared_ptr<SplitterPanel> second_panel_;
    Control::Ptr splitter_;
    Orientation orientation_{Orientation::vertical};
    SplitFixedPanel fixed_panel_{SplitFixedPanel::none};
    SplitFixedPanel collapse_panel_{SplitFixedPanel::none};
    double requested_distance_{-1.0};
    double effective_distance_{};
    double remembered_distance_{-1.0};
    double previous_axis_extent_{};
    double previous_second_extent_{};
    double splitter_width_{3.0};
    double splitter_hit_width_{9.0};
    double first_minimum_{25.0};
    double second_minimum_{25.0};
    std::optional<double> first_maximum_;
    std::optional<double> second_maximum_;
    double keyboard_increment_{4.0};
    double automatic_collapse_threshold_{};
    double pointer_offset_{};
    bool first_collapsed_{};
    bool second_collapsed_{};
    SplitCollapseOrigin first_collapse_origin_{SplitCollapseOrigin::none};
    SplitCollapseOrigin second_collapse_origin_{SplitCollapseOrigin::none};
    bool splitter_fixed_{};
    bool pointer_tracking_{};
    bool collapse_tab_tracking_{};
    bool automatic_collapse_suppressed_{};
    bool tree_initialized_{};
    Event<const SplitChangeEvent&> splitter_changed_;
};

} // namespace gui_forms
