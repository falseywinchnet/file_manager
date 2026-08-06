#include "gui_forms/container_controls.hpp"

#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>

namespace gui_forms {
namespace {

void require_finite_nonnegative(double value, const char* message) {
    if (!std::isfinite(value) || value < 0.0) {
        throw std::invalid_argument(message);
    }
}

[[nodiscard]] std::uint64_t virtual_semantic_runtime_id(
    std::string_view stable_id) noexcept {
    std::uint64_t value = 1469598103934665603ULL;
    for (const unsigned char byte : stable_id) {
        value ^= byte;
        value *= 1099511628211ULL;
    }
    return value | (std::uint64_t{1} << 63U);
}

class SplitterGrip final : public Control {
public:
    explicit SplitterGrip(StableId stable_id)
        : Control(std::move(stable_id)) {
        set_focusable(true);
    }

    void set_orientation(Orientation orientation) {
        if (orientation_ == orientation) return;
        orientation_ = orientation;
        set_cursor(orientation == Orientation::vertical
                       ? CursorKind::resize_horizontal
                       : CursorKind::resize_vertical);
        invalidate(Dirty::paint | Dirty::semantics | Dirty::accessibility);
    }

    void set_visible_width(double width) {
        if (visible_width_ == width) return;
        visible_width_ = width;
        invalidate(Dirty::paint);
    }

    void on_focus_changed(bool focused) override {
        focused_ = focused;
        invalidate(invalidation::focus);
    }

    void on_paint(Painter& painter, Rect) override {
        const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                          committed_arranged_bounds().height};
        const BasicControlStyle style;
        if (orientation_ == Orientation::vertical) {
            const double width = std::min(visible_width_, bounds.width);
            const double x = std::floor((bounds.width - width) * 0.5);
            painter.fill_rect({x, 0.0, width, bounds.height}, style.face);
            painter.draw_line({x, 0.0}, {x, bounds.height}, style.highlight, 1.0);
            painter.draw_line({x + std::max(0.0, width - 1.0), 0.0},
                              {x + std::max(0.0, width - 1.0), bounds.height},
                              style.dark_border, 1.0);
        } else {
            const double height = std::min(visible_width_, bounds.height);
            const double y = std::floor((bounds.height - height) * 0.5);
            painter.fill_rect({0.0, y, bounds.width, height}, style.face);
            painter.draw_line({0.0, y}, {bounds.width, y}, style.highlight, 1.0);
            painter.draw_line({0.0, y + std::max(0.0, height - 1.0)},
                              {bounds.width, y + std::max(0.0, height - 1.0)},
                              style.dark_border, 1.0);
        }
        if (focused_) {
            painter.stroke_rect({1.0, 1.0, std::max(0.0, bounds.width - 2.0),
                                 std::max(0.0, bounds.height - 2.0)},
                                style.accent, 1.0);
        }
    }

private:
    Orientation orientation_{Orientation::vertical};
    double visible_width_{3.0};
    bool focused_{};
};

} // namespace

ContainerControl::ContainerControl(StableId stable_id)
    : Control(std::move(stable_id)) {}

bool ContainerControl::contains_descendant(const Control::Ptr& control) const noexcept {
    if (!control || control.get() == this) {
        return false;
    }
    for (Control::Ptr ancestor = control->parent(); ancestor;
         ancestor = ancestor->parent()) {
        if (ancestor.get() == this) {
            return true;
        }
    }
    return false;
}

Control::Ptr ContainerControl::active_control() const noexcept {
    const Window* owner = window();
    if (owner == nullptr) {
        return {};
    }
    Control::Ptr focused = owner->focused_control();
    return contains_descendant(focused) ? focused : Control::Ptr{};
}

bool ContainerControl::request_active_control(const Control::Ptr& control) {
    require_mutable();
    if (!contains_descendant(control) || window() == nullptr) {
        return false;
    }
    return window()->request_focus(control);
}

bool ContainerControl::clear_active_control() {
    require_mutable();
    if (window() == nullptr || !active_control()) {
        return false;
    }
    return window()->request_focus({});
}

UserControl::UserControl(StableId stable_id)
    : ContainerControl(std::move(stable_id)) {}

void UserControl::on_attached_to_window() {
    attached_ = true;
    if (!loaded_) {
        loaded_ = true;
        loaded_event_.emit();
    }
}

void UserControl::on_attachment_committed() noexcept {
    ++attachment_count_;
}

void UserControl::on_detached_from_window() noexcept {
    attached_ = false;
}

TabPage::TabPage(StableId stable_id, std::string text)
    : Panel(std::move(stable_id)), text_(std::move(text)) {
    set_background(Color::rgba(255, 255, 255));
    set_border_style(BorderStyle::line);
}

void TabPage::set_text(std::string text) {
    require_mutable();
    if (text_ == text) return;
    text_ = std::move(text);
    invalidate(Dirty::paint | Dirty::semantics | Dirty::measure);
    if (const Control::Ptr owner = parent()) {
        owner->invalidate(Dirty::paint | Dirty::semantics | Dirty::measure);
    }
}

SemanticDescriptor TabPage::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::group;
    descriptor.name = accessible_name().empty() ? text_ : accessible_name();
    descriptor.description = accessible_description();
    descriptor.exposed = true;
    return descriptor;
}

TabControl::TabControl(StableId stable_id)
    : ContainerControl(std::move(stable_id)) {
    set_focusable(true);
}

std::vector<std::shared_ptr<TabPage>> TabControl::pages() const {
    std::vector<std::shared_ptr<TabPage>> result;
    result.reserve(pages_.size());
    for (const std::weak_ptr<TabPage>& weak : pages_) {
        if (const auto page = weak.lock();
            page && page->is_alive() && page->parent().get() == this) {
            result.push_back(page);
        }
    }
    return result;
}

std::size_t TabControl::page_count() const {
    return pages().size();
}

std::shared_ptr<TabPage> TabControl::page_at(std::size_t index) const {
    const auto live = pages();
    if (index >= live.size()) throw std::out_of_range("TabControl page index");
    return live[index];
}

std::optional<std::size_t> TabControl::index_of(
    const std::shared_ptr<TabPage>& page) const {
    if (!page) return std::nullopt;
    const auto live = pages();
    const auto found = std::find(live.begin(), live.end(), page);
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
    page->set_visible(false);
    add_child(page);
    pages_.push_back(page);
    if (!selected_page_.lock()) {
        selected_page_ = page;
        page->set_visible(true);
        const TabSelectionChange change{std::nullopt, 0U};
        selected_index_changed_.emit(change);
    }
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
}

std::shared_ptr<TabPage> TabControl::remove_page(const TabPage& page) {
    require_mutable();
    const auto live_before = pages();
    const auto found = std::find_if(
        live_before.begin(), live_before.end(),
        [&page](const auto& candidate) { return candidate.get() == &page; });
    if (found == live_before.end()) return {};
    const std::size_t removed_index =
        static_cast<std::size_t>(found - live_before.begin());
    const std::optional<std::size_t> old_selected = selected_index();
    const bool removing_selected = selected_page_.lock().get() == &page;
    pages_.erase(std::remove_if(
        pages_.begin(), pages_.end(), [&page](const std::weak_ptr<TabPage>& weak) {
            const auto candidate = weak.lock();
            return !candidate || candidate.get() == &page;
        }), pages_.end());
    const Control::Ptr removed = remove_child(page.runtime_id());
    if (!removed) return {};

    const auto live_after = pages();
    if (removing_selected) {
        selected_page_.reset();
        if (!live_after.empty()) {
            const std::size_t next = std::min(removed_index, live_after.size() - 1U);
            selected_page_ = live_after[next];
            live_after[next]->set_visible(true);
        }
    }
    const std::optional<std::size_t> new_selected = selected_index();
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
    if (old_selected != new_selected || removing_selected) {
        const TabSelectionChange change{old_selected, new_selected};
        selected_index_changed_.emit(change);
    }
    return std::dynamic_pointer_cast<TabPage>(removed);
}

void TabControl::remember_page_focus(const std::shared_ptr<TabPage>& page) {
    if (!page || window() == nullptr) return;
    const Control::Ptr focused = window()->focused_control();
    if (!focused) return;
    for (Control::Ptr current = focused; current; current = current->parent()) {
        if (current == page) {
            remembered_focus_[page->runtime_id().value] = focused;
            return;
        }
    }
}

void TabControl::restore_page_focus(const std::shared_ptr<TabPage>& page,
                                    bool selection_owned_focus) {
    if (!selection_owned_focus || !page || window() == nullptr) return;
    const auto found = remembered_focus_.find(page->runtime_id().value);
    if (found != remembered_focus_.end()) {
        if (const Control::Ptr candidate = found->second.lock();
            candidate && candidate->eligible_for_input()) {
            for (Control::Ptr current = candidate; current; current = current->parent()) {
                if (current == page) {
                    if (window()->request_focus(candidate)) return;
                    break;
                }
            }
        }
    }
    static_cast<void>(window()->request_focus(shared_from_this()));
}

void TabControl::set_selected_index(std::size_t index) {
    require_mutable();
    const auto live = pages();
    if (index >= live.size()) throw std::out_of_range("TabControl selected index");
    const std::optional<std::size_t> old_index = selected_index();
    if (old_index && *old_index == index) return;
    const auto old_page = selected_page_.lock();
    bool selection_owned_focus{};
    if (old_page && window() != nullptr) {
        const Control::Ptr focused = window()->focused_control();
        for (Control::Ptr current = focused; current; current = current->parent()) {
            if (current == old_page) {
                selection_owned_focus = true;
                break;
            }
        }
        remember_page_focus(old_page);
        old_page->set_visible(false);
    }
    selected_page_ = live[index];
    live[index]->set_visible(true);
    invalidate(Dirty::arrange | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
    restore_page_focus(live[index], selection_owned_focus);
    const TabSelectionChange change{old_index, index};
    selected_index_changed_.emit(change);
}

void TabControl::set_selected_tab(const std::shared_ptr<TabPage>& page) {
    const auto index = index_of(page);
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
    if (alignment_ == TabAlignment::top || alignment_ == TabAlignment::bottom) {
        const double y = alignment_ == TabAlignment::top
            ? 0.0 : std::max(0.0, bounds.height - item_size_.height);
        return {static_cast<double>(index) * item_size_.width, y,
                item_size_.width, std::min(item_size_.height, bounds.height)};
    }
    const double x = alignment_ == TabAlignment::left
        ? 0.0 : std::max(0.0, bounds.width - item_size_.width);
    return {x, static_cast<double>(index) * item_size_.height,
            std::min(item_size_.width, bounds.width), item_size_.height};
}

Rect TabControl::display_bounds() const noexcept {
    const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                      committed_arranged_bounds().height};
    switch (alignment_) {
    case TabAlignment::top:
        return {0.0, std::min(bounds.height, item_size_.height - 1.0),
                bounds.width,
                std::max(0.0, bounds.height - item_size_.height + 1.0)};
    case TabAlignment::bottom:
        return {0.0, 0.0, bounds.width,
                std::max(0.0, bounds.height - item_size_.height + 1.0)};
    case TabAlignment::left:
        return {std::min(bounds.width, item_size_.width - 1.0), 0.0,
                std::max(0.0, bounds.width - item_size_.width + 1.0),
                bounds.height};
    case TabAlignment::right:
        return {0.0, 0.0,
                std::max(0.0, bounds.width - item_size_.width + 1.0),
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
            const auto page = weak.lock();
            return !page || !page->is_alive() || page->parent().get() != this;
        }), pages_.end());
    auto selected = selected_page_.lock();
    if (selected && selected->is_alive() && selected->parent().get() == this) return;
    selected_page_.reset();
    const auto live = pages();
    if (!live.empty()) {
        selected_page_ = live.front();
        live.front()->set_visible(true);
    }
}

void TabControl::arrange(Rect final_bounds) {
    ContainerControl::arrange(final_bounds);
    reconcile_pages();
    const Rect display = display_bounds();
    const auto selected = selected_page_.lock();
    for (const auto& page : pages()) {
        page->set_requested_bounds(display);
        page->set_visible(page == selected);
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
    const auto live = pages();
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
        const double text_width = static_cast<double>(live[index]->text().size()) *
                                  font_.size * 0.55;
        painter.draw_text_utf8(
            {tab.x + std::max(5.0, (tab.width - text_width) * 0.5),
             tab.y + (tab.height + font_.size) * 0.5 - 2.0},
            live[index]->text(), font_, enabled() ? style_.text
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
    const auto live = pages();
    for (std::size_t index = 0U; index < live.size(); ++index) {
        if (!tab_bounds(index).contains(local)) continue;
        if (window() != nullptr) {
            static_cast<void>(window()->request_focus(shared_from_this()));
        }
        pointer_engaged_ = true;
        set_selected_index(index);
        event.handled = true;
        return;
    }
}

void TabControl::select_relative(int delta) {
    const auto live = pages();
    if (live.empty()) return;
    const std::size_t current = selected_index().value_or(0U);
    const auto count = static_cast<std::ptrdiff_t>(live.size());
    const auto next = (static_cast<std::ptrdiff_t>(current) + delta + count) % count;
    set_selected_index(static_cast<std::size_t>(next));
}

void TabControl::on_key_preview(KeyEvent& event) {
    if (event.action != KeyAction::down || !enabled() || page_count() == 0U ||
        window() == nullptr) return;
    const auto modifiers = static_cast<std::uint8_t>(event.modifiers);
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
    if (window()->focused_control().get() != this) return;
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
    if (const auto selected = selected_page_.lock()) descriptor.value = selected->text();
    descriptor.actions = {SemanticAction::focus};
    descriptor.exposed = true;
    return descriptor;
}

std::vector<SemanticNode> TabControl::semantic_virtual_children() const {
    std::vector<SemanticNode> nodes;
    const auto live = pages();
    nodes.reserve(live.size());
    const Rect absolute = absolute_bounds();
    const auto selected = selected_page_.lock();
    for (std::size_t index = 0U; index < live.size(); ++index) {
        SemanticNode node;
        node.stable_id = std::string(live[index]->stable_id().value()) + ".tab";
        node.runtime_id = virtual_semantic_runtime_id(node.stable_id);
        node.role = SemanticRole::tab;
        node.name = live[index]->text();
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
    const auto live = pages();
    for (std::size_t index = 0U; index < live.size(); ++index) {
        if (child_stable_id !=
            std::string(live[index]->stable_id().value()) + ".tab") continue;
        if (window() != nullptr) {
            static_cast<void>(window()->request_focus(shared_from_this()));
        }
        set_selected_index(index);
        return true;
    }
    return false;
}

SplitterPanel::SplitterPanel(StableId stable_id)
    : ContainerControl(std::move(stable_id)) {}

SplitContainer::SplitContainer(StableId stable_id)
    : ContainerControl(std::move(stable_id)) {
    const std::string prefix(this->stable_id().value());
    first_panel_ = make_control<SplitterPanel>(StableId(prefix + ".panel1"));
    second_panel_ = make_control<SplitterPanel>(StableId(prefix + ".panel2"));
    splitter_ = make_control<SplitterGrip>(StableId(prefix + ".splitter"));
    update_splitter_cursor();
}

void SplitContainer::initialize_control_tree() {
    require_mutable();
    if (tree_initialized_) return;
    add_child(first_panel_);
    add_child(second_panel_);
    add_child(splitter_);
    tree_initialized_ = true;
}

void SplitContainer::set_orientation(Orientation orientation) {
    require_mutable();
    if (orientation_ == orientation) return;
    orientation_ = orientation;
    previous_axis_extent_ = 0.0;
    previous_second_extent_ = 0.0;
    update_splitter_cursor();
    invalidate(invalidation::bounds);
}

void SplitContainer::set_splitter_distance(double distance) {
    require_finite_nonnegative(distance,
                               "splitter distance must be finite and nonnegative");
    set_distance(distance, SplitChangeReason::programmatic);
}

void SplitContainer::set_splitter_width(double width) {
    require_mutable();
    if (!std::isfinite(width) || width <= 0.0) {
        throw std::invalid_argument("splitter width must be finite and positive");
    }
    if (splitter_width_ == width) return;
    splitter_width_ = width;
    if (splitter_hit_width_ < width) splitter_hit_width_ = width;
    static_cast<SplitterGrip&>(*splitter_).set_visible_width(width);
    invalidate(invalidation::bounds);
}

void SplitContainer::set_splitter_hit_width(double width) {
    require_mutable();
    if (!std::isfinite(width) || width <= 0.0) {
        throw std::invalid_argument(
            "splitter hit width must be finite and positive");
    }
    width = std::max(width, splitter_width_);
    if (splitter_hit_width_ == width) return;
    splitter_hit_width_ = width;
    invalidate(Dirty::arrange | Dirty::hit_test | Dirty::semantics |
               Dirty::accessibility);
}

void SplitContainer::set_first_minimum(double extent) {
    require_mutable();
    require_finite_nonnegative(extent,
                               "first panel minimum must be finite and nonnegative");
    if (first_minimum_ == extent) return;
    first_minimum_ = extent;
    invalidate(invalidation::bounds);
}

void SplitContainer::set_second_minimum(double extent) {
    require_mutable();
    require_finite_nonnegative(extent,
                               "second panel minimum must be finite and nonnegative");
    if (second_minimum_ == extent) return;
    second_minimum_ = extent;
    invalidate(invalidation::bounds);
}

void SplitContainer::set_first_collapsed(bool collapsed) {
    require_mutable();
    if (first_collapsed_ == collapsed) return;
    if (collapsed && second_collapsed_) {
        throw std::logic_error("both split panels may not be collapsed");
    }
    const double old = effective_distance_;
    if (collapsed) {
        if (effective_distance_ > 0.0) remembered_distance_ = effective_distance_;
        transfer_focus_from(first_panel_);
    } else if (remembered_distance_ >= 0.0) {
        requested_distance_ = remembered_distance_;
    }
    first_collapsed_ = collapsed;
    first_panel_->set_visible(!collapsed);
    const double total = axis_extent(committed_arranged_bounds());
    effective_distance_ = collapsed ? 0.0
                                    : constrained_distance(requested_distance_, total);
    invalidate(invalidation::bounds);
    const SplitChangeEvent change{old, effective_distance_,
                                  SplitChangeReason::collapse};
    splitter_changed_.emit(change);
}

void SplitContainer::set_second_collapsed(bool collapsed) {
    require_mutable();
    if (second_collapsed_ == collapsed) return;
    if (collapsed && first_collapsed_) {
        throw std::logic_error("both split panels may not be collapsed");
    }
    const double old = effective_distance_;
    if (collapsed) {
        if (effective_distance_ > 0.0) remembered_distance_ = effective_distance_;
        transfer_focus_from(second_panel_);
    } else if (remembered_distance_ >= 0.0) {
        requested_distance_ = remembered_distance_;
    }
    second_collapsed_ = collapsed;
    second_panel_->set_visible(!collapsed);
    const double total = axis_extent(committed_arranged_bounds());
    effective_distance_ = collapsed
        ? std::max(0.0, total - splitter_width_)
        : constrained_distance(requested_distance_, total);
    invalidate(invalidation::bounds);
    const SplitChangeEvent change{old, effective_distance_,
                                  SplitChangeReason::collapse};
    splitter_changed_.emit(change);
}

void SplitContainer::set_splitter_fixed(bool fixed) {
    require_mutable();
    if (splitter_fixed_ == fixed) return;
    splitter_fixed_ = fixed;
    invalidate(Dirty::semantics | Dirty::accessibility);
}

void SplitContainer::set_fixed_panel(SplitFixedPanel panel) {
    require_mutable();
    if (fixed_panel_ == panel) return;
    fixed_panel_ = panel;
    invalidate(Dirty::semantics | Dirty::accessibility);
}

void SplitContainer::set_keyboard_increment(double increment) {
    require_mutable();
    if (!std::isfinite(increment) || increment <= 0.0) {
        throw std::invalid_argument(
            "splitter keyboard increment must be finite and positive");
    }
    keyboard_increment_ = increment;
}

Size SplitContainer::measure(Size available) {
    const Rect requested = requested_bounds();
    return {std::min(available.width,
                     requested.width > 0.0 ? requested.width : available.width),
            std::min(available.height,
                     requested.height > 0.0 ? requested.height : available.height)};
}

void SplitContainer::arrange(Rect final_bounds) {
    ContainerControl::arrange(final_bounds);
    const double total = axis_extent(final_bounds);
    const double available = std::max(0.0, total - splitter_width_);
    double desired = requested_distance_ < 0.0 ? available * 0.5
                                               : requested_distance_;
    if (previous_axis_extent_ > 0.0 && total != previous_axis_extent_ &&
        fixed_panel_ == SplitFixedPanel::second && !second_collapsed_) {
        desired = std::max(0.0, available - previous_second_extent_);
    }
    const double old = effective_distance_;
    effective_distance_ = constrained_distance(desired, total);
    if (requested_distance_ >= 0.0 || fixed_panel_ == SplitFixedPanel::second) {
        requested_distance_ = effective_distance_;
    }
    const double second_extent = std::max(0.0, available - effective_distance_);
    const double hit_width = std::min(total, std::max(splitter_width_,
                                                      splitter_hit_width_));
    const double hit_origin = std::clamp(
        effective_distance_ + (splitter_width_ - hit_width) * 0.5,
        0.0, std::max(0.0, total - hit_width));
    if (orientation_ == Orientation::vertical) {
        first_panel_->set_requested_bounds(
            {0.0, 0.0, effective_distance_, final_bounds.height});
        second_panel_->set_requested_bounds(
            {effective_distance_ + splitter_width_, 0.0, second_extent,
             final_bounds.height});
        splitter_->set_requested_bounds(
            {hit_origin, 0.0, hit_width, final_bounds.height});
    } else {
        first_panel_->set_requested_bounds(
            {0.0, 0.0, final_bounds.width, effective_distance_});
        second_panel_->set_requested_bounds(
            {0.0, effective_distance_ + splitter_width_, final_bounds.width,
             second_extent});
        splitter_->set_requested_bounds(
            {0.0, hit_origin, final_bounds.width, hit_width});
    }
    previous_axis_extent_ = total;
    previous_second_extent_ = second_extent;
    if (old != effective_distance_) {
        const SplitChangeEvent change{old, effective_distance_,
                                      SplitChangeReason::container_resize};
        splitter_changed_.emit(change);
    }
}

void SplitContainer::on_pointer_preview(PointerEvent& event) {
    if (splitter_fixed_ || first_collapsed_ || second_collapsed_) return;
    if (event.action == PointerAction::down &&
        event.button == PointerButton::primary &&
        splitter_->absolute_bounds().contains(event.position)) {
        pointer_tracking_ = true;
        pointer_offset_ = pointer_axis(event.position) - effective_distance_;
        if (window() != nullptr) window()->request_focus(splitter_);
        splitter_->set_pointer_capture(true);
        event.handled = true;
    } else if (event.action == PointerAction::move && pointer_tracking_) {
        set_distance(pointer_axis(event.position) - pointer_offset_,
                     SplitChangeReason::pointer);
        event.handled = true;
    } else if (event.action == PointerAction::up && pointer_tracking_) {
        pointer_tracking_ = false;
        set_distance(pointer_axis(event.position) - pointer_offset_,
                     SplitChangeReason::pointer);
        event.handled = true;
    }
}

void SplitContainer::on_key_preview(KeyEvent& event) {
    if (event.action != KeyAction::down || splitter_fixed_ ||
        first_collapsed_ || second_collapsed_ || window() == nullptr ||
        window()->focused_control() != splitter_) return;
    double delta{};
    if (orientation_ == Orientation::vertical) {
        if (event.physical_key == PhysicalKey::left) delta = -keyboard_increment_;
        else if (event.physical_key == PhysicalKey::right) delta = keyboard_increment_;
        else return;
    } else {
        if (event.physical_key == PhysicalKey::up) delta = -keyboard_increment_;
        else if (event.physical_key == PhysicalKey::down) delta = keyboard_increment_;
        else return;
    }
    if ((static_cast<std::uint8_t>(event.modifiers) &
         static_cast<std::uint8_t>(Modifier::shift)) != 0U) {
        delta *= 10.0;
    }
    set_distance(effective_distance_ + delta, SplitChangeReason::keyboard);
    event.handled = true;
}

SemanticDescriptor SplitContainer::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::split_pane;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    descriptor.value = std::to_string(effective_distance_);
    descriptor.exposed = true;
    return descriptor;
}

double SplitContainer::axis_extent(Rect bounds) const noexcept {
    return orientation_ == Orientation::vertical ? bounds.width : bounds.height;
}

double SplitContainer::pointer_axis(Point point) const noexcept {
    const Rect absolute = absolute_bounds();
    return orientation_ == Orientation::vertical ? point.x - absolute.x
                                                  : point.y - absolute.y;
}

double SplitContainer::constrained_distance(double requested,
                                            double total_extent) const noexcept {
    const double available = std::max(0.0, total_extent - splitter_width_);
    if (first_collapsed_) return 0.0;
    if (second_collapsed_) return available;
    if (!std::isfinite(requested) || requested < 0.0) requested = available * 0.5;
    double lower = std::min(first_minimum_, available);
    double upper = std::max(0.0, available - second_minimum_);
    if (lower > upper) {
        const double requested_minimum = first_minimum_ + second_minimum_;
        const double compromise = requested_minimum > 0.0
            ? available * first_minimum_ / requested_minimum
            : available * 0.5;
        lower = upper = compromise;
    }
    return std::clamp(requested, lower, upper);
}

void SplitContainer::set_distance(double distance, SplitChangeReason reason) {
    require_mutable();
    require_finite_nonnegative(distance,
                               "splitter distance must be finite and nonnegative");
    const double old = effective_distance_;
    requested_distance_ = distance;
    const double total = axis_extent(committed_arranged_bounds());
    effective_distance_ = total > 0.0 ? constrained_distance(distance, total)
                                      : distance;
    if (old == effective_distance_) return;
    invalidate(invalidation::bounds);
    const SplitChangeEvent change{old, effective_distance_, reason};
    splitter_changed_.emit(change);
}

void SplitContainer::transfer_focus_from(
    const std::shared_ptr<SplitterPanel>& panel) {
    if (window() == nullptr) return;
    const Control::Ptr focused = window()->focused_control();
    if (focused == panel || panel->contains_descendant(focused)) {
        window()->request_focus(splitter_);
    }
}

void SplitContainer::update_splitter_cursor() {
    auto& splitter = static_cast<SplitterGrip&>(*splitter_);
    splitter.set_orientation(orientation_);
    splitter.set_visible_width(splitter_width_);
}

} // namespace gui_forms
