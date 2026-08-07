#pragma once

#include "gui_forms/scrolling.hpp"
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

// Retained focus and validation container. AutoValidate is inherited through
// nested containers while the Window owns the single deterministic focus
// transition and cancellation order.
class ContainerControl : public ScrollableControl {
public:
    explicit ContainerControl(StableId stable_id);

    [[nodiscard]] bool contains_descendant(const Control::Ptr& control) const noexcept;
    [[nodiscard]] Control::Ptr active_control() const noexcept;
    bool request_active_control(const Control::Ptr& control);
    bool clear_active_control();
    [[nodiscard]] AutoValidate auto_validate() const noexcept {
        return auto_validate_;
    }
    [[nodiscard]] AutoValidate effective_auto_validate() const noexcept;
    void set_auto_validate(AutoValidate value);
    [[nodiscard]] Event<AutoValidate>& auto_validate_changed() noexcept {
        return auto_validate_changed_;
    }
    bool validate(bool check_auto_validate = false);
    bool validate_children(
        ValidationConstraints constraints = ValidationConstraints::selectable);
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    [[nodiscard]] AutoValidate authored_auto_validate() const noexcept override {
        return auto_validate_;
    }
    AutoValidate auto_validate_{AutoValidate::inherit};
    Event<AutoValidate> auto_validate_changed_;
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

// Absolute authored layout expressed in a fixed design coordinate space and
// scaled into the arranged bounds. Child ownership remains ordinary retained
// Control ownership; authored slots survive host resize without subclasses.
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
    void reconcile_slots();
    Size design_size_;
    std::unordered_map<std::uint64_t, Rect> slots_;
};

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

enum class FlowDirection : std::uint8_t {
    left_to_right,
    right_to_left,
    top_down,
    bottom_up,
};

// Retained WinForms-style flow layout. Child order is visual-tree order;
// margins are physical, FlowDirection controls the main axis, and FlowBreak
// terminates the current row/column without changing child ownership.
class FlowLayoutPanel final : public ContainerControl {
public:
    explicit FlowLayoutPanel(StableId stable_id);

    [[nodiscard]] FlowDirection flow_direction() const noexcept {
        return flow_direction_;
    }
    void set_flow_direction(FlowDirection direction);
    [[nodiscard]] bool wrap_contents() const noexcept { return wrap_contents_; }
    void set_wrap_contents(bool wrap);
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
    [[nodiscard]] Size layout_children(Size available, bool assign);
    void reconcile_flow_breaks();

    std::unordered_map<std::uint64_t, bool> flow_breaks_;
    FlowDirection flow_direction_{FlowDirection::left_to_right};
    bool wrap_contents_{true};
};

enum class TableSizeMode : std::uint8_t {
    absolute,
    percent,
    auto_size,
};

struct TableLayoutStyle final {
    TableSizeMode size_mode{TableSizeMode::auto_size};
    double size{};
    friend constexpr bool operator==(const TableLayoutStyle&,
                                     const TableLayoutStyle&) = default;
};

struct TableLayoutCellPosition final {
    std::size_t column{};
    std::size_t row{};
    friend constexpr bool operator==(const TableLayoutCellPosition&,
                                     const TableLayoutCellPosition&) = default;
};

enum class TableLayoutGrowStyle : std::uint8_t {
    fixed_size,
    add_rows,
    add_columns,
};

enum class TableCellBorderStyle : std::uint8_t {
    none,
    single,
    inset,
    outset,
};

// Deterministic retained table layout with absolute, percent, and auto tracks.
// Explicit cells and spans coexist with row-major automatic placement. Track
// growth is bounded and overflow is reported instead of silently overlapping.
class TableLayoutPanel final : public ContainerControl {
public:
    explicit TableLayoutPanel(StableId stable_id);

    [[nodiscard]] std::size_t column_count() const noexcept {
        return column_count_;
    }
    void set_column_count(std::size_t count);
    [[nodiscard]] std::size_t row_count() const noexcept { return row_count_; }
    void set_row_count(std::size_t count);
    [[nodiscard]] TableLayoutGrowStyle grow_style() const noexcept {
        return grow_style_;
    }
    void set_grow_style(TableLayoutGrowStyle style);
    [[nodiscard]] bool auto_size() const noexcept override {
        return Control::auto_size();
    }
    void set_auto_size(bool auto_size) override;
    [[nodiscard]] TableCellBorderStyle cell_border_style() const noexcept {
        return cell_border_style_;
    }
    void set_cell_border_style(TableCellBorderStyle style);

    [[nodiscard]] std::span<const TableLayoutStyle> column_styles() const noexcept {
        return column_styles_;
    }
    [[nodiscard]] std::span<const TableLayoutStyle> row_styles() const noexcept {
        return row_styles_;
    }
    void set_column_style(std::size_t column, TableLayoutStyle style);
    void set_row_style(std::size_t row, TableLayoutStyle style);

    void set_cell_position(const Control& child,
                           TableLayoutCellPosition position);
    void clear_cell_position(const Control& child);
    [[nodiscard]] std::optional<TableLayoutCellPosition> cell_position(
        const Control& child) const;
    void set_column_span(const Control& child, std::size_t span);
    [[nodiscard]] std::size_t column_span(const Control& child) const;
    void set_row_span(const Control& child, std::size_t span);
    [[nodiscard]] std::size_t row_span(const Control& child) const;
    [[nodiscard]] Control::Ptr control_from_position(std::size_t column,
                                                     std::size_t row) const;

    [[nodiscard]] std::span<const double> column_widths() const noexcept {
        return column_widths_;
    }
    [[nodiscard]] std::span<const double> row_heights() const noexcept {
        return row_heights_;
    }
    [[nodiscard]] bool layout_overflowed() const noexcept {
        return layout_overflowed_;
    }

    [[nodiscard]] Size measure(Size available) override;
    void arrange(Rect final_bounds) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    struct CellMetadata final {
        std::optional<TableLayoutCellPosition> position;
        std::size_t column_span{1U};
        std::size_t row_span{1U};
    };

    [[nodiscard]] CellMetadata& metadata_for(const Control& child);
    [[nodiscard]] const CellMetadata* metadata_for(const Control& child) const;
    void reconcile_metadata();
    [[nodiscard]] Size layout_children(Size available, bool assign);
    static void validate_style(TableLayoutStyle style);

    std::unordered_map<std::uint64_t, CellMetadata> metadata_;
    std::unordered_map<std::uint64_t, TableLayoutCellPosition> resolved_cells_;
    std::vector<TableLayoutStyle> column_styles_{TableLayoutStyle{}};
    std::vector<TableLayoutStyle> row_styles_{TableLayoutStyle{}};
    std::vector<double> column_widths_;
    std::vector<double> row_heights_;
    std::size_t column_count_{1U};
    std::size_t row_count_{1U};
    TableLayoutGrowStyle grow_style_{TableLayoutGrowStyle::add_rows};
    TableCellBorderStyle cell_border_style_{TableCellBorderStyle::none};
    bool layout_overflowed_{};
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
    FontSpec font_{FontRole::control, 11.0, 600, false, 0.24};
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

// The panels remain ordinary retained containers. Their stable identities are
// derived from the owning split container and survive resize/collapse cycles.
class SplitterPanel final : public ContainerControl {
public:
    explicit SplitterPanel(StableId stable_id);

    // A SplitterPanel is the allocated pane surface, not merely an invisible
    // child owner. Painting the background from its committed bounds keeps the
    // visual allocation truthful while the splitter is moving.
    [[nodiscard]] Color background() const noexcept { return background_; }
    void set_background(Color color);
    void on_paint(Painter& painter, Rect local_damage) override;

private:
    Color background_{Color::rgba(0, 0, 0, 0)};
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
