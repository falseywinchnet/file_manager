#include "gui_forms/controls/label/label.hpp"
#include "../basic/basic_control_rendering.hpp"
#include "gui_forms/detail/bound_member_function.hpp"
#include "gui_forms/detail/property_binding_adapters.hpp"
#include "gui_forms/text.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

namespace gui_forms {
namespace {
// These resolvers borrow their host only for the synchronous label_lines call.
struct LabelWidth final {
    const Label& label;
    FontSpec font{};
    double operator()(const std::string_view value) const {
        const ResolvedTextLayout layout = label.resolve_text_layout_utf8(value, font);
        return layout.logical_size.width;
    }
};

struct PainterLabelWidth final {
    Painter& painter;
    FontSpec font{};
    double operator()(const std::string_view value) const {
        const ResolvedTextLayout layout = painter.resolve_text_layout_utf8(value, font);
        return layout.logical_size.width;
    }
};
} // namespace

Label::Label(StableId stable_id, std::string text)
    : Control(std::move(stable_id)), text_(std::move(text)) {
    define_bindable_property({
        {"Text", BindingValueKind::text, "Appearance",
         "Text displayed by the label.", BindingValue{std::string{}},
         invalidation::text_content},
        detail::BindingMemberGetter<Label, std::string>(
            *this, &Label::text_),
        detail::ConvertedPropertySetter<Label, std::string>(
            *this, &Label::set_text, BindingValueKind::text,
            "Label.Text binding requires text"),
        detail::EventChangeConnector<const std::string&>(text_changed_), {}, {}});

    PropertyDescriptor font;
    font.name = "Font";
    font.kind = BindingValueKind::font;
    font.category = "Appearance";
    font.description =
        "Effective font; reset resumes inherited theme typography.";
    font.invalidation_effects = Dirty::measure | Dirty::paint | Dirty::semantics;
    font.bindable = false;
    define_bindable_property({
        std::move(font),
        detail::BindingNoexceptMethodGetter<Label, FontSpec>(
            *this, &Label::font),
        detail::DirectPropertySetter<Label, FontSpec>(
            *this, &Label::set_font), {},
        detail::BoundMemberFunction<void (Label::*)()>(
            *this, &Label::clear_font),
        detail::BoundMemberFunction<bool (Label::*)() const noexcept>(
            *this, &Label::has_font_override),
        detail::BoundMemberFunction<
            PropertyValueOrigin (Label::*)() const noexcept>(
                *this, &Label::font_property_origin)});

    PropertyDescriptor foreground;
    foreground.name = "ForeColor";
    foreground.kind = BindingValueKind::color;
    foreground.category = "Appearance";
    foreground.description =
        "Effective text color; reset resumes inherited theme color.";
    foreground.invalidation_effects = Dirty::paint | Dirty::semantics;
    foreground.bindable = false;
    define_bindable_property({
        std::move(foreground),
        detail::BindingNoexceptMethodGetter<Label, Color>(
            *this, &Label::foreground),
        detail::DirectPropertySetter<Label, Color>(
            *this, &Label::set_foreground), {},
        detail::BoundMemberFunction<void (Label::*)()>(
            *this, &Label::clear_foreground),
        detail::BoundMemberFunction<bool (Label::*)() const noexcept>(
            *this, &Label::has_foreground_override),
        detail::BoundMemberFunction<
            PropertyValueOrigin (Label::*)() const noexcept>(
                *this, &Label::foreground_property_origin)});

    PropertyDescriptor maximum_lines;
    maximum_lines.name = "MaximumLines";
    maximum_lines.kind = BindingValueKind::unsigned_integer;
    maximum_lines.category = "Layout";
    maximum_lines.description =
        "Maximum painted and measured line count; zero keeps every line.";
    maximum_lines.default_value = BindingValue{std::uint64_t{0}};
    maximum_lines.invalidation_effects = Dirty::measure | Dirty::paint;
    maximum_lines.bindable = false;
    define_bindable_property({
        std::move(maximum_lines),
        detail::BoundMemberFunction<BindingValue (Label::*)() const>(
            *this, &Label::maximum_lines_property_value),
        detail::BoundMemberFunction<
            void (Label::*)(const BindingValue&)>(
                *this, &Label::set_maximum_lines_property_value),
        {}, {}, {}});
}

PropertyValueOrigin Label::font_property_origin() const noexcept {
    return has_font_override() ? PropertyValueOrigin::local
                               : PropertyValueOrigin::inherited;
}

PropertyValueOrigin Label::foreground_property_origin() const noexcept {
    return has_foreground_override() ? PropertyValueOrigin::local
                                     : PropertyValueOrigin::inherited;
}

BindingValue Label::maximum_lines_property_value() const {
    return BindingValue{static_cast<std::uint64_t>(maximum_lines_)};
}

void Label::set_maximum_lines_property_value(const BindingValue& value) {
    set_maximum_lines(static_cast<std::size_t>(
        std::get<std::uint64_t>(value)));
}

void Label::set_text(std::string text) {
    require_mutable();
    if (text_ == text) {
        return;
    }
    text_ = std::move(text);
    // Text only changes retained geometry when this label is content-sized.
    // Fixed labels are a common high-rate telemetry surface; making every text
    // update a measure request needlessly invalidates every layout ancestor.
    const Rect requested = requested_bounds();
    Dirty effects = Dirty::paint | Dirty::semantics | Dirty::accessibility;
    if (auto_size() || requested.width <= 0.0 || requested.height <= 0.0) {
        effects |= Dirty::measure | Dirty::arrange;
    }
    invalidate(effects);
    publish_change(text_changed_, text_);
}

FontSpec Label::font() const noexcept {
    if (font_override_) return *font_override_;
    const ThemeTypographyTokens& typography =
        effective_theme().structure().typography;
    switch (text_style_role_) {
    case TextStyleRole::body: return typography.field;
    case TextStyleRole::control: return typography.control;
    case TextStyleRole::caption: return typography.caption;
    case TextStyleRole::heading: return typography.heading;
    case TextStyleRole::title: return typography.title;
    case TextStyleRole::monospace: return typography.monospace;
    }
    return typography.field;
}

void Label::set_font(FontSpec font) {
    require_mutable();
    if (!valid_font_spec(font)) {
        throw std::invalid_argument("Label font specification is invalid");
    }
    if (font_override_ && *font_override_ == font) {
        return;
    }
    font_override_ = font;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void Label::clear_font() {
    require_mutable();
    if (!font_override_) return;
    font_override_.reset();
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void Label::set_text_style_role(TextStyleRole role) {
    require_mutable();
    switch (role) {
    case TextStyleRole::body:
    case TextStyleRole::control:
    case TextStyleRole::caption:
    case TextStyleRole::heading:
    case TextStyleRole::title:
    case TextStyleRole::monospace:
        break;
    default:
        throw std::invalid_argument("Label text style role is invalid");
    }
    if (text_style_role_ == role) return;
    text_style_role_ = role;
    if (!font_override_) {
        invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
    } else {
        invalidate(Dirty::semantics);
    }
}

Color Label::foreground() const noexcept {
    if (foreground_override_) return *foreground_override_;
    return effective_theme().resolve(ControlVisualRole::panel,
                                     visual_context()).text;
}

void Label::set_foreground(Color color) {
    require_mutable();
    if (foreground_override_ && *foreground_override_ == color) {
        return;
    }
    foreground_override_ = color;
    invalidate(Dirty::paint | Dirty::semantics);
}

void Label::clear_foreground() {
    require_mutable();
    if (!foreground_override_) return;
    foreground_override_.reset();
    invalidate(Dirty::paint | Dirty::semantics);
}

void Label::set_alignment(HorizontalAlignment alignment) {
    require_mutable();
    switch (alignment) {
    case HorizontalAlignment::near:
    case HorizontalAlignment::center:
    case HorizontalAlignment::far:
        break;
    default:
        throw std::invalid_argument("Label horizontal alignment is invalid");
    }
    if (alignment_ == alignment) {
        return;
    }
    alignment_ = alignment;
    invalidate(Dirty::paint | Dirty::semantics);
}

void Label::set_vertical_alignment(VerticalAlignment alignment) {
    require_mutable();
    switch (alignment) {
    case VerticalAlignment::near:
    case VerticalAlignment::center:
    case VerticalAlignment::far:
        break;
    default:
        throw std::invalid_argument("Label vertical alignment is invalid");
    }
    if (vertical_alignment_ == alignment) {
        return;
    }
    vertical_alignment_ = alignment;
    invalidate(Dirty::paint | Dirty::semantics);
}

void Label::set_text_wrapping(TextWrapping wrapping) {
    require_mutable();
    switch (wrapping) {
    case TextWrapping::no_wrap:
    case TextWrapping::word:
        break;
    default:
        throw std::invalid_argument("Label text wrapping mode is invalid");
    }
    if (text_wrapping_ == wrapping) {
        return;
    }
    text_wrapping_ = wrapping;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void Label::set_line_spacing(double spacing) {
    require_mutable();
    if (!std::isfinite(spacing) || spacing < 0.75 || spacing > 3.0) {
        throw std::invalid_argument("Label line spacing must be finite and between 0.75 and 3.0");
    }
    if (line_spacing_ == spacing) {
        return;
    }
    line_spacing_ = spacing;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void Label::set_text_case_transform(TextCaseTransform transform) {
    require_mutable();
    switch (transform) {
    case TextCaseTransform::none:
    case TextCaseTransform::uppercase_ascii:
    case TextCaseTransform::lowercase_ascii:
        break;
    default:
        throw std::invalid_argument("Label text case transform is invalid");
    }
    if (text_case_transform_ == transform) return;
    text_case_transform_ = transform;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void Label::set_maximum_lines(std::size_t maximum_lines) {
    require_mutable();
    if (maximum_lines > 4096U) {
        throw std::out_of_range(
            "Label maximum lines must be zero or at most 4096");
    }
    if (maximum_lines_ == maximum_lines) return;
    maximum_lines_ = maximum_lines;
    invalidate(Dirty::measure | Dirty::paint);
}

void Label::set_use_mnemonic(bool value) {
    require_mutable();
    if (use_mnemonic_ == value) return;
    use_mnemonic_ = value;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

Size Label::measure(Size available) {
    const Rect requested = requested_bounds();
    const std::string text = display_text();
    const FontSpec font = effective_font((*this).font());
    const double wrap_width = requested.width > 0.0
        ? requested.width : available.width;
    const TextWidthResolver resolve{LabelWidth{*this, font}};
    std::vector<std::string> lines = label_lines(
        text, font, std::max(0.0, wrap_width - 4.0), text_wrapping_, resolve, maximum_lines_);
    double content_width{};
    double line_height{};
    for (const std::string& line : lines) {
        const ResolvedTextLayout metrics = resolve_text_layout_utf8(line, font);
        content_width = std::max(content_width, metrics.logical_size.width);
        line_height = std::max(
            line_height, std::max(font.size, metrics.logical_size.height) *
                             line_spacing_);
    }
    if (line_height <= 0.0) line_height = font.size * line_spacing_;
    const double preferred_width = requested.width > 0.0
        ? requested.width : content_width + 4.0;
    const double preferred_height = requested.height > 0.0
        ? requested.height
        : static_cast<double>(lines.size()) * line_height + 4.0;
    return {std::min(available.width, preferred_width),
            std::min(available.height, preferred_height)};
}

std::string Label::display_text() const {
    std::string display = use_mnemonic_
        ? parse_mnemonic_text(text_).display_text : text_;
    for (char& character : display) {
        if (text_case_transform_ == TextCaseTransform::uppercase_ascii &&
            character >= 'a' && character <= 'z') {
            character = static_cast<char>(character - 'a' + 'A');
        } else if (text_case_transform_ == TextCaseTransform::lowercase_ascii &&
                   character >= 'A' && character <= 'Z') {
            character = static_cast<char>(character - 'A' + 'a');
        }
    }
    return display;
}

void Label::paint_label_text(Painter& painter, std::string_view text) const {
    const Rect arranged = committed_arranged_bounds();
    const FontSpec font = effective_font((*this).font());
    const TextWidthResolver resolve{PainterLabelWidth{painter, font}};
    std::vector<std::string> lines = label_lines(
        text, font, std::max(0.0, arranged.width - 4.0), text_wrapping_, resolve, maximum_lines_);
    std::vector<ResolvedTextLayout> metrics;
    metrics.reserve(lines.size());
    double line_height{};
    for (const std::string& line : lines) {
        metrics.push_back(painter.resolve_text_layout_utf8(line, font));
        line_height = std::max(
            line_height,
            std::max(font.size, metrics.back().logical_size.height) *
                line_spacing_);
    }
    if (line_height <= 0.0) line_height = font.size * line_spacing_;
    const double block_height = static_cast<double>(lines.size()) * line_height;
    double top = 1.0;
    if (vertical_alignment_ == VerticalAlignment::center) {
        top = std::max(1.0, (arranged.height - block_height) * 0.5);
    } else if (vertical_alignment_ == VerticalAlignment::far) {
        top = std::max(1.0, arranged.height - block_height - 1.0);
    }
    const Color color = foreground();
    for (std::size_t index = 0; index < lines.size(); ++index) {
        const double text_width = metrics[index].logical_size.width;
        double x = 2.0;
        if (alignment_ == HorizontalAlignment::center) {
            x = std::max(2.0, (arranged.width - text_width) * 0.5);
        } else if (alignment_ == HorizontalAlignment::far) {
            x = std::max(2.0, arranged.width - text_width - 2.0);
        }
        const double leading = std::max(
            0.0, line_height - metrics[index].logical_size.height);
        const double baseline = snap_text_baseline(
            top + leading * 0.5 + metrics[index].ascent +
                static_cast<double>(index) * line_height);
        painter.draw_text_utf8({x, baseline}, lines[index], font, color);
    }
}

void Label::on_paint(Painter& painter, Rect) {
    paint_label_text(painter, display_text());
}

bool Label::mnemonic_matches(char32_t character) const noexcept {
    return use_mnemonic_ && is_mnemonic(character, text_);
}

bool Label::process_mnemonic_self(char32_t character) {
    if (!mnemonic_matches(character)) return false;
    static_cast<void>(focus_next_after_self());
    return true;
}

bool Label::hit_test_local(Point) const {
    return false;
}

SemanticDescriptor Label::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::static_text;
    descriptor.name = accessible_name().empty()
        ? (use_mnemonic_ ? parse_mnemonic_text(text_).display_text : text_)
        : accessible_name();
    descriptor.description = accessible_description();
    descriptor.exposed = !descriptor.name.empty();
    return descriptor;
}

} // namespace gui_forms
