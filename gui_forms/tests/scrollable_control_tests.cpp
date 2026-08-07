#include "gui_forms/gui_forms.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <type_traits>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

bool near(double left, double right) {
    return std::abs(left - right) < 0.0001;
}

void test_control_family_and_manual_axis_contract() {
    static_assert(std::is_base_of_v<ScrollableControl, Panel>);
    static_assert(std::is_base_of_v<ScrollableControl, ContainerControl>);

    auto panel = make_control<Panel>(StableId("scroll.manual"));
    require(!panel->auto_scroll() && !panel->hscroll() && !panel->vscroll(),
            "scrollable controls must default to an idle unscrolled viewport");
    require(panel->horizontal_scroll().enabled() &&
                panel->horizontal_scroll().minimum() == 0.0 &&
                panel->horizontal_scroll().maximum() == 100.0 &&
                panel->horizontal_scroll().large_change() == 10.0 &&
                panel->horizontal_scroll().small_change() == 1.0,
            "manual ScrollProperties defaults must match the Forms contract");

    panel->set_hscroll(true);
    panel->horizontal_scroll().set_maximum(250.0);
    panel->horizontal_scroll().set_large_change(50.0);
    panel->horizontal_scroll().set_value(80.0);
    Window window(panel, {100.0, 80.0});
    window.perform_layout();
    require(panel->hscroll() && !panel->vscroll() &&
                panel->viewport_rectangle() == Rect{0.0, 0.0, 100.0, 64.0} &&
                panel->scroll_position() == Point{80.0, 0.0},
            "manual scroll visibility and value must survive retained layout");

    panel->horizontal_scroll().set_large_change(500.0);
    panel->horizontal_scroll().set_small_change(600.0);
    require(panel->horizontal_scroll().large_change() == 251.0 &&
                panel->horizontal_scroll().small_change() == 251.0,
            "effective scroll changes must clamp without losing authored values");

    bool bad_minimum{};
    bool bad_value{};
    try {
        panel->horizontal_scroll().set_minimum(-1.0);
    } catch (const std::invalid_argument&) {
        bad_minimum = true;
    }
    try {
        panel->horizontal_scroll().set_value(300.0);
    } catch (const std::out_of_range&) {
        bad_value = true;
    }
    require(bad_minimum && bad_value,
            "ScrollProperties must reject invalid ranges and out-of-range values");
}

void test_auto_scroll_projection_viewport_and_resize() {
    auto panel = make_control<Panel>(StableId("scroll.auto"));
    auto child = make_control<Button>(StableId("scroll.auto.child"), "Content");
    child->set_requested_bounds({20.0, 10.0, 220.0, 160.0});
    panel->add_child(child);
    panel->set_auto_scroll_margin({5.0, 7.0});
    panel->set_auto_scroll(true);
    Window window(panel, {100.0, 80.0});
    window.perform_layout();

    ScrollSnapshot initial = panel->scroll_snapshot();
    require(initial.horizontal.visible && initial.vertical.visible &&
                initial.viewport_rectangle == Rect{0.0, 0.0, 84.0, 64.0} &&
                initial.display_rectangle == Rect{0.0, 0.0, 248.0, 180.0},
            "automatic scroll layout must solve two-axis scrollbar interdependence");

    const Rect authored = child->requested_bounds();
    require(panel->scroll_to({50.0, 40.0}),
            "programmatic scroll must move a visible automatic viewport");
    window.perform_layout();
    require(panel->auto_scroll_position() == Point{-50.0, -40.0} &&
                child->requested_bounds() == authored &&
                child->committed_arranged_bounds() ==
                    Rect{-30.0, -30.0, 220.0, 160.0} &&
                panel->display_rectangle() == Rect{-50.0, -40.0, 248.0, 180.0},
            "scrolling must project retained child geometry without destructive drift");
    window.perform_layout();
    require(child->committed_arranged_bounds() ==
                Rect{-30.0, -30.0, 220.0, 160.0},
            "repeated layout must not accumulate the scroll translation");
    require(!panel->get_child_at_point({10.0, 70.0}) &&
                window.hit_test({10.0, 70.0}) == panel,
            "children must not receive hits through scrollbar chrome");

    window.resize({300.0, 220.0});
    window.perform_layout();
    require(!panel->hscroll() && !panel->vscroll() &&
                panel->scroll_position() == Point{} &&
                child->committed_arranged_bounds() == authored,
            "viewport growth must hide unnecessary bars, clamp position, and restore projection");
}

void test_scroll_gestures_events_semantics_and_state() {
    auto panel = make_control<Panel>(StableId("scroll.input"));
    auto content = make_control<Control>(StableId("scroll.input.content"));
    content->set_requested_bounds({0.0, 0.0, 40.0, 300.0});
    panel->add_child(content);
    panel->set_auto_scroll(true);
    Window window(panel, {100.0, 100.0});
    window.perform_layout();

    std::size_t notifications{};
    ScrollEvent last;
    auto token = panel->scroll().subscribe([&](ScrollEvent& event) {
        ++notifications;
        last = event;
    });
    require(panel->scroll_to({0.0, 25.0}) && notifications == 0,
            "programmatic viewport changes must not masquerade as user Scroll events");
    window.perform_layout();

    PointerEvent wheel;
    wheel.action = PointerAction::wheel;
    wheel.position = {20.0, 20.0};
    wheel.wheel_delta = {0.0, -1.0};
    require(window.dispatch_pointer(wheel) &&
                panel->scroll_position().y > 25.0 && notifications == 0,
            "wheel input must update retained position while preserving WinForms Scroll suppression");

    const auto nodes = panel->semantic_virtual_children();
    require(nodes.size() == 1U &&
                nodes.front().role == SemanticRole::scroll_bar &&
                nodes.front().stable_id == "scroll.input.vertical-scroll",
            "visible axes must publish stable virtual accessibility scrollbars");
    const double before = panel->scroll_position().y;
    require(panel->on_semantic_child_action(
                "scroll.input.vertical-scroll", SemanticAction::increment, "") &&
                panel->scroll_position().y > before && notifications == 1U &&
                last.type == ScrollEventType::small_increment &&
                last.orientation == ScrollOrientation::vertical,
            "semantic scrollbar actions must use the same eventful interaction path");
    require(panel->get_scroll_state(
                ScrollableControl::scroll_state_user_has_scrolled),
            "eventful scrolling must retain the user-scrolled state");

    bool bad_state{};
    try {
        panel->set_scroll_state(0x80000000U, true);
    } catch (const std::invalid_argument&) {
        bad_state = true;
    }
    require(bad_state, "unknown scroll state bits must be rejected explicitly");
}

void test_control_into_view_offset_and_minimum_extent() {
    auto panel = make_control<Panel>(StableId("scroll.into-view"));
    auto target = make_control<Button>(StableId("scroll.into-view.target"), "Target");
    target->set_requested_bounds({180.0, 120.0, 20.0, 20.0});
    target->set_auto_scroll_offset({3.0, 5.0});
    panel->add_child(target);
    panel->set_auto_scroll_min_size({220.0, 180.0});
    Window window(panel, {100.0, 80.0});
    window.perform_layout();
    require(panel->auto_scroll() && panel->hscroll() && panel->vscroll(),
            "AutoScrollMinSize must enable automatic scrolling and contribute extent");

    panel->scroll_control_into_view(target);
    window.perform_layout();
    require(near(panel->scroll_position().x, 113.0) &&
                near(panel->scroll_position().y, 71.0),
            "ScrollControlIntoView must honor viewport geometry and AutoScrollOffset");

    bool invalid_margin{};
    try {
        panel->set_auto_scroll_margin(Size{-1.0, 0.0});
    } catch (const std::invalid_argument&) {
        invalid_margin = true;
    }
    panel->set_auto_scroll_margin(-5.0, 9.0);
    require(invalid_margin && panel->auto_scroll_margin() == Size{0.0, 9.0},
            "property assignment must reject negative margins while the Forms helper clamps them");
}

} // namespace

int main() {
    try {
        test_control_family_and_manual_axis_contract();
        test_auto_scroll_projection_viewport_and_resize();
        test_scroll_gestures_events_semantics_and_state();
        test_control_into_view_offset_and_minimum_extent();
        std::cout << "gui_forms_scrollable_control_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_scrollable_control_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
