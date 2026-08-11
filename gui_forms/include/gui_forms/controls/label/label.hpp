#pragma once

#include "gui_forms/control.hpp"
#include "gui_forms/event.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace gui_forms {

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

enum class TextCaseTransform : std::uint8_t {
    none,
    uppercase_ascii,
    lowercase_ascii,
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
    [[nodiscard]] TextCaseTransform text_case_transform() const noexcept {
        return text_case_transform_;
    }
    void set_text_case_transform(TextCaseTransform transform);
    // Zero keeps all produced lines. A positive value bounds both desired
    // height and painting while semantics retain the complete authored text.
    [[nodiscard]] std::size_t maximum_lines() const noexcept {
        return maximum_lines_;
    }
    void set_maximum_lines(std::size_t maximum_lines);
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
    [[nodiscard]] PropertyValueOrigin font_property_origin() const noexcept;
    [[nodiscard]] PropertyValueOrigin foreground_property_origin() const noexcept;
    [[nodiscard]] BindingValue maximum_lines_property_value() const;
    void set_maximum_lines_property_value(const BindingValue& value);
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
    TextCaseTransform text_case_transform_{TextCaseTransform::none};
    std::size_t maximum_lines_{};
    bool use_mnemonic_{true};
    Event<const std::string&> text_changed_;
};

} // namespace gui_forms
