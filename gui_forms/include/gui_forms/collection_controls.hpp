#pragma once

#include "gui_forms/basic_controls.hpp"

#include <chrono>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace gui_forms {

struct TreeViewItem final {
    std::string stable_id;
    std::string text;
    std::size_t depth{};
    bool expandable{};
    bool expanded{};
    bool enabled{true};
};

struct TreeSelectionChange final {
    std::string previous_id;
    std::string current_id;
};

struct TreeExpansionChange final {
    std::string stable_id;
    bool expanded{};
};

// A retained, flat-model tree. Only visible rows are painted and published as
// semantic children; item identity and expansion state remain stable while
// descendants are unrealized.
class TreeView final : public Panel {
public:
    explicit TreeView(StableId stable_id);

    [[nodiscard]] std::span<const TreeViewItem> items() const noexcept {
        return items_;
    }
    void set_items(std::vector<TreeViewItem> items);
    [[nodiscard]] std::string_view selected_id() const noexcept {
        return selected_id_;
    }
    void set_selected_id(std::string_view stable_id);
    [[nodiscard]] std::string_view active_id() const noexcept {
        return active_id_;
    }
    [[nodiscard]] bool expanded(std::string_view stable_id) const;
    void set_expanded(std::string_view stable_id, bool expanded);
    [[nodiscard]] double item_height() const noexcept { return item_height_; }
    void set_item_height(double height);
    [[nodiscard]] double indentation() const noexcept { return indentation_; }
    void set_indentation(double indentation);
    [[nodiscard]] std::size_t top_row() const noexcept { return top_row_; }
    void set_top_row(std::size_t row);
    [[nodiscard]] FontSpec font() const noexcept { return font_; }
    void set_font(FontSpec font);

    [[nodiscard]] Event<const TreeSelectionChange&>& selection_changed() noexcept {
        return selection_changed_;
    }
    [[nodiscard]] Event<const TreeExpansionChange&>& expansion_changed() noexcept {
        return expansion_changed_;
    }
    [[nodiscard]] Event<const std::string&>& item_activated() noexcept {
        return item_activated_;
    }

    void arrange(Rect final_bounds) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    void on_pointer(PointerEvent& event) override;
    void on_key(KeyEvent& event) override;
    void on_text_input(TextInputEvent& event) override;
    void on_focus_changed(bool focused) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    [[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override;
    bool on_semantic_child_action(std::string_view stable_id,
                                  SemanticAction action,
                                  std::string_view value) override;

private:
    void rebuild_visible();
    [[nodiscard]] std::optional<std::size_t> item_index(
        std::string_view stable_id) const noexcept;
    [[nodiscard]] std::optional<std::size_t> visible_row_at(
        Point absolute) const noexcept;
    [[nodiscard]] std::size_t visible_row_count() const noexcept;
    void ensure_visible(std::size_t visible_row);
    void select_visible_row(std::size_t visible_row, bool activate);
    void type_select(std::string_view text);

    std::vector<TreeViewItem> items_;
    std::vector<std::size_t> visible_;
    std::string selected_id_;
    std::string active_id_;
    std::optional<std::size_t> hovered_row_;
    std::optional<std::size_t> pressed_row_;
    std::uint32_t pressed_click_count_{1U};
    bool pressed_expander_{};
    std::size_t top_row_{};
    double item_height_{24.0};
    double indentation_{18.0};
    FontSpec font_{FontRole::content, 12.0, 400, false};
    std::string type_prefix_;
    std::chrono::steady_clock::time_point last_type_time_{};
    bool focused_{};
    Event<const TreeSelectionChange&> selection_changed_;
    Event<const TreeExpansionChange&> expansion_changed_;
    Event<const std::string&> item_activated_;
};

enum class ObjectViewMode : std::uint8_t {
    icons,
    details,
};

enum class ObjectGlyph : std::uint8_t {
    folder,
    document,
    image,
    archive,
    audio,
    code,
};

struct ObjectViewItem final {
    std::string stable_id;
    std::string name;
    std::string secondary_text;
    std::string description;
    ObjectGlyph glyph{ObjectGlyph::document};
    bool enabled{true};
};

struct ObjectSelectionChange final {
    std::string previous_id;
    std::string current_id;
    std::vector<std::string> previous_ids;
    std::vector<std::string> current_ids;
};

enum class ObjectSelectionMode : std::uint8_t {
    single,
    multiple,
};

struct ObjectContextRequest final {
    std::string stable_id;
    Point screen_position{};
};

// A stable-ID virtual object collection with icon and details projections.
// It owns no per-item controls: paint, hit testing, keyboard spatial movement,
// and semantic realization operate on a bounded visible window.
class ObjectView final : public Panel {
public:
    explicit ObjectView(StableId stable_id);

    [[nodiscard]] std::span<const ObjectViewItem> items() const noexcept {
        return items_;
    }
    void set_items(std::vector<ObjectViewItem> items);
    [[nodiscard]] ObjectViewMode view_mode() const noexcept { return view_mode_; }
    void set_view_mode(ObjectViewMode mode);
    [[nodiscard]] std::string_view selected_id() const noexcept {
        return selected_id_;
    }
    void set_selected_id(std::string_view stable_id);
    [[nodiscard]] std::span<const std::string> selected_ids() const noexcept {
        return selected_ids_;
    }
    void set_selected_ids(std::vector<std::string> stable_ids,
                          std::string_view primary_id = {});
    void clear_selection();
    void select_all();
    [[nodiscard]] ObjectSelectionMode selection_mode() const noexcept {
        return selection_mode_;
    }
    void set_selection_mode(ObjectSelectionMode mode);
    [[nodiscard]] std::string_view selection_anchor_id() const noexcept {
        return selection_anchor_id_;
    }
    [[nodiscard]] std::string_view focused_id() const noexcept {
        return focused_id_;
    }
    [[nodiscard]] Size icon_cell_size() const noexcept { return icon_cell_size_; }
    void set_icon_cell_size(Size size);
    [[nodiscard]] double details_row_height() const noexcept {
        return details_row_height_;
    }
    void set_details_row_height(double height);
    [[nodiscard]] std::size_t top_row() const noexcept { return top_row_; }
    void set_top_row(std::size_t row);
    [[nodiscard]] FontSpec font() const noexcept { return font_; }
    void set_font(FontSpec font);

    [[nodiscard]] Event<const ObjectSelectionChange&>& selection_changed() noexcept {
        return selection_changed_;
    }
    [[nodiscard]] Event<const std::string&>& item_activated() noexcept {
        return item_activated_;
    }
    [[nodiscard]] Event<const ObjectContextRequest&>& context_requested() noexcept {
        return context_requested_;
    }

    void arrange(Rect final_bounds) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    void on_pointer(PointerEvent& event) override;
    void on_key(KeyEvent& event) override;
    void on_text_input(TextInputEvent& event) override;
    void on_focus_changed(bool focused) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    [[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override;
    bool on_semantic_child_action(std::string_view stable_id,
                                  SemanticAction action,
                                  std::string_view value) override;

private:
    [[nodiscard]] std::optional<std::size_t> item_index(
        std::string_view stable_id) const noexcept;
    [[nodiscard]] std::size_t columns() const noexcept;
    [[nodiscard]] double row_height() const noexcept;
    [[nodiscard]] std::size_t visible_row_count() const noexcept;
    [[nodiscard]] Rect item_bounds(std::size_t index) const noexcept;
    [[nodiscard]] std::optional<std::size_t> index_at(Point absolute) const noexcept;
    void ensure_visible(std::size_t index);
    void select_index(std::size_t index, bool activate,
                      Modifier modifiers = Modifier::none);
    void focus_index(std::size_t index);
    void apply_selection(std::vector<std::string> stable_ids,
                         std::string primary_id,
                         std::string anchor_id,
                         bool move_focus);
    [[nodiscard]] bool is_selected(std::string_view stable_id) const noexcept;
    [[nodiscard]] std::vector<std::string> range_selection(
        std::size_t target_index, bool preserve_existing) const;
    void type_select(std::string_view text);
    void paint_glyph(Painter& painter, Rect bounds, ObjectGlyph glyph,
                     bool enabled) const;

    std::vector<ObjectViewItem> items_;
    std::string selected_id_;
    std::vector<std::string> selected_ids_;
    std::string focused_id_;
    std::string selection_anchor_id_;
    std::optional<std::size_t> hovered_index_;
    std::optional<std::size_t> pressed_index_;
    std::uint32_t pressed_click_count_{1U};
    PointerButton pressed_button_{PointerButton::none};
    ObjectSelectionMode selection_mode_{ObjectSelectionMode::multiple};
    ObjectViewMode view_mode_{ObjectViewMode::icons};
    Size icon_cell_size_{112.0, 91.0};
    double details_row_height_{28.0};
    std::size_t top_row_{};
    FontSpec font_{FontRole::content, 11.0, 400, false};
    std::string type_prefix_;
    std::chrono::steady_clock::time_point last_type_time_{};
    bool focused_{};
    Event<const ObjectSelectionChange&> selection_changed_;
    Event<const std::string&> item_activated_;
    Event<const ObjectContextRequest&> context_requested_;
};

} // namespace gui_forms
