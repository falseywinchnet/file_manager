#include "gui_forms/basic_controls.hpp"
#include "gui_forms/input_controls.hpp"
#include "gui_forms/range_controls.hpp"
#include "gui_forms/window.hpp"
#include "support/named_callbacks.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

const SemanticNode* find_node(const std::vector<SemanticNode>& nodes,
                              std::string_view stable_id) {
    for (const SemanticNode& node : nodes) {
        if (node.stable_id == stable_id) return &node;
        if (const SemanticNode* found = find_node(node.children, stable_id)) return found;
    }
    return nullptr;
}

bool has_action(const SemanticNode& node, SemanticAction action) {
    return std::find(node.actions.begin(), node.actions.end(), action) !=
           node.actions.end();
}

class ExerciseSemanticsOffThread final {
public:
    ExerciseSemanticsOffThread(Window& window, bool& snapshot_rejected,
                               bool& action_rejected)
        : window_(window), snapshot_rejected_(snapshot_rejected),
          action_rejected_(action_rejected) {}

    void operator()() const {
        try {
            static_cast<void>(window_.semantic_snapshot());
        } catch (const std::logic_error&) {
            snapshot_rejected_ = true;
        }
        try {
            static_cast<void>(window_.perform_semantic_action(
                "thread.button", SemanticAction::press));
        } catch (const std::logic_error&) {
            action_rejected_ = true;
        }
    }

private:
    Window& window_;
    bool& snapshot_rejected_;
    bool& action_rejected_;
};

void test_snapshot_roles_states_hierarchy_and_json() {
    std::shared_ptr<gui_forms::Panel> root = make_control<Panel>(StableId("semantics.root"));
    std::shared_ptr<gui_forms::GroupBox> group = make_control<GroupBox>(StableId("semantics.group"), "Receiver");
    (*group).set_requested_bounds({10.0, 10.0, 380.0, 220.0});
    std::shared_ptr<gui_forms::Label> label = make_control<Label>(StableId("semantics.label"), "Frequency");
    (*label).set_requested_bounds({12.0, 24.0, 100.0, 24.0});
    std::shared_ptr<gui_forms::TextBox> field = make_control<TextBox>(StableId("semantics.field"), "101.500");
    (*field).set_accessible_name("Frequency in MHz");
    (*field).set_requested_bounds({112.0, 24.0, 150.0, 28.0});
    std::shared_ptr<gui_forms::CheckBox> check = make_control<CheckBox>(StableId("semantics.check"), "Noise reduction");
    (*check).set_checked(true);
    (*check).set_requested_bounds({12.0, 62.0, 180.0, 28.0});
    std::shared_ptr<gui_forms::ProgressBar> progress = make_control<ProgressBar>(StableId("semantics.progress"));
    (*progress).set_accessible_name("Signal buffer");
    (*progress).set_value(42.0);
    (*progress).set_requested_bounds({12.0, 104.0, 220.0, 20.0});
    std::shared_ptr<gui_forms::Button> hidden = make_control<Button>(StableId("semantics.hidden"), "Hidden");
    (*hidden).set_visible(false);
    (*group).add_child(label);
    (*group).add_child(field);
    (*group).add_child(check);
    (*group).add_child(progress);
    (*group).add_child(hidden);
    (*root).add_child(group);

    Window window(root, {420.0, 260.0});
    require(window.request_focus(field), "semantic fixture must retain field focus");
    const SemanticSnapshot first = window.semantic_snapshot();
    const SemanticSnapshot second = window.semantic_snapshot();
    require(first.roots.size() == 1U && first.node_count == 5U,
            "snapshot must expose the named group and four eligible stock controls");
    require(first.to_json() == second.to_json(),
            "unchanged semantic snapshots must serialize deterministically");

    const SemanticNode* group_node = find_node(first.roots, "semantics.group");
    const SemanticNode* field_node = find_node(first.roots, "semantics.field");
    const SemanticNode* check_node = find_node(first.roots, "semantics.check");
    const SemanticNode* progress_node = find_node(first.roots, "semantics.progress");
    require(group_node && (*group_node).role == SemanticRole::group &&
                (*group_node).children.size() == 4U,
            "semantic hierarchy must preserve exposed retained parents");
    require(field_node && (*field_node).role == SemanticRole::text_box &&
                (*field_node).name == "Frequency in MHz" &&
                (*field_node).value == "101.500" &&
                has_semantic_state((*field_node).states, SemanticState::focused) &&
                has_action(*field_node, SemanticAction::set_value),
            "TextBox semantics must publish identity, value, focus and edit action");
    require(check_node && (*check_node).role == SemanticRole::check_box &&
                has_semantic_state((*check_node).states, SemanticState::checked),
            "CheckBox semantics must publish checked state");
    require(progress_node && (*progress_node).role == SemanticRole::progress_bar &&
                (*progress_node).value == "42" &&
                (*progress_node).bounds == Rect{22.0, 114.0, 220.0, 20.0},
            "ProgressBar semantics must publish value and absolute retained bounds");
    require(!find_node(first.roots, "semantics.hidden"),
            "ineligible hidden controls must not leak into semantic snapshots");
}

void test_semantic_actions_use_control_behavior() {
    std::shared_ptr<gui_forms::Panel> root = make_control<Panel>(StableId("actions.root"));
    std::shared_ptr<gui_forms::Button> button = make_control<Button>(StableId("actions.button"), "Apply");
    std::shared_ptr<gui_forms::CheckBox> check = make_control<CheckBox>(StableId("actions.check"), "Enabled");
    std::shared_ptr<gui_forms::TrackBar> slider = make_control<TrackBar>(StableId("actions.slider"));
    (*slider).set_small_change(2.5);
    (*slider).set_value(25.0);
    std::shared_ptr<gui_forms::TextBox> field = make_control<TextBox>(StableId("actions.field"), "old");
    std::shared_ptr<gui_forms::ComboBox> combo = make_control<ComboBox>(StableId("actions.combo"));
    (*combo).set_items({"Local", "Remote"});
    (*root).add_child(button);
    (*root).add_child(check);
    (*root).add_child(slider);
    (*root).add_child(field);
    (*root).add_child(combo);
    Window window(root, {500.0, 300.0});

    unsigned presses{};
    SubscriptionToken pressed = (*button).clicked().subscribe(
        test_support::IncrementCounter<unsigned, ButtonBase&>(presses));
    require(window.perform_semantic_action("actions.button", SemanticAction::press) &&
                presses == 1U,
            "semantic press must execute the Button activation contract once");
    require(window.perform_semantic_action("actions.check", SemanticAction::press) &&
                (*check).checked(),
            "semantic press must use CheckBox auto-check behavior");
    require(window.perform_semantic_action("actions.slider", SemanticAction::increment) &&
                (*slider).value() == 27.5,
            "semantic slider increment must use the public small change");
    require(window.perform_semantic_action("actions.slider", SemanticAction::set_value,
                                            "63.5") && (*slider).value() == 63.5,
            "semantic slider set-value must use bounded range behavior");
    require(window.perform_semantic_action("actions.field", SemanticAction::set_value,
                                            "new value") && (*field).text() == "new value",
            "semantic text set-value must use the TextBox mutation contract");
    (*field).set_read_only(true);
    require(!window.perform_semantic_action("actions.field", SemanticAction::set_value,
                                             "blocked") && (*field).text() == "new value",
            "semantic actions must preserve TextBox read-only state");
    require(window.perform_semantic_action("actions.combo", SemanticAction::expand) &&
                (*combo).dropped_down() && window.focus_scope_depth() == 1U,
            "semantic combo expansion must enter the real popup focus scope");
    require(window.perform_semantic_action("actions.combo.popup.list.item.1",
                                            SemanticAction::press) &&
                (*combo).selected_index() == 1U && !(*combo).dropped_down() &&
                window.focus_scope_depth() == 0U &&
                !find_node(window.semantic_snapshot().roots,
                           "actions.combo.popup.list"),
            "semantic popup row press must commit, detach, and restore focus safely");
    require(window.perform_semantic_action("actions.combo", SemanticAction::expand) &&
                (*combo).dropped_down(),
            "ComboBox must reopen after semantic row activation");
    require(window.perform_semantic_action("actions.combo", SemanticAction::collapse) &&
                !(*combo).dropped_down() && window.focus_scope_depth() == 0U,
            "semantic combo collapse must revoke the real popup scope");
    require(!find_node(window.semantic_snapshot().roots,
                       "actions.combo.popup.list"),
            "detached popup semantics must disappear in the closing generation");
}

void test_semantics_enforce_ui_thread() {
    std::shared_ptr<gui_forms::Button> button = make_control<Button>(StableId("thread.button"), "Threaded");
    Window window(button, {120.0, 30.0});
    bool snapshot_rejected{};
    bool action_rejected{};
    std::thread worker(ExerciseSemanticsOffThread(
        window, snapshot_rejected, action_rejected));
    worker.join();
    require(snapshot_rejected && action_rejected,
            "semantic reads and actions must reject non-owner threads");
}

void test_virtual_list_items_are_selectable() {
    std::shared_ptr<gui_forms::ListBox> list = make_control<ListBox>(StableId("virtual.list"));
    (*list).set_accessible_name("Saved locations");
    (*list).set_items({"Desktop", "Documents", "Downloads"});
    (*list).set_requested_bounds({10.0, 20.0, 180.0, 82.0});
    Window window(list, {220.0, 130.0});
    const SemanticSnapshot before = window.semantic_snapshot();
    const SemanticNode* list_node = find_node(before.roots, "virtual.list");
    const SemanticNode* item = find_node(before.roots, "virtual.list.item.1");
    require(list_node && (*list_node).children.size() == 3U && item &&
                (*item).role == SemanticRole::list_item && (*item).name == "Documents" &&
                has_action(*item, SemanticAction::select),
            "painted ListBox rows must publish selectable virtual semantic children");
    require(window.perform_semantic_action("virtual.list.item.1",
                                            SemanticAction::select) &&
                (*list).selected_index() == 1U && window.focused_control() == list,
            "virtual list-item selection must route through retained ListBox behavior");
    const SemanticSnapshot after = window.semantic_snapshot();
    const SemanticNode* selected = find_node(after.roots, "virtual.list.item.1");
    require(selected && has_semantic_state((*selected).states, SemanticState::selected) &&
                has_semantic_state((*selected).states, SemanticState::focused),
            "selected virtual list item must publish selection and active focus state");
    std::size_t activated = 99U;
    SubscriptionToken activation = (*list).item_activated().subscribe(
        test_support::RecordValue<std::size_t>(activated));
    require(window.perform_semantic_action("virtual.list.item.2",
                                            SemanticAction::press) &&
                (*list).selected_index() == 2U && activated == 2U,
            "virtual list-item press must complete selection then activation");
}

} // namespace

int main() {
    try {
        test_snapshot_roles_states_hierarchy_and_json();
        test_semantic_actions_use_control_behavior();
        test_virtual_list_items_are_selectable();
        test_semantics_enforce_ui_thread();
        std::cout << "gui_forms_semantic_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_semantic_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
