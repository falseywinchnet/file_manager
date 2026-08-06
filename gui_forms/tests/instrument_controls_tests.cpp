#include "gui_forms/instrument_controls.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
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
        if (const SemanticNode* nested = find_semantic(node.children, id)) {
            return nested;
        }
    }
    return nullptr;
}

InstrumentFieldSpec choice(std::string id, std::string name,
                           std::string value,
                           std::vector<std::string> choices,
                           double weight = 1.0) {
    return {std::move(id), std::move(name), std::move(value),
            InstrumentFieldEditor::choice, std::move(choices), {}, weight};
}

InstrumentFieldSpec text(std::string id, std::string name,
                         std::string value, bool required = false,
                         double weight = 1.0) {
    InstrumentFieldSpec result{std::move(id), std::move(name), std::move(value),
                               InstrumentFieldEditor::text};
    result.required = required;
    result.width_weight = weight;
    return result;
}

std::vector<InstrumentModuleSpec> default_modules() {
    return {
        {"criteria.kind", "Kind criterion",
         {choice("property", "Property", "Kind", {"Kind", "Name"}),
          choice("operator", "Operator", "is", {"is", "is not"}, .8),
          choice("value", "Value", "Images", {"Images", "Documents"}, 1.1)},
         "live · inexpensive", InstrumentModuleState::live, 30, true, true},
        {"criteria.modified", "Modified criterion",
         {choice("property", "Property", "Modified", {"Modified", "Created"}),
          choice("operator", "Operator", "during", {"during", "before"}, .8),
          choice("value", "Value", "2026", {"2026", "2025"}, 1.1)},
         "live · inexpensive", InstrumentModuleState::live, 20, true, true},
        {"criteria.content", "Content criterion",
         {choice("property", "Property", "Content", {"Content", "Name"}),
          choice("operator", "Operator", "resembles", {"resembles", "contains"}, .8),
          text("value", "Value", "Facade", true, 1.1)},
         "staged · expensive · not applied", InstrumentModuleState::staged,
         10, true, true},
    };
}

std::shared_ptr<InstrumentRack> make_rack() {
    auto rack = make_control<InstrumentRack>(StableId("criteria.rack"));
    rack->set_modules(default_modules());
    auto actions = make_control<Panel>(StableId("criteria.actions"));
    actions->set_accessible_name("Criteria actions");
    auto add = make_control<Button>(StableId("criteria.add"), "+ module");
    add->set_requested_bounds({4.0, 5.0, 92.0, 28.0});
    auto apply = make_control<Button>(StableId("criteria.apply"), "Apply 1");
    apply->set_default_button(true);
    apply->set_visual_style(ButtonVisualStyle::accent);
    apply->set_requested_bounds({100.0, 5.0, 70.0, 28.0});
    actions->add_child(add);
    actions->add_child(apply);
    rack->set_action_content(actions, 170.0);
    return rack;
}

void test_real_fields_stable_reconciliation_and_semantics() {
    auto rack = make_rack();
    const Control::Ptr original_kind = rack->field_editor("criteria.kind", "property");
    const Control::Ptr content_value = rack->field_editor("criteria.content", "value");
    require(std::dynamic_pointer_cast<ComboBox>(original_kind) != nullptr &&
                std::dynamic_pointer_cast<TextBox>(content_value) != nullptr,
            "InstrumentRack must own genuine choice and text field controls");
    Window window(rack, {900.0, 110.0});
    window.perform_layout();
    const auto kind_bounds = rack->module_bounds("criteria.kind");
    const auto content_bounds = rack->module_bounds("criteria.content");
    require(kind_bounds && content_bounds && kind_bounds->width == 210.0 &&
                content_bounds->x == 430.0 &&
                rack->content_height() == 66.0,
            "reference rack must keep three 210px modules and one flexible action instrument on one line");

    auto replacement = default_modules();
    replacement[1].status_text = "live · inexpensive · generation 87";
    rack->set_modules(replacement);
    window.perform_layout();
    require(rack->field_editor("criteria.kind", "property") == original_kind,
            "stable module/field identities must retain editor instances across model replacement");

    const SemanticSnapshot semantics = window.semantic_snapshot();
    const SemanticNode* group = find_semantic(semantics.roots, "criteria.rack");
    const SemanticNode* staged = find_semantic(semantics.roots, "criteria.content");
    const SemanticNode* state = find_semantic(semantics.roots,
                                               "criteria.content.state");
    const SemanticNode* apply = find_semantic(semantics.roots, "criteria.apply");
    require(group && group->role == SemanticRole::group &&
                group->value == "3 modules" && staged && state && apply &&
                state->name == "Application state" &&
                state->description.find("staged") != std::string::npos,
            "rack/module/status/action semantics must expose staged meaning without relying on color");
}

void test_field_toggle_validation_move_and_focus_safe_remove() {
    auto rack = make_rack();
    Window window(rack, {900.0, 110.0});
    window.perform_layout();
    std::string trace;
    auto toggled = rack->module_toggled().subscribe(
        [&trace](const InstrumentModuleToggle& change) {
            trace += change.module_id + (change.enabled ? "=on;" : "=off;");
        });
    auto committed = rack->field_committed().subscribe(
        [&trace](const InstrumentFieldChange& change) {
            trace += change.module_id + "." + change.field_id + "=" +
                     change.current_value + ";";
        });
    require(window.perform_semantic_action("criteria.kind.enable",
                                           SemanticAction::press) &&
                trace == "criteria.kind=off;",
            "module enable semantics must emit one typed toggle transition");

    const auto kind_value = std::dynamic_pointer_cast<ComboBox>(
        rack->field_editor("criteria.kind", "value"));
    kind_value->set_selected_index(1U);
    require(trace.ends_with("criteria.kind.value=Documents;"),
            "choice fields must commit through the rack's typed field event");

    const auto content_value = std::dynamic_pointer_cast<TextBox>(
        rack->field_editor("criteria.content", "value"));
    require(window.request_focus(content_value),
            "text criterion must accept keyboard focus");
    content_value->set_text("");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::enter}) &&
                rack->modules()[2].state == InstrumentModuleState::invalid &&
                rack->modules()[2].fields[2].validation_message ==
                    "Value is required",
            "required text commit must publish persistent non-color validation");

    const Control::Ptr retained_kind = rack->field_editor("criteria.kind", "property");
    require(window.perform_semantic_action("criteria.kind.enable",
                                           SemanticAction::press),
            "reorder setup must re-enable the live module");
    auto moved = rack->move_requested().subscribe(
        [&rack](const InstrumentModuleMoveRequest& request) {
            auto modules = rack->modules();
            InstrumentModuleSpec moving = modules[request.previous_index];
            modules.erase(modules.begin() +
                          static_cast<std::ptrdiff_t>(request.previous_index));
            modules.insert(modules.begin() +
                           static_cast<std::ptrdiff_t>(request.requested_index),
                           std::move(moving));
            rack->set_modules(std::move(modules));
        });
    require(window.request_focus(retained_kind) &&
                window.dispatch_key({KeyAction::down, PhysicalKey::right,
                                     Modifier::alt}) &&
                rack->modules()[1].stable_id == "criteria.kind" &&
                rack->field_editor("criteria.kind", "property") == retained_kind,
            "Alt+Right must request a stable-identity reorder without recreating the editor");

    auto removed = rack->remove_requested().subscribe(
        [&rack](const InstrumentModuleRequest& request) {
            auto modules = rack->modules();
            std::erase_if(modules, [&](const InstrumentModuleSpec& module) {
                return module.stable_id == request.module_id;
            });
            rack->set_modules(std::move(modules));
        });
    const Control::Ptr remove_button = window.find("criteria.kind.remove");
    require(window.request_focus(remove_button) &&
                window.perform_semantic_action("criteria.kind.remove",
                                               SemanticAction::press) &&
                rack->modules().size() == 2U &&
                window.focused_control() &&
                window.focused_control()->stable_id().value() ==
                    "criteria.content.enable",
            "removing the focused module must transfer focus to the next retained module");
}

void test_responsive_wrap_compact_stack_and_scroll() {
    auto rack = make_rack();
    Window window(rack, {520.0, 150.0});
    window.perform_layout();
    const auto first = rack->module_bounds("criteria.kind");
    const auto third = rack->module_bounds("criteria.content");
    require(first && third && third->y > first->y &&
                rack->content_height() > 66.0,
            "narrow rack must wrap retained modules by authored order");

    window.resize({170.0, 90.0});
    window.perform_layout();
    const auto compact = rack->module_bounds("criteria.kind");
    const auto property = rack->field_editor("criteria.kind", "property");
    const auto value = rack->field_editor("criteria.kind", "value");
    require(compact && compact->width == 170.0 &&
                value->absolute_bounds().y > property->absolute_bounds().y &&
                rack->content_height() > 300.0,
            "severe width must stack fields inside each instrument instead of crushing three columns");
    rack->set_scroll_offset(10000.0);
    require(rack->scroll_offset() > 0.0 &&
                rack->scroll_offset() <= rack->content_height() - 90.0,
            "the rack's one bounded scroll plane must keep wrapped modules reachable");

    window.set_text_scale(2.25);
    window.resize({520.0, 180.0});
    window.perform_layout();
    require(rack->content_height() > 300.0,
            "large text must remeasure and wrap modules instead of clipping field controls");
}

void test_model_validation() {
    auto rack = make_control<InstrumentRack>(StableId("criteria.invalid"));
    auto duplicate = default_modules();
    duplicate[1].stable_id = duplicate[0].stable_id;
    bool duplicate_rejected{};
    bool empty_choice_rejected{};
    try {
        rack->set_modules(duplicate);
    } catch (const std::invalid_argument&) {
        duplicate_rejected = true;
    }
    auto empty_choice = default_modules();
    empty_choice[0].fields[0].choices.clear();
    try {
        rack->set_modules(empty_choice);
    } catch (const std::invalid_argument&) {
        empty_choice_rejected = true;
    }
    require(duplicate_rejected && empty_choice_rejected,
            "rack models must reject duplicate identities and unusable choice fields");
}

} // namespace

int main() {
    try {
        test_real_fields_stable_reconciliation_and_semantics();
        test_field_toggle_validation_move_and_focus_safe_remove();
        test_responsive_wrap_compact_stack_and_scroll();
        test_model_validation();
        std::cout << "gui_forms_instrument_controls_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_instrument_controls_tests: " << error.what()
                  << '\n';
        return EXIT_FAILURE;
    }
}
