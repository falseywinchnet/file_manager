#include "gui_forms/gui_forms.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

class OrderPainter final : public Painter {
public:
    void save() override {}
    void restore() override {}
    void translate(Point) override {}
    void clip_rect(Rect) override {}
    void fill_rect(Rect, Color) override {}
    void stroke_rect(Rect, Color, double) override {}
    void draw_line(Point, Point, Color, double) override {}
    void draw_text_utf8(Point,
                        std::string_view text,
                        FontSpec,
                        Color) override {
        order.emplace_back(text);
    }
    void draw_image(ImageId, Rect, double) override {}

    std::vector<std::string> order;
};

class ChunkControl final : public Control {
public:
    ChunkControl(StableId id, std::string label)
        : Control(std::move(id)), label_(std::move(label)) {}

    void on_paint(Painter& painter, Rect) override {
        ++paint_callbacks;
        painter.draw_text_utf8({2.0, 12.0}, label_, {}, Color::rgba(20, 30, 40));
    }

    std::uint64_t paint_callbacks{};

private:
    std::string label_;
};

struct Fixture {
    Fixture() {
        root->set_paint_plane(PaintPlane::backplane);
        root->set_requested_bounds({0.0, 0.0, 200.0, 120.0});
        overlay->set_paint_plane(PaintPlane::overlay);
        overlay->set_requested_bounds({0.0, 0.0, 200.0, 120.0});
        control->set_requested_bounds({10.0, 10.0, 80.0, 40.0});

        // Overlay precedes the ordinary child in tree order. Plane ordering,
        // not insertion order, must still place it last.
        root->add_child(overlay);
        root->add_child(control);
        window = std::make_unique<Window>(root, Size{200.0, 120.0});
        window->perform_layout();
        static_cast<void>(window->take_damage());
        window->paint(painter, {0.0, 0.0, 200.0, 120.0});
        window->reset_activity_metrics();
        painter.order.clear();
    }

    std::shared_ptr<ChunkControl> root =
        make_control<ChunkControl>(StableId("chunk.root"), "backplane");
    std::shared_ptr<ChunkControl> overlay =
        make_control<ChunkControl>(StableId("chunk.overlay"), "overlay");
    std::shared_ptr<ChunkControl> control =
        make_control<ChunkControl>(StableId("chunk.control"), "control");
    std::unique_ptr<Window> window;
    OrderPainter painter;
};

void test_plane_order_and_exposure_reuse() {
    Fixture fixture;
    const std::uint64_t root_callbacks = fixture.root->paint_callbacks;
    const std::uint64_t control_callbacks = fixture.control->paint_callbacks;
    const std::uint64_t overlay_callbacks = fixture.overlay->paint_callbacks;

    fixture.window->paint(fixture.painter, {0.0, 0.0, 200.0, 120.0});
    require(fixture.painter.order ==
                std::vector<std::string>{"backplane", "control", "overlay"},
            "display replay must order backplane, controls, then overlays");
    require(fixture.root->paint_callbacks == root_callbacks &&
                fixture.control->paint_callbacks == control_callbacks &&
                fixture.overlay->paint_callbacks == overlay_callbacks,
            "exposure paint must replay chunks without invoking control paint callbacks");
    const MetricsSnapshot metrics = fixture.window->metrics_snapshot();
    require(metrics.display_chunks_rebuilt == 0 && metrics.display_chunks_reused == 3 &&
                metrics.display_commands_replayed == 3,
            "exposure metrics must report three reused one-command chunks");
}

void test_local_rebuild_and_generation() {
    Fixture fixture;
    const auto root_before = fixture.root->display_chunk_info();
    const auto control_before = fixture.control->display_chunk_info();
    const auto overlay_before = fixture.overlay->display_chunk_info();
    require(root_before && control_before && overlay_before,
            "initial paint must populate every display chunk");

    fixture.control->invalidate(invalidation::paint_only);
    require(fixture.window->take_damage(PaintPlane::backplane).empty() &&
                fixture.window->take_damage(PaintPlane::overlay).empty(),
            "control invalidation must not damage backplane or overlay planes");
    DamageRegion damage = fixture.window->take_damage(PaintPlane::control);
    require(!damage.empty(), "control invalidation must damage the control plane");
    fixture.window->paint(fixture.painter, damage.bounds());

    const auto root_after = fixture.root->display_chunk_info();
    const auto control_after = fixture.control->display_chunk_info();
    const auto overlay_after = fixture.overlay->display_chunk_info();
    require(root_after->generation == root_before->generation &&
                overlay_after->generation == overlay_before->generation &&
                control_after->generation > control_before->generation,
            "only the invalidated control chunk may advance generation");
    const MetricsSnapshot metrics = fixture.window->metrics_snapshot();
    require(metrics.display_chunks_rebuilt == 1 && metrics.display_chunks_reused == 2 &&
                metrics.paint_invalidations_consumed == 1,
            "localized paint must rebuild one chunk and reuse two plane peers");
}

void test_plane_migration_damages_old_and_new_planes() {
    Fixture fixture;
    fixture.control->set_paint_plane(PaintPlane::overlay);
    require(!fixture.window->take_damage(PaintPlane::control).empty(),
            "paint-plane migration must clear the old plane");
    require(!fixture.window->take_damage(PaintPlane::overlay).empty(),
            "paint-plane migration must damage the new plane");
    require(!fixture.control->display_chunk_info().has_value(),
            "paint-plane migration must discard an incompatible cached chunk");
}

void test_invalid_plane_rejection_preserves_state() {
    Fixture fixture;
    const auto before = fixture.control->display_chunk_info();
    bool setter_rejected = false;
    try {
        fixture.control->set_paint_plane(static_cast<PaintPlane>(99));
    } catch (const std::invalid_argument&) {
        setter_rejected = true;
    }
    require(setter_rejected && fixture.control->paint_plane() == PaintPlane::control &&
                fixture.control->display_chunk_info()->generation == before->generation,
            "invalid paint-plane mutation must be rejected without changing cache state");

    fixture.control->invalidate(invalidation::paint_only);
    bool damage_rejected = false;
    try {
        static_cast<void>(fixture.window->take_damage(static_cast<PaintPlane>(99)));
    } catch (const std::invalid_argument&) {
        damage_rejected = true;
    }
    require(damage_rejected &&
                !fixture.window->take_damage(PaintPlane::control).empty(),
            "invalid damage-plane access must not consume valid pending damage");
}

void test_disposal_removes_cache_entry_and_returns_idle() {
    Fixture fixture;
    fixture.control->dispose();
    DamageRegion damage = fixture.window->take_damage();
    require(!damage.empty(), "disposing a cached control must damage its old pixels");
    fixture.window->paint(fixture.painter, damage.bounds());
    fixture.window->reset_activity_metrics();
    require(fixture.window->metrics_snapshot().display_cache_entries == 2,
            "disposed control must leave exactly two retained cache entries");
    require(!fixture.window->needs_frame() && !fixture.window->next_wake().has_value(),
            "completed disposal paint must return the window to idle");
}

} // namespace

int main() {
    try {
        test_plane_order_and_exposure_reuse();
        test_local_rebuild_and_generation();
        test_plane_migration_damages_old_and_new_planes();
        test_invalid_plane_rejection_preserves_state();
        test_disposal_removes_cache_entry_and_returns_idle();
        std::cout << "gui_forms_display_chunk_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_display_chunk_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
