#include "gui_forms/gui_forms.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

class RichPainter final : public Painter {
public:
    void save() override { ++saves; }
    void restore() override { ++restores; }
    void translate(Point) override {}
    void clip_rect(Rect rect) override {
        ++clips;
        last_clip = rect;
    }
    void clip_rounded_rect(Rect, double radius) override {
        ++rounded_clips;
        last_radius = radius;
    }
    void fill_rect(Rect, Color) override { ++solid_fills; }
    void fill_rounded_rect(Rect, double, Color) override { ++rounded_fills; }
    void stroke_rect(Rect, Color, double) override {}
    void stroke_rounded_rect(Rect, double radius, Color, double width) override {
        ++rounded_strokes;
        last_radius = radius;
        last_width = width;
    }
    void fill_linear_gradient(Rect, Point start, Point end,
                              std::span<const GradientStop> stops) override {
        ++linear_fills;
        last_start = start;
        last_end = end;
        last_stop_count = stops.size();
    }
    void fill_linear_gradient_spread(
        Rect, Point start, Point end,
        std::span<const GradientStop> stops,
        GradientSpreadMode spread) override {
        ++spread_linear_fills;
        last_start = start;
        last_end = end;
        last_stop_count = stops.size();
        last_spread = spread;
    }
    void fill_radial_gradient(Rect, Point center, Size radii,
                              std::span<const GradientStop> stops) override {
        ++radial_fills;
        last_center = center;
        last_radii = radii;
        last_stop_count = stops.size();
    }
    void draw_box_shadow(Rect, double, Point, double, double, Color) override {
        ++shadows;
    }
    void draw_line(Point, Point, Color, double) override {}
    void draw_text_utf8(Point, std::string_view, FontSpec, Color) override {}
    void draw_image(ImageId, Rect, double) override {}
    void draw_image_region(ImageId image, Rect source, Rect destination,
                           double opacity) override {
        image_regions.push_back({image, source, destination, opacity});
    }
    void fill_image_pattern(ImageId image, Size source_pixel_size,
                            Rect destination, Size logical_tile_size,
                            ImagePatternWrap wrap, double opacity) override {
        image_patterns.push_back({image, source_pixel_size, destination,
                                  logical_tile_size, wrap, opacity});
    }

    struct ImageRegion final {
        ImageId image;
        Rect source;
        Rect destination;
        double opacity{};
    };
    struct ImagePattern final {
        ImageId image;
        Size source_pixel_size;
        Rect destination;
        Size logical_tile_size;
        ImagePatternWrap wrap{ImagePatternWrap::tile};
        double opacity{};
    };

    unsigned saves{};
    unsigned restores{};
    unsigned clips{};
    unsigned rounded_clips{};
    unsigned solid_fills{};
    unsigned rounded_fills{};
    unsigned rounded_strokes{};
    unsigned linear_fills{};
    unsigned spread_linear_fills{};
    unsigned radial_fills{};
    unsigned shadows{};
    double last_radius{};
    double last_width{};
    Point last_start{};
    Point last_end{};
    Point last_center{};
    Size last_radii{};
    Rect last_clip{};
    std::size_t last_stop_count{};
    GradientSpreadMode last_spread{GradientSpreadMode::pad};
    std::vector<ImageRegion> image_regions;
    std::vector<ImagePattern> image_patterns;
};

class FallbackPainter final : public Painter {
public:
    void save() override {}
    void restore() override {}
    void translate(Point) override {}
    void clip_rect(Rect) override { ++clips; }
    void fill_rect(Rect, Color) override { ++fills; }
    void stroke_rect(Rect, Color, double) override { ++strokes; }
    void draw_line(Point, Point, Color, double) override {}
    void draw_text_utf8(Point, std::string_view, FontSpec, Color) override {}
    void draw_image(ImageId, Rect, double) override {}

    unsigned clips{};
    unsigned fills{};
    unsigned strokes{};
};

SurfaceMaterial specimen_material() {
    const std::vector<GradientStop> opaque{
        {0.0, Color::rgba(20, 50, 120)},
        {0.55, Color::rgba(55, 130, 205)},
        {1.0, Color::rgba(210, 85, 115)}};
    const std::vector<GradientStop> glow{
        {0.0, Color::rgba(255, 255, 255, 150)},
        {1.0, Color::rgba(255, 255, 255, 0)}};
    SurfaceMaterial material;
    material.fills = {
        MaterialFillLayer::linear({0.0, 0.0}, {1.0, 0.0}, opaque),
        MaterialFillLayer::radial({0.5, 0.5}, {0.5, 0.5}, glow)};
    material.shadows = {{{0.0, 3.0}, 6.0, 1.0,
                         Color::rgba(15, 25, 45, 80)}};
    material.border = MaterialBorder{Color::rgba(25, 45, 75), 2.0};
    material.corner_radius = 9.0;
    return material;
}

void test_material_validation_is_atomic() {
    auto panel = make_control<MaterialPanel>(StableId("material.atomic"));
    const SurfaceMaterial valid = specimen_material();
    unsigned changes{};
    auto token = panel->material_changed().subscribe(
        [&](const SurfaceMaterial&) { ++changes; });
    panel->set_material(valid);
    panel->set_material(valid);
    require(changes == 1U, "identical material assignment must be silent");

    SurfaceMaterial invalid = valid;
    invalid.fills[0].stops[1].offset = 0.0;
    bool rejected{};
    try {
        panel->set_material(std::move(invalid));
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected && panel->material() == valid && changes == 1U,
            "invalid material must be rejected without partial mutation");

    invalid = valid;
    invalid.fills.front().spread =
        static_cast<GradientSpreadMode>(0xffU);
    rejected = false;
    try {
        panel->set_material(std::move(invalid));
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected && panel->material() == valid && changes == 1U,
            "unknown gradient spread must be rejected atomically");
}

void test_retained_replay_preserves_material_operations() {
    auto panel = make_control<MaterialPanel>(StableId("material.replay"));
    panel->set_requested_bounds({0.0, 0.0, 200.0, 100.0});
    panel->set_material(specimen_material());
    Window window(panel, {200.0, 100.0});
    window.perform_layout();
    static_cast<void>(window.take_damage());

    RichPainter painter;
    window.paint(painter, {0.0, 0.0, 200.0, 100.0});
    require(painter.solid_fills == 1U && painter.shadows == 1U &&
                painter.rounded_clips == 1U &&
                painter.spread_linear_fills == 1U &&
                painter.linear_fills == 0U && painter.radial_fills == 1U &&
                painter.rounded_strokes == 2U && painter.last_width == 2.0 &&
                painter.saves >= 1U &&
                painter.restores == painter.saves,
            "window backplane and display chunks must retain every material operation");
    require(painter.last_start == Point{0.0, 0.0} &&
                painter.last_end == Point{200.0, 0.0} &&
                painter.last_center == Point{100.0, 50.0} &&
                painter.last_radii == Size{100.0, 50.0} &&
                painter.last_stop_count == 2U,
            "normalized material geometry must resolve against arranged bounds");
    require(painter.last_spread == GradientSpreadMode::pad,
            "retained material must preserve its explicit gradient spread");

    const auto before = panel->display_chunk_info();
    panel->set_material(SurfaceMaterial{});
    const DamageRegion damage = window.take_damage();
    require(before && !damage.empty(),
            "material mutation must invalidate its retained display chunk");
    window.paint(painter, damage.bounds());
    require(panel->display_chunk_info()->generation > before->generation,
            "material mutation must rebuild rather than reuse stale pixels");
}

void test_repeating_material_survives_record_and_replay() {
    auto panel = make_control<MaterialPanel>(StableId("material.repeat"));
    panel->set_requested_bounds({0.0, 0.0, 96.0, 48.0});
    SurfaceMaterial texture;
    texture.fills = {MaterialFillLayer::repeating_linear(
        {0.0, 0.0}, {8.0, 8.0},
        {{0.0, Color::rgba(20, 30, 40)},
         {0.48, Color::rgba(20, 30, 40)},
         {0.52, Color::rgba(65, 80, 92)},
         {1.0, Color::rgba(65, 80, 92)}})};
    panel->set_material(texture);
    Window window(panel, {96.0, 48.0});
    window.perform_layout();

    RichPainter painter;
    window.paint(painter, {0.0, 0.0, 96.0, 48.0});
    require(painter.spread_linear_fills == 1U &&
                painter.last_spread == GradientSpreadMode::repeat &&
                painter.last_start == Point{0.0, 0.0} &&
                painter.last_end == Point{8.0, 8.0} &&
                painter.last_stop_count == 4U,
            "repeat mode and logical period must survive the retained display list");
}

void test_image_materials_retain_crop_tile_and_nine_patch() {
    auto panel = make_control<MaterialPanel>(StableId("material.images"));
    panel->set_requested_bounds({0.0, 0.0, 30.0, 20.0});
    Window window(panel, {30.0, 20.0});
    const std::vector<std::byte> patch_pixels(12U * 12U * 4U,
                                               std::byte{0xff});
    const ImageLoadResult patch = window.load_bgra32_premultiplied(
        12U, 12U, 48U, patch_pixels);
    require(static_cast<bool>(patch), "nine-patch fixture must load");
    SurfaceMaterial material;
    material.fills = {MaterialFillLayer::nine_patch(
        patch.image, {12.0, 12.0}, {3.0, 3.0, 3.0, 3.0}, 1.5, 0.75)};
    panel->set_material(material);
    window.perform_layout();

    RichPainter painter;
    window.paint(painter, {0.0, 0.0, 30.0, 20.0});
    require(painter.image_regions.size() == 9U,
            "nine-patch material must retain exactly nine source crops");
    require(painter.image_regions.front().image == patch.image &&
                painter.image_regions.front().source == Rect{0.0, 0.0, 3.0, 3.0} &&
                painter.image_regions.front().destination == Rect{0.0, 0.0, 2.0, 2.0} &&
                painter.image_regions.front().opacity == 0.75 &&
                painter.image_regions[4].source == Rect{3.0, 3.0, 6.0, 6.0} &&
                painter.image_regions[4].destination == Rect{2.0, 2.0, 26.0, 16.0},
            "nine-patch must preserve source corners and stretch only the center bands");

    const std::vector<std::byte> tile_pixels(4U * 4U * 4U,
                                              std::byte{0xff});
    const ImageLoadResult tile = window.load_bgra32_premultiplied(
        4U, 4U, 16U, tile_pixels);
    require(static_cast<bool>(tile), "tile fixture must load");
    material.fills = {MaterialFillLayer::tiled_image(
        tile.image, {4.0, 4.0}, 1.0)};
    panel->set_requested_bounds({0.0, 0.0, 10.0, 6.0});
    panel->set_material(material);
    window.resize({10.0, 6.0});
    window.perform_layout();
    painter.image_regions.clear();
    painter.image_patterns.clear();
    window.paint(painter, {0.0, 0.0, 10.0, 6.0});
    require(painter.image_regions.empty() && painter.image_patterns.size() == 1U &&
                painter.image_patterns.front().image == tile.image &&
                painter.image_patterns.front().source_pixel_size == Size{4.0, 4.0} &&
                painter.image_patterns.front().destination == Rect{0.0, 0.0, 10.0, 6.0} &&
                painter.image_patterns.front().logical_tile_size == Size{4.0, 4.0} &&
                painter.image_patterns.front().wrap == ImagePatternWrap::tile,
            "tiled image material must retain one exact-period pattern command");

    SurfaceMaterial invalid = material;
    invalid.fills.front().image_slice = {3.0, 0.0, 3.0, 0.0};
    bool rejected{};
    try {
        panel->set_material(std::move(invalid));
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected && panel->material() == material,
            "non-nine-patch slice metadata must be rejected atomically");

    invalid = material;
    invalid.fills.front() = MaterialFillLayer::stretched_image(
        ImageId{UINT64_C(0xffffffffffffffff)}, {4.0, 4.0});
    rejected = false;
    try {
        panel->set_material(std::move(invalid));
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected && panel->material() == material,
            "an attached MaterialPanel must reject missing or stale-size image IDs");
}

void test_minimal_painter_fallbacks_are_bounded() {
    FallbackPainter painter;
    const std::vector<GradientStop> stops{
        {0.0, Color::rgba(0, 0, 0)},
        {1.0, Color::rgba(255, 255, 255)}};
    painter.clip_rounded_rect({0.0, 0.0, 100.0, 40.0}, 8.0);
    painter.fill_rounded_rect({0.0, 0.0, 100.0, 40.0}, 8.0,
                              Color::rgba(30, 40, 50));
    painter.fill_linear_gradient({0.0, 0.0, 100.0, 40.0}, {0.0, 0.0},
                                 {100.0, 0.0}, stops);
    painter.fill_linear_gradient_spread(
        {0.0, 0.0, 100.0, 40.0}, {0.0, 0.0}, {8.0, 8.0}, stops,
        GradientSpreadMode::repeat);
    painter.fill_radial_gradient({0.0, 0.0, 100.0, 40.0}, {50.0, 20.0},
                                 {50.0, 20.0}, stops);
    painter.draw_box_shadow({5.0, 5.0, 90.0, 30.0}, 6.0, {0.0, 2.0},
                            4.0, 0.0, Color::rgba(0, 0, 0, 80));
    require(painter.clips == 2U && painter.fills > 50U &&
                painter.fills < 1000U && painter.strokes > 0U &&
                painter.strokes < 100U,
            "minimal-host material fallbacks must be visible and bounded");
}

void test_shadow_outsets_participate_in_damage() {
    auto root = make_control<Panel>(StableId("material.damage.root"));
    root->set_requested_bounds({0.0, 0.0, 120.0, 100.0});
    auto panel = make_control<MaterialPanel>(StableId("material.damage.panel"));
    panel->set_requested_bounds({30.0, 25.0, 40.0, 30.0});
    SurfaceMaterial shadowed;
    shadowed.shadows = {{{4.0, 5.0}, 3.0, 1.0,
                         Color::rgba(0, 0, 0, 100)}};
    panel->set_material(shadowed);
    root->add_child(panel);
    Window window(root, {120.0, 100.0});
    window.perform_layout();
    static_cast<void>(window.take_damage());
    RichPainter painter;
    window.paint(painter, {0.0, 0.0, 120.0, 100.0});

    const Insets outsets = panel->visual_outsets();
    require(outsets == Insets{6.0, 5.0, 14.0, 15.0},
            "material shadow must publish conservative visual outsets");
    require(painter.shadows == 1U && painter.last_clip.x <= -6.0 &&
                painter.last_clip.y <= -5.0 &&
                painter.last_clip.width >= 60.0 &&
                painter.last_clip.height >= 50.0,
            "retained replay must not clip a decoration to arranged bounds");
    panel->set_material(SurfaceMaterial{});
    const Rect damage = window.take_damage().bounds();
    require(damage.x <= 24.0 && damage.y <= 20.0 &&
                damage.x + damage.width >= 84.0 &&
                damage.y + damage.height >= 70.0,
            "shrinking a decoration must damage its former visual extent");
}

} // namespace

int main() {
    try {
        test_material_validation_is_atomic();
        test_retained_replay_preserves_material_operations();
        test_repeating_material_survives_record_and_replay();
        test_image_materials_retain_crop_tile_and_nine_patch();
        test_minimal_painter_fallbacks_are_bounded();
        test_shadow_outsets_participate_in_damage();
        std::cout << "gui_forms_material_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_material_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
