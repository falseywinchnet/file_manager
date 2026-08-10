#include "drawing_skia.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <span>
#include <string>
#include <thread>
#include <vector>

namespace {

using namespace gui_drawing;
using namespace gui_drawing::render;

[[noreturn]] void fail(const char* expression, int line) {
    std::cerr << "drawing_skia_tests:" << line << ": " << expression << '\n';
    std::exit(1);
}

#define CHECK(expression) do { if (!(expression)) fail(#expression, __LINE__); } while (false)

std::vector<std::byte> read_file(const char* path) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    CHECK(static_cast<bool>(input));
    const std::streamsize size = input.tellg();
    CHECK(size > 0);
    std::vector<std::byte> bytes(static_cast<std::size_t>(size));
    input.seekg(0);
    input.read(reinterpret_cast<char*>(bytes.data()), size);
    CHECK(static_cast<bool>(input));
    return bytes;
}

std::uint64_t checksum(const ImageSnapshot& image) {
    std::uint64_t value = UINT64_C(1469598103934665603);
    for (const std::byte byte : image.pixels()) {
        value ^= std::to_integer<std::uint8_t>(byte);
        value *= UINT64_C(1099511628211);
    }
    return value;
}

class ExecuteSkiaOffThread final {
public:
    ExecuteSkiaOffThread(SkiaExecutor& executor,
                         const GraphicsRecorder& recorder, Bitmap& target,
                         std::atomic<RasterError>& result)
        : executor_(executor), recorder_(recorder), target_(target),
          result_(result) {}

    void operator()() const {
        result_ = executor_.execute(recorder_, target_).error;
    }

private:
    SkiaExecutor& executor_;
    const GraphicsRecorder& recorder_;
    Bitmap& target_;
    std::atomic<RasterError>& result_;
};

void command_execution_and_png_round_trip() {
    SkiaExecutor executor;
    const std::vector<std::byte> font = read_file(GUI_DRAWING_TEST_FONT);
    CHECK(executor.register_typeface("Portsmouth Rapids", font));

    Bitmap target(64, 48);
    SolidBrush red(Color::from_name("red"));
    SolidBrush green(Color::from_name("lime"));
    Pen blue(Color::from_name("blue"), 2.0);
    Font title("Portsmouth Rapids", 12.0, FontStyle::bold);
    StringFormat format;
    GraphicsPath path(FillMode::winding);
    path.add_rectangle({42, 4, 12, 10});
    path.add_ellipse({45, 7, 8, 8});

    GraphicsRecorder recorder;
    recorder.clear(Color::from_name("white"));
    recorder.fill_rectangle(red, {2, 2, 12, 10});
    recorder.draw_ellipse(blue, {18, 2, 14, 12});
    const PointF triangle[] = {{4, 20}, {16, 20}, {10, 32}};
    recorder.fill_polygon(green, triangle, FillMode::winding);
    recorder.draw_path(blue, path);
    recorder.draw_string("M11f", title, red, {34, 34}, format);
    const RasterResult rendered = executor.execute(recorder, target);
    CHECK(rendered && rendered.commands_executed == recorder.commands().size());
    CHECK(target.get_pixel(3, 3).argb() == UINT32_C(0xffff0000));
    CHECK(target.get_pixel(0, 0).argb() == UINT32_C(0xffffffff));
    CHECK(target.get_pixel(10, 24).green() > 200U);
    const std::uint64_t first_checksum = checksum(target.snapshot());
    CHECK(first_checksum != UINT64_C(1469598103934665603));

    RasterError encode_error = RasterError::internal_error;
    const std::vector<std::byte> png = executor.encode_png(target, &encode_error);
    CHECK(encode_error == RasterError::none && png.size() > 32U);
    constexpr std::array<std::uint8_t, 8> signature{
        0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
    for (std::size_t index = 0; index < signature.size(); ++index) {
        CHECK(std::to_integer<std::uint8_t>(png[index]) == signature[index]);
    }
    const std::vector<std::byte> repeated = executor.encode_png(target, &encode_error);
    CHECK(repeated == png);

    DecodeResult decoded = executor.decode_png(png);
    CHECK(decoded && (*decoded.bitmap).width() == 64U &&
          (*decoded.bitmap).height() == 48U);
    CHECK(checksum((*decoded.bitmap).snapshot()) == first_checksum);

    std::vector<std::byte> malformed = png;
    malformed[0] = std::byte{0};
    CHECK(executor.decode_png(malformed).error == RasterError::decode_failed);
    PngCodecLimits tiny;
    tiny.maximum_encoded_bytes = 16;
    CHECK(executor.decode_png(png, tiny).error == RasterError::encoded_limit_exceeded);
    CHECK(executor.encode_png(target, &encode_error, tiny).empty());
    CHECK(encode_error == RasterError::encoded_limit_exceeded);
}

void image_attributes_clip_and_snapshot_execution() {
    SkiaExecutor executor;
    Bitmap source(2, 2);
    source.set_pixel(0, 0, Color::from_name("blue"));
    source.set_pixel(1, 0, Color::from_name("red"));
    source.set_pixel(0, 1, Color::from_name("white"));
    source.set_pixel(1, 1, Color::from_name("black"));
    ImageAttributes attributes;
    std::array<double, 25> matrix{};
    matrix[0] = matrix[6] = matrix[12] = matrix[18] = matrix[24] = 1.0;
    matrix[18] = 0.5;
    attributes.set_color_matrix(matrix);
    const ImageAttributesSnapshot::ColorRemap remap[] = {
        {Color::from_name("blue"), Color::from_name("yellow")},
    };
    attributes.set_remap_table(remap);

    GraphicsRecorder recorder;
    recorder.set_quality(SmoothingMode::anti_alias, InterpolationMode::nearest,
                         PixelOffsetMode::none, CompositingMode::source_copy,
                         CompositingQuality::high_quality);
    recorder.translate(4, 3);
    recorder.set_clip({0, 0, 4, 4});
    recorder.draw_image(source, {0, 0, 4, 4}, {0, 0, 2, 2}, attributes);
    source.set_pixel(0, 0, Color::from_name("lime"));

    Bitmap target(12, 10);
    CHECK(executor.execute(recorder, target));
    const Color retained_yellow = target.get_pixel(4, 3);
    CHECK(retained_yellow.red() > 240U && retained_yellow.green() > 240U &&
          retained_yellow.blue() < 8U && retained_yellow.alpha() == 128U);
    CHECK(target.get_pixel(8, 3).alpha() == 0U);

    GraphicsRecorder missing;
    ImageReference logical_only(99, 2, 2, PixelFormat::bgra32_premultiplied);
    missing.clear(Color::from_name("red"));
    missing.draw_image(logical_only, {0, 0, 2, 2}, {0, 0, 2, 2}, attributes);
    const ImageSnapshot before_failed_execution = target.snapshot();
    const std::uint64_t before_failed_checksum = checksum(before_failed_execution);
    const std::uint64_t before_failed_generation = target.generation();
    CHECK(executor.execute(missing, target).error == RasterError::missing_image_pixels);
    CHECK(target.generation() == before_failed_generation);
    CHECK(checksum(target.snapshot()) == before_failed_checksum);

    std::atomic<RasterError> foreign_error{RasterError::none};
    std::thread foreign(ExecuteSkiaOffThread(
        executor, recorder, target, foreign_error));
    foreign.join();
    CHECK(foreign_error.load() == RasterError::wrong_thread);
}

void texture_brush_tiles_and_mirrors_retained_pixels() {
    SkiaExecutor executor;
    Bitmap source(2, 1);
    source.set_pixel(0, 0, Color::from_name("blue"));
    source.set_pixel(1, 0, Color::from_name("red"));
    TextureBrush texture(source, WrapMode::tile_flip_x);

    GraphicsRecorder recorder;
    recorder.set_quality(SmoothingMode::none, InterpolationMode::nearest,
                         PixelOffsetMode::none, CompositingMode::source_copy,
                         CompositingQuality::high_speed);
    recorder.fill_rectangle(texture, {0.0, 0.0, 6.0, 2.0});
    source.set_pixel(0, 0, Color::from_name("lime"));

    Bitmap target(6, 2);
    CHECK(executor.execute(recorder, target));
    const std::array<std::uint32_t, 6> expected{
        Color::from_name("blue").argb(), Color::from_name("red").argb(),
        Color::from_name("red").argb(), Color::from_name("blue").argb(),
        Color::from_name("blue").argb(), Color::from_name("red").argb()};
    for (std::uint32_t x = 0; x < expected.size(); ++x) {
        CHECK(target.get_pixel(x, 0).argb() == expected[x]);
    }
}

void transparent_png_channels_are_premultiplied() {
    constexpr std::array<std::uint8_t, 70> encoded{
        0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a,
        0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
        0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0x15, 0xc4,
        0x89, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x44, 0x41,
        0x54, 0x78, 0x9c, 0x63, 0xf8, 0xf8, 0xf1, 0x23,
        0x03, 0x00, 0x08, 0x7e, 0x02, 0xd4, 0x4f, 0x05,
        0x2f, 0xd9, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45,
        0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
    };
    std::array<std::byte, encoded.size()> bytes{};
    for (std::size_t index = 0; index < encoded.size(); ++index) {
        bytes[index] = static_cast<std::byte>(encoded[index]);
    }
    SkiaExecutor executor;
    DecodeResult decoded = executor.decode_png(bytes);
    CHECK(decoded && (*decoded.bitmap).width() == 1U &&
          (*decoded.bitmap).height() == 1U);
    const ImageSnapshot snapshot = (*decoded.bitmap).snapshot();
    CHECK(snapshot.pixels().size() == 4U);
    CHECK(snapshot.pixels()[0] == std::byte{0} &&
          snapshot.pixels()[1] == std::byte{0} &&
          snapshot.pixels()[2] == std::byte{0} &&
          snapshot.pixels()[3] == std::byte{0});
}

void gradients_and_hatches_are_native_raster_commands() {
    SkiaExecutor executor;
    Bitmap target(48, 16);
    LinearGradientBrush linear({0, 0, 16, 16}, Color::from_name("red"),
                               Color::from_name("blue"), 0.0,
                               WrapMode::clamp);
    HatchBrush hatch(HatchStyle::diagonal_cross, Color::from_name("white"),
                     Color::from_name("black"));
    const PointF boundary[] = {{32, 0}, {48, 0}, {48, 16}, {32, 16}};
    PathGradientBrush radial(boundary);
    radial.set_center_point({40, 8});
    radial.set_center_color(Color::from_name("white"));
    const Color surround[] = {Color::from_name("black")};
    radial.set_surround_colors(surround);

    GraphicsRecorder recorder;
    recorder.fill_rectangle(linear, {0, 0, 16, 16});
    recorder.fill_rectangle(hatch, {16, 0, 16, 16});
    recorder.fill_rectangle(radial, {32, 0, 16, 16});
    CHECK(executor.execute(recorder, target));

    CHECK(target.get_pixel(1, 8).red() > target.get_pixel(1, 8).blue());
    CHECK(target.get_pixel(14, 8).blue() > target.get_pixel(14, 8).red());
    CHECK(target.get_pixel(16, 0).argb() != target.get_pixel(17, 0).argb() ||
          target.get_pixel(16, 1).argb() != target.get_pixel(17, 1).argb());
    const Color center = target.get_pixel(40, 8);
    const Color edge = target.get_pixel(47, 8);
    CHECK(center.red() > edge.red());
}

void connected_line_figures_fill_as_one_contour() {
    SkiaExecutor executor;
    Bitmap target(20, 20);
    SolidBrush green(Color::from_name("lime"));
    GraphicsPath path(FillMode::winding);
    path.add_line({2, 2}, {17, 2});
    path.add_line({17, 2}, {17, 17});
    path.add_line({17, 17}, {2, 17});
    path.close_figure();

    GraphicsRecorder recorder;
    recorder.fill_path(green, path);
    CHECK(executor.execute(recorder, target));
    CHECK(target.get_pixel(10, 10).green() > 240U);
    CHECK(target.get_pixel(10, 10).alpha() == 255U);
    CHECK(target.get_pixel(0, 0).alpha() == 0U);
}

void quadratic_bezier_polygon_and_pie_paths_rasterize() {
    SkiaExecutor executor;
    Bitmap target(64, 40);
    SolidBrush green(Color::from_name("lime"));
    SolidBrush blue(Color::from_name("blue"));

    GraphicsPath curved(FillMode::winding);
    curved.start_figure();
    curved.add_line({2, 20}, {2, 12});
    curved.add_quadratic({2, 12}, {10, 2}, {18, 12});
    curved.add_bezier({18, 12}, {22, 2}, {30, 2}, {34, 12});
    curved.add_line({34, 12}, {34, 28});
    curved.add_line({34, 28}, {2, 28});
    curved.close_figure();

    GraphicsPath primitives;
    const PointF triangle[] = {{40, 4}, {60, 4}, {50, 18}};
    primitives.add_polygon(triangle);
    primitives.add_pie({40, 18, 20, 20}, 180.0, 180.0);

    GraphicsRecorder recorder;
    recorder.fill_path(green, curved);
    recorder.fill_path(blue, primitives);
    CHECK(executor.execute(recorder, target));
    CHECK(target.get_pixel(16, 20).green() > 240U);
    CHECK(target.get_pixel(50, 10).blue() > 240U);
    CHECK(target.get_pixel(50, 24).blue() > 240U);
    CHECK(target.get_pixel(0, 0).alpha() == 0U);
}

} // namespace

int main() {
    command_execution_and_png_round_trip();
    image_attributes_clip_and_snapshot_execution();
    texture_brush_tiles_and_mirrors_retained_pixels();
    transparent_png_channels_are_premultiplied();
    gradients_and_hatches_are_native_raster_commands();
    connected_line_figures_fill_as_one_contour();
    quadratic_bezier_polygon_and_pie_paths_rasterize();
    return 0;
}
