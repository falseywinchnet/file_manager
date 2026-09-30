#include "gui_forms/controls/panel/breadcrumb_trail/breadcrumb_trail.hpp"

#include "../collection_control_utilities.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace gui_forms {

using namespace collection_detail;

namespace {
// Non-owning spans are used only during this call; indices address widths.
double breadcrumb_occupied_width(const std::span<const std::size_t> indices,
                                  const std::span<const double> widths,
                                  const bool overflow, const double overlap) {
    double total = 0.0;
    bool first = true;
    for (const std::size_t index : indices) {
        const double shared_edge = first ? 0.0 : overlap;
        total += widths[index] - shared_edge;
        first = false;
    }
    if (overflow) total += 42.0 - (first ? 0.0 : overlap);
    return total;
}
} // namespace

BreadcrumbTrail::BreadcrumbTrail(StableId stable_id,
                                 std::string editor_stable_id)
    : Panel(std::move(stable_id)),
      editor_stable_id_(std::move(editor_stable_id)) {
    const std::string prefix((*this).stable_id().value());
    if (editor_stable_id_.empty()) editor_stable_id_ = prefix + ".editor";
    overflow_stable_id_ = prefix + ".overflow";
    edit_stable_id_ = prefix + ".edit";
    set_paint_plane(PaintPlane::control);
    set_background(style().paper);
    set_border_style(BorderStyle::sunken);
    set_focusable(true);
}

void BreadcrumbTrail::initialize_control_tree() {
    if (editor_) return;
    editor_ = make_control<TextBox>(StableId(editor_stable_id_));
    (*editor_).set_border_style(BorderStyle::none);
    (*editor_).set_placeholder_text("Enter an exact path");
    (*editor_).set_accessible_name("Exact location path");
    (*editor_).set_visible(false);
    (*editor_).set_font(font_);
    add_child(editor_);
    const std::weak_ptr<BreadcrumbTrail> weak =
        std::static_pointer_cast<BreadcrumbTrail>(shared_from_this());
    // The trail owns the editor and tokens. Callbacks observe the trail weakly;
    // locking keeps it alive for one invocation and expired targets do nothing.
    editor_commit_ = (*editor_).committed().subscribe(
        *this, EditorCommitCallback{weak});
    editor_cancel_ = (*editor_).cancelled().subscribe(
        *this, EditorCancelCallback{weak});
}

void BreadcrumbTrail::set_font(FontSpec font) {
    require_mutable();
    if (!valid_font_spec(font)) throw std::invalid_argument("BreadcrumbTrail font is invalid");
    if (font_ == font) return;
    font_ = font;
    if (editor_) (*editor_).set_font(font);
    rebuild_layout(committed_arranged_bounds().width, committed_arranged_bounds().height);
    normalize_active();
    invalidate(Dirty::measure | Dirty::layout | Dirty::paint | Dirty::hit_test |
               Dirty::semantics | Dirty::accessibility);
}

void BreadcrumbTrail::set_appearance(BreadcrumbAppearance appearance) {
    require_mutable();
    if (appearance != BreadcrumbAppearance::plain && appearance != BreadcrumbAppearance::raised) {
        throw std::invalid_argument("BreadcrumbTrail appearance is invalid");
    }
    if (appearance_ == appearance) return;
    appearance_ = appearance;
    rebuild_layout(committed_arranged_bounds().width, committed_arranged_bounds().height);
    normalize_active();
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

void BreadcrumbTrail::EditorCommitCallback::operator()(
    const std::string& text) const {
    if (const std::shared_ptr<BreadcrumbTrail> trail = target.lock()) {
        (*trail).editor_committed(text);
    }
}

void BreadcrumbTrail::EditorCancelCallback::operator()() const {
    if (const std::shared_ptr<BreadcrumbTrail> trail = target.lock()) {
        (*trail).editor_cancelled();
    }
}

void BreadcrumbTrail::editor_committed(std::string text) {
    publish_change(edit_committed_, text);
}

void BreadcrumbTrail::editor_cancelled() {
    set_editing(false);
    if (window()) {
        static_cast<void>((*window()).request_focus(shared_from_this()));
    }
    publish_change(edit_cancelled_);
}

void BreadcrumbTrail::set_segments(std::vector<BreadcrumbSegment> segments) {
    require_mutable();
    std::unordered_set<std::string> identities{};
    for (const BreadcrumbSegment& segment : segments) {
        validate_identity_text(segment.stable_id, segment.text);
        const std::pair<std::unordered_set<std::string>::iterator, bool> insertion =
            identities.insert(segment.stable_id);
        if (!validate_utf8(segment.description).valid() ||
            segment.description.size() > 4096U ||
            segment.stable_id == overflow_stable_id_ ||
            segment.stable_id == edit_stable_id_ ||
            !insertion.second) {
            throw std::invalid_argument(
                "BreadcrumbTrail requires unique segment IDs, valid text, and bounded descriptions");
        }
    }
    const std::string retained_active = active_id_;
    segments_ = std::move(segments);
    active_id_ = segment_index(retained_active) ? retained_active : std::string{};
    rebuild_layout(committed_arranged_bounds().width,
                   committed_arranged_bounds().height);
    normalize_active();
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test |
               Dirty::semantics | Dirty::accessibility);
}

std::optional<std::size_t> BreadcrumbTrail::segment_index(
    std::string_view stable_id) const noexcept {
    const std::size_t count = segments_.size();
    for (std::size_t index = 0U; index < count; ++index) {
        if (segments_[index].stable_id == stable_id) {
            const std::optional<std::size_t> result(index);
            return result;
        }
    }
    return {};
}

void BreadcrumbTrail::set_active_id(std::string_view stable_id) {
    require_mutable();
    if (!stable_id.empty() && stable_id != overflow_stable_id_ &&
        stable_id != edit_stable_id_ && !segment_index(stable_id)) {
        throw std::out_of_range("BreadcrumbTrail active ID is not in its model");
    }
    if (active_id_ == stable_id) return;
    active_id_ = stable_id;
    normalize_active();
    invalidate(Dirty::paint | Dirty::semantics | Dirty::accessibility);
}

void BreadcrumbTrail::set_editing(bool editing) {
    require_mutable();
    if (editing_ == editing) return;
    editing_ = editing;
    if (!editing_) tab_completion_available_ = false;
    if (editor_) (*editor_).set_visible(editing_);
    invalidate(Dirty::paint | Dirty::hit_test | Dirty::semantics |
               Dirty::accessibility);
}

void BreadcrumbTrail::set_tab_completion_available(bool available) {
    require_mutable();
    available = available && editing_;
    if (tab_completion_available_ == available) return;
    tab_completion_available_ = available;
    invalidate(Dirty::semantics | Dirty::accessibility);
}

void BreadcrumbTrail::begin_edit(std::string text, bool select_all) {
    require_mutable();
    if (!editor_) {
        throw std::logic_error(
            "BreadcrumbTrail must be created through make_control before editing");
    }
    (*editor_).set_text(std::move(text));
    set_editing(true);
    if (select_all) (*editor_).select_all();
    if (window()) static_cast<void>((*window()).request_focus(editor_));
    publish_change(edit_started_, std::string((*editor_).text()));
}

double BreadcrumbTrail::natural_width(
    const BreadcrumbSegment& segment) const {
    FontSpec font = effective_font(font_);
    if (!segments_.empty() && &segment == &segments_.back()) {
        font.weight = 700;
    }
    const double text_width =
        resolve_text_layout_utf8(segment.text, font).logical_size.width;
    const double padding = appearance_ == BreadcrumbAppearance::raised ? 33.0 : 23.0;
    const double result = std::clamp(padding + text_width, 38.0, 220.0);
    return result;
}

void BreadcrumbTrail::rebuild_layout(double width, double height) {
    visible_items_.clear();
    hidden_segment_ids_.clear();
    const std::size_t segment_count_bound = segments_.size();
    if (segment_count_bound > visible_items_.max_size() - 2U) {
        throw std::length_error("BreadcrumbTrail layout exceeds vector capacity");
    }
    visible_items_.reserve(segment_count_bound + 2U);
    hidden_segment_ids_.reserve(segment_count_bound);
    hovered_visible_.reset();
    pressed_visible_.reset();
    if (width <= 0.0 || height <= 0.0) return;

    const double inner_y = 1.0;
    const double inner_height = std::max(0.0, height - 2.0);
    const double edit_x = std::max(1.0, width - edit_width_ - 1.0);
    const double segment_extent = std::max(0.0, edit_x - 1.0);
    std::vector<std::size_t> visible_segments{};
    visible_segments.reserve(segments_.size());

    const std::size_t segment_count = segments_.size();
    std::vector<double> natural_widths(segment_count, 0.0);
    for (std::size_t index = 0U; index < segment_count; ++index) {
        natural_widths[index] = natural_width(segments_[index]);
    }
    // Temporary proposal storage is allocated once and reused in the suffix walk.
    std::vector<std::size_t> proposed{};
    proposed.reserve(segment_count);

    for (std::size_t index = 0; index < segments_.size(); ++index) {
        visible_segments.push_back(index);
    }
    bool overflow = breadcrumb_occupied_width(visible_segments, natural_widths, false, edge_overlap_) > segment_extent;
    if (overflow && segments_.size() > 2U) {
        visible_segments = {0U, segments_.size() - 1U};
        for (std::size_t candidate = segments_.size() - 1U;
             candidate > 1U; --candidate) {
            proposed = visible_segments;
            proposed.insert(proposed.begin() + 1,
                            candidate - 1U);
            if (breadcrumb_occupied_width(proposed, natural_widths, true, edge_overlap_) > segment_extent) break;
            // Both allocations retain their capacity for the next proposal.
            visible_segments.swap(proposed);
        }
        std::size_t visible_position = 0U;
        for (std::size_t index = 0U; index < segment_count; ++index) {
            if (visible_position < visible_segments.size() &&
                visible_segments[visible_position] == index) {
                ++visible_position;
            } else {
                hidden_segment_ids_.push_back(segments_[index].stable_id);
            }
        }
        overflow = !hidden_segment_ids_.empty();
    } else if (overflow) {
        // A minimum-width host can still preserve both authorities. Their
        // faces compress, but no path identity is silently removed.
        overflow = false;
    }

    double x = 1.0;
    for (std::size_t position = 0U; position < visible_segments.size(); ++position) {
        if (overflow && position == 1U) {
            const double remaining = std::max(0.0, edit_x - x);
            const double item_width = std::min(42.0, remaining + edge_overlap_);
            const VisibleItem item{.kind = VisibleKind::overflow, .segment_index = 0U,
                .bounds = {x, inner_y, item_width, inner_height}};
            visible_items_.push_back(item);
            x += std::max(0.0, item_width - edge_overlap_);
        }
        const std::size_t index = visible_segments[position];
        const double remaining = std::max(0.0, edit_x - x);
        const double item_width = std::min(natural_widths[index], remaining + edge_overlap_);
        const VisibleItem item{.kind = VisibleKind::segment, .segment_index = index,
            .bounds = {x, inner_y, item_width, inner_height}};
        visible_items_.push_back(item);
        x += std::max(0.0, item_width - edge_overlap_);
    }
    visible_items_.push_back(
        {VisibleKind::edit, 0U,
         {edit_x, inner_y, std::max(0.0, width - edit_x - 1.0), inner_height}});
}

void BreadcrumbTrail::normalize_active() {
    if (visible_items_.empty()) {
        active_id_.clear();
        return;
    }
    bool visible = false;
    for (const VisibleItem& item : visible_items_) {
        const std::string_view identity = visible_stable_id(item);
        if (identity == active_id_) {
            visible = true;
            break;
        }
    }
    if (visible) return;
    active_id_ = edit_stable_id_;
    for (std::size_t remaining = visible_items_.size(); remaining > 0U; --remaining) {
        const VisibleItem& item = visible_items_[remaining - 1U];
        if (item.kind == VisibleKind::segment) {
            active_id_ = segments_[item.segment_index].stable_id;
            break;
        }
    }
}

void BreadcrumbTrail::arrange(Rect final_bounds) {
    arrange_self(final_bounds);
    rebuild_layout(final_bounds.width, final_bounds.height);
    normalize_active();
    if (editor_) {
        set_child_layout(editor_,
            {1.0, 1.0, std::max(0.0, final_bounds.width - 2.0),
             std::max(0.0, final_bounds.height - 2.0)});
    }
}

std::string_view BreadcrumbTrail::visible_stable_id(
    const VisibleItem& item) const noexcept {
    if (item.kind == VisibleKind::overflow) return overflow_stable_id_;
    if (item.kind == VisibleKind::edit) return edit_stable_id_;
    return segments_[item.segment_index].stable_id;
}

void BreadcrumbTrail::on_paint(Painter& painter, Rect damage) {
    Panel::on_paint(painter, damage);
    if (editing_) return;
    if (appearance_ == BreadcrumbAppearance::raised) {
        paint_raised(painter);
        return;
    }
    const FontSpec font = effective_font(font_);
    const double baseline = snap_text_baseline(std::max(
        font.size, committed_arranged_bounds().height * 0.5 + font.size * 0.34));
    painter.save();
    painter.clip_rect(local_bounds());

    // Paint every overlapping face before its shared chevron edge. Drawing a
    // joint inside this loop lets the next face erase that joint because each
    // successor deliberately overlaps the preceding face by edge_overlap_.
    for (std::size_t index = 0; index < visible_items_.size(); ++index) {
        const VisibleItem& item = visible_items_[index];
        const bool active = focused_ && visible_stable_id(item) == active_id_;
        const bool hovered = hovered_visible_ == index;
        Color face = style().paper;
        Color foreground = style().text;
        std::string_view text{};
        if (item.kind == VisibleKind::edit) {
            const Color terminal_top = hovered || active
                ? style().accent_light
                : style().accent;
            const std::array<GradientStop, 2> stops{{
                {0.0, terminal_top}, {1.0, style().dark_border}}};
            painter.fill_linear_gradient(
                item.bounds, {item.bounds.x, item.bounds.y},
                {item.bounds.x, item.bounds.y + item.bounds.height}, stops);
            foreground = style().highlight;
            text = "./";
        } else if (item.kind == VisibleKind::overflow) {
            face = hovered ? style().face_light : style().face;
            text = "…";
        } else {
            const BreadcrumbSegment& segment = segments_[item.segment_index];
            if (!segment.enabled) foreground = style().disabled_text;
            if (hovered || active) face = style().face_light;
            text = segment.text;
        }
        if (item.kind != VisibleKind::edit) painter.fill_rect(item.bounds, face);
        FontSpec item_font = font;
        if (item.kind == VisibleKind::edit ||
            (item.kind == VisibleKind::segment &&
             item.segment_index + 1U == segments_.size())) {
            item_font.weight = 700;
        }
        const double text_width =
            painter.measure_text_utf8(text, item_font).width;
        const double text_x = item.kind == VisibleKind::segment
            ? item.bounds.x + 9.0
            : item.bounds.x + std::max(0.0,
                (item.bounds.width - text_width) * 0.5);
        painter.draw_text_utf8(
            {text_x, baseline}, text, item_font, foreground);
    }

    // Shared edges and focus rings are the foreground geometry of the one
    // continuous location instrument, so no later face may cover them.
    for (std::size_t index = 0; index < visible_items_.size(); ++index) {
        const VisibleItem& item = visible_items_[index];
        const bool active = focused_ && visible_stable_id(item) == active_id_;
        if (index + 1U < visible_items_.size()) {
            const double edge_x = item.bounds.x + item.bounds.width - edge_overlap_;
            const double middle = item.bounds.y + item.bounds.height * 0.5;
            painter.draw_line({edge_x, item.bounds.y + 2.0},
                              {edge_x + edge_overlap_ - 1.0, middle},
                              style().dark_border, 1.0);
            painter.draw_line({edge_x + edge_overlap_ - 1.0, middle},
                              {edge_x, item.bounds.y + item.bounds.height - 2.0},
                              style().highlight, 1.0);
        }
        if (active) {
            painter.stroke_rect(
                {item.bounds.x + 2.5, item.bounds.y + 2.5,
                 std::max(0.0, item.bounds.width - 5.0),
                 std::max(0.0, item.bounds.height - 5.0)},
                style().accent, 1.0);
        }
    }
    painter.restore();
}

void BreadcrumbTrail::paint_raised(Painter& painter) {
    const FontSpec font = effective_font(font_);
    const double baseline = snap_text_baseline(std::max(
        font.size, committed_arranged_bounds().height * 0.5 + font.size * 0.34));
    painter.save();
    painter.clip_rect(local_bounds());
    // Back-to-front preserves the complete pointed nose of each preceding
    // segment. Nine bounded strips express the tip through the public Painter
    // vocabulary; every backend receives the same retained geometry.
    for (std::size_t remaining = visible_items_.size(); remaining > 0U; --remaining) {
        const std::size_t index = remaining - 1U;
        const VisibleItem& item = visible_items_[index];
        const Rect bounds = item.bounds;
        if (bounds.width <= 0.0 || bounds.height <= 0.0) continue;
        const bool current = item.kind == VisibleKind::segment &&
            item.segment_index + 1U == segments_.size();
        const bool active = focused_ && visible_stable_id(item) == active_id_;
        const bool hot = hovered_visible_ == index || active;
        const bool pressed = pressed_visible_ == index;
        const bool terminal = item.kind == VisibleKind::edit;
        const double tip = terminal ? 0.0 : std::min(edge_overlap_, bounds.width * 0.25);
        const double shoulder = bounds.x + bounds.width - tip;
        const double middle = bounds.y + bounds.height * 0.5;
        Color top = style().highlight;
        Color bottom = current ? style().accent_light : style().face;
        if (hot) bottom = style().accent_light;
        if (pressed) { top = style().face; bottom = style().face_light; }
        if (terminal) { top = style().accent; bottom = style().dark_border; }
        const std::array<GradientStop, 3> stops{{
            {0.0, top}, {0.45, pressed ? bottom : style().face_light}, {1.0, bottom}}};
        const std::array<GradientStop, 2> terminal_stops{{{0.0, top}, {1.0, bottom}}};
        const Point start{bounds.x, bounds.y};
        const Point end{bounds.x, bounds.y + bounds.height};
        const Rect body{bounds.x, bounds.y, bounds.width - tip, bounds.height};
        if (terminal) painter.fill_linear_gradient(body, start, end, terminal_stops);
        else painter.fill_linear_gradient(body, start, end, stops);
        for (unsigned strip = 0U; strip < 9U && tip > 0.0; ++strip) {
            const double left = tip * static_cast<double>(strip) / 9.0;
            const double right = tip * static_cast<double>(strip + 1U) / 9.0;
            const double inset = bounds.height * 0.5 * left / tip;
            const Rect slice{shoulder + left, bounds.y + inset,
                             right - left, bounds.height - inset * 2.0};
            painter.fill_linear_gradient(slice, start, end, stops);
        }
        if (!terminal) {
            const std::array<GradientStop, 4> grain{{
                {0.0, {255, 255, 255, 18}}, {0.32, {255, 255, 255, 18}},
                {0.34, {255, 255, 255, 0}}, {1.0, {255, 255, 255, 0}}}};
            painter.fill_linear_gradient_spread(body, {bounds.x, bounds.y},
                {bounds.x + 3.0, bounds.y + 3.0}, grain, GradientSpreadMode::repeat);
        }
        painter.draw_line({bounds.x, bounds.y + 0.5}, {shoulder, bounds.y + 0.5},
                          pressed ? style().dark_border : style().highlight, 1.0);
        painter.draw_line({bounds.x, bounds.y + bounds.height - 0.5},
                          {shoulder, bounds.y + bounds.height - 0.5}, style().border, 1.0);
        if (!terminal) {
            painter.draw_line({shoulder, bounds.y + 0.5},
                              {bounds.x + bounds.width - 0.5, middle}, style().dark_border, 1.0);
            painter.draw_line({bounds.x + bounds.width - 0.5, middle},
                              {shoulder, bounds.y + bounds.height - 0.5}, style().dark_border, 1.0);
            painter.draw_line({shoulder - 1.0, bounds.y + 1.0},
                              {bounds.x + bounds.width - 2.0, middle}, style().highlight, 1.0);
        }
        std::string_view text = terminal ? "./" : "…";
        Color foreground = terminal ? style().highlight : style().text;
        if (item.kind == VisibleKind::segment) {
            text = segments_[item.segment_index].text;
            if (!segments_[item.segment_index].enabled) foreground = style().disabled_text;
        }
        FontSpec item_font = font;
        if (current || terminal) item_font.weight = 700;
        const double left_padding = index == 0U || terminal ? 9.0 : edge_overlap_ + 5.0;
        const double text_x = bounds.x + left_padding;
        painter.save();
        painter.clip_rect({text_x, bounds.y + 1.0,
                           std::max(0.0, shoulder - text_x - 2.0), std::max(0.0, bounds.height - 2.0)});
        painter.draw_text_utf8({text_x, baseline}, text, item_font, foreground);
        painter.restore();
        if (active) painter.stroke_rect({bounds.x + left_padding - 2.0, bounds.y + 2.5,
            std::max(0.0, shoulder - bounds.x - left_padding), std::max(0.0, bounds.height - 5.0)}, style().accent, 1.0);
    }
    painter.restore();
}

std::optional<std::size_t> BreadcrumbTrail::visible_index_at(
    Point absolute) const noexcept {
    const Rect bounds = absolute_bounds();
    const Point local{absolute.x - bounds.x, absolute.y - bounds.y};
    for (std::size_t index = visible_items_.size(); index > 0U; --index) {
        const VisibleItem& item = visible_items_[index - 1U];
        if (!item.bounds.contains(local)) continue;
        if (appearance_ == BreadcrumbAppearance::raised && item.kind != VisibleKind::edit) {
            const double half_height = item.bounds.height * 0.5;
            const double tip = std::min(edge_overlap_, item.bounds.width * 0.25);
            const double distance = std::abs(local.y - item.bounds.y - half_height);
            const double point_x = item.bounds.x + item.bounds.width - tip * distance / half_height;
            if (local.x > point_x) continue;
            if (index > 1U) {
                const Rect previous = visible_items_[index - 2U].bounds;
                const double previous_tip = std::min(edge_overlap_, previous.width * 0.25);
                const double previous_edge = previous.x + previous.width - previous_tip * distance / half_height;
                if (local.x < previous_edge) continue;
            }
        }
        const std::optional<std::size_t> result(index - 1U);
        return result;
    }
    return {};
}

void BreadcrumbTrail::activate_visible(std::size_t index) {
    if (index >= visible_items_.size()) return;
    const VisibleItem& item = visible_items_[index];
    active_id_ = std::string(visible_stable_id(item));
    invalidate(Dirty::paint | Dirty::semantics | Dirty::accessibility);
    if (item.kind == VisibleKind::edit) {
        begin_edit(editor_ ? std::string((*editor_).text()) : std::string{});
    } else if (item.kind == VisibleKind::overflow) {
        publish_change(overflow_activated_);
    } else if (segments_[item.segment_index].enabled) {
        publish_change(segment_activated_,
                       segments_[item.segment_index].stable_id);
    }
}

void BreadcrumbTrail::on_pointer(PointerEvent& event) {
    if (!eligible_for_input() || editing_) return;
    if (event.action == PointerAction::move) {
        const std::optional<std::size_t> hovered = visible_index_at(event.position);
        if (hovered != hovered_visible_) {
            hovered_visible_ = hovered;
            invalidate(Dirty::paint);
        }
        return;
    }
    if (event.action == PointerAction::leave) {
        hovered_visible_.reset();
        invalidate(Dirty::paint);
        return;
    }
    if (event.button != PointerButton::primary) return;
    if (event.action == PointerAction::down) {
        if (window()) static_cast<void>((*window()).request_focus(shared_from_this()));
        pressed_visible_ = visible_index_at(event.position);
        event.handled = pressed_visible_.has_value();
    } else if (event.action == PointerAction::up) {
        const std::optional<std::size_t> released = visible_index_at(event.position);
        if (released && released == pressed_visible_) activate_visible(*released);
        event.handled = released.has_value();
        pressed_visible_.reset();
    }
}

void BreadcrumbTrail::on_key_preview(KeyEvent& event) {
    if (!editing_ || !tab_completion_available_ || !enabled() ||
        event.action != KeyAction::down ||
        event.physical_key != PhysicalKey::tab) {
        return;
    }
    publish_change(edit_completion_requested_);
    event.handled = true;
}

void BreadcrumbTrail::on_key(KeyEvent& event) {
    if (!focused_ || !enabled() || editing_ ||
        event.action != KeyAction::down || visible_items_.empty()) return;
    std::size_t active{};
    for (std::size_t index = 0U; index < visible_items_.size(); ++index) {
        const std::string_view identity = visible_stable_id(visible_items_[index]);
        if (identity == active_id_) {
            active = index;
            break;
        }
    }
    if (event.physical_key == PhysicalKey::enter ||
        event.physical_key == PhysicalKey::space) {
        activate_visible(active);
        event.handled = true;
        return;
    }
    if (event.physical_key == PhysicalKey::left && active > 0U) --active;
    else if (event.physical_key == PhysicalKey::right &&
             active + 1U < visible_items_.size()) ++active;
    else if (event.physical_key == PhysicalKey::home) active = 0U;
    else if (event.physical_key == PhysicalKey::end) {
        active = visible_items_.size() - 1U;
    } else return;
    active_id_ = std::string(visible_stable_id(visible_items_[active]));
    invalidate(Dirty::paint | Dirty::semantics | Dirty::accessibility);
    event.handled = true;
}

void BreadcrumbTrail::on_focus_changed(bool focused) {
    focused_ = focused;
    normalize_active();
    invalidate(Dirty::paint | Dirty::semantics | Dirty::accessibility);
}

SemanticDescriptor BreadcrumbTrail::semantic_descriptor() const {
    SemanticDescriptor descriptor{};
    descriptor.role = SemanticRole::group;
    descriptor.name = accessible_name();
    descriptor.value = editor_ ? std::string((*editor_).text()) : std::string{};
    descriptor.description = accessible_description();
    descriptor.actions = {SemanticAction::focus};
    descriptor.exposed = true;
    descriptor.include_descendants = true;
    return descriptor;
}

std::vector<SemanticNode> BreadcrumbTrail::semantic_virtual_children() const {
    std::vector<SemanticNode> nodes{};
    if (editing_) return nodes;
    nodes.reserve(visible_items_.size());
    const Rect trail = absolute_bounds();
    for (const VisibleItem& item : visible_items_) {
        SemanticNode node{};
        node.stable_id = std::string(visible_stable_id(item));
        node.runtime_id = virtual_runtime_id(node.stable_id);
        node.role = SemanticRole::button;
        node.bounds = {trail.x + item.bounds.x, trail.y + item.bounds.y,
                       item.bounds.width, item.bounds.height};
        node.states = SemanticState::visible | SemanticState::focusable;
        if (effectively_enabled()) node.states |= SemanticState::enabled;
        if (focused_ && node.stable_id == active_id_) {
            node.states |= SemanticState::focused;
        }
        node.actions = {SemanticAction::focus, SemanticAction::press};
        if (item.kind == VisibleKind::edit) {
            node.name = "Edit complete path";
            node.description =
                "Switch this breadcrumb to one inline exact-path editor";
        } else if (item.kind == VisibleKind::overflow) {
            node.name = "Hidden path segments";
            node.description = std::to_string(hidden_segment_ids_.size()) +
                " middle path segments";
            node.actions.push_back(SemanticAction::show_menu);
        } else {
            const BreadcrumbSegment& segment = segments_[item.segment_index];
            node.name = segment.text;
            node.description = segment.description;
            if (!segment.enabled) {
                node.states = SemanticState::visible | SemanticState::focusable;
            }
        }
        nodes.push_back(std::move(node));
    }
    return nodes;
}

bool BreadcrumbTrail::on_semantic_child_action(
    std::string_view stable_id, SemanticAction action, std::string_view) {
    if (editing_) return false;
    std::size_t index = 0U;
    while (index < visible_items_.size()) {
        const std::string_view identity = visible_stable_id(visible_items_[index]);
        if (identity == stable_id) break;
        ++index;
    }
    if (index == visible_items_.size()) return false;
    if (action != SemanticAction::focus && action != SemanticAction::press &&
        action != SemanticAction::show_menu) return false;
    if (window()) static_cast<void>((*window()).request_focus(shared_from_this()));
    active_id_ = std::string(stable_id);
    if (action == SemanticAction::press || action == SemanticAction::show_menu) {
        activate_visible(index);
    } else {
        invalidate(Dirty::paint | Dirty::semantics | Dirty::accessibility);
    }
    return true;
}

} // namespace gui_forms
