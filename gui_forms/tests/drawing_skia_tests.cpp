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
    CHECK(decoded && decoded.bitmap->width() == 64U && decoded.bitmap->height() == 48U);
    CHECK(checksum(decoded.bitmap->snapshot()) == first_checksum);

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
    std::thread foreign([&] {
        foreign_error = executor.execute(recorder, target).error;
    });
    foreign.join();
    CHECK(foreign_error.load() == RasterError::wrong_thread);
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

} // namespace

int main() {
    command_execution_and_png_round_trip();
    image_attributes_clip_and_snapshot_execution();
    gradients_and_hatches_are_native_raster_commands();
    connected_line_figures_fill_as_one_contour();
    return 0;
}
