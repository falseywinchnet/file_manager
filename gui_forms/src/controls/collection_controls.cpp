#include "gui_forms/collection_controls.hpp"

#include "gui_forms/text.hpp"
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

constexpr std::chrono::milliseconds type_timeout{850};

std::uint64_t virtual_runtime_id(std::string_view id) noexcept {
    std::uint64_t hash = 1469598103934665603ULL;
    for (const unsigned char byte : id) {
        hash ^= byte;
        hash *= 1099511628211ULL;
    }
    return hash | (1ULL << 63U);
}

std::string fold_ascii(std::string_view text) {
    std::string result(text);
    for (char& character : result) {
        if (character >= 'A' && character <= 'Z') {
            character = static_cast<char>(character - 'A' + 'a');
        }
    }
    return result;
}

void validate_identity_text(std::string_view id, std::string_view text) {
    if (id.empty() || !validate_utf8(id).valid() || !validate_utf8(text).valid()) {
        throw std::invalid_argument("collection item identity and text must be valid UTF-8");
    }
}

Color with_alpha(Color color, std::uint8_t alpha) noexcept {
    color.alpha = alpha;
    return color;
}

void draw_emphasized_text(Painter& painter, Point origin,
                          std::string_view text,
                          std::span<const std::string> terms,
                          FontSpec font, Color foreground,
                          Color mark) {
    if (terms.empty() || text.empty()) {
        painter.draw_text_utf8(origin, text, font, foreground);
        return;
    }
    const std::string folded = fold_ascii(text);
    std::size_t cursor{};
    double x = origin.x;
    while (cursor < text.size()) {
        std::size_t next = text.size();
        std::size_t length{};
        for (const std::string& term : terms) {
            if (term.empty()) continue;
            const std::string folded_term = fold_ascii(term);
            const std::size_t found = folded.find(folded_term, cursor);
            if (found < next || (found == next && folded_term.size() > length)) {
                next = found;
                length = folded_term.size();
            }
        }
        if (next == std::string::npos || next >= text.size()) {
            const std::string_view tail = text.substr(cursor);
            painter.draw_text_utf8({x, origin.y}, tail, font, foreground);
            break;
        }
        if (next > cursor) {
            const std::string_view prefix = text.substr(cursor, next - cursor);
            painter.draw_text_utf8({x, origin.y}, prefix, font, foreground);
            x += painter.measure_text_utf8(prefix, font).width;
        }
        const std::string_view emphasized = text.substr(next, length);
        const double width = painter.measure_text_utf8(emphasized, font).width;
        painter.fill_rect({x - 1.0, origin.y - font.size,
                           width + 2.0, font.size + 3.0}, mark);
        painter.draw_text_utf8({x, origin.y}, emphasized, font, foreground);
        x += width;
        cursor = next + length;
    }
}

} // namespace

TreeView::TreeView(StableId stable_id) : Panel(std::move(stable_id)) {
    set_paint_plane(PaintPlane::control);
    set_background(style().paper);
    set_border_style(BorderStyle::sunken);
    set_focusable(true);
}

void TreeView::set_items(std::vector<TreeViewItem> items) {
    require_mutable();
    std::unordered_set<std::string> identities;
    for (std::size_t index = 0; index < items.size(); ++index) {
        validate_identity_text(items[index].stable_id, items[index].text);
        if (!identities.insert(items[index].stable_id).second ||
            (index == 0U && items[index].depth != 0U) ||
            (index != 0U && items[index].depth > items[index - 1U].depth + 1U)) {
            throw std::invalid_argument("TreeView requires unique IDs and a valid preorder depth sequence");
        }
    }
    const std::string retained_selection = selected_id_;
    const std::string retained_active = active_id_;
    items_ = std::move(items);
    selected_id_ = item_index(retained_selection) ? retained_selection : std::string{};
    active_id_ = item_index(retained_active) ? retained_active : std::string{};
    rebuild_visible();
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

std::optional<std::size_t> TreeView::item_index(std::string_view id) const noexcept {
    const auto found = std::find_if(items_.begin(), items_.end(),
        [id](const TreeViewItem& item) { return item.stable_id == id; });
    return found == items_.end() ? std::optional<std::size_t>{}
                                : std::optional<std::size_t>{static_cast<std::size_t>(
                                      std::distance(items_.begin(), found))};
}

void TreeView::set_selected_id(std::string_view id) {
    require_mutable();
    if (!id.empty() && !item_index(id)) {
        throw std::out_of_range("TreeView selection ID is not in the model");
    }
    const std::string next(id);
    if (selected_id_ == next) return;
    TreeSelectionChange change{selected_id_, next};
    selected_id_ = next;
    if (!next.empty()) active_id_ = next;
    invalidate(Dirty::paint | Dirty::semantics);
    selection_changed_.emit(change);
}

bool TreeView::expanded(std::string_view id) const {
    const auto index = item_index(id);
    if (!index) throw std::out_of_range("TreeView expansion ID is not in the model");
    return items_[*index].expanded;
}

void TreeView::set_expanded(std::string_view id, bool value) {
    require_mutable();
    const auto index = item_index(id);
    if (!index) throw std::out_of_range("TreeView expansion ID is not in the model");
    TreeViewItem& item = items_[*index];
    if (!item.expandable || item.expanded == value) return;
    bool selected_descendant{};
    if (!value) {
        if (const auto selected = item_index(selected_id_);
            selected && *selected > *index && items_[*selected].depth > item.depth) {
            selected_descendant = true;
        }
    }
    item.expanded = value;
    rebuild_visible();
    invalidate(Dirty::paint | Dirty::hit_test | Dirty::semantics);
    expansion_changed_.emit({item.stable_id, value});
    if (selected_descendant && is_alive()) set_selected_id(item.stable_id);
}

void TreeView::set_item_height(double height) {
    require_mutable();
    if (!std::isfinite(height) || height < 14.0 || height > 256.0) {
        throw std::invalid_argument("TreeView item height must be finite and between 14 and 256");
    }
    if (item_height_ == height) return;
    item_height_ = height;
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

void TreeView::set_indentation(double indentation) {
    require_mutable();
    if (!std::isfinite(indentation) || indentation < 8.0 || indentation > 96.0) {
        throw std::invalid_argument("TreeView indentation must be finite and between 8 and 96");
    }
    if (indentation_ == indentation) return;
    indentation_ = indentation;
    invalidate(Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

void TreeView::set_top_row(std::size_t row) {
    require_mutable();
    if (visible_.empty()) {
        if (row != 0U) throw std::out_of_range("empty TreeView top row must be zero");
    } else if (row >= visible_.size()) {
        throw std::out_of_range("TreeView top row is outside the visible model");
    }
    if (top_row_ == row) return;
    top_row_ = row;
    invalidate(Dirty::paint | Dirty::semantics);
}

void TreeView::set_font(FontSpec font) {
    require_mutable();
    if (!valid_font_spec(font)) throw std::invalid_argument("TreeView font is invalid");
    if (font_ == font) return;
    font_ = font;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void TreeView::rebuild_visible() {
    visible_.clear();
    std::optional<std::size_t> collapsed_depth;
    for (std::size_t index = 0; index < items_.size(); ++index) {
        const TreeViewItem& item = items_[index];
        if (collapsed_depth && item.depth > *collapsed_depth) continue;
        collapsed_depth.reset();
        visible_.push_back(index);
        if (item.expandable && !item.expanded) collapsed_depth = item.depth;
    }
    top_row_ = visible_.empty() ? 0U : std::min(top_row_, visible_.size() - 1U);
    hovered_row_.reset();
    pressed_row_.reset();
    pressed_expander_ = false;
}

std::size_t TreeView::visible_row_count() const noexcept {
    double height = local_bounds().height;
    if (height <= 4.0) height = requested_bounds().height;
    const double row_height = item_height_ * effective_text_scale();
    return std::max<std::size_t>(1U, static_cast<std::size_t>(
        std::floor(std::max(0.0, height - 4.0) / row_height)));
}

std::optional<std::size_t> TreeView::visible_row_at(Point absolute) const noexcept {
    const Rect bounds = absolute_bounds();
    if (!bounds.contains(absolute) || absolute.y < bounds.y + 2.0) return {};
    const double row_height = item_height_ * effective_text_scale();
    const std::size_t row = top_row_ + static_cast<std::size_t>(
        std::floor((absolute.y - bounds.y - 2.0) / row_height));
    return row < visible_.size() ? std::optional<std::size_t>{row}
                                 : std::optional<std::size_t>{};
}

void TreeView::ensure_visible(std::size_t row) {
    const std::size_t count = visible_row_count();
    if (row < top_row_) top_row_ = row;
    else if (row >= top_row_ + count) top_row_ = row - count + 1U;
}

void TreeView::select_visible_row(std::size_t row, bool activate) {
    if (row >= visible_.size()) return;
    const TreeViewItem& item = items_[visible_[row]];
    if (!item.enabled) return;
    active_id_ = item.stable_id;
    set_selected_id(item.stable_id);
    ensure_visible(row);
    if (activate && is_alive()) item_activated_.emit(item.stable_id);
}

void TreeView::arrange(Rect final_bounds) {
    Panel::arrange(final_bounds);
    if (const auto active = item_index(active_id_)) {
        const auto row = std::find(visible_.begin(), visible_.end(), *active);
        if (row != visible_.end()) ensure_visible(static_cast<std::size_t>(
            std::distance(visible_.begin(), row)));
    }
}

void TreeView::on_paint(Painter& painter, Rect damage) {
    Panel::on_paint(painter, damage);
    const Rect bounds = local_bounds();
    const FontSpec font = effective_font(font_);
    const double text_scale = effective_text_scale();
    const double row_height = item_height_ * text_scale;
    painter.save();
    painter.clip_rect({2.0, 2.0, std::max(0.0, bounds.width - 4.0),
                       std::max(0.0, bounds.height - 4.0)});
    const std::size_t end = std::min(visible_.size(), top_row_ + visible_row_count() + 1U);
    for (std::size_t row = top_row_; row < end; ++row) {
        const TreeViewItem& item = items_[visible_[row]];
        const double y = 2.0 + static_cast<double>(row - top_row_) * row_height;
        const bool selected = item.stable_id == selected_id_;
        const bool active = focused_ && item.stable_id == active_id_;
        if (selected) {
            painter.fill_rect({2.0, y, std::max(0.0, bounds.width - 4.0), row_height},
                              focused_ ? style().accent : style().accent_light);
        } else if (hovered_row_ == row) {
            painter.fill_rect({2.0, y, std::max(0.0, bounds.width - 4.0), row_height},
                              style().face_light);
        }
        const double indent = 7.0 + static_cast<double>(item.depth) *
            indentation_ * text_scale;
        if (item.expandable) {
            const Color arrow = item.enabled ? style().dark_border : style().disabled_text;
            if (item.expanded) {
                painter.draw_line({indent, y + 9.0}, {indent + 4.0, y + 13.0}, arrow, 1.4);
                painter.draw_line({indent + 4.0, y + 13.0}, {indent + 8.0, y + 9.0}, arrow, 1.4);
            } else {
                painter.draw_line({indent + 2.0, y + 7.0}, {indent + 6.0, y + 11.0}, arrow, 1.4);
                painter.draw_line({indent + 6.0, y + 11.0}, {indent + 2.0, y + 15.0}, arrow, 1.4);
            }
        }
        const Color text = !item.enabled ? style().disabled_text
            : selected && focused_ ? style().highlight : style().text;
        painter.draw_text_utf8({indent + 14.0,
                                y + std::max(font.size, row_height * 0.5 + 4.0)},
                               item.text, font, text);
        if (active) {
            painter.stroke_rect({3.5, y + 1.5, std::max(0.0, bounds.width - 7.0),
                                 std::max(0.0, row_height - 3.0)},
                                selected ? style().highlight : style().accent, 1.0);
        }
    }
    painter.restore();
}

void TreeView::on_pointer(PointerEvent& event) {
    if (!eligible_for_input()) return;
    if (event.action == PointerAction::move) {
        const auto row = visible_row_at(event.position);
        if (row != hovered_row_) { hovered_row_ = row; invalidate(Dirty::paint); }
        return;
    }
    if (event.action == PointerAction::leave) {
        hovered_row_.reset();
        invalidate(Dirty::paint);
        return;
    }
    if (event.action == PointerAction::wheel && !visible_.empty() &&
        event.wheel_delta.y != 0.0) {
        const std::ptrdiff_t next = std::clamp<std::ptrdiff_t>(
            static_cast<std::ptrdiff_t>(top_row_) +
                (event.wheel_delta.y > 0.0 ? -3 : 3),
            0, static_cast<std::ptrdiff_t>(visible_.size() - 1U));
        set_top_row(static_cast<std::size_t>(next));
        event.handled = true;
        return;
    }
    if (event.action == PointerAction::down && event.button == PointerButton::primary) {
        if (window()) static_cast<void>(window()->request_focus(shared_from_this()));
        pressed_row_ = visible_row_at(event.position);
        pressed_click_count_ = event.click_count;
        pressed_expander_ = false;
        if (!pressed_row_) return;
        const TreeViewItem& item = items_[visible_[*pressed_row_]];
        const double local_x = event.position.x - absolute_bounds().x;
        const double arrow_x = 7.0 + static_cast<double>(item.depth) *
            indentation_ * effective_text_scale();
        if (item.expandable && local_x >= arrow_x - 3.0 && local_x <= arrow_x + 12.0) {
            pressed_expander_ = true;
            set_expanded(item.stable_id, !item.expanded);
        } else {
            select_visible_row(*pressed_row_, false);
        }
        event.handled = true;
    } else if (event.action == PointerAction::up &&
               event.button == PointerButton::primary) {
        const auto row = visible_row_at(event.position);
        if (row && pressed_row_ == row && !pressed_expander_) {
            select_visible_row(*row, pressed_click_count_ >= 2U);
        }
        pressed_row_.reset();
        pressed_expander_ = false;
        event.handled = row.has_value();
    }
}

void TreeView::on_key(KeyEvent& event) {
    if (!focused_ || !enabled() || event.action != KeyAction::down || visible_.empty()) return;
    auto active_index_value = item_index(active_id_);
    auto active_row_iterator = active_index_value
        ? std::find(visible_.begin(), visible_.end(), *active_index_value)
        : visible_.end();
    std::size_t row = active_row_iterator == visible_.end() ? 0U
        : static_cast<std::size_t>(std::distance(visible_.begin(), active_row_iterator));
    if (event.physical_key == PhysicalKey::enter) {
        select_visible_row(row, true); event.handled = true; return;
    }
    if (event.physical_key == PhysicalKey::up && row > 0U) --row;
    else if (event.physical_key == PhysicalKey::down && row + 1U < visible_.size()) ++row;
    else if (event.physical_key == PhysicalKey::home) row = 0U;
    else if (event.physical_key == PhysicalKey::end) row = visible_.size() - 1U;
    else if (event.physical_key == PhysicalKey::page_up) {
        row = row > visible_row_count() ? row - visible_row_count() : 0U;
    } else if (event.physical_key == PhysicalKey::page_down) {
        row = std::min(row + visible_row_count(), visible_.size() - 1U);
    } else if (event.physical_key == PhysicalKey::right) {
        TreeViewItem& item = items_[visible_[row]];
        if (item.expandable && !item.expanded) set_expanded(item.stable_id, true);
        else if (row + 1U < visible_.size() &&
                 items_[visible_[row + 1U]].depth > item.depth) ++row;
    } else if (event.physical_key == PhysicalKey::left) {
        TreeViewItem& item = items_[visible_[row]];
        if (item.expandable && item.expanded) set_expanded(item.stable_id, false);
        else if (item.depth > 0U) {
            while (row > 0U && items_[visible_[row]].depth >= item.depth) --row;
        }
    } else return;
    select_visible_row(row, false);
    event.handled = true;
}

void TreeView::type_select(std::string_view text) {
    const auto now = std::chrono::steady_clock::now();
    if (now - last_type_time_ > type_timeout) type_prefix_.clear();
    last_type_time_ = now;
    type_prefix_ += fold_ascii(text);
    if (visible_.empty()) return;
    std::size_t start{};
    if (const auto active = item_index(active_id_)) {
        const auto found = std::find(visible_.begin(), visible_.end(), *active);
        if (found != visible_.end()) start = (static_cast<std::size_t>(
            std::distance(visible_.begin(), found)) + 1U) % visible_.size();
    }
    for (std::size_t offset = 0; offset < visible_.size(); ++offset) {
        const std::size_t row = (start + offset) % visible_.size();
        if (fold_ascii(items_[visible_[row]].text).starts_with(type_prefix_)) {
            select_visible_row(row, false);
            return;
        }
    }
}

void TreeView::on_text_input(TextInputEvent& event) {
    if (!focused_ || !enabled() || event.composing || event.text_utf8.empty() ||
        !validate_utf8(event.text_utf8).valid()) return;
    type_select(event.text_utf8);
    event.handled = true;
}

void TreeView::on_focus_changed(bool focused) {
    focused_ = focused;
    if (focused_ && active_id_.empty() && !visible_.empty()) {
        active_id_ = selected_id_.empty() ? items_[visible_.front()].stable_id : selected_id_;
    }
    invalidate(Dirty::paint | Dirty::semantics);
}

SemanticDescriptor TreeView::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::list;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    descriptor.value = selected_id_;
    descriptor.actions = {SemanticAction::focus};
    descriptor.exposed = true;
    return descriptor;
}

std::vector<SemanticNode> TreeView::semantic_virtual_children() const {
    std::vector<SemanticNode> nodes;
    const std::size_t end = std::min(visible_.size(), top_row_ + visible_row_count());
    nodes.reserve(end - std::min(top_row_, end));
    const Rect tree_bounds = absolute_bounds();
    const double row_height = item_height_ * effective_text_scale();
    for (std::size_t row = top_row_; row < end; ++row) {
        const TreeViewItem& item = items_[visible_[row]];
        SemanticNode node;
        node.stable_id = item.stable_id;
        node.runtime_id = virtual_runtime_id(node.stable_id);
        node.role = SemanticRole::list_item;
        node.name = item.text;
        node.description = "level " + std::to_string(item.depth + 1U);
        node.bounds = {tree_bounds.x + 2.0,
                       tree_bounds.y + 2.0 +
                           static_cast<double>(row - top_row_) * row_height,
                       std::max(0.0, tree_bounds.width - 4.0), row_height};
        node.states = SemanticState::visible | SemanticState::focusable;
        if (effectively_enabled() && item.enabled) node.states |= SemanticState::enabled;
        if (item.stable_id == selected_id_) node.states |= SemanticState::selected;
        if (focused_ && item.stable_id == active_id_) node.states |= SemanticState::focused;
        if (item.expandable && item.expanded) node.states |= SemanticState::expanded;
        node.actions = {SemanticAction::focus, SemanticAction::select, SemanticAction::press};
        if (item.expandable) node.actions.push_back(item.expanded
            ? SemanticAction::collapse : SemanticAction::expand);
        nodes.push_back(std::move(node));
    }
    return nodes;
}

bool TreeView::on_semantic_child_action(std::string_view id,
                                        SemanticAction action,
                                        std::string_view) {
    const auto index = item_index(id);
    if (!index) return false;
    const auto row = std::find(visible_.begin(), visible_.end(), *index);
    if (row == visible_.end()) return false;
    if (action == SemanticAction::expand || action == SemanticAction::collapse) {
        set_expanded(id, action == SemanticAction::expand);
        return true;
    }
    if (action != SemanticAction::focus && action != SemanticAction::select &&
        action != SemanticAction::press) return false;
    if (window()) static_cast<void>(window()->request_focus(shared_from_this()));
    select_visible_row(static_cast<std::size_t>(std::distance(visible_.begin(), row)),
                       action == SemanticAction::press);
    return true;
}

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
        selection_changed_.emit(change);
    }
}

std::optional<std::size_t> ObjectView::item_index(std::string_view id) const noexcept {
    const auto found = std::find_if(items_.begin(), items_.end(),
        [id](const ObjectViewItem& item) { return item.stable_id == id; });
    return found == items_.end() ? std::optional<std::size_t>{}
                                : std::optional<std::size_t>{static_cast<std::size_t>(
                                      std::distance(items_.begin(), found))};
}

void ObjectView::set_view_mode(ObjectViewMode mode) {
    require_mutable();
    if (view_mode_ == mode) return;
    const auto selected = item_index(selected_id_);
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
    if (const auto index = item_index(primary)) ensure_visible(*index);
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
    selection_changed_.emit(change);
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
        const auto found = std::find(ids.begin(), ids.end(), target);
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
    if (const auto focused = item_index(focused_id_)) ensure_visible(*focused);
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
            paint_glyph(painter,
                        {cell.x + (cell.width - glyph_width) * .5, cell.y + 4.0,
                         glyph_width, 43.0}, item.glyph, item.enabled);
            const Size measured = painter.measure_text_utf8(item.name, font);
            const double text_x = cell.x + std::max(4.0, (cell.width - measured.width) * .5);
            painter.draw_text_utf8({text_x, cell.y +
                                    std::max(65.0, 48.0 + font.size)},
                                   item.name, font,
                                   item.enabled ? style().text : style().disabled_text);
            if (!item.secondary_text.empty()) {
                FontSpec authored_secondary{font_.role,
                                            std::max(8.0, font_.size - 1.5),
                                            400, false};
                const FontSpec secondary = effective_font(authored_secondary);
                painter.draw_text_utf8({cell.x + 5.0, cell.y + cell.height - 6.0},
                                       item.secondary_text, secondary,
                                       style().disabled_text);
            }
        } else {
            paint_glyph(painter, {cell.x + 6.0, cell.y + 3.0, 24.0,
                                   std::max(18.0, cell.height - 6.0)},
                        item.glyph, item.enabled);
            painter.draw_text_utf8({cell.x + 38.0, cell.y + cell.height * .5 + 4.0},
                                   item.name, font, item.enabled ? style().text
                                                                 : style().disabled_text);
            if (!item.secondary_text.empty()) {
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
        const auto next = index_at(event.position);
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
        if (window()) static_cast<void>(window()->request_focus(shared_from_this()));
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
        const auto index = index_at(event.position);
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
    auto current = item_index(focused_id_);
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
    const auto now = std::chrono::steady_clock::now();
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
    const auto index = item_index(id);
    if (!index || (action != SemanticAction::focus &&
                   action != SemanticAction::select &&
                   action != SemanticAction::press &&
                   action != SemanticAction::show_menu)) return false;
    if (window()) static_cast<void>(window()->request_focus(shared_from_this()));
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
        selection_changed_.emit({selection_change.previous_id, selected_id_});
    }
    if (pin_change.previous_id != pinned_id_) {
        pin_changed_.emit({pin_change.previous_id, pinned_id_});
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
    selection_changed_.emit(change);
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
    expansion_changed_.emit({std::string(stable_id), after,
                             stable_id == pinned_id_, reason});
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
    pin_changed_.emit({previous, pinned_id_});
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
        painter.fill_rect({row.x + 1.0, row.y + 1.0, 4.0,
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
