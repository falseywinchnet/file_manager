#include "gui_forms/canvas.hpp"
#include "gui_forms/window.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <chrono>
#include <stdexcept>
#include <string_view>

namespace {

using namespace gui_forms;

[[noreturn]] void fail(const char* expression, int line) {
    std::cerr << "canvas_tests:" << line << ": " << expression << '\n';
    std::exit(1);
}

#define CHECK(expression) do { if (!(expression)) fail(#expression, __LINE__); } while (false)

class CapturePainter final : public Painter {
public:
    void save() override {}
    void restore() override {}
    void translate(Point) override {}
    void clip_rect(Rect rect) override { last_clip = rect; }
    void fill_rect(Rect, Color) override { ++fills; }
    void stroke_rect(Rect, Color, double) override {}
    void draw_line(Point, Point, Color, double) override {}
    void draw_text_utf8(Point, std::string_view, FontSpec, Color) override {}
    void draw_image(ImageId, Rect destination, double) override {
        ++images;
        last_destination = destination;
    }
    void draw_image_region_sampled(ImageId image, Rect source,
                                   Rect destination, ImageSampling sampling,
                                   double) override {
        CHECK(image.value != 0U);
        ++sampled_images;
        last_source = source;
        last_destination = destination;
        last_sampling = sampling;
    }

    std::uint64_t fills{};
    std::uint64_t images{};
    std::uint64_t sampled_images{};
    Rect last_clip{};
    Rect last_source{};
    Rect last_destination{};
    ImageSampling last_sampling{ImageSampling::linear};
};

// A consumer canvas owns tools and overlays while the base keeps bitmap lifetime.
class ToolCanvas final : public RasterCanvas {
public:
    explicit ToolCanvas(StableId id) : RasterCanvas(std::move(id)) {}
    void on_pointer(PointerEvent& event) override {
        ++pointer_events;
        if (event.action == PointerAction::down) set_pointer_capture(true);
        if (event.action == PointerAction::up) set_pointer_capture(false);
        event.handled = true;
    }
    void on_paint_overlay(Painter& painter, Rect) override {
        ++overlays;
        painter.stroke_rect(bitmap_to_client({1, 1, 2, 2}), Color::rgba(0, 100, 200), 1);
    }
    unsigned pointer_events{};
    unsigned overlays{};
};
void derived_canvas_keeps_routing_presentation_and_resource_lifetime() {
    const std::shared_ptr<Control> root = make_control<Control>(StableId("tools.root"));
    const std::shared_ptr<ToolCanvas> canvas = make_control<ToolCanvas>(StableId("tools.canvas"));
    (*canvas).set_requested_bounds({10, 10, 40, 40});
    (*canvas).set_bitmap(std::make_shared<gui_drawing::Bitmap>(4, 4));
    (*canvas).set_view(2, {0, 0});
    (*root).add_child(canvas);
    Window window(root, {100, 100});
    window.perform_layout();
    CapturePainter painter;
    CHECK(window.paint(painter).has_value());
    CHECK(painter.sampled_images == 1 && (*canvas).overlays == 1);
    CHECK(window.dispatch_pointer({PointerAction::down, PointerButton::primary, {12, 12}}));
    CHECK((*canvas).has_pointer_capture());
    CHECK(window.dispatch_pointer({PointerAction::up, PointerButton::primary, {90, 90}}));
    CHECK((*canvas).pointer_events == 2 && !(*canvas).has_pointer_capture());
    CHECK(window.image_resource_snapshot().resource_count == 1);
    (*canvas).dispose();
    CHECK(window.image_resource_snapshot().resource_count == 0);
}

void canvas_projects_bitmap_changes_as_local_window_damage() {
    std::shared_ptr<gui_forms::Control> root = make_control<Control>(StableId("canvas.root"));
    std::shared_ptr<gui_forms::RasterCanvas> canvas = make_control<RasterCanvas>(StableId("canvas.surface"));
    (*root).set_requested_bounds({0, 0, 120, 90});
    (*canvas).set_requested_bounds({10, 20, 80, 60});
    (*canvas).set_zoom(2.0);
    (*canvas).set_transparency_cell_size(12.0);
    std::shared_ptr<gui_drawing::Bitmap> bitmap = std::make_shared<gui_drawing::Bitmap>(4, 4);
    (*canvas).set_bitmap(bitmap);
    (*root).add_child(canvas);

    Window window(root, {120, 90});
    window.perform_layout();
    CHECK(window.image_resource_snapshot().resource_count == 1U);
    CHECK((*canvas).presented_generation() == 1U);
    CHECK((*canvas).transparency_cell_size() == 12.0);
    CHECK((*canvas).client_to_bitmap({4, 6}) ==
          (gui_drawing::PointF{2, 3}));
    CHECK((*canvas).bitmap_to_client({1, 1, 2, 2}) ==
          (Rect{2, 2, 4, 4}));

    CapturePainter painter;
    CHECK(window.paint(painter).has_value());
    CHECK(painter.sampled_images == 1U);
    CHECK(painter.last_sampling == ImageSampling::nearest);
    CHECK(painter.last_source == (Rect{0, 0, 4, 4}));
    CHECK(painter.last_destination == (Rect{0, 0, 8, 8}));
    static_cast<void>(window.take_damage());

    gui_drawing::BitmapEditView edit = (*bitmap).begin_edit({1, 1, 2, 2});
    for (std::int32_t row = 0; row < 2; ++row) {
        for (std::int32_t column = 0; column < 2; ++column) {
            std::byte* pixel = edit.writable_data +
                static_cast<std::size_t>(row) * edit.row_bytes +
                static_cast<std::size_t>(column) * 4U;
            pixel[0] = std::byte{0};
            pixel[1] = std::byte{0};
            pixel[2] = std::byte{255};
            pixel[3] = std::byte{255};
        }
    }
    CHECK((*bitmap).commit_edit(edit.token) == 2U);
    CHECK((*canvas).synchronize_bitmap());
    CHECK((*canvas).presented_generation() == 2U);
    const DamageRegion damage = window.take_damage();
    CHECK(damage.rectangle_count() == 1U);
    CHECK(damage.bounds() == (Rect{12, 22, 4, 4}));

    edit = (*bitmap).begin_edit({1, 1, 1, 1});
    edit.writable_data[0] = std::byte{255};
    (*bitmap).cancel_edit(edit.token);
    CHECK((*canvas).synchronize_bitmap());
    CHECK(window.take_damage().empty());

    const Control::Ptr detached = (*root).remove_child((*canvas).runtime_id());
    CHECK(detached == canvas);
    CHECK(window.image_resource_snapshot().resource_count == 0U);
}

void canvas_converts_rgba_storage_without_changing_document_truth() {
    std::shared_ptr<gui_forms::Control> root = make_control<Control>(StableId("canvas.rgba.root"));
    std::shared_ptr<gui_forms::RasterCanvas> canvas = make_control<RasterCanvas>(StableId("canvas.rgba"));
    (*root).set_requested_bounds({0, 0, 20, 20});
    (*canvas).set_requested_bounds({0, 0, 20, 20});
    std::shared_ptr<gui_drawing::Bitmap> bitmap = std::make_shared<gui_drawing::Bitmap>(
        1, 1, gui_drawing::PixelFormat::rgba32_premultiplied);
    (*bitmap).set_pixel(0, 0, gui_drawing::Color::from_rgb(255, 0, 0));
    (*canvas).set_bitmap(bitmap);
    (*root).add_child(canvas);
    Window window(root, {20, 20});
    window.perform_layout();

    const std::vector<ImageId> ids = window.image_resources().image_ids();
    CHECK(ids.size() == 1U);
    const std::optional<ImageResourceView> resource = window.image_resources().find(ids.front());
    CHECK(resource && (*resource).encoded.size() == 4U);
    CHECK((*resource).encoded[0] == std::byte{0});
    CHECK((*resource).encoded[1] == std::byte{0});
    CHECK((*resource).encoded[2] == std::byte{255});
    CHECK((*resource).encoded[3] == std::byte{255});
    CHECK((*bitmap).get_pixel(0, 0).argb() == UINT32_C(0xffff0000));

    CHECK(window.remove_image(ids.front()));
    (*bitmap).set_pixel(0, 0, gui_drawing::Color::from_name("blue"));
    CHECK((*canvas).synchronize_bitmap());
    CHECK((*canvas).presented_generation() == (*bitmap).generation());
    CHECK(window.image_resource_snapshot().resource_count == 1U);
}

void tiled_updates_include_sampling_gutters_and_recover_resources() {
    const std::shared_ptr<RasterCanvas> canvas = make_control<RasterCanvas>(StableId("tiles"));
    const std::shared_ptr<gui_drawing::Bitmap> bitmap = std::make_shared<gui_drawing::Bitmap>(1025, 1025);
    (*canvas).set_bitmap(bitmap);
    Window window(canvas, {200, 200});
    window.perform_layout();
    std::vector<ImageId> ids = window.image_resources().image_ids();
    CHECK(ids.size() == 9);
    std::vector<std::uint64_t> revisions;
    revisions.reserve(ids.size());
    for (const ImageId id : ids) {
        const std::optional<ImageResourceView> resource = window.image_resources().find(id);
        CHECK(resource.has_value());
        revisions.push_back((*resource).content_hash);
    }
    (*bitmap).set_pixel(511, 511, gui_drawing::Color::from_name("red"));
    CHECK((*canvas).synchronize_bitmap());
    ids = window.image_resources().image_ids();
    unsigned changed{};
    for (std::size_t index = 0; index < ids.size(); ++index) {
        const ImageResourceView resource = *window.image_resources().find(ids[index]);
        if (resource.content_hash != revisions[index]) ++changed;
        CHECK(resource.metadata.width <= 514 && resource.metadata.height <= 514);
        revisions[index] = resource.content_hash;
    }
    CHECK(changed == 4); // Four tiles contain this corner through their gutters.
    (*bitmap).set_pixel(200, 200, gui_drawing::Color::from_name("blue"));
    CHECK((*canvas).synchronize_bitmap());
    changed = 0;
    ids = window.image_resources().image_ids();
    for (std::size_t index = 0; index < ids.size(); ++index) {
        const std::optional<ImageResourceView> resource = window.image_resources().find(ids[index]);
        CHECK(resource.has_value());
        if ((*resource).content_hash != revisions[index]) ++changed;
    }
    CHECK(changed == 1);
    CHECK(window.remove_image(ids[3]));
    CHECK((*canvas).synchronize_bitmap()); // No additional document mutation is needed.
    CHECK(window.image_resource_snapshot().resource_count == 9);
    (*canvas).clear_bitmap();
    CHECK(window.image_resource_snapshot().resource_count == 0);
    (*canvas).set_bitmap(bitmap);
    CHECK(window.image_resource_snapshot().resource_count == 9);
    (*canvas).dispose();
    CHECK(window.image_resource_snapshot().resource_count == 0);
}

void explicit_window_quota_rolls_back_partial_tile_publication() {
    ImageRegistryLimits limits;
    limits.maximum_total_encoded_bytes = 8000;
    const std::shared_ptr<RasterCanvas> canvas = make_control<RasterCanvas>(StableId("quota"));
    Window window(canvas, {100, 100}, limits);
    (*canvas).set_bitmap(std::make_shared<gui_drawing::Bitmap>(1025, 3));
    CHECK((*canvas).last_resource_error() == ImageResourceError::registry_encoded_limit_exceeded);
    CHECK(window.image_resource_snapshot().resource_count == 0);
    (*canvas).set_bitmap(std::make_shared<gui_drawing::Bitmap>(1, 1));
    CHECK((*canvas).last_resource_error() == ImageResourceError::none);
    CHECK(window.image_resource_snapshot().resource_count == 1);
}

void hundred_megapixel_publication_probe() {
    const std::chrono::steady_clock::time_point started = std::chrono::steady_clock::now();
    const std::shared_ptr<gui_drawing::Bitmap> bitmap = std::make_shared<gui_drawing::Bitmap>(10000, 10000);
    const std::shared_ptr<RasterCanvas> canvas = make_control<RasterCanvas>(StableId("large"));
    ImageRegistryLimits limits;
    limits.maximum_total_encoded_bytes = 512ULL * 1024ULL * 1024ULL;
    limits.maximum_total_decoded_bytes = 512ULL * 1024ULL * 1024ULL;
    (*canvas).set_bitmap(bitmap);
    Window window(canvas, {1200, 800}, limits);
    window.perform_layout();
    const std::chrono::steady_clock::time_point published = std::chrono::steady_clock::now();
    std::vector<ImageId> ids = window.image_resources().image_ids();
    CHECK(ids.size() == 400);
    std::vector<std::uint64_t> revisions;
    revisions.reserve(ids.size());
    for (const ImageId id : ids) {
        const std::optional<ImageResourceView> resource = window.image_resources().find(id);
        CHECK(resource.has_value());
        revisions.push_back((*resource).content_hash);
    }
    (*bitmap).set_pixel(333, 333, gui_drawing::Color::from_name("red"));
    CHECK((*canvas).synchronize_bitmap());
    const std::chrono::steady_clock::time_point updated = std::chrono::steady_clock::now();
    ids = window.image_resources().image_ids();
    unsigned changed{};
    std::uint64_t revised_bytes{};
    for (std::size_t index = 0; index < ids.size(); ++index) {
        const ImageResourceView resource = *window.image_resources().find(ids[index]);
        if (resource.content_hash != revisions[index]) { ++changed; revised_bytes += resource.encoded.size(); }
    }
    CHECK(changed == 1 && revised_bytes <= 514U * 514U * 4U);
    std::cout << "100M pixels, tiles=" << ids.size() << ", registry_bytes="
        << window.image_resource_snapshot().encoded_bytes << ", revised_cache_bytes=" << revised_bytes
        << ", initial_ms=" << std::chrono::duration<double, std::milli>(published - started).count()
        << ", one_pixel_ms=" << std::chrono::duration<double, std::milli>(updated - published).count() << '\n';
}

} // namespace

int main(int argc, char** argv) {
    derived_canvas_keeps_routing_presentation_and_resource_lifetime();
    canvas_projects_bitmap_changes_as_local_window_damage();
    canvas_converts_rgba_storage_without_changing_document_truth();
    tiled_updates_include_sampling_gutters_and_recover_resources();
    explicit_window_quota_rolls_back_partial_tile_publication();
    if (argc > 1 && std::string_view(argv[1]) == "--large") hundred_megapixel_publication_probe();
    return 0;
}
