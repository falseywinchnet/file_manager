#pragma once

#include "gui_forms/commands.hpp"
#include "gui_forms/component.hpp"
#include "gui_forms/control.hpp"
#include "gui_forms/event.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace gui_forms {

enum class MenuItemKind : std::uint8_t {
    command,
    check,
    radio,
    separator,
    submenu,
};

// Declarative presentation of shared command authority. Command state is
// snapshotted immediately before the menu opens; one invocation still flows
// through Command::execute regardless of pointer, keyboard, or semantic input.
struct MenuItemSpec final {
    std::string stable_id;
    MenuItemKind kind{MenuItemKind::command};
    std::shared_ptr<Command> command;
    std::string text;
    std::vector<MenuItemSpec> children;
};

struct MenuItemInvocation final {
    std::string menu_id;
    std::string item_id;
    std::string command_id;
    std::string source_id;
};

inline constexpr std::size_t maximum_menu_depth = 8U;
inline constexpr std::size_t maximum_menu_items = 1024U;

// Retained popup-menu controller. Its visual rows are temporary popup
// presentations, while MenuItemSpec identities and bound Command instances are
// stable application state. Click-away/Escape revoke the popup and restore the
// invoker's focus scope deterministically.
class ContextMenu final : public Component {
public:
    explicit ContextMenu(std::string stable_id);
    ~ContextMenu() override;

    [[nodiscard]] const std::string& stable_id() const noexcept {
        return stable_id_;
    }
    [[nodiscard]] const std::vector<MenuItemSpec>& items() const noexcept {
        return items_;
    }
    void set_items(std::vector<MenuItemSpec> items);

    void show(const Control::Ptr& owner, Point window_position);
    void close() noexcept;
    [[nodiscard]] bool is_open() const noexcept;

    [[nodiscard]] Event<const MenuItemInvocation&>& item_invoked() noexcept {
        return item_invoked_;
    }
    [[nodiscard]] Event<bool>& open_changed() noexcept { return open_changed_; }

protected:
    void on_dispose() noexcept override;

private:
    friend class MenuStrip;
    struct Impl;

    void set_root_navigation_handler(std::function<bool(int)> handler);
    void set_outside_pointer_handler(
        std::function<bool(const PointerEvent&)> handler);

    std::string stable_id_;
    std::vector<MenuItemSpec> items_;
    std::unique_ptr<Impl> impl_;
    Event<const MenuItemInvocation&> item_invoked_;
    Event<bool> open_changed_;
};

struct MenuStripItemSpec final {
    std::string stable_id;
    std::string text;
    std::vector<MenuItemSpec> items;
    bool enabled{true};
    bool visible{true};
};

struct MenuStripInvocation final {
    std::string menu_strip_id;
    std::string top_level_id;
    MenuItemInvocation item;
};

// Retained top-level application menu. The bar stays in ordinary layout; its
// active menu is presented by ContextMenu as a separate window popup root.
// Pointer, keyboard, and semantic activation converge on the same shared
// Command instances contained by MenuItemSpec.
class MenuStrip final : public Control {
public:
    explicit MenuStrip(StableId stable_id);
    ~MenuStrip() override;

    [[nodiscard]] const std::vector<MenuStripItemSpec>& items() const noexcept {
        return items_;
    }
    void set_items(std::vector<MenuStripItemSpec> items);
    [[nodiscard]] std::optional<std::size_t> active_index() const noexcept {
        return active_index_;
    }
    bool open(std::size_t index);
    void close() noexcept;
    [[nodiscard]] bool is_open() const noexcept;

    [[nodiscard]] Event<const MenuStripInvocation&>& item_invoked() noexcept {
        return item_invoked_;
    }
    [[nodiscard]] Event<std::optional<std::size_t>>& open_changed() noexcept {
        return open_changed_;
    }

    [[nodiscard]] Size measure(Size available) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    void on_pointer(PointerEvent& event) override;
    void on_key(KeyEvent& event) override;
    void on_focus_changed(bool focused) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;
    [[nodiscard]] std::vector<SemanticNode> semantic_virtual_children() const override;
    bool on_semantic_child_action(std::string_view stable_id,
                                  SemanticAction action,
                                  std::string_view value) override;

protected:
    void on_dispose() noexcept override;

private:
    [[nodiscard]] std::vector<Rect> item_bounds() const;
    [[nodiscard]] std::optional<std::size_t> index_at(Point window_point) const;
    [[nodiscard]] std::optional<std::size_t> next_enabled(
        std::size_t start, int direction) const;
    bool navigate_root(int direction);
    bool handle_popup_pointer(const PointerEvent& event);
    void set_hot(std::optional<std::size_t> index);

    std::vector<MenuStripItemSpec> items_;
    std::unique_ptr<ContextMenu> popup_;
    std::optional<std::size_t> active_index_;
    std::optional<std::size_t> hot_index_;
    bool focused_{};
    bool switching_{};
    SubscriptionToken popup_invoked_;
    SubscriptionToken popup_changed_;
    Event<const MenuStripInvocation&> item_invoked_;
    Event<std::optional<std::size_t>> open_changed_;
};

} // namespace gui_forms
