#include "gui_forms/gui_forms.hpp"

#include <memory>

int main() {
    auto root = std::make_shared<gui_forms::Panel>(
        gui_forms::StableId("installed.consumer.root"));
    auto label = std::make_shared<gui_forms::Label>(
        gui_forms::StableId("installed.consumer.label"),
        "GUI.Forms installed package consumer");
    label->set_requested_bounds({8, 8, 260, 24});
    root->add_child(label);
    auto tree = std::make_shared<gui_forms::TreeView>(
        gui_forms::StableId("installed.consumer.tree"));
    tree->set_requested_bounds({8, 36, 140, 80});
    tree->set_items({{"installed.node.root", "Root", 0, true, true},
                     {"installed.node.child", "Child", 1}});
    tree->set_selected_id("installed.node.child");
    root->add_child(tree);
    auto objects = std::make_shared<gui_forms::ObjectView>(
        gui_forms::StableId("installed.consumer.objects"));
    objects->set_requested_bounds({156, 36, 156, 80});
    objects->set_items({{"installed.object", "Fixture", "1 KB",
                         "Installed consumer fixture",
                         gui_forms::ObjectGlyph::document},
                        {"installed.object.second", "Second fixture", "2 KB",
                         "Installed consumer second fixture",
                         gui_forms::ObjectGlyph::folder}});
    objects->set_selected_ids({"installed.object", "installed.object.second"},
                              "installed.object");
    root->add_child(objects);
    auto button = std::make_shared<gui_forms::Button>(
        gui_forms::StableId("installed.consumer.command"), "stale");
    auto command = std::make_shared<gui_forms::Command>(
        "installed.command", "Installed command");
    command->set_description("Installed public command metadata");
    gui_forms::CommandBinding binding(command, button);
    button->set_requested_bounds({8, 116, 180, 28});
    root->add_child(button);
    gui_forms::ContextMenu menu("installed.menu");
    menu.set_items({{"installed.menu.command",
                     gui_forms::MenuItemKind::command, command}});
    auto menu_strip = std::make_shared<gui_forms::MenuStrip>(
        gui_forms::StableId("installed.menu_strip"));
    menu_strip->set_requested_bounds({0, 152, 320, 24});
    menu_strip->set_items({{"installed.menu.file", "File", {
        {"installed.menu.file.command", gui_forms::MenuItemKind::command,
         command},
    }}});
    root->add_child(menu_strip);
    auto properties = std::make_shared<gui_forms::PropertyList>(
        gui_forms::StableId("installed.properties"));
    properties->set_requested_bounds({320, 36, 192, 200});
    properties->set_groups({
        {"installed.properties.identity", "IDENTITY", {
            {"installed.properties.kind", "Kind", "Fixture", "Object kind"},
        }},
        {"installed.properties.editable", "EDITABLE", {
            {"installed.properties.name", "Name", "Fixture",
             "Session name", gui_forms::PropertyEditorKind::text},
            {"installed.properties.handler", "Opens with", "Preview",
             "Session handler", gui_forms::PropertyEditorKind::choice,
             {"Preview", "Inspector"}},
        }},
    });
    root->add_child(properties);
    auto split = std::make_shared<gui_forms::SplitContainer>(
        gui_forms::StableId("installed.split"));
    split->initialize_control_tree();
    split->set_requested_bounds({8, 184, 304, 150});
    split->set_splitter_distance(190);
    split->set_collapse_panel(gui_forms::SplitFixedPanel::second);
    root->add_child(split);
    gui_forms::Window window(root, {520, 350});
    window.perform_layout();
    std::size_t accelerator_invocations{};
    auto invoked = command->invoked().subscribe(
        [&accelerator_invocations](const gui_forms::CommandInvocation&) {
            ++accelerator_invocations;
        });
    auto accelerator = window.register_accelerator(
        *command, {gui_forms::PhysicalKey::left, gui_forms::Modifier::alt},
        [command] { return command->execute("installed.accelerator"); },
        {true});
    const bool accelerated = window.dispatch_key(
        {gui_forms::KeyAction::down, gui_forms::PhysicalKey::left,
         gui_forms::Modifier::alt});
    menu.show(button, {8, 8});
    const bool menu_visible = window.find("installed.menu.popup.panel.0") != nullptr;
    menu.close();
    const bool strip_open = menu_strip->open(0U) &&
        window.find("installed.menu_strip.menu.popup.panel.0") != nullptr;
    menu_strip->close();
    const bool property_edit = properties->set_validation(
        "installed.properties.name", "Installed validation") &&
        properties->set_value("installed.properties.name", "Renamed fixture") &&
        properties->value("installed.properties.name") == "Renamed fixture";
    const double remembered_split = split->splitter_distance();
    const bool collapsed = window.perform_semantic_action(
        "installed.split", gui_forms::SemanticAction::collapse);
    window.perform_layout();
    const bool restored = window.perform_semantic_action(
        "installed.split", gui_forms::SemanticAction::expand);
    window.perform_layout();
    return window.find("installed.consumer.label") == label &&
        tree->selected_id() == "installed.node.child" &&
        objects->selected_id() == "installed.object" &&
        objects->selected_ids().size() == 2U && menu_visible && strip_open &&
        accelerated && accelerator.connected() && accelerator_invocations == 1U &&
        property_edit && collapsed && restored && !split->second_collapsed() &&
        split->splitter_distance() == remembered_split &&
        button->text() == "Installed command" &&
        button->accessible_description() ==
            "Installed public command metadata" ? 0 : 1;
}
