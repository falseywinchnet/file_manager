#include "gui_forms/controls/panel/list_box/checked_list_box/checked_list_box.hpp"

#include "gui_forms/detail/algorithm/binary_search.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace gui_forms {

CheckedListBox::CheckedListBox(StableId stable_id)
    : ListBox(std::move(stable_id)) {}

void CheckedListBox::set_items(std::vector<std::string> items) {
    const std::vector<CheckState> previous = check_states_;
    check_states_.assign(items.size(), CheckState::unchecked);
    try {
        ListBox::set_items(std::move(items));
    } catch (...) {
        // If ListBox rejected the new collection before mutation, restore the
        // matching check model. If a user callback threw after mutation, the
        // new sizes already match and are the only coherent retained state.
        if ((*this).items().size() == previous.size()) check_states_ = previous;
        throw;
    }
    invalidate(Dirty::paint | Dirty::semantics);
}

void CheckedListBox::add_item(std::string item) {
    add_item(std::move(item), CheckState::unchecked);
}

void CheckedListBox::add_item(std::string item, CheckState state) {
    if (state != CheckState::unchecked && state != CheckState::checked &&
        state != CheckState::indeterminate) {
        throw std::invalid_argument("invalid CheckedListBox item state");
    }
    ListBox::add_item(std::move(item));
    try {
        check_states_.push_back(state);
    } catch (...) {
        ListBox::remove_item(items().size() - 1U);
        throw;
    }
    invalidate(Dirty::paint | Dirty::semantics);
}

void CheckedListBox::remove_item(std::size_t index) {
    if (index >= check_states_.size()) {
        throw std::out_of_range("CheckedListBox item index");
    }
    check_states_.erase(check_states_.begin() + static_cast<std::ptrdiff_t>(index));
    ListBox::remove_item(index);
    invalidate(Dirty::paint | Dirty::semantics);
}

void CheckedListBox::clear_items() {
    ListBox::clear_items();
    check_states_.clear();
    invalidate(Dirty::paint | Dirty::semantics);
}

CheckState CheckedListBox::item_check_state(std::size_t index) const {
    if (index >= check_states_.size()) {
        throw std::out_of_range("CheckedListBox item index");
    }
    return check_states_[index];
}

bool CheckedListBox::item_checked(std::size_t index) const {
    return item_check_state(index) == CheckState::checked;
}

void CheckedListBox::set_item_check_state(std::size_t index,
                                          CheckState state) {
    require_mutable();
    if (index >= check_states_.size()) {
        throw std::out_of_range("CheckedListBox item index");
    }
    if (state != CheckState::unchecked && state != CheckState::checked &&
        state != CheckState::indeterminate) {
        throw std::invalid_argument("invalid CheckedListBox item state");
    }
    if (check_states_[index] == state) return;
    ItemCheckEvent change{index, check_states_[index], state, false};
    item_checking_.emit(change);
    if (!is_alive() || change.cancel) return;
    if (change.new_state != CheckState::unchecked &&
        change.new_state != CheckState::checked &&
        change.new_state != CheckState::indeterminate) {
        throw std::invalid_argument(
            "CheckedListBox ItemCheck callback produced an invalid state");
    }
    if (check_states_[index] == change.new_state) return;
    check_states_[index] = change.new_state;
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(item_check_state_changed_, index, check_states_[index]);
}

void CheckedListBox::set_item_checked(std::size_t index, bool checked) {
    set_item_check_state(index, checked ? CheckState::checked
                                        : CheckState::unchecked);
}

void CheckedListBox::toggle_item(std::size_t index) {
    const CheckState current = item_check_state(index);
    set_item_check_state(index, current == CheckState::checked
                                    ? CheckState::unchecked
                                    : CheckState::checked);
}

std::vector<std::size_t> CheckedListBox::checked_indices() const {
    std::vector<std::size_t> result;
    for (std::size_t index = 0U; index < check_states_.size(); ++index) {
        if (check_states_[index] == CheckState::checked) result.push_back(index);
    }
    return result;
}

void CheckedListBox::set_check_on_click(bool enabled) {
    require_mutable();
    if (check_on_click_ == enabled) return;
    check_on_click_ = enabled;
    invalidate(Dirty::semantics);
}

void CheckedListBox::set_indicator_size(double size) {
    require_mutable();
    if (!std::isfinite(size) || size < 8.0 || size > 32.0) {
        throw std::out_of_range(
            "CheckedListBox indicator size must be finite and between 8 and 32");
    }
    if (indicator_size_ == size) return;
    indicator_size_ = size;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void CheckedListBox::on_pointer(PointerEvent& event) {
    if (!eligible_for_input()) {
        ListBox::on_pointer(event);
        return;
    }
    const std::optional<std::size_t> index = event.action == PointerAction::down &&
                           event.button == PointerButton::primary
        ? index_at(event.position) : std::optional<std::size_t>{};
    const bool was_selected = index && detail::binary_search_contains(
        selected_indices(), *index);
    ListBox::on_pointer(event);
    if (!is_alive() || !index || *index >= items().size()) return;
    if (check_on_click_ || was_selected) {
        toggle_item(*index);
        event.handled = true;
    }
}

void CheckedListBox::on_key(KeyEvent& event) {
    if (event.action == KeyAction::down &&
        event.physical_key == PhysicalKey::space && focused_for_extension() &&
        enabled()) {
        if (const std::optional<std::size_t> index = active_index_for_extension()) {
            toggle_item(*index);
            event.handled = true;
        }
        return;
    }
    ListBox::on_key(event);
}

double CheckedListBox::row_text_left() const noexcept {
    return indicator_size_ + 14.0;
}

void CheckedListBox::paint_row_adornment(Painter& painter, std::size_t index,
                                          Rect row, bool selected,
                                          bool focused) const {
    if (index >= check_states_.size()) return;
    const double size = std::min(indicator_size_,
                                 std::max(8.0, row.height - 8.0));
    const double x = row.x + 5.0;
    const double y = row.y + (row.height - size) * 0.5;
    const Color paper = selected && focused ? style().highlight : style().paper;
    const Color mark = selected && focused ? style().accent : style().accent;
    painter.fill_rect({x, y, size, size}, paper);
    painter.stroke_rect({x + 0.5, y + 0.5, size - 1.0, size - 1.0},
                        style().dark_border, 1.0);
    if (check_states_[index] == CheckState::checked) {
        painter.draw_line({x + 3.0, y + size * 0.52},
                          {x + size * 0.44, y + size - 3.0}, mark, 2.0);
        painter.draw_line({x + size * 0.44, y + size - 3.0},
                          {x + size - 2.5, y + 2.5}, mark, 2.0);
    } else if (check_states_[index] == CheckState::indeterminate) {
        painter.fill_rect({x + 3.0, y + size * 0.5 - 1.5,
                           std::max(0.0, size - 6.0), 3.0}, mark);
    }
}

std::vector<SemanticNode> CheckedListBox::semantic_virtual_children() const {
    std::vector<SemanticNode> nodes = ListBox::semantic_virtual_children();
    for (std::size_t index = 0U;
         index < nodes.size() && index < check_states_.size(); ++index) {
        nodes[index].role = SemanticRole::check_list_item;
        if (check_states_[index] == CheckState::checked) {
            nodes[index].states |= SemanticState::checked;
            nodes[index].value = "checked";
        } else if (check_states_[index] == CheckState::indeterminate) {
            nodes[index].states |= SemanticState::mixed;
            nodes[index].value = "indeterminate";
        } else {
            nodes[index].value = "unchecked";
        }
    }
    return nodes;
}

bool CheckedListBox::on_semantic_child_action(std::string_view stable_id,
                                               SemanticAction action,
                                               std::string_view value) {
    if (action != SemanticAction::press) {
        return ListBox::on_semantic_child_action(stable_id, action, value);
    }
    const std::string prefix = std::string((*this).stable_id().value()) + ".item.";
    if (!stable_id.starts_with(prefix)) return false;
    const std::string_view suffix = stable_id.substr(prefix.size());
    std::size_t index{};
    const std::from_chars_result parsed = std::from_chars(suffix.data(), suffix.data() + suffix.size(),
                                        index);
    if (parsed.ec != std::errc{} || parsed.ptr != suffix.data() + suffix.size() ||
        index >= items().size()) return false;
    if (window() != nullptr) {
        static_cast<void>((*window()).request_focus(shared_from_this()));
    }
    select_index(index, false, false);
    if (is_alive()) toggle_item(index);
    return true;
}

} // namespace gui_forms
