#pragma once

#include "gui_forms/controls/panel/panel.hpp"
#include "gui_forms/scheduler.hpp"
#include "gui_forms/text.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
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
    [[nodiscard]] std::size_t maximum_length() const noexcept {
        return maximum_length_;
    }
    // Zero admits any text that fits the TextStore. A positive limit applies
    // to user edits, paste, and semantic set-value without truncating a value
    // assigned programmatically through set_text().
    void set_maximum_length(std::size_t length);
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
    std::size_t maximum_length_{};
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
    static constexpr std::size_t maximum_configured_length_ = 16U * 1024U * 1024U;
};

} // namespace gui_forms
