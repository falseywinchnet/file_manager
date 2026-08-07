#pragma once

#include "gui_forms/container_controls.hpp"

#include <optional>

namespace gui_forms {

struct CardLayout final {
    Insets padding{12.0, 10.0, 12.0, 10.0};
    double section_gap{8.0};
    double header_extent{30.0};
    double footer_extent{30.0};

    friend constexpr bool operator==(const CardLayout&,
                                     const CardLayout&) = default;
};

// General retained header/body/footer composition. Card owns each installed
// section as an ordinary visual child and returns the detached predecessor on
// replacement; it never hides demo-only layout or renderer code.
class Card : public Panel {
public:
    explicit Card(StableId stable_id);

    [[nodiscard]] Control::Ptr header() const noexcept { return header_; }
    [[nodiscard]] Control::Ptr body() const noexcept { return body_; }
    [[nodiscard]] Control::Ptr footer() const noexcept { return footer_; }
    [[nodiscard]] Control::Ptr set_header(Control::Ptr control);
    [[nodiscard]] Control::Ptr set_body(Control::Ptr control);
    [[nodiscard]] Control::Ptr set_footer(Control::Ptr control);

    [[nodiscard]] const CardLayout& card_layout() const noexcept { return layout_; }
    [[nodiscard]] bool uses_theme_layout() const noexcept {
        return uses_theme_layout_;
    }
    [[nodiscard]] CardLayout effective_card_layout() const noexcept;
    void set_card_layout(CardLayout layout);
    void reset_card_layout_to_theme();
    [[nodiscard]] bool interactive() const noexcept { return interactive_; }
    void set_interactive(bool interactive);
    [[nodiscard]] bool selected() const noexcept { return selected_; }
    void set_selected(bool selected);
    [[nodiscard]] bool hovered_visual() const noexcept { return hovered_; }
    [[nodiscard]] bool pressed_visual() const noexcept { return pressed_; }
    [[nodiscard]] bool focused_visual() const noexcept { return focused_; }
    [[nodiscard]] Event<Card&>& activated() noexcept { return activated_; }
    [[nodiscard]] Event<bool>& selected_changed() noexcept {
        return selected_changed_;
    }

    [[nodiscard]] Size measure(Size available) override;
    void arrange(Rect final_bounds) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] Insets visual_outsets() const noexcept override;
    [[nodiscard]] bool hit_test_local(Point local_point) const override;
    void on_pointer(PointerEvent& event) override;
    void on_key(KeyEvent& event) override;
    void on_focus_changed(bool focused) override;
    void on_activate() override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    bool on_semantic_action(SemanticAction action,
                            std::string_view value) override;

private:
    Control::Ptr replace_section(Control::Ptr& slot, Control::Ptr replacement);
    [[nodiscard]] ControlVisualContext current_context() const noexcept;

    Control::Ptr header_;
    Control::Ptr body_;
    Control::Ptr footer_;
    CardLayout layout_;
    Event<Card&> activated_;
    Event<bool> selected_changed_;
    std::uint32_t keyboard_key_{};
    bool interactive_{};
    bool selected_{};
    bool hovered_{};
    bool pressed_{};
    bool focused_{};
    bool uses_theme_layout_{true};
};

enum class ReviewDisposition : std::uint8_t {
    neutral,
    information,
    accepted,
    pending,
    warning,
    rejected,
};

struct ReviewRecord final {
    std::string key;
    std::string title;
    std::string summary;
    std::string verdict;
    ReviewDisposition disposition{ReviewDisposition::neutral};

    friend bool operator==(const ReviewRecord&,
                           const ReviewRecord&) = default;
};

// A typed, content-agnostic review projection over Card. It owns theme-aware
// title/body/verdict labels and projects pending/rejected state into the same
// retained visual and semantic laws as any other Card. Consumers supply data,
// never a private renderer or demo-only layout subclass.
class ReviewCard final : public Card {
public:
    explicit ReviewCard(StableId stable_id);
    void initialize_control_tree();

    [[nodiscard]] const ReviewRecord& record() const noexcept { return record_; }
    void set_record(ReviewRecord record);
    [[nodiscard]] std::shared_ptr<Label> title_label() const noexcept {
        return title_label_;
    }
    [[nodiscard]] std::shared_ptr<Label> summary_label() const noexcept {
        return summary_label_;
    }
    [[nodiscard]] std::shared_ptr<Label> verdict_label() const noexcept {
        return verdict_label_;
    }
    [[nodiscard]] Event<const ReviewRecord&>& record_changed() noexcept {
        return record_changed_;
    }

    [[nodiscard]] Size measure(Size available) override;
    void arrange(Rect final_bounds) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    ReviewRecord record_;
    std::shared_ptr<Label> title_label_;
    std::shared_ptr<Label> summary_label_;
    std::shared_ptr<Label> verdict_label_;
    Event<const ReviewRecord&> record_changed_;
    bool initialized_{};
};

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
