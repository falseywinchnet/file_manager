#include "gui_forms/gui_forms.hpp"

#include <cstdlib>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

using namespace gui_forms;

class RecordingPainter final : public Painter {
public:
    void save() override { ++save_count; }
    void restore() override { ++restore_count; }
    void translate(Point) override {}
    void clip_rect(Rect) override {}
    void fill_rect(Rect, Color) override { ++draw_count; }
    void stroke_rect(Rect, Color, double) override { ++draw_count; }
    void draw_line(Point, Point, Color, double) override { ++draw_count; }
    void draw_text_utf8(Point, std::string_view, FontSpec, Color) override { ++draw_count; }
    void draw_image(ImageId, Rect, double) override { ++draw_count; }

    std::uint64_t save_count{};
    std::uint64_t restore_count{};
    std::uint64_t draw_count{};
};

class ProbeControl : public Control {
public:
    explicit ProbeControl(StableId id) : Control(std::move(id)) {}

    Size measure(Size available) override {
        ++measure_count;
        return Control::measure(available);
    }

    void arrange(Rect bounds) override {
        ++arrange_count;
        Control::arrange(bounds);
    }

    void on_paint(Painter& painter, Rect damage) override {
        ++paint_count;
        painter.fill_rect(damage, Color::rgba(10, 20, 30));
    }

    void on_pointer_preview(PointerEvent& event) override {
        phases.push_back(event.phase);
        if (handle_preview_release && event.action == PointerAction::up) {
            event.handled = true;
        }
    }

    void on_pointer(PointerEvent& event) override {
        phases.push_back(event.phase);
        if (event.action == PointerAction::up && release_callback) {
            release_callback();
        }
    }

    void on_pointer_bubble(PointerEvent& event) override {
        phases.push_back(event.phase);
    }

    void on_focus_changed(bool focused) override {
        focus_state = focused;
    }

    void on_activate() override {
        ++activation_count;
        invalidate(Dirty::paint);
    }

    std::uint64_t measure_count{};
    std::uint64_t arrange_count{};
    std::uint64_t paint_count{};
    std::uint64_t activation_count{};
    bool focus_state{};
    bool handle_preview_release{};
    std::function<void()> release_callback;
    std::vector<EventPhase> phases;
};

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void paint_pending(Window& window, RecordingPainter& painter) {
    DamageRegion damage = window.take_damage();
    if (!damage.empty()) {
        window.paint(painter, damage.bounds());
    }
}

struct Fixture {
    Fixture() {
        root->set_requested_bounds({0.0, 0.0, 200.0, 120.0});
        child->set_requested_bounds({10.0, 10.0, 40.0, 30.0});
        child->set_focusable(true);
        root->add_child(child);
        window = std::make_unique<Window>(root, Size{200.0, 120.0});
        window->perform_layout();
        paint_pending(*window, painter);
        window->reset_activity_metrics();
    }

    std::shared_ptr<ProbeControl> root = make_control<ProbeControl>(StableId("root"));
    std::shared_ptr<ProbeControl> child = make_control<ProbeControl>(StableId("child"));
    std::unique_ptr<Window> window;
    RecordingPainter painter;
};

void test_nested_scopes_and_read_barrier() {
    Fixture fixture;
    const Rect old_bounds = fixture.child->committed_arranged_bounds();
    {
        auto outer = fixture.window->begin_update();
        fixture.child->set_requested_bounds({20.0, 12.0, 44.0, 30.0});
        {
            auto inner = fixture.window->begin_update();
            fixture.child->set_requested_bounds({30.0, 14.0, 48.0, 32.0});
            require(fixture.child->arranged_bounds() == old_bounds,
                    "arranged read inside update scope must return committed geometry");
        }
        require(fixture.window->metrics_snapshot().arrange_passes == 0,
                "inner update scope must not flush layout");
    }
    const MetricsSnapshot snapshot = fixture.window->metrics_snapshot();
    require(fixture.child->arranged_bounds() == Rect{30.0, 14.0, 48.0, 32.0},
            "outer update close must commit final requested geometry");
    require(snapshot.update_scopes_started == 2, "both update scopes must be counted");
    require(snapshot.maximum_update_scope_depth == 2, "maximum nesting depth must be structured");
    require(snapshot.arrange_passes == 1, "nested mutations must coalesce to one arrange pass");
    require(snapshot.flush_count == 1, "nested mutations must coalesce to one flush");
}

void test_paint_only_does_not_measure() {
    Fixture fixture;
    fixture.child->invalidate(Dirty::paint);
    paint_pending(*fixture.window, fixture.painter);
    const MetricsSnapshot snapshot = fixture.window->metrics_snapshot();
    require(snapshot.measure_passes == 0, "paint-only mutation must not measure");
    require(snapshot.arrange_passes == 0, "paint-only mutation must not arrange");
    require(snapshot.paint_passes == 1, "paint-only mutation must produce one paint pass");
    require(snapshot.display_chunks_rebuilt == 1 && snapshot.display_chunks_reused == 1,
            "paint-only mutation must rebuild one chunk and reuse its retained ancestor");
    require(snapshot.paint_invalidations_consumed == 1,
            "paint-only mutation must consume one declared paint invalidation");
}

void test_layout_flushes_before_hit_test() {
    Fixture fixture;
    fixture.child->set_requested_bounds({80.0, 20.0, 40.0, 30.0});
    require(fixture.window->hit_test({85.0, 25.0}) == fixture.child,
            "hit test must observe newly arranged layout");
    const MetricsSnapshot snapshot = fixture.window->metrics_snapshot();
    require(snapshot.read_barrier_flushes == 1,
            "position-dependent hit test must record a layout read barrier");
}

void test_topmost_hit_and_activation() {
    Fixture fixture;
    auto top = make_control<ProbeControl>(StableId("top"));
    top->set_requested_bounds({10.0, 10.0, 40.0, 30.0});
    top->set_focusable(true);
    fixture.root->add_child(top);
    fixture.window->perform_layout();

    require(fixture.window->hit_test({15.0, 15.0}) == top,
            "last retained sibling must be topmost for hit testing");
    fixture.window->dispatch_pointer({PointerAction::down, PointerButton::primary,
                                      {15.0, 15.0}});
    fixture.window->dispatch_pointer({PointerAction::up, PointerButton::primary,
                                      {15.0, 15.0}});
    require(top->activation_count == 1, "eligible press/release must activate exactly once");
    require(top->focus_state, "primary press must focus an eligible target");
    require(fixture.window->metrics_snapshot().activations == 1,
            "activation must be counted structurally");

    fixture.window->dispatch_pointer({PointerAction::down, PointerButton::primary,
                                      {15.0, 15.0}});
    fixture.window->dispatch_pointer({PointerAction::up, PointerButton::primary,
                                      {150.0, 100.0}});
    require(top->activation_count == 1,
            "release away from pressed target must not activate");
}

void test_routed_phases_and_consumed_release() {
    Fixture fixture;
    fixture.window->dispatch_pointer({PointerAction::down, PointerButton::primary,
                                      {15.0, 15.0}});
    require(fixture.root->phases ==
                std::vector<EventPhase>{EventPhase::preview, EventPhase::bubble},
            "pointer down must preview root-to-target and bubble target-to-root");
    require(fixture.child->phases ==
                std::vector<EventPhase>{EventPhase::preview, EventPhase::target},
            "target must observe preview before its Forms-like event");

    fixture.child->handle_preview_release = true;
    require(fixture.window->dispatch_pointer({PointerAction::up, PointerButton::primary,
                                              {15.0, 15.0}}),
            "preview may consume release");
    require(fixture.child->activation_count == 0,
            "consumed release must not synthesize activation");

    auto other = make_control<ProbeControl>(StableId("other"));
    other->set_requested_bounds({100.0, 10.0, 40.0, 30.0});
    other->set_focusable(true);
    fixture.root->add_child(other);
    fixture.window->perform_layout();
    fixture.window->dispatch_pointer({PointerAction::down, PointerButton::primary,
                                      {105.0, 15.0}});
    require(fixture.window->focused_control() == other,
            "consumed physical release must still clear pointer capture");
}

void test_capture_released_before_release_callback() {
    Fixture fixture;
    bool callback_observed_release = false;
    fixture.child->release_callback = [&] {
        callback_observed_release = !fixture.window->captured_control();
    };
    fixture.window->dispatch_pointer({PointerAction::down, PointerButton::primary,
                                      {15.0, 15.0}});
    require(fixture.window->captured_control() == fixture.child,
            "primary down must capture before release ordering gate");
    fixture.window->dispatch_pointer({PointerAction::up, PointerButton::primary,
                                      {15.0, 15.0}});
    require(callback_observed_release,
            "pointer capture must be released before synchronous release callback");
    require(fixture.child->activation_count == 1,
            "early capture release must preserve qualified activation");
}

void test_disabled_control_is_ineligible() {
    Fixture fixture;
    fixture.child->set_enabled(false);
    require(!fixture.window->request_focus(fixture.child),
            "disabled control must reject focus");
    fixture.window->dispatch_pointer({PointerAction::down, PointerButton::primary,
                                      {15.0, 15.0}});
    fixture.window->dispatch_pointer({PointerAction::up, PointerButton::primary,
                                      {15.0, 15.0}});
    require(fixture.child->activation_count == 0, "disabled control must not activate");
}

void test_stable_ids_and_detached_lifetime() {
    Fixture fixture;
    require(fixture.window->find("child") == fixture.child, "stable ID must resolve attached control");
    const MetricsSnapshot before = fixture.window->metrics_snapshot();
    require(before.control_count == 2 && before.stable_id_count == 2,
            "population snapshot must count retained controls and IDs");

    auto duplicate = make_control<ProbeControl>(StableId("child"));
    bool duplicate_rejected = false;
    try {
        fixture.root->add_child(duplicate);
    } catch (const std::logic_error&) {
        duplicate_rejected = true;
    }
    require(duplicate_rejected, "duplicate stable ID must be rejected");

    Control::Ptr detached = fixture.root->remove_child(fixture.child->runtime_id());
    require(detached == fixture.child, "removal must return a strong detached handle");
    require(!detached->parent(), "detached control must have a weak empty parent");
    require(!fixture.window->find("child"), "detached stable ID must leave window registry");
}

void test_static_tree_factory() {
    ControlFactory factory;
    factory.register_type("probe", [](StableId id) {
        return make_control<ProbeControl>(std::move(id));
    });
    StaticNode description{"probe", "static.root", {0.0, 0.0, 100.0, 50.0},
                           {{"probe", "static.child", {2.0, 3.0, 20.0, 10.0}, {}}}};
    Control::Ptr root = build_static_tree(description, factory);
    Window window(root, {100.0, 50.0});
    require(window.find("static.child") != nullptr,
            "compiled static description must retain stable child ID");
}

void test_cursor_inheritance_and_override() {
    Fixture fixture;
    require(fixture.child->effective_cursor() == CursorKind::arrow,
            "controls must default to the portable arrow cursor");
    fixture.root->set_cursor(CursorKind::hand);
    require(fixture.child->effective_cursor() == CursorKind::hand,
            "unset child cursor must inherit from the retained parent");
    fixture.child->set_cursor(CursorKind::text);
    require(fixture.child->effective_cursor() == CursorKind::text,
            "explicit child cursor must override inherited cursor");
    fixture.child->set_cursor(std::nullopt);
    require(fixture.child->effective_cursor() == CursorKind::hand,
            "clearing a cursor override must restore inheritance");
}

void test_damage_and_idle_metrics() {
    DamageRegion region;
    region.add({0.0, 0.0, 10.0, 10.0});
    region.add({5.0, 0.0, 10.0, 10.0});
    require(region.area() == 150.0, "damage area must not double count overlaps");

    Fixture fixture;
    require(!fixture.window->needs_frame(), "fully painted retained tree must idle without a frame");
    fixture.window->metrics().set_renderer("recording-cpu", true);
    fixture.window->metrics().record_present(1000);
    fixture.window->metrics().record_present(2500);
    const MetricsSnapshot snapshot = fixture.window->metrics_snapshot();
    require(snapshot.frames_presented == 2 && snapshot.worst_present_duration_nanoseconds == 2500,
            "present durations must be structured and retain worst observation");
    require(snapshot.to_json().find("\"cpu_only\":true") != std::string::npos,
            "machine snapshot must declare CPU-only renderer capability");
}

} // namespace

int main() {
    try {
        test_nested_scopes_and_read_barrier();
        test_paint_only_does_not_measure();
        test_layout_flushes_before_hit_test();
        test_topmost_hit_and_activation();
        test_routed_phases_and_consumed_release();
        test_capture_released_before_release_callback();
        test_disabled_control_is_ineligible();
        test_stable_ids_and_detached_lifetime();
        test_static_tree_factory();
        test_cursor_inheritance_and_override();
        test_damage_and_idle_metrics();
        std::cout << "gui_forms_core_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_core_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
