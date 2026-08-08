#include "gui_forms/menu_controls.hpp"

#include "gui_forms/basic_controls.hpp"
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
namespace {

constexpr double menu_width = 268.0;
constexpr double command_row_height = 28.0;
constexpr double separator_row_height = 9.0;
constexpr double menu_border = 2.0;

[[nodiscard]] std::string menu_display_text(std::string_view text) {
    return parse_mnemonic_text(text).display_text;
}

struct MenuSnapshot final {
    std::string stable_id;
    MenuItemKind kind{MenuItemKind::command};
    std::shared_ptr<Command> command;
    CommandState command_state;
    std::string text;
    std::vector<MenuSnapshot> children;
};

double row_height(const MenuSnapshot& item, double text_scale) noexcept {
    return (item.kind == MenuItemKind::separator ? separator_row_height
                                                 : command_row_height) * text_scale;
}

void validate_specs(const std::vector<MenuItemSpec>& items,
                    std::unordered_set<std::string>& identities,
                    std::size_t depth, std::size_t& count) {
    if (depth > maximum_menu_depth) {
        throw std::invalid_argument("ContextMenu nesting exceeds the bounded depth");
    }
    for (const MenuItemSpec& item : items) {
        ++count;
        if (count > maximum_menu_items) {
            throw std::invalid_argument("ContextMenu exceeds the bounded item count");
        }
        if (item.stable_id.empty() || !validate_utf8(item.stable_id).valid() ||
            !validate_utf8(item.text).valid() ||
            !identities.insert(item.stable_id).second) {
            throw std::invalid_argument(
                "ContextMenu items require unique stable IDs and valid UTF-8");
        }
        if (item.kind == MenuItemKind::separator) {
            if (item.command || !item.children.empty()) {
                throw std::invalid_argument(
                    "ContextMenu separator may not own a command or children");
            }
        } else if (item.kind == MenuItemKind::submenu) {
            if (item.command || item.text.empty() || item.children.empty()) {
                throw std::invalid_argument(
                    "ContextMenu submenu requires text and children but no command");
            }
        } else if (!item.command || !item.children.empty()) {
            throw std::invalid_argument(
                "ContextMenu command/check/radio item requires one shared command");
        }
        validate_specs(item.children, identities, depth + 1U, count);
    }
}

std::vector<MenuSnapshot> snapshot_items(const std::vector<MenuItemSpec>& specs) {
    std::vector<MenuSnapshot> result;
    result.reserve(specs.size());
    for (const MenuItemSpec& spec : specs) {
        MenuSnapshot snapshot;
        snapshot.stable_id = spec.stable_id;
        snapshot.kind = spec.kind;
        snapshot.command = spec.command;
        snapshot.text = spec.text;
        if (spec.command) {
            snapshot.command_state = spec.command->state();
            if (!snapshot.command_state.visible) continue;
            if (snapshot.text.empty()) snapshot.text = snapshot.command_state.text;
        }
        snapshot.children = snapshot_items(spec.children);
        if (snapshot.kind == MenuItemKind::submenu && snapshot.children.empty()) {
            continue;
        }
        result.push_back(std::move(snapshot));
    }
    // Leading, trailing, and repeated separators carry no meaning after hidden
    // command filtering. Normalize them in the immutable opening snapshot.
    while (!result.empty() && result.front().kind == MenuItemKind::separator) {
        result.erase(result.begin());
    }
    while (!result.empty() && result.back().kind == MenuItemKind::separator) {
        result.pop_back();
    }
    result.erase(std::unique(result.begin(), result.end(),
        [](const MenuSnapshot& left, const MenuSnapshot& right) {
            return left.kind == MenuItemKind::separator &&
                   right.kind == MenuItemKind::separator;
        }), result.end());
    return result;
}

Color menu_ink(const MenuSnapshot& item, const BasicControlStyle& style) noexcept {
    if (item.kind != MenuItemKind::submenu && !item.command_state.enabled) {
        return style.disabled_text;
    }
    if (item.command_state.destructive) return Color::rgba(145, 44, 42);
    return style.text;
}

} // namespace

struct ContextMenu::Impl final {
    class MenuLayer;
    class MenuPanel;
    class MenuRow;

    struct PanelState final {
        std::shared_ptr<MenuPanel> panel;
        std::vector<std::shared_ptr<MenuRow>> rows;
        const std::vector<MenuSnapshot>* items{};
        std::optional<std::size_t> parent_index;
        std::optional<std::size_t> child_source_index;
        double scroll_offset{};
        double content_height{};
        double viewport_height{};
        double width{};
    };

    explicit Impl(ContextMenu& public_owner) : owner(public_owner) {}

    [[nodiscard]] double text_scale() const noexcept {
        return window ? window->presentation_settings().text_scale : 1.0;
    }

    class MenuLayer final : public Panel {
    public:
        MenuLayer(StableId id, Impl& impl) : Panel(std::move(id)), impl_(impl) {
            set_paint_plane(PaintPlane::overlay);
            set_border_style(BorderStyle::none);
            set_background(Color::rgba(0, 0, 0, 0));
        }

        void on_pointer(PointerEvent& event) override {
            if (impl_.outside_pointer_handler &&
                impl_.outside_pointer_handler(event)) {
                event.handled = true;
                return;
            }
            if (event.phase == EventPhase::target &&
                event.action == PointerAction::down &&
                event.button != PointerButton::none) {
                impl_.close();
                event.handled = true;
            }
        }

        [[nodiscard]] SemanticDescriptor semantic_descriptor() const override {
            SemanticDescriptor descriptor;
            descriptor.exposed = false;
            descriptor.include_descendants = true;
            return descriptor;
        }

    private:
        Impl& impl_;
    };

    class MenuPanel final : public Panel {
    public:
        MenuPanel(StableId id, Impl& impl, std::size_t depth)
            : Panel(std::move(id)), impl_(impl), depth_(depth) {
            set_paint_plane(PaintPlane::overlay);
            set_background(Color::rgba(250, 253, 255));
            set_border_style(BorderStyle::line);
        }

        void on_pointer(PointerEvent& event) override {
            if (event.action == PointerAction::wheel && event.wheel_delta.y != 0.0) {
                impl_.scroll(depth_, event.wheel_delta.y > 0.0 ? -3 : 3);
                event.handled = true;
            } else if (event.action == PointerAction::down &&
                       event.button != PointerButton::none) {
                event.handled = true;
            }
        }

        void on_pointer_bubble(PointerEvent& event) override {
            if (!event.handled && event.action == PointerAction::wheel &&
                event.wheel_delta.y != 0.0) {
                impl_.scroll(depth_, event.wheel_delta.y > 0.0 ? -3 : 3);
                event.handled = true;
            }
        }

        [[nodiscard]] SemanticDescriptor semantic_descriptor() const override {
            SemanticDescriptor descriptor;
            descriptor.role = SemanticRole::menu;
            descriptor.name = "Menu";
            descriptor.exposed = true;
            descriptor.include_descendants = true;
            return descriptor;
        }

    private:
        Impl& impl_;
        std::size_t depth_{};
    };

    class MenuRow final : public Control {
    public:
        MenuRow(StableId id, Impl& impl, std::size_t depth, std::size_t index,
                const MenuSnapshot& item)
            : Control(std::move(id)), impl_(impl), depth_(depth), index_(index),
              item_(item) {
            set_paint_plane(PaintPlane::overlay);
            set_focusable(item.kind != MenuItemKind::separator && enabled_item());
            set_cursor(item.kind == MenuItemKind::separator
                ? std::optional<CursorKind>{} : CursorKind::hand);
        }

        [[nodiscard]] bool enabled_item() const noexcept {
            return item_.kind == MenuItemKind::submenu || item_.command_state.enabled;
        }

        void set_hot(bool hot) {
            if (hot_ == hot) return;
            hot_ = hot;
            invalidate(Dirty::paint);
        }

        void on_paint(Painter& painter, Rect) override {
            const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                              committed_arranged_bounds().height};
            const BasicControlStyle style;
            if (item_.kind == MenuItemKind::separator) {
                painter.draw_line({30.0, bounds.height * .5},
                                  {std::max(31.0, bounds.width - 7.0),
                                   bounds.height * .5},
                                  style.border, 1.0);
                return;
            }
            if (hot_ || focused_) {
                painter.fill_rect({2.0, 1.0, std::max(0.0, bounds.width - 4.0),
                                   std::max(0.0, bounds.height - 2.0)},
                                  enabled_item() ? style.accent_light
                                                 : style.face_light);
                painter.stroke_rect({2.5, 1.5, std::max(0.0, bounds.width - 5.0),
                                     std::max(0.0, bounds.height - 3.0)},
                                    enabled_item() ? style.border
                                                   : style.face, 1.0);
            }
            const Color ink = menu_ink(item_, style);
            if ((item_.kind == MenuItemKind::check ||
                 item_.kind == MenuItemKind::radio) && item_.command_state.checked) {
                if (item_.kind == MenuItemKind::check) {
                    painter.draw_line({8.0, 14.0}, {12.0, 18.0}, ink, 1.8);
                    painter.draw_line({12.0, 18.0}, {20.0, 8.0}, ink, 1.8);
                } else {
                    painter.stroke_rect({9.5, 8.5, 11.0, 11.0}, ink, 1.0);
                    painter.fill_rect({13.0, 12.0, 5.0, 5.0}, ink);
                }
            }
            const FontSpec font = effective_font({FontRole::control, 11.0,
                                static_cast<std::uint16_t>(
                                    item_.command_state.default_action ? 700 : 400),
                                false, 0.12});
            const double baseline = std::max(font.size,
                                              bounds.height * 0.5 + font.size * 0.35);
            painter.draw_text_utf8({30.0, baseline},
                                   menu_display_text(item_.text), font, ink);
            if (!item_.command_state.shortcut.empty()) {
                const FontSpec shortcut_font = effective_font(
                    {FontRole::content, 10.0, 400, false});
                const Size size = painter.measure_text_utf8(
                    item_.command_state.shortcut, shortcut_font);
                painter.draw_text_utf8(
                    {std::max(34.0, bounds.width - size.width - 25.0), baseline},
                    item_.command_state.shortcut, shortcut_font,
                    enabled_item() ? style.disabled_text : style.face);
            }
            if (item_.kind == MenuItemKind::submenu) {
                const double x = bounds.width - 13.0;
                painter.draw_line({x - 3.0, 9.0}, {x + 1.0, 14.0}, ink, 1.2);
                painter.draw_line({x + 1.0, 14.0}, {x - 3.0, 19.0}, ink, 1.2);
            }
        }

        void on_pointer(PointerEvent& event) override {
            if (item_.kind == MenuItemKind::separator) return;
            if (event.action == PointerAction::move) {
                impl_.hover(depth_, index_);
                event.handled = true;
            } else if (event.action == PointerAction::down &&
                       event.button == PointerButton::primary) {
                pressed_ = true;
                impl_.hover(depth_, index_);
                event.handled = true;
            } else if (event.action == PointerAction::up &&
                       event.button == PointerButton::primary) {
                const bool invoke = pressed_ && absolute_bounds().contains(event.position);
                pressed_ = false;
                if (invoke) impl_.activate(depth_, index_);
                event.handled = true;
            } else if (event.action == PointerAction::leave) {
                pressed_ = false;
            }
        }

        void on_key(KeyEvent& event) override {
            if (!focused_ || event.action != KeyAction::down) return;
            if (event.physical_key == PhysicalKey::down) {
                impl_.move_focus(depth_, index_, 1);
            } else if (event.physical_key == PhysicalKey::up) {
                impl_.move_focus(depth_, index_, -1);
            } else if (event.physical_key == PhysicalKey::home) {
                impl_.focus_edge(depth_, false);
            } else if (event.physical_key == PhysicalKey::end) {
                impl_.focus_edge(depth_, true);
            } else if (event.physical_key == PhysicalKey::right) {
                if (item_.kind == MenuItemKind::submenu) {
                    impl_.open_submenu(depth_, index_, true);
                } else if (depth_ == 0U) {
                    static_cast<void>(impl_.navigate_root(1));
                }
            } else if (event.physical_key == PhysicalKey::left) {
                impl_.close_submenu(depth_);
            } else if (event.physical_key == PhysicalKey::enter ||
                       event.physical_key == PhysicalKey::space) {
                impl_.activate(depth_, index_);
            } else if (event.physical_key == PhysicalKey::escape) {
                impl_.close();
            } else {
                return;
            }
            event.handled = true;
        }

        void on_focus_changed(bool focused) override {
            focused_ = focused;
            set_hot(focused);
            if (focused) impl_.ensure_visible(depth_, index_);
            invalidate(Dirty::paint | Dirty::semantics);
        }

        [[nodiscard]] SemanticDescriptor semantic_descriptor() const override {
            SemanticDescriptor descriptor;
            descriptor.role = item_.kind == MenuItemKind::separator
                ? SemanticRole::separator : SemanticRole::menu_item;
            descriptor.name = menu_display_text(item_.text);
            descriptor.description = item_.command_state.description;
            if (!item_.command_state.shortcut.empty()) {
                if (!descriptor.description.empty()) descriptor.description += " · ";
                descriptor.description += item_.command_state.shortcut;
            }
            if (!enabled_item() && !item_.command_state.availability_reason.empty()) {
                if (!descriptor.description.empty()) descriptor.description += " · ";
                descriptor.description += item_.command_state.availability_reason;
            }
            if (item_.command_state.checked) descriptor.states |= SemanticState::checked;
            if (item_.kind == MenuItemKind::submenu &&
                impl_.submenu_open(depth_, index_)) {
                descriptor.states |= SemanticState::expanded;
            }
            descriptor.exposed = true;
            descriptor.actions = item_.kind == MenuItemKind::submenu
                ? std::vector<SemanticAction>{SemanticAction::focus,
                                              SemanticAction::expand,
                                              SemanticAction::collapse}
                : item_.kind == MenuItemKind::separator
                    ? std::vector<SemanticAction>{}
                    : std::vector<SemanticAction>{SemanticAction::focus,
                                                  SemanticAction::press};
            return descriptor;
        }

        bool on_semantic_action(SemanticAction action,
                                std::string_view) override {
            if (action == SemanticAction::focus) {
                if (window()) return window()->request_focus(shared_from_this());
                return false;
            }
            if (action == SemanticAction::press &&
                item_.kind != MenuItemKind::submenu && enabled_item()) {
                impl_.activate(depth_, index_);
                return true;
            }
            if (action == SemanticAction::expand &&
                item_.kind == MenuItemKind::submenu) {
                impl_.open_submenu(depth_, index_, true);
                return true;
            }
            if (action == SemanticAction::collapse &&
                item_.kind == MenuItemKind::submenu) {
                impl_.remove_panels_after(depth_);
                return true;
            }
            return false;
        }

    protected:
        [[nodiscard]] bool mnemonic_matches(
            char32_t character) const noexcept override {
            return item_.kind != MenuItemKind::separator && enabled_item() &&
                   is_mnemonic(character, item_.text);
        }

        bool process_mnemonic_self(char32_t character) override {
            if (!mnemonic_matches(character)) return false;
            impl_.activate(depth_, index_);
            return true;
        }

    private:
        Impl& impl_;
        std::size_t depth_{};
        std::size_t index_{};
        const MenuSnapshot& item_;
        bool hot_{};
        bool focused_{};
        bool pressed_{};
    };

    void show(const Control::Ptr& invoker, Point position,
              const std::vector<MenuItemSpec>& specs) {
        close();
        if (!invoker || !invoker->attached_window()) {
            throw std::logic_error("ContextMenu requires an attached owner control");
        }
        snapshot = snapshot_items(specs);
        if (snapshot.empty()) {
            throw std::logic_error("ContextMenu has no visible items to show");
        }
        window = invoker->attached_window();
        owner_control = invoker;
        const Size client = window->client_size();
        layer = make_control<MenuLayer>(
            StableId(owner.stable_id_ + ".popup.layer"), *this);
        layer->set_requested_bounds({0.0, 0.0, client.width, client.height});
        open_panel(0U, snapshot, position.x, position.y, {});
        popup_token = window->open_popup(invoker, layer);
        if (Event<>* closed = popup_token.closed_event()) {
            popup_revocation = closed->subscribe(owner, [this] {
                on_popup_revoked();
            });
        }
        Control::Ptr preferred = first_focusable(0U);
        focus_scope = window->begin_focus_scope(layer, preferred);
        open = true;
        owner.open_changed_.emit(true);
    }

    void close() noexcept {
        if (closing) return;
        closing = true;
        if (window && focus_scope) {
            try {
                static_cast<void>(window->end_focus_scope(focus_scope));
            } catch (...) {
            }
        }
        focus_scope = {};
        popup_token.disconnect();
        popup_revocation.disconnect();
        const bool changed = open;
        reset_visual_state();
        closing = false;
        if (changed && owner.is_alive()) owner.open_changed_.emit(false);
    }

    void on_popup_revoked() noexcept {
        if (closing) return;
        if (window && focus_scope) {
            try {
                static_cast<void>(window->end_focus_scope(
                    focus_scope, FocusScopeCloseReason::owner_unavailable));
            } catch (...) {
            }
        }
        focus_scope = {};
        popup_revocation.disconnect();
        const bool changed = open;
        reset_visual_state();
        if (changed && owner.is_alive()) owner.open_changed_.emit(false);
    }

    void reset_visual_state() noexcept {
        panels.clear();
        layer.reset();
        owner_control.reset();
        window = nullptr;
        snapshot.clear();
        open = false;
    }

    void open_panel(std::size_t depth, const std::vector<MenuSnapshot>& items,
                    double requested_x, double requested_y,
                    std::optional<std::size_t> parent_index) {
        if (!layer || !window || depth >= maximum_menu_depth) return;
        remove_panels_from(depth);
        const Size client = window->client_size();
        const double panel_width = std::max(0.0,
            std::min(menu_width * text_scale(), client.width - 8.0));
        double content = menu_border * 2.0;
        for (const MenuSnapshot& item : items) content += row_height(item, text_scale());
        const double maximum_height = std::max(command_row_height + menu_border * 2.0,
                                                client.height - 8.0);
        const double viewport = std::min(content, maximum_height);
        const double x = std::clamp(requested_x, 4.0,
                                    std::max(4.0, client.width - panel_width - 4.0));
        const double y = std::clamp(requested_y, 4.0,
                                    std::max(4.0, client.height - viewport - 4.0));
        PanelState state;
        state.items = &items;
        state.parent_index = parent_index;
        state.content_height = content;
        state.viewport_height = viewport;
        state.width = panel_width;
        state.panel = make_control<MenuPanel>(
            StableId(owner.stable_id_ + ".popup.panel." + std::to_string(depth)),
            *this, depth);
        state.panel->set_requested_bounds({x, y, panel_width, viewport});
        state.rows.reserve(items.size());
        for (std::size_t index = 0; index < items.size(); ++index) {
            auto row = make_control<MenuRow>(
                StableId(owner.stable_id_ + ".popup.row." + items[index].stable_id),
                *this, depth, index, items[index]);
            state.panel->add_child(row);
            state.rows.push_back(std::move(row));
        }
        layer->add_child(state.panel);
        panels.push_back(std::move(state));
        layout_rows(depth);
    }

    void layout_rows(std::size_t depth) {
        if (depth >= panels.size()) return;
        PanelState& state = panels[depth];
        double y = menu_border - state.scroll_offset;
        for (std::size_t index = 0; index < state.rows.size(); ++index) {
            const double height = row_height((*state.items)[index], text_scale());
            state.rows[index]->set_requested_bounds(
                {menu_border, y,
                 std::max(0.0, state.width - menu_border * 2.0), height});
            state.rows[index]->set_visible(
                y + height > menu_border && y < state.viewport_height - menu_border);
            y += height;
        }
    }

    void remove_panels_from(std::size_t depth) {
        while (panels.size() > depth) {
            const auto panel = panels.back().panel;
            panels.pop_back();
            if (layer && panel && panel->parent().get() == layer.get()) {
                static_cast<void>(layer->remove_child(panel->runtime_id()));
            }
        }
        if (depth > 0U && depth - 1U < panels.size()) {
            panels[depth - 1U].child_source_index.reset();
        }
    }

    void remove_panels_after(std::size_t depth) {
        remove_panels_from(depth + 1U);
    }

    [[nodiscard]] Control::Ptr first_focusable(std::size_t depth) const {
        if (depth >= panels.size()) return {};
        for (const auto& row : panels[depth].rows) {
            if (row->focusable() && row->effectively_visible()) return row;
        }
        for (const auto& row : panels[depth].rows) {
            if (row->focusable()) return row;
        }
        return {};
    }

    void ensure_visible(std::size_t depth, std::size_t index) {
        if (depth >= panels.size() || index >= panels[depth].rows.size()) return;
        PanelState& state = panels[depth];
        double top = menu_border;
        for (std::size_t current = 0; current < index; ++current) {
            top += row_height((*state.items)[current], text_scale());
        }
        const double bottom = top + row_height((*state.items)[index], text_scale());
        const double visible_top = state.scroll_offset + menu_border;
        const double visible_bottom = state.scroll_offset + state.viewport_height - menu_border;
        if (top < visible_top) state.scroll_offset = std::max(0.0, top - menu_border);
        else if (bottom > visible_bottom) {
            state.scroll_offset = std::min(
                std::max(0.0, state.content_height - state.viewport_height),
                bottom - state.viewport_height + menu_border);
        }
        layout_rows(depth);
    }

    void scroll(std::size_t depth, int rows) {
        if (depth >= panels.size()) return;
        PanelState& state = panels[depth];
        const double maximum = std::max(0.0,
            state.content_height - state.viewport_height);
        const double next = std::clamp(
            state.scroll_offset + static_cast<double>(rows) * command_row_height *
                text_scale(),
            0.0, maximum);
        if (next == state.scroll_offset) return;
        state.scroll_offset = next;
        layout_rows(depth);
    }

    void hover(std::size_t depth, std::size_t index) {
        if (depth >= panels.size() || index >= panels[depth].rows.size()) return;
        for (std::size_t current = 0; current < panels[depth].rows.size(); ++current) {
            panels[depth].rows[current]->set_hot(current == index);
        }
        if (window && panels[depth].rows[index]->focusable()) {
            static_cast<void>(window->request_focus(panels[depth].rows[index]));
        }
        const MenuSnapshot& item = (*panels[depth].items)[index];
        if (item.kind == MenuItemKind::submenu) {
            open_submenu(depth, index, false);
        } else {
            remove_panels_after(depth);
        }
    }

    void open_submenu(std::size_t depth, std::size_t index, bool focus_first) {
        if (depth >= panels.size() || index >= panels[depth].rows.size()) return;
        const MenuSnapshot& item = (*panels[depth].items)[index];
        if (item.kind != MenuItemKind::submenu || item.children.empty()) return;
        if (panels[depth].child_source_index != index ||
            panels.size() <= depth + 1U) {
            const Rect parent = panels[depth].panel->absolute_bounds();
            const Rect row = panels[depth].rows[index]->absolute_bounds();
            const Size client = window->client_size();
            const double child_width = std::max(0.0,
                std::min(menu_width * text_scale(), client.width - 8.0));
            double x = parent.x + parent.width - 3.0;
            if (x + child_width + 4.0 > client.width) {
                x = parent.x - child_width + 3.0;
            }
            panels[depth].child_source_index = index;
            open_panel(depth + 1U, item.children, x, row.y, index);
            panels[depth].child_source_index = index;
        }
        if (focus_first && window) {
            if (const Control::Ptr first = first_focusable(depth + 1U)) {
                static_cast<void>(window->request_focus(first));
            }
        }
    }

    void close_submenu(std::size_t depth) {
        if (depth == 0U) {
            if (navigate_root(-1)) return;
            close();
            return;
        }
        const std::optional<std::size_t> parent = panels[depth].parent_index;
        remove_panels_from(depth);
        if (window && parent && depth - 1U < panels.size() &&
            *parent < panels[depth - 1U].rows.size()) {
            static_cast<void>(window->request_focus(
                panels[depth - 1U].rows[*parent]));
        }
    }

    void move_focus(std::size_t depth, std::size_t index, int direction) {
        if (depth >= panels.size() || panels[depth].rows.empty()) return;
        const std::size_t count = panels[depth].rows.size();
        for (std::size_t step = 1U; step <= count; ++step) {
            const std::ptrdiff_t raw = static_cast<std::ptrdiff_t>(index) +
                static_cast<std::ptrdiff_t>(direction) *
                    static_cast<std::ptrdiff_t>(step);
            const std::size_t candidate = static_cast<std::size_t>(
                (raw % static_cast<std::ptrdiff_t>(count) +
                 static_cast<std::ptrdiff_t>(count)) %
                static_cast<std::ptrdiff_t>(count));
            if (panels[depth].rows[candidate]->focusable()) {
                ensure_visible(depth, candidate);
                if (window) static_cast<void>(window->request_focus(
                    panels[depth].rows[candidate]));
                return;
            }
        }
    }

    void focus_edge(std::size_t depth, bool last) {
        if (depth >= panels.size()) return;
        auto& rows = panels[depth].rows;
        if (last) {
            for (std::size_t index = rows.size(); index > 0U; --index) {
                if (rows[index - 1U]->focusable()) {
                    ensure_visible(depth, index - 1U);
                    if (window) static_cast<void>(window->request_focus(rows[index - 1U]));
                    return;
                }
            }
        } else {
            for (std::size_t index = 0U; index < rows.size(); ++index) {
                if (rows[index]->focusable()) {
                    ensure_visible(depth, index);
                    if (window) static_cast<void>(window->request_focus(rows[index]));
                    return;
                }
            }
        }
    }

    void activate(std::size_t depth, std::size_t index) {
        if (depth >= panels.size() || index >= panels[depth].rows.size()) return;
        const MenuSnapshot& item = (*panels[depth].items)[index];
        if (item.kind == MenuItemKind::submenu) {
            open_submenu(depth, index, true);
            return;
        }
        if (item.kind == MenuItemKind::separator || !item.command ||
            !item.command_state.enabled) return;
        const std::string source_id = std::string(
            panels[depth].rows[index]->stable_id().value());
        const std::string command_id = item.command->stable_id();
        if (!item.command->execute(source_id)) return;
        MenuItemInvocation invocation{owner.stable_id_, item.stable_id,
                                      command_id, source_id};
        owner.item_invoked_.emit(invocation);
        close();
    }

    [[nodiscard]] bool submenu_open(std::size_t depth,
                                    std::size_t index) const noexcept {
        return depth < panels.size() &&
               panels[depth].child_source_index == index &&
               panels.size() > depth + 1U;
    }

    [[nodiscard]] bool navigate_root(int direction) {
        return root_navigation_handler && root_navigation_handler(direction);
    }

    ContextMenu& owner;
    Window* window{};
    Control::Ptr owner_control;
    std::shared_ptr<MenuLayer> layer;
    std::vector<MenuSnapshot> snapshot;
    std::vector<PanelState> panels;
    PopupToken popup_token;
    SubscriptionToken popup_revocation;
    FocusScopeId focus_scope{};
    bool open{};
    bool closing{};
    std::function<bool(int)> root_navigation_handler;
    std::function<bool(const PointerEvent&)> outside_pointer_handler;
};

ContextMenu::ContextMenu(std::string stable_id)
    : stable_id_(std::move(stable_id)), impl_(std::make_unique<Impl>(*this)) {
    if (stable_id_.empty() || !validate_utf8(stable_id_).valid()) {
        throw std::invalid_argument("ContextMenu identity must be valid UTF-8");
    }
}

ContextMenu::~ContextMenu() {
    close();
}

void ContextMenu::set_items(std::vector<MenuItemSpec> items) {
    if (!is_alive()) throw std::logic_error("disposed ContextMenu cannot change items");
    std::unordered_set<std::string> identities;
    std::size_t count{};
    validate_specs(items, identities, 1U, count);
    close();
    items_ = std::move(items);
}

void ContextMenu::show(const Control::Ptr& owner, Point window_position) {
    if (!is_alive()) throw std::logic_error("disposed ContextMenu cannot open");
    if (!std::isfinite(window_position.x) || !std::isfinite(window_position.y)) {
        throw std::invalid_argument("ContextMenu position must be finite");
    }
    impl_->show(owner, window_position, items_);
}

void ContextMenu::close() noexcept {
    if (impl_) impl_->close();
}

bool ContextMenu::is_open() const noexcept {
    return impl_ && impl_->open;
}

void ContextMenu::set_root_navigation_handler(
    std::function<bool(int)> handler) {
    impl_->root_navigation_handler = std::move(handler);
}

void ContextMenu::set_outside_pointer_handler(
    std::function<bool(const PointerEvent&)> handler) {
    impl_->outside_pointer_handler = std::move(handler);
}

void ContextMenu::on_dispose() noexcept {
    close();
}

namespace {

constexpr double menu_strip_height = 24.0;
constexpr double menu_strip_horizontal_padding = 11.0;
constexpr double menu_strip_character_width = 7.35;

double menu_strip_item_width(std::string_view text) noexcept {
    return std::max(42.0, menu_strip_horizontal_padding * 2.0 +
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
          std::string(this->stable_id().value()) + ".menu")) {
    set_focusable(true);
    set_cursor(CursorKind::arrow);
    set_paint_plane(PaintPlane::control);
    popup_->set_root_navigation_handler(
        [this](int direction) { return navigate_root(direction); });
    popup_->set_outside_pointer_handler(
        [this](const PointerEvent& event) { return handle_popup_pointer(event); });
    popup_invoked_ = popup_->item_invoked().subscribe(
        *this, [this](const MenuItemInvocation& invocation) {
            if (!active_index_ || *active_index_ >= items_.size()) return;
            MenuStripInvocation forwarded{std::string(this->stable_id().value()),
                items_[*active_index_].stable_id, invocation};
            item_invoked_.emit(forwarded);
        });
    popup_changed_ = popup_->open_changed().subscribe(
        *this, [this](bool open_state) {
            if (open_state || switching_) return;
            if (!active_index_) return;
            active_index_.reset();
            invalidate(Dirty::paint | Dirty::semantics);
            publish_change(open_changed_, active_index_);
        });
}

MenuStrip::~MenuStrip() {
    close();
}

void MenuStrip::set_items(std::vector<MenuStripItemSpec> items) {
    require_mutable();
    std::unordered_set<std::string> identities;
    for (const auto& item : items) {
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

bool MenuStrip::open(std::size_t index) {
    require_mutable();
    if (index >= items_.size() || !items_[index].visible ||
        !items_[index].enabled || !attached_window()) return false;
    if (active_index_ == index && popup_->is_open()) return true;
    switching_ = popup_->is_open();
    if (switching_) popup_->close();
    active_index_ = index;
    hot_index_ = index;
    popup_->set_items(items_[index].items);
    const auto bounds = item_bounds();
    const Rect absolute = absolute_bounds();
    popup_->show(shared_from_this(),
                 {absolute.x + bounds[index].x,
                  absolute.y + bounds[index].y + bounds[index].height - 1.0});
    switching_ = false;
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(open_changed_, active_index_);
    return true;
}

void MenuStrip::close() noexcept {
    if (!popup_) return;
    popup_->close();
}

bool MenuStrip::is_open() const noexcept {
    return popup_ && popup_->is_open();
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
        const double width = menu_strip_item_width(display) *
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
    const auto bounds = item_bounds();
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
    const auto next = next_enabled(*active_index_, direction);
    return next && open(*next);
}

bool MenuStrip::handle_popup_pointer(const PointerEvent& event) {
    if (event.action != PointerAction::move &&
        event.action != PointerAction::down) return false;
    const auto index = index_at(event.position);
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
    const auto bounds = item_bounds();
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
        painter.draw_text_utf8({item.x + menu_strip_horizontal_padding *
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
        const auto index = index_at(event.position);
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
        const auto index = index_at(event.position);
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
        const auto next = next_enabled(*current,
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
    const auto bounds = item_bounds();
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
    const auto found = std::find_if(items_.begin(), items_.end(),
        [id](const MenuStripItemSpec& item) { return item.stable_id == id; });
    if (found == items_.end() || !found->visible || !found->enabled) return false;
    const std::size_t index = static_cast<std::size_t>(
        std::distance(items_.begin(), found));
    if (action == SemanticAction::collapse) {
        if (active_index_ != index) return false;
        close();
        return true;
    }
    if (action != SemanticAction::focus && action != SemanticAction::press &&
        action != SemanticAction::expand) return false;
    if (window()) static_cast<void>(window()->request_focus(shared_from_this()));
    set_hot(index);
    if (action != SemanticAction::focus) return open(index);
    return true;
}

bool MenuStrip::mnemonic_matches(char32_t character) const noexcept {
    if (!use_mnemonic_) return false;
    return std::any_of(items_.begin(), items_.end(),
        [character](const MenuStripItemSpec& item) {
            return item.visible && item.enabled &&
                   is_mnemonic(character, item.text);
        });
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
        const auto found = std::find(matches.begin(), matches.end(), *current);
        if (found != matches.end()) {
            const auto next = std::next(found);
            selected = next == matches.end() ? matches.front() : *next;
        }
    }
    if (window() && !window()->request_focus(shared_from_this())) {
        // The mnemonic is still owned, but prevent-mode validation may reject
        // the focus transaction that would authorize opening the popup.
        return true;
    }
    set_hot(selected);
    return open(selected);
}

void MenuStrip::on_dispose() noexcept {
    close();
    if (popup_) popup_->dispose();
}

} // namespace gui_forms
