#pragma once

#include "gui_forms/control.hpp"
#include "gui_forms/event.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace gui_forms {

// Provisional built-in appearance for the bounded M6 control extractions. It is a
// value record rather than a platform theme object; M9 may replace its source
// without changing control state or event contracts.
struct BasicControlStyle final {
    Color face{Color::rgba(229, 234, 239)};
    Color face_light{Color::rgba(247, 249, 251)};
    Color paper{Color::rgba(255, 255, 255)};
    Color highlight{Color::rgba(255, 255, 255)};
    Color border{Color::rgba(148, 162, 175)};
    Color dark_border{Color::rgba(76, 94, 111)};
    Color text{Color::rgba(27, 39, 51)};
    Color disabled_text{Color::rgba(132, 143, 153)};
    Color accent{Color::rgba(38, 114, 185)};
    Color accent_light{Color::rgba(216, 235, 249)};
    Color link{Color::rgba(25, 82, 139)};
    Color visited_link{Color::rgba(93, 65, 145)};

    friend constexpr bool operator==(const BasicControlStyle&,
                                     const BasicControlStyle&) = default;
};

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

class Panel : public Control {
public:
    explicit Panel(StableId stable_id);

    [[nodiscard]] BorderStyle border_style() const noexcept { return border_style_; }
    void set_border_style(BorderStyle style);
    [[nodiscard]] Color background() const noexcept { return background_; }
    void set_background(Color color);
    [[nodiscard]] const BasicControlStyle& style() const noexcept { return style_; }
    void set_style(BasicControlStyle style);

    void on_paint(Painter& painter, Rect local_damage) override;

protected:
    [[nodiscard]] Rect local_bounds() const noexcept;
    void paint_panel(Painter& painter, Rect bounds) const;

private:
    BasicControlStyle style_;
    Color background_{style_.face};
    BorderStyle border_style_{BorderStyle::none};
};

class GroupBox : public Panel {
public:
    explicit GroupBox(StableId stable_id, std::string text = {});

    [[nodiscard]] const std::string& text() const noexcept { return text_; }
    void set_text(std::string text);
    [[nodiscard]] FontSpec font() const noexcept { return font_; }
    void set_font(FontSpec font);

    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    std::string text_;
    FontSpec font_{FontRole::control, 12.0, 600, false};
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
    [[nodiscard]] FontSpec font() const noexcept { return font_; }
    void set_font(FontSpec font);
    [[nodiscard]] Color foreground() const noexcept { return foreground_; }
    void set_foreground(Color color);
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
    std::string text_;
    FontSpec font_{FontRole::content, 12.0, 400, false};
    Color foreground_{Color::rgba(27, 39, 51)};
    HorizontalAlignment alignment_{HorizontalAlignment::near};
    VerticalAlignment vertical_alignment_{VerticalAlignment::center};
    TextWrapping text_wrapping_{TextWrapping::no_wrap};
    double line_spacing_{1.25};
    Event<const std::string&> text_changed_;
};

class ButtonBase : public Control {
public:
    explicit ButtonBase(StableId stable_id, std::string text = {});

    [[nodiscard]] const std::string& text() const noexcept { return text_; }
    virtual void set_text(std::string text);
    [[nodiscard]] FontSpec font() const noexcept { return font_; }
    void set_font(FontSpec font);
    [[nodiscard]] const BasicControlStyle& style() const noexcept { return style_; }
    void set_style(BasicControlStyle style);
    [[nodiscard]] bool pressed_visual() const noexcept {
        return pointer_pressed_ || keyboard_pressed_;
    }
    [[nodiscard]] bool focused_visual() const noexcept { return focused_; }
    [[nodiscard]] Event<ButtonBase&>& clicked() noexcept { return clicked_; }
    [[nodiscard]] Event<const std::string&>& text_changed() noexcept {
        return text_changed_;
    }

    [[nodiscard]] Size measure(Size available) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    void on_pointer(PointerEvent& event) override;
    void on_key(KeyEvent& event) override;
    void on_focus_changed(bool focused) override;
    void on_activate() override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    bool on_semantic_action(SemanticAction action,
                            std::string_view value) override;

protected:
    [[nodiscard]] Rect local_bounds() const noexcept;
    void paint_button_frame(Painter& painter, Rect bounds, bool default_cue) const;
    void paint_button_text(Painter& painter, Rect bounds,
                           std::string_view text) const;

private:
    std::string text_;
    FontSpec font_{FontRole::control, 12.0, 400, false};
    BasicControlStyle style_;
    Event<ButtonBase&> clicked_;
    Event<const std::string&> text_changed_;
    std::uint32_t keyboard_key_{};
    bool pointer_engaged_{};
    bool pointer_pressed_{};
    bool keyboard_pressed_{};
    bool focused_{};
};

class Button : public ButtonBase {
public:
    explicit Button(StableId stable_id, std::string text = {});

    [[nodiscard]] bool default_button() const noexcept { return default_button_; }
    void set_default_button(bool is_default);
    [[nodiscard]] ButtonVisualStyle visual_style() const noexcept {
        return visual_style_;
    }
    void set_visual_style(ButtonVisualStyle style);
    void on_paint(Painter& painter, Rect local_damage) override;

private:
    bool default_button_{};
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
