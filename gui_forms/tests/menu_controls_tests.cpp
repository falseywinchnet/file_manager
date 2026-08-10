#include "gui_forms/menu_controls.hpp"
#include "gui_forms/container_controls.hpp"
#include "gui_forms/window.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

const SemanticNode* find_semantic(const std::vector<SemanticNode>& nodes,
                                  std::string_view id) {
    for (const SemanticNode& node : nodes) {
        if (node.stable_id == id) return &node;
        if (const SemanticNode* found = find_semantic(node.children, id)) return found;
    }
    return nullptr;
}

void test_context_menu_command_snapshot_keyboard_nesting_and_restore() {
    auto root = make_control<TableLayoutPanel>(StableId("root"));
    root->set_column_count(1U);
    root->set_row_count(1U);
    root->set_column_style(0U, {TableSizeMode::percent, 1.0});
    root->set_row_style(0U, {TableSizeMode::percent, 1.0});
    auto owner = make_control<Button>(StableId("owner"), "Open context menu");
    owner->set_requested_bounds({20.0, 20.0, 160.0, 32.0});
    root->add_child(owner);
    root->set_cell_position(*owner, {0U, 0U});
    Window window(root, {400.0, 300.0});
    require(window.request_focus(owner), "menu invoker must accept initial focus");

    auto open = std::make_shared<Command>("file.open", "Open");
    open->set_default_action(true);
    auto copy = std::make_shared<Command>("edit.copy", "Copy");
    copy->set_shortcut("Ctrl+C");
    copy->set_description("Copy fixture reference");
    auto remove = std::make_shared<Command>("file.delete", "Delete");
    remove->set_enabled(false);
    remove->set_destructive(true);
    remove->set_availability_reason("Fixture is read-only");
    auto details = std::make_shared<Command>("view.details", "Details");
    details->set_checked(true);
    auto icons = std::make_shared<Command>("view.icons", "Icons");

    ContextMenu menu("object.context");
    menu.set_preferred_width(180.0);
    require(menu.preferred_width() == 180.0,
            "ContextMenu must retain an explicit preferred popup width");
    menu.set_items({
        {"open", MenuItemKind::command, open},
        {"separator.primary", MenuItemKind::separator},
        {"copy", MenuItemKind::command, copy},
        {"delete", MenuItemKind::command, remove},
        {"details", MenuItemKind::check, details},
        {"view", MenuItemKind::submenu, {}, "View", {
            {"view.icons", MenuItemKind::radio, icons},
            {"view.details", MenuItemKind::radio, details},
        }},
    });

    std::string command_trace;
    auto copy_invoked = copy->invoked().subscribe(
        [&command_trace](const CommandInvocation& invocation) {
            command_trace = invocation.command_id + "@" + invocation.source_id;
        });
    std::size_t open_changes{};
    auto opened = menu.open_changed().subscribe(
        [&open_changes](bool) { ++open_changes; });

    menu.show(owner, {395.0, 295.0});
    require(menu.is_open() && window.focus_scope_depth() == 1U,
            "opening a context menu must create one contained focus scope");
    const auto panel = window.find("object.context.popup.panel.0");
    require(panel && panel->absolute_bounds().width == 180.0 &&
                panel->absolute_bounds().x >= 0.0 &&
                panel->absolute_bounds().y >= 0.0 &&
                panel->absolute_bounds().x + panel->absolute_bounds().width <= 400.0 &&
                panel->absolute_bounds().y + panel->absolute_bounds().height <= 300.0,
            "root menu placement must avoid every client edge");
    const auto snapshot = window.semantic_snapshot();
    const SemanticNode* menu_node = find_semantic(
        snapshot.roots, "object.context.popup.panel.0");
    const SemanticNode* details_node = find_semantic(
        snapshot.roots, "object.context.popup.row.details");
    const SemanticNode* delete_node = find_semantic(
        snapshot.roots, "object.context.popup.row.delete");
    require(menu_node && menu_node->role == SemanticRole::menu && details_node &&
                details_node->role == SemanticRole::menu_item &&
                has_semantic_state(details_node->states, SemanticState::checked) &&
                delete_node && delete_node->description.find("read-only") !=
                    std::string::npos,
            "menu semantics must publish menu/item/check and availability facts");

    require(window.dispatch_key({KeyAction::down, PhysicalKey::down}) &&
                window.focused_control()->stable_id().value() ==
                    "object.context.popup.row.copy",
            "Down must skip a separator and reach the next enabled command");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::down}) &&
                window.focused_control()->stable_id().value() ==
                    "object.context.popup.row.details",
            "keyboard traversal must skip disabled command rows");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::up}) &&
                window.dispatch_key({KeyAction::down, PhysicalKey::enter}) &&
                !menu.is_open() && window.focus_scope_depth() == 0U &&
                window.focused_control() == owner &&
                command_trace.starts_with("edit.copy@object.context.popup.row.copy"),
            "Enter must execute shared command once, close, and restore invoker focus");

    std::string nested_trace;
    auto icons_invoked = icons->invoked().subscribe(
        [&nested_trace](const CommandInvocation& invocation) {
            nested_trace = invocation.command_id;
        });
    menu.show(owner, {120.0, 80.0});
    require(window.dispatch_key({KeyAction::down, PhysicalKey::end}) &&
                window.focused_control()->stable_id().value() ==
                    "object.context.popup.row.view" &&
                window.dispatch_key({KeyAction::down, PhysicalKey::right}) &&
                window.find("object.context.popup.panel.1") != nullptr &&
                window.focused_control()->stable_id().value() ==
                    "object.context.popup.row.view.icons",
            "Right must open a bounded nested submenu and focus its first command");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::enter}) &&
                nested_trace == "view.icons" && !menu.is_open(),
            "nested keyboard activation must converge on shared command authority");

    menu.show(owner, {120.0, 80.0});
    require(window.dispatch_pointer({PointerAction::down,
                                     PointerButton::primary,
                                     {2.0, 2.0}}) &&
                !menu.is_open() && window.focused_control() == owner,
            "click-away must revoke the popup and restore focus");
    require(open_changes == 6U,
            "three complete open/close cycles must publish six ordered state changes");
}

void test_context_menu_scroll_and_validation_bounds() {
    auto root = make_control<Panel>(StableId("scroll.root"));
    auto owner = make_control<Button>(StableId("scroll.owner"), "Menu");
    owner->set_requested_bounds({4.0, 4.0, 80.0, 30.0});
    root->add_child(owner);
    Window window(root, {320.0, 190.0});
    require(window.request_focus(owner), "scroll menu owner must focus");

    std::vector<MenuItemSpec> specs;
    std::vector<std::shared_ptr<Command>> commands;
    for (std::size_t index = 0; index < 30U; ++index) {
        auto command = std::make_shared<Command>(
            "long." + std::to_string(index), "Command " + std::to_string(index));
        commands.push_back(command);
        specs.push_back({"item." + std::to_string(index),
                         MenuItemKind::command, command});
    }
    ContextMenu menu("long.menu");
    menu.set_items(std::move(specs));
    menu.show(owner, {20.0, 20.0});
    const auto panel = window.find("long.menu.popup.panel.0");
    require(panel && panel->absolute_bounds().height <= 182.0,
            "long menu must use a bounded on-screen scroll viewport");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::end}) &&
                window.focused_control()->stable_id().value() ==
                    "long.menu.popup.row.item.29",
            "End must scroll the last logical command into the visible menu viewport");
    const Rect focused_bounds = window.focused_control()->absolute_bounds();
    require(panel->absolute_bounds().contains(Point{
                focused_bounds.x + focused_bounds.width * .5,
                focused_bounds.y + focused_bounds.height * .5}),
            "End must reveal the last logical command inside the menu viewport");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::escape}) &&
                !menu.is_open(),
            "Escape must close a scrolled menu without selecting a command");

    bool duplicate_rejected{};
    try {
        menu.set_items({{"same", MenuItemKind::command, commands.front()},
                        {"same", MenuItemKind::command, commands.back()}});
    } catch (const std::invalid_argument&) {
        duplicate_rejected = true;
    }
    require(duplicate_rejected,
            "menu model validation must reject duplicate stable identities");
}

void test_menu_strip_retained_switching_commands_and_semantics() {
    auto root = make_control<TableLayoutPanel>(StableId("menustrip.root"));
    root->set_column_count(1U);
    root->set_row_count(2U);
    root->set_column_style(0U, {TableSizeMode::percent, 1.0});
    root->set_row_style(0U, {TableSizeMode::absolute, 24.0});
    root->set_row_style(1U, {TableSizeMode::percent, 1.0});
    auto strip = make_control<MenuStrip>(StableId("menustrip"));
    strip->set_item_padding(18.0);
    require(strip->item_padding() == 18.0,
            "MenuStrip must retain explicit top-level item padding");
    auto content = make_control<Panel>(StableId("menustrip.content"));
    root->add_child(strip);
    root->add_child(content);
    root->set_cell_position(*strip, {0U, 0U});
    root->set_cell_position(*content, {0U, 1U});

    auto open = std::make_shared<Command>("file.open", "Open");
    auto details = std::make_shared<Command>("view.details", "Details");
    auto icons = std::make_shared<Command>("view.icons", "Icons");
    strip->set_items({
        {"menustrip.file", "File", {
            {"file.open", MenuItemKind::command, open},
        }},
        {"menustrip.view", "View", {
            {"view.icons", MenuItemKind::radio, icons},
            {"view.details", MenuItemKind::radio, details},
        }},
    });
    Window window(root, {420.0, 240.0});

    require(window.request_focus(strip), "menu strip must accept keyboard focus");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::down}) &&
                strip->is_open() && strip->active_index() == 0U &&
                window.focus_scope_depth() == 1U,
            "Down on the menu bar must open its focused retained menu");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::right}) &&
                strip->is_open() && strip->active_index() == 1U &&
                window.focused_control()->stable_id().value() ==
                    "menustrip.menu.popup.row.view.icons",
            "Right from a root command must switch top-level menus in one popup scope");

    std::string trace;
    auto invoked = strip->item_invoked().subscribe(
        [&trace](const MenuStripInvocation& invocation) {
            trace = invocation.top_level_id + "/" + invocation.item.command_id;
        });
    require(window.dispatch_key({KeyAction::down, PhysicalKey::down}) &&
                window.dispatch_key({KeyAction::down, PhysicalKey::enter}) &&
                trace == "menustrip.view/view.details" && !strip->is_open() &&
                window.focused_control() == strip,
            "menu-bar keyboard invocation must execute and restore bar focus");

    const SemanticSnapshot snapshot = window.semantic_snapshot();
    const SemanticNode* bar = find_semantic(snapshot.roots, "menustrip");
    const SemanticNode* file = find_semantic(snapshot.roots, "menustrip.file");
    require(bar && bar->role == SemanticRole::menu_bar && file &&
                file->role == SemanticRole::menu_bar_item,
            "menu strip must publish distinct bar and top-level item roles");
    require(window.perform_semantic_action("menustrip.file",
                                           SemanticAction::expand) &&
                strip->is_open() && strip->active_index() == 0U,
            "semantic expansion must open the same retained popup path");
    strip->close();

    require(window.dispatch_pointer({PointerAction::down, PointerButton::primary,
                                     {70.0, 12.0}}) &&
                strip->active_index() == 1U,
            "pointer activation must open the top-level item under its geometry");
    require(window.dispatch_pointer({PointerAction::move, PointerButton::none,
                                     {12.0, 12.0}}) &&
                strip->active_index() == 0U,
            "moving across the open menu bar must switch menus without click-through");
    strip->close();
}

void test_menu_mnemonics_strip_markers_and_popup_activation() {
    auto root = make_control<Panel>(StableId("mnemonic.root"));
    auto strip = make_control<MenuStrip>(StableId("mnemonic.strip"));
    strip->set_requested_bounds({0.0, 0.0, 360.0, 24.0});
    auto owner = make_control<Button>(StableId("mnemonic.owner"), "Owner");
    owner->set_requested_bounds({20.0, 50.0, 90.0, 28.0});
    root->add_child(strip);
    root->add_child(owner);

    auto first = std::make_shared<Command>("first", "First");
    auto second = std::make_shared<Command>("second", "Second");
    strip->set_items({
        {"mnemonic.file", "&File", {
            {"first", MenuItemKind::command, first},
        }},
        {"mnemonic.format", "&Format", {
            {"second", MenuItemKind::command, second},
        }},
    });
    Window window(root, {360.0, 220.0});
    window.perform_layout();

    const std::vector<SemanticNode> top_level = strip->semantic_virtual_children();
    require(top_level.size() == 2U && top_level[0].name == "File" &&
                top_level[1].name == "Format",
            "MenuStrip mnemonic markers must stay out of retained semantics");

    KeyEvent alt_f{KeyAction::down, PhysicalKey::f};
    alt_f.modifiers = Modifier::alt;
    require(window.dispatch_key(alt_f) && strip->is_open() &&
                strip->active_index() == 0U,
            "a top-level menu mnemonic must focus and open its retained popup");
    strip->close();
    require(window.dispatch_key(alt_f) && strip->is_open() &&
                strip->active_index() == 1U,
            "duplicate top-level menu mnemonics must cycle deterministically");
    strip->close();

    auto copy = std::make_shared<Command>("copy", "Copy");
    std::uint64_t copy_count{};
    auto copy_invoked = copy->invoked().subscribe(
        [&copy_count](const CommandInvocation&) { ++copy_count; });
    ContextMenu popup("mnemonic.popup");
    popup.set_items({
        {"copy", MenuItemKind::command, copy, "&Copy"},
    });
    popup.show(owner, {20.0, 84.0});
    const Control::Ptr row = window.find("mnemonic.popup.popup.row.copy");
    require(row && row->semantic_descriptor().name == "Copy",
            "popup menu rows must expose marker-free authored text");
    KeyEvent alt_c{KeyAction::down, PhysicalKey::c};
    alt_c.modifiers = Modifier::alt;
    require(window.dispatch_key(alt_c) && copy_count == 1U && !popup.is_open(),
            "an active popup mnemonic must execute through shared command authority");

    strip->set_use_mnemonic(false);
    require(!window.dispatch_key(alt_f),
            "UseMnemonic false must leave the ampersand literal and revoke activation");
}

} // namespace

int main() {
    try {
        test_context_menu_command_snapshot_keyboard_nesting_and_restore();
        test_context_menu_scroll_and_validation_bounds();
        test_menu_strip_retained_switching_commands_and_semantics();
        test_menu_mnemonics_strip_markers_and_popup_activation();
        std::cout << "gui_forms_menu_controls_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_menu_controls_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
