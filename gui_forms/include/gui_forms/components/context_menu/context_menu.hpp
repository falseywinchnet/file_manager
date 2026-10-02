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
#include <utility>
#include <vector>

namespace gui_forms {

enum class MenuItemKind : std::uint8_t {
    command,
    check,
    radio,
    separator,
    submenu,
};

// Pointer opening keeps the popup focused without selecting a command until
// hover or keyboard navigation. Keyboard opening selects the first enabled row.
enum class MenuOpenMode : std::uint8_t { keyboard, pointer };

// Declarative presentation of shared command authority. Command state is
// snapshotted immediately before the menu opens; one invocation still flows
// through Command::execute regardless of pointer, keyboard, or semantic input.
struct MenuItemSpec final {
    MenuItemSpec() = default;
    MenuItemSpec(std::string stable_identity, MenuItemKind item_kind,
                 std::shared_ptr<Command> item_command = {},
                 std::string item_text = {},
                 std::vector<MenuItemSpec> item_children = {})
        : stable_id(std::move(stable_identity)), kind(item_kind),
          command(std::move(item_command)), text(std::move(item_text)),
          children(std::move(item_children)) {}

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
    [[nodiscard]] double preferred_width() const noexcept {
        return preferred_width_;
    }
    void set_preferred_width(double width);

    // Unknown modes throw invalid_argument without closing an existing popup.
    void show(const Control::Ptr& owner, const Point window_position,
              const MenuOpenMode mode = MenuOpenMode::keyboard);
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
    double preferred_width_{268.0};
    std::unique_ptr<Impl> impl_;
    Event<const MenuItemInvocation&> item_invoked_;
    Event<bool> open_changed_;
};

} // namespace gui_forms
