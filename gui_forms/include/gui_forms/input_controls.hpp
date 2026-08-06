#pragma once

#include "gui_forms/basic_controls.hpp"
#include "gui_forms/scheduler.hpp"
#include "gui_forms/text.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <cstdint>
#include <deque>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace gui_forms {

struct TextSelection final {
    Utf8Offset anchor{};
    Utf8Offset caret{};

    [[nodiscard]] Utf8Offset start() const noexcept {
        return Utf8Offset(std::min(anchor.value(), caret.value()));
    }
    [[nodiscard]] Utf8Offset end() const noexcept {
        return Utf8Offset(std::max(anchor.value(), caret.value()));
    }
    [[nodiscard]] std::size_t length() const noexcept {
        return end().value() - start().value();
    }
    [[nodiscard]] bool empty() const noexcept { return anchor == caret; }
    friend constexpr bool operator==(const TextSelection&,
                                     const TextSelection&) = default;
};

class TextBox final : public Panel {
public:
    explicit TextBox(StableId stable_id, std::string text = {});

    [[nodiscard]] std::string_view text() const noexcept { return store_.utf8(); }
    void set_text(std::string text);
    [[nodiscard]] std::string_view placeholder_text() const noexcept {
        return placeholder_;
    }
    void set_placeholder_text(std::string text);
    [[nodiscard]] bool read_only() const noexcept { return read_only_; }
    void set_read_only(bool read_only);
    [[nodiscard]] char32_t password_character() const noexcept {
        return password_character_;
    }
    void set_password_character(char32_t character);
    [[nodiscard]] bool use_system_password_character() const noexcept {
        return use_system_password_character_;
    }
    void set_use_system_password_character(bool enabled);
    [[nodiscard]] bool password_protected() const noexcept {
        return use_system_password_character_ || password_character_ != U'\0';
    }
    [[nodiscard]] FontSpec font() const noexcept { return font_; }
    void set_font(FontSpec font);
    [[nodiscard]] TextSelection selection() const noexcept { return selection_; }
    void select(Utf8Offset anchor, Utf8Offset caret);
    void select_all();
    [[nodiscard]] std::string selected_text() const;
    [[nodiscard]] bool can_undo() const noexcept { return !undo_.empty(); }
    [[nodiscard]] bool can_redo() const noexcept { return !redo_.empty(); }
    bool undo();
    bool redo();
    bool replace_selection(std::string_view replacement);
    bool delete_selection();
    // Clipboard commands use the Window's portable HostServices seam. A
    // protected field never exports its selected secret.
    bool copy();
    bool cut();
    bool paste();

    [[nodiscard]] Event<const std::string&>& text_changed() noexcept {
        return text_changed_;
    }
    [[nodiscard]] Event<const TextSelection&>& selection_changed() noexcept {
        return selection_changed_;
    }
    [[nodiscard]] Event<const std::string&>& committed() noexcept {
        return committed_;
    }
    [[nodiscard]] Event<>& cancelled() noexcept { return cancelled_; }

    void on_paint(Painter& painter, Rect local_damage) override;
    void on_pointer(PointerEvent& event) override;
    void on_key(KeyEvent& event) override;
    void on_text_input(TextInputEvent& event) override;
    void on_focus_changed(bool focused) override;
    void on_frame(FrameTime now) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    bool on_semantic_action(SemanticAction action,
                            std::string_view value) override;

protected:
    void on_detached_from_window() noexcept override;

private:
    struct Snapshot final {
        std::string text;
        TextSelection selection;
    };

    [[nodiscard]] Snapshot snapshot() const;
    void apply_snapshot(Snapshot snapshot);
    void set_selection(TextSelection selection, bool reveal_caret = true);
    bool replace(Utf8Offset start, Utf8Offset end,
                 std::string_view replacement, bool record_history = true);
    [[nodiscard]] Utf8Offset position_at(double local_x) const noexcept;
    [[nodiscard]] double boundary_x(Utf8Offset offset) const noexcept;
    [[nodiscard]] Utf8Offset previous_word_boundary(Utf8Offset offset) const;
    [[nodiscard]] Utf8Offset next_word_boundary(Utf8Offset offset) const;
    [[nodiscard]] std::string display_text() const;
    void reset_caret_blink();
    void schedule_caret_blink();
    void push_history(std::deque<Snapshot>& history, Snapshot snapshot);
    void clear_redo() noexcept;

    TextStore store_;
    std::string placeholder_;
    FontSpec font_{FontRole::content, 12.0, 400, false};
    TextSelection selection_{};
    std::vector<double> layout_positions_;
    std::vector<std::uint64_t> layout_offsets_;
    double layout_text_scale_{};
    double horizontal_offset_{};
    bool read_only_{};
    char32_t password_character_{};
    bool use_system_password_character_{};
    bool focused_{};
    bool selecting_{};
    bool caret_visible_{true};
    FrameRequestToken caret_frame_;
    std::deque<Snapshot> undo_;
    std::deque<Snapshot> redo_;
    std::size_t history_bytes_{};
    Event<const std::string&> text_changed_;
    Event<const TextSelection&> selection_changed_;
    Event<const std::string&> committed_;
    Event<> cancelled_;

    static constexpr double text_left_ = 6.0;
    static constexpr std::size_t maximum_history_entries_ = 128U;
    static constexpr std::size_t maximum_history_bytes_ = 8U * 1024U * 1024U;
};

enum class ListSelectionMode : std::uint8_t {
    one,
    multiple_extended,
};

struct ListSelectionChange final {
    std::vector<std::size_t> previous;
    std::vector<std::size_t> current;
    std::optional<std::size_t> active_index;
};

class ListBox : public Panel {
public:
    explicit ListBox(StableId stable_id);

    [[nodiscard]] std::span<const std::string> items() const noexcept {
        return items_;
    }
    virtual void set_items(std::vector<std::string> items);
    virtual void add_item(std::string item);
    virtual void remove_item(std::size_t index);
    virtual void clear_items();
    // Optional stable semantic identities for model-backed rows. Calling
    // set_items clears custom identities; supply exactly one unique, nonempty
    // ID per item after the collection mutation. Without custom IDs the
    // historical <list>.item.<index> identity remains in force.
    void set_item_stable_ids(std::vector<std::string> stable_ids);
    [[nodiscard]] std::string item_stable_id(std::size_t index) const;
    [[nodiscard]] ListSelectionMode selection_mode() const noexcept {
        return selection_mode_;
    }
    void set_selection_mode(ListSelectionMode mode);
    [[nodiscard]] std::span<const std::size_t> selected_indices() const noexcept {
        return selected_;
    }
    [[nodiscard]] std::optional<std::size_t> selected_index() const noexcept;
    void select_index(std::size_t index, bool extend = false, bool toggle = false);
    void clear_selection();
    [[nodiscard]] std::size_t top_index() const noexcept { return top_index_; }
    void set_top_index(std::size_t index);
    [[nodiscard]] double item_height() const noexcept { return item_height_; }
    void set_item_height(double height);
    [[nodiscard]] FontSpec font() const noexcept { return font_; }
    void set_font(FontSpec font);

    [[nodiscard]] Event<const ListSelectionChange&>& selection_changed() noexcept {
        return selection_changed_;
    }
    [[nodiscard]] Event<std::size_t>& item_activated() noexcept {
        return item_activated_;
    }

    void on_paint(Painter& painter, Rect local_damage) override;
    void arrange(Rect final_bounds) override;
    void on_pointer(PointerEvent& event) override;
    void on_key(KeyEvent& event) override;
    void on_focus_changed(bool focused) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    [[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override;
    bool on_semantic_child_action(std::string_view stable_id,
                                  SemanticAction action,
                                  std::string_view value) override;

protected:
    [[nodiscard]] virtual double row_text_left() const noexcept;
    virtual void paint_row_adornment(Painter& painter, std::size_t index,
                                     Rect row_bounds, bool selected,
                                     bool focused) const;
    [[nodiscard]] std::optional<std::size_t> active_index_for_extension()
        const noexcept { return active_index_; }
    [[nodiscard]] bool focused_for_extension() const noexcept { return focused_; }
    [[nodiscard]] std::optional<std::size_t> index_at(
        Point absolute) const noexcept;

private:
    void apply_selection(std::vector<std::size_t> selection,
                         std::optional<std::size_t> active);
    void ensure_visible(std::size_t index);
    [[nodiscard]] std::size_t visible_row_count() const noexcept;

    std::vector<std::string> items_;
    std::vector<std::string> item_stable_ids_;
    std::vector<std::size_t> selected_;
    std::optional<std::size_t> active_index_;
    std::optional<std::size_t> anchor_index_;
    std::optional<std::size_t> hovered_index_;
    std::size_t top_index_{};
    double item_height_{26.0};
    FontSpec font_{FontRole::content, 12.0, 400, false};
    ListSelectionMode selection_mode_{ListSelectionMode::one};
    bool focused_{};
    Event<const ListSelectionChange&> selection_changed_;
    Event<std::size_t> item_activated_;
};

struct ItemCheckEvent final {
    std::size_t index{};
    CheckState current_state{CheckState::unchecked};
    CheckState new_state{CheckState::unchecked};
    bool cancel{};
};

class CheckedListBox final : public ListBox {
public:
    explicit CheckedListBox(StableId stable_id);

    void set_items(std::vector<std::string> items) override;
    void add_item(std::string item) override;
    void add_item(std::string item, CheckState state);
    void remove_item(std::size_t index) override;
    void clear_items() override;
    [[nodiscard]] CheckState item_check_state(std::size_t index) const;
    [[nodiscard]] bool item_checked(std::size_t index) const;
    void set_item_check_state(std::size_t index, CheckState state);
    void set_item_checked(std::size_t index, bool checked);
    void toggle_item(std::size_t index);
    [[nodiscard]] std::vector<std::size_t> checked_indices() const;
    [[nodiscard]] bool check_on_click() const noexcept { return check_on_click_; }
    void set_check_on_click(bool enabled);
    [[nodiscard]] Event<ItemCheckEvent&>& item_checking() noexcept {
        return item_checking_;
    }
    [[nodiscard]] Event<std::size_t, CheckState>& item_check_state_changed()
        noexcept { return item_check_state_changed_; }

    void on_pointer(PointerEvent& event) override;
    void on_key(KeyEvent& event) override;
    [[nodiscard]] std::vector<SemanticNode> semantic_virtual_children()
        const override;
    bool on_semantic_child_action(std::string_view stable_id,
                                  SemanticAction action,
                                  std::string_view value) override;

protected:
    [[nodiscard]] double row_text_left() const noexcept override;
    void paint_row_adornment(Painter& painter, std::size_t index,
                             Rect row_bounds, bool selected,
                             bool focused) const override;

private:
    std::vector<CheckState> check_states_;
    bool check_on_click_{};
    Event<ItemCheckEvent&> item_checking_;
    Event<std::size_t, CheckState> item_check_state_changed_;
};

class ComboBox final : public Panel {
public:
    explicit ComboBox(StableId stable_id);

    [[nodiscard]] std::span<const std::string> items() const noexcept {
        return items_;
    }
    void set_items(std::vector<std::string> items);
    void add_item(std::string item);
    [[nodiscard]] std::optional<std::size_t> selected_index() const noexcept {
        return selected_index_;
    }
    void set_selected_index(std::optional<std::size_t> index);
    [[nodiscard]] std::string_view selected_text() const noexcept;
    [[nodiscard]] std::string_view placeholder_text() const noexcept {
        return placeholder_;
    }
    void set_placeholder_text(std::string text);
    [[nodiscard]] bool dropped_down() const noexcept { return dropped_down_; }
    void set_dropped_down(bool dropped_down);
    [[nodiscard]] std::size_t maximum_drop_down_items() const noexcept {
        return maximum_drop_down_items_;
    }
    void set_maximum_drop_down_items(std::size_t count);
    [[nodiscard]] FontSpec font() const noexcept { return font_; }
    void set_font(FontSpec font);

    [[nodiscard]] Event<std::optional<std::size_t>>& selected_index_changed() noexcept {
        return selected_index_changed_;
    }
    [[nodiscard]] Event<bool>& drop_down_changed() noexcept {
        return drop_down_changed_;
    }

    void on_paint(Painter& painter, Rect local_damage) override;
    void on_pointer(PointerEvent& event) override;
    void on_key(KeyEvent& event) override;
    void on_focus_changed(bool focused) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    bool on_semantic_action(SemanticAction action,
                            std::string_view value) override;

protected:
    void on_detached_from_window() noexcept override;

private:
    void open_drop_down();
    void close_drop_down();
    void commit_popup_selection(std::size_t index);
    void on_popup_revoked();

    std::vector<std::string> items_;
    std::optional<std::size_t> selected_index_;
    std::string placeholder_;
    FontSpec font_{FontRole::content, 12.0, 400, false};
    std::size_t maximum_drop_down_items_{8U};
    bool dropped_down_{};
    bool focused_{};
    std::shared_ptr<Panel> popup_layer_;
    std::shared_ptr<ListBox> popup_list_;
    std::uint64_t popup_scope_{};
    PopupToken popup_token_;
    SubscriptionToken popup_selection_;
    SubscriptionToken popup_activation_;
    SubscriptionToken popup_dismissal_;
    SubscriptionToken popup_revocation_;
    Event<std::optional<std::size_t>> selected_index_changed_;
    Event<bool> drop_down_changed_;
    bool closing_popup_{};
};

class NumericUpDown final : public Panel {
public:
    explicit NumericUpDown(StableId stable_id);
    void initialize_control_tree();

    [[nodiscard]] double minimum() const noexcept { return minimum_; }
    [[nodiscard]] double maximum() const noexcept { return maximum_; }
    void set_range(double minimum, double maximum);
    [[nodiscard]] double value() const noexcept { return value_; }
    void set_value(double value);
    [[nodiscard]] double increment() const noexcept { return increment_; }
    void set_increment(double increment);
    [[nodiscard]] std::uint8_t decimal_places() const noexcept {
        return decimal_places_;
    }
    void set_decimal_places(std::uint8_t places);
    [[nodiscard]] bool hexadecimal() const noexcept { return hexadecimal_; }
    void set_hexadecimal(bool hexadecimal);
    [[nodiscard]] std::shared_ptr<TextBox> editor() const noexcept { return editor_; }
    [[nodiscard]] Event<double>& value_changed() noexcept { return value_changed_; }

    void arrange(Rect final_bounds) override;
    void on_key_preview(KeyEvent& event) override;
    void on_pointer_preview(PointerEvent& event) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    bool on_semantic_action(SemanticAction action,
                            std::string_view value) override;

private:
    void step(int direction);
    void commit_editor_text();
    void synchronize_editor();
    [[nodiscard]] std::string formatted_value() const;

    std::shared_ptr<TextBox> editor_;
    Control::Ptr spinner_;
    SubscriptionToken editor_change_;
    SubscriptionToken spinner_step_;
    double minimum_{0.0};
    double maximum_{100.0};
    double value_{};
    double increment_{1.0};
    std::uint8_t decimal_places_{};
    bool hexadecimal_{};
    bool synchronizing_{};
    Event<double> value_changed_;
};

} // namespace gui_forms
