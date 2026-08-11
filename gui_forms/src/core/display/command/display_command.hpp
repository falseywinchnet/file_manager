#pragma once

#include "gui_forms/control.hpp"
#include "gui_forms/display.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace gui_forms::detail {

enum class DisplayOperation : std::uint8_t {
    save = 0,
    restore = 1,
    translate = 2,
    clip_rect = 3,
    fill_rect = 4,
    stroke_rect = 5,
    draw_line = 6,
    draw_text_utf8 = 7,
    draw_image = 8,
    clip_rounded_rect = 9,
    fill_rounded_rect = 10,
    stroke_rounded_rect = 11,
    fill_linear_gradient = 12,
    fill_radial_gradient = 13,
    draw_box_shadow = 14,
    fill_linear_gradient_spread = 15,
    draw_image_region = 16,
    fill_image_pattern = 17,
    draw_image_region_sampled = 18,
    draw_live_surface = 19,
    draw_inset_box_shadow = 20,
};

struct DisplayCommand final {
    DisplayOperation operation{};
    Point first{};
    Point second{};
    Rect rect{};
    Color color{};
    FontSpec font{};
    ImageId image{};
    std::shared_ptr<LiveSurface> live_surface;
    double scalar{};
    double secondary_scalar{};
    double tertiary_scalar{};
    std::vector<GradientStop> gradient_stops;
    GradientSpreadMode gradient_spread{GradientSpreadMode::pad};
    ImagePatternWrap image_pattern_wrap{ImagePatternWrap::tile};
    ImageSampling image_sampling{ImageSampling::linear};
    std::string text;
};

} // namespace gui_forms::detail
