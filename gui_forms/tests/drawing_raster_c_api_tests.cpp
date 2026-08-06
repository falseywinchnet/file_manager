#include "gui_forms/drawing_c_api.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

extern "C" gd_result gdr_initialize(void);

namespace {

[[noreturn]] void fail(const char* expression, int line) {
    std::cerr << "drawing_raster_c_api_tests:" << line << ": "
              << expression << '\n';
    std::exit(1);
}

#define CHECK(expression) do { if (!(expression)) fail(#expression, __LINE__); } while (false)

} // namespace

int main() {
    gd_api_v0 api{};
    api.struct_size = sizeof(api);
    CHECK(gd_get_api_v0(GD_ABI_VERSION_0_1, &api) == GD_OK);
    CHECK(api.raster_service_install != nullptr);
    CHECK(api.recorder_execute != nullptr);
    CHECK(api.bitmap_encode_png != nullptr);
    CHECK(api.bitmap_decode_png != nullptr);
    CHECK(api.measure_string != nullptr);
    CHECK(api.recorder_execute_from != nullptr);
    CHECK(gdr_initialize() == GD_OK);
    CHECK(gdr_initialize() == GD_OK);

    gd_handle bitmap{};
    gd_handle recorder{};
    CHECK(api.bitmap_create(8, 6, GD_PIXEL_BGRA32_PREMULTIPLIED,
                            &bitmap) == GD_OK);
    CHECK(api.recorder_create(&recorder) == GD_OK);
    CHECK(api.recorder_clear(
              recorder, gd_color{UINT32_C(0xff336699), 0}) == GD_OK);
    std::uint64_t executed{};
    CHECK(api.recorder_execute(recorder, bitmap, &executed) == GD_OK);
    CHECK(executed == 1U);
    gd_color pixel{};
    CHECK(api.bitmap_get_pixel(bitmap, 0, 0, &pixel) == GD_OK);
    CHECK(pixel.argb == UINT32_C(0xff336699));

    CHECK(api.recorder_clear(
              recorder, gd_color{UINT32_C(0xff993311), 0}) == GD_OK);
    CHECK(api.recorder_execute_from(recorder, bitmap, 1U, &executed) == GD_OK);
    CHECK(executed == 1U);
    CHECK(api.bitmap_get_pixel(bitmap, 0, 0, &pixel) == GD_OK);
    CHECK(pixel.argb == UINT32_C(0xff993311));
    CHECK(api.recorder_execute_from(recorder, bitmap, 2U, &executed) == GD_OK);
    CHECK(executed == 0U);

    std::uint64_t required{};
    CHECK(api.bitmap_encode_png(bitmap, nullptr, 0, &required) ==
          GD_ERROR_BUFFER_TOO_SMALL);
    CHECK(required > 32U);
    std::vector<std::byte> png(static_cast<std::size_t>(required));
    CHECK(api.bitmap_encode_png(bitmap, png.data(), png.size(), &required) == GD_OK);
    gd_handle decoded{};
    CHECK(api.bitmap_decode_png(png.data(), png.size(), &decoded) == GD_OK);
    CHECK(api.bitmap_get_pixel(decoded, 7, 5, &pixel) == GD_OK);
    CHECK(pixel.argb == UINT32_C(0xff993311));

    const char family[] = "Portsmouth Rapids";
    gd_handle font{};
    gd_handle format{};
    CHECK(api.font_create({family, sizeof(family) - 1U}, 12.0, 0U,
                          3U, 1U, &font) == GD_OK);
    CHECK(api.string_format_create(0U, &format) == GD_OK);
    const char narrow[] = "iiii";
    const char wide[] = "WWWW";
    gd_size narrow_size{};
    gd_size wide_size{};
    CHECK(api.measure_string({narrow, sizeof(narrow) - 1U}, font, {}, 0.0,
                             &narrow_size) == GD_OK);
    CHECK(api.measure_string({wide, sizeof(wide) - 1U}, font, format, 0.0,
                             &wide_size) == GD_OK);
    CHECK(narrow_size.width > 0.0);
    CHECK(wide_size.width > narrow_size.width);
    CHECK(wide_size.height > 12.0);
    const char wrapped[] = "alpha beta gamma";
    gd_size unbounded{};
    gd_size bounded{};
    CHECK(api.measure_string({wrapped, sizeof(wrapped) - 1U}, font, format,
                             0.0, &unbounded) == GD_OK);
    CHECK(api.measure_string({wrapped, sizeof(wrapped) - 1U}, font, format,
                             unbounded.width / 2.0, &bounded) == GD_OK);
    CHECK(bounded.width <= unbounded.width / 2.0);
    CHECK(bounded.height > unbounded.height);

    CHECK(api.release(format) == GD_OK);
    CHECK(api.release(font) == GD_OK);
    CHECK(api.release(decoded) == GD_OK);
    CHECK(api.release(recorder) == GD_OK);
    CHECK(api.release(bitmap) == GD_OK);
    return 0;
}
