#include "gui_forms/date_time_picker.hpp"

#include "gui_forms/text.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace gui_forms {
namespace {

constexpr std::int64_t days_from_civil(std::int64_t year, unsigned month,
                                       unsigned day) noexcept {
    year -= month <= 2U;
    const std::int64_t era = (year >= 0 ? year : year - 399) / 400;
    const unsigned year_of_era = static_cast<unsigned>(year - era * 400);
    const unsigned day_of_year =
        (153U * (month > 2U ? month - 3U : month + 9U) + 2U) / 5U + day - 1U;
    const unsigned day_of_era = year_of_era * 365U + year_of_era / 4U -
        year_of_era / 100U + day_of_year;
    return era * 146097 + static_cast<std::int64_t>(day_of_era) - 719468;
}

DateTimeValue civil_from_days(std::int64_t serial, DateTimeValue time) noexcept {
    serial += 719468;
    const std::int64_t era = (serial >= 0 ? serial : serial - 146096) / 146097;
    const unsigned day_of_era = static_cast<unsigned>(serial - era * 146097);
    const unsigned year_of_era =
        (day_of_era - day_of_era / 1460U + day_of_era / 36524U -
         day_of_era / 146096U) / 365U;
    std::int64_t year = static_cast<std::int64_t>(year_of_era) + era * 400;
    const unsigned day_of_year = day_of_era -
        (365U * year_of_era + year_of_era / 4U - year_of_era / 100U);
    const unsigned month_prime = (5U * day_of_year + 2U) / 153U;
    const unsigned day = day_of_year - (153U * month_prime + 2U) / 5U + 1U;
    const unsigned month = month_prime < 10U ? month_prime + 3U : month_prime - 9U;
    year += month <= 2U;
    if (year < std::numeric_limits<std::int32_t>::min() ||
        year > std::numeric_limits<std::int32_t>::max()) return time;
    time.year = static_cast<std::int32_t>(year);
    time.month = static_cast<std::uint8_t>(month);
    time.day = static_cast<std::uint8_t>(day);
    return time;
}

[[nodiscard]] bool includes(Modifier value, Modifier requested) noexcept {
    return (static_cast<std::uint8_t>(value) &
            static_cast<std::uint8_t>(requested)) != 0U;
}

[[nodiscard]] constexpr DateTimeValue civil_part(DateTimeValue value) noexcept {
    value.hour = 0U;
    value.minute = 0U;
    value.second = 0U;
    value.millisecond = 0U;
    return value;
}

[[nodiscard]] constexpr bool civil_in_range(DateTimeValue value,
                                             DateTimeValue minimum,
                                             DateTimeValue maximum) noexcept {
    const DateTimeValue date = civil_part(value);
    return date >= civil_part(minimum) && date <= civil_part(maximum);
}

[[nodiscard]] std::string padded(unsigned value, unsigned digits) {
    char buffer[32]{};
    std::snprintf(buffer, sizeof(buffer), digits == 2U ? "%02u" : "%04u", value);
    return buffer;
}

[[nodiscard]] std::string format_date_time(
    DateTimeValue value, std::string_view pattern,
    const DateTimeFormatProvider& provider) {
    std::string result;
    bool quoted = false;
    for (std::size_t index = 0U; index < pattern.size();) {
        if (pattern[index] == '\'') {
            quoted = !quoted;
            ++index;
            continue;
        }
        if (pattern[index] == '\\' && index + 1U < pattern.size()) {
            result.push_back(pattern[index + 1U]);
            index += 2U;
            continue;
        }
        if (quoted || !std::isalpha(static_cast<unsigned char>(pattern[index]))) {
            result.push_back(pattern[index++]);
            continue;
        }
        const char token = pattern[index];
        std::size_t count = 1U;
        while (index + count < pattern.size() && pattern[index + count] == token) {
            ++count;
        }
        switch (token) {
        case 'y':
            result += count <= 2U
                ? padded(static_cast<unsigned>(std::abs(value.year) % 100), 2U)
                : padded(static_cast<unsigned>(std::abs(value.year)), 4U);
            break;
        case 'M':
            if (count >= 4U) result += provider.month_names[value.month - 1U];
            else if (count == 3U) result += provider.abbreviated_month_names[value.month - 1U];
            else if (count == 2U) result += padded(value.month, 2U);
            else result += std::to_string(value.month);
            break;
        case 'd':
            if (count >= 4U) result += provider.day_names[day_of_week(value)];
            else if (count == 3U) result += provider.abbreviated_day_names[day_of_week(value)];
            else if (count == 2U) result += padded(value.day, 2U);
            else result += std::to_string(value.day);
            break;
        case 'H':
            result += count >= 2U ? padded(value.hour, 2U)
                                  : std::to_string(value.hour);
            break;
        case 'h': {
            const unsigned hour = value.hour % 12U == 0U ? 12U : value.hour % 12U;
            result += count >= 2U ? padded(hour, 2U) : std::to_string(hour);
            break;
        }
        case 'm':
            result += count >= 2U ? padded(value.minute, 2U)
                                  : std::to_string(value.minute);
            break;
        case 's':
            result += count >= 2U ? padded(value.second, 2U)
                                  : std::to_string(value.second);
            break;
        case 't': {
            const std::string& designator = value.hour < 12U
                ? provider.am_designator : provider.pm_designator;
            result += count == 1U && !designator.empty()
                ? designator.substr(0U, 1U) : designator;
            break;
        }
        default:
            result.append(pattern.substr(index, count));
            break;
        }
        index += count;
    }
    return result;
}

void validate_provider(const DateTimeFormatProvider& provider) {
    const auto valid_text = [](const std::string& value) {
        return !value.empty() && validate_utf8(value).valid();
    };
    for (const auto& value : provider.month_names) {
        if (!valid_text(value)) throw std::invalid_argument("DateTime month name");
    }
    for (const auto& value : provider.abbreviated_month_names) {
        if (!valid_text(value)) throw std::invalid_argument("DateTime abbreviated month name");
    }
    for (const auto& value : provider.day_names) {
        if (!valid_text(value)) throw std::invalid_argument("DateTime day name");
    }
    for (const auto& value : provider.abbreviated_day_names) {
        if (!valid_text(value)) throw std::invalid_argument("DateTime abbreviated day name");
    }
    if (!valid_text(provider.long_date_pattern) ||
        !valid_text(provider.short_date_pattern) ||
        !valid_text(provider.time_pattern) ||
        !validate_utf8(provider.am_designator).valid() ||
        !validate_utf8(provider.pm_designator).valid()) {
        throw std::invalid_argument("DateTime format provider text");
    }
}

[[nodiscard]] std::uint64_t virtual_runtime_id(std::string_view stable_id) noexcept {
    std::uint64_t value = 1469598103934665603ULL;
    for (const unsigned char byte : stable_id) {
        value ^= byte;
        value *= 1099511628211ULL;
    }
    return value | (std::uint64_t{1} << 63U);
}

class CalendarPopupLayer final : public Panel {
public:
    explicit CalendarPopupLayer(StableId stable_id) : Panel(std::move(stable_id)) {
        set_paint_plane(PaintPlane::overlay);
        set_border_style(BorderStyle::none);
        set_background(Color::rgba(0, 0, 0, 0));
    }

    [[nodiscard]] Event<>& dismissed() noexcept { return dismissed_; }

    void on_pointer(PointerEvent& event) override {
        if (event.action == PointerAction::down &&
            event.button == PointerButton::primary) {
            dismissed_.emit();
            event.handled = true;
        }
    }

private:
    Event<> dismissed_;
};

class CalendarPopup final : public Control {
public:
    CalendarPopup(StableId stable_id, DateTimeValue selected,
                  DateTimeValue minimum, DateTimeValue maximum,
                  DateTimeFormatProvider provider, BasicControlStyle style)
        : Control(std::move(stable_id)), selected_(selected), minimum_(minimum),
          maximum_(maximum), provider_(std::move(provider)), style_(style),
          display_year_(selected.year), display_month_(selected.month) {
        set_paint_plane(PaintPlane::overlay);
        set_focusable(true);
    }

    [[nodiscard]] Event<DateTimeValue>& committed() noexcept { return committed_; }
    [[nodiscard]] Event<>& cancelled() noexcept { return cancelled_; }

    void on_paint(Painter& painter, Rect) override {
        const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                          committed_arranged_bounds().height};
        const double s = effective_text_scale();
        painter.fill_rect({3.0 * s, 3.0 * s,
                           std::max(0.0, bounds.width - 3.0 * s),
                           std::max(0.0, bounds.height - 3.0 * s)},
                          Color::rgba(30, 41, 50, 70));
        const Rect body{0.0, 0.0, std::max(0.0, bounds.width - 3.0 * s),
                        std::max(0.0, bounds.height - 3.0 * s)};
        painter.fill_rect(body, style_.paper);
        painter.stroke_rect({0.5, 0.5, std::max(0.0, body.width - 1.0),
                             std::max(0.0, body.height - 1.0)},
                            style_.dark_border, 1.0);
        painter.fill_rect({1.0, 1.0, std::max(0.0, body.width - 2.0), 38.0 * s},
                          Color::rgba(43, 79, 111));
        const std::string title = provider_.month_names[display_month_ - 1U] +
                                  " " + std::to_string(display_year_);
        painter.draw_text_utf8({74.0 * s, 25.0 * s}, title,
                               effective_font({FontRole::control, 12.0, 700, false}),
                               Color::rgba(255, 255, 255));
        const Color previous_color = can_change_month(-1)
            ? Color::rgba(235, 242, 247) : Color::rgba(111, 137, 157);
        const Color next_color = can_change_month(1)
            ? Color::rgba(235, 242, 247) : Color::rgba(111, 137, 157);
        painter.draw_line({18.0 * s, 20.0 * s}, {23.0 * s, 15.0 * s}, previous_color, 1.5);
        painter.draw_line({18.0 * s, 20.0 * s}, {23.0 * s, 25.0 * s}, previous_color, 1.5);
        painter.draw_line({body.width - 19.0 * s, 20.0 * s},
                          {body.width - 24.0 * s, 15.0 * s},
                          next_color, 1.5);
        painter.draw_line({body.width - 19.0 * s, 20.0 * s},
                          {body.width - 24.0 * s, 25.0 * s},
                          next_color, 1.5);

        constexpr std::array<std::string_view, 7> narrow_days{
            "S", "M", "T", "W", "T", "F", "S"};
        for (std::size_t column = 0U; column < 7U; ++column) {
            painter.draw_text_utf8({(13.0 + static_cast<double>(column) * 39.0) * s,
                                    58.0 * s},
                                   narrow_days[column],
                                   effective_font({FontRole::control, 10.0, 700, false}),
                                   style_.disabled_text);
        }
        const auto dates = cell_dates();
        for (std::size_t cell = 0U; cell < dates.size(); ++cell) {
            const std::size_t row = cell / 7U;
            const std::size_t column = cell % 7U;
            const Rect cell_bounds{(5.0 + static_cast<double>(column) * 39.0) * s,
                                   (66.0 + static_cast<double>(row) * 27.0) * s,
                                   37.0 * s, 25.0 * s};
            const DateTimeValue date = dates[cell];
            const bool in_range = civil_in_range(date, minimum_, maximum_);
            const bool in_month = date.month == display_month_ &&
                                  date.year == display_year_;
            const bool selected = date.year == selected_.year &&
                                  date.month == selected_.month &&
                                  date.day == selected_.day;
            if (selected) {
                painter.fill_rect(cell_bounds, style_.accent);
            } else if (hovered_cell_ == cell && in_range) {
                painter.fill_rect(cell_bounds, style_.accent_light);
            }
            painter.draw_text_utf8(
                {cell_bounds.x + (date.day < 10U ? 12.0 : 8.0) * s,
                 cell_bounds.y + 17.0 * s}, std::to_string(date.day),
                effective_font({FontRole::content, 11.0,
                 static_cast<std::uint16_t>(selected ? 700U : 400U), false}),
                selected ? style_.highlight
                         : !in_range || !in_month ? style_.disabled_text
                                                  : style_.text);
            if (focused_ && selected) {
                painter.stroke_rect({cell_bounds.x + 1.5, cell_bounds.y + 1.5,
                                     cell_bounds.width - 3.0,
                                     cell_bounds.height - 3.0},
                                    style_.highlight, 1.0);
            }
        }
        painter.draw_line({8.0 * s, 231.0 * s},
                          {body.width - 8.0 * s, 231.0 * s},
                          style_.border, 1.0);
        painter.draw_text_utf8({10.0 * s, 248.0 * s},
                               "Arrows move · Enter selects · Esc cancels",
                               effective_font({FontRole::content, 9.0, 400, false}),
                               style_.disabled_text);
    }

    void on_pointer(PointerEvent& event) override {
        if (!eligible_for_input()) return;
        const Rect bounds = absolute_bounds();
        const Point local{event.position.x - bounds.x, event.position.y - bounds.y};
        if (event.action == PointerAction::move) {
            const auto next = cell_at(local);
            if (next != hovered_cell_) {
                hovered_cell_ = next;
                invalidate(Dirty::paint);
            }
            return;
        }
        if (event.action == PointerAction::leave) {
            hovered_cell_.reset();
            invalidate(Dirty::paint);
            return;
        }
        if (event.button != PointerButton::primary) return;
        if (event.action == PointerAction::down) {
            const double s = effective_text_scale();
            if (local.y < 40.0 * s && local.x < 46.0 * s) {
                change_month(-1);
                pressed_cell_.reset();
                header_pressed_ = true;
                event.handled = true;
                return;
            }
            if (local.y < 40.0 * s && local.x > bounds.width - 49.0 * s) {
                change_month(1);
                pressed_cell_.reset();
                header_pressed_ = true;
                event.handled = true;
                return;
            }
            if (const auto cell = cell_at(local)) {
                const DateTimeValue date = cell_dates()[*cell];
                if (civil_in_range(date, minimum_, maximum_)) {
                    selected_ = std::clamp(date, minimum_, maximum_);
                    pressed_cell_ = cell;
                    invalidate(Dirty::paint | Dirty::semantics);
                    event.handled = true;
                }
            }
        } else if (event.action == PointerAction::up) {
            const auto cell = cell_at(local);
            const bool commit = pressed_cell_ && cell == pressed_cell_;
            const bool acknowledge = commit || pressed_cell_.has_value() || header_pressed_;
            pressed_cell_.reset();
            header_pressed_ = false;
            if (commit) committed_.emit(selected_);
            event.handled = acknowledge;
        }
    }

    void on_key(KeyEvent& event) override {
        if (!focused_ || event.action != KeyAction::down) return;
        if (event.physical_key == PhysicalKey::escape) {
            cancelled_.emit();
            event.handled = true;
            return;
        }
        if (event.physical_key == PhysicalKey::enter) {
            committed_.emit(selected_);
            event.handled = true;
            return;
        }
        if (event.physical_key == PhysicalKey::left) move_selection(-1);
        else if (event.physical_key == PhysicalKey::right) move_selection(1);
        else if (event.physical_key == PhysicalKey::up) move_selection(-7);
        else if (event.physical_key == PhysicalKey::down) move_selection(7);
        else if (event.physical_key == PhysicalKey::page_up) change_month(-1);
        else if (event.physical_key == PhysicalKey::page_down) change_month(1);
        else return;
        event.handled = true;
    }

    void on_focus_changed(bool focused) override {
        focused_ = focused;
        invalidate(Dirty::paint | Dirty::semantics);
    }

    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override {
        SemanticDescriptor descriptor;
        descriptor.role = SemanticRole::calendar;
        descriptor.name = provider_.month_names[display_month_ - 1U] + " " +
                          std::to_string(display_year_);
        descriptor.value = format_date_time(selected_, provider_.long_date_pattern,
                                            provider_);
        descriptor.actions = {SemanticAction::focus};
        descriptor.exposed = true;
        return descriptor;
    }

    [[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override {
        std::vector<SemanticNode> nodes;
        const auto dates = cell_dates();
        const Rect popup = absolute_bounds();
        const double s = effective_text_scale();
        nodes.reserve(dates.size() + 2U);
        for (const int direction : {-1, 1}) {
            SemanticNode node;
            node.stable_id = std::string(stable_id().value()) +
                (direction < 0 ? ".previous_month" : ".next_month");
            node.runtime_id = virtual_runtime_id(node.stable_id);
            node.role = SemanticRole::button;
            node.name = direction < 0 ? "Previous month" : "Next month";
            node.bounds = direction < 0
                ? Rect{popup.x + 4.0 * s, popup.y + 3.0 * s,
                       42.0 * s, 34.0 * s}
                : Rect{popup.x + popup.width - 49.0 * s, popup.y + 3.0 * s,
                       42.0 * s, 34.0 * s};
            node.states = SemanticState::visible | SemanticState::focusable;
            if (can_change_month(direction)) {
                node.states |= SemanticState::enabled;
                node.actions = {SemanticAction::press};
            }
            nodes.push_back(std::move(node));
        }
        for (std::size_t cell = 0U; cell < dates.size(); ++cell) {
            const DateTimeValue date = dates[cell];
            SemanticNode node;
            node.stable_id = date_stable_id(date);
            node.runtime_id = virtual_runtime_id(node.stable_id);
            node.role = SemanticRole::date_cell;
            node.name = format_date_time(date, provider_.long_date_pattern, provider_);
            node.value = iso_date(date);
            const std::size_t row = cell / 7U;
            const std::size_t column = cell % 7U;
            node.bounds = {popup.x +
                               (5.0 + static_cast<double>(column) * 39.0) * s,
                           popup.y +
                               (66.0 + static_cast<double>(row) * 27.0) * s,
                           37.0 * s, 25.0 * s};
            node.states |= SemanticState::visible | SemanticState::focusable;
            if (civil_in_range(date, minimum_, maximum_)) {
                node.states |= SemanticState::enabled;
                node.actions = {SemanticAction::select, SemanticAction::press};
            }
            if (date.year == selected_.year && date.month == selected_.month &&
                date.day == selected_.day) {
                node.states |= SemanticState::selected;
                if (focused_) node.states |= SemanticState::focused;
            }
            nodes.push_back(std::move(node));
        }
        return nodes;
    }

    bool on_semantic_child_action(std::string_view stable_id,
                                  SemanticAction action,
                                  std::string_view) override {
        const std::string prefix(this->stable_id().value());
        if (action == SemanticAction::press &&
            stable_id == prefix + ".previous_month") {
            if (!can_change_month(-1)) return false;
            change_month(-1);
            return true;
        }
        if (action == SemanticAction::press &&
            stable_id == prefix + ".next_month") {
            if (!can_change_month(1)) return false;
            change_month(1);
            return true;
        }
        if (action != SemanticAction::select && action != SemanticAction::press) {
            return false;
        }
        for (const DateTimeValue date : cell_dates()) {
            if (date_stable_id(date) != stable_id) continue;
            if (!civil_in_range(date, minimum_, maximum_)) return false;
            selected_ = std::clamp(date, minimum_, maximum_);
            invalidate(Dirty::paint | Dirty::semantics);
            if (action == SemanticAction::press) committed_.emit(selected_);
            return true;
        }
        return false;
    }

private:
    [[nodiscard]] std::array<DateTimeValue, 42> cell_dates() const noexcept {
        std::array<DateTimeValue, 42> result{};
        DateTimeValue first = selected_;
        first.year = display_year_;
        first.month = display_month_;
        first.day = 1U;
        const DateTimeValue start = add_days(first, -static_cast<int>(day_of_week(first)));
        for (std::size_t index = 0U; index < result.size(); ++index) {
            result[index] = add_days(start, static_cast<std::int64_t>(index));
        }
        return result;
    }

    [[nodiscard]] std::optional<std::size_t> cell_at(Point local) const noexcept {
        const double s = effective_text_scale();
        if (local.x < 5.0 * s || local.y < 66.0 * s ||
            local.x >= 278.0 * s || local.y >= 228.0 * s) {
            return {};
        }
        const std::size_t column = static_cast<std::size_t>(
            (local.x - 5.0 * s) / (39.0 * s));
        const std::size_t row = static_cast<std::size_t>(
            (local.y - 66.0 * s) / (27.0 * s));
        if (column >= 7U || row >= 6U) return {};
        return row * 7U + column;
    }

    void move_selection(int days) {
        const DateTimeValue candidate = add_days(selected_, days);
        if (!civil_in_range(candidate, minimum_, maximum_)) return;
        const DateTimeValue next = std::clamp(candidate, minimum_, maximum_);
        if (next == selected_) return;
        selected_ = next;
        display_year_ = next.year;
        display_month_ = next.month;
        invalidate(Dirty::paint | Dirty::semantics);
    }

    [[nodiscard]] DateTimeValue month_candidate(int direction) const noexcept {
        int month_index = display_year_ * 12 + static_cast<int>(display_month_) - 1 + direction;
        std::int32_t year = month_index >= 0 ? month_index / 12
            : static_cast<std::int32_t>((month_index - 11) / 12);
        int month = month_index - year * 12 + 1;
        DateTimeValue next = selected_;
        next.year = year;
        next.month = static_cast<std::uint8_t>(month);
        next.day = std::min(next.day, days_in_month(next.year, next.month));
        return std::clamp(next, minimum_, maximum_);
    }

    [[nodiscard]] bool can_change_month(int direction) const noexcept {
        const DateTimeValue next = month_candidate(direction);
        return next.year != display_year_ || next.month != display_month_;
    }

    void change_month(int direction) {
        const DateTimeValue next = month_candidate(direction);
        if (next.year == display_year_ && next.month == display_month_) return;
        selected_ = next;
        display_year_ = next.year;
        display_month_ = next.month;
        invalidate(Dirty::paint | Dirty::semantics);
    }

    [[nodiscard]] std::string date_stable_id(DateTimeValue date) const {
        return std::string(stable_id().value()) + ".day." + iso_date(date);
    }

    [[nodiscard]] static std::string iso_date(DateTimeValue date) {
        return padded(static_cast<unsigned>(date.year), 4U) + "-" +
               padded(date.month, 2U) + "-" + padded(date.day, 2U);
    }

    DateTimeValue selected_;
    DateTimeValue minimum_;
    DateTimeValue maximum_;
    DateTimeFormatProvider provider_;
    BasicControlStyle style_;
    std::int32_t display_year_;
    std::uint8_t display_month_;
    std::optional<std::size_t> hovered_cell_;
    std::optional<std::size_t> pressed_cell_;
    bool header_pressed_{};
    bool focused_{};
    Event<DateTimeValue> committed_;
    Event<> cancelled_;
};

[[nodiscard]] bool parse_iso_date(std::string_view text, DateTimeValue& value) {
    if (text.size() != 10U || text[4] != '-' || text[7] != '-') return false;
    int year{};
    unsigned month{};
    unsigned day{};
    const auto year_result = std::from_chars(text.data(), text.data() + 4U, year);
    const auto month_result = std::from_chars(text.data() + 5U, text.data() + 7U, month);
    const auto day_result = std::from_chars(text.data() + 8U, text.data() + 10U, day);
    if (year_result.ec != std::errc{} || month_result.ec != std::errc{} ||
        day_result.ec != std::errc{}) return false;
    value.year = year;
    value.month = static_cast<std::uint8_t>(month);
    value.day = static_cast<std::uint8_t>(day);
    return valid_date_time(value);
}

} // namespace

std::uint8_t days_in_month(std::int32_t year, std::uint8_t month) noexcept {
    if (month < 1U || month > 12U) return 0U;
    constexpr std::array<std::uint8_t, 12> days{
        31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};
    if (month != 2U) return days[month - 1U];
    const bool leap = year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
    return static_cast<std::uint8_t>(leap ? 29U : 28U);
}

bool valid_date_time(DateTimeValue value) noexcept {
    return value.month >= 1U && value.month <= 12U && value.day >= 1U &&
           value.day <= days_in_month(value.year, value.month) &&
           value.hour <= 23U && value.minute <= 59U && value.second <= 59U &&
           value.millisecond <= 999U;
}

DateTimeValue add_days(DateTimeValue value, std::int64_t days) noexcept {
    if (!valid_date_time(value)) return value;
    const std::int64_t serial = days_from_civil(value.year, value.month, value.day);
    if ((days > 0 && serial > std::numeric_limits<std::int64_t>::max() - days) ||
        (days < 0 && serial < std::numeric_limits<std::int64_t>::min() - days)) {
        return value;
    }
    return civil_from_days(serial + days, value);
}

std::uint8_t day_of_week(DateTimeValue value) noexcept {
    if (!valid_date_time(value)) return 0U;
    std::int64_t weekday = (days_from_civil(value.year, value.month, value.day) + 4) % 7;
    if (weekday < 0) weekday += 7;
    return static_cast<std::uint8_t>(weekday);
}

DateTimeFormatProvider DateTimeFormatProvider::english_united_states() {
    DateTimeFormatProvider provider;
    provider.month_names = {"January", "February", "March", "April", "May", "June",
                            "July", "August", "September", "October", "November",
                            "December"};
    provider.abbreviated_month_names = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    provider.day_names = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
                          "Friday", "Saturday"};
    provider.abbreviated_day_names = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    return provider;
}

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
    value_changed_.emit(value_);
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
    if (value_changes) value_changed_.emit(value_);
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
    checked_changed_.emit(checked_);
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
    const double x = std::clamp(picker.x, 0.0, std::max(0.0, client.width - width));
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
    drop_down_changed_.emit(true);
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
    if (changed) drop_down_changed_.emit(false);
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
    if (changed) drop_down_changed_.emit(false);
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
