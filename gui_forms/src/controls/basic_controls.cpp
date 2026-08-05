#include "gui_forms/basic_controls.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace gui_forms {
namespace {

[[nodiscard]] double estimated_text_width(std::string_view text,
                                          FontSpec font) noexcept {
    return static_cast<double>(text.size()) * font.size * 0.56;
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
        painter.stroke_rect({bounds.x + 4.5, bounds.y + 4.5,
                             bounds.width - 9.0, bounds.height - 9.0}, color, 1.0);
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

Size Label::measure(Size available) {
    const Rect requested = requested_bounds();
    const std::string text = display_text();
    const double preferred_width = requested.width > 0.0
        ? requested.width : estimated_text_width(text, font_) + 4.0;
    const double preferred_height = requested.height > 0.0
        ? requested.height : font_.size + 8.0;
    return {std::min(available.width, preferred_width),
            std::min(available.height, preferred_height)};
}

std::string Label::display_text() const {
    return text_;
}

void Label::paint_label_text(Painter& painter, std::string_view text) const {
    const Rect arranged = committed_arranged_bounds();
    const double text_width = estimated_text_width(text, font_);
    double x = 2.0;
    if (alignment_ == HorizontalAlignment::center) {
        x = std::max(2.0, (arranged.width - text_width) * 0.5);
    } else if (alignment_ == HorizontalAlignment::far) {
        x = std::max(2.0, arranged.width - text_width - 2.0);
    }
    const double baseline = std::max(font_.size,
        (arranged.height + font_.size) * 0.5 - 1.0);
    painter.draw_text_utf8({x, baseline}, text, font_,
                           enabled() ? foreground_ : Color::rgba(132, 143, 153));
}

void Label::on_paint(Painter& painter, Rect) {
    paint_label_text(painter, display_text());
}

bool Label::hit_test_local(Point) const {
    return false;
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

void Button::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    paint_button_frame(painter, bounds, default_button_);
    paint_button_text(painter, bounds, text());
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

void CheckBox::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    const BasicControlStyle& colors = style();
    const Rect box{1.0, std::max(1.0, (bounds.height - 15.0) * 0.5), 15.0, 15.0};
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
    if (check_state_ == CheckState::checked) {
        painter.draw_line({4.0, box.y + 7.0}, {7.0, box.y + 10.0},
                          colors.accent, 2.0);
        painter.draw_line({7.0, box.y + 10.0}, {14.0, box.y + 3.0},
                          colors.accent, 2.0);
    } else if (check_state_ == CheckState::indeterminate) {
        painter.fill_rect({4.0, box.y + 6.0, 9.0, 4.0}, colors.accent);
    }
    painter.draw_text_utf8({23.0, std::max(font().size,
                              (bounds.height + font().size) * 0.5 - 1.0)},
                           text(), font(), enabled() ? colors.text : colors.disabled_text);
    if (focused_visual()) {
        paint_focus(painter, {19.0, 1.0, std::max(0.0, bounds.width - 19.0),
                              std::max(0.0, bounds.height - 2.0)}, colors.text);
    }
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

void RadioButton::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    const BasicControlStyle& colors = style();
    const double top = std::max(1.0, (bounds.height - 15.0) * 0.5);
    const Point outline[] = {{5.0, top}, {11.0, top}, {15.0, top + 4.0},
                             {15.0, top + 10.0}, {11.0, top + 14.0},
                             {5.0, top + 14.0}, {1.0, top + 10.0},
                             {1.0, top + 4.0}, {5.0, top}};
    for (std::size_t index = 1; index < std::size(outline); ++index) {
        painter.draw_line(outline[index - 1], outline[index], colors.border, 1.0);
    }
    if (checked_) {
        painter.fill_rect({6.0, top + 5.0, 5.0, 5.0}, colors.accent);
    }
    painter.draw_text_utf8({23.0, std::max(font().size,
                              (bounds.height + font().size) * 0.5 - 1.0)},
                           text(), font(), enabled() ? colors.text : colors.disabled_text);
    if (focused_visual()) {
        paint_focus(painter, {19.0, 1.0, std::max(0.0, bounds.width - 19.0),
                              std::max(0.0, bounds.height - 2.0)}, colors.text);
    }
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

} // namespace gui_forms
