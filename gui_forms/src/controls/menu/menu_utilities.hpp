#pragma once

#include "gui_forms/basic_controls.hpp"
#include "gui_forms/components/context_menu/context_menu.hpp"
#include "gui_forms/text.hpp"

#include <algorithm>
#include <stdexcept>
#include <unordered_set>

namespace gui_forms::menu_detail {

inline constexpr double menu_width = 268.0;
inline constexpr double command_row_height = 28.0;
inline constexpr double separator_row_height = 9.0;
inline constexpr double menu_border = 2.0;

[[nodiscard]] inline std::string menu_display_text(std::string_view text) {
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

inline double row_height(const MenuSnapshot& item,
                         double text_scale) noexcept {
    return (item.kind == MenuItemKind::separator ? separator_row_height
                                                 : command_row_height) *
           text_scale;
}

inline void validate_specs(const std::vector<MenuItemSpec>& items,
                           std::unordered_set<std::string>& identities,
                           std::size_t depth, std::size_t& count) {
    if (depth > maximum_menu_depth) {
        throw std::invalid_argument(
            "ContextMenu nesting exceeds the bounded depth");
    }
    for (const MenuItemSpec& item : items) {
        ++count;
        if (count > maximum_menu_items) {
            throw std::invalid_argument(
                "ContextMenu exceeds the bounded item count");
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

inline std::vector<MenuSnapshot> snapshot_items(
    const std::vector<MenuItemSpec>& specs) {
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

inline Color menu_ink(const MenuSnapshot& item,
                      const BasicControlStyle& style) noexcept {
    if (item.kind != MenuItemKind::submenu && !item.command_state.enabled) {
        return style.disabled_text;
    }
    if (item.command_state.destructive) return Color::rgba(145, 44, 42);
    return style.text;
}

} // namespace gui_forms::menu_detail
