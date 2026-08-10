#include "gui_forms/gui_forms.hpp"

#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace {

using namespace gui_forms;
using namespace std::chrono_literals;

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

const SemanticNode* find_role(const std::vector<SemanticNode>& nodes,
                              SemanticRole role) {
    for (const auto& node : nodes) {
        if (node.role == role) return &node;
        if (const SemanticNode* found = find_role(node.children, role)) return found;
    }
    return nullptr;
}

std::size_t count_role(const std::vector<SemanticNode>& nodes,
                       SemanticRole role) {
    std::size_t count = 0U;
    for (const auto& node : nodes) {
        count += node.role == role ? 1U : 0U;
        count += count_role(node.children, role);
    }
    return count;
}

struct Fixture final {
    Fixture() : window(root, {420.0, 240.0}) {
        root->set_requested_bounds({0.0, 0.0, 420.0, 240.0});
        button->set_requested_bounds({40.0, 45.0, 130.0, 34.0});
        other->set_requested_bounds({220.0, 45.0, 130.0, 34.0});
        root->add_child(button);
        root->add_child(other);
        window.perform_layout();
        static_cast<void>(window.take_damage());
    }

    Control::Ptr root = make_control<Control>(StableId("tooltip.root"));
    std::shared_ptr<Button> button =
        make_control<Button>(StableId("tooltip.target"), "Hover target");
    std::shared_ptr<Button> other =
        make_control<Button>(StableId("tooltip.other"), "Other target");
    Window window;
};

void hover(Fixture& fixture, Point point) {
    static_cast<void>(fixture.window.dispatch_pointer(
        {PointerAction::move, PointerButton::none, point}));
}

void fire_next(Window& window) {
    const auto wake = window.next_wake();
    require(wake.has_value(), "test expected a scheduled UI deadline");
    static_cast<void>(window.poll_frame_schedule(*wake));
}

void test_hover_delay_cancel_and_accessible_overlay() {
    Fixture fixture;
    ToolTip tips(fixture.window);
    tips.set_initial_delay(30ms);
    tips.set_auto_pop_delay(200ms);
    tips.set_tool_tip(fixture.button, "Opens the calibrated input selector");
    int shown = 0;
    int hidden = 0;
    auto changes = tips.visibility_changed().subscribe(
        [&](const ToolTipEvent& event) { event.shown ? ++shown : ++hidden; });

    hover(fixture, {60.0, 60.0});
    require(!tips.visible() && fixture.window.next_wake().has_value(),
            "hover must arm the initial delay without opening synchronously");
    hover(fixture, {390.0, 210.0});
    require(!tips.visible() && !fixture.window.next_wake().has_value(),
            "leaving before the delay must cancel the pending popup and wake");

    hover(fixture, {60.0, 60.0});
    fire_next(fixture.window);
    fixture.window.perform_layout();
    const SemanticSnapshot snapshot = fixture.window.semantic_snapshot();
    const SemanticNode* tooltip = find_role(snapshot.roots, SemanticRole::tool_tip);
    require(tips.visible() && shown == 1 && hidden == 0 && tooltip &&
                tooltip->name == "Opens the calibrated input selector" &&
                fixture.window.hit_test({65.0, 60.0}) == fixture.button,
            "a due hover must open a named semantic, input-transparent overlay");

    hover(fixture, {390.0, 210.0});
    require(!tips.visible() && shown == 1 && hidden == 1 &&
                !find_role(fixture.window.semantic_snapshot().roots,
                           SemanticRole::tool_tip),
            "leave must remove both the retained popup and its semantic node");
}

void test_focus_policy_autopop_and_moving_target() {
    Fixture fixture;
    ToolTip tips(fixture.window);
    tips.set_initial_delay(1ms);
    tips.set_auto_pop_delay(40ms);
    tips.set_tool_tip(fixture.button, "Keyboard help");
    require(fixture.window.request_focus(fixture.button),
            "fixture button must accept focus");
    fire_next(fixture.window);
    fixture.window.perform_layout();
    const SemanticSnapshot before_snapshot = fixture.window.semantic_snapshot();
    const SemanticNode* before =
        find_role(before_snapshot.roots, SemanticRole::tool_tip);
    require(before && tips.visible(), "keyboard focus must follow the same delayed policy");
    const Rect old_bounds = before->bounds;

    fixture.button->set_requested_bounds({250.0, 145.0, 130.0, 34.0});
    fixture.window.perform_layout();
    const SemanticSnapshot after_snapshot = fixture.window.semantic_snapshot();
    const SemanticNode* after =
        find_role(after_snapshot.roots, SemanticRole::tool_tip);
    require(after && after->bounds != old_bounds && after->bounds.x >= 250.0,
            "a visible tooltip must remain anchored when its target moves");

    fire_next(fixture.window);
    require(!tips.visible(), "auto-pop deadline must close a focused tooltip");
}

void test_repeated_focus_show_hide_reuses_no_stale_overlay_identity() {
    Fixture fixture;
    ToolTip tips(fixture.window);
    tips.set_initial_delay(1ms);
    tips.set_reshow_delay(1ms);
    tips.set_auto_pop_delay(5s);
    tips.set_tool_tip(fixture.button, "Repeated keyboard help");
    for (std::size_t cycle = 0U; cycle < 32U; ++cycle) {
        require(fixture.window.request_focus(fixture.button),
                "repeated tooltip target must accept focus");
        fire_next(fixture.window);
        fixture.window.perform_layout();
        const std::size_t shown = count_role(
            fixture.window.semantic_snapshot().roots, SemanticRole::tool_tip);
        if (!tips.visible() || shown != 1U) {
            throw std::runtime_error(
                "tooltip focus cycle " + std::to_string(cycle) +
                " opened visible=" + (tips.visible() ? "true" : "false") +
                " count=" + std::to_string(shown) + " frame_faults=" +
                std::to_string(
                    fixture.window.metrics_snapshot().frame_callback_faults));
        }
        require(fixture.window.request_focus(fixture.other),
                "focus must leave the repeated tooltip target");
        require(!tips.visible() &&
                    count_role(fixture.window.semantic_snapshot().roots,
                               SemanticRole::tool_tip) == 0U,
                "focus departure must detach the tooltip subtree and identity");
    }
}

void test_explicit_show_multiple_providers_and_owner_disposal() {
    Fixture fixture;
    ToolTip first(fixture.window);
    ToolTip second(fixture.window);
    first.set_tool_tip(fixture.button, "First provider");
    second.set_tool_tip(fixture.button, "Second provider");
    first.show(fixture.button, 0ms);
    second.show(fixture.button, 0ms);
    fixture.window.perform_layout();
    require(first.visible() && second.visible(),
            "independent providers must be able to own independent overlays");
    const SemanticSnapshot snapshot = fixture.window.semantic_snapshot();
    require(count_role(snapshot.roots, SemanticRole::tool_tip) == 2U,
            "both provider overlays must remain in the retained semantic tree");

    fixture.button->dispose();
    require(!first.visible() && !second.visible() && !first.active_control() &&
                !second.active_control(),
            "disposing a target must synchronously revoke every provider popup");
    require(!find_role(fixture.window.semantic_snapshot().roots,
                       SemanticRole::tool_tip),
            "owner disposal must leave no stale tooltip semantic nodes");
}

void test_mapping_removal_and_provider_disposal_are_quiescent() {
    Fixture fixture;
    auto tips = std::make_unique<ToolTip>(fixture.window);
    tips->set_initial_delay(20ms);
    tips->set_tool_tip(fixture.button, "Disposable mapping");
    hover(fixture, {60.0, 60.0});
    require(tips->remove_tool_tip(*fixture.button) && !fixture.window.next_wake(),
            "removing a pending mapping must revoke its timer immediately");
    tips->set_tool_tip(fixture.button, "Disposable provider");
    tips->show(fixture.button, 0ms);
    require(tips->visible(), "explicit persistent tooltip must open");
    fixture.window.perform_layout();
    const SemanticSnapshot explicit_snapshot = fixture.window.semantic_snapshot();
    const SemanticNode* explicit_tip =
        find_role(explicit_snapshot.roots, SemanticRole::tool_tip);
    require(explicit_tip && explicit_tip->bounds.x >= fixture.button->absolute_bounds().x,
            "programmatic ToolTip show must anchor to its target, not the client origin");
    tips->dispose();
    require(!fixture.window.next_wake() &&
                !find_role(fixture.window.semantic_snapshot().roots,
                           SemanticRole::tool_tip),
            "provider disposal must synchronously remove popup and scheduled work");
}

void test_show_always_supports_a_disabled_visible_owner() {
    Fixture fixture;
    ToolTip tips(fixture.window);
    tips.set_show_always(true);
    fixture.button->set_enabled(false);
    tips.set_tool_tip(fixture.button, "Why this command is unavailable");
    tips.show(fixture.button, 0ms);
    fixture.window.perform_layout();
    const SemanticSnapshot snapshot = fixture.window.semantic_snapshot();
    const SemanticNode* tip = find_role(snapshot.roots, SemanticRole::tool_tip);
    require(tips.visible() && tip && tip->name == "Why this command is unavailable",
            "ShowAlways must allow passive help owned by a disabled visible control");
}

void test_maximum_width_is_bounded_and_shapes_overlay() {
    Fixture fixture;
    ToolTip tips(fixture.window);
    tips.set_maximum_width(120.0);
    require(tips.maximum_width() == 120.0,
            "ToolTip must retain an explicit bounded maximum width");
    bool rejected = false;
    try {
        tips.set_maximum_width(40.0);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected, "ToolTip must reject unusably narrow maximum widths");
    tips.set_tool_tip(
        fixture.button,
        "A deliberately long tooltip sentence that must wrap into multiple lines");
    tips.show(fixture.button, 0ms);
    fixture.window.perform_layout();
    const SemanticNode* tip = find_role(
        fixture.window.semantic_snapshot().roots, SemanticRole::tool_tip);
    require(tip && tip->bounds.width <= 140.0,
            "ToolTip bubble must honor maximum text width plus chrome");
}

} // namespace

int main() {
    try {
        test_hover_delay_cancel_and_accessible_overlay();
        test_focus_policy_autopop_and_moving_target();
        test_repeated_focus_show_hide_reuses_no_stale_overlay_identity();
        test_explicit_show_multiple_providers_and_owner_disposal();
        test_mapping_removal_and_provider_disposal_are_quiescent();
        test_show_always_supports_a_disabled_visible_owner();
        test_maximum_width_is_bounded_and_shapes_overlay();
        std::cout << "tooltip tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "tooltip tests failed: " << error.what() << '\n';
        return 1;
    }
}
