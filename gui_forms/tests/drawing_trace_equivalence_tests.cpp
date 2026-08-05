#include "drawing_test_trace.hpp"
#include "gui_forms/drawing.hpp"
#include "gui_forms/drawing_c_api.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace {

[[noreturn]] void fail(const char* expression, int line) {
    std::cerr << "drawing_trace_equivalence_tests:" << line << ": "
              << expression << '\n';
    std::exit(1);
}

#define CHECK(expression) do { if (!(expression)) fail(#expression, __LINE__); } while (false)

gd_string_view c_view(std::string_view value) {
    return {value.data(), static_cast<std::uint64_t>(value.size())};
}

std::string via_cpp() {
    using namespace gui_drawing;
    SolidBrush brush(Color::from_argb(UINT32_C(0xffff0000)));
    Pen pen(Color::from_argb(UINT32_C(0xffffff00)), 2.0);
    pen.set_dash_style(DashStyle::dash);
    Font font("Lucida Grande", 12.0, FontStyle::bold, GraphicsUnit::point, 1);
    StringFormat format(4);
    format.set_alignment(StringAlignment::center);
    format.set_trimming(StringTrimming::ellipsis_character);
    GraphicsRecorder recorder;
    recorder.clear(Color::from_argb(UINT32_C(0xff000000)));
    const auto token = recorder.save();
    recorder.translate(10, 5);
    recorder.set_clip({0, 0, 100, 50});
    recorder.set_quality(SmoothingMode::anti_alias, InterpolationMode::bicubic,
                         PixelOffsetMode::half, CompositingMode::source_over,
                         CompositingQuality::high_quality);
    recorder.fill_rectangle(brush, {1, 2, 30, 20});
    recorder.draw_rectangle(pen, {1, 2, 30, 20});
    recorder.draw_line(pen, {1, 2}, {31, 22});
    recorder.draw_string("field \xce\xa9", font, brush, {4, 7}, format);
    recorder.restore(token);
    recorder.close();
    return recorder.deterministic_trace();
}

std::string via_c() {
    gd_api_v0 api{};
    api.struct_size = sizeof(api);
    CHECK(gd_get_api_v0(GD_ABI_VERSION_0_1, &api) == GD_OK);
    gd_handle brush{}, pen{}, font{}, format{}, recorder{};
    std::uint64_t token{}, required{};
    CHECK(api.solid_brush_create({UINT32_C(0xffff0000), 0}, &brush) == GD_OK);
    CHECK(api.pen_create({UINT32_C(0xffffff00), 0}, 2, &pen) == GD_OK);
    CHECK(api.pen_set_dash_style(pen, GD_DASH_DASH) == GD_OK);
    CHECK(api.font_create(c_view("Lucida Grande"), 12, 1, 3, 1, &font) == GD_OK);
    CHECK(api.string_format_create(4, &format) == GD_OK);
    CHECK(api.string_format_set(format, GD_STRING_CENTER, GD_STRING_NEAR,
                                GD_TRIM_ELLIPSIS_CHARACTER, 4) == GD_OK);
    CHECK(api.recorder_create(&recorder) == GD_OK);
    CHECK(api.recorder_clear(recorder, {UINT32_C(0xff000000), 0}) == GD_OK);
    CHECK(api.recorder_save(recorder, &token) == GD_OK);
    CHECK(api.recorder_translate(recorder, 10, 5) == GD_OK);
    CHECK(api.recorder_set_clip(recorder, {0, 0, 100, 50}) == GD_OK);
    CHECK(api.recorder_set_quality(recorder, 4, 5, 4, 0, 2) == GD_OK);
    CHECK(api.recorder_fill_rectangle(recorder, brush, {1, 2, 30, 20}) == GD_OK);
    CHECK(api.recorder_draw_rectangle(recorder, pen, {1, 2, 30, 20}) == GD_OK);
    CHECK(api.recorder_draw_line(recorder, pen, {1, 2}, {31, 22}) == GD_OK);
    CHECK(api.recorder_draw_string(recorder, c_view("field \xce\xa9"), font,
                                   brush, {4, 7}, format) == GD_OK);
    CHECK(api.recorder_restore(recorder, token) == GD_OK);
    CHECK(api.recorder_close(recorder) == GD_OK);
    CHECK(api.recorder_trace(recorder, nullptr, 0, &required) ==
          GD_ERROR_BUFFER_TOO_SMALL);
    std::string trace(required, '\0');
    CHECK(api.recorder_trace(recorder, trace.data(), trace.size(), &required) == GD_OK);
    CHECK(api.release(recorder) == GD_OK);
    CHECK(api.release(format) == GD_OK);
    CHECK(api.release(font) == GD_OK);
    CHECK(api.release(pen) == GD_OK);
    CHECK(api.release(brush) == GD_OK);
    return trace;
}

} // namespace

int main() {
    const std::string cpp_trace = via_cpp();
    const std::string c_trace = via_c();
    CHECK(cpp_trace == gui_drawing_expected_trace);
    CHECK(c_trace == cpp_trace);

    gd_api_v0 api{};
    api.struct_size = sizeof(api);
    CHECK(gd_get_api_v0(GD_ABI_VERSION_0_1, &api) == GD_OK);
    gd_handle brush{};
    CHECK(api.solid_brush_create({UINT32_C(0xffffffff), 0}, &brush) == GD_OK);
    gd_result foreign_result = GD_OK;
    std::thread foreign([&] {
        std::uint32_t kind{};
        foreign_result = api.object_kind(brush, &kind);
    });
    foreign.join();
    CHECK(foreign_result == GD_ERROR_WRONG_THREAD);
    CHECK(api.release(brush) == GD_OK);
    return 0;
}
