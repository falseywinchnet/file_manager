#include "skia_raster.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <span>
#include <vector>

namespace {

std::uint32_t crc32(std::span<const std::byte> bytes) {
    std::uint32_t crc = 0xffffffffU;
    for (const std::byte value : bytes) {
        crc ^= std::to_integer<std::uint8_t>(value);
        for (unsigned bit = 0; bit < 8; ++bit) {
            const std::uint32_t mask = 0U - (crc & 1U);
            crc = (crc >> 1U) ^ (0xedb88320U & mask);
        }
    }
    return crc ^ 0xffffffffU;
}

void write_u32(std::span<std::byte> destination, std::uint32_t value) {
    destination[0] = static_cast<std::byte>(value >> 24U);
    destination[1] = static_cast<std::byte>(value >> 16U);
    destination[2] = static_cast<std::byte>(value >> 8U);
    destination[3] = static_cast<std::byte>(value);
}

std::vector<std::byte> read_file(const char* path) {
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream) return {};
    const std::streamsize length = stream.tellg();
    if (length <= 0) return {};
    std::vector<std::byte> bytes(static_cast<std::size_t>(length));
    stream.seekg(0);
    stream.read(reinterpret_cast<char*>(bytes.data()), length);
    return stream ? bytes : std::vector<std::byte>{};
}

std::size_t ink_pixels(const std::uint8_t* pixels, std::size_t row_bytes,
                       int scale, int left, int top, int right, int bottom) {
    std::size_t count{};
    for (int y = top * scale; y < bottom * scale; ++y) {
        const std::uint8_t* row = pixels + static_cast<std::size_t>(y) * row_bytes;
        for (int x = left * scale; x < right * scale; ++x) {
            const std::uint8_t* pixel = row + static_cast<std::size_t>(x) * 4U;
            if (pixel[0] != 241U || pixel[1] != 238U || pixel[2] != 226U) {
                ++count;
            }
        }
    }
    return count;
}

const std::uint8_t* pixel_at(const std::uint8_t* pixels, std::size_t row_bytes,
                             int scale, int x, int y) {
    return pixels + static_cast<std::size_t>(y * scale) * row_bytes +
           static_cast<std::size_t>(x * scale) * 4U;
}

} // namespace

int main() {
    constexpr std::array<std::uint8_t, 70> png = {
        0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
        0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
        0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0x15, 0xc4, 0x89, 0x00, 0x00, 0x00,
        0x0d, 0x49, 0x44, 0x41, 0x54, 0x78, 0xda, 0x63, 0xf8, 0xcf, 0xc0, 0xf0,
        0x1f, 0x00, 0x05, 0x00, 0x01, 0xff, 0x56, 0xc7, 0x2f, 0x0d, 0x00, 0x00,
        0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
    };
    gui_forms::render::SkiaRaster raster;
    gui_forms::ImageRegistry images;

    std::vector<std::byte> broken_payload(
        std::as_bytes(std::span{png}).begin(), std::as_bytes(std::span{png}).end());
    broken_payload[41] = std::byte{0};
    write_u32(std::span<std::byte>(broken_payload.data() + 54, 4),
              crc32(std::span<const std::byte>(broken_payload.data() + 37, 17)));
    gui_forms::ImageRegistry broken_images;
    const gui_forms::ImageLoadResult structurally_valid =
        broken_images.load_png(broken_payload);
    gui_forms::render::SkiaRaster broken_raster;
    if (!structurally_valid || broken_raster.synchronize_images(broken_images)) {
        std::fputs("malformed compressed payload crossed the eager decoder boundary\n", stderr);
        return 1;
    }

    const std::vector<std::byte> font_bytes =
        read_file(GUI_FORMS_TEST_CONTROL_FONT);
    const std::vector<std::byte> cjk_bytes =
        read_file(GUI_FORMS_TEST_CJK_FONT);
    const std::vector<std::byte> emoji_bytes =
        read_file(GUI_FORMS_TEST_EMOJI_FONT);
    if (font_bytes.empty() || cjk_bytes.empty() || emoji_bytes.empty()) {
        std::fputs("failed to open bundled control font\n", stderr);
        return 2;
    }
    if (!raster.register_typeface(
            gui_forms::FontRole::control, 400, false, font_bytes) ||
        !raster.register_fallback_typeface(400, false, cjk_bytes) ||
        !raster.register_fallback_typeface(400, false, emoji_bytes)) {
        std::fputs("Skia rejected the bundled Portsmouth Rapids face\n", stderr);
        return 4;
    }
    if (!raster.resize(gui_forms::Size{220.0, 120.0}, 2.0)) {
        std::fputs("failed to allocate Skia raster surface\n", stderr);
        return 5;
    }
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
        std::fputs("validated PNG registry resource failed eager decode\n", stderr);
        return 6;
    }
    gui_forms::DamageRegion damage;
    damage.add(gui_forms::Rect{0.0, 0.0, 220.0, 120.0});
    raster.begin_frame(damage);
    raster.fill_rect(gui_forms::Rect{0.0, 0.0, 220.0, 120.0},
                     gui_forms::Color::rgba(241, 238, 226));
    raster.fill_rect(gui_forms::Rect{8.0, 8.0, 74.0, 30.0},
                     gui_forms::Color::rgba(57, 93, 155));
    raster.stroke_rect(gui_forms::Rect{8.5, 8.5, 73.0, 29.0},
                       gui_forms::Color::rgba(255, 210, 122), 1.0);
    raster.draw_line(gui_forms::Point{8.0, 47.0}, gui_forms::Point{150.0, 47.0},
                     gui_forms::Color::rgba(53, 107, 98), 1.0);
    raster.draw_text_utf8(gui_forms::Point{12.0, 29.0}, "GUI.Forms CPU",
                          gui_forms::FontSpec{}, gui_forms::Color::rgba(255, 255, 255));
    raster.draw_image(loaded.image, gui_forms::Rect{130.0, 8.0, 20.0, 20.0}, 1.0);
    raster.draw_image(raw.image, gui_forms::Rect{130.0, 32.0, 20.0, 20.0}, 1.0);
    raster.draw_image_region(strip.image, {1.0, 0.0, 1.0, 1.0},
                             {92.0, 84.0, 8.0, 8.0}, 1.0);
    raster.fill_image_pattern(strip.image, {2.0, 1.0},
                              {94.0, 98.0, 48.0, 16.0}, {16.0, 8.0},
                              gui_forms::ImagePatternWrap::tile, 1.0);
    raster.draw_text_utf8(gui_forms::Point{8.0, 82.0}, "日本語",
                          {gui_forms::FontRole::control, 24.0, 400, false},
                          gui_forms::Color::rgba(30, 42, 54));
    raster.draw_text_utf8(gui_forms::Point{112.0, 82.0}, "🚀",
                          {gui_forms::FontRole::control, 24.0, 400, false},
                          gui_forms::Color::rgba(30, 42, 54));
    constexpr std::array<gui_forms::GradientStop, 3> title_gradient{{
        {0.0, gui_forms::Color::rgba(18, 55, 140)},
        {0.55, gui_forms::Color::rgba(55, 140, 215)},
        {1.0, gui_forms::Color::rgba(220, 76, 105)}}};
    constexpr std::array<gui_forms::GradientStop, 2> radial_glow{{
        {0.0, gui_forms::Color::rgba(255, 255, 255, 230)},
        {1.0, gui_forms::Color::rgba(75, 110, 180, 25)}}};
    const gui_forms::Rect title{165.0, 10.0, 45.0, 36.0};
    raster.draw_box_shadow(title, 7.0, {0.0, 3.0}, 5.0, 0.0,
                           gui_forms::Color::rgba(18, 28, 48, 110));
    raster.save();
    raster.clip_rounded_rect(title, 7.0);
    raster.fill_linear_gradient(title, {165.0, 10.0}, {210.0, 10.0},
                                title_gradient);
    raster.restore();
    raster.stroke_rounded_rect({165.5, 10.5, 44.0, 35.0}, 6.5,
                               gui_forms::Color::rgba(20, 36, 70), 1.0);
    const gui_forms::Rect glow{165.0, 70.0, 45.0, 36.0};
    raster.fill_radial_gradient(glow, {187.5, 88.0}, {22.5, 18.0},
                                radial_glow);
    raster.fill_rounded_rect({125.0, 96.0, 25.0, 18.0}, 7.0,
                             gui_forms::Color::rgba(210, 65, 80));
    constexpr std::array<gui_forms::GradientStop, 2> repeating_gradient{{
        {0.0, gui_forms::Color::rgba(12, 28, 44)},
        {1.0, gui_forms::Color::rgba(225, 236, 246)}}};
    raster.fill_linear_gradient_spread(
        {8.0, 100.0, 80.0, 14.0}, {8.0, 100.0}, {16.0, 100.0},
        repeating_gradient, gui_forms::GradientSpreadMode::repeat);
    raster.end_frame();

    const auto* pixels = static_cast<const std::uint8_t*>(raster.pixels());
    if (pixels == nullptr || raster.byte_size() == 0) {
        std::fputs("Skia raster surface exposed no pixels\n", stderr);
        return 7;
    }
    if (pixels[0] != 241U || pixels[1] != 238U || pixels[2] != 226U ||
        pixels[3] != 255U) {
        std::fputs("Skia raster RGBA byte-order contract changed\n", stderr);
        return 8;
    }
    const std::size_t cjk_ink = ink_pixels(
        pixels, raster.row_bytes(), 2, 6, 55, 104, 92);
    const std::size_t emoji_ink = ink_pixels(
        pixels, raster.row_bytes(), 2, 108, 55, 158, 92);
    if (cjk_ink < 50U || emoji_ink < 20U) {
        std::fprintf(stderr,
                     "registered fallback shaped without raster ink: cjk=%zu emoji=%zu\n",
                     cjk_ink, emoji_ink);
        return 9;
    }
    const std::uint8_t* rounded_corner = pixel_at(
        pixels, raster.row_bytes(), 2, 125, 96);
    const std::uint8_t* title_left = pixel_at(
        pixels, raster.row_bytes(), 2, 170, 28);
    const std::uint8_t* title_right = pixel_at(
        pixels, raster.row_bytes(), 2, 204, 28);
    const std::uint8_t* glow_center = pixel_at(
        pixels, raster.row_bytes(), 2, 187, 88);
    const std::uint8_t* glow_edge = pixel_at(
        pixels, raster.row_bytes(), 2, 166, 88);
    const std::uint8_t* repeat_first = pixel_at(
        pixels, raster.row_bytes(), 2, 10, 106);
    const std::uint8_t* repeat_second = pixel_at(
        pixels, raster.row_bytes(), 2, 18, 106);
    const std::uint8_t* repeat_contrast = pixel_at(
        pixels, raster.row_bytes(), 2, 14, 106);
    const std::uint8_t* cropped_green = pixel_at(
        pixels, raster.row_bytes(), 2, 95, 87);
    const std::uint8_t* pattern_red = pixel_at(
        pixels, raster.row_bytes(), 2, 98, 104);
    const std::uint8_t* pattern_green = pixel_at(
        pixels, raster.row_bytes(), 2, 106, 104);
    const std::uint8_t* pattern_repeat = pixel_at(
        pixels, raster.row_bytes(), 2, 114, 104);
    if (rounded_corner[0] != 241U || rounded_corner[1] != 238U ||
        rounded_corner[2] != 226U || title_left[2] <= title_left[0] ||
        title_right[0] <= title_right[2] || glow_center[0] <= glow_edge[0] ||
        glow_center[1] <= glow_edge[1]) {
        std::fputs("retained material raster contract changed\n", stderr);
        return 11;
    }
    if (cropped_green[0] > 8U || cropped_green[1] < 240U ||
        cropped_green[2] > 8U || cropped_green[3] != 255U) {
        std::fputs("Skia source-region image crop contract changed\n", stderr);
        return 13;
    }
    if (pattern_red[0] < 220U || pattern_red[1] > 24U ||
        pattern_green[0] > 24U || pattern_green[1] < 220U ||
        pattern_repeat[0] < 220U || pattern_repeat[1] > 24U) {
        std::fputs("Skia exact image-pattern period contract changed\n", stderr);
        return 14;
    }
    if (repeat_first[0] != repeat_second[0] ||
        repeat_first[1] != repeat_second[1] ||
        repeat_first[2] != repeat_second[2] ||
        repeat_contrast[0] <= repeat_first[0] + 50U) {
        std::fputs("Skia repeating-gradient period contract changed\n", stderr);
        return 12;
    }
    std::uint64_t checksum = 1469598103934665603ULL;
    for (std::size_t offset = 0; offset < raster.byte_size(); offset += 97) {
        checksum ^= pixels[offset];
        checksum *= 1099511628211ULL;
    }
    if (checksum == 1469598103934665603ULL) {
        std::fputs("Skia raster surface remained empty\n", stderr);
        return 10;
    }
    std::printf("{\"renderer\":\"skia-cpu\",\"width\":%u,\"height\":%u,"
                "\"bytes\":%zu,\"checksum\":%llu}\n",
                raster.pixel_width(), raster.pixel_height(), raster.byte_size(),
                static_cast<unsigned long long>(checksum));
    return 0;
}
