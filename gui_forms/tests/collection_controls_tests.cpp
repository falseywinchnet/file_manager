#include "gui_forms/collection_controls.hpp"
#include "gui_forms/commands.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void test_tree_visibility_identity_and_navigation() {
    auto tree = make_control<TreeView>(StableId("tree"));
    tree->set_requested_bounds({0.0, 0.0, 240.0, 140.0});
    tree->set_items({
        {"tree.local", "Local", 0, true, true},
        {"tree.home", "quentin", 1, true, true},
        {"tree.desktop", "Desktop", 2},
        {"tree.work", "Work", 2, true, false},
        {"tree.projects", "Projects", 3},
        {"tree.reference", "Reference", 3},
        {"tree.volumes", "Volumes", 0, true, false},
        {"tree.disk", "Macintosh HD", 1},
    });
    tree->set_selected_id("tree.work");
    Window window(tree, {240.0, 140.0});
    require(tree->children().empty(),
            "TreeView must not allocate one retained control per logical item");
    const auto collapsed = tree->semantic_virtual_children();
    require(collapsed.size() == 5U && collapsed.back().stable_id == "tree.volumes",
            "collapsed TreeView descendants must leave the visible semantic window");

    std::string trace;
    auto expansion = tree->expansion_changed().subscribe(
        [&trace](const TreeExpansionChange& change) {
            trace += change.stable_id + (change.expanded ? ":open\n" : ":closed\n");
        });
    auto selection = tree->selection_changed().subscribe(
        [&trace](const TreeSelectionChange& change) {
            trace += change.current_id + ":selected\n";
        });
    tree->set_expanded("tree.work", true);
    require(tree->semantic_virtual_children().size() == 5U,
            "TreeView semantic realization must remain clipped to visible rows");
    require(window.request_focus(tree), "TreeView must accept retained focus");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::right}) &&
                tree->selected_id() == "tree.projects",
            "TreeView Right must enter the first expanded child");
    require(window.dispatch_text({"ref"}) && tree->selected_id() == "tree.reference",
            "TreeView type-to-select must use stable visible-row navigation");
    require(trace == "tree.work:open\ntree.projects:selected\ntree.reference:selected\n",
            "TreeView expansion and selection events must be deterministic");
}

void test_tree_model_validation() {
    auto tree = make_control<TreeView>(StableId("tree.invalid"));
    bool rejected{};
    try {
        tree->set_items({{"duplicate", "One", 0},
                         {"duplicate", "Two", 2}});
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected, "TreeView must reject duplicate IDs and invalid depth jumps");
}

void test_object_virtualization_view_preservation_and_input() {
    auto objects = make_control<ObjectView>(StableId("objects"));
    objects->set_requested_bounds({0.0, 0.0, 420.0, 190.0});
    std::vector<ObjectViewItem> model;
    model.reserve(1000U);
    for (std::size_t index = 0; index < 1000U; ++index) {
        model.push_back({"object." + std::to_string(index),
                         index == 777U ? "Quartz report" :
                             "Object " + std::to_string(index),
                         std::to_string(index) + " KB",
                         "Deterministic fixture object",
                         index % 5U == 0U ? ObjectGlyph::folder
                                          : ObjectGlyph::document});
    }
    objects->set_items(std::move(model));
    objects->set_selected_id("object.5");
    Window window(objects, {420.0, 190.0});
    require(objects->children().empty(),
            "ObjectView must not allocate one retained control per logical item");
    require(objects->semantic_virtual_children().size() <= 12U,
            "ObjectView semantic realization must remain bounded for 1000 items");
    require(window.request_focus(objects), "ObjectView must accept retained focus");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::right}) &&
                objects->selected_id() == "object.6",
            "ObjectView Right must use deterministic spatial navigation");
    require(window.dispatch_text({"quartz"}) &&
                objects->selected_id() == "object.777",
            "ObjectView type-to-select must reach an unrealized stable item");
    objects->set_view_mode(ObjectViewMode::details);
    require(objects->selected_id() == "object.777" &&
                objects->semantic_virtual_children().size() <= 8U,
            "ObjectView mode changes must preserve stable selection and bounded semantics");

    std::string activated;
    auto activation = objects->item_activated().subscribe(
        [&activated](const std::string& id) { activated = id; });
    require(window.dispatch_key({KeyAction::down, PhysicalKey::enter}) &&
                activated == "object.777",
            "ObjectView Enter must activate the focused stable item");
    activated.clear();
    const auto visible = objects->semantic_virtual_children();
    const auto target = std::find_if(visible.begin(), visible.end(),
        [](const SemanticNode& node) { return node.stable_id == "object.777"; });
    require(target != visible.end(), "focused ObjectView item must be realized");
    const Point center{target->bounds.x + target->bounds.width * .5,
                       target->bounds.y + target->bounds.height * .5};
    PointerEvent single_down{PointerAction::down, PointerButton::primary, center};
    PointerEvent single_up{PointerAction::up, PointerButton::primary, center};
    require(window.dispatch_pointer(single_down) && window.dispatch_pointer(single_up) &&
                activated.empty(),
            "ObjectView single click must select without invoking the default action");
    PointerEvent double_down{PointerAction::down, PointerButton::primary, center};
    double_down.click_count = 2U;
    PointerEvent double_up{PointerAction::up, PointerButton::primary, center};
    double_up.click_count = 2U;
    require(window.dispatch_pointer(double_down) && window.dispatch_pointer(double_up) &&
                activated == "object.777",
            "ObjectView native double click must invoke exactly one default action");
    require(objects->on_semantic_child_action("object.777", SemanticAction::select, {}) &&
                objects->selected_id() == "object.777",
            "ObjectView semantic selection must share the ordinary selection path");
}

Point semantic_center(const ObjectView& view, std::string_view stable_id) {
    const auto nodes = view.semantic_virtual_children();
    const auto found = std::find_if(nodes.begin(), nodes.end(),
        [stable_id](const SemanticNode& node) {
            return node.stable_id == stable_id;
        });
    require(found != nodes.end(), "test item must be semantically realized");
    return {found->bounds.x + found->bounds.width * .5,
            found->bounds.y + found->bounds.height * .5};
}

void pointer_click(Window& window, Point point, PointerButton button,
                   Modifier modifiers = Modifier::none) {
    PointerEvent down{PointerAction::down, button, point, {}, modifiers};
    PointerEvent up{PointerAction::up, button, point, {}, modifiers};
    require(window.dispatch_pointer(down) && window.dispatch_pointer(up),
            "collection pointer click must be retained and handled");
}

void require_selection(const ObjectView& view,
                       std::initializer_list<std::string_view> expected,
                       const char* message) {
    if (view.selected_ids().size() != expected.size()) {
        throw std::runtime_error(message);
    }
    std::size_t index{};
    for (const std::string_view id : expected) {
        if (view.selected_ids()[index++] != id) {
            throw std::runtime_error(message);
        }
    }
}

void test_object_multiselection_pointer_keyboard_and_semantics() {
    auto objects = make_control<ObjectView>(StableId("objects.multi"));
    objects->set_requested_bounds({0.0, 0.0, 320.0, 270.0});
    objects->set_icon_cell_size({100.0, 80.0});
    std::vector<ObjectViewItem> model;
    for (std::size_t index = 0; index < 8U; ++index) {
        model.push_back({"multi." + std::to_string(index),
                         "Item " + std::to_string(index), {}, {},
                         ObjectGlyph::document});
    }
    objects->set_items(model);
    Window window(objects, {320.0, 270.0});
    require(window.request_focus(objects),
            "multi-selection collection must accept focus");

    std::vector<ObjectSelectionChange> changes;
    auto changed = objects->selection_changed().subscribe(
        [&changes](const ObjectSelectionChange& change) {
            changes.push_back(change);
        });

    pointer_click(window, semantic_center(*objects, "multi.1"),
                  PointerButton::primary);
    require_selection(*objects, {"multi.1"},
                      "plain click must establish one stable selection");
    require(objects->selected_id() == "multi.1" &&
                objects->selection_anchor_id() == "multi.1",
            "plain click must establish primary and range-anchor identities");

    pointer_click(window, semantic_center(*objects, "multi.3"),
                  PointerButton::primary, Modifier::meta);
    require_selection(*objects, {"multi.1", "multi.3"},
                      "Command-click must toggle without discarding selection");
    require(objects->selected_id() == "multi.3",
            "newly toggled item must become the primary selection");

    pointer_click(window, semantic_center(*objects, "multi.6"),
                  PointerButton::primary, Modifier::shift);
    require_selection(*objects, {"multi.3", "multi.4", "multi.5", "multi.6"},
                      "Shift-click must replace selection with an inclusive visual range");
    require(objects->selection_anchor_id() == "multi.3" &&
                objects->focused_id() == "multi.6",
            "range selection must preserve anchor and move independent focus");

    std::string context_id;
    auto context = objects->context_requested().subscribe(
        [&context_id](const ObjectContextRequest& request) {
            context_id = request.stable_id;
        });
    pointer_click(window, semantic_center(*objects, "multi.4"),
                  PointerButton::secondary);
    require_selection(*objects, {"multi.3", "multi.4", "multi.5", "multi.6"},
                      "right click inside multiselection must preserve it");
    require(context_id == "multi.4",
            "right click must target the item without rewriting selection authority");
    const auto semantic_menu_nodes = objects->semantic_virtual_children();
    const auto semantic_menu_node = std::find_if(
        semantic_menu_nodes.begin(), semantic_menu_nodes.end(),
        [](const SemanticNode& node) { return node.stable_id == "multi.4"; });
    require(semantic_menu_node != semantic_menu_nodes.end() &&
                std::find(semantic_menu_node->actions.begin(),
                          semantic_menu_node->actions.end(),
                          SemanticAction::show_menu) != semantic_menu_node->actions.end(),
            "object rows must publish a distinct semantic Show Menu action");
    context_id.clear();
    require(objects->on_semantic_child_action(
                "multi.4", SemanticAction::show_menu, {}) &&
                context_id == "multi.4",
            "semantic Show Menu must enter the same stable context-request path");
    pointer_click(window, semantic_center(*objects, "multi.0"),
                  PointerButton::secondary);
    require_selection(*objects, {"multi.0"},
                      "right click outside selection must select its context target");

    require(window.dispatch_key({KeyAction::down, PhysicalKey::a, Modifier::control}),
            "Control+A must be consumed by the focused collection");
    require(objects->selected_ids().size() == 8U,
            "Select All must select every logical enabled item");
    const auto all_nodes = objects->semantic_virtual_children();
    require(std::all_of(all_nodes.begin(), all_nodes.end(), [](const SemanticNode& node) {
                return has_semantic_state(node.states, SemanticState::selected);
            }),
            "every realized member of a multiselection must publish selected semantics");

    require(window.dispatch_key({KeyAction::down, PhysicalKey::right,
                                 Modifier::control}) &&
                objects->focused_id() == "multi.1" &&
                objects->selected_ids().size() == 8U,
            "Control+Arrow must move focus without mutating selection");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::space,
                                 Modifier::control}),
            "Control+Space must toggle the focused stable item");
    require_selection(*objects,
                      {"multi.0", "multi.2", "multi.3", "multi.4",
                       "multi.5", "multi.6", "multi.7"},
                      "Control+Space must remove only the focused item");

    const Point background{310.0, 266.0};
    pointer_click(window, background, PointerButton::primary);
    require(objects->selected_ids().empty() && objects->selected_id().empty(),
            "plain background click must clear selection without losing focus");

    objects->set_selected_ids({"multi.6", "multi.2"}, "multi.6");
    std::reverse(model.begin(), model.end());
    objects->set_items(model);
    require_selection(*objects, {"multi.6", "multi.2"},
                      "model replacement must retain selected stable IDs in new visual order");
    objects->set_view_mode(ObjectViewMode::details);
    require_selection(*objects, {"multi.6", "multi.2"},
                      "view projection changes must preserve the complete selection set");
    require(!changes.empty() && changes.back().current_ids.size() == 2U,
            "selection events must carry deterministic previous/current snapshots");
}

void test_shared_command_binding() {
    auto command = std::make_shared<Command>("view.mode", "Icons  ▼");
    auto ribbon = make_control<Button>(StableId("ribbon.view"), "stale");
    auto status = make_control<Button>(StableId("status.view"), "stale");
    CommandBinding ribbon_binding(command, ribbon);
    CommandBinding status_binding(command, status);
    require(ribbon->text() == "Icons  ▼" && status->text() == "Icons  ▼",
            "shared command must initialize every bound presentation");
    std::string trace;
    auto invoked = command->invoked().subscribe(
        [&trace](const CommandInvocation& invocation) {
            trace += invocation.command_id + "@" + invocation.source_id + "\n";
        });
    ribbon->on_activate();
    status->on_activate();
    require(trace == "view.mode@ribbon.view\nview.mode@status.view\n",
            "bound presentations must converge on one ordered command path");
    command->set_enabled(false);
    ribbon->on_activate();
    require(!ribbon->enabled() && !status->enabled() &&
                trace == "view.mode@ribbon.view\nview.mode@status.view\n",
            "disabled command state must synchronize and reject execution");
}

} // namespace

int main() {
    try {
        test_tree_visibility_identity_and_navigation();
        test_tree_model_validation();
        test_object_virtualization_view_preservation_and_input();
        test_object_multiselection_pointer_keyboard_and_semantics();
        test_shared_command_binding();
        std::cout << "gui_forms_collection_controls_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_collection_controls_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
