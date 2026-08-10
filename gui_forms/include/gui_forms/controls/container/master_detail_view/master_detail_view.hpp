#pragma once

#include "gui_forms/container_controls.hpp"

#include <cstdint>
#include <memory>

namespace gui_forms {

enum class MasterDetailDisplayMode : std::uint8_t {
    automatic,
    side_by_side,
    master_only,
    detail_only,
};

struct MasterDetailLayout final {
    Orientation orientation{Orientation::vertical};
    double master_extent{260.0};
    double splitter_width{3.0};
    double splitter_hit_width{9.0};
    double master_minimum{120.0};
    double detail_minimum{240.0};
    double compact_threshold{720.0};
    bool resizable{true};

    friend constexpr bool operator==(const MasterDetailLayout&,
                                     const MasterDetailLayout&) = default;
};

struct MasterDetailPresentationChange final {
    MasterDetailDisplayMode previous{MasterDetailDisplayMode::side_by_side};
    MasterDetailDisplayMode current{MasterDetailDisplayMode::side_by_side};
    bool automatic{};
};

// A renderer-neutral retained master/detail shell. Consumers provide ordinary
// controls for both roles; GUI.Forms owns responsive presentation, a genuine
// draggable/keyboard splitter, focus transfer on collapse, and compact
// master/detail navigation. This is intentionally content-agnostic so decision
// browsers, settings panes, inspectors, and navigation stacks share one law.
class MasterDetailView final : public ContainerControl {
public:
    explicit MasterDetailView(StableId stable_id);
    void initialize_control_tree();

    [[nodiscard]] Control::Ptr master() const noexcept { return master_; }
    [[nodiscard]] Control::Ptr detail() const noexcept { return detail_; }
    [[nodiscard]] Control::Ptr set_master(Control::Ptr control);
    [[nodiscard]] Control::Ptr set_detail(Control::Ptr control);
    [[nodiscard]] std::shared_ptr<SplitContainer> split_container() const noexcept {
        return split_;
    }

    [[nodiscard]] const MasterDetailLayout& master_detail_layout() const noexcept {
        return layout_;
    }
    [[nodiscard]] bool uses_theme_layout() const noexcept {
        return uses_theme_layout_;
    }
    [[nodiscard]] MasterDetailLayout effective_master_detail_layout() const noexcept;
    void set_master_detail_layout(MasterDetailLayout layout);
    void reset_master_detail_layout_to_theme();
    [[nodiscard]] MasterDetailDisplayMode display_mode() const noexcept {
        return display_mode_;
    }
    void set_display_mode(MasterDetailDisplayMode mode);
    [[nodiscard]] MasterDetailDisplayMode effective_display_mode() const noexcept {
        return effective_display_mode_;
    }
    [[nodiscard]] bool compact_detail_visible() const noexcept {
        return compact_detail_visible_;
    }
    void set_compact_detail_visible(bool visible);
    void show_master();
    void show_detail();

    [[nodiscard]] Event<const MasterDetailPresentationChange&>&
    presentation_changed() noexcept { return presentation_changed_; }

    [[nodiscard]] Size measure(Size available) override;
    void arrange(Rect final_bounds) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    [[nodiscard]] Control::Ptr replace_role(Control::Ptr& slot,
                                            const std::shared_ptr<SplitterPanel>& panel,
                                            Control::Ptr replacement);
    [[nodiscard]] MasterDetailDisplayMode resolve_display_mode(
        Size available) const noexcept;
    void apply_display_mode(MasterDetailDisplayMode mode, bool automatic);
    void configure_split();

    std::shared_ptr<SplitContainer> split_;
    Control::Ptr master_;
    Control::Ptr detail_;
    MasterDetailLayout layout_;
    Event<const MasterDetailPresentationChange&> presentation_changed_;
    MasterDetailDisplayMode display_mode_{MasterDetailDisplayMode::automatic};
    MasterDetailDisplayMode effective_display_mode_{
        MasterDetailDisplayMode::side_by_side};
    bool compact_detail_visible_{};
    bool initialized_{};
    bool uses_theme_layout_{true};
};

} // namespace gui_forms
