#pragma once

#include "gui_forms/basic_controls.hpp"
#include "gui_forms/scheduler.hpp"

#include <chrono>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace gui_forms {

struct TreeViewItem final {
    std::string stable_id;
    std::string text;
    std::size_t depth{};
    bool expandable{};
    bool expanded{};
    bool enabled{true};
    std::string image_key;
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
    [[nodiscard]] std::shared_ptr<ImageList> image_list() const noexcept {
        return image_list_;
    }
    void set_image_list(std::shared_ptr<ImageList> image_list);

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

protected:
    void on_attached_to_window() override;

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
    std::shared_ptr<ImageList> image_list_;
    SubscriptionToken image_list_changed_;
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
    std::string image_key;
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
    [[nodiscard]] std::shared_ptr<ImageList> image_list() const noexcept {
        return image_list_;
    }
    void set_image_list(std::shared_ptr<ImageList> image_list);

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

protected:
    void on_attached_to_window() override;

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
    std::shared_ptr<ImageList> image_list_;
    SubscriptionToken image_list_changed_;
    Event<const ObjectSelectionChange&> selection_changed_;
    Event<const std::string&> item_activated_;
    Event<const ObjectContextRequest&> context_requested_;
};

// A factual, variable-height virtual record suitable for correspondence,
// evidence, activity, and inspection ledgers. Compact and expanded
// presentations share one stable logical item; expanding a row never creates a
// retained child control per model item.
struct CorrespondenceItem final {
    std::string stable_id;
    std::string title;
    std::string secondary_text;
    std::string metadata;
    std::string excerpt;
    std::string metric;
    std::string information_heading;
    std::vector<std::string> information_tags;
    std::string information_detail;
    ObjectGlyph glyph{ObjectGlyph::document};
    bool enabled{true};
    bool unavailable{};
    bool stale{};
    // Case-insensitive ASCII terms whose matching excerpt ranges receive a
    // restrained evidence mark. Text remains one factual string; terms do not
    // become commands or ranking tokens.
    std::vector<std::string> emphasis_terms;
};

struct CorrespondenceSelectionChange final {
    std::string previous_id;
    std::string current_id;
};

struct CorrespondencePinChange final {
    std::string previous_id;
    std::string current_id;
};

enum class CorrespondenceExpansionReason : std::uint8_t {
    programmatic,
    hover_intent,
    keyboard_focus,
    pin,
};

struct CorrespondenceExpansionChange final {
    std::string stable_id;
    bool expanded{};
    bool pinned{};
    CorrespondenceExpansionReason reason{
        CorrespondenceExpansionReason::programmatic};
};

// Variable-height virtual list with three independent transient states:
// keyboard focus, delayed pointer inspection, and one persistent pinned item.
// Sparse expanded identities keep position lookup logarithmic without a
// per-item control or per-item height object. Height changes preserve a stable
// anchor row and local offset, preventing the pointer-target oscillation common
// to naive hover expansion.
class CorrespondenceView final : public Panel {
public:
    explicit CorrespondenceView(StableId stable_id);

    [[nodiscard]] std::span<const CorrespondenceItem> items() const noexcept {
        return items_;
    }
    void set_items(std::vector<CorrespondenceItem> items);
    [[nodiscard]] std::string_view selected_id() const noexcept {
        return selected_id_;
    }
    void set_selected_id(std::string_view stable_id);
    [[nodiscard]] std::string_view focused_id() const noexcept {
        return focused_id_;
    }
    [[nodiscard]] std::string_view hovered_id() const noexcept {
        return hovered_id_;
    }
    [[nodiscard]] std::string_view pinned_id() const noexcept {
        return pinned_id_;
    }
    void set_pinned_id(std::string_view stable_id);
    [[nodiscard]] bool expanded(std::string_view stable_id) const;
    [[nodiscard]] double compact_height() const noexcept {
        return compact_height_;
    }
    void set_compact_height(double height);
    [[nodiscard]] double expanded_height() const noexcept {
        return expanded_height_;
    }
    void set_expanded_height(double height);
    [[nodiscard]] std::chrono::milliseconds hover_intent_delay() const noexcept {
        return hover_intent_delay_;
    }
    void set_hover_intent_delay(std::chrono::milliseconds delay);
    [[nodiscard]] double scroll_offset() const noexcept { return scroll_offset_; }
    void set_scroll_offset(double offset);
    [[nodiscard]] double content_height() const noexcept;
    [[nodiscard]] std::size_t realized_count() const noexcept;
    [[nodiscard]] std::optional<Rect> item_bounds(
        std::string_view stable_id) const noexcept;
    [[nodiscard]] FontSpec font() const noexcept { return font_; }
    void set_font(FontSpec font);

    [[nodiscard]] Event<const CorrespondenceSelectionChange&>&
    selection_changed() noexcept { return selection_changed_; }
    [[nodiscard]] Event<const CorrespondencePinChange&>& pin_changed() noexcept {
        return pin_changed_;
    }
    [[nodiscard]] Event<const CorrespondenceExpansionChange&>&
    expansion_changed() noexcept { return expansion_changed_; }
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
    void on_focus_changed(bool focused) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    [[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override;
    bool on_semantic_child_action(std::string_view stable_id,
                                  SemanticAction action,
                                  std::string_view value) override;

protected:
    void on_detached_from_window() noexcept override;

private:
    struct ScrollAnchor final {
        std::string stable_id;
        std::size_t index{};
        double viewport_y{};
        bool valid{};
    };

    [[nodiscard]] std::optional<std::size_t> item_index(
        std::string_view stable_id) const noexcept;
    [[nodiscard]] std::vector<std::size_t> expanded_indices() const;
    [[nodiscard]] bool expanded_index(std::size_t index) const noexcept;
    [[nodiscard]] double scaled_compact_height() const noexcept;
    [[nodiscard]] double scaled_expanded_height() const noexcept;
    [[nodiscard]] double row_height(std::size_t index) const noexcept;
    [[nodiscard]] double row_top(std::size_t index) const noexcept;
    [[nodiscard]] double viewport_height() const noexcept;
    [[nodiscard]] double maximum_scroll_offset() const noexcept;
    [[nodiscard]] std::size_t first_visible_index() const noexcept;
    [[nodiscard]] std::pair<std::size_t, std::size_t> realized_range() const noexcept;
    [[nodiscard]] Rect item_bounds(std::size_t index) const noexcept;
    [[nodiscard]] std::optional<std::size_t> index_at(Point absolute) const noexcept;
    [[nodiscard]] ScrollAnchor capture_anchor(std::size_t preferred) const noexcept;
    void restore_anchor(ScrollAnchor anchor);
    void clamp_scroll_offset() noexcept;
    void ensure_visible(std::size_t index);
    void focus_index(std::size_t index,
                     CorrespondenceExpansionReason reason);
    void set_hovered_index(std::optional<std::size_t> index);
    void schedule_hover_intent(std::size_t index);
    void clear_hover_intent(bool clear_expansion);
    void apply_hover_expansion(std::string stable_id);
    void emit_expansion_delta(std::string_view stable_id, bool before,
                              bool after,
                              CorrespondenceExpansionReason reason);
    void paint_glyph(Painter& painter, Rect bounds, ObjectGlyph glyph,
                     bool enabled) const;

    std::vector<CorrespondenceItem> items_;
    std::string selected_id_;
    std::string focused_id_;
    std::string hovered_id_;
    std::string hover_expanded_id_;
    std::string pending_hover_id_;
    std::string pinned_id_;
    std::optional<std::size_t> pressed_index_;
    PointerButton pressed_button_{PointerButton::none};
    std::uint32_t pressed_click_count_{1U};
    double compact_height_{45.0};
    double expanded_height_{126.0};
    double scroll_offset_{};
    std::chrono::milliseconds hover_intent_delay_{180};
    FontSpec font_{FontRole::content, 11.0, 400, false};
    FrameRequestToken hover_timer_;
    bool focused_{};
    Event<const CorrespondenceSelectionChange&> selection_changed_;
    Event<const CorrespondencePinChange&> pin_changed_;
    Event<const CorrespondenceExpansionChange&> expansion_changed_;
    Event<const std::string&> item_activated_;
    Event<const ObjectContextRequest&> context_requested_;
};

} // namespace gui_forms
