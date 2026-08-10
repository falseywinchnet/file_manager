#include "gui_forms/controls/scrollable_control/container_control/tab_control/tab_control.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>

namespace gui_forms {

namespace {
[[nodiscard]] std::uint64_t virtual_semantic_runtime_id(
    std::string_view stable_id) noexcept {
    std::uint64_t value = 1469598103934665603ULL;
    for (const unsigned char byte : stable_id) {
        value ^= byte;
        value *= 1099511628211ULL;
    }
    return value | (std::uint64_t{1} << 63U);
}

} // namespace

TabControl::TabControl(StableId stable_id)
    : ContainerControl(std::move(stable_id)) {
    set_focusable(true);
}

std::vector<std::shared_ptr<TabPage>> TabControl::pages() const {
    std::vector<std::shared_ptr<TabPage>> result;
    result.reserve(pages_.size());
    for (const std::weak_ptr<TabPage>& weak : pages_) {
        if (const std::shared_ptr<gui_forms::TabPage> page = weak.lock();
            page && (*page).is_alive() && (*page).parent().get() == this) {
            result.push_back(page);
        }
    }
    return result;
}

std::size_t TabControl::page_count() const {
    return pages().size();
}

std::shared_ptr<TabPage> TabControl::page_at(std::size_t index) const {
    const std::vector<std::shared_ptr<TabPage>> live = pages();
    if (index >= live.size()) throw std::out_of_range("TabControl page index");
    return live[index];
}

std::optional<std::size_t> TabControl::index_of(
    const std::shared_ptr<TabPage>& page) const {
    if (!page) return std::nullopt;
    const std::vector<std::shared_ptr<TabPage>> live = pages();
    const std::vector<std::shared_ptr<TabPage>>::const_iterator found =
        std::find(live.begin(), live.end(), page);
    if (found == live.end()) return std::nullopt;
    return static_cast<std::size_t>(found - live.begin());
}

std::optional<std::size_t> TabControl::selected_index() const {
    return index_of(selected_page_.lock());
}

void TabControl::add_page(std::shared_ptr<TabPage> page) {
    require_mutable();
    if (!page) throw std::invalid_argument("TabControl page may not be null");
    if (index_of(page)) return;
    (*page).set_visible(false);
    add_child(page);
    pages_.push_back(page);
    if (!selected_page_.lock()) {
        selected_page_ = page;
        (*page).set_visible(true);
        const TabSelectionChange change{std::nullopt, 0U};
        publish_change(selected_index_changed_, change);
    }
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
}

std::shared_ptr<TabPage> TabControl::remove_page(const TabPage& page) {
    require_mutable();
    const std::vector<std::shared_ptr<TabPage>> live_before = pages();
    const std::vector<std::shared_ptr<TabPage>>::const_iterator found =
        std::find_if(
        live_before.begin(), live_before.end(),
        [&page](const auto& candidate) { return candidate.get() == &page; });
    if (found == live_before.end()) return {};
    const std::size_t removed_index =
        static_cast<std::size_t>(found - live_before.begin());
    const std::optional<std::size_t> old_selected = selected_index();
    const bool removing_selected = selected_page_.lock().get() == &page;
    pages_.erase(std::remove_if(
        pages_.begin(), pages_.end(), [&page](const std::weak_ptr<TabPage>& weak) {
            const std::shared_ptr<gui_forms::TabPage> candidate = weak.lock();
            return !candidate || candidate.get() == &page;
        }), pages_.end());
    const Control::Ptr removed = remove_child(page.runtime_id());
    if (!removed) return {};

    const std::vector<std::shared_ptr<TabPage>> live_after = pages();
    if (removing_selected) {
        selected_page_.reset();
        if (!live_after.empty()) {
            const std::size_t next = std::min(removed_index, live_after.size() - 1U);
            selected_page_ = live_after[next];
            (*live_after[next]).set_visible(true);
        }
    }
    const std::optional<std::size_t> new_selected = selected_index();
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
    if (old_selected != new_selected || removing_selected) {
        const TabSelectionChange change{old_selected, new_selected};
        publish_change(selected_index_changed_, change);
    }
    return std::dynamic_pointer_cast<TabPage>(removed);
}

void TabControl::remember_page_focus(const std::shared_ptr<TabPage>& page) {
    if (!page || window() == nullptr) return;
    const Control::Ptr focused = (*window()).focused_control();
    if (!focused) return;
    for (Control::Ptr current = focused; current; current = (*current).parent()) {
        if (current == page) {
            remembered_focus_[(*page).runtime_id().value] = focused;
            return;
        }
    }
}

void TabControl::restore_page_focus(const std::shared_ptr<TabPage>& page,
                                    bool selection_owned_focus) {
    if (!selection_owned_focus || !page || window() == nullptr) return;
    const RememberedFocusMap::iterator found =
        remembered_focus_.find((*page).runtime_id().value);
    if (found != remembered_focus_.end()) {
        if (const Control::Ptr candidate = (*found).second.lock();
            candidate && (*candidate).eligible_for_input()) {
            for (Control::Ptr current = candidate; current; current = (*current).parent()) {
                if (current == page) {
                    if ((*window()).request_focus(candidate)) return;
                    break;
                }
            }
        }
    }
    static_cast<void>((*window()).request_focus(shared_from_this()));
}

void TabControl::set_selected_index(std::size_t index) {
    require_mutable();
    const std::vector<std::shared_ptr<TabPage>> live = pages();
    if (index >= live.size()) throw std::out_of_range("TabControl selected index");
    const std::optional<std::size_t> old_index = selected_index();
    if (old_index && *old_index == index) return;
    const std::shared_ptr<gui_forms::TabPage> old_page = selected_page_.lock();
    bool selection_owned_focus{};
    if (old_page && window() != nullptr) {
        const Control::Ptr focused = (*window()).focused_control();
        for (Control::Ptr current = focused; current; current = (*current).parent()) {
            if (current == old_page) {
                selection_owned_focus = true;
                break;
            }
        }
        remember_page_focus(old_page);
        (*old_page).set_visible(false);
    }
    selected_page_ = live[index];
    (*live[index]).set_visible(true);
    invalidate(Dirty::arrange | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
    restore_page_focus(live[index], selection_owned_focus);
    const TabSelectionChange change{old_index, index};
    publish_change(selected_index_changed_, change);
}

void TabControl::set_selected_tab(const std::shared_ptr<TabPage>& page) {
    const std::optional<std::size_t> index = index_of(page);
    if (!index) throw std::invalid_argument("TabPage does not belong to TabControl");
    set_selected_index(*index);
}

void TabControl::set_alignment(TabAlignment alignment) {
    require_mutable();
    if (alignment_ == alignment) return;
    alignment_ = alignment;
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
}

void TabControl::set_appearance(TabAppearance appearance) {
    require_mutable();
    if (appearance_ == appearance) return;
    appearance_ = appearance;
    invalidate(Dirty::paint | Dirty::semantics);
}

void TabControl::set_item_size(Size size) {
    require_mutable();
    if (!std::isfinite(size.width) || !std::isfinite(size.height) ||
        size.width < 24.0 || size.height < 18.0) {
        throw std::invalid_argument(
            "TabControl item size must be finite and at least 24 by 18");
    }
    if (item_size_ == size) return;
    item_size_ = size;
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
}

void TabControl::set_style(BasicControlStyle style) {
    require_mutable();
    if (style_ == style) return;
    style_ = std::move(style);
    invalidate(Dirty::paint | Dirty::semantics);
}

Rect TabControl::tab_bounds(std::size_t index) const {
    if (index >= page_count()) throw std::out_of_range("TabControl tab index");
    const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                      committed_arranged_bounds().height};
    const Size item{item_size_.width * effective_text_scale(),
                    item_size_.height * effective_text_scale()};
    if (alignment_ == TabAlignment::top || alignment_ == TabAlignment::bottom) {
        const double y = alignment_ == TabAlignment::top
            ? 0.0 : std::max(0.0, bounds.height - item.height);
        return {static_cast<double>(index) * item.width, y,
                item.width, std::min(item.height, bounds.height)};
    }
    const double x = alignment_ == TabAlignment::left
        ? 0.0 : std::max(0.0, bounds.width - item.width);
    return {x, static_cast<double>(index) * item.height,
            std::min(item.width, bounds.width), item.height};
}

Rect TabControl::display_bounds() const noexcept {
    const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                      committed_arranged_bounds().height};
    const Size item{item_size_.width * effective_text_scale(),
                    item_size_.height * effective_text_scale()};
    switch (alignment_) {
    case TabAlignment::top:
        return {0.0, std::min(bounds.height, item.height - 1.0),
                bounds.width,
                std::max(0.0, bounds.height - item.height + 1.0)};
    case TabAlignment::bottom:
        return {0.0, 0.0, bounds.width,
                std::max(0.0, bounds.height - item.height + 1.0)};
    case TabAlignment::left:
        return {std::min(bounds.width, item.width - 1.0), 0.0,
                std::max(0.0, bounds.width - item.width + 1.0),
                bounds.height};
    case TabAlignment::right:
        return {0.0, 0.0,
                std::max(0.0, bounds.width - item.width + 1.0),
                bounds.height};
    }
    return bounds;
}

Size TabControl::measure(Size available) {
    const Rect requested = requested_bounds();
    return {std::min(available.width,
                     requested.width > 0.0 ? requested.width : available.width),
            std::min(available.height,
                     requested.height > 0.0 ? requested.height : available.height)};
}

void TabControl::reconcile_pages() {
    pages_.erase(std::remove_if(
        pages_.begin(), pages_.end(), [this](const std::weak_ptr<TabPage>& weak) {
            const std::shared_ptr<gui_forms::TabPage> page = weak.lock();
            return !page || !(*page).is_alive() || (*page).parent().get() != this;
        }), pages_.end());
    std::shared_ptr<gui_forms::TabPage> selected = selected_page_.lock();
    if (selected && (*selected).is_alive() && (*selected).parent().get() == this) return;
    selected_page_.reset();
    const std::vector<std::shared_ptr<TabPage>> live = pages();
    if (!live.empty()) {
        selected_page_ = live.front();
        (*live.front()).set_visible(true);
    }
}

void TabControl::arrange(Rect final_bounds) {
    arrange_self(final_bounds);
    reconcile_pages();
    const Rect display = display_bounds();
    const std::shared_ptr<gui_forms::TabPage> selected = selected_page_.lock();
    for (const std::shared_ptr<gui_forms::TabPage>& page : pages()) {
        set_child_layout(page, display);
        (*page).set_visible(page == selected);
    }
}

void TabControl::on_paint(Painter& painter, Rect) {
    const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                      committed_arranged_bounds().height};
    painter.fill_rect(bounds, style_.face);
    const Rect display = display_bounds();
    painter.fill_rect(display, style_.paper);
    painter.stroke_rect({display.x + 0.5, display.y + 0.5,
                         std::max(0.0, display.width - 1.0),
                         std::max(0.0, display.height - 1.0)},
                        style_.border, 1.0);
    const std::vector<std::shared_ptr<TabPage>> live = pages();
    const FontSpec font = effective_font(font_);
    const std::optional<std::size_t> selected = selected_index();
    for (std::size_t index = 0U; index < live.size(); ++index) {
        Rect tab = tab_bounds(index);
        const bool active = selected && *selected == index;
        if (appearance_ == TabAppearance::buttons && !active) {
            tab = {tab.x + 2.0, tab.y + 2.0,
                   std::max(0.0, tab.width - 4.0),
                   std::max(0.0, tab.height - 4.0)};
        }
        const Color face = active ? style_.paper
            : appearance_ == TabAppearance::flat_buttons ? style_.face_light
                                                         : style_.face;
        painter.fill_rect(tab, face);
        painter.stroke_rect({tab.x + 0.5, tab.y + 0.5,
                             std::max(0.0, tab.width - 1.0),
                             std::max(0.0, tab.height - 1.0)},
                            active ? style_.dark_border : style_.border, 1.0);
        if (active && appearance_ == TabAppearance::normal) {
            if (alignment_ == TabAlignment::top) {
                painter.fill_rect({tab.x + 1.0, tab.y + tab.height - 2.0,
                                   std::max(0.0, tab.width - 2.0), 3.0},
                                  style_.paper);
            } else if (alignment_ == TabAlignment::bottom) {
                painter.fill_rect({tab.x + 1.0, tab.y - 1.0,
                                   std::max(0.0, tab.width - 2.0), 3.0},
                                  style_.paper);
            } else if (alignment_ == TabAlignment::left) {
                painter.fill_rect({tab.x + tab.width - 2.0, tab.y + 1.0, 3.0,
                                   std::max(0.0, tab.height - 2.0)}, style_.paper);
            } else {
                painter.fill_rect({tab.x - 1.0, tab.y + 1.0, 3.0,
                                   std::max(0.0, tab.height - 2.0)}, style_.paper);
            }
        }
        const double text_width = static_cast<double>((*live[index]).text().size()) *
                                  font.size * 0.55;
        painter.draw_text_utf8(
            {tab.x + std::max(5.0, (tab.width - text_width) * 0.5),
             tab.y + (tab.height + font.size) * 0.5 - 2.0},
            (*live[index]).text(), font, enabled() ? style_.text
                                                  : style_.disabled_text);
        if (active && focused_) {
            painter.stroke_rect({tab.x + 4.5, tab.y + 4.5,
                                 std::max(0.0, tab.width - 9.0),
                                 std::max(0.0, tab.height - 9.0)},
                                style_.accent, 1.0);
        }
    }
}

void TabControl::on_pointer(PointerEvent& event) {
    if (!eligible_for_input()) return;
    if (event.action == PointerAction::up &&
        event.button == PointerButton::primary && pointer_engaged_) {
        pointer_engaged_ = false;
        event.handled = true;
        return;
    }
    if (event.action != PointerAction::down ||
        event.button != PointerButton::primary) return;
    const Rect absolute = absolute_bounds();
    const Point local{event.position.x - absolute.x, event.position.y - absolute.y};
    const std::vector<std::shared_ptr<TabPage>> live = pages();
    for (std::size_t index = 0U; index < live.size(); ++index) {
        if (!tab_bounds(index).contains(local)) continue;
        if (window() != nullptr) {
            static_cast<void>((*window()).request_focus(shared_from_this()));
        }
        pointer_engaged_ = true;
        set_selected_index(index);
        event.handled = true;
        return;
    }
}

void TabControl::select_relative(int delta) {
    const std::vector<std::shared_ptr<TabPage>> live = pages();
    if (live.empty()) return;
    const std::size_t current = selected_index().value_or(0U);
    const std::ptrdiff_t count = static_cast<std::ptrdiff_t>(live.size());
    const std::ptrdiff_t next = (static_cast<std::ptrdiff_t>(current) + delta + count) % count;
    set_selected_index(static_cast<std::size_t>(next));
}

void TabControl::on_key_preview(KeyEvent& event) {
    if (event.action != KeyAction::down || !enabled() || page_count() == 0U ||
        window() == nullptr) return;
    const std::uint8_t modifiers = static_cast<std::uint8_t>(event.modifiers);
    const bool command =
        (modifiers & static_cast<std::uint8_t>(Modifier::control)) != 0U ||
        (modifiers & static_cast<std::uint8_t>(Modifier::meta)) != 0U;
    if (event.physical_key == PhysicalKey::tab && command) {
        const bool reverse =
            (modifiers & static_cast<std::uint8_t>(Modifier::shift)) != 0U;
        select_relative(reverse ? -1 : 1);
        event.handled = true;
        return;
    }
    if ((*window()).focused_control().get() != this) return;
    if (event.physical_key == PhysicalKey::home) {
        set_selected_index(0U);
    } else if (event.physical_key == PhysicalKey::end) {
        set_selected_index(page_count() - 1U);
    } else if ((alignment_ == TabAlignment::top ||
                alignment_ == TabAlignment::bottom) &&
               event.physical_key == PhysicalKey::left) {
        select_relative(-1);
    } else if ((alignment_ == TabAlignment::top ||
                alignment_ == TabAlignment::bottom) &&
               event.physical_key == PhysicalKey::right) {
        select_relative(1);
    } else if ((alignment_ == TabAlignment::left ||
                alignment_ == TabAlignment::right) &&
               event.physical_key == PhysicalKey::up) {
        select_relative(-1);
    } else if ((alignment_ == TabAlignment::left ||
                alignment_ == TabAlignment::right) &&
               event.physical_key == PhysicalKey::down) {
        select_relative(1);
    } else {
        return;
    }
    event.handled = true;
}

void TabControl::on_focus_changed(bool focused) {
    focused_ = focused;
    if (!focused) pointer_engaged_ = false;
    invalidate(Dirty::paint | Dirty::semantics);
}

SemanticDescriptor TabControl::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::tab_group;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    if (const std::shared_ptr<gui_forms::TabPage> selected = selected_page_.lock()) descriptor.value = (*selected).text();
    descriptor.actions = {SemanticAction::focus};
    descriptor.exposed = true;
    return descriptor;
}

std::vector<SemanticNode> TabControl::semantic_virtual_children() const {
    std::vector<SemanticNode> nodes;
    const std::vector<std::shared_ptr<TabPage>> live = pages();
    nodes.reserve(live.size());
    const Rect absolute = absolute_bounds();
    const std::shared_ptr<gui_forms::TabPage> selected = selected_page_.lock();
    for (std::size_t index = 0U; index < live.size(); ++index) {
        SemanticNode node;
        node.stable_id = std::string((*live[index]).stable_id().value()) + ".tab";
        node.runtime_id = virtual_semantic_runtime_id(node.stable_id);
        node.role = SemanticRole::tab;
        node.name = (*live[index]).text();
        const Rect local = tab_bounds(index);
        node.bounds = {absolute.x + local.x, absolute.y + local.y,
                       local.width, local.height};
        if (effectively_enabled()) node.states |= SemanticState::enabled;
        node.states |= SemanticState::visible | SemanticState::focusable;
        if (live[index] == selected) node.states |= SemanticState::selected;
        node.actions = {SemanticAction::focus, SemanticAction::select,
                        SemanticAction::press};
        nodes.push_back(std::move(node));
    }
    return nodes;
}

bool TabControl::on_semantic_child_action(std::string_view child_stable_id,
                                          SemanticAction action,
                                          std::string_view) {
    if (action != SemanticAction::focus && action != SemanticAction::select &&
        action != SemanticAction::press) return false;
    const std::vector<std::shared_ptr<TabPage>> live = pages();
    for (std::size_t index = 0U; index < live.size(); ++index) {
        if (child_stable_id !=
            std::string((*live[index]).stable_id().value()) + ".tab") continue;
        if (window() != nullptr) {
            static_cast<void>((*window()).request_focus(shared_from_this()));
        }
        set_selected_index(index);
        return true;
    }
    return false;
}

} // namespace gui_forms
