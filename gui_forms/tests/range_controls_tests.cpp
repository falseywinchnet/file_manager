#include "gui_forms/container_controls.hpp"
#include "gui_forms/range_controls.hpp"
#include "gui_forms/window.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

class RecordingPainter final : public Painter {
public:
    void save() override { ++saves; }
    void restore() override { ++restores; }
    void translate(Point) override {}
    void clip_rect(Rect) override { ++clips; }
    void fill_rect(Rect, Color) override { ++fills; }
    void stroke_rect(Rect, Color, double) override { ++strokes; }
    void draw_line(Point, Point, Color, double) override { ++lines; }
    void draw_text_utf8(Point, std::string_view, FontSpec, Color) override {}
    void draw_image(ImageId, Rect, double) override {}

    std::uint64_t saves{};
    std::uint64_t restores{};
    std::uint64_t clips{};
    std::uint64_t fills{};
    std::uint64_t strokes{};
    std::uint64_t lines{};
};

void test_container_focus_identity() {
    auto root = make_control<ContainerControl>(StableId("container.root"));
    auto nested = make_control<UserControl>(StableId("container.user"));
    auto first = make_control<Button>(StableId("container.first"), "First");
    auto second = make_control<Button>(StableId("container.second"), "Second");
    first->set_requested_bounds({0.0, 0.0, 80.0, 24.0});
    second->set_requested_bounds({0.0, 28.0, 80.0, 24.0});
    nested->add_child(second);
    root->add_child(first);
    root->add_child(nested);
    Window window(root, {120.0, 80.0});

    require(root->contains_descendant(second) && nested->contains_descendant(second) &&
                !nested->contains_descendant(first),
            "container descendant identity must follow the logical retained tree");
    require(root->request_active_control(second) &&
                root->active_control() == second &&
                nested->active_control() == second,
            "container must request and query a focused nested descendant");
    require(nested->clear_active_control() && !window.focused_control() &&
                !root->active_control(),
            "nested container must clear focus only when it owns the active descendant");

    auto foreign = make_control<Button>(StableId("container.foreign"), "Foreign");
    require(!root->request_active_control(foreign),
            "container must reject detached or foreign controls");
}

void test_range_validation_and_event_order() {
    auto slider = make_control<TrackBar>(StableId("range.slider"));
    slider->set_range(0.0, 100.0);
    slider->set_value(50.0);
    slider->set_requested_bounds({0.0, 0.0, 200.0, 28.0});
    Window window(slider, {200.0, 28.0});
    require(window.request_focus(slider), "range test slider must accept focus");

    std::string order;
    auto range = slider->range_changed().subscribe(
        [&order](double, double) { order += "range\n"; });
    auto scroll = slider->scroll().subscribe(
        [&order](const RangeScrollEvent& event) {
            require(event.action == RangeAction::small_increment,
                    "right key must report small increment");
            order += "scroll\n";
        });
    auto value = slider->value_changed().subscribe(
        [&order](double) { order += "value\n"; });

    KeyEvent right;
    right.action = KeyAction::down;
    right.physical_key = PhysicalKey::right;
    require(window.dispatch_key(right) && slider->value() == 51.0 &&
                order == "scroll\nvalue\n",
            "input range mutation must publish scroll before value_changed");

    order.clear();
    slider->set_value(75.0);
    require(order == "value\n",
            "programmatic range mutation must publish value without a scroll event");

    order.clear();
    slider->set_range(80.0, 120.0);
    require(slider->value() == 80.0 && order == "range\nvalue\n",
            "range boundary mutation must publish range then a clamped value");

    bool invalid_range = false;
    bool invalid_value = false;
    bool invalid_change = false;
    try {
        slider->set_range(10.0, 10.0);
    } catch (const std::invalid_argument&) {
        invalid_range = true;
    }
    try {
        slider->set_value(200.0);
    } catch (const std::out_of_range&) {
        invalid_value = true;
    }
    try {
        slider->set_small_change(0.0);
    } catch (const std::invalid_argument&) {
        invalid_change = true;
    }
    require(invalid_range && invalid_value && invalid_change,
            "range substrate must reject degenerate ranges, overflow, and nonpositive steps");
}

void test_pointer_capture_keyboard_and_disabled_state() {
    auto slider = make_control<TrackBar>(StableId("capture.slider"));
    slider->set_requested_bounds({10.0, 10.0, 200.0, 28.0});
    slider->set_value(25.0);
    Window window(slider, {240.0, 60.0});

    PointerEvent down;
    down.action = PointerAction::down;
    down.button = PointerButton::primary;
    down.position = {110.0, 24.0};
    down.pointer_id = 7;
    require(window.dispatch_pointer(down) && window.captured_control() == slider,
            "track pointer down must enter retained capture");
    PointerEvent move;
    move.action = PointerAction::move;
    move.position = {500.0, 24.0};
    move.pointer_id = 7;
    require(window.dispatch_pointer(move) && slider->value() == 100.0,
            "captured track drag must clamp outside movement to maximum");
    PointerEvent up = move;
    up.action = PointerAction::up;
    up.button = PointerButton::primary;
    require(window.dispatch_pointer(up) && !window.captured_control(),
            "track pointer up must deterministically release capture");

    KeyEvent home;
    home.action = KeyAction::down;
    home.physical_key = PhysicalKey::home;
    require(window.dispatch_key(home) && slider->value() == 0.0,
            "Home must select the first range boundary");
    KeyEvent page_up = home;
    page_up.physical_key = PhysicalKey::page_up;
    require(window.dispatch_key(page_up) && slider->value() == 10.0,
            "Page Up must apply the declared large change");

    slider->set_enabled(false);
    const double disabled_value = slider->value();
    require(!window.dispatch_key(page_up) && slider->value() == disabled_value,
            "disabled range controls must reject keyboard mutation");
}

void test_progress_rendering_and_vertical_geometry() {
    auto panel = make_control<Panel>(StableId("range.panel"));
    panel->set_requested_bounds({0.0, 0.0, 240.0, 180.0});
    auto horizontal = make_control<ProgressBar>(StableId("range.progress.horizontal"));
    horizontal->set_requested_bounds({10.0, 10.0, 200.0, 20.0});
    horizontal->set_value(50.0);
    auto vertical = make_control<ProgressBar>(StableId("range.progress.vertical"));
    vertical->set_orientation(Orientation::vertical);
    vertical->set_requested_bounds({10.0, 40.0, 20.0, 120.0});
    vertical->set_value(75.0);
    auto track = make_control<TrackBar>(StableId("range.track.vertical"));
    track->set_orientation(Orientation::vertical);
    track->set_requested_bounds({50.0, 40.0, 28.0, 120.0});
    track->set_value(75.0);
    panel->add_child(horizontal);
    panel->add_child(vertical);
    panel->add_child(track);
    Window window(panel, {240.0, 180.0});
    window.perform_layout();

    RecordingPainter painter;
    window.paint(painter, {0.0, 0.0, 240.0, 180.0});
    require(painter.saves == painter.restores && painter.clips >= 4 &&
                painter.fills >= 8 && painter.lines >= 16,
            "horizontal and vertical ranges must render retained painter commands");
    const Rect progress_bounds = horizontal->absolute_bounds();
    const Point progress_center{progress_bounds.x + progress_bounds.width * 0.5,
                                progress_bounds.y + progress_bounds.height * 0.5};
    require(window.hit_test(progress_center) == panel,
            "noninteractive ProgressBar must not intercept its container");
}

void test_disposal_and_ui_thread_guard() {
    auto root = make_control<Panel>(StableId("range.dispose.root"));
    auto slider = make_control<TrackBar>(StableId("range.dispose.slider"));
    slider->set_requested_bounds({0.0, 0.0, 180.0, 28.0});
    root->add_child(slider);
    Window window(root, {180.0, 28.0});
    require(window.request_focus(slider), "disposal slider must receive focus");
    std::uint64_t later_values = 0;
    auto dispose_on_scroll = slider->scroll().subscribe(
        [slider](const RangeScrollEvent&) { slider->dispose(); });
    auto value = slider->value_changed().subscribe(
        [&later_values](double) { ++later_values; });
    KeyEvent right;
    right.action = KeyAction::down;
    right.physical_key = PhysicalKey::right;
    require(window.dispatch_key(right) && !slider->is_alive() && later_values == 0,
            "range disposal during scroll must suppress later value callbacks");

    auto progress = make_control<ProgressBar>(StableId("range.thread.progress"));
    Window thread_window(progress, {100.0, 20.0});
    bool rejected = false;
    std::thread worker([&] {
        try {
            progress->set_value(50.0);
        } catch (const std::logic_error&) {
            rejected = true;
        }
    });
    worker.join();
    require(rejected && progress->value() == 0.0,
            "range properties must preserve the core UI-thread guard");
}

} // namespace

int main() {
    try {
        test_container_focus_identity();
        test_range_validation_and_event_order();
        test_pointer_capture_keyboard_and_disabled_state();
        test_progress_rendering_and_vertical_geometry();
        test_disposal_and_ui_thread_guard();
        std::cout << "gui_forms_range_controls_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_range_controls_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
