#include "coregraphics_raster.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <span>
#include <vector>

namespace {

const std::uint8_t* pixel_at(const std::uint8_t* pixels,
                             std::size_t row_bytes, int scale,
                             int x, int y) {
    return pixels + static_cast<std::size_t>(y * scale) * row_bytes +
           static_cast<std::size_t>(x * scale) * 4U;
}

} // namespace

int main() {
    constexpr std::array<std::uint8_t, 70> png{
        0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
        0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
        0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0x15, 0xc4, 0x89, 0x00, 0x00, 0x00,
        0x0d, 0x49, 0x44, 0x41, 0x54, 0x78, 0xda, 0x63, 0xf8, 0xcf, 0xc0, 0xf0,
        0x1f, 0x00, 0x05, 0x00, 0x01, 0xff, 0x56, 0xc7, 0x2f, 0x0d, 0x00, 0x00,
        0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
    };
    gui_forms::render::CoreGraphicsRaster raster;
    gui_forms::ImageRegistry images;
    const gui_forms::ImageLoadResult loaded =
        images.load_png(std::as_bytes(std::span{png}));
    constexpr std::array<std::byte, 4> bgra{
        std::byte{0}, std::byte{128}, std::byte{255}, std::byte{255}};
    const gui_forms::ImageLoadResult raw =
        images.load_bgra32_premultiplied(1, 1, 4, bgra);
    constexpr std::array<std::byte, 8> strip_bgra{
        std::byte{0}, std::byte{0}, std::byte{255}, std::byte{255},
        std::byte{0}, std::byte{255}, std::byte{0}, std::byte{255}};
    const gui_forms::ImageLoadResult strip =
        images.load_bgra32_premultiplied(2, 1, 8, strip_bgra);
    if (!loaded || !raw || !strip || !raster.synchronize_images(images)) {
        std::fputs("CoreGraphics rejected the validated PNG resource\n", stderr);
        return 1;
    }

    std::ifstream font_file(GUI_FORMS_TEST_CONTROL_FONT, std::ios::binary | std::ios::ate);
    if (!font_file) {
        std::fputs("failed to open bundled control font\n", stderr);
        return 2;
    }
    const auto font_size = font_file.tellg();
    if (font_size <= 0) {
        return 3;
    }
    std::vector<std::byte> font_bytes(static_cast<std::size_t>(font_size));
    font_file.seekg(0);
    font_file.read(reinterpret_cast<char*>(font_bytes.data()), font_size);
    if (!font_file || !raster.register_typeface(
                          gui_forms::FontRole::control, 400, false, font_bytes)) {
        std::fputs("CoreText rejected the bundled control face\n", stderr);
        return 4;
    }
    if (!raster.resize({160.0, 96.0}, 2.0)) {
        return 5;
    }
    gui_forms::DamageRegion damage;
    damage.add({0.0, 0.0, 160.0, 96.0});
    raster.begin_frame(damage);
    raster.fill_rect({0.0, 0.0, 160.0, 96.0},
                     gui_forms::Color::rgba(241, 238, 226));
    raster.fill_rect({8.0, 8.0, 74.0, 30.0},
                     gui_forms::Color::rgba(57, 93, 155));
    raster.stroke_rect({8.5, 8.5, 73.0, 29.0},
                       gui_forms::Color::rgba(255, 210, 122), 1.0);
    raster.draw_line({8.0, 47.0}, {150.0, 47.0},
                     gui_forms::Color::rgba(53, 107, 98), 1.0);
    raster.draw_text_utf8({12.0, 29.0}, "GUI.Forms CPU", {},
                          gui_forms::Color::rgba(255, 255, 255));
    raster.draw_image(loaded.image, {130.0, 8.0, 20.0, 20.0}, 1.0);
    raster.draw_image(raw.image, {130.0, 32.0, 20.0, 20.0}, 1.0);
    raster.draw_image_region(strip.image, {1.0, 0.0, 1.0, 1.0},
                             {92.0, 82.0, 8.0, 8.0}, 1.0);
    raster.fill_image_pattern(strip.image, {2.0, 1.0},
                              {102.0, 64.0, 48.0, 14.0}, {16.0, 8.0},
                              gui_forms::ImagePatternWrap::tile, 1.0);
    constexpr std::array<gui_forms::GradientStop, 2> repeating_gradient{{
        {0.0, gui_forms::Color::rgba(12, 28, 44)},
        {1.0, gui_forms::Color::rgba(225, 236, 246)}}};
    raster.fill_linear_gradient_spread(
        {8.0, 64.0, 80.0, 14.0}, {8.0, 64.0}, {16.0, 64.0},
        repeating_gradient, gui_forms::GradientSpreadMode::repeat);
    raster.end_frame();

    const auto* pixels = static_cast<const std::uint8_t*>(raster.pixels());
    if (pixels == nullptr || raster.byte_size() == 0 ||
        pixels[0] != 241U || pixels[1] != 238U || pixels[2] != 226U ||
        pixels[3] != 255U) {
        std::fputs("CoreGraphics RGBA surface contract failed\n", stderr);
        return 6;
    }
    const std::uint8_t* repeat_first = pixel_at(
        pixels, raster.row_bytes(), 2, 10, 70);
    const std::uint8_t* repeat_second = pixel_at(
        pixels, raster.row_bytes(), 2, 18, 70);
    const std::uint8_t* repeat_contrast = pixel_at(
        pixels, raster.row_bytes(), 2, 14, 70);
    const std::uint8_t* cropped_green = pixel_at(
        pixels, raster.row_bytes(), 2, 95, 85);
    const std::uint8_t* pattern_red = pixel_at(
        pixels, raster.row_bytes(), 2, 106, 70);
    const std::uint8_t* pattern_green = pixel_at(
        pixels, raster.row_bytes(), 2, 114, 70);
    const std::uint8_t* pattern_repeat = pixel_at(
        pixels, raster.row_bytes(), 2, 122, 70);
    const auto channel_distance = [](std::uint8_t first, std::uint8_t second) {
        return first > second ? first - second : second - first;
    };
    if (channel_distance(repeat_first[0], repeat_second[0]) > 2U ||
        channel_distance(repeat_first[1], repeat_second[1]) > 2U ||
        channel_distance(repeat_first[2], repeat_second[2]) > 2U ||
        repeat_contrast[0] <= repeat_first[0] + 45U) {
        std::fprintf(stderr,
                     "CoreGraphics repeating-gradient period contract changed: "
                     "first=%u,%u,%u second=%u,%u,%u contrast=%u,%u,%u\n",
                     repeat_first[0], repeat_first[1], repeat_first[2],
                     repeat_second[0], repeat_second[1], repeat_second[2],
                     repeat_contrast[0], repeat_contrast[1], repeat_contrast[2]);
        return 8;
    }
    if (cropped_green[0] > 8U || cropped_green[1] < 240U ||
        cropped_green[2] > 8U || cropped_green[3] != 255U) {
        std::fputs("CoreGraphics source-region image crop contract changed\n", stderr);
        return 9;
    }
    if (pattern_red[0] < 220U || pattern_red[1] > 24U ||
        pattern_green[0] > 24U || pattern_green[1] < 220U ||
        pattern_repeat[0] < 220U || pattern_repeat[1] > 24U) {
        std::fprintf(stderr,
                     "CoreGraphics exact image-pattern period changed: "
                     "red=%u,%u green=%u,%u repeat=%u,%u\n",
                     pattern_red[0], pattern_red[1], pattern_green[0],
                     pattern_green[1], pattern_repeat[0], pattern_repeat[1]);
        return 10;
    }
    std::uint64_t checksum = 1469598103934665603ULL;
    for (std::size_t offset = 0; offset < raster.byte_size(); offset += 97U) {
        checksum ^= pixels[offset];
        checksum *= 1099511628211ULL;
    }
    if (checksum == 1469598103934665603ULL) {
        return 7;
    }
    std::printf("{\"renderer\":\"coregraphics-cpu\",\"width\":%u,"
                "\"height\":%u,\"bytes\":%zu,\"checksum\":%llu}\n",
                raster.pixel_width(), raster.pixel_height(), raster.byte_size(),
                static_cast<unsigned long long>(checksum));
    return 0;
}
