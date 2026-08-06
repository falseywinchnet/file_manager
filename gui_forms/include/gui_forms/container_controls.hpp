#pragma once

#include "gui_forms/control.hpp"
#include "gui_forms/range_controls.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace gui_forms {

// The first reusable focus-container identity. Validation, scaling, scrolling,
// and dialog-key routing remain separate contracts owned by later M5/M6 slices.
class ContainerControl : public Control {
public:
    explicit ContainerControl(StableId stable_id);

    [[nodiscard]] bool contains_descendant(const Control::Ptr& control) const noexcept;
    [[nodiscard]] Control::Ptr active_control() const noexcept;
    bool request_active_control(const Control::Ptr& control);
    bool clear_active_control();
};

// A retained composition root with a one-shot lifetime load notification and
// a successful whole-subtree attachment count.
class UserControl : public ContainerControl {
public:
    explicit UserControl(StableId stable_id);

    [[nodiscard]] Event<>& loaded() noexcept { return loaded_event_; }
    [[nodiscard]] bool is_loaded() const noexcept { return loaded_; }
    [[nodiscard]] bool is_attached() const noexcept { return attached_; }
    [[nodiscard]] std::uint64_t attachment_count() const noexcept {
        return attachment_count_;
    }

protected:
    void on_attached_to_window() override;
    void on_attachment_committed() noexcept override;
    void on_detached_from_window() noexcept override;

private:
    Event<> loaded_event_;
    std::uint64_t attachment_count_{};
    bool loaded_{};
    bool attached_{};
};

enum class TabAlignment : std::uint8_t {
    top,
    bottom,
    left,
    right,
};

enum class TabAppearance : std::uint8_t {
    normal,
    buttons,
    flat_buttons,
};

struct TabSelectionChange final {
    std::optional<std::size_t> old_index;
    std::optional<std::size_t> new_index;
};

class TabPage final : public Panel {
public:
    explicit TabPage(StableId stable_id, std::string text = {});

    [[nodiscard]] const std::string& text() const noexcept { return text_; }
    void set_text(std::string text);
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    std::string text_;
};

// A renderer-neutral retained tab family. Tabs are headers owned by this
// control while TabPage instances remain ordinary retained child containers.
// Hidden pages do not paint, hit-test, or enter the semantic tree.
class TabControl final : public ContainerControl {
public:
    explicit TabControl(StableId stable_id);

    void add_page(std::shared_ptr<TabPage> page);
    [[nodiscard]] std::shared_ptr<TabPage> remove_page(const TabPage& page);
    [[nodiscard]] std::vector<std::shared_ptr<TabPage>> pages() const;
    [[nodiscard]] std::size_t page_count() const;
    [[nodiscard]] std::shared_ptr<TabPage> page_at(std::size_t index) const;

    [[nodiscard]] std::optional<std::size_t> selected_index() const;
    [[nodiscard]] std::shared_ptr<TabPage> selected_tab() const noexcept {
        return selected_page_.lock();
    }
    void set_selected_index(std::size_t index);
    void set_selected_tab(const std::shared_ptr<TabPage>& page);
    [[nodiscard]] TabAlignment alignment() const noexcept { return alignment_; }
    void set_alignment(TabAlignment alignment);
    [[nodiscard]] TabAppearance appearance() const noexcept { return appearance_; }
    void set_appearance(TabAppearance appearance);
    [[nodiscard]] Size item_size() const noexcept { return item_size_; }
    void set_item_size(Size size);
    [[nodiscard]] const BasicControlStyle& style() const noexcept { return style_; }
    void set_style(BasicControlStyle style);
    [[nodiscard]] Rect tab_bounds(std::size_t index) const;
    [[nodiscard]] Rect display_bounds() const noexcept;
    [[nodiscard]] Event<const TabSelectionChange&>& selected_index_changed() noexcept {
        return selected_index_changed_;
    }

    [[nodiscard]] Size measure(Size available) override;
    void arrange(Rect final_bounds) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    void on_pointer(PointerEvent& event) override;
    void on_key_preview(KeyEvent& event) override;
    void on_focus_changed(bool focused) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    [[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override;
    bool on_semantic_child_action(std::string_view stable_id,
                                  SemanticAction action,
                                  std::string_view value) override;

private:
    [[nodiscard]] std::optional<std::size_t> index_of(
        const std::shared_ptr<TabPage>& page) const;
    void select_relative(int delta);
    void remember_page_focus(const std::shared_ptr<TabPage>& page);
    void restore_page_focus(const std::shared_ptr<TabPage>& page,
                            bool selection_owned_focus);
    void reconcile_pages();

    std::vector<std::weak_ptr<TabPage>> pages_;
    std::weak_ptr<TabPage> selected_page_;
    std::unordered_map<std::uint64_t, Control::WeakPtr> remembered_focus_;
    BasicControlStyle style_;
    Size item_size_{120.0, 30.0};
    FontSpec font_{FontRole::control, 11.0, 600, false};
    TabAlignment alignment_{TabAlignment::top};
    TabAppearance appearance_{TabAppearance::normal};
    bool pointer_engaged_{};
    bool focused_{};
    Event<const TabSelectionChange&> selected_index_changed_;
};

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

struct SplitChangeEvent final {
    double old_distance{};
    double new_distance{};
    SplitChangeReason reason{SplitChangeReason::programmatic};
};

// The panels remain ordinary retained containers. Their stable identities are
// derived from the owning split container and survive resize/collapse cycles.
class SplitterPanel final : public ContainerControl {
public:
    explicit SplitterPanel(StableId stable_id);
};

// A retained two-pane composition with one physical splitter. The painted seam
// and its pointer target are deliberately separate dimensions: a three-pixel
// seam can retain a nine-pixel desktop hit target without changing allocation.
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
    [[nodiscard]] bool first_collapsed() const noexcept { return first_collapsed_; }
    void set_first_collapsed(bool collapsed);
    [[nodiscard]] bool second_collapsed() const noexcept { return second_collapsed_; }
    void set_second_collapsed(bool collapsed);
    [[nodiscard]] bool splitter_fixed() const noexcept { return splitter_fixed_; }
    void set_splitter_fixed(bool fixed);
    [[nodiscard]] SplitFixedPanel fixed_panel() const noexcept { return fixed_panel_; }
    void set_fixed_panel(SplitFixedPanel panel);
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

private:
    [[nodiscard]] double axis_extent(Rect bounds) const noexcept;
    [[nodiscard]] double pointer_axis(Point point) const noexcept;
    [[nodiscard]] double constrained_distance(double requested,
                                              double total_extent) const noexcept;
    void set_distance(double distance, SplitChangeReason reason);
    void transfer_focus_from(const std::shared_ptr<SplitterPanel>& panel);
    void update_splitter_cursor();

    std::shared_ptr<SplitterPanel> first_panel_;
    std::shared_ptr<SplitterPanel> second_panel_;
    Control::Ptr splitter_;
    Orientation orientation_{Orientation::vertical};
    SplitFixedPanel fixed_panel_{SplitFixedPanel::none};
    double requested_distance_{-1.0};
    double effective_distance_{};
    double remembered_distance_{-1.0};
    double previous_axis_extent_{};
    double previous_second_extent_{};
    double splitter_width_{3.0};
    double splitter_hit_width_{9.0};
    double first_minimum_{25.0};
    double second_minimum_{25.0};
    double keyboard_increment_{4.0};
    double pointer_offset_{};
    bool first_collapsed_{};
    bool second_collapsed_{};
    bool splitter_fixed_{};
    bool pointer_tracking_{};
    bool tree_initialized_{};
    Event<const SplitChangeEvent&> splitter_changed_;
};

} // namespace gui_forms
