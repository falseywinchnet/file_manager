#include "gui_forms/controls/panel/object_view/object_view.hpp"

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

namespace {

std::string elide_object_name(Painter& painter, std::string_view text,
                              FontSpec font, const double maximum_width) {
    if (maximum_width <= 0.0 ||
        painter.measure_text_utf8(text, font).width <= maximum_width) {
        return std::string(text);
    }
    constexpr std::string_view ellipsis = "…";
    std::string prefix(text);
    while (!prefix.empty()) {
        std::size_t scalar = prefix.size() - 1U;
        while (scalar > 0U &&
               (static_cast<unsigned char>(prefix[scalar]) & 0xc0U) == 0x80U) {
            --scalar;
        }
        prefix.resize(scalar);
        std::string candidate(prefix);
        candidate.append(ellipsis);
        if (painter.measure_text_utf8(candidate, font).width <= maximum_width) {
            return candidate;
        }
    }
    return std::string(ellipsis);
}

} // namespace

using namespace collection_detail;
ObjectView::ObjectView(StableId stable_id) : Panel(std::move(stable_id)) {
    set_paint_plane(PaintPlane::control);
    set_background(style().paper);
    set_focusable(true);
}

void ObjectView::set_items(std::vector<ObjectViewItem> items) {
    require_mutable();
    std::unordered_set<std::string> identities;
    for (const ObjectViewItem& item : items) {
        validate_identity_text(item.stable_id, item.name);
        if (!validate_utf8(item.secondary_text).valid() ||
            !validate_utf8(item.description).valid() ||
            item.image_key.size() > 256U ||
            (!item.image_key.empty() && !validate_utf8(item.image_key).valid()) ||
            !identities.insert(item.stable_id).second) {
            throw std::invalid_argument("ObjectView requires unique IDs and valid UTF-8");
        }
    }
    const std::vector<std::string> retained_selection = selected_ids_;
    const std::string retained_primary = selected_id_;
    const std::string retained_focus = focused_id_;
    const std::string retained_anchor = selection_anchor_id_;
    items_ = std::move(items);
    selected_ids_.clear();
    for (const ObjectViewItem& item : items_) {
        if (std::find(retained_selection.begin(), retained_selection.end(),
                      item.stable_id) != retained_selection.end()) {
            selected_ids_.push_back(item.stable_id);
        }
    }
    selected_id_ = std::find(selected_ids_.begin(), selected_ids_.end(),
                             retained_primary) != selected_ids_.end()
        ? retained_primary
        : selected_ids_.empty() ? std::string{} : selected_ids_.front();
    focused_id_ = item_index(retained_focus) ? retained_focus : std::string{};
    selection_anchor_id_ = item_index(retained_anchor) ? retained_anchor
                                                       : selected_id_;
    top_row_ = std::min(top_row_, items_.empty() ? 0U :
        (items_.size() - 1U) / columns());
    hovered_index_.reset();
    pressed_index_.reset();
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test | Dirty::semantics);
    if (retained_selection != selected_ids_ || retained_primary != selected_id_) {
        ObjectSelectionChange change;
        change.previous_id = retained_primary;
        change.current_id = selected_id_;
        change.previous_ids = retained_selection;
        change.current_ids = selected_ids_;
        publish_change(selection_changed_, change);
    }
}

std::optional<std::size_t> ObjectView::item_index(std::string_view id) const noexcept {
    for (std::size_t index = 0U; index < items_.size(); ++index) {
        if (items_[index].stable_id == id) return index;
    }
    return {};
}

void ObjectView::set_view_mode(ObjectViewMode mode) {
    require_mutable();
    if (view_mode_ == mode) return;
    const std::optional<std::size_t> selected = item_index(selected_id_);
    view_mode_ = mode;
    top_row_ = 0U;
    if (selected) ensure_visible(*selected);
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

void ObjectView::set_selected_id(std::string_view id) {
    require_mutable();
    if (!id.empty() && !item_index(id)) {
        throw std::out_of_range("ObjectView selection ID is not in the model");
    }
    const std::string next(id);
    apply_selection(next.empty() ? std::vector<std::string>{}
                                 : std::vector<std::string>{next},
                    next, next, false);
}

void ObjectView::set_selected_ids(std::vector<std::string> ids,
                                  std::string_view primary_id) {
    require_mutable();
    if (selection_mode_ == ObjectSelectionMode::single && ids.size() > 1U) {
        throw std::invalid_argument(
            "ObjectView single-selection mode accepts at most one selected ID");
    }
    apply_selection(std::move(ids), std::string(primary_id),
                    std::string(primary_id), false);
}

void ObjectView::clear_selection() {
    require_mutable();
    apply_selection({}, {}, {}, false);
}

void ObjectView::select_all() {
    require_mutable();
    if (items_.empty()) {
        clear_selection();
        return;
    }
    std::vector<std::string> ids;
    ids.reserve(selection_mode_ == ObjectSelectionMode::single ? 1U : items_.size());
    for (const ObjectViewItem& item : items_) {
        if (item.enabled) {
            ids.push_back(item.stable_id);
            if (selection_mode_ == ObjectSelectionMode::single) break;
        }
    }
    const std::string primary = ids.empty() ? std::string{} : ids.front();
    apply_selection(std::move(ids), primary, primary, true);
    if (const std::optional<std::size_t> index = item_index(primary)) ensure_visible(*index);
}

void ObjectView::set_selection_mode(ObjectSelectionMode mode) {
    require_mutable();
    if (selection_mode_ == mode) return;
    selection_mode_ = mode;
    if (mode == ObjectSelectionMode::single && selected_ids_.size() > 1U) {
        const std::string keep = selected_id_.empty() ? selected_ids_.front()
                                                       : selected_id_;
        apply_selection({keep}, keep, keep, false);
    }
    invalidate(Dirty::semantics);
}

bool ObjectView::is_selected(std::string_view id) const noexcept {
    return std::find(selected_ids_.begin(), selected_ids_.end(), id) !=
           selected_ids_.end();
}

void ObjectView::apply_selection(std::vector<std::string> ids,
                                 std::string primary,
                                 std::string anchor,
                                 bool move_focus) {
    std::unordered_set<std::string> requested;
    for (const std::string& id : ids) {
        if (!validate_utf8(id).valid() || !item_index(id)) {
            throw std::out_of_range("ObjectView selected ID is not in the model");
        }
        requested.insert(id);
    }
    if (selection_mode_ == ObjectSelectionMode::single && requested.size() > 1U) {
        throw std::invalid_argument(
            "ObjectView single-selection mode accepts at most one selected ID");
    }
    std::vector<std::string> normalized;
    normalized.reserve(requested.size());
    for (const ObjectViewItem& item : items_) {
        if (requested.contains(item.stable_id)) normalized.push_back(item.stable_id);
    }
    if (normalized.empty()) {
        primary.clear();
    } else if (primary.empty()) {
        primary = normalized.front();
    } else if (std::find(normalized.begin(), normalized.end(), primary) ==
               normalized.end()) {
        throw std::invalid_argument(
            "ObjectView primary selection must be one of the selected IDs");
    }
    if (!anchor.empty() && !item_index(anchor)) {
        throw std::out_of_range("ObjectView selection anchor is not in the model");
    }
    const bool changed = selected_ids_ != normalized || selected_id_ != primary;
    const std::vector<std::string> previous_ids = selected_ids_;
    const std::string previous_id = selected_id_;
    selected_ids_ = std::move(normalized);
    selected_id_ = std::move(primary);
    selection_anchor_id_ = std::move(anchor);
    if (move_focus) focused_id_ = selected_id_;
    if (!changed) {
        if (move_focus) invalidate(Dirty::paint | Dirty::semantics);
        return;
    }
    ObjectSelectionChange change;
    change.previous_id = previous_id;
    change.current_id = selected_id_;
    change.previous_ids = previous_ids;
    change.current_ids = selected_ids_;
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(selection_changed_, change);
}

std::vector<std::string> ObjectView::range_selection(
    std::size_t target_index, bool preserve_existing) const {
    std::vector<std::string> result = preserve_existing ? selected_ids_
                                                        : std::vector<std::string>{};
    std::optional<std::size_t> resolved_anchor = item_index(selection_anchor_id_);
    if (!resolved_anchor) resolved_anchor = item_index(focused_id_);
    if (!resolved_anchor) resolved_anchor = item_index(selected_id_);
    const std::size_t anchor_index = resolved_anchor.value_or(target_index);
    const std::size_t first = std::min(anchor_index, target_index);
    const std::size_t last = std::max(anchor_index, target_index);
    for (std::size_t index = first; index <= last; ++index) {
        if (!items_[index].enabled ||
            std::find(result.begin(), result.end(), items_[index].stable_id) !=
                result.end()) continue;
        result.push_back(items_[index].stable_id);
    }
    return result;
}

void ObjectView::set_icon_cell_size(Size size) {
    require_mutable();
    if (!std::isfinite(size.width) || !std::isfinite(size.height) ||
        size.width < 86.0 || size.height < 78.0 ||
        size.width > 512.0 || size.height > 512.0) {
        throw std::invalid_argument("ObjectView icon cell must be 86x78 through 512x512");
    }
    if (icon_cell_size_ == size) return;
    icon_cell_size_ = size;
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

void ObjectView::set_details_row_height(double height) {
    require_mutable();
    if (!std::isfinite(height) || height < 20.0 || height > 256.0) {
        throw std::invalid_argument("ObjectView details row height must be 20 through 256");
    }
    if (details_row_height_ == height) return;
    details_row_height_ = height;
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

void ObjectView::set_show_secondary_text(bool show) {
    require_mutable();
    if (show_secondary_text_ == show) return;
    show_secondary_text_ = show;
    invalidate(Dirty::paint | Dirty::semantics);
}

void ObjectView::set_top_row(std::size_t row) {
    require_mutable();
    const std::size_t rows = items_.empty() ? 0U : (items_.size() + columns() - 1U) / columns();
    if (rows == 0U) {
        if (row != 0U) throw std::out_of_range("empty ObjectView top row must be zero");
    } else if (row >= rows) throw std::out_of_range("ObjectView top row is outside the model");
    if (top_row_ == row) return;
    top_row_ = row;
    invalidate(Dirty::paint | Dirty::semantics);
}

void ObjectView::set_font(FontSpec font) {
    require_mutable();
    if (!valid_font_spec(font)) throw std::invalid_argument("ObjectView font is invalid");
    if (font_ == font) return;
    font_ = font;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void ObjectView::set_image_list(std::shared_ptr<ImageList> image_list) {
    require_mutable();
    if (image_list && !(*image_list).is_alive()) {
        throw std::invalid_argument("ObjectView requires a live ImageList");
    }
    if (image_list && window() && !(*image_list).belongs_to(*window())) {
        throw std::invalid_argument("ObjectView and ImageList must belong to one Window");
    }
    if (image_list_ == image_list) return;
    image_list_changed_.disconnect();
    image_list_ = std::move(image_list);
    if (image_list_) {
        image_list_changed_ = (*image_list_).changed().subscribe(
            *this, Delegate<const ImageListChange&>::bind<
                ObjectView, &ObjectView::image_list_content_changed>(*this));
    }
    invalidate(Dirty::paint | Dirty::semantics);
}

void ObjectView::image_list_content_changed(const ImageListChange&) {
    if (is_alive()) invalidate(Dirty::paint | Dirty::semantics);
}

bool ObjectView::paint_item_image(
    Painter& painter, const ObjectViewItem& item,
    std::size_t index, bool selected, Rect destination_bounds) {
    if (!image_list_ || !(*image_list_).is_alive() || item.image_key.empty()) {
        return false;
    }
    const ImageVisualState state = !item.enabled
        ? ImageVisualState::disabled
        : selected ? ImageVisualState::selected
        : hovered_index_ == index ? ImageVisualState::hot
                                  : ImageVisualState::normal;
    const ImageListResolution resolved = (*image_list_).resolve(
        item.image_key, state, window() ? (*window()).scale() : 1.0);
    if (!resolved) return false;
    const Rect destination = fit_image_rect(
        destination_bounds, resolved.source_size);
    painter.draw_image(
        resolved.image, destination,
        !item.enabled &&
                resolved.resolved_state != ImageVisualState::disabled
            ? 0.45 : 1.0);
    return true;
}

void ObjectView::on_attached_to_window() {
    Panel::on_attached_to_window();
    if (image_list_ && !(*image_list_).belongs_to(*window())) {
        throw std::logic_error("ObjectView cannot attach to a different ImageList Window");
    }
}

std::size_t ObjectView::columns() const noexcept {
    if (view_mode_ == ObjectViewMode::details) return 1U;
    double width = local_bounds().width;
    if (width <= 4.0) width = requested_bounds().width;
    const double scaled_width = icon_cell_size_.width * effective_text_scale();
    return std::max<std::size_t>(1U, static_cast<std::size_t>(
        std::floor(std::max(0.0, width - 8.0) / scaled_width)));
}

double ObjectView::row_height() const noexcept {
    return (view_mode_ == ObjectViewMode::icons ? icon_cell_size_.height
                                                : details_row_height_) *
        effective_text_scale();
}

std::size_t ObjectView::visible_row_count() const noexcept {
    double height = local_bounds().height;
    if (height <= 4.0) height = requested_bounds().height;
    return std::max<std::size_t>(1U, static_cast<std::size_t>(
        std::ceil(std::max(0.0, height - 4.0) / row_height())));
}

Rect ObjectView::item_bounds(std::size_t index) const noexcept {
    const std::size_t column_count = columns();
    const std::size_t row = index / column_count;
    const std::size_t column = index % column_count;
    const double available_width = std::max(0.0, local_bounds().width - 8.0);
    const double cell_width = view_mode_ == ObjectViewMode::icons
        ? available_width / static_cast<double>(column_count) : available_width;
    return {4.0 + static_cast<double>(column) * cell_width,
            2.0 + static_cast<double>(row - top_row_) * row_height(),
            cell_width, row_height()};
}

std::optional<std::size_t> ObjectView::index_at(Point absolute) const noexcept {
    const Rect bounds = absolute_bounds();
    if (!bounds.contains(absolute)) return {};
    const double local_x = absolute.x - bounds.x - 4.0;
    const double local_y = absolute.y - bounds.y - 2.0;
    if (local_x < 0.0 || local_y < 0.0) return {};
    const std::size_t column_count = columns();
    const double cell_width = std::max(1.0, (bounds.width - 8.0) / column_count);
    const std::size_t column = view_mode_ == ObjectViewMode::details ? 0U
        : std::min(column_count - 1U, static_cast<std::size_t>(local_x / cell_width));
    const std::size_t row = top_row_ + static_cast<std::size_t>(local_y / row_height());
    const std::size_t index = row * column_count + column;
    return index < items_.size() ? std::optional<std::size_t>{index}
                                 : std::optional<std::size_t>{};
}

std::string_view ObjectView::item_id_at(const Point absolute) const noexcept {
    const auto index = index_at(absolute);
    return index ? std::string_view(items_[*index].stable_id) : std::string_view{};
}

void ObjectView::ensure_visible(std::size_t index) {
    const std::size_t row = index / columns();
    const std::size_t count = visible_row_count();
    if (row < top_row_) top_row_ = row;
    else if (row >= top_row_ + count) top_row_ = row - count + 1U;
}

void ObjectView::focus_index(std::size_t index) {
    if (index >= items_.size() || !items_[index].enabled) return;
    if (focused_id_ != items_[index].stable_id) {
        focused_id_ = items_[index].stable_id;
        invalidate(Dirty::paint | Dirty::semantics);
    }
    ensure_visible(index);
}

void ObjectView::select_index(std::size_t index, bool activate,
                              Modifier modifiers) {
    if (index >= items_.size() || !items_[index].enabled) return;
    const std::string target = items_[index].stable_id;
    const bool toggle = selection_mode_ == ObjectSelectionMode::multiple &&
        (has_modifier(modifiers, Modifier::control) ||
         has_modifier(modifiers, Modifier::meta));
    const bool extend = selection_mode_ == ObjectSelectionMode::multiple &&
        has_modifier(modifiers, Modifier::shift);
    if (extend) {
        const std::string anchor = selection_anchor_id_.empty()
            ? (focused_id_.empty() ? target : focused_id_)
            : selection_anchor_id_;
        apply_selection(range_selection(index, toggle), target, anchor, true);
    } else if (toggle) {
        std::vector<std::string> ids = selected_ids_;
        const std::vector<std::string>::iterator found =
            std::find(ids.begin(), ids.end(), target);
        if (found == ids.end()) ids.push_back(target);
        else ids.erase(found);
        const std::string primary = std::find(ids.begin(), ids.end(), target) != ids.end()
            ? target : ids.empty() ? std::string{} : ids.back();
        apply_selection(std::move(ids), primary, target, false);
        focus_index(index);
    } else {
        apply_selection({target}, target, target, true);
    }
    ensure_visible(index);
    if (activate && is_alive()) item_activated_.emit(target);
}

void ObjectView::arrange(Rect final_bounds) {
    Panel::arrange(final_bounds);
    if (const std::optional<std::size_t> focused = item_index(focused_id_)) ensure_visible(*focused);
}

void ObjectView::paint_glyph(Painter& painter, Rect b, ObjectGlyph glyph,
                             bool item_enabled) const {
    const Color border = item_enabled ? style().dark_border : style().disabled_text;
    const Color base = !item_enabled ? style().face
        : glyph == ObjectGlyph::folder ? Color::rgba(79, 137, 196)
        : glyph == ObjectGlyph::image ? Color::rgba(83, 154, 119)
        : glyph == ObjectGlyph::audio ? Color::rgba(151, 95, 173)
        : glyph == ObjectGlyph::archive ? Color::rgba(190, 135, 61)
        : glyph == ObjectGlyph::code ? Color::rgba(67, 132, 135)
        : Color::rgba(107, 126, 158);
    if (glyph == ObjectGlyph::folder) {
        painter.fill_rect({b.x + 3.0, b.y + 9.0, b.width - 6.0, b.height - 12.0}, base);
        painter.fill_rect({b.x + 7.0, b.y + 4.0, b.width * .42, 8.0},
                          with_alpha(style().highlight, 180));
        painter.fill_rect({b.x + 5.0, b.y + 12.0, b.width - 10.0, 3.0},
                          with_alpha(style().highlight, 130));
        painter.stroke_rect({b.x + 3.5, b.y + 9.5, b.width - 7.0, b.height - 13.0},
                            border, 1.0);
        return;
    }
    painter.fill_rect({b.x + 7.0, b.y + 2.0, b.width - 14.0, b.height - 4.0},
                      style().paper);
    painter.fill_rect({b.x + 7.0, b.y + 2.0, b.width - 14.0, 7.0}, base);
    painter.stroke_rect({b.x + 7.5, b.y + 2.5, b.width - 15.0, b.height - 5.0},
                        border, 1.0);
    if (glyph == ObjectGlyph::image) {
        painter.fill_rect({b.x + 12.0, b.y + 14.0, b.width - 24.0, b.height - 22.0},
                          with_alpha(base, 120));
    } else if (glyph == ObjectGlyph::audio) {
        painter.draw_line({b.x + 15.0, b.y + b.height * .65},
                          {b.x + b.width - 14.0, b.y + b.height * .35}, base, 3.0);
    } else {
        painter.fill_rect({b.x + 12.0, b.y + 16.0, b.width - 24.0, 2.0},
                          style().border);
        painter.fill_rect({b.x + 12.0, b.y + 22.0, b.width - 29.0, 2.0},
                          style().border);
    }
}

void ObjectView::on_paint(Painter& painter, Rect damage) {
    Panel::on_paint(painter, damage);
    const Rect bounds = local_bounds();
    const FontSpec font = effective_font(font_);
    painter.save();
    painter.clip_rect({2.0, 2.0, std::max(0.0, bounds.width - 4.0),
                       std::max(0.0, bounds.height - 4.0)});
    const std::size_t first = top_row_ * columns();
    const std::size_t end = std::min(items_.size(),
        (top_row_ + visible_row_count() + 1U) * columns());
    for (std::size_t index = first; index < end; ++index) {
        const ObjectViewItem& item = items_[index];
        const Rect cell = item_bounds(index);
        painter.save();
        painter.clip_rect({cell.x + 1.0, cell.y + 1.0,
                           std::max(0.0, cell.width - 2.0),
                           std::max(0.0, cell.height - 2.0)});
        const bool selected = is_selected(item.stable_id);
        const bool active = focused_ && item.stable_id == focused_id_;
        if (selected) painter.fill_rect({cell.x + 1.0, cell.y + 1.0,
                                         cell.width - 2.0, cell.height - 2.0},
                                        focused_ ? style().accent_light
                                                 : with_alpha(style().accent_light, 190));
        else if (hovered_index_ == index) {
            painter.fill_rect({cell.x + 1.0, cell.y + 1.0,
                               cell.width - 2.0, cell.height - 2.0}, style().face_light);
        }
        if (view_mode_ == ObjectViewMode::icons) {
            const double glyph_width = std::min(46.0, cell.width - 18.0);
            const Rect glyph_bounds{cell.x + (cell.width - glyph_width) * .5,
                                    cell.y + 4.0, glyph_width, 43.0};
            if (!paint_item_image(
                    painter, item, index, selected, glyph_bounds)) {
                paint_glyph(painter, glyph_bounds, item.glyph, item.enabled);
            }
            const std::string display_name = elide_object_name(
                painter, item.name, font, std::max(0.0, cell.width - 8.0));
            const Size measured = painter.measure_text_utf8(display_name, font);
            const double text_x = cell.x + std::max(4.0, (cell.width - measured.width) * .5);
            painter.draw_text_utf8({text_x, cell.y +
                                    std::max(65.0, 48.0 + font.size)},
                                   display_name, font,
                                   item.enabled ? style().text : style().disabled_text);
            if (show_secondary_text_ && !item.secondary_text.empty()) {
                FontSpec authored_secondary{font_.role,
                                            std::max(8.0, font_.size - 1.5),
                                            400, false};
                const FontSpec secondary = effective_font(authored_secondary);
                painter.draw_text_utf8({cell.x + 5.0, cell.y + cell.height - 6.0},
                                       item.secondary_text, secondary,
                                       style().disabled_text);
            }
        } else {
            const Rect glyph_bounds{cell.x + 6.0, cell.y + 3.0, 24.0,
                                    std::max(18.0, cell.height - 6.0)};
            if (!paint_item_image(
                    painter, item, index, selected, glyph_bounds)) {
                paint_glyph(painter, glyph_bounds, item.glyph, item.enabled);
            }
            painter.draw_text_utf8({cell.x + 38.0, cell.y + cell.height * .5 + 4.0},
                                   item.name, font, item.enabled ? style().text
                                                                 : style().disabled_text);
            if (show_secondary_text_ && !item.secondary_text.empty()) {
                painter.draw_text_utf8({cell.x + std::max(180.0, cell.width * .68),
                                        cell.y + cell.height * .5 + 4.0},
                                       item.secondary_text, font, style().disabled_text);
            }
            painter.draw_line({cell.x + 2.0, cell.y + cell.height - 1.0},
                              {cell.x + cell.width - 2.0, cell.y + cell.height - 1.0},
                              with_alpha(style().border, 80), 1.0);
        }
        if (active) {
            painter.stroke_rect({cell.x + 2.5, cell.y + 2.5,
                                 std::max(0.0, cell.width - 5.0),
                                 std::max(0.0, cell.height - 5.0)},
                                style().accent, 1.0);
        }
        painter.restore();
    }
    painter.restore();
}

void ObjectView::on_pointer(PointerEvent& event) {
    if (!eligible_for_input()) return;
    if (event.action == PointerAction::move) {
        const std::optional<std::size_t> next = index_at(event.position);
        if (next != hovered_index_) { hovered_index_ = next; invalidate(Dirty::paint); }
        return;
    }
    if (event.action == PointerAction::leave) {
        hovered_index_.reset(); invalidate(Dirty::paint); return;
    }
    if (event.action == PointerAction::wheel && !items_.empty() &&
        event.wheel_delta.y != 0.0) {
        const std::size_t rows = (items_.size() + columns() - 1U) / columns();
        const std::ptrdiff_t next = std::clamp<std::ptrdiff_t>(
            static_cast<std::ptrdiff_t>(top_row_) +
                (event.wheel_delta.y > 0.0 ? -2 : 2), 0,
            static_cast<std::ptrdiff_t>(rows - 1U));
        set_top_row(static_cast<std::size_t>(next)); event.handled = true; return;
    }
    if (event.action == PointerAction::down &&
        (event.button == PointerButton::primary ||
         event.button == PointerButton::secondary)) {
        if (window()) static_cast<void>((*window()).request_focus(shared_from_this()));
        pressed_index_ = index_at(event.position);
        pressed_click_count_ = event.click_count;
        pressed_button_ = event.button;
        if (pressed_index_) {
            if (event.button == PointerButton::secondary) {
                if (!is_selected(items_[*pressed_index_].stable_id)) {
                    select_index(*pressed_index_, false);
                } else {
                    focus_index(*pressed_index_);
                }
            } else {
                select_index(*pressed_index_, false, event.modifiers);
            }
        } else if (event.button == PointerButton::primary &&
                   !has_modifier(event.modifiers, Modifier::shift) &&
                   !has_modifier(event.modifiers, Modifier::control) &&
                   !has_modifier(event.modifiers, Modifier::meta)) {
            clear_selection();
        }
        // A deliberate background click belongs to the collection even when
        // it clears selection and has no item target.
        event.handled = true;
    } else if (event.action == PointerAction::up) {
        const std::optional<std::size_t> index = index_at(event.position);
        if (index && pressed_index_ == index && event.button == pressed_button_) {
            if (pressed_button_ == PointerButton::secondary) {
                context_requested_.emit({items_[*index].stable_id, event.position});
            } else if (pressed_button_ == PointerButton::primary &&
                       pressed_click_count_ >= 2U && items_[*index].enabled &&
                       is_alive()) {
                item_activated_.emit(items_[*index].stable_id);
            }
            event.handled = true;
        } else if (!index && !pressed_index_ &&
                   event.button == PointerButton::secondary &&
                   pressed_button_ == PointerButton::secondary && is_alive()) {
            clear_selection();
            context_requested_.emit({{}, event.position});
            event.handled = true;
        }
        if (event.button == pressed_button_ && pressed_button_ != PointerButton::none) {
            event.handled = true;
        }
        pressed_index_.reset();
        pressed_button_ = PointerButton::none;
    }
}

void ObjectView::on_key(KeyEvent& event) {
    if (!focused_ || !enabled() || event.action != KeyAction::down || items_.empty()) return;
    std::optional<std::size_t> current = item_index(focused_id_);
    if (!current) current = item_index(selected_id_);
    std::size_t index = current.value_or(0U);
    const bool toggle_modifier = has_modifier(event.modifiers, Modifier::control) ||
                                 has_modifier(event.modifiers, Modifier::meta);
    if (event.physical_key == PhysicalKey::a && toggle_modifier &&
        selection_mode_ == ObjectSelectionMode::multiple) {
        select_all();
        event.handled = true;
        return;
    }
    if (event.physical_key == PhysicalKey::enter) {
        if (items_[index].enabled && is_alive()) {
            item_activated_.emit(items_[index].stable_id);
        }
        event.handled = true;
        return;
    }
    if (event.physical_key == PhysicalKey::space) {
        select_index(index, false, event.modifiers);
        event.handled = true;
        return;
    }
    const std::size_t column_count = columns();
    if (event.physical_key == PhysicalKey::left && index > 0U) --index;
    else if (event.physical_key == PhysicalKey::right && index + 1U < items_.size()) ++index;
    else if (event.physical_key == PhysicalKey::up) {
        index = index >= column_count ? index - column_count : 0U;
    } else if (event.physical_key == PhysicalKey::down) {
        index = std::min(index + column_count, items_.size() - 1U);
    } else if (event.physical_key == PhysicalKey::home) index = 0U;
    else if (event.physical_key == PhysicalKey::end) index = items_.size() - 1U;
    else if (event.physical_key == PhysicalKey::page_up) {
        const std::size_t amount = visible_row_count() * column_count;
        index = index > amount ? index - amount : 0U;
    } else if (event.physical_key == PhysicalKey::page_down) {
        index = std::min(index + visible_row_count() * column_count, items_.size() - 1U);
    } else return;
    if (toggle_modifier && !has_modifier(event.modifiers, Modifier::shift)) {
        focus_index(index);
    } else {
        select_index(index, false, event.modifiers);
    }
    event.handled = true;
}

void ObjectView::type_select(std::string_view text) {
    const std::chrono::steady_clock::time_point now =
        std::chrono::steady_clock::now();
    if (now - last_type_time_ > type_timeout) type_prefix_.clear();
    last_type_time_ = now;
    type_prefix_ += fold_ascii(text);
    if (items_.empty()) return;
    const std::size_t start = (item_index(focused_id_).value_or(items_.size() - 1U) + 1U)
        % items_.size();
    for (std::size_t offset = 0; offset < items_.size(); ++offset) {
        const std::size_t index = (start + offset) % items_.size();
        if (fold_ascii(items_[index].name).starts_with(type_prefix_)) {
            select_index(index, false);
            return;
        }
    }
}

void ObjectView::on_text_input(TextInputEvent& event) {
    if (!focused_ || !enabled() || event.composing || event.text_utf8.empty() ||
        !validate_utf8(event.text_utf8).valid()) return;
    type_select(event.text_utf8);
    event.handled = true;
}

void ObjectView::on_focus_changed(bool focused) {
    focused_ = focused;
    if (focused_ && focused_id_.empty() && !items_.empty()) {
        focused_id_ = selected_id_.empty() ? items_.front().stable_id : selected_id_;
    }
    invalidate(Dirty::paint | Dirty::semantics);
}

SemanticDescriptor ObjectView::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::list;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    descriptor.value = selected_ids_.size() <= 1U
        ? selected_id_ : std::to_string(selected_ids_.size()) + " selected";
    descriptor.actions = {SemanticAction::focus};
    descriptor.exposed = true;
    return descriptor;
}

std::vector<SemanticNode> ObjectView::semantic_virtual_children() const {
    std::vector<SemanticNode> nodes;
    const std::size_t first = top_row_ * columns();
    const std::size_t end = std::min(items_.size(),
        (top_row_ + visible_row_count()) * columns());
    nodes.reserve(end - std::min(first, end));
    const Rect absolute = absolute_bounds();
    for (std::size_t index = first; index < end; ++index) {
        const ObjectViewItem& item = items_[index];
        const Rect local = item_bounds(index);
        SemanticNode node;
        node.stable_id = item.stable_id;
        node.runtime_id = virtual_runtime_id(node.stable_id);
        node.role = SemanticRole::list_item;
        node.name = item.name;
        node.value = item.secondary_text;
        node.description = item.description;
        node.bounds = {absolute.x + local.x, absolute.y + local.y,
                       local.width, local.height};
        node.states = SemanticState::visible | SemanticState::focusable;
        if (effectively_enabled() && item.enabled) node.states |= SemanticState::enabled;
        if (is_selected(item.stable_id)) node.states |= SemanticState::selected;
        if (focused_ && item.stable_id == focused_id_) node.states |= SemanticState::focused;
        node.actions = {SemanticAction::focus, SemanticAction::select,
                        SemanticAction::press, SemanticAction::show_menu};
        nodes.push_back(std::move(node));
    }
    return nodes;
}

bool ObjectView::on_semantic_child_action(std::string_view id,
                                          SemanticAction action,
                                          std::string_view) {
    const std::optional<std::size_t> index = item_index(id);
    if (!index || (action != SemanticAction::focus &&
                   action != SemanticAction::select &&
                   action != SemanticAction::press &&
                   action != SemanticAction::show_menu)) return false;
    if (window()) static_cast<void>((*window()).request_focus(shared_from_this()));
    if (action == SemanticAction::show_menu) {
        if (!is_selected(id)) select_index(*index, false);
        else focus_index(*index);
        const Rect local = item_bounds(*index);
        const Rect absolute = absolute_bounds();
        context_requested_.emit({std::string(id),
            {absolute.x + local.x + local.width * .5,
             absolute.y + local.y + local.height * .5}});
    } else if (action == SemanticAction::focus) {
        focus_index(*index);
    } else {
        select_index(*index, action == SemanticAction::press);
    }
    return true;
}


} // namespace gui_forms
