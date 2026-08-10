#include "gui_forms/controls/panel/date_time_picker/date_time_picker.hpp"

#include "calendar_popup/calendar_popup.hpp"
#include "calendar_popup_layer/calendar_popup_layer.hpp"
#include "date_time_utilities.hpp"
#include "gui_forms/text.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace gui_forms {
using namespace date_time_detail;

DateTimePicker::DateTimePicker(StableId stable_id) : Panel(std::move(stable_id)) {
    set_background(style_.paper);
    set_border_style(BorderStyle::sunken);
    set_focusable(true);
}

void DateTimePicker::set_value(DateTimeValue value) {
    require_mutable();
    if (!valid_date_time(value) || value < minimum_ || value > maximum_) {
        throw std::out_of_range("DateTimePicker value is invalid or outside its range");
    }
    if (value_ == value) return;
    value_ = value;
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(value_changed_, value_);
}

void DateTimePicker::set_range(DateTimeValue minimum, DateTimeValue maximum) {
    require_mutable();
    if (!valid_date_time(minimum) || !valid_date_time(maximum) || minimum > maximum) {
        throw std::invalid_argument("DateTimePicker requires a valid ordered range");
    }
    if (minimum_ == minimum && maximum_ == maximum) return;
    minimum_ = minimum;
    maximum_ = maximum;
    const DateTimeValue clamped = std::clamp(value_, minimum_, maximum_);
    const bool value_changes = clamped != value_;
    value_ = clamped;
    if (dropped_down_) {
        close_drop_down();
    }
    invalidate(Dirty::paint | Dirty::semantics);
    if (value_changes) publish_change(value_changed_, value_);
}

void DateTimePicker::set_format(DateTimePickerFormat format) {
    require_mutable();
    if (format_ == format) return;
    format_ = format;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void DateTimePicker::set_custom_format(std::string format) {
    require_mutable();
    if (!validate_utf8(format).valid()) {
        throw std::invalid_argument("DateTimePicker custom format must be valid UTF-8");
    }
    if (custom_format_ == format) return;
    custom_format_ = std::move(format);
    if (format_ == DateTimePickerFormat::custom) {
        invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
    }
}

void DateTimePicker::set_format_provider(DateTimeFormatProvider provider) {
    require_mutable();
    validate_provider(provider);
    format_provider_ = std::move(provider);
    if (dropped_down_) close_drop_down();
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

std::string DateTimePicker::formatted_value() const {
    std::string_view pattern;
    switch (format_) {
    case DateTimePickerFormat::long_date: pattern = format_provider_.long_date_pattern; break;
    case DateTimePickerFormat::short_date: pattern = format_provider_.short_date_pattern; break;
    case DateTimePickerFormat::time: pattern = format_provider_.time_pattern; break;
    case DateTimePickerFormat::custom: pattern = custom_format_; break;
    }
    return format_date_time(value_, pattern, format_provider_);
}

void DateTimePicker::set_show_check_box(bool show) {
    require_mutable();
    if (show_check_box_ == show) return;
    show_check_box_ = show;
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

void DateTimePicker::set_checked(bool checked) {
    require_mutable();
    if (checked_ == checked) return;
    checked_ = checked;
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(checked_changed_, checked_);
}

void DateTimePicker::set_show_up_down(bool show) {
    require_mutable();
    if (show_up_down_ == show) return;
    show_up_down_ = show;
    if (show && dropped_down_) close_drop_down();
    invalidate(Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

void DateTimePicker::set_dropped_down(bool dropped_down) {
    require_mutable();
    if (dropped_down == dropped_down_) return;
    if (dropped_down) open_drop_down(); else close_drop_down();
}

void DateTimePicker::set_drop_down_alignment(
    DateTimeDropDownAlignment alignment) {
    require_mutable();
    if (alignment != DateTimeDropDownAlignment::left &&
        alignment != DateTimeDropDownAlignment::right) {
        throw std::invalid_argument(
            "DateTimePicker drop-down alignment is invalid");
    }
    if (drop_down_alignment_ == alignment) return;
    drop_down_alignment_ = alignment;
    invalidate(Dirty::paint | Dirty::semantics);
}

void DateTimePicker::set_font(FontSpec font) {
    require_mutable();
    if (!valid_font_spec(font)) {
        throw std::invalid_argument("DateTimePicker font specification is invalid");
    }
    if (font_ == font) return;
    font_ = font;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void DateTimePicker::set_style(BasicControlStyle style) {
    require_mutable();
    if (style_ == style) return;
    style_ = style;
    set_background(style_.paper);
    invalidate(Dirty::paint);
}

DateTimePicker::HitPart DateTimePicker::part_at(Point absolute) const noexcept {
    const Rect bounds = absolute_bounds();
    if (!bounds.contains(absolute)) return HitPart::none;
    const double local_x = absolute.x - bounds.x;
    const double local_y = absolute.y - bounds.y;
    if (show_check_box_ && local_x < 25.0) return HitPart::check;
    if (local_x >= bounds.width - 24.0) {
        if (!show_up_down_) return HitPart::arrow_down;
        return local_y < bounds.height * 0.5 ? HitPart::arrow_up : HitPart::arrow_down;
    }
    return HitPart::body;
}

void DateTimePicker::on_paint(Painter& painter, Rect damage) {
    Panel::on_paint(painter, damage);
    const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                      committed_arranged_bounds().height};
    const FontSpec font = effective_font(font_);
    const double button_width = std::min(24.0, bounds.width);
    const double button_x = std::max(0.0, bounds.width - button_width);
    painter.fill_rect({button_x, 1.0, std::max(0.0, button_width - 1.0),
                       std::max(0.0, bounds.height - 2.0)}, style_.face);
    painter.draw_line({button_x, 1.0}, {button_x, bounds.height - 1.0},
                      style_.border, 1.0);
    const double center_x = button_x + button_width * 0.5;
    if (show_up_down_) {
        const double middle = std::floor(bounds.height * 0.5);
        painter.draw_line({button_x, middle}, {bounds.width, middle},
                          style_.border, 1.0);
        painter.draw_line({center_x - 3.5, middle - 4.0}, {center_x, middle - 7.0},
                          style_.dark_border, 1.0);
        painter.draw_line({center_x, middle - 7.0}, {center_x + 3.5, middle - 4.0},
                          style_.dark_border, 1.0);
        painter.draw_line({center_x - 3.5, middle + 4.0}, {center_x, middle + 7.0},
                          style_.dark_border, 1.0);
        painter.draw_line({center_x, middle + 7.0}, {center_x + 3.5, middle + 4.0},
                          style_.dark_border, 1.0);
    } else {
        const double center_y = bounds.height * 0.5 + (dropped_down_ ? 2.0 : -1.0);
        const double direction = dropped_down_ ? -1.0 : 1.0;
        painter.draw_line({center_x - 4.0, center_y - direction * 2.0},
                          {center_x, center_y + direction * 2.0},
                          style_.dark_border, 1.0);
        painter.draw_line({center_x, center_y + direction * 2.0},
                          {center_x + 4.0, center_y - direction * 2.0},
                          style_.dark_border, 1.0);
    }
    double text_left = 7.0;
    if (show_check_box_) {
        const double check_y = std::max(3.0, (bounds.height - 14.0) * 0.5);
        painter.fill_rect({5.0, check_y, 14.0, 14.0}, style_.paper);
        painter.stroke_rect({5.5, check_y + 0.5, 13.0, 13.0}, style_.dark_border, 1.0);
        if (checked_) {
            painter.draw_line({8.0, check_y + 7.0}, {11.0, check_y + 10.0},
                              style_.accent, 2.0);
            painter.draw_line({11.0, check_y + 10.0}, {17.0, check_y + 3.5},
                              style_.accent, 2.0);
        }
        text_left = 25.0;
    }
    painter.save();
    painter.clip_rect({text_left, 2.0,
                       std::max(0.0, button_x - text_left - 3.0),
                       std::max(0.0, bounds.height - 4.0)});
    painter.draw_text_utf8(
        {text_left, std::max(font.size, (bounds.height + font.size) * 0.5 - 1.0)},
        formatted_value(), font,
        enabled() && (!show_check_box_ || checked_) ? style_.text
                                                    : style_.disabled_text);
    painter.restore();
    if (focused_ || dropped_down_) {
        painter.stroke_rect({1.5, 1.5, std::max(0.0, bounds.width - 3.0),
                             std::max(0.0, bounds.height - 3.0)},
                            style_.accent, 1.0);
    }
}

void DateTimePicker::on_pointer(PointerEvent& event) {
    if (!eligible_for_input() || event.button != PointerButton::primary) return;
    if (event.action == PointerAction::down) {
        pressed_part_ = part_at(event.position);
        activation_part_ = HitPart::none;
        event.handled = pressed_part_ != HitPart::none;
        if (event.handled) invalidate(Dirty::paint);
    } else if (event.action == PointerAction::up && pressed_part_ != HitPart::none) {
        const HitPart released = part_at(event.position);
        if (released == pressed_part_) activation_part_ = pressed_part_;
        pressed_part_ = HitPart::none;
        event.handled = true;
        invalidate(Dirty::paint);
    }
}

void DateTimePicker::on_activate() {
    HitPart part = std::exchange(activation_part_, HitPart::none);
    if (part == HitPart::none) part = HitPart::body;
    if (part == HitPart::check) {
        set_checked(!checked_);
    } else if (show_up_down_ && part == HitPart::arrow_up) {
        step_days(1);
    } else if (show_up_down_ && part == HitPart::arrow_down) {
        step_days(-1);
    } else if (!show_up_down_) {
        set_dropped_down(!dropped_down_);
    }
}

void DateTimePicker::on_key(KeyEvent& event) {
    if (!focused_ || !enabled() || event.action != KeyAction::down) return;
    if ((event.physical_key == PhysicalKey::f4 ||
         (event.physical_key == PhysicalKey::down &&
          includes(event.modifiers, Modifier::alt))) && !show_up_down_) {
        set_dropped_down(!dropped_down_);
        event.handled = true;
    } else if (event.physical_key == PhysicalKey::up ||
               event.physical_key == PhysicalKey::right) {
        step_days(1);
        event.handled = true;
    } else if (event.physical_key == PhysicalKey::down ||
               event.physical_key == PhysicalKey::left) {
        step_days(-1);
        event.handled = true;
    } else if (event.physical_key == PhysicalKey::space && show_check_box_) {
        set_checked(!checked_);
        event.handled = true;
    }
}

void DateTimePicker::on_focus_changed(bool focused) {
    focused_ = focused;
    invalidate(Dirty::paint | Dirty::semantics);
}

void DateTimePicker::step_days(int days) {
    if (show_check_box_ && !checked_) set_checked(true);
    const DateTimeValue candidate = add_days(value_, days);
    if (!civil_in_range(candidate, minimum_, maximum_)) return;
    set_value(std::clamp(candidate, minimum_, maximum_));
}

void DateTimePicker::open_drop_down() {
    if (dropped_down_ || show_up_down_ || !attached() || window() == nullptr) return;
    const Control::Ptr owner = shared_from_this();
    const Size client = window()->client_size();
    const Rect picker = absolute_bounds();
    const double width = 286.0 * effective_text_scale();
    const double height = 260.0 * effective_text_scale();
    const double requested_x =
        drop_down_alignment_ == DateTimeDropDownAlignment::right
        ? picker.x + picker.width - width : picker.x;
    const double x = std::clamp(
        requested_x, 0.0, std::max(0.0, client.width - width));
    const double below = picker.y + picker.height;
    const double y = below + height <= client.height ? below
        : std::max(0.0, picker.y - height);
    const std::string prefix(stable_id().value());
    auto layer = make_control<CalendarPopupLayer>(StableId(prefix + ".popup.layer"));
    layer->set_requested_bounds({0.0, 0.0, client.width, client.height});
    auto calendar = make_control<CalendarPopup>(
        StableId(prefix + ".popup.calendar"), value_, minimum_, maximum_,
        format_provider_, style_);
    calendar->set_requested_bounds({x, y, width, height});
    layer->add_child(calendar);
    PopupToken token = window()->open_popup(owner, layer);

    popup_layer_ = layer;
    popup_calendar_ = calendar;
    popup_token_ = std::move(token);
    const std::weak_ptr<DateTimePicker> weak =
        std::static_pointer_cast<DateTimePicker>(shared_from_this());
    popup_commit_ = calendar->committed().subscribe(
        *this, [weak](DateTimeValue date) {
            if (const auto picker = weak.lock()) picker->commit_popup_value(date);
        });
    popup_cancel_ = calendar->cancelled().subscribe(*this, [weak] {
        if (const auto picker = weak.lock()) picker->close_drop_down();
    });
    popup_dismiss_ = layer->dismissed().subscribe(*this, [weak] {
        if (const auto picker = weak.lock()) picker->close_drop_down();
    });
    if (Event<>* closed = popup_token_.closed_event()) {
        popup_revocation_ = closed->subscribe(*this, [weak] {
            if (const auto picker = weak.lock()) picker->on_popup_revoked();
        });
    }
    popup_scope_ = window()->begin_focus_scope(layer, calendar).value;
    dropped_down_ = true;
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(drop_down_changed_, true);
}

void DateTimePicker::close_drop_down() {
    if (!dropped_down_ && !popup_layer_) return;
    closing_popup_ = true;
    popup_commit_.disconnect();
    popup_cancel_.disconnect();
    popup_dismiss_.disconnect();
    if (window() != nullptr && popup_scope_ != 0U) {
        static_cast<void>(window()->end_focus_scope(FocusScopeId{popup_scope_}));
    }
    popup_scope_ = 0U;
    popup_token_.disconnect();
    popup_revocation_.disconnect();
    popup_calendar_.reset();
    popup_layer_.reset();
    const bool changed = dropped_down_;
    dropped_down_ = false;
    closing_popup_ = false;
    invalidate(Dirty::paint | Dirty::semantics);
    if (changed) publish_change(drop_down_changed_, false);
}

void DateTimePicker::on_popup_revoked() {
    if (closing_popup_) return;
    popup_commit_.disconnect();
    popup_cancel_.disconnect();
    popup_dismiss_.disconnect();
    popup_revocation_.disconnect();
    if (window() != nullptr && popup_scope_ != 0U) {
        static_cast<void>(window()->end_focus_scope(
            FocusScopeId{popup_scope_}, FocusScopeCloseReason::owner_unavailable));
    }
    popup_scope_ = 0U;
    popup_calendar_.reset();
    popup_layer_.reset();
    const bool changed = dropped_down_;
    dropped_down_ = false;
    invalidate(Dirty::paint | Dirty::semantics);
    if (changed) publish_change(drop_down_changed_, false);
}

void DateTimePicker::commit_popup_value(DateTimeValue value) {
    set_value(value);
    close_drop_down();
}

SemanticDescriptor DateTimePicker::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::date_picker;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    descriptor.value = formatted_value();
    if (show_check_box_ && checked_) descriptor.states |= SemanticState::checked;
    if (dropped_down_) descriptor.states |= SemanticState::expanded;
    descriptor.actions = {SemanticAction::focus, SemanticAction::increment,
                          SemanticAction::decrement, SemanticAction::set_value};
    if (show_check_box_) {
        descriptor.actions.push_back(SemanticAction::press);
    }
    if (!show_up_down_) {
        descriptor.actions.push_back(dropped_down_ ? SemanticAction::collapse
                                                   : SemanticAction::expand);
    }
    descriptor.exposed = true;
    return descriptor;
}

bool DateTimePicker::on_semantic_action(SemanticAction action,
                                        std::string_view value) {
    if (action == SemanticAction::focus) {
        return Control::on_semantic_action(action, value);
    }
    if (action == SemanticAction::increment) {
        step_days(1);
        return true;
    }
    if (action == SemanticAction::decrement) {
        step_days(-1);
        return true;
    }
    if (action == SemanticAction::press && show_check_box_) {
        set_checked(!checked_);
        return true;
    }
    if (action == SemanticAction::expand && !show_up_down_) {
        set_dropped_down(true);
        return dropped_down_;
    }
    if (action == SemanticAction::collapse && !show_up_down_) {
        set_dropped_down(false);
        return !dropped_down_;
    }
    if (action == SemanticAction::set_value) {
        DateTimeValue parsed = value_;
        if (!parse_iso_date(value, parsed) || parsed < minimum_ || parsed > maximum_) {
            return false;
        }
        if (show_check_box_ && !checked_) set_checked(true);
        set_value(parsed);
        return true;
    }
    return false;
}

void DateTimePicker::on_detached_from_window() noexcept {
    try {
        close_drop_down();
    } catch (...) {
    }
}

} // namespace gui_forms
