#pragma once

#include "gui_forms/scrolling.hpp"
#include "gui_forms/event.hpp"
#include "gui_forms/image_list.hpp"
#include "gui_forms/theme.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace gui_forms {

enum class BorderStyle : std::uint8_t {
    none,
    line,
    sunken,
    raised,
};

enum class HorizontalAlignment : std::uint8_t {
    near,
    center,
    far,
};

enum class VerticalAlignment : std::uint8_t {
    near,
    center,
    far,
};

enum class TextWrapping : std::uint8_t {
    no_wrap,
    word,
};

enum class TextStyleRole : std::uint8_t {
    body,
    control,
    caption,
    heading,
    title,
    monospace,
};

enum class ContentAlignment : std::uint8_t {
    top_left,
    top_center,
    top_right,
    middle_left,
    middle_center,
    middle_right,
    bottom_left,
    bottom_center,
    bottom_right,
};

enum class TextImageRelation : std::uint8_t {
    overlay,
    image_before_text,
    text_before_image,
    image_above_text,
    text_above_image,
};

enum class ButtonVisualStyle : std::uint8_t {
    standard,
    flat,
    accent,
    command,
};

enum class ChoiceIndicatorStyle : std::uint8_t {
    classic,
    modern,
    toggle,
};

// Mirrors the five System.Windows.Forms PictureBoxSizeMode policies. Image
// storage remains owned by Window's renderer-neutral ImageRegistry; PictureBox
// is only a retained presentation consumer of a generational ImageId.
enum class PictureBoxSizeMode : std::uint8_t {
    normal,
    stretch_image,
    auto_size,
    center_image,
    zoom,
};

class Panel : public ScrollableControl {
public:
    explicit Panel(StableId stable_id);

    [[nodiscard]] BorderStyle border_style() const noexcept { return border_style_; }
    void set_border_style(BorderStyle style);
    [[nodiscard]] Color background() const noexcept;
    [[nodiscard]] bool has_background_override() const noexcept {
        return background_override_.has_value();
    }
    void set_background(Color color);
    void clear_background();
    [[nodiscard]] const BasicControlStyle& style() const noexcept;
    [[nodiscard]] bool has_style_override() const noexcept {
        return style_override_.has_value();
    }
    void set_style(BasicControlStyle style);
    void clear_style();
    [[nodiscard]] ControlVisualRole visual_role() const noexcept {
        return visual_role_;
    }
    void set_visual_role(ControlVisualRole role);

    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] Insets visual_outsets() const noexcept override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

protected:
    [[nodiscard]] Rect local_bounds() const noexcept;
    void paint_panel(Painter& painter, Rect bounds) const;

private:
    std::optional<BasicControlStyle> style_override_;
    std::optional<Color> background_override_;
    ControlVisualRole visual_role_{ControlVisualRole::panel};
    BorderStyle border_style_{BorderStyle::none};
};

class GroupBox : public Panel {
public:
    explicit GroupBox(StableId stable_id, std::string text = {});

    [[nodiscard]] const std::string& text() const noexcept { return text_; }
    void set_text(std::string text);
    [[nodiscard]] FontSpec font() const noexcept { return font_; }
    void set_font(FontSpec font);
    [[nodiscard]] bool use_mnemonic() const noexcept { return use_mnemonic_; }
    void set_use_mnemonic(bool value);

    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    [[nodiscard]] bool mnemonic_matches(
        char32_t character) const noexcept override;
    bool process_mnemonic_self(char32_t character) override;
    std::string text_;
    FontSpec font_{FontRole::control, 12.0, 600, false, 0.24};
    bool use_mnemonic_{true};
};

class PictureBox : public Panel {
public:
    explicit PictureBox(StableId stable_id);

    [[nodiscard]] ImageId image() const noexcept { return image_; }
    void set_image(ImageId image);
    void clear_image();
    [[nodiscard]] bool has_valid_image() const noexcept;
    [[nodiscard]] Size image_size() const noexcept;
    [[nodiscard]] PictureBoxSizeMode size_mode() const noexcept {
        return size_mode_;
    }
    void set_size_mode(PictureBoxSizeMode mode);
    [[nodiscard]] double image_opacity() const noexcept { return image_opacity_; }
    void set_image_opacity(double opacity);
    [[nodiscard]] Rect image_bounds() const noexcept;
    [[nodiscard]] Event<ImageId>& image_changed() noexcept { return image_changed_; }

    [[nodiscard]] Size measure(Size available) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] bool hit_test_local(Point local_point) const override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    [[nodiscard]] Rect content_bounds() const noexcept;

    ImageId image_{};
    PictureBoxSizeMode size_mode_{PictureBoxSizeMode::normal};
    double image_opacity_{1.0};
    Event<ImageId> image_changed_;
};

class Label : public Control {
public:
    explicit Label(StableId stable_id, std::string text = {});

    [[nodiscard]] const std::string& text() const noexcept { return text_; }
    virtual void set_text(std::string text);
    [[nodiscard]] FontSpec font() const noexcept;
    [[nodiscard]] bool has_font_override() const noexcept {
        return font_override_.has_value();
    }
    void set_font(FontSpec font);
    void clear_font();
    [[nodiscard]] TextStyleRole text_style_role() const noexcept {
        return text_style_role_;
    }
    void set_text_style_role(TextStyleRole role);
    [[nodiscard]] Color foreground() const noexcept;
    [[nodiscard]] bool has_foreground_override() const noexcept {
        return foreground_override_.has_value();
    }
    void set_foreground(Color color);
    void clear_foreground();
    [[nodiscard]] HorizontalAlignment alignment() const noexcept { return alignment_; }
    void set_alignment(HorizontalAlignment alignment);
    [[nodiscard]] VerticalAlignment vertical_alignment() const noexcept {
        return vertical_alignment_;
    }
    void set_vertical_alignment(VerticalAlignment alignment);
    [[nodiscard]] TextWrapping text_wrapping() const noexcept { return text_wrapping_; }
    void set_text_wrapping(TextWrapping wrapping);
    [[nodiscard]] double line_spacing() const noexcept { return line_spacing_; }
    void set_line_spacing(double spacing);
    [[nodiscard]] bool use_mnemonic() const noexcept { return use_mnemonic_; }
    void set_use_mnemonic(bool value);
    [[nodiscard]] Event<const std::string&>& text_changed() noexcept {
        return text_changed_;
    }

    [[nodiscard]] Size measure(Size available) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] bool hit_test_local(Point local_point) const override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

protected:
    [[nodiscard]] virtual std::string display_text() const;
    void paint_label_text(Painter& painter, std::string_view text) const;

private:
    [[nodiscard]] bool mnemonic_matches(
        char32_t character) const noexcept override;
    bool process_mnemonic_self(char32_t character) override;
    std::string text_;
    std::optional<FontSpec> font_override_;
    std::optional<Color> foreground_override_;
    TextStyleRole text_style_role_{TextStyleRole::body};
    HorizontalAlignment alignment_{HorizontalAlignment::near};
    VerticalAlignment vertical_alignment_{VerticalAlignment::center};
    TextWrapping text_wrapping_{TextWrapping::no_wrap};
    double line_spacing_{1.25};
    bool use_mnemonic_{true};
    Event<const std::string&> text_changed_;
};

class ButtonBase : public Control {
public:
    explicit ButtonBase(StableId stable_id, std::string text = {});

    [[nodiscard]] const std::string& text() const noexcept { return text_; }
    virtual void set_text(std::string text);
    [[nodiscard]] FontSpec font() const noexcept { return font_; }
    void set_font(FontSpec font);
    [[nodiscard]] const BasicControlStyle& style() const noexcept;
    [[nodiscard]] bool has_style_override() const noexcept {
        return style_override_.has_value();
    }
    void set_style(BasicControlStyle style);
    void clear_style();
    [[nodiscard]] ImageId image() const noexcept { return image_; }
    void set_image(ImageId image);
    void clear_image();
    [[nodiscard]] std::shared_ptr<ImageList> image_list() const noexcept {
        return image_list_;
    }
    void set_image_list(std::shared_ptr<ImageList> image_list);
    [[nodiscard]] int image_index() const noexcept { return image_index_; }
    void set_image_index(int image_index);
    [[nodiscard]] const std::string& image_key() const noexcept {
        return image_key_;
    }
    void set_image_key(std::string image_key);
    [[nodiscard]] ContentAlignment image_alignment() const noexcept {
        return image_alignment_;
    }
    void set_image_alignment(ContentAlignment alignment);
    [[nodiscard]] ContentAlignment text_alignment() const noexcept {
        return text_alignment_;
    }
    void set_text_alignment(ContentAlignment alignment);
    [[nodiscard]] TextImageRelation text_image_relation() const noexcept {
        return text_image_relation_;
    }
    void set_text_image_relation(TextImageRelation relation);
    [[nodiscard]] double image_gap() const noexcept { return image_gap_; }
    void set_image_gap(double gap);
    [[nodiscard]] bool use_mnemonic() const noexcept { return use_mnemonic_; }
    void set_use_mnemonic(bool value);
    // Executes the same validated command path used by mnemonics, semantic
    // press, and a Window accept/cancel button. Returns false when unavailable
    // or when validation prevents the command.
    bool perform_click();
    [[nodiscard]] bool pressed_visual() const noexcept {
        return pointer_pressed_ || keyboard_pressed_;
    }
    [[nodiscard]] bool focused_visual() const noexcept { return focused_; }
    [[nodiscard]] bool hovered_visual() const noexcept { return hovered_; }
    // Optional disclosure state for buttons that own a popup or retained
    // disclosure region. Nullopt is an ordinary push button; false/true expose
    // collapsed/expanded semantics without conflating visual pressed state.
    [[nodiscard]] std::optional<bool> expanded_state() const noexcept {
        return expanded_state_;
    }
    void set_expanded_state(std::optional<bool> expanded);
    [[nodiscard]] Event<ButtonBase&>& clicked() noexcept { return clicked_; }
    [[nodiscard]] Event<const std::string&>& text_changed() noexcept {
        return text_changed_;
    }

    [[nodiscard]] Size measure(Size available) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] Insets visual_outsets() const noexcept override;
    void on_pointer(PointerEvent& event) override;
    void on_key(KeyEvent& event) override;
    void on_focus_changed(bool focused) override;
    void on_activate() override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    bool on_semantic_action(SemanticAction action,
                            std::string_view value) override;

protected:
    [[nodiscard]] Rect local_bounds() const noexcept;
    [[nodiscard]] std::string display_text() const;
    void paint_button_frame(Painter& painter, Rect bounds, bool default_cue) const;
    void paint_button_text(Painter& painter, Rect bounds,
                           std::string_view text) const;
    void paint_button_content(Painter& painter, Rect bounds,
                              std::string_view text, Color foreground,
                              Point offset = {}, bool selected = false,
                              bool command_alignment = false) const;
    void paint_themed_button(Painter& painter, Rect bounds,
                             ControlVisualRole role, bool default_cue,
                             bool command_alignment = false) const;
    [[nodiscard]] ImageListResolution resolved_button_image(
        bool selected = false) const noexcept;
    void on_attached_to_window() override;
    bool perform_dialog_command() override;
    [[nodiscard]] bool supports_dialog_command() const noexcept override {
        return true;
    }
    [[nodiscard]] bool mnemonic_matches(
        char32_t character) const noexcept override;
    bool process_mnemonic_self(char32_t character) override;

private:
    std::string text_;
    FontSpec font_{FontRole::control, 12.0, 400, false, 0.24};
    std::optional<BasicControlStyle> style_override_;
    ImageId image_{};
    std::shared_ptr<ImageList> image_list_;
    SubscriptionToken image_list_changed_;
    std::string image_key_;
    int image_index_{-1};
    ContentAlignment image_alignment_{ContentAlignment::middle_center};
    ContentAlignment text_alignment_{ContentAlignment::middle_center};
    TextImageRelation text_image_relation_{TextImageRelation::overlay};
    double image_gap_{4.0};
    Event<ButtonBase&> clicked_;
    Event<const std::string&> text_changed_;
    std::uint32_t keyboard_key_{};
    bool pointer_engaged_{};
    bool pointer_pressed_{};
    bool keyboard_pressed_{};
    bool focused_{};
    bool hovered_{};
    bool use_mnemonic_{true};
    std::optional<bool> expanded_state_;
};

class Button : public ButtonBase {
public:
    explicit Button(StableId stable_id, std::string text = {});

    [[nodiscard]] bool default_button() const noexcept { return default_button_; }
    void set_default_button(bool is_default);
    [[nodiscard]] DialogResult dialog_result() const noexcept {
        return dialog_result_;
    }
    void set_dialog_result(DialogResult result);
    [[nodiscard]] Event<DialogResult>& dialog_result_changed() noexcept {
        return dialog_result_changed_;
    }
    [[nodiscard]] ButtonVisualStyle visual_style() const noexcept {
        return visual_style_;
    }
    void set_visual_style(ButtonVisualStyle style);
    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] Insets visual_outsets() const noexcept override;

private:
    void notify_default(bool value) override;
    void on_activate() override;
    [[nodiscard]] DialogResult command_dialog_result() const noexcept override {
        return dialog_result_;
    }
    void assign_cancel_dialog_result() override;
    bool default_button_{};
    DialogResult dialog_result_{DialogResult::none};
    Event<DialogResult> dialog_result_changed_;
    ButtonVisualStyle visual_style_{ButtonVisualStyle::standard};
};

enum class CheckState : std::uint8_t {
    unchecked,
    checked,
    indeterminate,
};

class CheckBox : public ButtonBase {
public:
    explicit CheckBox(StableId stable_id, std::string text = {});

    [[nodiscard]] CheckState check_state() const noexcept { return check_state_; }
    void set_check_state(CheckState state);
    [[nodiscard]] bool checked() const noexcept {
        return check_state_ == CheckState::checked;
    }
    void set_checked(bool checked);
    [[nodiscard]] bool three_state() const noexcept { return three_state_; }
    void set_three_state(bool enabled);
    [[nodiscard]] bool auto_check() const noexcept { return auto_check_; }
    void set_auto_check(bool enabled);
    [[nodiscard]] ChoiceIndicatorStyle indicator_style() const noexcept {
        return indicator_style_;
    }
    void set_indicator_style(ChoiceIndicatorStyle style);
    [[nodiscard]] Event<CheckState>& check_state_changed() noexcept {
        return check_state_changed_;
    }
    [[nodiscard]] Event<bool>& checked_changed() noexcept {
        return checked_changed_;
    }

    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] Insets visual_outsets() const noexcept override;
    void on_activate() override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    CheckState check_state_{CheckState::unchecked};
    Event<CheckState> check_state_changed_;
    Event<bool> checked_changed_;
    bool three_state_{};
    bool auto_check_{true};
    ChoiceIndicatorStyle indicator_style_{ChoiceIndicatorStyle::classic};
};

class RadioButton : public ButtonBase {
public:
    explicit RadioButton(StableId stable_id, std::string text = {});

    [[nodiscard]] bool checked() const noexcept { return checked_; }
    void set_checked(bool checked);
    [[nodiscard]] const std::string& group_name() const noexcept { return group_name_; }
    void set_group_name(std::string name);
    [[nodiscard]] bool auto_check() const noexcept { return auto_check_; }
    void set_auto_check(bool enabled);
    [[nodiscard]] ChoiceIndicatorStyle indicator_style() const noexcept {
        return indicator_style_;
    }
    void set_indicator_style(ChoiceIndicatorStyle style);
    [[nodiscard]] Event<bool>& checked_changed() noexcept {
        return checked_changed_;
    }

    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] Insets visual_outsets() const noexcept override;
    void on_activate() override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    void set_checked_without_exclusion(bool checked);

    std::string group_name_;
    Event<bool> checked_changed_;
    bool checked_{};
    bool auto_check_{true};
    ChoiceIndicatorStyle indicator_style_{ChoiceIndicatorStyle::classic};
};

class LinkLabel : public ButtonBase {
public:
    explicit LinkLabel(StableId stable_id, std::string text = {});

    [[nodiscard]] bool visited() const noexcept { return visited_; }
    void set_visited(bool visited);
    void on_paint(Painter& painter, Rect local_damage) override;
    void on_activate() override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    bool visited_{};
};

} // namespace gui_forms
