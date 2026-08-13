#pragma once

#include "gui_forms/controls/panel/text_box/text_box.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace gui_forms {

struct BreadcrumbSegment final {
    std::string stable_id;
    std::string text;
    std::string description;
    bool enabled{true};
};

// One retained location instrument. Logical segments are virtual semantic
// children; the exact-path TextBox is the sole retained child and occupies the
// same row only while editing.
class BreadcrumbTrail final : public Panel {
public:
    static constexpr bool initialize_tree_after_construction = true;

    explicit BreadcrumbTrail(StableId stable_id,
                             std::string editor_stable_id = {});
    void initialize_control_tree();

    [[nodiscard]] std::span<const BreadcrumbSegment> segments() const noexcept {
        return segments_;
    }
    void set_segments(std::vector<BreadcrumbSegment> segments);
    [[nodiscard]] std::span<const std::string> hidden_segment_ids() const noexcept {
        return hidden_segment_ids_;
    }
    [[nodiscard]] std::string_view active_id() const noexcept {
        return active_id_;
    }
    void set_active_id(std::string_view stable_id);

    [[nodiscard]] bool editing() const noexcept { return editing_; }
    void set_editing(bool editing);
    void begin_edit(std::string text, bool select_all = true);
    [[nodiscard]] std::shared_ptr<TextBox> editor() const noexcept {
        return editor_;
    }
    [[nodiscard]] std::string_view overflow_stable_id() const noexcept {
        return overflow_stable_id_;
    }
    [[nodiscard]] std::string_view edit_stable_id() const noexcept {
        return edit_stable_id_;
    }

    [[nodiscard]] Event<const std::string&>& segment_activated() noexcept {
        return segment_activated_;
    }
    [[nodiscard]] Event<>& overflow_activated() noexcept {
        return overflow_activated_;
    }
    [[nodiscard]] Event<const std::string&>& edit_committed() noexcept {
        return edit_committed_;
    }
    [[nodiscard]] Event<>& edit_cancelled() noexcept {
        return edit_cancelled_;
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

private:
    enum class VisibleKind : std::uint8_t { segment, overflow, edit };
    struct VisibleItem final {
        VisibleKind kind{VisibleKind::segment};
        std::size_t segment_index{};
        Rect bounds{};
    };

    struct EditorCommitCallback final {
        std::weak_ptr<BreadcrumbTrail> target;
        void operator()(const std::string& text) const;
    };
    struct EditorCancelCallback final {
        std::weak_ptr<BreadcrumbTrail> target;
        void operator()() const;
    };

    void editor_committed(std::string text);
    void editor_cancelled();
    void rebuild_layout(double width, double height);
    [[nodiscard]] double natural_width(const BreadcrumbSegment& segment) const noexcept;
    [[nodiscard]] std::optional<std::size_t> segment_index(
        std::string_view stable_id) const noexcept;
    [[nodiscard]] std::optional<std::size_t> visible_index_at(Point absolute) const noexcept;
    [[nodiscard]] std::string_view visible_stable_id(const VisibleItem& item) const noexcept;
    void activate_visible(std::size_t index);
    void normalize_active();

    std::vector<BreadcrumbSegment> segments_;
    std::vector<VisibleItem> visible_items_;
    std::vector<std::string> hidden_segment_ids_;
    std::string editor_stable_id_;
    std::string overflow_stable_id_;
    std::string edit_stable_id_;
    std::string active_id_;
    std::shared_ptr<TextBox> editor_;
    SubscriptionToken editor_commit_;
    SubscriptionToken editor_cancel_;
    std::optional<std::size_t> hovered_visible_;
    std::optional<std::size_t> pressed_visible_;
    FontSpec font_{FontRole::control, 10.5, 600, false};
    bool editing_{};
    bool focused_{};
    Event<const std::string&> segment_activated_;
    Event<> overflow_activated_;
    Event<const std::string&> edit_committed_;
    Event<> edit_cancelled_;

    static constexpr double edge_overlap_ = 8.0;
    static constexpr double edit_width_ = 36.0;
};

} // namespace gui_forms
