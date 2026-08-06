#include "gui_forms/inspection_controls.hpp"
#include "gui_forms/window.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

const SemanticNode* find_semantic(const std::vector<SemanticNode>& nodes,
                                  std::string_view id) {
    for (const auto& node : nodes) {
        if (node.stable_id == id) return &node;
        if (const auto* found = find_semantic(node.children, id)) return found;
    }
    return nullptr;
}

std::shared_ptr<PropertyList> make_properties() {
    auto properties = make_control<PropertyList>(StableId("inspection.properties"));
    properties->set_requested_bounds({0.0, 0.0, 288.0, 260.0});
    properties->set_accessible_name("Selection properties");
    properties->set_groups({
        {"inspection.identity", "IDENTITY", {
            {"inspection.kind", "Kind", "PNG image", "Fixture kind"},
            {"inspection.location", "Location", "~/Work/Projects",
             "Fixture location"},
        }},
        {"inspection.editable", "EDITABLE", {
            {"inspection.name", "Name", "Facade Study.png",
             "Session-only fixture name", PropertyEditorKind::text, {}, {},
             true, true},
            {"inspection.handler", "Opens with", "Preview",
             "Session-only fixture handler", PropertyEditorKind::choice,
             {"Preview", "Image Laboratory"}},
        }},
    });
    return properties;
}

void test_grouped_editors_commit_cancel_validation_and_semantics() {
    auto properties = make_properties();
    Window window(properties, {288.0, 260.0});
    window.perform_layout();
    auto name = std::dynamic_pointer_cast<TextBox>(
        properties->editor("inspection.name"));
    auto handler = std::dynamic_pointer_cast<ComboBox>(
        properties->editor("inspection.handler"));
    require(name && handler && name->stable_id().value() ==
                "inspection.name.editor" &&
                handler->stable_id().value() == "inspection.handler.editor",
            "PropertyList must own stable stock editor controls");

    std::string commit_trace;
    auto committed = properties->value_committed().subscribe(
        [&commit_trace](const PropertyValueChange& change) {
            commit_trace += change.row_id + "=" + change.current_value + ";";
        });
    require(window.request_focus(name), "property text editor must focus");
    name->select_all();
    require(window.dispatch_text({"Facade Final.png"}) &&
                window.dispatch_key({KeyAction::down, PhysicalKey::enter}) &&
                properties->value("inspection.name") == "Facade Final.png" &&
                commit_trace == "inspection.name=Facade Final.png;",
            "Enter must commit one PropertyList text value through public events");
    name->select_all();
    require(window.dispatch_text({"discard me"}) &&
                window.dispatch_key({KeyAction::down, PhysicalKey::escape}) &&
                name->text() == "Facade Final.png" &&
                properties->value("inspection.name") == "Facade Final.png",
            "Escape must restore the last committed text property value");

    handler->set_selected_index(1U);
    require(properties->value("inspection.handler") == "Image Laboratory" &&
                commit_trace.ends_with(
                    "inspection.handler=Image Laboratory;"),
            "choice selection must commit through the same property event model");

    const double height_before = properties->content_height();
    require(properties->set_validation("inspection.name", "Name already exists") &&
                properties->content_height() ==
                    height_before + 18.0,
            "inline validation must participate in the one owning scroll geometry");
    const SemanticSnapshot semantic = window.semantic_snapshot();
    const SemanticNode* grid = find_semantic(semantic.roots,
                                              "inspection.properties");
    const SemanticNode* group = find_semantic(semantic.roots,
                                               "inspection.editable");
    const SemanticNode* row = find_semantic(semantic.roots, "inspection.kind");
    const SemanticNode* editor = find_semantic(semantic.roots,
                                                "inspection.name.editor");
    require(grid && grid->role == SemanticRole::property_grid && group &&
                group->role == SemanticRole::property_group && row &&
                row->role == SemanticRole::property_row && editor &&
                editor->description.find("Name already exists") !=
                    std::string::npos,
            "property semantics must expose grid/group/row and editor validation");

    require(window.perform_semantic_action("inspection.editable",
                                           SemanticAction::collapse) &&
                !name->visible() && !handler->visible() &&
                properties->content_height() < height_before,
            "semantic group collapse must remove its editors from layout and task order");
    require(window.perform_semantic_action("inspection.editable",
                                           SemanticAction::expand) &&
                name->visible() && handler->visible(),
            "semantic group expansion must restore retained editor instances");
}

void test_scrolling_responsive_layout_and_validation_bounds() {
    auto properties = make_properties();
    auto preview = make_control<Panel>(StableId("inspection.preview"));
    properties->set_header_content(preview, 72.0);
    Window window(properties, {288.0, 120.0});
    window.perform_layout();
    require(properties->content_height() > 120.0,
            "constrained PropertyList fixture must require its one scroll plane");
    require(properties->header_content() == preview &&
                properties->header_height() == 72.0,
            "preview/header content must participate in the PropertyList scroll owner");
    properties->set_scroll_offset(10000.0);
    require(properties->scroll_offset() > 0.0 &&
                properties->scroll_offset() <=
                    properties->content_height() - 120.0,
            "PropertyList scrolling must clamp to its content extent");
    const auto name = properties->editor("inspection.name");
    require(window.request_focus(name) &&
                name->absolute_bounds().y >= properties->absolute_bounds().y &&
                name->absolute_bounds().y + name->absolute_bounds().height <=
                    properties->absolute_bounds().y +
                        properties->absolute_bounds().height,
            "focusing an editor must reveal it inside the owning scroll plane");

    window.resize({210.0, 180.0});
    window.perform_layout();
    require(name->absolute_bounds().x <= 10.0 &&
                name->absolute_bounds().width >= 190.0,
            "narrow PropertyList must stack label and editor without clipping");

    bool duplicate_rejected{};
    try {
        properties->set_groups({
            {"same", "First", {{"duplicate", "A", "1"}}},
            {"same", "Second", {{"duplicate", "B", "2"}}},
        });
    } catch (const std::invalid_argument&) {
        duplicate_rejected = true;
    }
    require(duplicate_rejected,
            "PropertyList must reject duplicate group/row identities");
}

} // namespace

int main() {
    try {
        test_grouped_editors_commit_cancel_validation_and_semantics();
        test_scrolling_responsive_layout_and_validation_bounds();
        std::cout << "gui_forms_inspection_controls_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_inspection_controls_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
