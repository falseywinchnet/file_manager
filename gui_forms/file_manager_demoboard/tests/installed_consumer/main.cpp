#include "gui_forms/gui_forms.hpp"

#include <memory>
#include <string>
#include <vector>

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
    auto popup_owner = std::make_shared<gui_forms::Button>(
        gui_forms::StableId("installed.popup.owner"), "Open popup");
    popup_owner->set_requested_bounds({190, 116, 120, 28});
    popup_owner->set_expanded_state(false);
    root->add_child(popup_owner);
    auto stable_list = std::make_shared<gui_forms::ListBox>(
        gui_forms::StableId("installed.stable_list"));
    stable_list->set_requested_bounds({320, 244, 192, 82});
    stable_list->set_items({"Alpha", "Beta"});
    stable_list->set_item_stable_ids(
        {"installed.stable_list.alpha", "installed.stable_list.beta"});
    root->add_child(stable_list);
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
    auto correspondence = std::make_shared<gui_forms::CorrespondenceView>(
        gui_forms::StableId("installed.correspondence"));
    correspondence->set_requested_bounds({8, 340, 504, 170});
    std::vector<gui_forms::CorrespondenceItem> correspondence_items{
        {"installed.correspondence.first", "First evidence", "Fixture / first",
         "Exact installed-package reason", "Matched public evidence text", "98%",
         "Provider information", {"Installed index"}, "Generation 1",
         gui_forms::ObjectGlyph::document},
        {"installed.correspondence.second", "Second evidence", "Fixture / second",
         "Second installed-package reason", "Another matched excerpt", "82%",
         "Provider information", {"Installed index"}, "Generation 1",
         gui_forms::ObjectGlyph::image},
    };
    correspondence_items.front().emphasis_terms = {"evidence"};
    correspondence->set_items(std::move(correspondence_items));
    correspondence->set_selected_id("installed.correspondence.first");
    correspondence->set_pinned_id("installed.correspondence.first");
    root->add_child(correspondence);
    auto rack = std::make_shared<gui_forms::InstrumentRack>(
        gui_forms::StableId("installed.instrument_rack"));
    rack->set_requested_bounds({8, 516, 504, 100});
    rack->set_modules({
        {"installed.criterion.kind", "Kind criterion",
         {{"property", "Property", "Kind",
           gui_forms::InstrumentFieldEditor::choice, {"Kind", "Name"}},
          {"value", "Value", "Images",
           gui_forms::InstrumentFieldEditor::choice,
           {"Images", "Documents"}}},
         "live · installed consumer", gui_forms::InstrumentModuleState::live,
         10, true, true},
        {"installed.criterion.content", "Content criterion",
         {{"value", "Value", "Facade",
           gui_forms::InstrumentFieldEditor::text}},
         "staged · installed consumer",
         gui_forms::InstrumentModuleState::staged, 5, true, true},
    });
    root->add_child(rack);
    gui_forms::Window window(root, {520, 624});
    window.perform_layout();
    gui_forms::AnchoredPopupPlacement popup_placement;
    popup_placement.preferred_size = {160, 80};
    popup_placement.horizontal_alignment =
        gui_forms::PopupHorizontalAlignment::far;
    popup_placement.gap = 2;
    auto popup_content = std::make_shared<gui_forms::Panel>(
        gui_forms::StableId("installed.popup.content"));
    auto popup_layer = std::make_shared<gui_forms::AnchoredPopupLayer>(
        gui_forms::StableId("installed.popup.layer"), popup_owner,
        popup_placement);
    popup_layer->set_content(popup_content);
    popup_layer->set_requested_bounds({0, 0, 520, 350});
    auto popup = window.open_popup(popup_owner, popup_layer);
    window.perform_layout();
    const bool popup_resolved = popup.connected() &&
        popup_layer->resolved_placement().bounds ==
            gui_forms::Rect{150, 146, 160, 80};
    popup.disconnect();
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
    std::string correspondence_activation;
    auto correspondence_invoked = correspondence->item_activated().subscribe(
        [&correspondence_activation](const std::string& stable_id) {
            correspondence_activation = stable_id;
        });
    const bool correspondence_activated = window.perform_semantic_action(
        "installed.correspondence.first.activate",
        gui_forms::SemanticAction::press);
    return window.find("installed.consumer.label") == label &&
        tree->selected_id() == "installed.node.child" &&
        objects->selected_id() == "installed.object" &&
        objects->selected_ids().size() == 2U && menu_visible && strip_open &&
        accelerated && accelerator.connected() && accelerator_invocations == 1U &&
        property_edit && collapsed && restored && !split->second_collapsed() &&
        split->splitter_distance() == remembered_split &&
        popup_resolved &&
        correspondence->items().size() == 2U &&
        correspondence->children().empty() &&
        correspondence->expanded("installed.correspondence.first") &&
        correspondence_activated &&
        correspondence_activation == "installed.correspondence.first" &&
        rack->modules().size() == 2U &&
        std::dynamic_pointer_cast<gui_forms::ComboBox>(
            rack->field_editor("installed.criterion.kind", "property")) &&
        std::dynamic_pointer_cast<gui_forms::TextBox>(
            rack->field_editor("installed.criterion.content", "value")) &&
        stable_list->item_stable_id(1U) == "installed.stable_list.beta" &&
        popup_owner->expanded_state() == false &&
        button->text() == "Installed command" &&
        button->accessible_description() ==
            "Installed public command metadata" ? 0 : 1;
}
