#include "gui_forms/basic_controls.hpp"
#include "gui_forms/container_controls.hpp"
#include "gui_forms/window.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

bool near(double left, double right) {
    return std::abs(left - right) < 0.001;
}

class GripRecordingPainter final : public Painter {
public:
    void save() override {}
    void restore() override {}
    void translate(Point) override {}
    void clip_rect(Rect) override {}
    void fill_rect(Rect bounds, Color) override { fills.push_back(bounds); }
    void stroke_rect(Rect bounds, Color, double) override {
        strokes.push_back(bounds);
    }
    void draw_box_shadow(Rect bounds, double, Point, double, double,
                         Color) override {
        shadows.push_back(bounds);
    }
    void draw_line(Point, Point, Color, double) override { ++lines; }
    void draw_text_utf8(Point, std::string_view text, FontSpec,
                        Color) override {
        texts.emplace_back(text);
    }
    void draw_image(ImageId, Rect, double) override {}

    std::vector<Rect> fills;
    std::vector<Rect> strokes;
    std::vector<Rect> shadows;
    std::vector<std::string> texts;
    std::size_t lines{};
};

class CountSplitReasons final {
public:
    CountSplitReasons(std::size_t& pointer_changes,
                      std::size_t& keyboard_changes)
        : pointer_changes_(pointer_changes),
          keyboard_changes_(keyboard_changes) {}

    void operator()(const SplitChangeEvent& event) const {
        if (event.reason == SplitChangeReason::pointer) ++pointer_changes_;
        if (event.reason == SplitChangeReason::keyboard) ++keyboard_changes_;
    }

private:
    std::size_t& pointer_changes_;
    std::size_t& keyboard_changes_;
};

class SetSplitterOffThread final {
public:
    SetSplitterOffThread(SplitContainer& split, bool& rejected)
        : split_(split), rejected_(rejected) {}

    void operator()() const {
        try {
            split_.set_splitter_distance(90.0);
        } catch (const std::logic_error&) {
            rejected_ = true;
        }
    }

private:
    SplitContainer& split_;
    bool& rejected_;
};

void test_stable_tree_and_constrained_geometry() {
    std::shared_ptr<gui_forms::SplitContainer> split = make_control<SplitContainer>(StableId("split.shell"));
    (*split).set_splitter_width(3.0);
    (*split).set_splitter_hit_width(9.0);
    (*split).set_first_minimum(80.0);
    (*split).set_second_minimum(120.0);
    (*split).set_splitter_distance(140.0);
    Window window(split, {500.0, 240.0});
    window.perform_layout();

    require((*(*split).first_panel()).stable_id().value() == "split.shell.panel1" &&
                (*(*split).second_panel()).stable_id().value() == "split.shell.panel2" &&
                (*(*split).splitter_control()).stable_id().value() ==
                    "split.shell.splitter",
            "split children require durable owner-derived identities");
    require(near((*(*split).first_panel()).arranged_bounds().width, 140.0) &&
                near((*(*split).second_panel()).arranged_bounds().x, 143.0) &&
                near((*(*split).second_panel()).arranged_bounds().width, 357.0),
            "vertical split geometry must allocate two panels around the visible seam");
    require(near((*(*split).splitter_control()).arranged_bounds().width, 9.0),
            "splitter hit geometry must be wider than its three-pixel paint geometry");

    (*split).set_splitter_distance(490.0);
    window.perform_layout();
    require(near((*split).splitter_distance(), 377.0),
            "second-panel minimum must clamp an excessive splitter distance");

    (*split).set_first_minimum(400.0);
    (*split).set_second_minimum(400.0);
    window.resize({203.0, 100.0});
    window.perform_layout();
    require(near((*split).splitter_distance(), 100.0) &&
                near((*(*split).second_panel()).arranged_bounds().width, 100.0),
            "impossible minimums must resolve by a deterministic proportional compromise");
}

void test_live_pointer_and_keyboard_resize() {
    std::shared_ptr<gui_forms::SplitContainer> split = make_control<SplitContainer>(StableId("split.input"));
    (*split).set_splitter_distance(120.0);
    (*split).set_keyboard_increment(4.0);
    Window window(split, {400.0, 180.0});
    window.perform_layout();

    std::size_t pointer_changes = 0;
    std::size_t keyboard_changes = 0;
    SubscriptionToken changed = (*split).splitter_changed().subscribe(
        CountSplitReasons(pointer_changes, keyboard_changes));

    PointerEvent down;
    down.action = PointerAction::down;
    down.button = PointerButton::primary;
    down.position = {121.0, 70.0};
    down.pointer_id = 9;
    require(window.dispatch_pointer(down) &&
                window.captured_control() == (*split).splitter_control(),
            "splitter press must enter retained pointer capture");

    PointerEvent move = down;
    move.action = PointerAction::move;
    move.button = PointerButton::none;
    move.position = {181.0, 70.0};
    require(window.dispatch_pointer(move) && near((*split).splitter_distance(), 180.0),
            "splitter must update continuously during pointer movement");
    window.perform_layout();
    require(near((*(*split).first_panel()).arranged_bounds().width, 180.0) &&
                near((*(*split).second_panel()).arranged_bounds().x, 183.0) &&
                near((*(*split).second_panel()).arranged_bounds().width, 217.0),
            "pointer movement must commit both pane allocations before release");

    PointerEvent up = move;
    up.action = PointerAction::up;
    up.button = PointerButton::primary;
    require(window.dispatch_pointer(up) && !window.captured_control() &&
                pointer_changes >= 1,
            "splitter release must end capture without postponing the value update");

    require(window.focused_control() == (*split).splitter_control(),
            "pointer resizing must leave the physical splitter keyboard-focusable");
    KeyEvent right;
    right.action = KeyAction::down;
    right.physical_key = PhysicalKey::right;
    right.modifiers = Modifier::shift;
    require(window.dispatch_key(right) && near((*split).splitter_distance(), 220.0) &&
                keyboard_changes == 1,
            "Shift+Arrow must apply the declared larger keyboard increment");

    (*split).set_splitter_fixed(true);
    const double fixed = (*split).splitter_distance();
    require(!window.dispatch_key(right) && near((*split).splitter_distance(), fixed),
            "fixed splitters must reject keyboard mutation");
}

void test_pane_surface_background() {
    std::shared_ptr<gui_forms::SplitContainer> split = make_control<SplitContainer>(StableId("split.surface"));
    const Color first = Color::rgba(225, 235, 243);
    const Color second = Color::rgba(250, 250, 250);
    (*(*split).first_panel()).set_background(first);
    (*(*split).second_panel()).set_background(second);
    require((*(*split).first_panel()).background() == first &&
                (*(*split).second_panel()).background() == second,
            "splitter panels must own their allocation-filling surface color");
}

void test_collapse_focus_restore_and_fixed_panel_resize() {
    std::shared_ptr<gui_forms::SplitContainer> split = make_control<SplitContainer>(StableId("split.collapse"));
    (*split).set_splitter_distance(110.0);
    std::shared_ptr<gui_forms::Button> field = make_control<Button>(StableId("split.collapse.field"), "Field");
    (*field).set_requested_bounds({8.0, 8.0, 80.0, 24.0});
    (*(*split).first_panel()).add_child(field);
    Window window(split, {400.0, 180.0});
    window.perform_layout();
    require(window.request_focus(field), "first panel fixture must accept focus");

    (*split).set_first_collapsed(true);
    window.perform_layout();
    require(!(*(*split).first_panel()).visible() &&
                window.focused_control() == (*split).splitter_control() &&
                near((*(*split).second_panel()).arranged_bounds().width, 397.0),
            "collapsing a pane must remove it from input and transfer focus to its seam");

    bool rejected_both = false;
    try {
        (*split).set_second_collapsed(true);
    } catch (const std::logic_error&) {
        rejected_both = true;
    }
    require(rejected_both, "a split composition may not collapse both panels");

    (*split).set_first_collapsed(false);
    window.perform_layout();
    require((*(*split).first_panel()).visible() && near((*split).splitter_distance(), 110.0),
            "restoring a panel must recover its remembered extent");

    (*split).set_fixed_panel(SplitFixedPanel::second);
    const double second_extent = (*(*split).second_panel()).arranged_bounds().width;
    window.resize({520.0, 180.0});
    window.perform_layout();
    require(near((*(*split).second_panel()).arranged_bounds().width, second_extent),
            "a fixed second panel must retain its extent when the container grows");
}

void test_horizontal_orientation_and_thread_guard() {
    std::shared_ptr<gui_forms::SplitContainer> split = make_control<SplitContainer>(StableId("split.horizontal"));
    (*split).set_orientation(Orientation::horizontal);
    (*split).set_splitter_distance(70.0);
    Window window(split, {260.0, 200.0});
    window.perform_layout();
    require(near((*(*split).first_panel()).arranged_bounds().height, 70.0) &&
                near((*(*split).second_panel()).arranged_bounds().y, 73.0) &&
                (*(*split).splitter_control()).effective_cursor() ==
                    CursorKind::resize_vertical,
            "horizontal orientation must divide height and expose a vertical-resize cursor");

    bool rejected = false;
    std::thread worker(SetSplitterOffThread(*split, rejected));
    worker.join();
    require(rejected, "attached split mutation must preserve UI-thread enforcement");
}

void test_seam_tab_pointer_keyboard_and_semantic_collapse() {
    std::shared_ptr<gui_forms::SplitContainer> split = make_control<SplitContainer>(StableId("split.seam-tab"));
    (*split).set_splitter_distance(270.0);
    (*split).set_collapse_panel(SplitFixedPanel::second);
    (*split).set_accessible_name("Inspector split");
    Window window(split, {400.0, 180.0});
    window.perform_layout();
    const double remembered = (*(*split).second_panel()).arranged_bounds().width;

    const Rect seam = (*(*split).splitter_control()).absolute_bounds();
    PointerEvent down;
    down.action = PointerAction::down;
    down.button = PointerButton::primary;
    down.position = {seam.x + seam.width * .5,
                     seam.y + seam.height * .5};
    require(window.dispatch_pointer(down) &&
                window.captured_control() == (*split).splitter_control(),
            "seam collapse tab must capture its compact activation target");
    PointerEvent up = down;
    up.action = PointerAction::up;
    require(window.dispatch_pointer(up) && (*split).second_collapsed(),
            "seam collapse tab activation must collapse its declared panel");
    window.perform_layout();
    require(!(*(*split).second_panel()).visible() &&
                window.perform_semantic_action("split.seam-tab",
                                               SemanticAction::expand),
            "collapsed seam must leave the pane out of task order and expose restore");
    window.perform_layout();
    require(!(*split).second_collapsed() &&
                near((*(*split).second_panel()).arranged_bounds().width, remembered),
            "semantic restore must recover the remembered pane extent");

    (*split).set_splitter_fixed(true);
    require(window.request_focus((*split).splitter_control()) &&
                window.dispatch_key({KeyAction::down, PhysicalKey::enter}) &&
                (*split).second_collapsed(),
            "fixed splitter seam tab must remain keyboard-operable for collapse");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::space}) &&
                !(*split).second_collapsed(),
            "Space must restore a collapsed pane through the focused seam tab");
}

void test_seam_grip_rest_proximity_focus_press_and_orientation_paint() {
    auto split = make_control<SplitContainer>(StableId("split.grip.visual"));
    split->set_splitter_width(3.0);
    split->set_splitter_hit_width(9.0);
    split->set_splitter_distance(120.0);
    split->set_collapse_panel(SplitFixedPanel::second);
    Window window(split, {400.0, 180.0});
    window.perform_layout();
    const Control::Ptr grip = split->splitter_control();
    const Rect seam = grip->absolute_bounds();
    const Insets outsets = grip->visual_outsets();
    require(near(seam.width, 9.0) && near(outsets.left, 10.5) &&
                near(outsets.right, 10.5) && near(outsets.top, 0.0) &&
                near(outsets.bottom, 0.0),
            "vertical seam must retain a nine-unit hit strip and declare the complete engaged shadow outsets");

    GripRecordingPainter rest;
    grip->on_paint(rest, {0.0, 0.0, seam.width, seam.height});
    require(rest.fills.size() == 2U && near(rest.fills[0].width, 3.0) &&
                near(rest.fills[1].width, 7.0) &&
                near(rest.fills[1].height, 28.0) && rest.shadows.empty() &&
                rest.texts.size() == 1U,
            "rest seam must paint a three-unit rail and quiet seven-by-twenty-eight actuator");

    const Point center{seam.x + seam.width * 0.5,
                       seam.y + seam.height * 0.5};
    static_cast<void>(window.dispatch_pointer(
        {PointerAction::move, PointerButton::none, center}));
    GripRecordingPainter hot;
    grip->on_paint(hot, {0.0, 0.0, seam.width, seam.height});
    require(hot.fills.size() == 2U && near(hot.fills[1].width, 12.0) &&
                near(hot.fills[1].height, 42.0) &&
                near(hot.fills[1].x, -1.5) && hot.shadows.size() == 1U &&
                hot.lines >= 3U,
            "pointer proximity must enlarge, deepen, and increase contrast before activation");

    static_cast<void>(window.dispatch_pointer(
        {PointerAction::move, PointerButton::none, {20.0, 20.0}}));
    GripRecordingPainter left;
    grip->on_paint(left, {0.0, 0.0, seam.width, seam.height});
    require(left.fills.size() == 2U && near(left.fills[1].width, 7.0) &&
                near(left.fills[1].height, 28.0) && left.shadows.empty(),
            "leaving the actuator proximity must restore its quiet rest geometry");

    require(window.request_focus(grip),
            "splitter grip must accept keyboard focus");
    static_cast<void>(window.dispatch_key(
        {KeyAction::down, PhysicalKey::a}));
    GripRecordingPainter focused;
    grip->on_paint(focused, {0.0, 0.0, seam.width, seam.height});
    require(focused.fills.size() == 2U &&
                near(focused.fills[1].width, 12.0) &&
                near(focused.fills[1].height, 42.0) &&
                focused.shadows.size() == 1U,
            "keyboard focus cue must expose the same enlarged mechanical grip");

    static_cast<void>(window.request_focus({}));
    PointerEvent down;
    down.action = PointerAction::down;
    down.button = PointerButton::primary;
    down.position = center;
    require(window.dispatch_pointer(down),
            "collapse actuator press must route through the expanded activation box");
    GripRecordingPainter pressed;
    grip->on_paint(pressed, {0.0, 0.0, seam.width, seam.height});
    require(pressed.fills.size() == 2U &&
                near(pressed.fills[1].width, 12.0) &&
                near(pressed.fills[1].height, 42.0) &&
                pressed.shadows.size() == 1U,
            "pressed collapse actuator must retain its pre-activation engaged treatment");
    PointerEvent up = down;
    up.action = PointerAction::up;
    require(window.dispatch_pointer(up) && split->second_collapsed(),
            "engaged grip release must preserve the existing pane-collapse action");

    auto horizontal = make_control<SplitContainer>(
        StableId("split.grip.horizontal"));
    horizontal->set_orientation(Orientation::horizontal);
    horizontal->set_splitter_width(3.0);
    horizontal->set_splitter_hit_width(9.0);
    horizontal->set_splitter_distance(50.0);
    horizontal->set_collapse_panel(SplitFixedPanel::first);
    Window horizontal_window(horizontal, {180.0, 120.0});
    horizontal_window.perform_layout();
    const Control::Ptr horizontal_grip = horizontal->splitter_control();
    const Insets horizontal_outsets = horizontal_grip->visual_outsets();
    GripRecordingPainter horizontal_rest;
    horizontal_grip->on_paint(
        horizontal_rest, {0.0, 0.0,
                          horizontal_grip->absolute_bounds().width,
                          horizontal_grip->absolute_bounds().height});
    require(near(horizontal_outsets.top, 8.5) &&
                near(horizontal_outsets.bottom, 12.5) &&
                horizontal_rest.fills.size() == 2U &&
                near(horizontal_rest.fills[0].height, 3.0) &&
                near(horizontal_rest.fills[1].width, 28.0) &&
                near(horizontal_rest.fills[1].height, 7.0),
            "horizontal grip must transpose the same seam, actuator, and asymmetric shadow outsets");
}

void test_automatic_accommodation_and_user_override() {
    std::shared_ptr<gui_forms::SplitContainer> split = make_control<SplitContainer>(StableId("split.accommodation"));
    (*split).set_splitter_distance(250.0);
    (*split).set_collapse_panel(SplitFixedPanel::second);
    (*split).set_automatic_collapse_threshold(360.0);
    Window window(split, {400.0, 160.0});
    window.perform_layout();
    require(!(*split).second_collapsed(),
            "roomy split must begin expanded above its authored threshold");

    window.resize({330.0, 160.0});
    window.perform_layout();
    require((*split).second_collapsed() &&
                (*split).second_collapse_origin() ==
                    SplitCollapseOrigin::automatic_accommodation,
            "crossing below the threshold must record automatic collapse origin");

    require(window.request_focus((*split).splitter_control()) &&
                window.dispatch_key({KeyAction::down, PhysicalKey::enter}) &&
                !(*split).second_collapsed(),
            "user must be able to restore an automatically collapsed pane");
    window.perform_layout();
    require(!(*split).second_collapsed(),
            "user restore below threshold must suppress immediate recollapse");

    window.resize({400.0, 160.0});
    window.perform_layout();
    window.resize({330.0, 160.0});
    window.perform_layout();
    require((*split).second_collapsed() &&
                (*split).second_collapse_origin() ==
                    SplitCollapseOrigin::automatic_accommodation,
            "crossing above then below must re-arm automatic accommodation");
    window.resize({400.0, 160.0});
    window.perform_layout();
    require(!(*split).second_collapsed(),
            "room restoration must restore only an automatically collapsed pane");

    (*split).set_second_collapsed(true, SplitCollapseOrigin::user);
    window.resize({500.0, 160.0});
    window.perform_layout();
    require((*split).second_collapsed() &&
                (*split).second_collapse_origin() == SplitCollapseOrigin::user,
            "room restoration must not override a user-collapsed pane");
}

void test_content_aware_maximum_extents() {
    std::shared_ptr<gui_forms::SplitContainer> right_bounded = make_control<SplitContainer>(StableId("split.max.right"));
    (*right_bounded).set_second_maximum(180.0);
    (*right_bounded).set_splitter_distance(100.0);
    Window right_window(right_bounded, {600.0, 160.0});
    right_window.perform_layout();
    require(near((*(*right_bounded).second_panel()).arranged_bounds().width, 180.0),
            "second maximum must stop a pane consuming space its content cannot use");
    (*right_bounded).set_splitter_distance(0.0);
    right_window.perform_layout();
    require(near((*(*right_bounded).second_panel()).arranged_bounds().width, 180.0),
            "programmatic and pointer distances must share maximum constraints");

    std::shared_ptr<gui_forms::SplitContainer> left_bounded = make_control<SplitContainer>(StableId("split.max.left"));
    (*left_bounded).set_first_maximum(200.0);
    (*left_bounded).set_splitter_distance(500.0);
    Window left_window(left_bounded, {600.0, 160.0});
    left_window.perform_layout();
    require(near((*(*left_bounded).first_panel()).arranged_bounds().width, 200.0),
            "first maximum must symmetrically bound content-aware pane growth");

    bool invalid_rejected{};
    try {
        (*left_bounded).set_first_maximum(10.0);
    } catch (const std::invalid_argument&) {
        invalid_rejected = true;
    }
    require(invalid_rejected,
            "pane maximum may not contradict its declared minimum");
}

} // namespace

int main() {
    try {
        test_stable_tree_and_constrained_geometry();
        test_live_pointer_and_keyboard_resize();
        test_pane_surface_background();
        test_collapse_focus_restore_and_fixed_panel_resize();
        test_horizontal_orientation_and_thread_guard();
        test_seam_tab_pointer_keyboard_and_semantic_collapse();
        test_seam_grip_rest_proximity_focus_press_and_orientation_paint();
        test_automatic_accommodation_and_user_override();
        test_content_aware_maximum_extents();
        std::cout << "split-container-tests: pass\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "split-container-tests: fail: " << error.what() << '\n';
        return 1;
    }
}
