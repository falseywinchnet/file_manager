#include "gui_forms/basic_controls.hpp"
#include "gui_forms/text.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

namespace gui_forms {
namespace {

std::shared_ptr<const PropertyEnumDescriptor> picture_box_size_mode_enum() {
    static const auto value = std::make_shared<const PropertyEnumDescriptor>(
        PropertyEnumDescriptor{
            "System.Windows.Forms.PictureBoxSizeMode",
            {{"Normal", 0}, {"StretchImage", 1}, {"AutoSize", 2},
             {"CenterImage", 3}, {"Zoom", 4}},
            false});
    return value;
}

BindingValue picture_box_size_mode_value(PictureBoxSizeMode mode) {
    PropertyDescriptor descriptor;
    descriptor.kind = BindingValueKind::enumeration;
    descriptor.enumeration = picture_box_size_mode_enum();
    const auto normalized = convert_property_value(
        BindingValue{static_cast<std::int64_t>(mode)}, descriptor);
    if (!normalized) {
        throw std::logic_error(
            "GUI.Forms retained PictureBoxSizeMode is outside its property schema");
    }
    return *normalized;
}

[[nodiscard]] double estimated_text_width(std::string_view text,
                                          FontSpec font) noexcept {
    std::size_t scalars{};
    for (const unsigned char byte : text) {
        if ((byte & 0xc0U) != 0x80U) {
            ++scalars;
        }
    }
    return static_cast<double>(scalars) * font.size * 0.56;
}

[[nodiscard]] bool valid_content_alignment(ContentAlignment alignment) noexcept {
    switch (alignment) {
    case ContentAlignment::top_left:
    case ContentAlignment::top_center:
    case ContentAlignment::top_right:
    case ContentAlignment::middle_left:
    case ContentAlignment::middle_center:
    case ContentAlignment::middle_right:
    case ContentAlignment::bottom_left:
    case ContentAlignment::bottom_center:
    case ContentAlignment::bottom_right: return true;
    }
    return false;
}

[[nodiscard]] bool valid_text_image_relation(TextImageRelation relation) noexcept {
    switch (relation) {
    case TextImageRelation::overlay:
    case TextImageRelation::image_before_text:
    case TextImageRelation::text_before_image:
    case TextImageRelation::image_above_text:
    case TextImageRelation::text_above_image: return true;
    }
    return false;
}

[[nodiscard]] Rect aligned_rect(Rect bounds, Size size,
                                ContentAlignment alignment) noexcept {
    const double width = std::min(std::max(0.0, size.width), bounds.width);
    const double height = std::min(std::max(0.0, size.height), bounds.height);
    double x = bounds.x;
    double y = bounds.y;
    switch (alignment) {
    case ContentAlignment::top_center:
    case ContentAlignment::middle_center:
    case ContentAlignment::bottom_center:
        x += (bounds.width - width) * 0.5;
        break;
    case ContentAlignment::top_right:
    case ContentAlignment::middle_right:
    case ContentAlignment::bottom_right:
        x += bounds.width - width;
        break;
    default: break;
    }
    switch (alignment) {
    case ContentAlignment::middle_left:
    case ContentAlignment::middle_center:
    case ContentAlignment::middle_right:
        y += (bounds.height - height) * 0.5;
        break;
    case ContentAlignment::bottom_left:
    case ContentAlignment::bottom_center:
    case ContentAlignment::bottom_right:
        y += bounds.height - height;
        break;
    default: break;
    }
    return {x, y, width, height};
}

[[nodiscard]] std::vector<std::string> label_lines(std::string_view text,
                                                    FontSpec font,
                                                    double width,
                                                    TextWrapping wrapping) {
    std::vector<std::string> lines;
    std::size_t paragraph_start{};
    while (paragraph_start <= text.size()) {
        const std::size_t newline = text.find('\n', paragraph_start);
        const std::size_t paragraph_end = newline == std::string_view::npos
            ? text.size() : newline;
        const std::string_view paragraph =
            text.substr(paragraph_start, paragraph_end - paragraph_start);
        if (wrapping == TextWrapping::no_wrap || width <= 4.0 || paragraph.empty()) {
            lines.emplace_back(paragraph);
        } else {
            std::string line;
            std::size_t cursor{};
            while (cursor < paragraph.size()) {
                while (cursor < paragraph.size() &&
                       std::isspace(static_cast<unsigned char>(paragraph[cursor])) != 0) {
                    ++cursor;
                }
                if (cursor >= paragraph.size()) {
                    break;
                }
                std::size_t word_end = cursor;
                while (word_end < paragraph.size() &&
                       std::isspace(static_cast<unsigned char>(paragraph[word_end])) == 0) {
                    ++word_end;
                }
                const std::string_view word = paragraph.substr(cursor, word_end - cursor);
                std::string candidate = line;
                if (!candidate.empty()) {
                    candidate.push_back(' ');
                }
                candidate.append(word);
                if (!line.empty() && estimated_text_width(candidate, font) > width) {
                    lines.push_back(std::move(line));
                    line.assign(word);
                } else {
                    line = std::move(candidate);
                }
                cursor = word_end;
            }
            if (!line.empty()) {
                lines.push_back(std::move(line));
            } else if (paragraph.empty()) {
                lines.emplace_back();
            }
        }
        if (newline == std::string_view::npos) {
            break;
        }
        paragraph_start = newline + 1U;
    }
    if (lines.empty()) {
        lines.emplace_back();
    }
    return lines;
}

void paint_relief(Painter& painter, Rect bounds, const BasicControlStyle& style,
                  bool pressed) {
    painter.fill_rect(bounds, style.face);
    if (!pressed && bounds.width > 3.0 && bounds.height > 3.0) {
        painter.fill_rect({bounds.x + 1.0, bounds.y + 1.0,
                           std::max(0.0, bounds.width - 2.0),
                           std::floor(std::max(0.0, bounds.height - 2.0) * 0.45)},
                          style.face_light);
    }
    const Color top = pressed ? style.dark_border : style.highlight;
    const Color bottom = pressed ? style.highlight : style.dark_border;
    painter.draw_line({bounds.x, bounds.y},
                      {bounds.x + bounds.width - 1.0, bounds.y}, top, 1.0);
    painter.draw_line({bounds.x, bounds.y},
                      {bounds.x, bounds.y + bounds.height - 1.0}, top, 1.0);
    painter.draw_line({bounds.x, bounds.y + bounds.height - 1.0},
                      {bounds.x + bounds.width - 1.0,
                       bounds.y + bounds.height - 1.0}, bottom, 1.0);
    painter.draw_line({bounds.x + bounds.width - 1.0, bounds.y},
                      {bounds.x + bounds.width - 1.0,
                       bounds.y + bounds.height - 1.0}, bottom, 1.0);
    if (bounds.width > 4.0 && bounds.height > 4.0) {
        painter.stroke_rect({bounds.x + 1.5, bounds.y + 1.5,
                             bounds.width - 3.0, bounds.height - 3.0},
                            pressed ? style.border : style.face_light, 1.0);
    }
}

void fill_radio_disc(Painter& painter, double left, double top, Color color,
                     bool inner) {
    // Painter deliberately exposes only renderer-neutral primitives today.
    // These one-pixel chords preserve a genuinely round 15 px WinForms radio
    // indicator without adding an ellipse primitive to every host backend.
    static constexpr int outer_left[] = {5, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 5};
    static constexpr int outer_width[] = {5, 9, 11, 13, 15, 15, 15, 15, 15, 15, 15, 13, 11, 9, 5};
    static constexpr int inner_left[] = {2, 1, 0, 0, 0, 1, 2};
    static constexpr int inner_width[] = {3, 5, 7, 7, 7, 5, 3};
    if (inner) {
        for (int row = 0; row < 7; ++row) {
            painter.fill_rect({left + 4.0 + inner_left[row], top + 4.0 + row,
                               static_cast<double>(inner_width[row]), 1.0}, color);
        }
        return;
    }
    for (int row = 0; row < 15; ++row) {
        painter.fill_rect({left + outer_left[row], top + row,
                           static_cast<double>(outer_width[row]), 1.0}, color);
    }
}

void fill_radio_face(Painter& painter, double left, double top, Color color) {
    static constexpr int face_left[] = {5, 4, 3, 2, 1, 1, 1, 1, 1, 2, 3, 4, 5};
    static constexpr int face_width[] = {5, 7, 9, 11, 13, 13, 13, 13, 13, 11, 9, 7, 5};
    for (int row = 0; row < 13; ++row) {
        painter.fill_rect({left + face_left[row], top + 1.0 + row,
                           static_cast<double>(face_width[row]), 1.0}, color);
    }
}

void paint_focus(Painter& painter, Rect bounds, Color color) {
    if (bounds.width > 9.0 && bounds.height > 9.0) {
        const double left = bounds.x + 4.5;
        const double top = bounds.y + 4.5;
        const double right = bounds.x + bounds.width - 4.5;
        const double bottom = bounds.y + bounds.height - 4.5;
        for (double x = left; x < right; x += 2.0) {
            painter.draw_line({x, top}, {std::min(x + 1.0, right), top}, color, 1.0);
            painter.draw_line({x, bottom}, {std::min(x + 1.0, right), bottom}, color, 1.0);
        }
        for (double y = top + 1.0; y < bottom; y += 2.0) {
            painter.draw_line({left, y}, {left, std::min(y + 1.0, bottom)}, color, 1.0);
            painter.draw_line({right, y}, {right, std::min(y + 1.0, bottom)}, color, 1.0);
        }
    }
}

void paint_theme_cues(Painter& painter, Rect bounds,
                      const ControlVisualRecipe& recipe,
                      ControlVisualContext context) {
    if (context.defaulted && recipe.default_width > 0.0 &&
        bounds.width > recipe.default_width * 2.0 &&
        bounds.height > recipe.default_width * 2.0) {
        const double inset = recipe.default_width * 0.5;
        painter.stroke_rounded_rect(
            {bounds.x + inset, bounds.y + inset,
             bounds.width - inset * 2.0, bounds.height - inset * 2.0},
            std::max(0.0, recipe.material.corner_radius - inset),
            recipe.default_ring, recipe.default_width);
    }
    if (context.focused && recipe.focus_width > 0.0 &&
        bounds.width > 8.0 && bounds.height > 8.0) {
        const double inset = std::max(3.0, recipe.default_width + 1.0);
        painter.stroke_rounded_rect(
            {bounds.x + inset, bounds.y + inset,
             bounds.width - inset * 2.0, bounds.height - inset * 2.0},
            std::max(0.0, recipe.material.corner_radius - inset),
            recipe.focus_ring, recipe.focus_width);
    }
}

} // namespace

Panel::Panel(StableId stable_id) : ScrollableControl(std::move(stable_id)) {
    set_paint_plane(PaintPlane::backplane);
}

void Panel::set_border_style(BorderStyle style) {
    require_mutable();
    if (border_style_ == style) {
        return;
    }
    border_style_ = style;
    invalidate(Dirty::paint | Dirty::semantics);
}

void Panel::set_background(Color color) {
    require_mutable();
    if (background_override_ && *background_override_ == color) return;
    background_override_ = color;
    invalidate(Dirty::paint);
}

Color Panel::background() const noexcept {
    if (background_override_) return *background_override_;
    const ControlVisualRecipe& recipe = effective_theme().resolve(
        visual_role_, visual_context());
    const MaterialFillLayer& fill = recipe.material.fills.front();
    return fill.kind == MaterialFillKind::solid ? fill.color
                                                : fill.stops.front().color;
}

void Panel::clear_background() {
    require_mutable();
    if (!background_override_) return;
    background_override_.reset();
    invalidate(Dirty::style | Dirty::paint);
}

void Panel::set_style(BasicControlStyle style) {
    require_mutable();
    if (style_override_ && *style_override_ == style) return;
    style_override_ = std::move(style);
    invalidate(Dirty::paint | Dirty::semantics);
}

const BasicControlStyle& Panel::style() const noexcept {
    return style_override_ ? *style_override_ : effective_theme().basic_style();
}

void Panel::clear_style() {
    require_mutable();
    if (!style_override_) return;
    style_override_.reset();
    invalidate(Dirty::style | Dirty::paint | Dirty::semantics);
}

void Panel::set_visual_role(ControlVisualRole role) {
    require_mutable();
    if (role != ControlVisualRole::window && role != ControlVisualRole::panel &&
        role != ControlVisualRole::card) {
        throw std::invalid_argument(
            "panel visual role must be window, panel, or card");
    }
    if (visual_role_ == role) return;
    visual_role_ = role;
    invalidate(Dirty::style | Dirty::paint | Dirty::semantics);
}

Rect Panel::local_bounds() const noexcept {
    const Rect arranged = committed_arranged_bounds();
    return {0.0, 0.0, arranged.width, arranged.height};
}

void Panel::paint_panel(Painter& painter, Rect bounds) const {
    const BasicControlStyle& colors = style();
    if (!background_override_ && !style_override_) {
        paint_surface_material(
            painter, bounds,
            effective_theme().resolve(visual_role_, visual_context()).material);
        if (border_style_ == BorderStyle::none) return;
    } else {
        painter.fill_rect(bounds, background());
    }
    switch (border_style_) {
    case BorderStyle::none:
        break;
    case BorderStyle::line:
        painter.stroke_rect({0.5, 0.5, std::max(0.0, bounds.width - 1.0),
                             std::max(0.0, bounds.height - 1.0)},
                            colors.border, 1.0);
        break;
    case BorderStyle::sunken:
        painter.draw_line({0.0, 0.0}, {bounds.width, 0.0}, colors.dark_border, 1.0);
        painter.draw_line({0.0, 0.0}, {0.0, bounds.height}, colors.dark_border, 1.0);
        painter.draw_line({0.0, bounds.height - 1.0},
                          {bounds.width, bounds.height - 1.0}, colors.highlight, 1.0);
        painter.draw_line({bounds.width - 1.0, 0.0},
                          {bounds.width - 1.0, bounds.height}, colors.highlight, 1.0);
        break;
    case BorderStyle::raised:
        painter.draw_line({0.0, 0.0}, {bounds.width, 0.0}, colors.highlight, 1.0);
        painter.draw_line({0.0, 0.0}, {0.0, bounds.height}, colors.highlight, 1.0);
        painter.draw_line({0.0, bounds.height - 1.0},
                          {bounds.width, bounds.height - 1.0}, colors.dark_border, 1.0);
        painter.draw_line({bounds.width - 1.0, 0.0},
                          {bounds.width - 1.0, bounds.height}, colors.dark_border, 1.0);
        break;
    }
}

Insets Panel::visual_outsets() const noexcept {
    if (background_override_ || style_override_) return {};
    return surface_material_visual_outsets(
        effective_theme().resolve(visual_role_, visual_context()).material);
}

void Panel::on_paint(Painter& painter, Rect) {
    paint_panel(painter, local_bounds());
}

SemanticDescriptor Panel::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::group;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    descriptor.exposed = !descriptor.name.empty() ||
                         !descriptor.description.empty();
    return descriptor;
}

GroupBox::GroupBox(StableId stable_id, std::string text)
    : Panel(std::move(stable_id)), text_(std::move(text)) {
    set_border_style(BorderStyle::none);
}

void GroupBox::set_text(std::string text) {
    require_mutable();
    if (text_ == text) {
        return;
    }
    text_ = std::move(text);
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void GroupBox::set_font(FontSpec font) {
    require_mutable();
    if (!valid_font_spec(font)) {
        throw std::invalid_argument("GroupBox font specification is invalid");
    }
    if (font_ == font) {
        return;
    }
    font_ = font;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void GroupBox::set_use_mnemonic(bool value) {
    require_mutable();
    if (use_mnemonic_ == value) return;
    use_mnemonic_ = value;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void GroupBox::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    const FontSpec font = effective_font(font_);
    const std::string caption = use_mnemonic_
        ? parse_mnemonic_text(text_).display_text : text_;
    const double caption_height = std::max(16.0, font.size + 4.0);
    const double rule_y = std::max(10.5, caption_height * 0.66);
    painter.fill_rect(bounds, background());
    painter.stroke_rect({0.5, rule_y, std::max(0.0, bounds.width - 1.0),
                         std::max(0.0, bounds.height - rule_y - 0.5)},
                        style().border, 1.0);
    const double caption_width = std::min(
        std::max(0.0, bounds.width - 18.0), estimated_text_width(caption, font) + 12.0);
    painter.fill_rect({9.0, 1.0, caption_width, caption_height}, background());
    painter.draw_text_utf8({13.0, std::max(font.size, rule_y + font.size * 0.36)},
                           caption, font,
                           enabled() ? style().text : style().disabled_text);
}

bool GroupBox::mnemonic_matches(char32_t character) const noexcept {
    return use_mnemonic_ && is_mnemonic(character, text_);
}

bool GroupBox::process_mnemonic_self(char32_t character) {
    if (!mnemonic_matches(character)) return false;
    static_cast<void>(focus_next_after_self());
    return true;
}

SemanticDescriptor GroupBox::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::group;
    descriptor.name = accessible_name().empty()
        ? (use_mnemonic_ ? parse_mnemonic_text(text_).display_text : text_)
        : accessible_name();
    descriptor.description = accessible_description();
    descriptor.exposed = true;
    return descriptor;
}

PictureBox::PictureBox(StableId stable_id) : Panel(std::move(stable_id)) {
    set_background(Color::rgba(255, 255, 255));
    PropertyDescriptor image;
    image.name = "Image";
    image.kind = BindingValueKind::image;
    image.category = "Appearance";
    image.description = "Generational image resource displayed by this control.";
    image.default_value = BindingValue{ImageId{}};
    image.invalidation_effects = Dirty::measure | Dirty::paint | Dirty::semantics;
    define_bindable_property({
        std::move(image),
        [this] { return BindingValue{image_}; },
        [this](const BindingValue& value) {
            set_image(std::get<ImageId>(value));
        },
        [this](Component& owner, std::function<void()> changed) {
            return image_changed_.subscribe(owner,
                [changed = std::move(changed)](ImageId) { changed(); });
        },
        [this] { clear_image(); },
        [this] { return image_.value != 0U; }});

    PropertyDescriptor size_mode;
    size_mode.name = "SizeMode";
    size_mode.kind = BindingValueKind::enumeration;
    size_mode.category = "Appearance";
    size_mode.description = "Image placement and scaling policy.";
    size_mode.default_value = BindingValue{PropertyEnumValue{
        "System.Windows.Forms.PictureBoxSizeMode", "Normal", 0}};
    size_mode.invalidation_effects =
        Dirty::measure | Dirty::paint | Dirty::semantics;
    size_mode.bindable = false;
    size_mode.enumeration = picture_box_size_mode_enum();
    define_bindable_property({
        std::move(size_mode),
        [this] { return picture_box_size_mode_value(size_mode_); },
        [this](const BindingValue& value) {
            set_size_mode(static_cast<PictureBoxSizeMode>(
                std::get<PropertyEnumValue>(value).value));
        }, {}, {}, {}});

    PropertyDescriptor opacity;
    opacity.name = "ImageOpacity";
    opacity.kind = BindingValueKind::number;
    opacity.category = "Appearance";
    opacity.description = "Image opacity from zero through one.";
    opacity.default_value = BindingValue{1.0};
    opacity.invalidation_effects = Dirty::paint | Dirty::semantics;
    opacity.bindable = false;
    define_bindable_property({
        std::move(opacity),
        [this] { return BindingValue{image_opacity_}; },
        [this](const BindingValue& value) {
            set_image_opacity(std::get<double>(value));
        }, {}, {}, {}});
}

void PictureBox::set_image(ImageId image) {
    require_mutable();
    if (image_ == image) {
        return;
    }
    image_ = image;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
    publish_change(image_changed_, image_);
}

void PictureBox::clear_image() {
    set_image({});
}

bool PictureBox::has_valid_image() const noexcept {
    return window() != nullptr && image_.value != 0U &&
           window()->image_resources().find(image_).has_value();
}

Size PictureBox::image_size() const noexcept {
    if (window() == nullptr || image_.value == 0U) {
        return {};
    }
    const auto resource = window()->image_resources().find(image_);
    if (!resource) {
        return {};
    }
    return {static_cast<double>(resource->metadata.width),
            static_cast<double>(resource->metadata.height)};
}

void PictureBox::set_size_mode(PictureBoxSizeMode mode) {
    require_mutable();
    if (size_mode_ == mode) {
        return;
    }
    size_mode_ = mode;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void PictureBox::set_image_opacity(double opacity) {
    require_mutable();
    if (!std::isfinite(opacity) || opacity < 0.0 || opacity > 1.0) {
        throw std::invalid_argument(
            "PictureBox image opacity must be finite and between zero and one");
    }
    if (image_opacity_ == opacity) {
        return;
    }
    image_opacity_ = opacity;
    invalidate(Dirty::paint | Dirty::semantics);
}

Rect PictureBox::content_bounds() const noexcept {
    const Rect bounds = local_bounds();
    const double inset = border_style() == BorderStyle::none ? 0.0 : 1.0;
    return {inset, inset, std::max(0.0, bounds.width - inset * 2.0),
            std::max(0.0, bounds.height - inset * 2.0)};
}

Rect PictureBox::image_bounds() const noexcept {
    const Rect content = content_bounds();
    const Size source = image_size();
    if (source.width <= 0.0 || source.height <= 0.0 || content.empty()) {
        return {};
    }
    switch (size_mode_) {
    case PictureBoxSizeMode::stretch_image:
        return content;
    case PictureBoxSizeMode::center_image:
        return {content.x + (content.width - source.width) * 0.5,
                content.y + (content.height - source.height) * 0.5,
                source.width, source.height};
    case PictureBoxSizeMode::zoom: {
        const double scale = std::min(content.width / source.width,
                                      content.height / source.height);
        const double width = source.width * scale;
        const double height = source.height * scale;
        return {content.x + (content.width - width) * 0.5,
                content.y + (content.height - height) * 0.5,
                width, height};
    }
    case PictureBoxSizeMode::normal:
    case PictureBoxSizeMode::auto_size:
        return {content.x, content.y, source.width, source.height};
    }
    return {};
}

Size PictureBox::measure(Size available) {
    const Rect requested = requested_bounds();
    if (size_mode_ == PictureBoxSizeMode::auto_size) {
        const Size source = image_size();
        const double border = border_style() == BorderStyle::none ? 0.0 : 2.0;
        if (source.width > 0.0 && source.height > 0.0) {
            return {std::min(available.width, source.width + border),
                    std::min(available.height, source.height + border)};
        }
    }
    return {std::min(available.width, std::max(0.0, requested.width)),
            std::min(available.height, std::max(0.0, requested.height))};
}

void PictureBox::on_paint(Painter& painter, Rect) {
    paint_panel(painter, local_bounds());
    if (!has_valid_image() || image_opacity_ <= 0.0) {
        return;
    }
    const Rect destination = image_bounds();
    if (!destination.empty()) {
        painter.draw_image(image_, destination, image_opacity_);
    }
}

bool PictureBox::hit_test_local(Point) const {
    return false;
}

SemanticDescriptor PictureBox::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::image;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    const Size source = image_size();
    if (source.width > 0.0 && source.height > 0.0) {
        descriptor.value = std::to_string(static_cast<std::uint32_t>(source.width)) +
                           " x " +
                           std::to_string(static_cast<std::uint32_t>(source.height));
    }
    descriptor.exposed = has_valid_image() || !descriptor.name.empty() ||
                         !descriptor.description.empty();
    return descriptor;
}

Label::Label(StableId stable_id, std::string text)
    : Control(std::move(stable_id)), text_(std::move(text)) {
    define_bindable_property({
        {"Text", BindingValueKind::text, "Appearance",
         "Text displayed by the label.", BindingValue{std::string{}},
         invalidation::text_content},
        [this] { return BindingValue{text_}; },
        [this](const BindingValue& value) {
            const auto converted = convert_binding_value(value, BindingValueKind::text);
            if (!converted) throw std::invalid_argument("Label.Text binding requires text");
            set_text(std::get<std::string>(*converted));
        },
        [this](Component& owner, std::function<void()> changed) {
            return text_changed_.subscribe(owner,
                [changed = std::move(changed)](const std::string&) { changed(); });
        }, {}, {}});

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
        [this] { return BindingValue{this->font()}; },
        [this](const BindingValue& value) {
            set_font(std::get<FontSpec>(value));
        }, {},
        [this] { clear_font(); },
        [this] { return has_font_override(); },
        [this] {
            return has_font_override() ? PropertyValueOrigin::local
                                       : PropertyValueOrigin::inherited;
        }});

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
        [this] { return BindingValue{this->foreground()}; },
        [this](const BindingValue& value) {
            set_foreground(std::get<Color>(value));
        }, {},
        [this] { clear_foreground(); },
        [this] { return has_foreground_override(); },
        [this] {
            return has_foreground_override() ? PropertyValueOrigin::local
                                             : PropertyValueOrigin::inherited;
        }});
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
    if (alignment_ == alignment) {
        return;
    }
    alignment_ = alignment;
    invalidate(Dirty::paint | Dirty::semantics);
}

void Label::set_vertical_alignment(VerticalAlignment alignment) {
    require_mutable();
    if (vertical_alignment_ == alignment) {
        return;
    }
    vertical_alignment_ = alignment;
    invalidate(Dirty::paint | Dirty::semantics);
}

void Label::set_text_wrapping(TextWrapping wrapping) {
    require_mutable();
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

void Label::set_use_mnemonic(bool value) {
    require_mutable();
    if (use_mnemonic_ == value) return;
    use_mnemonic_ = value;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

Size Label::measure(Size available) {
    const Rect requested = requested_bounds();
    const std::string text = display_text();
    const FontSpec font = effective_font(this->font());
    const double wrap_width = requested.width > 0.0
        ? requested.width : available.width;
    const auto lines = label_lines(text, font, std::max(0.0, wrap_width - 4.0),
                                   text_wrapping_);
    double content_width{};
    for (const std::string& line : lines) {
        content_width = std::max(content_width, estimated_text_width(line, font));
    }
    const double preferred_width = requested.width > 0.0
        ? requested.width : content_width + 4.0;
    const double preferred_height = requested.height > 0.0
        ? requested.height
        : static_cast<double>(lines.size()) * font.size * line_spacing_ + 4.0;
    return {std::min(available.width, preferred_width),
            std::min(available.height, preferred_height)};
}

std::string Label::display_text() const {
    return use_mnemonic_ ? parse_mnemonic_text(text_).display_text : text_;
}

void Label::paint_label_text(Painter& painter, std::string_view text) const {
    const Rect arranged = committed_arranged_bounds();
    const FontSpec font = effective_font(this->font());
    const auto lines = label_lines(text, font, std::max(0.0, arranged.width - 4.0),
                                   text_wrapping_);
    const double line_height = font.size * line_spacing_;
    const double block_height = static_cast<double>(lines.size()) * line_height;
    double top = 1.0;
    if (vertical_alignment_ == VerticalAlignment::center) {
        top = std::max(1.0, (arranged.height - block_height) * 0.5);
    } else if (vertical_alignment_ == VerticalAlignment::far) {
        top = std::max(1.0, arranged.height - block_height - 1.0);
    }
    const Color color = foreground();
    for (std::size_t index = 0; index < lines.size(); ++index) {
        const double text_width = estimated_text_width(lines[index], font);
        double x = 2.0;
        if (alignment_ == HorizontalAlignment::center) {
            x = std::max(2.0, (arranged.width - text_width) * 0.5);
        } else if (alignment_ == HorizontalAlignment::far) {
            x = std::max(2.0, arranged.width - text_width - 2.0);
        }
        const double baseline = top + font.size +
            static_cast<double>(index) * line_height;
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
    descriptor.name = accessible_name().empty() ? display_text() : accessible_name();
    descriptor.description = accessible_description();
    descriptor.exposed = !descriptor.name.empty();
    return descriptor;
}

ButtonBase::ButtonBase(StableId stable_id, std::string text)
    : Control(std::move(stable_id)), text_(std::move(text)) {
    set_focusable(true);
    set_cursor(CursorKind::hand);
    define_bindable_property({
        {"Text", BindingValueKind::text, "Appearance",
         "Text displayed by the button.", BindingValue{std::string{}},
         invalidation::text_content},
        [this] { return BindingValue{text_}; },
        [this](const BindingValue& value) {
            const auto converted = convert_binding_value(value, BindingValueKind::text);
            if (!converted) throw std::invalid_argument("ButtonBase.Text binding requires text");
            set_text(std::get<std::string>(*converted));
        },
        [this](Component& owner, std::function<void()> changed) {
            return text_changed_.subscribe(owner,
                [changed = std::move(changed)](const std::string&) { changed(); });
        }, {}, {}});

    PropertyDescriptor font;
    font.name = "Font";
    font.kind = BindingValueKind::font;
    font.category = "Appearance";
    font.description = "Font used for button measurement and painting.";
    font.default_value = BindingValue{
        FontSpec{FontRole::control, 12.0, 400, false, 0.24}};
    font.invalidation_effects = Dirty::measure | Dirty::paint | Dirty::semantics;
    font.bindable = false;
    define_bindable_property({
        std::move(font),
        [this] { return BindingValue{font_}; },
        [this](const BindingValue& value) {
            set_font(std::get<FontSpec>(value));
        }, {}, {}, {}});

    PropertyDescriptor image;
    image.name = "Image";
    image.kind = BindingValueKind::image;
    image.category = "Appearance";
    image.description = "Direct generational image resource for button content.";
    image.default_value = BindingValue{ImageId{}};
    image.invalidation_effects = Dirty::measure | Dirty::paint | Dirty::semantics;
    image.bindable = false;
    define_bindable_property({
        std::move(image),
        [this] { return BindingValue{image_}; },
        [this](const BindingValue& value) {
            set_image(std::get<ImageId>(value));
        }, {},
        [this] { clear_image(); },
        [this] {
            return image_.value != 0U || image_index_ >= 0 || !image_key_.empty();
        }});
}

void ButtonBase::set_text(std::string text) {
    require_mutable();
    if (text_ == text) {
        return;
    }
    text_ = std::move(text);
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
    publish_change(text_changed_, text_);
}

void ButtonBase::set_font(FontSpec font) {
    require_mutable();
    if (!valid_font_spec(font)) {
        throw std::invalid_argument("Button font specification is invalid");
    }
    if (font_ == font) {
        return;
    }
    font_ = font;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void ButtonBase::set_style(BasicControlStyle style) {
    require_mutable();
    if (style_override_ && *style_override_ == style) {
        return;
    }
    style_override_ = std::move(style);
    invalidate(Dirty::paint | Dirty::semantics);
}

const BasicControlStyle& ButtonBase::style() const noexcept {
    return style_override_ ? *style_override_ : effective_theme().basic_style();
}

void ButtonBase::clear_style() {
    require_mutable();
    if (!style_override_) return;
    style_override_.reset();
    invalidate(Dirty::style | Dirty::paint | Dirty::semantics);
}

void ButtonBase::set_image(ImageId image) {
    require_mutable();
    if (image.value != 0U && window() != nullptr &&
        !window()->image_resources().find(image)) {
        throw std::invalid_argument("Button image ID is not live in its Window");
    }
    if (image_ == image && image_index_ == -1 && image_key_.empty()) return;
    image_ = image;
    image_index_ = -1;
    image_key_.clear();
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void ButtonBase::clear_image() {
    set_image({});
}

void ButtonBase::set_image_list(std::shared_ptr<ImageList> image_list) {
    require_mutable();
    if (image_list && !image_list->is_alive()) {
        throw std::invalid_argument("Button requires a live ImageList");
    }
    if (image_list && window() != nullptr &&
        !image_list->belongs_to(*window())) {
        throw std::invalid_argument(
            "Button and ImageList must belong to the same Window");
    }
    if (image_list && image_index_ >= 0 &&
        static_cast<std::size_t>(image_index_) >= image_list->count()) {
        throw std::out_of_range(
            "Button image index is outside the assigned ImageList");
    }
    if (image_list_ == image_list) return;
    image_list_changed_.disconnect();
    image_list_ = std::move(image_list);
    if (image_list_) {
        image_list_changed_ = image_list_->changed().subscribe(
            *this, [this](const ImageListChange&) {
                if (is_alive()) {
                    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
                }
            });
    }
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void ButtonBase::set_image_index(int image_index) {
    require_mutable();
    if (image_index < -1) {
        throw std::out_of_range("Button image index must be -1 or nonnegative");
    }
    if (image_index >= 0 && image_list_ &&
        static_cast<std::size_t>(image_index) >= image_list_->count()) {
        throw std::out_of_range("Button image index is outside its ImageList");
    }
    if (image_index_ == image_index && image_.value == 0U && image_key_.empty()) {
        return;
    }
    image_ = {};
    image_key_.clear();
    image_index_ = image_index;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void ButtonBase::set_image_key(std::string image_key) {
    require_mutable();
    if (image_key.size() > 256U) {
        throw std::invalid_argument("Button image key exceeds 256 bytes");
    }
    if (!image_key.empty() && !validate_utf8(image_key).valid()) {
        throw std::invalid_argument("Button image key must be valid UTF-8");
    }
    if (image_key_ == image_key && image_.value == 0U && image_index_ == -1) {
        return;
    }
    image_ = {};
    image_index_ = -1;
    image_key_ = std::move(image_key);
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void ButtonBase::set_image_alignment(ContentAlignment alignment) {
    require_mutable();
    if (!valid_content_alignment(alignment)) {
        throw std::invalid_argument("invalid Button image alignment");
    }
    if (image_alignment_ == alignment) return;
    image_alignment_ = alignment;
    invalidate(Dirty::paint | Dirty::semantics);
}

void ButtonBase::set_text_alignment(ContentAlignment alignment) {
    require_mutable();
    if (!valid_content_alignment(alignment)) {
        throw std::invalid_argument("invalid Button text alignment");
    }
    if (text_alignment_ == alignment) return;
    text_alignment_ = alignment;
    invalidate(Dirty::paint | Dirty::semantics);
}

void ButtonBase::set_text_image_relation(TextImageRelation relation) {
    require_mutable();
    if (!valid_text_image_relation(relation)) {
        throw std::invalid_argument("invalid Button text/image relation");
    }
    if (text_image_relation_ == relation) return;
    text_image_relation_ = relation;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void ButtonBase::set_image_gap(double gap) {
    require_mutable();
    if (!std::isfinite(gap) || gap < 0.0 || gap > 64.0) {
        throw std::invalid_argument(
            "Button image gap must be finite and between zero and 64");
    }
    if (image_gap_ == gap) return;
    image_gap_ = gap;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void ButtonBase::set_use_mnemonic(bool value) {
    require_mutable();
    if (use_mnemonic_ == value) return;
    use_mnemonic_ = value;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

std::string ButtonBase::display_text() const {
    return use_mnemonic_ ? parse_mnemonic_text(text_).display_text : text_;
}

bool ButtonBase::perform_click() {
    if (!prepare_command_activation()) return false;
    on_activate();
    return true;
}

bool ButtonBase::perform_dialog_command() { return perform_click(); }

void ButtonBase::on_attached_to_window() {
    Control::on_attached_to_window();
    if (image_list_ && !image_list_->belongs_to(*window())) {
        throw std::logic_error("Button cannot attach to a different ImageList Window");
    }
    if (image_.value != 0U && !window()->image_resources().find(image_)) {
        throw std::logic_error("Button cannot attach with a foreign or stale image ID");
    }
}

void ButtonBase::set_expanded_state(std::optional<bool> expanded) {
    require_mutable();
    if (expanded_state_ == expanded) return;
    expanded_state_ = expanded;
    invalidate(Dirty::paint | Dirty::semantics);
}

Size ButtonBase::measure(Size available) {
    const Rect requested = requested_bounds();
    const FontSpec font = effective_font(font_);
    const std::string display = display_text();
    const double text_width = display.empty() ? 0.0
                                               : estimated_text_width(display, font);
    const double text_height = display.empty() ? 0.0 : font.size * 1.25;
    Size image_size{};
    if (image_.value != 0U) {
        if (window() != nullptr) {
            if (const auto resource = window()->image_resources().find(image_)) {
                image_size = {static_cast<double>(resource->metadata.width),
                              static_cast<double>(resource->metadata.height)};
            }
        }
    } else if (image_list_ && image_list_->is_alive() &&
               (!image_key_.empty() || image_index_ >= 0)) {
        image_size = image_list_->image_size();
    }
    const bool has_text = text_width > 0.0;
    const bool has_image = image_size.width > 0.0 && image_size.height > 0.0;
    const double gap = has_text && has_image ? image_gap_ : 0.0;
    double content_width = text_width;
    double content_height = text_height;
    if (has_image) {
        switch (text_image_relation_) {
        case TextImageRelation::image_before_text:
        case TextImageRelation::text_before_image:
            content_width = image_size.width + gap + text_width;
            content_height = std::max(image_size.height, text_height);
            break;
        case TextImageRelation::image_above_text:
        case TextImageRelation::text_above_image:
            content_width = std::max(image_size.width, text_width);
            content_height = image_size.height + gap + text_height;
            break;
        case TextImageRelation::overlay:
            content_width = std::max(image_size.width, text_width);
            content_height = std::max(image_size.height, text_height);
            break;
        }
    }
    const double preferred_width = requested.width > 0.0
        ? requested.width : content_width + 22.0;
    const double preferred_height = requested.height > 0.0
        ? requested.height : std::max(24.0, content_height + 12.0);
    return {std::min(available.width, preferred_width),
            std::min(available.height, preferred_height)};
}

Rect ButtonBase::local_bounds() const noexcept {
    const Rect arranged = committed_arranged_bounds();
    return {0.0, 0.0, arranged.width, arranged.height};
}

void ButtonBase::paint_button_frame(Painter& painter, Rect bounds,
                                    bool default_cue) const {
    const BasicControlStyle& colors = style();
    paint_relief(painter, bounds, colors, pressed_visual());
    if (default_cue && bounds.width > 2.0 && bounds.height > 2.0) {
        painter.stroke_rect({0.5, 0.5, bounds.width - 1.0, bounds.height - 1.0},
                            colors.accent, 1.0);
    }
    if (focused_) {
        paint_focus(painter, bounds, colors.text);
    }
}

void ButtonBase::paint_button_text(Painter& painter, Rect bounds,
                                   std::string_view text) const {
    const BasicControlStyle& colors = style();
    const double offset = pressed_visual() ? 1.0 : 0.0;
    paint_button_content(painter, bounds, text,
                         effectively_enabled() ? colors.text
                                               : colors.disabled_text,
                         {offset, offset});
}

ImageListResolution ButtonBase::resolved_button_image(bool selected) const noexcept {
    ImageListResolution result;
    const ImageVisualState state = !effectively_enabled()
        ? ImageVisualState::disabled
        : pressed_visual() ? ImageVisualState::pressed
        : hovered_visual() ? ImageVisualState::hot
        : selected ? ImageVisualState::selected
                   : ImageVisualState::normal;
    result.requested_state = state;
    result.requested_scale = window() ? window()->scale() : 1.0;
    if (image_.value != 0U && window() != nullptr) {
        const auto resource = window()->image_resources().find(image_);
        if (!resource) return result;
        result.image = image_;
        result.source_size = {static_cast<double>(resource->metadata.width),
                              static_cast<double>(resource->metadata.height)};
        result.resolved_state = ImageVisualState::normal;
        result.resolved_scale = 1.0;
        return result;
    }
    if (!image_list_ || !image_list_->is_alive() || window() == nullptr ||
        !image_list_->belongs_to(*window())) return result;
    if (!image_key_.empty()) {
        return image_list_->resolve(image_key_, state, window()->scale());
    }
    if (image_index_ >= 0) {
        return image_list_->resolve(static_cast<std::size_t>(image_index_), state,
                                    window()->scale());
    }
    return result;
}

void ButtonBase::paint_button_content(Painter& painter, Rect bounds,
                                      std::string_view text, Color foreground,
                                      Point offset, bool selected,
                                      bool command_alignment) const {
    Rect content{bounds.x + (command_alignment ? 12.0 : 6.0), bounds.y + 4.0,
                 std::max(0.0, bounds.width -
                     (command_alignment ? 18.0 : 12.0)),
                 std::max(0.0, bounds.height - 8.0)};
    if (content.empty()) return;
    const FontSpec font = effective_font(font_);
    const Size measured = text.empty() ? Size{}
                                       : painter.measure_text_utf8(text, font);
    const Size text_size{text.empty() ? 0.0 : std::max(0.0, measured.width),
                         text.empty() ? 0.0
                                      : std::max(font.size * 1.2,
                                                 measured.height)};
    const ImageListResolution image = resolved_button_image(selected);
    Size image_size{};
    if (image) {
        image_size = image_list_ && image_.value == 0U
            ? image_list_->image_size() : image.source_size;
    }
    const bool has_text = text_size.width > 0.0 && text_size.height > 0.0;
    const bool has_image = image && image_size.width > 0.0 &&
                           image_size.height > 0.0;
    Rect text_rect{};
    Rect image_rect{};
    const ContentAlignment group_alignment = command_alignment
        ? ContentAlignment::middle_left : text_alignment_;
    const double gap = has_text && has_image ? image_gap_ : 0.0;

    if (has_text && has_image && text_image_relation_ != TextImageRelation::overlay) {
        const bool horizontal =
            text_image_relation_ == TextImageRelation::image_before_text ||
            text_image_relation_ == TextImageRelation::text_before_image;
        const Size group_size = horizontal
            ? Size{image_size.width + gap + text_size.width,
                   std::max(image_size.height, text_size.height)}
            : Size{std::max(image_size.width, text_size.width),
                   image_size.height + gap + text_size.height};
        const Rect group = aligned_rect(content, group_size, group_alignment);
        if (horizontal) {
            const bool image_first =
                text_image_relation_ == TextImageRelation::image_before_text;
            const double image_x = image_first ? group.x
                                               : group.x + text_size.width + gap;
            const double text_x = image_first ? group.x + image_size.width + gap
                                              : group.x;
            image_rect = {image_x,
                          group.y + (group.height - image_size.height) * 0.5,
                          image_size.width, image_size.height};
            text_rect = {text_x,
                         group.y + (group.height - text_size.height) * 0.5,
                         text_size.width, text_size.height};
        } else {
            const bool image_first =
                text_image_relation_ == TextImageRelation::image_above_text;
            const double image_y = image_first ? group.y
                                               : group.y + text_size.height + gap;
            const double text_y = image_first ? group.y + image_size.height + gap
                                              : group.y;
            image_rect = {group.x + (group.width - image_size.width) * 0.5,
                          image_y, image_size.width, image_size.height};
            text_rect = {group.x + (group.width - text_size.width) * 0.5,
                         text_y, text_size.width, text_size.height};
        }
    } else {
        if (has_image) image_rect = aligned_rect(content, image_size, image_alignment_);
        if (has_text) text_rect = aligned_rect(content, text_size, group_alignment);
    }

    if (has_image && !image_rect.empty()) {
        const double opacity = !effectively_enabled() &&
                image.resolved_state != ImageVisualState::disabled
            ? 0.45 : 1.0;
        painter.draw_image(image.image,
                           {image_rect.x + offset.x, image_rect.y + offset.y,
                            image_rect.width, image_rect.height},
                           opacity);
    }
    if (has_text && !text_rect.empty()) {
        painter.draw_text_utf8(
            {text_rect.x + offset.x,
             text_rect.y + offset.y + std::max(font.size, text_rect.height * 0.82)},
            text, font, foreground);
    }
}

void ButtonBase::paint_themed_button(Painter& painter, Rect bounds,
                                     ControlVisualRole role,
                                     bool default_cue,
                                     bool command_alignment) const {
    const ControlVisualContext context = visual_context(
        hovered_, pressed_visual(), false, focused_, default_cue);
    const ControlVisualRecipe& recipe = effective_theme().resolve(role, context);
    paint_surface_material(painter, bounds, recipe.material);
    paint_theme_cues(painter, bounds, recipe, context);
    const Point offset = context.surface == ControlSurfaceState::pressed
        ? recipe.pressed_content_offset : Point{};
    const std::string display = display_text();
    paint_button_content(painter, bounds, display, recipe.text, offset, false,
                         command_alignment);
}

void ButtonBase::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    const std::string display = display_text();
    if (has_style_override()) {
        paint_button_frame(painter, bounds, false);
        paint_button_text(painter, bounds, display);
    } else {
        paint_themed_button(painter, bounds, ControlVisualRole::button, false);
    }
}

Insets ButtonBase::visual_outsets() const noexcept {
    if (has_style_override()) return {};
    const ControlVisualContext context = visual_context(
        hovered_, pressed_visual(), false, focused_, false);
    return surface_material_visual_outsets(
        effective_theme().resolve(ControlVisualRole::button, context).material);
}

void ButtonBase::on_pointer(PointerEvent& event) {
    if (event.action == PointerAction::enter || event.action == PointerAction::leave) {
        const bool next = event.action == PointerAction::enter;
        if (hovered_ != next) {
            hovered_ = next;
            invalidate(Dirty::style | Dirty::paint);
        }
    } else if (event.button == PointerButton::primary &&
               event.action == PointerAction::down) {
        pointer_engaged_ = true;
        pointer_pressed_ = true;
        invalidate(Dirty::paint);
        event.handled = true;
    } else if (event.action == PointerAction::move && pointer_engaged_) {
        const bool inside = absolute_bounds().contains(event.position);
        if (pointer_pressed_ != inside) {
            pointer_pressed_ = inside;
            invalidate(Dirty::paint);
        }
        event.handled = true;
    } else if (event.button == PointerButton::primary &&
               event.action == PointerAction::up && pointer_engaged_) {
        pointer_engaged_ = false;
        pointer_pressed_ = false;
        invalidate(Dirty::paint);
        event.handled = true;
    }
}

void ButtonBase::on_key(KeyEvent& event) {
    const bool activation_key = event.physical_key == PhysicalKey::space ||
                                event.physical_key == PhysicalKey::enter;
    if (!activation_key) {
        return;
    }
    if (event.action == KeyAction::down && !event.repeat && !keyboard_pressed_) {
        keyboard_key_ = event.physical_key;
        keyboard_pressed_ = true;
        invalidate(Dirty::paint);
        event.handled = true;
    } else if (event.action == KeyAction::up && keyboard_pressed_ &&
               keyboard_key_ == event.physical_key) {
        keyboard_key_ = 0;
        keyboard_pressed_ = false;
        invalidate(Dirty::paint);
        event.handled = true;
        on_activate();
    }
}

bool ButtonBase::mnemonic_matches(char32_t character) const noexcept {
    return use_mnemonic_ && is_mnemonic(character, text_);
}

bool ButtonBase::process_mnemonic_self(char32_t character) {
    if (!mnemonic_matches(character)) return false;
    // WinForms treats a recognized mnemonic as owned even when validation
    // prevents the resulting command from firing.
    static_cast<void>(perform_click());
    return true;
}

void ButtonBase::on_focus_changed(bool focused) {
    focused_ = focused;
    if (!focused) {
        keyboard_key_ = 0;
        keyboard_pressed_ = false;
    }
    invalidate(Dirty::paint | Dirty::semantics);
}

void ButtonBase::on_activate() {
    clicked_.emit(*this);
}

SemanticDescriptor ButtonBase::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::button;
    descriptor.name = accessible_name().empty() ? display_text() : accessible_name();
    descriptor.description = accessible_description();
    descriptor.actions = {SemanticAction::focus, SemanticAction::press};
    if (visual_status() == ControlVisualStatus::pending) {
        descriptor.states |= SemanticState::busy;
    } else if (visual_status() == ControlVisualStatus::invalid) {
        descriptor.states |= SemanticState::invalid;
    }
    if (expanded_state_) {
        if (*expanded_state_) descriptor.states |= SemanticState::expanded;
        descriptor.actions.push_back(*expanded_state_ ? SemanticAction::collapse
                                                      : SemanticAction::expand);
    }
    descriptor.exposed = true;
    return descriptor;
}

bool ButtonBase::on_semantic_action(SemanticAction action, std::string_view value) {
    if (action == SemanticAction::press) {
        return perform_click();
    }
    if (expanded_state_ &&
        ((action == SemanticAction::expand && !*expanded_state_) ||
         (action == SemanticAction::collapse && *expanded_state_))) {
        on_activate();
        return true;
    }
    return Control::on_semantic_action(action, value);
}

Button::Button(StableId stable_id, std::string text)
    : ButtonBase(std::move(stable_id), std::move(text)) {}

void Button::set_default_button(bool is_default) {
    require_mutable();
    if (default_button_ == is_default) {
        return;
    }
    default_button_ = is_default;
    invalidate(Dirty::paint | Dirty::semantics);
}

void Button::notify_default(bool value) { set_default_button(value); }

void Button::set_dialog_result(DialogResult result) {
    require_mutable();
    switch (result) {
    case DialogResult::none:
    case DialogResult::ok:
    case DialogResult::cancel:
    case DialogResult::abort:
    case DialogResult::retry:
    case DialogResult::ignore:
    case DialogResult::yes:
    case DialogResult::no:
    case DialogResult::try_again:
    case DialogResult::continue_: break;
    default:
        throw std::invalid_argument("Button DialogResult is not a defined value");
    }
    if (dialog_result_ == result) return;
    dialog_result_ = result;
    invalidate(Dirty::semantics);
    publish_change(dialog_result_changed_, dialog_result_);
}

void Button::assign_cancel_dialog_result() {
    if (dialog_result_ == DialogResult::none) {
        set_dialog_result(DialogResult::cancel);
    }
}

void Button::on_activate() {
    Window* owner = attached_window();
    ButtonBase::on_activate();
    // Read the value after Click: a handler may deliberately replace or clear
    // DialogResult before the Form/Window observes it.
    if (dialog_result_ != DialogResult::none && is_alive() &&
        attached_window() == owner && owner != nullptr) {
        owner->set_dialog_result(dialog_result_);
    }
}

void Button::set_visual_style(ButtonVisualStyle style) {
    require_mutable();
    if (visual_style_ == style) {
        return;
    }
    visual_style_ = style;
    invalidate(Dirty::paint | Dirty::semantics);
}

void Button::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    const std::string display = display_text();
    if (!has_style_override()) {
        ControlVisualRole role = ControlVisualRole::button;
        if (visual_style_ == ButtonVisualStyle::accent) {
            role = ControlVisualRole::accent_button;
        } else if (visual_style_ == ButtonVisualStyle::command) {
            role = ControlVisualRole::command_button;
        }
        paint_themed_button(painter, bounds, role, default_button_,
                            visual_style_ == ButtonVisualStyle::command);
        return;
    }
    switch (visual_style_) {
    case ButtonVisualStyle::standard:
        paint_button_frame(painter, bounds, default_button_);
        paint_button_text(painter, bounds, display);
        break;
    case ButtonVisualStyle::flat:
        painter.fill_rect(bounds, pressed_visual() ? style().accent_light
                                                   : style().face_light);
        painter.stroke_rect({0.5, 0.5, std::max(0.0, bounds.width - 1.0),
                             std::max(0.0, bounds.height - 1.0)},
                            default_button_ ? style().accent : style().border, 1.0);
        paint_button_text(painter, bounds, display);
        break;
    case ButtonVisualStyle::accent: {
        const Color fill = pressed_visual() ? style().link : style().accent;
        painter.fill_rect(bounds, fill);
        painter.draw_line({0.0, 0.0}, {bounds.width - 1.0, 0.0},
                          style().accent_light, 1.0);
        painter.stroke_rect({0.5, 0.5, std::max(0.0, bounds.width - 1.0),
                             std::max(0.0, bounds.height - 1.0)},
                            style().dark_border, 1.0);
        const double offset = pressed_visual() ? 1.0 : 0.0;
        paint_button_content(painter, bounds, display,
                             enabled() ? style().paper : style().disabled_text,
                             {offset, offset});
        break;
    }
    case ButtonVisualStyle::command:
        painter.fill_rect(bounds, pressed_visual() ? style().accent_light
                                                   : style().face);
        painter.fill_rect({0.0, 0.0, 4.0, bounds.height}, style().accent);
        painter.stroke_rect({0.5, 0.5, std::max(0.0, bounds.width - 1.0),
                             std::max(0.0, bounds.height - 1.0)},
                            style().border, 1.0);
        paint_button_content(painter, bounds, display,
                             enabled() ? style().text : style().disabled_text,
                             {}, false, true);
        break;
    }
}

Insets Button::visual_outsets() const noexcept {
    if (has_style_override()) return {};
    ControlVisualRole role = ControlVisualRole::button;
    if (visual_style_ == ButtonVisualStyle::accent) {
        role = ControlVisualRole::accent_button;
    } else if (visual_style_ == ButtonVisualStyle::command) {
        role = ControlVisualRole::command_button;
    }
    const ControlVisualContext context = visual_context(
        hovered_visual(), pressed_visual(), false, focused_visual(),
        default_button_);
    return surface_material_visual_outsets(
        effective_theme().resolve(role, context).material);
}

CheckBox::CheckBox(StableId stable_id, std::string text)
    : ButtonBase(std::move(stable_id), std::move(text)) {
    set_text_alignment(ContentAlignment::middle_left);
    set_image_alignment(ContentAlignment::middle_left);
    define_bindable_property({
        {"Checked", BindingValueKind::boolean, "Behavior",
         "Whether the check box is in a checked state.", BindingValue{false},
         Dirty::paint | Dirty::semantics},
        [this] { return BindingValue{checked()}; },
        [this](const BindingValue& value) {
            const auto converted = convert_binding_value(value, BindingValueKind::boolean);
            if (!converted) throw std::invalid_argument("CheckBox.Checked binding requires Boolean");
            set_checked(std::get<bool>(*converted));
        },
        [this](Component& owner, std::function<void()> changed) {
            return checked_changed_.subscribe(owner,
                [changed = std::move(changed)](bool) { changed(); });
        }, {}, {}});
}

void CheckBox::set_check_state(CheckState state) {
    require_mutable();
    if (state != CheckState::unchecked && state != CheckState::checked &&
        state != CheckState::indeterminate) {
        throw std::invalid_argument("invalid check state");
    }
    if (!three_state_ && state == CheckState::indeterminate) {
        state = CheckState::checked;
    }
    if (check_state_ == state) {
        return;
    }
    const bool previous_checked = checked();
    check_state_ = state;
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(check_state_changed_, check_state_);
    if (!is_alive()) {
        return;
    }
    if (previous_checked != checked()) {
        publish_change(checked_changed_, checked());
    }
}

void CheckBox::set_checked(bool checked_value) {
    set_check_state(checked_value ? CheckState::checked : CheckState::unchecked);
}

void CheckBox::set_three_state(bool enabled_value) {
    require_mutable();
    if (three_state_ == enabled_value) {
        return;
    }
    three_state_ = enabled_value;
    if (!three_state_ && check_state_ == CheckState::indeterminate) {
        set_check_state(CheckState::checked);
        return;
    }
    invalidate(Dirty::semantics);
}

void CheckBox::set_auto_check(bool enabled_value) {
    require_mutable();
    if (auto_check_ == enabled_value) {
        return;
    }
    auto_check_ = enabled_value;
    invalidate(Dirty::semantics);
}

void CheckBox::set_indicator_style(ChoiceIndicatorStyle style_value) {
    require_mutable();
    if (indicator_style_ == style_value) {
        return;
    }
    indicator_style_ = style_value;
    invalidate(Dirty::paint | Dirty::semantics);
}

void CheckBox::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    if (!has_style_override()) {
        const double indicator_width =
            indicator_style_ == ChoiceIndicatorStyle::toggle ? 30.0 : 15.0;
        const Rect box{1.0, std::max(1.0, (bounds.height - 15.0) * 0.5),
                       indicator_width, 15.0};
        const ControlVisualContext context = visual_context(
            hovered_visual(), pressed_visual(), checked(), focused_visual());
        const ControlVisualRecipe& recipe = effective_theme().resolve(
            ControlVisualRole::choice, context);
        SurfaceMaterial indicator = recipe.material;
        indicator.corner_radius = indicator_style_ == ChoiceIndicatorStyle::toggle
            ? 7.5 : 2.0;
        paint_surface_material(painter, box, indicator);
        if (check_state_ == CheckState::checked &&
            indicator_style_ != ChoiceIndicatorStyle::toggle) {
            painter.draw_line({box.x + 3.0, box.y + 7.0},
                              {box.x + 6.0, box.y + 10.0}, recipe.glyph, 2.0);
            painter.draw_line({box.x + 6.0, box.y + 10.0},
                              {box.x + 13.0, box.y + 3.0}, recipe.glyph, 2.0);
        } else if (check_state_ == CheckState::indeterminate) {
            painter.fill_rect({box.x + 4.0, box.y + 6.0, 8.0, 3.0},
                              recipe.glyph);
        } else if (indicator_style_ == ChoiceIndicatorStyle::toggle) {
            const double knob_x = checked() ? box.x + box.width - 13.0
                                             : box.x + 2.0;
            painter.fill_rounded_rect({knob_x, box.y + 2.0, 11.0, 11.0},
                                      5.5, recipe.text);
        }
        paint_button_content(
            painter,
            {box.x + box.width + 1.0, 0.0,
             std::max(0.0, bounds.width - box.x - box.width - 1.0),
             bounds.height},
            display_text(), recipe.text, {}, checked());
        paint_theme_cues(painter, bounds, recipe, context);
        return;
    }
    const BasicControlStyle& colors = style();
    const double indicator_width =
        indicator_style_ == ChoiceIndicatorStyle::toggle ? 30.0 : 15.0;
    const Rect box{1.0, std::max(1.0, (bounds.height - 15.0) * 0.5),
                   indicator_width, 15.0};
    if (indicator_style_ == ChoiceIndicatorStyle::classic) {
        painter.fill_rect(box, colors.paper);
        painter.draw_line({box.x, box.y}, {box.x + box.width, box.y},
                          colors.dark_border, 1.0);
        painter.draw_line({box.x, box.y}, {box.x, box.y + box.height},
                          colors.dark_border, 1.0);
        painter.draw_line({box.x, box.y + box.height - 1.0},
                          {box.x + box.width, box.y + box.height - 1.0},
                          colors.highlight, 1.0);
        painter.draw_line({box.x + box.width - 1.0, box.y},
                          {box.x + box.width - 1.0, box.y + box.height},
                          colors.highlight, 1.0);
    } else if (indicator_style_ == ChoiceIndicatorStyle::modern) {
        painter.fill_rect(box, checked() ? colors.accent : colors.paper);
        painter.stroke_rect({box.x + 0.5, box.y + 0.5, box.width - 1.0,
                             box.height - 1.0},
                            checked() ? colors.accent : colors.border, 1.0);
    } else {
        painter.fill_rect(box, checked() ? colors.accent : colors.border);
        const double knob_x = checked() ? box.x + box.width - 13.0 : box.x + 2.0;
        painter.fill_rect({knob_x, box.y + 2.0, 11.0, 11.0}, colors.paper);
    }
    if (check_state_ == CheckState::checked) {
        if (indicator_style_ != ChoiceIndicatorStyle::toggle) {
            const Color mark = indicator_style_ == ChoiceIndicatorStyle::modern
                ? colors.paper : colors.accent;
            painter.draw_line({4.0, box.y + 7.0}, {7.0, box.y + 10.0}, mark, 2.0);
            painter.draw_line({7.0, box.y + 10.0}, {14.0, box.y + 3.0}, mark, 2.0);
        }
    } else if (check_state_ == CheckState::indeterminate) {
        painter.fill_rect({4.0, box.y + 6.0, 9.0, 4.0}, colors.accent);
    }
    paint_button_content(
        painter,
        {box.x + box.width + 1.0, 0.0,
         std::max(0.0, bounds.width - box.x - box.width - 1.0), bounds.height},
        display_text(), enabled() ? colors.text : colors.disabled_text, {}, checked());
}

Insets CheckBox::visual_outsets() const noexcept {
    if (has_style_override()) return {};
    const ControlVisualContext context = visual_context(
        hovered_visual(), pressed_visual(), checked(), focused_visual());
    return surface_material_visual_outsets(
        effective_theme().resolve(ControlVisualRole::choice, context).material);
}

void CheckBox::on_activate() {
    if (auto_check_) {
        CheckState next = CheckState::unchecked;
        if (check_state_ == CheckState::unchecked) {
            next = CheckState::checked;
        } else if (check_state_ == CheckState::checked && three_state_) {
            next = CheckState::indeterminate;
        }
        set_check_state(next);
        if (!is_alive()) {
            return;
        }
    }
    ButtonBase::on_activate();
}

SemanticDescriptor CheckBox::semantic_descriptor() const {
    SemanticDescriptor descriptor = ButtonBase::semantic_descriptor();
    descriptor.role = SemanticRole::check_box;
    descriptor.value = check_state_ == CheckState::checked ? "checked"
        : check_state_ == CheckState::indeterminate ? "mixed" : "unchecked";
    if (check_state_ == CheckState::checked) descriptor.states |= SemanticState::checked;
    if (check_state_ == CheckState::indeterminate) descriptor.states |= SemanticState::mixed;
    return descriptor;
}

RadioButton::RadioButton(StableId stable_id, std::string text)
    : ButtonBase(std::move(stable_id), std::move(text)) {
    set_text_alignment(ContentAlignment::middle_left);
    set_image_alignment(ContentAlignment::middle_left);
    define_bindable_property({
        {"Checked", BindingValueKind::boolean, "Behavior",
         "Whether the radio button is selected within its group.",
         BindingValue{false}, Dirty::paint | Dirty::semantics},
        [this] { return BindingValue{checked_}; },
        [this](const BindingValue& value) {
            const auto converted = convert_binding_value(value, BindingValueKind::boolean);
            if (!converted) throw std::invalid_argument("RadioButton.Checked binding requires Boolean");
            set_checked(std::get<bool>(*converted));
        },
        [this](Component& owner, std::function<void()> changed) {
            return checked_changed_.subscribe(owner,
                [changed = std::move(changed)](bool) { changed(); });
        }, {}, {}});
}

void RadioButton::set_checked_without_exclusion(bool checked_value) {
    require_mutable();
    if (checked_ == checked_value) {
        return;
    }
    checked_ = checked_value;
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(checked_changed_, checked_);
}

void RadioButton::set_checked(bool checked_value) {
    require_mutable();
    if (checked_ == checked_value) {
        return;
    }
    if (checked_value) {
        if (const Control::Ptr owner = parent()) {
            for (const Control::Ptr& sibling : owner->children()) {
                auto peer = std::dynamic_pointer_cast<RadioButton>(sibling);
                if (peer && peer.get() != this && peer->group_name_ == group_name_ &&
                    peer->checked_) {
                    peer->set_checked_without_exclusion(false);
                    if (!is_alive()) {
                        return;
                    }
                }
            }
        }
    }
    set_checked_without_exclusion(checked_value);
}

void RadioButton::set_group_name(std::string name) {
    require_mutable();
    if (group_name_ == name) {
        return;
    }
    group_name_ = std::move(name);
    invalidate(Dirty::semantics);
    if (checked_) {
        if (const Control::Ptr owner = parent()) {
            for (const Control::Ptr& sibling : owner->children()) {
                auto peer = std::dynamic_pointer_cast<RadioButton>(sibling);
                if (peer && peer.get() != this && peer->group_name_ == group_name_ &&
                    peer->checked_) {
                    peer->set_checked_without_exclusion(false);
                    if (!is_alive()) {
                        return;
                    }
                }
            }
        }
    }
}

void RadioButton::set_auto_check(bool enabled_value) {
    require_mutable();
    if (auto_check_ == enabled_value) {
        return;
    }
    auto_check_ = enabled_value;
    invalidate(Dirty::semantics);
}

void RadioButton::set_indicator_style(ChoiceIndicatorStyle style_value) {
    require_mutable();
    if (indicator_style_ == style_value) {
        return;
    }
    indicator_style_ = style_value;
    invalidate(Dirty::paint | Dirty::semantics);
}

void RadioButton::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    if (!has_style_override()) {
        const double indicator_width =
            indicator_style_ == ChoiceIndicatorStyle::toggle ? 30.0 : 15.0;
        const double top = std::max(1.0, (bounds.height - 15.0) * 0.5);
        const Rect indicator_bounds{1.0, top, indicator_width, 15.0};
        const ControlVisualContext context = visual_context(
            hovered_visual(), pressed_visual(), checked_, focused_visual());
        const ControlVisualRecipe& recipe = effective_theme().resolve(
            ControlVisualRole::choice, context);
        SurfaceMaterial indicator = recipe.material;
        indicator.corner_radius = 7.5;
        paint_surface_material(painter, indicator_bounds, indicator);
        if (indicator_style_ == ChoiceIndicatorStyle::toggle) {
            const double knob_x = checked_ ? indicator_bounds.x +
                    indicator_bounds.width - 13.0 : indicator_bounds.x + 2.0;
            painter.fill_rounded_rect(
                {knob_x, indicator_bounds.y + 2.0, 11.0, 11.0}, 5.5,
                recipe.text);
        } else if (checked_) {
            painter.fill_rounded_rect(
                {indicator_bounds.x + 4.0, indicator_bounds.y + 4.0,
                 7.0, 7.0}, 3.5, recipe.glyph);
        }
        paint_button_content(
            painter,
            {indicator_bounds.x + indicator_bounds.width + 1.0, 0.0,
             std::max(0.0, bounds.width - indicator_bounds.x -
                                   indicator_bounds.width - 1.0),
             bounds.height},
            display_text(), recipe.text, {}, checked_);
        paint_theme_cues(painter, bounds, recipe, context);
        return;
    }
    const BasicControlStyle& colors = style();
    const double indicator_width =
        indicator_style_ == ChoiceIndicatorStyle::toggle ? 30.0 : 15.0;
    const double top = std::max(1.0, (bounds.height - 15.0) * 0.5);
    if (indicator_style_ == ChoiceIndicatorStyle::toggle) {
        const Rect track{1.0, top, indicator_width, 15.0};
        painter.fill_rect(track, checked_ ? colors.accent : colors.border);
        const double knob_x = checked_ ? track.x + track.width - 13.0
                                       : track.x + 2.0;
        painter.fill_rect({knob_x, track.y + 2.0, 11.0, 11.0}, colors.paper);
        paint_button_content(
            painter,
            {track.x + track.width + 1.0, 0.0,
             std::max(0.0, bounds.width - track.x - track.width - 1.0),
             bounds.height},
            display_text(), enabled() ? colors.text : colors.disabled_text, {}, checked_);
        return;
    }
    const Color ring = indicator_style_ == ChoiceIndicatorStyle::modern && checked_
        ? colors.accent : colors.border;
    fill_radio_disc(painter, 1.0, top, ring, false);
    fill_radio_face(painter, 1.0, top, colors.paper);
    if (checked_) {
        fill_radio_disc(painter, 1.0, top,
                        indicator_style_ == ChoiceIndicatorStyle::modern
                            ? colors.accent : colors.dark_border,
                        true);
    }
    paint_button_content(
        painter, {17.0, 0.0, std::max(0.0, bounds.width - 17.0), bounds.height},
        display_text(), enabled() ? colors.text : colors.disabled_text, {}, checked_);
}

Insets RadioButton::visual_outsets() const noexcept {
    if (has_style_override()) return {};
    const ControlVisualContext context = visual_context(
        hovered_visual(), pressed_visual(), checked_, focused_visual());
    return surface_material_visual_outsets(
        effective_theme().resolve(ControlVisualRole::choice, context).material);
}

void RadioButton::on_activate() {
    if (auto_check_ && !checked_) {
        set_checked(true);
        if (!is_alive()) {
            return;
        }
    }
    ButtonBase::on_activate();
}

SemanticDescriptor RadioButton::semantic_descriptor() const {
    SemanticDescriptor descriptor = ButtonBase::semantic_descriptor();
    descriptor.role = SemanticRole::radio_button;
    descriptor.value = checked_ ? "selected" : "not selected";
    if (checked_) {
        descriptor.states |= SemanticState::checked;
        descriptor.states |= SemanticState::selected;
    }
    return descriptor;
}

LinkLabel::LinkLabel(StableId stable_id, std::string text)
    : ButtonBase(std::move(stable_id), std::move(text)) {}

void LinkLabel::set_visited(bool visited_value) {
    require_mutable();
    if (visited_ == visited_value) {
        return;
    }
    visited_ = visited_value;
    invalidate(Dirty::paint | Dirty::semantics);
}

void LinkLabel::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    const BasicControlStyle& colors = style();
    const Color foreground = enabled()
        ? (visited_ ? colors.visited_link : colors.link) : colors.disabled_text;
    const double baseline = std::max(font().size,
        (bounds.height + font().size) * 0.5 - 1.0);
    const double width = std::min(bounds.width - 4.0,
                                  estimated_text_width(display_text(), font()));
    painter.draw_text_utf8({2.0, baseline}, display_text(), font(), foreground);
    painter.draw_line({2.0, baseline + 2.0}, {2.0 + std::max(0.0, width), baseline + 2.0},
                      foreground, 1.0);
    if (focused_visual()) {
        paint_focus(painter, bounds, foreground);
    }
}

void LinkLabel::on_activate() {
    set_visited(true);
    if (is_alive()) {
        ButtonBase::on_activate();
    }
}

SemanticDescriptor LinkLabel::semantic_descriptor() const {
    SemanticDescriptor descriptor = ButtonBase::semantic_descriptor();
    descriptor.role = SemanticRole::link;
    descriptor.value = visited_ ? "visited" : "unvisited";
    return descriptor;
}

} // namespace gui_forms
