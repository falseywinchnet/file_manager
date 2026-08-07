#include "drawing_test_trace.hpp"
#include "gui_forms/drawing.hpp"

#include <atomic>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <thread>

namespace {

using namespace gui_drawing;

[[noreturn]] void fail(const char* expression, int line) {
    std::cerr << "drawing_core_tests:" << line << ": " << expression << '\n';
    std::exit(1);
}

#define CHECK(expression) do { if (!(expression)) fail(#expression, __LINE__); } while (false)

template <typename Exception, typename Operation>
void check_throws(Operation&& operation, int line) {
    try {
        operation();
    } catch (const Exception&) {
        return;
    } catch (...) {
        fail("unexpected exception type", line);
    }
    fail("expected exception", line);
}

#define CHECK_THROWS(type, expression) \
    check_throws<type>([&] { static_cast<void>(expression); }, __LINE__)

class FixedMetrics final : public TextMetricsProvider {
public:
    SizeF measure(std::string_view text, const FontSnapshot& font,
                  const StringFormatSnapshot&) const override {
        return {static_cast<double>(text.size()) * font.size / 2.0,
                font.size * 1.25};
    }
};

std::string record_reference_trace() {
    SolidBrush brush(Color::from_name("RED"));
    Pen pen(Color::from_html("#ffff00"), 2.0);
    pen.set_dash_style(DashStyle::dash);
    Font font("Lucida Grande", 12.0, FontStyle::bold);
    StringFormat format(4);
    format.set_alignment(StringAlignment::center);
    format.set_trimming(StringTrimming::ellipsis_character);

    GraphicsRecorder recorder;
    recorder.clear(Color::from_name("black"));
    const auto token = recorder.save();
    recorder.translate(10.0, 5.0);
    recorder.set_clip({0.0, 0.0, 100.0, 50.0});
    recorder.set_quality(SmoothingMode::anti_alias, InterpolationMode::bicubic,
                         PixelOffsetMode::half, CompositingMode::source_over,
                         CompositingQuality::high_quality);
    recorder.fill_rectangle(brush, {1.0, 2.0, 30.0, 20.0});
    recorder.draw_rectangle(pen, {1.0, 2.0, 30.0, 20.0});
    recorder.draw_line(pen, {1.0, 2.0}, {31.0, 22.0});
    recorder.draw_string("field \xce\xa9", font, brush, {4.0, 7.0}, format);
    recorder.restore(token);
    recorder.close();
    return recorder.deterministic_trace();
}

void geometry_and_color_are_deterministic() {
    RectI bounds{10, 20, 30, 40};
    CHECK(bounds.contains(PointI{10, 20}));
    CHECK(bounds.contains(PointI{39, 59}));
    CHECK(!bounds.contains(PointI{40, 59}));
    CHECK(RectI::intersection(bounds, {35, 10, 20, 30}) ==
          (RectI{35, 20, 5, 20}));
    CHECK(RectI::united(bounds, {0, 0, 5, 5}) == (RectI{0, 0, 40, 60}));

    PointI saturating{std::numeric_limits<std::int32_t>::max(), 0};
    saturating.offset(1, -1);
    CHECK(saturating.x == std::numeric_limits<std::int32_t>::max());
    CHECK(saturating.y == -1);

    RectF floating{1.0, 2.0, 4.0, 6.0};
    floating.inflate(1.0, 2.0);
    CHECK(floating == (RectF{0.0, 0.0, 6.0, 10.0}));
    CHECK_THROWS(std::invalid_argument,
                 floating.offset(std::numeric_limits<double>::infinity(), 0.0));

    CHECK(Color::from_name("DodgerBlue").argb() == UINT32_C(0xff1e90ff));
    CHECK(Color::from_name("missing").is_empty());
    CHECK(Color::from_html("#abc").argb() == UINT32_C(0xffaabbcc));
    CHECK(Color::from_html("#80402010").alpha() == 0x80U);
    CHECK(std::abs(Color::from_rgb(0, 128, 255).brightness() - 0.5) < 0.001);
    CHECK(default_system_palette().control.argb() == UINT32_C(0xfff0f0f0));
}

void transforms_resources_and_metrics_obey_contracts() {
    const Matrix transform = Matrix::translation(10.0, 20.0)
        .followed_by(Matrix::rotation_at(90.0, {10.0, 20.0}));
    CHECK(transform.finite());
    const PointF rotated = Matrix::rotation_at(90.0, {1.0, 1.0})
        .transform({2.0, 1.0});
    CHECK(std::abs(rotated.x - 1.0) < 1e-12);
    CHECK(std::abs(rotated.y - 2.0) < 1e-12);

    SolidBrush brush(Color::from_name("red"));
    Pen pen(brush, 1.5);
    const double custom_dash[] = {1.0, 2.0, 3.0};
    pen.set_dash_pattern(custom_dash);
    CHECK(pen.snapshot().dash_style == DashStyle::custom);
    CHECK(pen.snapshot().dash_pattern.size() == 3U);
    CHECK_THROWS(std::invalid_argument, pen.set_width(0.0));

    Font font("Lucida Grande", 12.0, FontStyle::regular);
    StringFormat format;
    GraphicsRecorder recorder;
    const SizeF measured = recorder.measure_string("abcd", font, format,
                                                   FixedMetrics{});
    CHECK(measured == (SizeF{24.0, 15.0}));
    CHECK(font.deterministic_height() == 15);

    recorder.set_clip({0.0, 0.0, 10.0, 10.0});
    CHECK(recorder.is_visible({9.99, 9.99}));
    CHECK(!recorder.is_visible({10.0, 10.0}));
    const auto token = recorder.save();
    recorder.restore(token);
    CHECK_THROWS(std::invalid_argument, recorder.restore(token));

    recorder.draw_line(pen, {0.0, 0.0}, {1.0, 1.0});
    pen.set_width(7.0);
    CHECK(recorder.commands().back().pen.width == 1.5);

    brush.dispose();
    brush.dispose();
    CHECK(brush.state() == ObjectState::disposed);
    CHECK_THROWS(std::logic_error, brush.color());

    std::atomic_bool rejected{false};
    std::thread foreign([&] {
        try {
            static_cast<void>(pen.snapshot());
        } catch (const std::logic_error&) {
            rejected = true;
        }
    });
    foreign.join();
    CHECK(rejected.load());

    recorder.close();
    CHECK_THROWS(std::logic_error, recorder.clear(Color::from_name("black")));
}

void extended_vocabulary_snapshots_resources() {
    GraphicsPath path(FillMode::winding);
    path.start_figure();
    path.add_line({2.0, 1.0}, {2.0, 9.0});
    path.add_rectangle({-1.0, -2.0, 4.0, 5.0});
    path.add_ellipse({10.0, 20.0, 5.0, 5.0});
    path.add_arc({20.0, 10.0, 8.0, 6.0}, 0.0, 90.0);
    path.close_figure();
    CHECK(path.bounds() == (RectF{-1.0, -2.0, 29.0, 27.0}));
    CHECK(path.is_visible({0.0, 0.0}));
    CHECK(!path.is_visible({100.0, 100.0}));
    CHECK(path.path_points().size() == 23U);
    auto copied_path = path.clone();
    copied_path->transform(Matrix::translation(4.0, 5.0));
    CHECK(copied_path->bounds() == (RectF{3.0, 3.0, 29.0, 27.0}));
    GraphicsPath appended;
    appended.add_line({-10.0, -10.0}, {-5.0, -5.0});
    appended.add_path(*copied_path, true);
    CHECK(appended.path_points().size() == 25U);
    Region region(RectF{0, 0, 10, 10});
    region.exclude({2, 2, 4, 4});
    CHECK(region.is_visible({1, 1}));
    CHECK(!region.is_visible({3, 3}));
    region.unite(path);
    CHECK(region.is_visible({12, 22}));
    CHECK(region.bounds() == (RectF{-1, -2, 29, 27}));
    Region path_region(path);
    CHECK(path_region.is_visible({12, 22}));

    SolidBrush brush(Color::from_name("blue"));
    Pen pen(Color::from_name("white"), 3.0);
    ImageReference image(42, 640, 480, PixelFormat::bgra32_premultiplied, 7);
    ImageAttributes attributes;
    std::array<double, 25> matrix{};
    matrix[0] = matrix[6] = matrix[12] = matrix[18] = matrix[24] = 1.0;
    matrix[18] = 0.5;
    attributes.set_color_matrix(matrix);
    const ImageAttributesSnapshot::ColorRemap remaps[] = {
        {Color::from_name("blue"), Color::from_name("yellow")},
    };
    attributes.set_remap_table(remaps);

    GraphicsRecorder recorder;
    recorder.draw_ellipse(pen, {1, 2, 3, 4});
    recorder.fill_ellipse(brush, {1, 2, 3, 4});
    const PointF polygon[] = {{0, 0}, {10, 0}, {5, 8}};
    recorder.fill_polygon(brush, polygon, FillMode::winding);
    recorder.draw_path(pen, path);
    recorder.fill_path(brush, path);
    recorder.draw_image(image, {0, 0, 320, 240}, {0, 0, 640, 480}, attributes);
    path.reset();
    attributes.reset_color_matrix();
    CHECK(recorder.commands()[3].path.elements.size() == 6U);
    CHECK(recorder.commands()[5].image.stable_id == 42U);
    CHECK(recorder.commands()[5].image_attributes.has_color_matrix);
    CHECK(recorder.commands()[5].image_attributes.remap_table.size() == 1U);
    recorder.close();
    const std::string trace = recorder.deterministic_trace();
    CHECK(trace.find("3|draw_path|path=1;0;1,2,1,2,9") != std::string::npos);
    CHECK(trace.find("5|draw_image|image=42,640,480,0,7") != std::string::npos);

    CHECK_THROWS(std::invalid_argument,
                 ImageReference(0, 1, 1, PixelFormat::bgra32_premultiplied));
    std::array<double, 25> invalid = matrix;
    invalid[3] = std::numeric_limits<double>::quiet_NaN();
    CHECK_THROWS(std::invalid_argument, attributes.set_color_matrix(invalid));
    const ImageAttributesSnapshot::ColorRemap duplicates[] = {
        {Color::from_name("red"), Color::from_name("white")},
        {Color::from_name("red"), Color::from_name("black")},
    };
    CHECK_THROWS(std::invalid_argument, attributes.set_remap_table(duplicates));

    HatchBrush hatch(HatchStyle::diagonal_cross, Color::from_name("white"),
                     Color::from_name("black"));
    CHECK(hatch.snapshot().kind == BrushKind::hatch);
    LinearGradientBrush linear({0, 0, 100, 20}, Color::from_name("red"),
                               Color::from_name("blue"), 15.0,
                               WrapMode::tile_flip_xy);
    const ColorBlend blend{{Color::from_name("red"), Color::from_name("white"),
                            Color::from_name("blue")},
                           {0.0, 0.5, 1.0}};
    linear.set_interpolation_colors(blend);
    CHECK(linear.snapshot().colors.size() == 3U);
    const double factors[] = {0.0, 0.25, 1.0};
    const double positions[] = {0.0, 0.75, 1.0};
    linear.set_blend(factors, positions);
    CHECK(linear.snapshot().colors[1].red() > linear.snapshot().colors[1].blue());
    const PointF gradient_shape[] = {{0, 0}, {20, 0}, {10, 20}};
    PathGradientBrush radial(gradient_shape);
    radial.set_center_color(Color::from_name("white"));
    const Color surrounds[] = {Color::from_name("black")};
    radial.set_surround_colors(surrounds);
    CHECK(radial.snapshot().points.size() == 3U);
    CHECK_THROWS(std::invalid_argument,
                 linear.set_interpolation_colors(
                     ColorBlend{{Color::from_name("red"), Color::from_name("blue")},
                                {0.1, 1.0}}));
}

void curve_polygon_and_pie_paths_are_retained_and_bounded() {
    GraphicsPath path;
    path.set_fill_mode(FillMode::winding);
    path.start_figure();
    path.add_quadratic({0, 10}, {5, 0}, {10, 10});
    path.add_bezier({10, 10}, {12, 20}, {18, 20}, {20, 10});
    const PointF continuation[] = {
        {20, 10}, {22, 0}, {28, 0}, {30, 10},
        {30, 10}, {32, 20}, {38, 20}, {40, 10},
    };
    // Two independent calls make the canonical 4 + 3n rule explicit.
    path.add_beziers(std::span<const PointF>(continuation, 4U));
    const PointF second_curve[] = {
        {30, 10}, {32, 20}, {38, 20}, {40, 10},
    };
    path.add_beziers(second_curve);
    path.close_figure();
    const std::size_t before_invalid = path.snapshot().elements.size();
    const PointF invalid[] = {{0, 0}, {1, 1}, {2, 2}};
    CHECK_THROWS(std::invalid_argument, path.add_beziers(invalid));
    CHECK(path.snapshot().elements.size() == before_invalid);
    CHECK(path.fill_mode() == FillMode::winding);
    CHECK(path.bounds() == (RectF{0, 0, 40, 20}));

    GraphicsPath polygon;
    const PointF triangle[] = {{2, 2}, {18, 2}, {10, 18}};
    polygon.add_polygon(triangle);
    CHECK(polygon.is_visible({10, 8}));
    CHECK(!polygon.is_visible({1, 1}));
    CHECK_THROWS(std::invalid_argument,
                 polygon.add_polygon(std::span<const PointF>(triangle, 2U)));

    GraphicsPath pie;
    pie.add_pie({0, 0, 20, 20}, 0.0, 90.0);
    CHECK(pie.is_visible({13, 13}));
    CHECK(!pie.is_visible({3, 3}));
    auto transformed = pie.clone();
    transformed->transform(Matrix::translation(5, 7));
    CHECK(transformed->bounds() == (RectF{5, 7, 20, 20}));
}

void bitmap_storage_and_leases_are_generation_safe() {
    Bitmap bitmap(2, 2);
    CHECK(bitmap.width() == 2U && bitmap.height() == 2U);
    CHECK(bitmap.generation() == 1U);
    bitmap.set_pixel(0, 0, Color::from_argb(128, 255, 0, 0));
    CHECK(bitmap.generation() == 2U);
    CHECK(bitmap.get_pixel(0, 0) == Color::from_argb(128, 255, 0, 0));
    CHECK_THROWS(std::out_of_range, bitmap.get_pixel(2, 0));

    const ImageSnapshot retained = bitmap.snapshot();
    CHECK(retained.has_pixels() && retained.row_bytes() == 8U);
    CHECK(retained.pixels().size() == 16U);
    CHECK(std::to_integer<std::uint8_t>(retained.pixels()[2]) == 128U);
    bitmap.set_pixel(0, 0, Color::from_name("blue"));
    CHECK(bitmap.generation() == 3U);
    CHECK(std::to_integer<std::uint8_t>(retained.pixels()[2]) == 128U);
    CHECK(bitmap.get_pixel(0, 0).argb() == Color::from_name("blue").argb());

    const BitmapLockView read = bitmap.lock(BitmapLockMode::read);
    CHECK(read.data != nullptr && read.writable_data == nullptr);
    CHECK_THROWS(std::logic_error, bitmap.snapshot());
    bitmap.unlock(read.token);
    CHECK_THROWS(std::invalid_argument, bitmap.unlock(read.token));

    const BitmapLockView write = bitmap.lock(BitmapLockMode::write);
    CHECK(write.writable_data != nullptr);
    write.writable_data[0] = std::byte{0};
    write.writable_data[1] = std::byte{255};
    write.writable_data[2] = std::byte{0};
    write.writable_data[3] = std::byte{255};
    bitmap.unlock(write.token);
    CHECK(bitmap.generation() == 4U);
    CHECK(bitmap.get_pixel(0, 0).argb() == Color::from_name("lime").argb());

    bitmap.set_pixel(1, 0, Color::from_name("red"));
    bitmap.set_pixel(0, 1, Color::from_name("blue"));
    bitmap.set_pixel(1, 1, Color::from_name("white"));
    auto clone = bitmap.clone({1, 0, 1, 2});
    CHECK(clone->width() == 1U && clone->height() == 2U);
    CHECK(clone->get_pixel(0, 0).argb() == Color::from_name("red").argb());
    clone->make_transparent(Color::from_name("red"));
    CHECK(clone->get_pixel(0, 0).alpha() == 0U);

    auto thumbnail = bitmap.thumbnail(1, 1);
    CHECK(thumbnail->get_pixel(0, 0).argb() == Color::from_name("lime").argb());

    ImageAttributes attributes;
    std::array<double, 25> matrix{};
    matrix[0] = matrix[6] = matrix[12] = matrix[18] = matrix[24] = 1.0;
    matrix[18] = 0.5;
    auto adjusted = bitmap.adjusted(attributes);
    CHECK(adjusted->get_pixel(0, 1).argb() == Color::from_name("blue").argb());
    attributes.set_color_matrix(matrix);
    const ImageAttributesSnapshot::ColorRemap remap[] = {
        {Color::from_name("blue"), Color::from_name("red")},
    };
    attributes.set_remap_table(remap);
    adjusted = bitmap.adjusted(attributes);
    CHECK(adjusted->get_pixel(0, 1).alpha() == 128U);
    CHECK(adjusted->get_pixel(0, 1).red() == 255U);

    GraphicsRecorder recorder;
    recorder.draw_image(bitmap, {0, 0, 2, 2}, {0, 0, 2, 2}, attributes);
    const std::uint64_t submitted_generation =
        recorder.commands().front().image.generation;
    bitmap.set_pixel(0, 0, Color::from_name("black"));
    CHECK(recorder.commands().front().image.generation == submitted_generation);
    CHECK(recorder.commands().front().image.pixels().data() !=
          bitmap.snapshot().pixels().data());

    TextureBrush texture(bitmap, WrapMode::tile_flip_x);
    texture.translate_transform(3.0, 4.0);
    texture.scale_transform(2.0, 1.5);
    const BrushSnapshot texture_snapshot = texture.snapshot();
    CHECK(texture_snapshot.kind == BrushKind::texture);
    CHECK(texture_snapshot.image.has_pixels());
    CHECK(texture_snapshot.wrap_mode == WrapMode::tile_flip_x);
    const PointF transformed_origin =
        texture_snapshot.transform.transform({0.0, 0.0});
    CHECK(transformed_origin.x == 6.0 && transformed_origin.y == 6.0);
    auto copied_texture = texture.clone();
    texture.reset_transform();
    CHECK(copied_texture->snapshot().transform == texture_snapshot.transform);
    GraphicsRecorder texture_recorder;
    texture_recorder.fill_rectangle(*copied_texture, {0.0, 0.0, 12.0, 8.0});
    CHECK(texture_recorder.commands().front().brush.kind == BrushKind::texture);
    CHECK(texture_recorder.deterministic_trace().find("image=") !=
          std::string::npos);
    CHECK_THROWS(std::invalid_argument, texture.scale_transform(0.0, 1.0));

    CHECK_THROWS(std::invalid_argument, Bitmap(0, 1));
    CHECK_THROWS(std::length_error, Bitmap(32768, 32768));

    std::atomic_bool rejected{false};
    std::thread foreign([&] {
        try {
            static_cast<void>(bitmap.get_pixel(0, 0));
        } catch (const std::logic_error&) {
            rejected = true;
        }
    });
    foreign.join();
    CHECK(rejected.load());
}

} // namespace

int main() {
    geometry_and_color_are_deterministic();
    transforms_resources_and_metrics_obey_contracts();
    extended_vocabulary_snapshots_resources();
    curve_polygon_and_pie_paths_are_retained_and_bounded();
    bitmap_storage_and_leases_are_generation_safe();
    const std::string trace = record_reference_trace();
    CHECK(trace == gui_drawing_expected_trace);
    CHECK(record_reference_trace() == trace);
    return 0;
}
