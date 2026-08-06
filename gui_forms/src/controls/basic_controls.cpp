#include "gui_forms/basic_controls.hpp"
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

} // namespace

Panel::Panel(StableId stable_id) : Control(std::move(stable_id)) {
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
    if (background_ == color) {
        return;
    }
    background_ = color;
    invalidate(Dirty::paint);
}

void Panel::set_style(BasicControlStyle style) {
    require_mutable();
    if (style_ == style) {
        return;
    }
    const bool inherited_background = background_ == style_.face;
    style_ = std::move(style);
    if (inherited_background) {
        background_ = style_.face;
    }
    invalidate(Dirty::paint | Dirty::semantics);
}

Rect Panel::local_bounds() const noexcept {
    const Rect arranged = committed_arranged_bounds();
    return {0.0, 0.0, arranged.width, arranged.height};
}

void Panel::paint_panel(Painter& painter, Rect bounds) const {
    painter.fill_rect(bounds, background_);
    switch (border_style_) {
    case BorderStyle::none:
        break;
    case BorderStyle::line:
        painter.stroke_rect({0.5, 0.5, std::max(0.0, bounds.width - 1.0),
                             std::max(0.0, bounds.height - 1.0)},
                            style_.border, 1.0);
        break;
    case BorderStyle::sunken:
        painter.draw_line({0.0, 0.0}, {bounds.width, 0.0}, style_.dark_border, 1.0);
        painter.draw_line({0.0, 0.0}, {0.0, bounds.height}, style_.dark_border, 1.0);
        painter.draw_line({0.0, bounds.height - 1.0},
                          {bounds.width, bounds.height - 1.0}, style_.highlight, 1.0);
        painter.draw_line({bounds.width - 1.0, 0.0},
                          {bounds.width - 1.0, bounds.height}, style_.highlight, 1.0);
        break;
    case BorderStyle::raised:
        painter.draw_line({0.0, 0.0}, {bounds.width, 0.0}, style_.highlight, 1.0);
        painter.draw_line({0.0, 0.0}, {0.0, bounds.height}, style_.highlight, 1.0);
        painter.draw_line({0.0, bounds.height - 1.0},
                          {bounds.width, bounds.height - 1.0}, style_.dark_border, 1.0);
        painter.draw_line({bounds.width - 1.0, 0.0},
                          {bounds.width - 1.0, bounds.height}, style_.dark_border, 1.0);
        break;
    }
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

void GroupBox::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    painter.fill_rect(bounds, background());
    painter.stroke_rect({0.5, 10.5, std::max(0.0, bounds.width - 1.0),
                         std::max(0.0, bounds.height - 11.0)},
                        style().border, 1.0);
    const double caption_width = std::min(
        std::max(0.0, bounds.width - 18.0), estimated_text_width(text_, font_) + 12.0);
    painter.fill_rect({9.0, 3.0, caption_width, 16.0}, background());
    painter.draw_text_utf8({13.0, 15.0}, text_, font_,
                           enabled() ? style().text : style().disabled_text);
}

SemanticDescriptor GroupBox::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::group;
    descriptor.name = accessible_name().empty() ? text_ : accessible_name();
    descriptor.description = accessible_description();
    descriptor.exposed = true;
    return descriptor;
}

PictureBox::PictureBox(StableId stable_id) : Panel(std::move(stable_id)) {
    set_background(Color::rgba(255, 255, 255));
}

void PictureBox::set_image(ImageId image) {
    require_mutable();
    if (image_ == image) {
        return;
    }
    image_ = image;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
    image_changed_.emit(image_);
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
    : Control(std::move(stable_id)), text_(std::move(text)) {}

void Label::set_text(std::string text) {
    require_mutable();
    if (text_ == text) {
        return;
    }
    text_ = std::move(text);
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
    text_changed_.emit(text_);
}

void Label::set_font(FontSpec font) {
    require_mutable();
    if (!valid_font_spec(font)) {
        throw std::invalid_argument("Label font specification is invalid");
    }
    if (font_ == font) {
        return;
    }
    font_ = font;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void Label::set_foreground(Color color) {
    require_mutable();
    if (foreground_ == color) {
        return;
    }
    foreground_ = color;
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

Size Label::measure(Size available) {
    const Rect requested = requested_bounds();
    const std::string text = display_text();
    const double wrap_width = requested.width > 0.0
        ? requested.width : available.width;
    const auto lines = label_lines(text, font_, std::max(0.0, wrap_width - 4.0),
                                   text_wrapping_);
    double content_width{};
    for (const std::string& line : lines) {
        content_width = std::max(content_width, estimated_text_width(line, font_));
    }
    const double preferred_width = requested.width > 0.0
        ? requested.width : content_width + 4.0;
    const double preferred_height = requested.height > 0.0
        ? requested.height
        : static_cast<double>(lines.size()) * font_.size * line_spacing_ + 4.0;
    return {std::min(available.width, preferred_width),
            std::min(available.height, preferred_height)};
}

std::string Label::display_text() const {
    return text_;
}

void Label::paint_label_text(Painter& painter, std::string_view text) const {
    const Rect arranged = committed_arranged_bounds();
    const auto lines = label_lines(text, font_, std::max(0.0, arranged.width - 4.0),
                                   text_wrapping_);
    const double line_height = font_.size * line_spacing_;
    const double block_height = static_cast<double>(lines.size()) * line_height;
    double top = 1.0;
    if (vertical_alignment_ == VerticalAlignment::center) {
        top = std::max(1.0, (arranged.height - block_height) * 0.5);
    } else if (vertical_alignment_ == VerticalAlignment::far) {
        top = std::max(1.0, arranged.height - block_height - 1.0);
    }
    const Color color = enabled() ? foreground_ : Color::rgba(132, 143, 153);
    for (std::size_t index = 0; index < lines.size(); ++index) {
        const double text_width = estimated_text_width(lines[index], font_);
        double x = 2.0;
        if (alignment_ == HorizontalAlignment::center) {
            x = std::max(2.0, (arranged.width - text_width) * 0.5);
        } else if (alignment_ == HorizontalAlignment::far) {
            x = std::max(2.0, arranged.width - text_width - 2.0);
        }
        const double baseline = top + font_.size +
            static_cast<double>(index) * line_height;
        painter.draw_text_utf8({x, baseline}, lines[index], font_, color);
    }
}

void Label::on_paint(Painter& painter, Rect) {
    paint_label_text(painter, display_text());
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
}

void ButtonBase::set_text(std::string text) {
    require_mutable();
    if (text_ == text) {
        return;
    }
    text_ = std::move(text);
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
    text_changed_.emit(text_);
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
    if (style_ == style) {
        return;
    }
    style_ = std::move(style);
    invalidate(Dirty::paint | Dirty::semantics);
}

Size ButtonBase::measure(Size available) {
    const Rect requested = requested_bounds();
    const double preferred_width = requested.width > 0.0
        ? requested.width : estimated_text_width(text_, font_) + 22.0;
    const double preferred_height = requested.height > 0.0
        ? requested.height : std::max(24.0, font_.size + 12.0);
    return {std::min(available.width, preferred_width),
            std::min(available.height, preferred_height)};
}

Rect ButtonBase::local_bounds() const noexcept {
    const Rect arranged = committed_arranged_bounds();
    return {0.0, 0.0, arranged.width, arranged.height};
}

void ButtonBase::paint_button_frame(Painter& painter, Rect bounds,
                                    bool default_cue) const {
    paint_relief(painter, bounds, style_, pressed_visual());
    if (default_cue && bounds.width > 2.0 && bounds.height > 2.0) {
        painter.stroke_rect({0.5, 0.5, bounds.width - 1.0, bounds.height - 1.0},
                            style_.accent, 1.0);
    }
    if (focused_) {
        paint_focus(painter, bounds, style_.text);
    }
}

void ButtonBase::paint_button_text(Painter& painter, Rect bounds,
                                   std::string_view text) const {
    const double x = std::max(6.0,
        (bounds.width - estimated_text_width(text, font_)) * 0.5);
    const double y = std::max(font_.size,
        (bounds.height + font_.size) * 0.5 - 1.0);
    const double offset = pressed_visual() ? 1.0 : 0.0;
    painter.draw_text_utf8({x + offset, y + offset}, text, font_,
                           enabled() ? style_.text : style_.disabled_text);
}

void ButtonBase::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    paint_button_frame(painter, bounds, false);
    paint_button_text(painter, bounds, text_);
}

void ButtonBase::on_pointer(PointerEvent& event) {
    if (event.button == PointerButton::primary && event.action == PointerAction::down) {
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
    descriptor.name = accessible_name().empty() ? text_ : accessible_name();
    descriptor.description = accessible_description();
    descriptor.actions = {SemanticAction::focus, SemanticAction::press};
    descriptor.exposed = true;
    return descriptor;
}

bool ButtonBase::on_semantic_action(SemanticAction action, std::string_view value) {
    if (action == SemanticAction::press) {
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
    switch (visual_style_) {
    case ButtonVisualStyle::standard:
        paint_button_frame(painter, bounds, default_button_);
        paint_button_text(painter, bounds, text());
        break;
    case ButtonVisualStyle::flat:
        painter.fill_rect(bounds, pressed_visual() ? style().accent_light
                                                   : style().face_light);
        painter.stroke_rect({0.5, 0.5, std::max(0.0, bounds.width - 1.0),
                             std::max(0.0, bounds.height - 1.0)},
                            default_button_ ? style().accent : style().border, 1.0);
        paint_button_text(painter, bounds, text());
        break;
    case ButtonVisualStyle::accent: {
        const Color fill = pressed_visual() ? style().link : style().accent;
        painter.fill_rect(bounds, fill);
        painter.draw_line({0.0, 0.0}, {bounds.width - 1.0, 0.0},
                          style().accent_light, 1.0);
        painter.stroke_rect({0.5, 0.5, std::max(0.0, bounds.width - 1.0),
                             std::max(0.0, bounds.height - 1.0)},
                            style().dark_border, 1.0);
        const double x = std::max(6.0,
            (bounds.width - estimated_text_width(text(), font())) * 0.5);
        const double y = std::max(font().size,
            (bounds.height + font().size) * 0.5 - 1.0);
        const double offset = pressed_visual() ? 1.0 : 0.0;
        painter.draw_text_utf8({x + offset, y + offset}, text(), font(),
                               enabled() ? style().paper : style().disabled_text);
        break;
    }
    case ButtonVisualStyle::command:
        painter.fill_rect(bounds, pressed_visual() ? style().accent_light
                                                   : style().face);
        painter.fill_rect({0.0, 0.0, 4.0, bounds.height}, style().accent);
        painter.stroke_rect({0.5, 0.5, std::max(0.0, bounds.width - 1.0),
                             std::max(0.0, bounds.height - 1.0)},
                            style().border, 1.0);
        painter.draw_text_utf8({12.0, std::max(font().size,
                                  (bounds.height + font().size) * 0.5 - 1.0)},
                               text(), font(), enabled() ? style().text
                                                        : style().disabled_text);
        break;
    }
}

CheckBox::CheckBox(StableId stable_id, std::string text)
    : ButtonBase(std::move(stable_id), std::move(text)) {}

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
    check_state_changed_.emit(check_state_);
    if (!is_alive()) {
        return;
    }
    if (previous_checked != checked()) {
        checked_changed_.emit(checked());
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
    painter.draw_text_utf8({box.x + box.width + 7.0, std::max(font().size,
                              (bounds.height + font().size) * 0.5 - 1.0)},
                           text(), font(), enabled() ? colors.text : colors.disabled_text);
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
    : ButtonBase(std::move(stable_id), std::move(text)) {}

void RadioButton::set_checked_without_exclusion(bool checked_value) {
    require_mutable();
    if (checked_ == checked_value) {
        return;
    }
    checked_ = checked_value;
    invalidate(Dirty::paint | Dirty::semantics);
    checked_changed_.emit(checked_);
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
        painter.draw_text_utf8({track.x + track.width + 7.0,
                                std::max(font().size,
                                    (bounds.height + font().size) * 0.5 - 1.0)},
                               text(), font(), enabled() ? colors.text
                                                        : colors.disabled_text);
        return;
    }
    const Point outline[] = {{5.0, top}, {11.0, top}, {15.0, top + 4.0},
                             {15.0, top + 10.0}, {11.0, top + 14.0},
                             {5.0, top + 14.0}, {1.0, top + 10.0},
                             {1.0, top + 4.0}, {5.0, top}};
    for (std::size_t index = 1; index < std::size(outline); ++index) {
        painter.draw_line(outline[index - 1], outline[index],
                          indicator_style_ == ChoiceIndicatorStyle::modern && checked_
                              ? colors.accent : colors.border,
                          indicator_style_ == ChoiceIndicatorStyle::modern ? 2.0 : 1.0);
    }
    if (checked_) {
        painter.fill_rect({indicator_style_ == ChoiceIndicatorStyle::modern ? 5.0 : 6.0,
                           top + (indicator_style_ == ChoiceIndicatorStyle::modern ? 4.0 : 5.0),
                           indicator_style_ == ChoiceIndicatorStyle::modern ? 7.0 : 5.0,
                           indicator_style_ == ChoiceIndicatorStyle::modern ? 7.0 : 5.0},
                          colors.accent);
    }
    painter.draw_text_utf8({23.0, std::max(font().size,
                              (bounds.height + font().size) * 0.5 - 1.0)},
                           text(), font(), enabled() ? colors.text : colors.disabled_text);
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
                                  estimated_text_width(text(), font()));
    painter.draw_text_utf8({2.0, baseline}, text(), font(), foreground);
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
