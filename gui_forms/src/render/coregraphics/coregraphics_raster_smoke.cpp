#include "coregraphics_raster.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <span>
#include <vector>

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
    if (!loaded || !raw || !raster.synchronize_images(images)) {
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
    raster.end_frame();

    const auto* pixels = static_cast<const std::uint8_t*>(raster.pixels());
    if (pixels == nullptr || raster.byte_size() == 0 ||
        pixels[0] != 241U || pixels[1] != 238U || pixels[2] != 226U ||
        pixels[3] != 255U) {
        std::fputs("CoreGraphics RGBA surface contract failed\n", stderr);
        return 6;
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
