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

#include "gui_forms/controls/panel/object_view/object_view.hpp"

namespace gui_forms {

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
    [[nodiscard]] double status_rail_width() const noexcept {
        return status_rail_width_;
    }
    void set_status_rail_width(double width);
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
    double status_rail_width_{4.0};
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
