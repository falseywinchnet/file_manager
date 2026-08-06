#include "gui_forms/gui_forms.hpp"
#include "../src/core/device_damage.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

class NullPainter final : public Painter {
public:
    void save() override {}
    void restore() override {}
    void translate(Point) override {}
    void clip_rect(Rect) override {}
    void fill_rect(Rect, Color) override { ++draws; }
    void stroke_rect(Rect, Color, double) override { ++draws; }
    void draw_line(Point, Point, Color, double) override { ++draws; }
    void draw_text_utf8(Point, std::string_view, FontSpec, Color) override { ++draws; }
    void draw_image(ImageId, Rect, double) override { ++draws; }

    std::uint64_t draws{};
};

class CountingControl : public Control {
public:
    explicit CountingControl(StableId id) : Control(std::move(id)) {}

    Size measure(Size available) override {
        ++measure_calls;
        return Control::measure(available);
    }
    void arrange(Rect bounds) override {
        ++arrange_calls;
        Control::arrange(bounds);
    }
    void on_paint(Painter& painter, Rect damage) override {
        ++paint_calls;
        painter.fill_rect(damage, Color::rgba(12, 34, 56));
    }

    std::uint64_t measure_calls{};
    std::uint64_t arrange_calls{};
    std::uint64_t paint_calls{};
};

class ReentrantLayoutControl final : public CountingControl {
public:
    explicit ReentrantLayoutControl(StableId id) : CountingControl(std::move(id)) {}

    void arrange(Rect bounds) override {
        CountingControl::arrange(bounds);
        Rect requested = requested_bounds();
        requested.x += 1.0;
        set_requested_bounds(requested);
    }
};

struct TreeFixture {
    TreeFixture() {
        root->set_requested_bounds({0.0, 0.0, 300.0, 180.0});
        left->set_requested_bounds({0.0, 0.0, 140.0, 180.0});
        right->set_requested_bounds({160.0, 0.0, 140.0, 180.0});
        left_leaf->set_requested_bounds({10.0, 10.0, 40.0, 30.0});
        left_sibling->set_requested_bounds({10.0, 60.0, 40.0, 30.0});
        right_leaf->set_requested_bounds({10.0, 10.0, 40.0, 30.0});
        left->add_child(left_leaf);
        left->add_child(left_sibling);
        right->add_child(right_leaf);
        root->add_child(left);
        root->add_child(right);
        window = std::make_unique<Window>(root, Size{300.0, 180.0});
        window->perform_layout();
        paint_pending();
        window->reset_activity_metrics();
    }

    void paint_pending() {
        DamageRegion damage = window->take_damage();
        if (!damage.empty()) {
            window->paint(painter, damage.bounds());
        }
    }

    std::shared_ptr<CountingControl> root =
        make_control<CountingControl>(StableId("typed.root"));
    std::shared_ptr<CountingControl> left =
        make_control<CountingControl>(StableId("typed.left"));
    std::shared_ptr<CountingControl> right =
        make_control<CountingControl>(StableId("typed.right"));
    std::shared_ptr<CountingControl> left_leaf =
        make_control<CountingControl>(StableId("typed.left.leaf"));
    std::shared_ptr<CountingControl> left_sibling =
        make_control<CountingControl>(StableId("typed.left.sibling"));
    std::shared_ptr<CountingControl> right_leaf =
        make_control<CountingControl>(StableId("typed.right.leaf"));
    std::unique_ptr<Window> window;
    NullPainter painter;
};

void test_typed_effect_vocabulary() {
    require(has_dirty(invalidation::bounds, Dirty::measure) &&
                has_dirty(invalidation::bounds, Dirty::arrange) &&
                has_dirty(invalidation::bounds, Dirty::paint) &&
                has_dirty(invalidation::bounds, Dirty::hit_test),
            "bounds effect must explicitly declare measure/arrange/paint/hit-test");
    require(has_dirty(invalidation::text_content, Dirty::text) &&
                has_dirty(invalidation::text_content, Dirty::accessibility),
            "text effect must remain distinct from paint and semantics");
    require(has_dirty(invalidation::style_only, Dirty::style) &&
                !has_dirty(invalidation::style_only, Dirty::measure),
            "paint-only style effect must not silently request measurement");
}

void test_affected_path_layout() {
    TreeFixture fixture;
    const std::uint64_t right_measure = fixture.right->measure_calls;
    const std::uint64_t right_arrange = fixture.right->arrange_calls;
    const std::uint64_t sibling_measure = fixture.left_sibling->measure_calls;
    const std::uint64_t sibling_arrange = fixture.left_sibling->arrange_calls;

    fixture.left_leaf->set_requested_bounds({20.0, 15.0, 44.0, 32.0});
    require(fixture.left_leaf->arranged_bounds() == Rect{20.0, 15.0, 44.0, 32.0},
            "read barrier must commit the affected leaf geometry");
    const MetricsSnapshot metrics = fixture.window->metrics_snapshot();
    require(metrics.controls_measured == 3 && metrics.controls_arranged == 3,
            "leaf bounds change must invoke layout only on root-to-leaf path");
    require(metrics.measure_nodes_visited == 3 && metrics.arrange_nodes_visited == 3,
            "layout visit metrics must match the affected path exactly");
    require(fixture.right->measure_calls == right_measure &&
                fixture.right->arrange_calls == right_arrange &&
                fixture.left_sibling->measure_calls == sibling_measure &&
                fixture.left_sibling->arrange_calls == sibling_arrange,
            "unaffected sibling subtrees must not receive layout callbacks");
}

void test_full_subtree_invalidation_is_explicit() {
    TreeFixture fixture;
    fixture.window->resize({320.0, 190.0});
    fixture.window->perform_layout();
    const MetricsSnapshot metrics = fixture.window->metrics_snapshot();
    require(metrics.controls_measured == 6 && metrics.controls_arranged == 6,
            "window resize must explicitly invalidate the complete retained subtree");
    require(metrics.measure_nodes_visited == 6 && metrics.arrange_nodes_visited == 6,
            "full-subtree traversal count must equal the six retained controls");
}

void test_damage_take_commits_layout_generated_geometry_damage() {
    auto root = make_control<CountingControl>(StableId("damage.layout.root"));
    root->set_padding({10.0, 10.0, 10.0, 10.0});
    auto fill = make_control<CountingControl>(StableId("damage.layout.fill"));
    fill->set_requested_bounds({0.0, 0.0, 20.0, 20.0});
    fill->set_dock(DockStyle::fill);
    auto top = make_control<CountingControl>(StableId("damage.layout.top"));
    top->set_requested_bounds({0.0, 0.0, 20.0, 24.0});
    top->set_dock(DockStyle::top);
    auto left = make_control<CountingControl>(StableId("damage.layout.left"));
    left->set_requested_bounds({0.0, 0.0, 50.0, 20.0});
    left->set_dock(DockStyle::left);
    root->add_child(fill);
    root->add_child(top);
    root->add_child(left);
    Window window(root, {300.0, 180.0});
    window.perform_layout();
    NullPainter painter;
    DamageRegion initial = window.take_damage();
    window.paint(painter, initial.bounds());
    const Rect old_top = top->arranged_bounds();
    const Rect old_fill = fill->arranged_bounds();

    left->set_visible(false);
    DamageRegion geometry_damage = window.take_damage();
    const Rect new_top = top->arranged_bounds();
    const Rect new_fill = fill->arranged_bounds();
    require(new_top.x < old_top.x && new_top.width > old_top.width &&
                new_fill.x < old_fill.x && new_fill.width > old_fill.width,
            "taking host damage must commit sibling geometry released by hidden Dock");
    const Rect bounds = geometry_damage.bounds();
    require(bounds.x <= new_top.x &&
                bounds.x + bounds.width >= old_top.x + old_top.width &&
                bounds.y <= new_fill.y &&
                bounds.y + bounds.height >= old_fill.y + old_fill.height,
            "host damage must cover both old and newly arranged docked sibling pixels");
    window.paint(painter, geometry_damage.bounds());
    require(!window.needs_frame(),
            "one host paint transaction must consume layout-generated damage completely");
}

void test_reentrant_layout_is_bounded() {
    auto root = make_control<ReentrantLayoutControl>(StableId("typed.reentrant"));
    root->set_requested_bounds({0.0, 0.0, 100.0, 80.0});
    Window window(root, {100.0, 80.0});
    window.reset_activity_metrics();

    window.perform_layout();
    const MetricsSnapshot metrics = window.metrics_snapshot();
    require(metrics.measure_passes == 4 && metrics.arrange_passes == 4,
            "reentrant layout must stop at the declared four-pass guard");
    require(metrics.bounded_pass_limit_hits == 1,
            "an unquiescent layout must report one structured pass-limit hit");
}

void test_paint_metrics_report_chunk_work() {
    TreeFixture fixture;
    fixture.left_leaf->invalidate(invalidation::paint_only);
    fixture.paint_pending();
    const MetricsSnapshot metrics = fixture.window->metrics_snapshot();
    require(metrics.measure_passes == 0 && metrics.arrange_passes == 0,
            "paint-only invalidation must not trigger layout");
    require(metrics.paint_invalidations_consumed == 1,
            "paint pass must report the exact consumed invalidation count");
    require(metrics.display_chunks_rebuilt == 1 && metrics.display_chunks_reused == 2,
            "leaf paint must rebuild one chunk and reuse the two affected ancestors");
    require(metrics.display_commands_replayed > 0 && metrics.display_cache_entries == 6,
            "chunk metrics must expose replay work and the six-entry retained cache");
    require(metrics.paint_nodes_visited >= metrics.controls_painted &&
                metrics.controls_painted > 0,
            "paint visit and callback counters must describe different work");
}

void test_declared_mutation_guard() {
    TreeFixture fixture;
#ifndef NDEBUG
    const Dirty before = fixture.left_leaf->dirty();
#endif
    bool rejected = false;
    try {
        fixture.left_leaf->invalidate_declared(Dirty::none);
    } catch (const std::logic_error&) {
        rejected = true;
    }
    const MetricsSnapshot metrics = fixture.window->metrics_snapshot();
    require(metrics.undeclared_mutations == 1,
            "undeclared mutation attempt must be a structured diagnostic");
#ifndef NDEBUG
    require(rejected && fixture.left_leaf->dirty() == before,
            "development build must reject undeclared mutation without changing dirtiness");
#else
    require(!rejected &&
                has_dirty(fixture.left_leaf->dirty(), invalidation::conservative_subtree),
            "production build must conservatively invalidate undeclared mutation subtree");
#endif
}

void test_damage_compaction_and_complexity_guard() {
    DamageRegion adjacent;
    adjacent.add({0.0, 0.0, 10.0, 10.0});
    adjacent.add({10.0, 0.0, 10.0, 10.0});
    require(adjacent.rectangle_count() == 1 && adjacent.area() == 200.0 &&
                adjacent.compaction_count() == 1,
            "exact adjacent damage must compact without adding overdraw");

    DamageRegion corner_overlap;
    corner_overlap.add({0.0, 0.0, 10.0, 10.0});
    corner_overlap.add({5.0, 5.0, 10.0, 10.0});
    require(corner_overlap.rectangle_count() == 2 && corner_overlap.area() == 175.0,
            "non-rectangular union must retain exact rectangles below complexity guard");

    DamageRegion guarded;
    for (std::size_t index = 0; index <= DamageRegion::maximum_rectangles; ++index) {
        guarded.add({static_cast<double>(index * 3U), 0.0, 1.0, 1.0});
    }
    require(guarded.rectangle_count() <= DamageRegion::maximum_rectangles &&
                guarded.collapse_count() == 1,
            "damage complexity guard must collapse an over-limit region deterministically");
}

void test_device_damage_alignment_is_outward_and_scale_exact() {
    const Rect one_percent{0.0, 0.0, 900.0, 6.2};
    require(detail::align_damage_outward(one_percent, 1.0) ==
                Rect{0.0, 0.0, 900.0, 7.0},
            "fractional 1x damage must cover the final device pixel");
    require(detail::align_damage_outward(one_percent, 2.0) ==
                Rect{0.0, 0.0, 900.0, 6.5},
            "fractional 2x damage must remain exact in logical coordinates");
    require(detail::align_damage_outward({-0.2, -0.2, 1.0, 1.0}, 2.0) ==
                Rect{-0.5, -0.5, 1.5, 1.5},
            "outward alignment must floor negative leading edges and ceil trailing edges");
    require(detail::align_damage_outward(one_percent, 0.0).empty(),
            "invalid scale must not produce a damage rectangle");
}

void test_idle_remains_quiescent_after_compacted_damage() {
    TreeFixture fixture;
    for (int index = 0; index < 20; ++index) {
        fixture.left_leaf->invalidate(invalidation::paint_only);
    }
    fixture.paint_pending();
    fixture.window->reset_activity_metrics();
    require(!fixture.window->needs_frame() && !fixture.window->next_wake().has_value(),
            "painted retained damage must return to frame-and-wake quiescence");
    const MetricsSnapshot idle = fixture.window->metrics_snapshot();
    require(idle.measure_passes == 0 && idle.arrange_passes == 0 &&
                idle.paint_passes == 0 && idle.paint_invalidations_consumed == 0,
            "idle snapshot must report no latent layout or paint work");
}

} // namespace

int main() {
    try {
        test_typed_effect_vocabulary();
        test_affected_path_layout();
        test_full_subtree_invalidation_is_explicit();
        test_damage_take_commits_layout_generated_geometry_damage();
        test_reentrant_layout_is_bounded();
        test_paint_metrics_report_chunk_work();
        test_declared_mutation_guard();
        test_damage_compaction_and_complexity_guard();
        test_device_damage_alignment_is_outward_and_scale_exact();
        test_idle_remains_quiescent_after_compacted_damage();
        std::cout << "gui_forms_invalidation_damage_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_invalidation_damage_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
