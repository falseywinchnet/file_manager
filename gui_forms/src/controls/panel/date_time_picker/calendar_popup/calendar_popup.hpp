#pragma once

#include "../date_time_utilities.hpp"
#include "gui_forms/control.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace gui_forms {
using namespace date_time_detail;

class CalendarPopup final : public Control {
public:
    ~CalendarPopup() override;

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
        const std::array<DateTimeValue, 42> dates = cell_dates();
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
            const std::optional<std::size_t> next = cell_at(local);
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
            if (const std::optional<std::size_t> cell = cell_at(local)) {
                const DateTimeValue date = cell_dates()[*cell];
                if (civil_in_range(date, minimum_, maximum_)) {
                    selected_ = std::clamp(date, minimum_, maximum_);
                    pressed_cell_ = cell;
                    invalidate(Dirty::paint | Dirty::semantics);
                    event.handled = true;
                }
            }
        } else if (event.action == PointerAction::up) {
            const std::optional<std::size_t> cell = cell_at(local);
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
        const std::array<DateTimeValue, 42> dates = cell_dates();
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
        const std::string prefix((*this).stable_id().value());
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

} // namespace gui_forms
