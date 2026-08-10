#include "gui_forms/controls/panel/correspondence_view/correspondence_view.hpp"

#include "../collection_control_utilities.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <unordered_set>

namespace gui_forms {

using namespace collection_detail;
CorrespondenceView::CorrespondenceView(StableId stable_id)
    : Panel(std::move(stable_id)) {
    set_paint_plane(PaintPlane::control);
    set_background(style().paper);
    set_border_style(BorderStyle::sunken);
    set_focusable(true);
}

std::optional<std::size_t> CorrespondenceView::item_index(
    std::string_view id) const noexcept {
    const auto found = std::find_if(items_.begin(), items_.end(),
        [id](const CorrespondenceItem& item) { return item.stable_id == id; });
    return found == items_.end() ? std::optional<std::size_t>{}
                                : std::optional<std::size_t>{
                                      static_cast<std::size_t>(
                                          std::distance(items_.begin(), found))};
}

void CorrespondenceView::set_items(std::vector<CorrespondenceItem> items) {
    require_mutable();
    std::unordered_set<std::string> identities;
    for (const CorrespondenceItem& item : items) {
        validate_identity_text(item.stable_id, item.title);
        const std::string_view fields[]{item.secondary_text, item.metadata,
            item.excerpt, item.metric, item.information_heading,
            item.information_detail};
        if (std::any_of(std::begin(fields), std::end(fields),
                [](std::string_view value) { return !validate_utf8(value).valid(); }) ||
            std::any_of(item.information_tags.begin(), item.information_tags.end(),
                [](const std::string& value) {
                    return value.empty() || !validate_utf8(value).valid();
                }) ||
            std::any_of(item.emphasis_terms.begin(), item.emphasis_terms.end(),
                [](const std::string& value) {
                    return value.empty() || !validate_utf8(value).valid();
                }) ||
            !identities.insert(item.stable_id).second) {
            throw std::invalid_argument(
                "CorrespondenceView requires unique IDs and valid UTF-8 fields");
        }
    }

    const ScrollAnchor anchor = capture_anchor(first_visible_index());
    const CorrespondenceSelectionChange selection_change{
        selected_id_, item_index(selected_id_) ? selected_id_ : std::string{}};
    const CorrespondencePinChange pin_change{
        pinned_id_, item_index(pinned_id_) ? pinned_id_ : std::string{}};
    items_ = std::move(items);
    const auto retain = [this](std::string& id) {
        if (!id.empty() && !item_index(id)) id.clear();
    };
    retain(selected_id_);
    retain(focused_id_);
    retain(pinned_id_);
    hovered_id_.clear();
    hover_expanded_id_.clear();
    pending_hover_id_.clear();
    pressed_index_.reset();
    hover_timer_.disconnect();
    restore_anchor(anchor);
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
    if (selection_change.previous_id != selected_id_) {
        publish_change(selection_changed_, CorrespondenceSelectionChange{
            selection_change.previous_id, selected_id_});
    }
    if (pin_change.previous_id != pinned_id_) {
        publish_change(pin_changed_, CorrespondencePinChange{
            pin_change.previous_id, pinned_id_});
    }
}

void CorrespondenceView::set_selected_id(std::string_view stable_id) {
    require_mutable();
    if (!stable_id.empty() && !item_index(stable_id)) {
        throw std::out_of_range(
            "CorrespondenceView selection ID is not in the model");
    }
    const std::string next(stable_id);
    if (selected_id_ == next) return;
    CorrespondenceSelectionChange change{selected_id_, next};
    selected_id_ = next;
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(selection_changed_, change);
}

bool CorrespondenceView::expanded(std::string_view stable_id) const {
    const auto index = item_index(stable_id);
    if (!index) {
        throw std::out_of_range(
            "CorrespondenceView expansion ID is not in the model");
    }
    return expanded_index(*index);
}

void CorrespondenceView::emit_expansion_delta(
    std::string_view stable_id, bool before, bool after,
    CorrespondenceExpansionReason reason) {
    if (stable_id.empty() || before == after || !is_alive()) return;
    publish_change(expansion_changed_, CorrespondenceExpansionChange{
        std::string(stable_id), after, stable_id == pinned_id_, reason});
}

void CorrespondenceView::set_pinned_id(std::string_view stable_id) {
    require_mutable();
    const auto next_index = stable_id.empty() ? std::optional<std::size_t>{}
                                               : item_index(stable_id);
    if (!stable_id.empty() && !next_index) {
        throw std::out_of_range(
            "CorrespondenceView pinned ID is not in the model");
    }
    const std::string next(stable_id);
    if (pinned_id_ == next) return;
    const std::string previous = pinned_id_;
    const bool previous_before = !previous.empty() && expanded(previous);
    const bool next_before = !next.empty() && expanded(next);
    std::size_t preferred = first_visible_index();
    if (next_index) {
        const double top = row_top(*next_index);
        const double bottom = top + row_height(*next_index);
        if (bottom > scroll_offset_ &&
            top < scroll_offset_ + viewport_height()) {
            preferred = *next_index;
        }
    }
    const ScrollAnchor anchor = capture_anchor(preferred);
    pinned_id_ = next;
    restore_anchor(anchor);
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
    publish_change(pin_changed_,
                   CorrespondencePinChange{previous, pinned_id_});
    if (!previous.empty()) {
        emit_expansion_delta(previous, previous_before, expanded(previous),
                             CorrespondenceExpansionReason::pin);
    }
    if (!next.empty() && next != previous) {
        emit_expansion_delta(next, next_before, expanded(next),
                             CorrespondenceExpansionReason::pin);
    }
}

void CorrespondenceView::set_compact_height(double height) {
    require_mutable();
    if (!std::isfinite(height) || height < 24.0 || height > 192.0 ||
        height >= expanded_height_) {
        throw std::invalid_argument(
            "CorrespondenceView compact height must be finite, bounded, and below expanded height");
    }
    if (compact_height_ == height) return;
    const ScrollAnchor anchor = capture_anchor(first_visible_index());
    compact_height_ = height;
    restore_anchor(anchor);
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
}

void CorrespondenceView::set_expanded_height(double height) {
    require_mutable();
    if (!std::isfinite(height) || height <= compact_height_ || height > 512.0) {
        throw std::invalid_argument(
            "CorrespondenceView expanded height must be finite, bounded, and above compact height");
    }
    if (expanded_height_ == height) return;
    const ScrollAnchor anchor = capture_anchor(first_visible_index());
    expanded_height_ = height;
    restore_anchor(anchor);
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
}

void CorrespondenceView::set_status_rail_width(double width) {
    require_mutable();
    if (!std::isfinite(width) || width < 1.0 || width > 12.0) {
        throw std::invalid_argument(
            "CorrespondenceView status rail width must be finite and between 1 and 12");
    }
    if (status_rail_width_ == width) return;
    status_rail_width_ = width;
    invalidate(Dirty::paint);
}

void CorrespondenceView::set_hover_intent_delay(
    std::chrono::milliseconds delay) {
    require_mutable();
    if (delay < std::chrono::milliseconds(1) ||
        delay > std::chrono::seconds(5)) {
        throw std::invalid_argument(
            "CorrespondenceView hover intent must be between 1 ms and 5 s");
    }
    if (hover_intent_delay_ == delay) return;
    hover_intent_delay_ = delay;
    if (const auto index = item_index(hovered_id_)) schedule_hover_intent(*index);
}

void CorrespondenceView::set_font(FontSpec font) {
    require_mutable();
    if (!valid_font_spec(font)) {
        throw std::invalid_argument("CorrespondenceView font is invalid");
    }
    if (font_ == font) return;
    font_ = font;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

std::vector<std::size_t> CorrespondenceView::expanded_indices() const {
    std::vector<std::size_t> indices;
    indices.reserve(3U);
    const auto insert = [this, &indices](std::string_view id) {
        if (const auto index = item_index(id);
            index && std::find(indices.begin(), indices.end(), *index) ==
                         indices.end()) {
            indices.push_back(*index);
        }
    };
    insert(pinned_id_);
    insert(hover_expanded_id_);
    if (focused_) insert(focused_id_);
    std::sort(indices.begin(), indices.end());
    return indices;
}

bool CorrespondenceView::expanded_index(std::size_t index) const noexcept {
    if (index >= items_.size()) return false;
    const std::string& id = items_[index].stable_id;
    return id == pinned_id_ || id == hover_expanded_id_ ||
           (focused_ && id == focused_id_);
}

double CorrespondenceView::scaled_compact_height() const noexcept {
    return compact_height_ * effective_text_scale();
}

double CorrespondenceView::scaled_expanded_height() const noexcept {
    return expanded_height_ * effective_text_scale();
}

double CorrespondenceView::row_height(std::size_t index) const noexcept {
    return expanded_index(index) ? scaled_expanded_height()
                                 : scaled_compact_height();
}

double CorrespondenceView::row_top(std::size_t index) const noexcept {
    const double compact = scaled_compact_height();
    const double delta = scaled_expanded_height() - compact;
    const auto expanded = expanded_indices();
    const std::size_t before = static_cast<std::size_t>(std::count_if(
        expanded.begin(), expanded.end(),
        [index](std::size_t value) { return value < index; }));
    return static_cast<double>(index) * compact +
           static_cast<double>(before) * delta;
}

double CorrespondenceView::content_height() const noexcept {
    if (items_.empty()) return 0.0;
    return static_cast<double>(items_.size()) * scaled_compact_height() +
        static_cast<double>(expanded_indices().size()) *
            (scaled_expanded_height() - scaled_compact_height());
}

double CorrespondenceView::viewport_height() const noexcept {
    double height = local_bounds().height;
    if (height <= 4.0) height = requested_bounds().height;
    return std::max(0.0, height - 4.0);
}

double CorrespondenceView::maximum_scroll_offset() const noexcept {
    return std::max(0.0, content_height() - viewport_height());
}

void CorrespondenceView::clamp_scroll_offset() noexcept {
    scroll_offset_ = std::clamp(scroll_offset_, 0.0, maximum_scroll_offset());
}

void CorrespondenceView::set_scroll_offset(double offset) {
    require_mutable();
    if (!std::isfinite(offset) || offset < 0.0) {
        throw std::invalid_argument(
            "CorrespondenceView scroll offset must be finite and nonnegative");
    }
    const double next = std::clamp(offset, 0.0, maximum_scroll_offset());
    if (scroll_offset_ == next) return;
    scroll_offset_ = next;
    clear_hover_intent(true);
    invalidate(Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

std::size_t CorrespondenceView::first_visible_index() const noexcept {
    if (items_.empty()) return 0U;
    std::size_t low{};
    std::size_t high = items_.size();
    while (low < high) {
        const std::size_t middle = low + (high - low) / 2U;
        if (row_top(middle) + row_height(middle) <= scroll_offset_) low = middle + 1U;
        else high = middle;
    }
    return std::min(low, items_.size() - 1U);
}

std::pair<std::size_t, std::size_t>
CorrespondenceView::realized_range() const noexcept {
    if (items_.empty()) return {0U, 0U};
    const std::size_t first_visible = first_visible_index();
    const std::size_t first = first_visible == 0U ? 0U : first_visible - 1U;
    const double limit = scroll_offset_ + viewport_height();
    std::size_t end = first_visible;
    while (end < items_.size() && row_top(end) < limit) ++end;
    end = std::min(items_.size(), end + 1U);
    return {first, end};
}

std::size_t CorrespondenceView::realized_count() const noexcept {
    const auto [first, end] = realized_range();
    return end - first;
}

Rect CorrespondenceView::item_bounds(std::size_t index) const noexcept {
    if (index >= items_.size()) return {};
    const double width = std::max(0.0, local_bounds().width - 4.0);
    return {2.0, 2.0 + row_top(index) - scroll_offset_, width,
            row_height(index)};
}

std::optional<Rect> CorrespondenceView::item_bounds(
    std::string_view stable_id) const noexcept {
    const auto index = item_index(stable_id);
    return index ? std::optional<Rect>{item_bounds(*index)}
                 : std::optional<Rect>{};
}

std::optional<std::size_t> CorrespondenceView::index_at(
    Point absolute) const noexcept {
    const Rect bounds = absolute_bounds();
    if (!bounds.contains(absolute)) return {};
    const double content_y = absolute.y - bounds.y - 2.0 + scroll_offset_;
    if (content_y < 0.0 || items_.empty()) return {};
    std::size_t low{};
    std::size_t high = items_.size();
    while (low < high) {
        const std::size_t middle = low + (high - low) / 2U;
        if (row_top(middle) + row_height(middle) <= content_y) low = middle + 1U;
        else high = middle;
    }
    if (low >= items_.size() || content_y < row_top(low)) return {};
    return low;
}

CorrespondenceView::ScrollAnchor CorrespondenceView::capture_anchor(
    std::size_t preferred) const noexcept {
    if (items_.empty()) return {};
    const std::size_t index = std::min(preferred, items_.size() - 1U);
    return {items_[index].stable_id, index,
            row_top(index) - scroll_offset_, true};
}

void CorrespondenceView::restore_anchor(ScrollAnchor anchor) {
    if (!anchor.valid || items_.empty()) {
        clamp_scroll_offset();
        return;
    }
    const std::size_t index = item_index(anchor.stable_id).value_or(
        std::min(anchor.index, items_.size() - 1U));
    scroll_offset_ = row_top(index) - anchor.viewport_y;
    clamp_scroll_offset();
}

void CorrespondenceView::ensure_visible(std::size_t index) {
    if (index >= items_.size()) return;
    const double top = row_top(index);
    const double bottom = top + row_height(index);
    const double height = viewport_height();
    if (top < scroll_offset_) scroll_offset_ = top;
    else if (bottom > scroll_offset_ + height) {
        scroll_offset_ = std::max(0.0, bottom - height);
    }
    clamp_scroll_offset();
}

void CorrespondenceView::focus_index(
    std::size_t index, CorrespondenceExpansionReason reason) {
    if (index >= items_.size() || !items_[index].enabled) return;
    const std::string previous = focused_id_;
    const bool previous_before = !previous.empty() && expanded(previous);
    const bool next_before = expanded(items_[index].stable_id);
    const ScrollAnchor anchor = capture_anchor(index);
    focused_id_ = items_[index].stable_id;
    restore_anchor(anchor);
    ensure_visible(index);
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
    if (!previous.empty()) {
        emit_expansion_delta(previous, previous_before, expanded(previous), reason);
    }
    if (focused_id_ != previous) {
        emit_expansion_delta(focused_id_, next_before, expanded(focused_id_), reason);
    }
}

void CorrespondenceView::schedule_hover_intent(std::size_t index) {
    hover_timer_.disconnect();
    if (index >= items_.size() || !items_[index].enabled || !window()) return;
    pending_hover_id_ = items_[index].stable_id;
    const std::string intended = pending_hover_id_;
    const auto self = std::static_pointer_cast<CorrespondenceView>(
        shared_from_this());
    std::weak_ptr<CorrespondenceView> weak = self;
    hover_timer_ = window()->schedule_ui_timer(
        *this, std::chrono::hours(24),
        FrameClock::now() + hover_intent_delay_,
        [weak, intended](FrameTime) {
            if (const auto view = weak.lock()) {
                view->hover_timer_.disconnect();
                view->apply_hover_expansion(intended);
            }
        });
}

void CorrespondenceView::apply_hover_expansion(std::string stable_id) {
    if (hovered_id_ != stable_id || pending_hover_id_ != stable_id ||
        !item_index(stable_id)) return;
    const std::string previous = hover_expanded_id_;
    const bool previous_before = !previous.empty() && expanded(previous);
    const bool next_before = expanded(stable_id);
    const ScrollAnchor anchor = capture_anchor(*item_index(stable_id));
    hover_expanded_id_ = std::move(stable_id);
    pending_hover_id_.clear();
    restore_anchor(anchor);
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
    if (!previous.empty()) {
        emit_expansion_delta(previous, previous_before, expanded(previous),
                             CorrespondenceExpansionReason::hover_intent);
    }
    emit_expansion_delta(hover_expanded_id_, next_before,
                         expanded(hover_expanded_id_),
                         CorrespondenceExpansionReason::hover_intent);
}

void CorrespondenceView::clear_hover_intent(bool clear_expansion) {
    hover_timer_.disconnect();
    pending_hover_id_.clear();
    if (!clear_expansion || hover_expanded_id_.empty()) return;
    const std::string previous = hover_expanded_id_;
    const bool before = expanded(previous);
    const std::size_t anchor_index = item_index(hovered_id_).value_or(
        first_visible_index());
    const ScrollAnchor anchor = capture_anchor(anchor_index);
    hover_expanded_id_.clear();
    restore_anchor(anchor);
    emit_expansion_delta(previous, before, expanded(previous),
                         CorrespondenceExpansionReason::hover_intent);
}

void CorrespondenceView::set_hovered_index(
    std::optional<std::size_t> index) {
    const std::string next = index && *index < items_.size()
        ? items_[*index].stable_id : std::string{};
    if (hovered_id_ == next) return;
    clear_hover_intent(true);
    hovered_id_ = next;
    if (index) schedule_hover_intent(*index);
    invalidate(Dirty::paint | Dirty::semantics);
}

void CorrespondenceView::arrange(Rect final_bounds) {
    Panel::arrange(final_bounds);
    clamp_scroll_offset();
    if (const auto focused = item_index(focused_id_)) ensure_visible(*focused);
}

void CorrespondenceView::paint_glyph(Painter& painter, Rect b,
                                     ObjectGlyph glyph,
                                     bool item_enabled) const {
    const Color border = item_enabled ? style().dark_border
                                      : style().disabled_text;
    const Color base = !item_enabled ? style().face
        : glyph == ObjectGlyph::folder ? Color::rgba(79, 137, 196)
        : glyph == ObjectGlyph::image ? Color::rgba(83, 154, 119)
        : glyph == ObjectGlyph::audio ? Color::rgba(151, 95, 173)
        : glyph == ObjectGlyph::archive ? Color::rgba(190, 135, 61)
        : glyph == ObjectGlyph::code ? Color::rgba(67, 132, 135)
        : Color::rgba(107, 126, 158);
    if (glyph == ObjectGlyph::folder) {
        painter.fill_rect({b.x + 2.0, b.y + 8.0, b.width - 4.0,
                           b.height - 10.0}, base);
        painter.fill_rect({b.x + 6.0, b.y + 4.0, b.width * .44, 7.0},
                          with_alpha(style().highlight, 190));
        painter.stroke_rect({b.x + 2.5, b.y + 8.5, b.width - 5.0,
                             b.height - 11.0}, border, 1.0);
        return;
    }
    painter.fill_rect({b.x + 5.0, b.y + 1.0, b.width - 10.0, b.height - 2.0},
                      style().paper);
    painter.fill_rect({b.x + 5.0, b.y + 1.0, b.width - 10.0, 6.0}, base);
    painter.stroke_rect({b.x + 5.5, b.y + 1.5, b.width - 11.0,
                         b.height - 3.0}, border, 1.0);
    if (glyph == ObjectGlyph::image) {
        painter.fill_rect({b.x + 9.0, b.y + 11.0, b.width - 18.0,
                           b.height - 17.0}, with_alpha(base, 120));
    } else {
        painter.draw_line({b.x + 9.0, b.y + 13.0},
                          {b.x + b.width - 9.0, b.y + 13.0}, base, 1.0);
        painter.draw_line({b.x + 9.0, b.y + 18.0},
                          {b.x + b.width - 12.0, b.y + 18.0}, base, 1.0);
    }
}

void CorrespondenceView::on_paint(Painter& painter, Rect damage) {
    Panel::on_paint(painter, damage);
    const Rect bounds = local_bounds();
    const FontSpec title_font = effective_font(font_);
    const FontSpec secondary_font = effective_font(
        {font_.role, std::max(8.0, font_.size - 2.0), 400, false});
    const FontSpec metric_font = effective_font(
        {FontRole::content, std::max(12.0, font_.size + 5.0), 700, false});
    const FontSpec evidence_font = effective_font(
        {FontRole::content, std::max(8.0, font_.size - 1.5), 400, false});
    const FontSpec evidence_bold = effective_font(
        {FontRole::content, std::max(8.0, font_.size - 1.5), 700, false});
    painter.save();
    painter.clip_rect({2.0, 2.0, std::max(0.0, bounds.width - 4.0),
                       std::max(0.0, bounds.height - 4.0)});
    const auto [first, end] = realized_range();
    for (std::size_t index = first; index < end; ++index) {
        const CorrespondenceItem& item = items_[index];
        const Rect row = item_bounds(index);
        const double visible_height = std::max(0.0, row.height - 5.0);
        const bool selected = item.stable_id == selected_id_;
        const bool active = focused_ && item.stable_id == focused_id_;
        const bool pinned = item.stable_id == pinned_id_;
        const bool is_expanded = expanded_index(index);
        painter.save();
        painter.clip_rect({row.x, row.y, row.width, visible_height});
        painter.fill_rect({row.x, row.y, row.width, visible_height},
                          selected ? style().accent_light :
                          item.stable_id == hovered_id_ ? style().face_light :
                          style().paper);
        painter.stroke_rect({row.x + .5, row.y + .5,
                             std::max(0.0, row.width - 1.0),
                             std::max(0.0, visible_height - 1.0)},
                            with_alpha(style().border, 105), 1.0);
        const Color rail = item.unavailable ? style().disabled_text
                           : item.stale ? Color::rgba(151, 112, 64)
                           : pinned ? style().accent
                                    : with_alpha(style().dark_border, 150);
        painter.fill_rect({row.x + 1.0, row.y + 1.0, status_rail_width_,
                           std::max(0.0, visible_height - 2.0)}, rail);
        paint_glyph(painter, {row.x + 10.0, row.y + 7.0, 30.0, 30.0},
                    item.glyph, item.enabled);
        const Color text = item.enabled ? style().text : style().disabled_text;
        painter.draw_text_utf8({row.x + 48.0, row.y + 17.0}, item.title,
                               {title_font.role, title_font.size, 700,
                                title_font.italic, title_font.letter_spacing}, text);
        painter.draw_text_utf8({row.x + 48.0, row.y + 34.0},
                               item.secondary_text, secondary_font,
                               item.unavailable ? style().disabled_text
                                                : style().dark_border);
        const double metric_x = row.x + std::max(48.0, row.width - 64.0);
        painter.draw_text_utf8({metric_x, row.y + 27.0}, item.metric,
                               metric_font, text);
        if (pinned || item.unavailable || item.stale) {
            const std::string state = pinned ? "PINNED"
                : item.unavailable ? "OFFLINE" : "STALE";
            painter.draw_text_utf8(
                {row.x + std::max(48.0, row.width - 132.0), row.y + 12.0},
                state, evidence_bold, rail);
        }
        if (is_expanded) {
            const double scale = effective_text_scale();
            const double compact = scaled_compact_height();
            const double body_top = row.y + compact - 5.0 * scale;
            painter.draw_line({row.x + 48.0, body_top},
                              {row.x + row.width - 8.0, body_top},
                              with_alpha(style().border, 120), 1.0);
            const bool wide = row.width >= 650.0 * scale;
            const double lane_width = wide ? std::min(190.0 * scale,
                std::max(150.0 * scale, row.width * .25)) : 0.0;
            const double evidence_x = row.x + 52.0 * scale;
            const double evidence_width = std::max(40.0,
                row.width - 62.0 * scale - lane_width);
            painter.draw_text_utf8({evidence_x, body_top + 16.0 * scale},
                                   item.metadata, evidence_font, text);
            const Rect excerpt_box{evidence_x, body_top + 22.0 * scale,
                evidence_width, std::max(20.0, 43.0 * scale)};
            painter.fill_rect(excerpt_box,
                              item.unavailable ? style().face
                                               : Color::rgba(251, 248, 237));
            painter.stroke_rect({excerpt_box.x + .5, excerpt_box.y + .5,
                                 std::max(0.0, excerpt_box.width - 1.0),
                                 std::max(0.0, excerpt_box.height - 1.0)},
                                with_alpha(style().border, 90), 1.0);
            painter.draw_text_utf8(
                {excerpt_box.x + 6.0, excerpt_box.y + 15.0 * scale},
                "MATCHED EXCERPT", evidence_bold, style().dark_border);
            draw_emphasized_text(
                painter,
                {excerpt_box.x + 6.0, excerpt_box.y + 31.0 * scale},
                item.excerpt, item.emphasis_terms, evidence_font, text,
                with_alpha(style().accent_light, 220));
            std::string tags;
            for (const std::string& tag : item.information_tags) {
                if (!tags.empty()) tags += "  ·  ";
                tags += tag;
            }
            if (wide) {
                const double lane_x = row.x + row.width - lane_width;
                painter.draw_line({lane_x, body_top + 7.0 * scale},
                                  {lane_x, row.y + visible_height - 7.0 * scale},
                                  with_alpha(style().border, 125), 1.0);
                painter.draw_text_utf8(
                    {lane_x + 9.0 * scale, body_top + 18.0 * scale},
                    item.information_heading, evidence_bold, text);
                painter.draw_text_utf8(
                    {lane_x + 9.0 * scale, body_top + 36.0 * scale}, tags,
                    evidence_font, style().dark_border);
                painter.draw_text_utf8(
                    {lane_x + 9.0 * scale, body_top + 54.0 * scale},
                    item.information_detail, evidence_font,
                    style().disabled_text);
            } else {
                painter.draw_text_utf8(
                    {evidence_x, row.y + visible_height - 7.0 * scale},
                    item.information_heading + " · " + tags,
                    evidence_font, style().dark_border);
            }
        }
        if (active) {
            painter.stroke_rect({row.x + 6.5, row.y + 2.5,
                                 std::max(0.0, row.width - 9.0),
                                 std::max(0.0, visible_height - 5.0)},
                                style().accent, 1.0);
        }
        painter.restore();
    }
    painter.restore();
}

void CorrespondenceView::on_pointer(PointerEvent& event) {
    if (!eligible_for_input()) return;
    if (event.action == PointerAction::move ||
        event.action == PointerAction::enter) {
        set_hovered_index(index_at(event.position));
        return;
    }
    if (event.action == PointerAction::leave) {
        set_hovered_index({});
        return;
    }
    if (event.action == PointerAction::wheel && event.wheel_delta.y != 0.0) {
        const double direction = event.wheel_delta.y > 0.0 ? -1.0 : 1.0;
        set_scroll_offset(std::clamp(
            scroll_offset_ + direction * scaled_compact_height() * 2.0,
            0.0, maximum_scroll_offset()));
        event.handled = true;
        return;
    }
    if (event.action == PointerAction::down &&
        (event.button == PointerButton::primary ||
         event.button == PointerButton::secondary)) {
        pressed_index_ = index_at(event.position);
        pressed_button_ = event.button;
        pressed_click_count_ = event.click_count;
        if (pressed_index_) {
            if (window()) {
                static_cast<void>(window()->request_focus(shared_from_this()));
            }
            set_selected_id(items_[*pressed_index_].stable_id);
            focus_index(*pressed_index_,
                        CorrespondenceExpansionReason::keyboard_focus);
        }
        event.handled = true;
        return;
    }
    if (event.action == PointerAction::up &&
        event.button == pressed_button_ && pressed_button_ != PointerButton::none) {
        const auto released = index_at(event.position);
        if (released && pressed_index_ == released) {
            const CorrespondenceItem& item = items_[*released];
            if (pressed_button_ == PointerButton::secondary) {
                context_requested_.emit({item.stable_id, event.position});
            } else if (pressed_click_count_ >= 2U) {
                item_activated_.emit(item.stable_id);
            } else {
                set_pinned_id(item.stable_id == pinned_id_ ? std::string_view{}
                                                           : item.stable_id);
            }
        }
        pressed_index_.reset();
        pressed_button_ = PointerButton::none;
        event.handled = true;
    }
}

void CorrespondenceView::on_key(KeyEvent& event) {
    if (!focused_ || !enabled() || event.action != KeyAction::down ||
        items_.empty()) return;
    std::size_t index = item_index(focused_id_)
        .value_or(item_index(selected_id_).value_or(0U));
    if (event.physical_key == PhysicalKey::enter) {
        if (items_[index].enabled) item_activated_.emit(items_[index].stable_id);
        event.handled = true;
        return;
    }
    if (event.physical_key == PhysicalKey::space) {
        set_pinned_id(items_[index].stable_id == pinned_id_
            ? std::string_view{} : std::string_view(items_[index].stable_id));
        event.handled = true;
        return;
    }
    if (event.physical_key == PhysicalKey::up) {
        if (index > 0U) --index;
    } else if (event.physical_key == PhysicalKey::down) {
        index = std::min(index + 1U, items_.size() - 1U);
    } else if (event.physical_key == PhysicalKey::home) {
        index = 0U;
    } else if (event.physical_key == PhysicalKey::end) {
        index = items_.size() - 1U;
    } else if (event.physical_key == PhysicalKey::page_up) {
        const std::size_t amount = std::max<std::size_t>(
            1U, static_cast<std::size_t>(viewport_height() /
                std::max(1.0, scaled_compact_height())));
        index = index > amount ? index - amount : 0U;
    } else if (event.physical_key == PhysicalKey::page_down) {
        const std::size_t amount = std::max<std::size_t>(
            1U, static_cast<std::size_t>(viewport_height() /
                std::max(1.0, scaled_compact_height())));
        index = std::min(index + amount, items_.size() - 1U);
    } else {
        return;
    }
    focus_index(index, CorrespondenceExpansionReason::keyboard_focus);
    set_selected_id(items_[index].stable_id);
    event.handled = true;
}

void CorrespondenceView::on_focus_changed(bool focused) {
    const std::string active = focused_id_.empty() && !items_.empty()
        ? (selected_id_.empty() ? items_.front().stable_id : selected_id_)
        : focused_id_;
    const bool before = !active.empty() && expanded(active);
    const ScrollAnchor anchor = capture_anchor(
        item_index(active).value_or(first_visible_index()));
    focused_ = focused;
    if (focused_ && focused_id_.empty()) focused_id_ = active;
    restore_anchor(anchor);
    if (focused_) {
        if (const auto index = item_index(focused_id_)) ensure_visible(*index);
    }
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
    if (!active.empty()) {
        emit_expansion_delta(active, before, expanded(active),
                             CorrespondenceExpansionReason::keyboard_focus);
    }
}

SemanticDescriptor CorrespondenceView::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::list;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    descriptor.value = selected_id_;
    descriptor.numeric_value = scroll_offset_;
    descriptor.minimum_value = 0.0;
    descriptor.maximum_value = maximum_scroll_offset();
    descriptor.actions = {SemanticAction::focus, SemanticAction::increment,
                          SemanticAction::decrement};
    descriptor.exposed = true;
    return descriptor;
}

std::vector<SemanticNode>
CorrespondenceView::semantic_virtual_children() const {
    std::vector<SemanticNode> nodes;
    const auto [first, end] = realized_range();
    nodes.reserve(end - first);
    const Rect absolute = absolute_bounds();
    for (std::size_t index = first; index < end; ++index) {
        const CorrespondenceItem& item = items_[index];
        const Rect local = item_bounds(index);
        SemanticNode node;
        node.stable_id = item.stable_id;
        node.runtime_id = virtual_runtime_id(node.stable_id);
        node.role = SemanticRole::list_item;
        node.name = item.title;
        node.value = item.metric;
        node.description = item.secondary_text;
        if (item.unavailable) node.description += " · source unavailable";
        if (item.stale) node.description += " · stale metadata";
        if (item.stable_id == pinned_id_) node.description += " · pinned";
        node.bounds = {absolute.x + local.x, absolute.y + local.y,
                       local.width, local.height};
        node.states = SemanticState::visible | SemanticState::focusable;
        if (effectively_enabled() && item.enabled) {
            node.states |= SemanticState::enabled;
        }
        if (item.stable_id == selected_id_) node.states |= SemanticState::selected;
        if (focused_ && item.stable_id == focused_id_) {
            node.states |= SemanticState::focused;
        }
        const bool is_expanded = expanded_index(index);
        if (is_expanded) node.states |= SemanticState::expanded;
        node.actions = {SemanticAction::focus, SemanticAction::select,
                        SemanticAction::press, SemanticAction::show_menu,
                        is_expanded ? SemanticAction::collapse
                                    : SemanticAction::expand};
        SemanticNode activate;
        activate.stable_id = node.stable_id + ".activate";
        activate.runtime_id = virtual_runtime_id(activate.stable_id);
        activate.role = SemanticRole::button;
        activate.name = "Open " + item.title;
        activate.description = item.unavailable
            ? "Source unavailable" : "Activate this correspondence";
        activate.bounds = node.bounds;
        activate.states = SemanticState::visible | SemanticState::focusable;
        if (effectively_enabled() && item.enabled) {
            activate.states |= SemanticState::enabled;
        }
        activate.actions = {SemanticAction::press};
        node.children.push_back(std::move(activate));
        if (is_expanded) {
            const auto text_child = [&node](std::string suffix,
                                            std::string name,
                                            std::string value,
                                            SemanticRole role =
                                                SemanticRole::static_text) {
                SemanticNode child;
                child.stable_id = node.stable_id + std::move(suffix);
                child.runtime_id = virtual_runtime_id(child.stable_id);
                child.role = role;
                child.name = std::move(name);
                child.value = std::move(value);
                child.bounds = node.bounds;
                child.states = SemanticState::visible;
                node.children.push_back(std::move(child));
            };
            text_child(".percentage", "Match evidence", item.metric);
            text_child(".metadata", "Why matched", item.metadata);
            text_child(".excerpt", "Matched excerpt", item.excerpt);
            std::string information = item.information_detail;
            for (const std::string& tag : item.information_tags) {
                if (!information.empty()) information += " · ";
                information += tag;
            }
            text_child(".plugins", item.information_heading,
                       std::move(information), SemanticRole::group);
        }
        nodes.push_back(std::move(node));
    }
    return nodes;
}

bool CorrespondenceView::on_semantic_child_action(
    std::string_view stable_id, SemanticAction action, std::string_view) {
    constexpr std::string_view activate_suffix = ".activate";
    if (stable_id.ends_with(activate_suffix) &&
        action == SemanticAction::press) {
        const std::string_view item_id = stable_id.substr(
            0U, stable_id.size() - activate_suffix.size());
        const auto activate_index = item_index(item_id);
        if (!activate_index) return false;
        if (window()) {
            static_cast<void>(window()->request_focus(shared_from_this()));
        }
        focus_index(*activate_index,
                    CorrespondenceExpansionReason::keyboard_focus);
        set_selected_id(item_id);
        if (items_[*activate_index].enabled) {
            item_activated_.emit(items_[*activate_index].stable_id);
        }
        return true;
    }
    const auto index = item_index(stable_id);
    if (!index || (action != SemanticAction::focus &&
                   action != SemanticAction::select &&
                   action != SemanticAction::press &&
                   action != SemanticAction::show_menu &&
                   action != SemanticAction::expand &&
                   action != SemanticAction::collapse)) return false;
    if (window()) static_cast<void>(window()->request_focus(shared_from_this()));
    focus_index(*index, CorrespondenceExpansionReason::keyboard_focus);
    if (action == SemanticAction::select) {
        set_selected_id(stable_id);
    } else if (action == SemanticAction::press) {
        set_selected_id(stable_id);
        item_activated_.emit(items_[*index].stable_id);
    } else if (action == SemanticAction::show_menu) {
        set_selected_id(stable_id);
        const Rect local = item_bounds(*index);
        const Rect absolute = absolute_bounds();
        context_requested_.emit({std::string(stable_id),
            {absolute.x + local.x + local.width * .5,
             absolute.y + local.y + std::min(local.height, viewport_height()) * .5}});
    } else if (action == SemanticAction::expand) {
        set_pinned_id(stable_id);
    } else if (action == SemanticAction::collapse) {
        if (pinned_id_ == stable_id) set_pinned_id({});
        if (hover_expanded_id_ == stable_id) clear_hover_intent(true);
    }
    return true;
}

void CorrespondenceView::on_detached_from_window() noexcept {
    hover_timer_.disconnect();
    pending_hover_id_.clear();
    Panel::on_detached_from_window();
}


} // namespace gui_forms
