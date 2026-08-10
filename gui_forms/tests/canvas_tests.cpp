#include "gui_forms/canvas.hpp"
#include "gui_forms/window.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
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

} // namespace

int main() {
    canvas_projects_bitmap_changes_as_local_window_damage();
    canvas_converts_rgba_storage_without_changing_document_truth();
    return 0;
}
