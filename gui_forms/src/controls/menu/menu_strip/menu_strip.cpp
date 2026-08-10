#include "gui_forms/controls/menu_strip/menu_strip.hpp"

#include "../menu_utilities.hpp"

#include "gui_forms/basic_controls.hpp"
#include "gui_forms/detail/bound_member_function.hpp"
#include "gui_forms/text.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <optional>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace gui_forms {

using menu_detail::menu_display_text;
using menu_detail::validate_specs;

namespace {

constexpr double menu_strip_height = 24.0;
constexpr double menu_strip_character_width = 7.35;

double menu_strip_item_width(std::string_view text, double padding) noexcept {
    return std::max(42.0, padding * 2.0 +
        static_cast<double>(text.size()) * menu_strip_character_width);
}

std::uint64_t menu_virtual_runtime_id(std::string_view id) noexcept {
    std::uint64_t hash = 1469598103934665603ULL;
    for (const unsigned char byte : id) {
        hash ^= byte;
        hash *= 1099511628211ULL;
    }
    return hash | (1ULL << 63U);
}

} // namespace

MenuStrip::MenuStrip(StableId stable_id)
    : Control(std::move(stable_id)),
      popup_(std::make_unique<ContextMenu>(
          std::string((*this).stable_id().value()) + ".menu")) {
    set_focusable(true);
    set_cursor(CursorKind::arrow);
    set_paint_plane(PaintPlane::control);
    (*popup_).set_root_navigation_handler(
        detail::BoundMemberFunction<bool (MenuStrip::*)(int)>(
            *this, &MenuStrip::navigate_root));
    (*popup_).set_outside_pointer_handler(
        detail::BoundMemberFunction<
            bool (MenuStrip::*)(const PointerEvent&)>(
                *this, &MenuStrip::handle_popup_pointer));
    popup_invoked_ = (*popup_).item_invoked().subscribe(
        *this, Delegate<const MenuItemInvocation&>::bind<
            MenuStrip, &MenuStrip::on_popup_item_invoked>(*this));
    popup_changed_ = (*popup_).open_changed().subscribe(
        *this, Delegate<bool>::bind<
            MenuStrip, &MenuStrip::on_popup_open_changed>(*this));
}

void MenuStrip::on_popup_item_invoked(
    const MenuItemInvocation& invocation) {
    if (!active_index_ || *active_index_ >= items_.size()) return;
    MenuStripInvocation forwarded{
        std::string(stable_id().value()),
        items_[*active_index_].stable_id, invocation};
    item_invoked_.emit(forwarded);
}

void MenuStrip::on_popup_open_changed(bool open_state) {
    if (open_state || switching_ || !active_index_) return;
    active_index_.reset();
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(open_changed_, active_index_);
}

MenuStrip::~MenuStrip() {
    close();
}

void MenuStrip::set_items(std::vector<MenuStripItemSpec> items) {
    require_mutable();
    std::unordered_set<std::string> identities;
    for (const gui_forms::MenuStripItemSpec& item : items) {
        if (item.stable_id.empty() || !validate_utf8(item.stable_id).valid() ||
            item.text.empty() || !validate_utf8(item.text).valid() ||
            item.items.empty() || !identities.insert(item.stable_id).second) {
            throw std::invalid_argument(
                "MenuStrip items require unique IDs, text, and menu contents");
        }
        std::unordered_set<std::string> child_identities;
        std::size_t count{};
        validate_specs(item.items, child_identities, 1U, count);
    }
    close();
    items_ = std::move(items);
    hot_index_.reset();
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

void MenuStrip::set_use_mnemonic(bool value) {
    require_mutable();
    if (use_mnemonic_ == value) return;
    use_mnemonic_ = value;
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

void MenuStrip::set_item_padding(double padding) {
    require_mutable();
    if (!std::isfinite(padding) || padding < 4.0 || padding > 64.0) {
        throw std::invalid_argument(
            "MenuStrip item padding must be finite and between 4 and 64");
    }
    if (item_padding_ == padding) return;
    close();
    item_padding_ = padding;
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
}

bool MenuStrip::open(std::size_t index) {
    require_mutable();
    if (index >= items_.size() || !items_[index].visible ||
        !items_[index].enabled || !attached_window()) return false;
    if (active_index_ == index && (*popup_).is_open()) return true;
    switching_ = (*popup_).is_open();
    if (switching_) (*popup_).close();
    active_index_ = index;
    hot_index_ = index;
    (*popup_).set_items(items_[index].items);
    const std::vector<Rect> bounds = item_bounds();
    const Rect absolute = absolute_bounds();
    (*popup_).show(shared_from_this(),
                 {absolute.x + bounds[index].x,
                  absolute.y + bounds[index].y + bounds[index].height - 1.0});
    switching_ = false;
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(open_changed_, active_index_);
    return true;
}

void MenuStrip::close() noexcept {
    if (!popup_) return;
    (*popup_).close();
}

bool MenuStrip::is_open() const noexcept {
    return popup_ && (*popup_).is_open();
}

Size MenuStrip::measure(Size available) {
    return {available.width, std::min(available.height,
                                     menu_strip_height * effective_text_scale())};
}

std::vector<Rect> MenuStrip::item_bounds() const {
    std::vector<Rect> result(items_.size());
    double x{};
    for (std::size_t index = 0; index < items_.size(); ++index) {
        if (!items_[index].visible) continue;
        const std::string display = use_mnemonic_
            ? menu_display_text(items_[index].text) : items_[index].text;
        const double width = menu_strip_item_width(display, item_padding_) *
            effective_text_scale();
        result[index] = {x, 0.0, width,
                         std::max(menu_strip_height * effective_text_scale(),
                                  committed_arranged_bounds().height)};
        x += width;
    }
    return result;
}

std::optional<std::size_t> MenuStrip::index_at(Point window_point) const {
    const Rect absolute = absolute_bounds();
    if (!absolute.contains(window_point)) return {};
    const Point local{window_point.x - absolute.x, window_point.y - absolute.y};
    const std::vector<Rect> bounds = item_bounds();
    for (std::size_t index = 0; index < bounds.size(); ++index) {
        if (items_[index].visible && bounds[index].contains(local)) return index;
    }
    return {};
}

std::optional<std::size_t> MenuStrip::next_enabled(
    std::size_t start, int direction) const {
    if (items_.empty()) return {};
    for (std::size_t step = 1U; step <= items_.size(); ++step) {
        const std::ptrdiff_t raw = static_cast<std::ptrdiff_t>(start) +
            static_cast<std::ptrdiff_t>(direction) *
                static_cast<std::ptrdiff_t>(step);
        const std::ptrdiff_t count = static_cast<std::ptrdiff_t>(items_.size());
        const std::size_t index = static_cast<std::size_t>((raw % count + count) % count);
        if (items_[index].visible && items_[index].enabled) return index;
    }
    return {};
}

bool MenuStrip::navigate_root(int direction) {
    if (!active_index_) return false;
    const std::optional<std::size_t> next = next_enabled(*active_index_, direction);
    return next && open(*next);
}

bool MenuStrip::handle_popup_pointer(const PointerEvent& event) {
    if (event.action != PointerAction::move &&
        event.action != PointerAction::down) return false;
    const std::optional<std::size_t> index = index_at(event.position);
    if (!index || !items_[*index].enabled) return false;
    set_hot(index);
    if (event.action == PointerAction::move && active_index_ != index) {
        return open(*index);
    }
    if (event.action == PointerAction::down) {
        if (active_index_ == index) close();
        else static_cast<void>(open(*index));
        return true;
    }
    return true;
}

void MenuStrip::set_hot(std::optional<std::size_t> index) {
    if (hot_index_ == index) return;
    hot_index_ = index;
    invalidate(Dirty::paint | Dirty::semantics);
}

void MenuStrip::on_paint(Painter& painter, Rect) {
    const Rect surface{0.0, 0.0, committed_arranged_bounds().width,
                      committed_arranged_bounds().height};
    const BasicControlStyle& style = effective_theme().basic_style();
    const ControlVisualRecipe& strip_recipe = effective_theme().resolve(
        ControlVisualRole::panel, visual_context());
    paint_surface_material(painter, surface, strip_recipe.material);
    painter.draw_line({0.0, std::max(0.0, surface.height - 1.0)},
                      {surface.width, std::max(0.0, surface.height - 1.0)},
                      style.border, 1.0);
    const std::vector<Rect> bounds = item_bounds();
    for (std::size_t index = 0; index < items_.size(); ++index) {
        if (!items_[index].visible) continue;
        const Rect item = bounds[index];
        const bool active = active_index_ == index;
        const bool hot = hot_index_ == index;
        ControlVisualContext item_context =
            visual_context(hot, active, active, focused_);
        if (!items_[index].enabled) {
            item_context.surface = ControlSurfaceState::disabled;
        }
        const ControlVisualRecipe& item_recipe = effective_theme().resolve(
            ControlVisualRole::menu_item, item_context);
        if (active || hot) {
            paint_surface_material(
                painter,
                {item.x + 1.0, 1.0, item.width - 2.0,
                 std::max(0.0, item.height - 2.0)},
                item_recipe.material);
        }
        const Color ink = item_recipe.text;
        const FontSpec font = effective_font({FontRole::control, 10.5,
                            static_cast<std::uint16_t>(active ? 700U : 400U),
                            false, 0.12});
        painter.draw_text_utf8({item.x + item_padding_ *
                                             effective_text_scale(),
                                item.y + std::max(font.size,
                                    item.height * 0.5 + font.size * 0.35)},
                               use_mnemonic_
                                   ? menu_display_text(items_[index].text)
                                   : items_[index].text,
                               font, ink);
    }
    if (focused_ && !active_index_ && hot_index_) {
        const Rect item = bounds[*hot_index_];
        painter.stroke_rect({item.x + 2.5, 2.5, item.width - 5.0,
                             std::max(0.0, item.height - 5.0)},
                            effective_theme().resolve(
                                ControlVisualRole::menu_item,
                                visual_context(true, false, false, true))
                                .focus_ring,
                            1.0);
    }
}

void MenuStrip::on_pointer(PointerEvent& event) {
    if (event.action == PointerAction::move) {
        const std::optional<std::size_t> index = index_at(event.position);
        set_hot(index);
        if (is_open() && index && active_index_ != index &&
            items_[*index].enabled) {
            static_cast<void>(open(*index));
        }
        event.handled = hot_index_.has_value();
    } else if (event.action == PointerAction::leave && !is_open()) {
        set_hot({});
    } else if (event.action == PointerAction::down &&
               event.button == PointerButton::primary) {
        const std::optional<std::size_t> index = index_at(event.position);
        if (index && items_[*index].enabled) {
            if (active_index_ == index && is_open()) close();
            else static_cast<void>(open(*index));
            event.handled = true;
        }
    }
}

void MenuStrip::on_key(KeyEvent& event) {
    if (!focused_ || event.action != KeyAction::down) return;
    std::optional<std::size_t> current = active_index_ ? active_index_ : hot_index_;
    if (!current) {
        for (std::size_t index = 0; index < items_.size(); ++index) {
            if (items_[index].visible && items_[index].enabled) {
                current = index;
                break;
            }
        }
    }
    if (!current) return;
    if (event.physical_key == PhysicalKey::left ||
        event.physical_key == PhysicalKey::right) {
        const std::optional<std::size_t> next = next_enabled(*current,
            event.physical_key == PhysicalKey::right ? 1 : -1);
        if (next) {
            if (is_open()) static_cast<void>(open(*next));
            else set_hot(next);
        }
    } else if (event.physical_key == PhysicalKey::down ||
               event.physical_key == PhysicalKey::enter ||
               event.physical_key == PhysicalKey::space) {
        static_cast<void>(open(*current));
    } else if (event.physical_key == PhysicalKey::escape) {
        close();
    } else {
        return;
    }
    event.handled = true;
}

void MenuStrip::on_focus_changed(bool focused) {
    focused_ = focused;
    if (focused_ && !hot_index_) {
        for (std::size_t index = 0; index < items_.size(); ++index) {
            if (items_[index].visible && items_[index].enabled) {
                hot_index_ = index;
                break;
            }
        }
    }
    invalidate(Dirty::paint | Dirty::semantics);
}

SemanticDescriptor MenuStrip::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::menu_bar;
    descriptor.name = accessible_name().empty() ? "Application menu" : accessible_name();
    descriptor.description = accessible_description();
    descriptor.actions = {SemanticAction::focus};
    descriptor.exposed = true;
    return descriptor;
}

std::vector<SemanticNode> MenuStrip::semantic_virtual_children() const {
    std::vector<SemanticNode> nodes;
    const Rect absolute = absolute_bounds();
    const std::vector<Rect> bounds = item_bounds();
    for (std::size_t index = 0; index < items_.size(); ++index) {
        if (!items_[index].visible) continue;
        SemanticNode node;
        node.stable_id = items_[index].stable_id;
        node.runtime_id = menu_virtual_runtime_id(node.stable_id);
        node.role = SemanticRole::menu_bar_item;
        node.name = use_mnemonic_
            ? menu_display_text(items_[index].text) : items_[index].text;
        node.bounds = {absolute.x + bounds[index].x, absolute.y + bounds[index].y,
                       bounds[index].width, bounds[index].height};
        node.states = SemanticState::visible | SemanticState::focusable;
        if (effectively_enabled() && items_[index].enabled) {
            node.states |= SemanticState::enabled;
        }
        if (active_index_ == index) {
            node.states |= SemanticState::selected;
            node.states |= SemanticState::expanded;
        }
        if (focused_ && hot_index_ == index) node.states |= SemanticState::focused;
        node.actions = {SemanticAction::focus, SemanticAction::press,
                        SemanticAction::expand, SemanticAction::collapse};
        nodes.push_back(std::move(node));
    }
    return nodes;
}

bool MenuStrip::on_semantic_child_action(std::string_view id,
                                         SemanticAction action,
                                         std::string_view) {
    std::vector<MenuStripItemSpec>::iterator found = items_.begin();
    while (found != items_.end() && (*found).stable_id != id) ++found;
    if (found == items_.end() || !(*found).visible || !(*found).enabled) return false;
    const std::size_t index = static_cast<std::size_t>(
        std::distance(items_.begin(), found));
    if (action == SemanticAction::collapse) {
        if (active_index_ != index) return false;
        close();
        return true;
    }
    if (action != SemanticAction::focus && action != SemanticAction::press &&
        action != SemanticAction::expand) return false;
    if (window()) static_cast<void>((*window()).request_focus(shared_from_this()));
    set_hot(index);
    if (action != SemanticAction::focus) return open(index);
    return true;
}

bool MenuStrip::mnemonic_matches(char32_t character) const noexcept {
    if (!use_mnemonic_) return false;
    for (const MenuStripItemSpec& item : items_) {
        if (item.visible && item.enabled &&
            is_mnemonic(character, item.text)) return true;
    }
    return false;
}

bool MenuStrip::process_mnemonic_self(char32_t character) {
    if (!mnemonic_matches(character)) return false;
    std::vector<std::size_t> matches;
    for (std::size_t index = 0U; index < items_.size(); ++index) {
        if (items_[index].visible && items_[index].enabled &&
            is_mnemonic(character, items_[index].text)) {
            matches.push_back(index);
        }
    }
    if (matches.empty()) return false;
    std::size_t selected = matches.front();
    const std::optional<std::size_t> current = active_index_
        ? active_index_ : hot_index_;
    if (current) {
        const std::vector<std::size_t>::iterator found =
            std::find(matches.begin(), matches.end(), *current);
        if (found != matches.end()) {
            const std::vector<std::size_t>::iterator next = std::next(found);
            selected = next == matches.end() ? matches.front() : *next;
        }
    }
    if (window() && !(*window()).request_focus(shared_from_this())) {
        // The mnemonic is still owned, but prevent-mode validation may reject
        // the focus transaction that would authorize opening the popup.
        return true;
    }
    set_hot(selected);
    return open(selected);
}

void MenuStrip::on_dispose() noexcept {
    close();
    if (popup_) (*popup_).dispose();
}


} // namespace gui_forms
