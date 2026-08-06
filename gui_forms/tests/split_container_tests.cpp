#include "gui_forms/basic_controls.hpp"
#include "gui_forms/container_controls.hpp"
#include "gui_forms/window.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

bool near(double left, double right) {
    return std::abs(left - right) < 0.001;
}

void test_stable_tree_and_constrained_geometry() {
    auto split = make_control<SplitContainer>(StableId("split.shell"));
    split->set_splitter_width(3.0);
    split->set_splitter_hit_width(9.0);
    split->set_first_minimum(80.0);
    split->set_second_minimum(120.0);
    split->set_splitter_distance(140.0);
    Window window(split, {500.0, 240.0});
    window.perform_layout();

    require(split->first_panel()->stable_id().value() == "split.shell.panel1" &&
                split->second_panel()->stable_id().value() == "split.shell.panel2" &&
                split->splitter_control()->stable_id().value() ==
                    "split.shell.splitter",
            "split children require durable owner-derived identities");
    require(near(split->first_panel()->arranged_bounds().width, 140.0) &&
                near(split->second_panel()->arranged_bounds().x, 143.0) &&
                near(split->second_panel()->arranged_bounds().width, 357.0),
            "vertical split geometry must allocate two panels around the visible seam");
    require(near(split->splitter_control()->arranged_bounds().width, 9.0),
            "splitter hit geometry must be wider than its three-pixel paint geometry");

    split->set_splitter_distance(490.0);
    window.perform_layout();
    require(near(split->splitter_distance(), 377.0),
            "second-panel minimum must clamp an excessive splitter distance");

    split->set_first_minimum(400.0);
    split->set_second_minimum(400.0);
    window.resize({203.0, 100.0});
    window.perform_layout();
    require(near(split->splitter_distance(), 100.0) &&
                near(split->second_panel()->arranged_bounds().width, 100.0),
            "impossible minimums must resolve by a deterministic proportional compromise");
}

void test_live_pointer_and_keyboard_resize() {
    auto split = make_control<SplitContainer>(StableId("split.input"));
    split->set_splitter_distance(120.0);
    split->set_keyboard_increment(4.0);
    Window window(split, {400.0, 180.0});
    window.perform_layout();

    std::size_t pointer_changes = 0;
    std::size_t keyboard_changes = 0;
    auto changed = split->splitter_changed().subscribe(
        [&](const SplitChangeEvent& event) {
            if (event.reason == SplitChangeReason::pointer) ++pointer_changes;
            if (event.reason == SplitChangeReason::keyboard) ++keyboard_changes;
        });

    PointerEvent down;
    down.action = PointerAction::down;
    down.button = PointerButton::primary;
    down.position = {121.0, 70.0};
    down.pointer_id = 9;
    require(window.dispatch_pointer(down) &&
                window.captured_control() == split->splitter_control(),
            "splitter press must enter retained pointer capture");

    PointerEvent move = down;
    move.action = PointerAction::move;
    move.button = PointerButton::none;
    move.position = {181.0, 70.0};
    require(window.dispatch_pointer(move) && near(split->splitter_distance(), 180.0),
            "splitter must update continuously during pointer movement");
    window.perform_layout();
    require(near(split->first_panel()->arranged_bounds().width, 180.0) &&
                near(split->second_panel()->arranged_bounds().x, 183.0) &&
                near(split->second_panel()->arranged_bounds().width, 217.0),
            "pointer movement must commit both pane allocations before release");

    PointerEvent up = move;
    up.action = PointerAction::up;
    up.button = PointerButton::primary;
    require(window.dispatch_pointer(up) && !window.captured_control() &&
                pointer_changes >= 1,
            "splitter release must end capture without postponing the value update");

    require(window.focused_control() == split->splitter_control(),
            "pointer resizing must leave the physical splitter keyboard-focusable");
    KeyEvent right;
    right.action = KeyAction::down;
    right.physical_key = PhysicalKey::right;
    right.modifiers = Modifier::shift;
    require(window.dispatch_key(right) && near(split->splitter_distance(), 220.0) &&
                keyboard_changes == 1,
            "Shift+Arrow must apply the declared larger keyboard increment");

    split->set_splitter_fixed(true);
    const double fixed = split->splitter_distance();
    require(!window.dispatch_key(right) && near(split->splitter_distance(), fixed),
            "fixed splitters must reject keyboard mutation");
}

void test_pane_surface_background() {
    auto split = make_control<SplitContainer>(StableId("split.surface"));
    const Color first = Color::rgba(225, 235, 243);
    const Color second = Color::rgba(250, 250, 250);
    split->first_panel()->set_background(first);
    split->second_panel()->set_background(second);
    require(split->first_panel()->background() == first &&
                split->second_panel()->background() == second,
            "splitter panels must own their allocation-filling surface color");
}

void test_collapse_focus_restore_and_fixed_panel_resize() {
    auto split = make_control<SplitContainer>(StableId("split.collapse"));
    split->set_splitter_distance(110.0);
    auto field = make_control<Button>(StableId("split.collapse.field"), "Field");
    field->set_requested_bounds({8.0, 8.0, 80.0, 24.0});
    split->first_panel()->add_child(field);
    Window window(split, {400.0, 180.0});
    window.perform_layout();
    require(window.request_focus(field), "first panel fixture must accept focus");

    split->set_first_collapsed(true);
    window.perform_layout();
    require(!split->first_panel()->visible() &&
                window.focused_control() == split->splitter_control() &&
                near(split->second_panel()->arranged_bounds().width, 397.0),
            "collapsing a pane must remove it from input and transfer focus to its seam");

    bool rejected_both = false;
    try {
        split->set_second_collapsed(true);
    } catch (const std::logic_error&) {
        rejected_both = true;
    }
    require(rejected_both, "a split composition may not collapse both panels");

    split->set_first_collapsed(false);
    window.perform_layout();
    require(split->first_panel()->visible() && near(split->splitter_distance(), 110.0),
            "restoring a panel must recover its remembered extent");

    split->set_fixed_panel(SplitFixedPanel::second);
    const double second_extent = split->second_panel()->arranged_bounds().width;
    window.resize({520.0, 180.0});
    window.perform_layout();
    require(near(split->second_panel()->arranged_bounds().width, second_extent),
            "a fixed second panel must retain its extent when the container grows");
}

void test_horizontal_orientation_and_thread_guard() {
    auto split = make_control<SplitContainer>(StableId("split.horizontal"));
    split->set_orientation(Orientation::horizontal);
    split->set_splitter_distance(70.0);
    Window window(split, {260.0, 200.0});
    window.perform_layout();
    require(near(split->first_panel()->arranged_bounds().height, 70.0) &&
                near(split->second_panel()->arranged_bounds().y, 73.0) &&
                split->splitter_control()->effective_cursor() ==
                    CursorKind::resize_vertical,
            "horizontal orientation must divide height and expose a vertical-resize cursor");

    bool rejected = false;
    std::thread worker([&] {
        try {
            split->set_splitter_distance(90.0);
        } catch (const std::logic_error&) {
            rejected = true;
        }
    });
    worker.join();
    require(rejected, "attached split mutation must preserve UI-thread enforcement");
}

} // namespace

int main() {
    try {
        test_stable_tree_and_constrained_geometry();
        test_live_pointer_and_keyboard_resize();
        test_pane_surface_background();
        test_collapse_focus_restore_and_fixed_panel_resize();
        test_horizontal_orientation_and_thread_guard();
        std::cout << "split-container-tests: pass\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "split-container-tests: fail: " << error.what() << '\n';
        return 1;
    }
}
