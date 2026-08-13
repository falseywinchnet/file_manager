#pragma once

#include "file_manager/filesystem_model.hpp"
#include "gui_forms/gui_forms.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace file_manager::house_art {

enum class Icon : std::uint8_t {
    folder,
    document,
    image,
    archive,
    audio,
    code,
    drive,
    home,
    app,
    back,
    forward,
    up,
    transfer,
    remove,
    view,
    sort,
    properties,
};

inline constexpr std::array<std::pair<Icon, std::string_view>, 17> icon_keys{{
    {Icon::folder, "folder"},
    {Icon::document, "document"},
    {Icon::image, "image"},
    {Icon::archive, "archive"},
    {Icon::audio, "audio"},
    {Icon::code, "code"},
    {Icon::drive, "drive"},
    {Icon::home, "home"},
    {Icon::app, "app"},
    {Icon::back, "back"},
    {Icon::forward, "forward"},
    {Icon::up, "up"},
    {Icon::transfer, "transfer"},
    {Icon::remove, "remove"},
    {Icon::view, "view"},
    {Icon::sort, "sort"},
    {Icon::properties, "properties"},
}};

[[nodiscard]] inline std::string_view key(const Icon icon) noexcept {
    for (const auto& [candidate, name] : icon_keys) {
        if (candidate == icon) return name;
    }
    return "document";
}

[[nodiscard]] inline std::string_view object_key(
    const EntryKind kind) noexcept {
    switch (kind) {
        case EntryKind::folder: return key(Icon::folder);
        case EntryKind::image: return key(Icon::image);
        case EntryKind::archive: return key(Icon::archive);
        case EntryKind::audio: return key(Icon::audio);
        case EntryKind::code: return key(Icon::code);
        case EntryKind::document:
        case EntryKind::symlink:
        case EntryKind::other:
            return key(Icon::document);
    }
    return key(Icon::document);
}

namespace detail {

using gui_forms::Color;

struct Raster final {
    explicit Raster(const std::uint32_t extent)
        : extent(extent), pixels(static_cast<std::size_t>(extent) * extent * 4U) {}

    std::uint32_t extent;
    std::vector<std::byte> pixels;

    static std::uint8_t mix(const std::uint8_t first,
                            const std::uint8_t second,
                            const double position) noexcept {
        return static_cast<std::uint8_t>(std::lround(
            static_cast<double>(first) * (1.0 - position) +
            static_cast<double>(second) * position));
    }

    static Color blend(const Color first, const Color second,
                       const double position) noexcept {
        return Color::rgba(mix(first.red, second.red, position),
                           mix(first.green, second.green, position),
                           mix(first.blue, second.blue, position),
                           mix(first.alpha, second.alpha, position));
    }

    void pixel(const int x, const int y, const Color source) noexcept {
        if (x < 0 || y < 0 || x >= static_cast<int>(extent) ||
            y >= static_cast<int>(extent) || source.alpha == 0U) return;
        const std::size_t offset =
            (static_cast<std::size_t>(y) * extent +
             static_cast<std::size_t>(x)) * 4U;
        const double source_alpha = static_cast<double>(source.alpha) / 255.0;
        const std::uint8_t destination_alpha =
            static_cast<std::uint8_t>(pixels[offset + 3U]);
        const double output_alpha = source_alpha +
            static_cast<double>(destination_alpha) / 255.0 *
                (1.0 - source_alpha);
        const auto composite = [&](const std::uint8_t channel,
                                   const std::size_t component) {
            const double destination =
                static_cast<double>(static_cast<std::uint8_t>(pixels[offset + component]));
            return static_cast<std::byte>(std::lround(
                static_cast<double>(channel) * source_alpha +
                destination * (1.0 - source_alpha)));
        };
        pixels[offset] = composite(source.blue, 0U);
        pixels[offset + 1U] = composite(source.green, 1U);
        pixels[offset + 2U] = composite(source.red, 2U);
        pixels[offset + 3U] = static_cast<std::byte>(
            std::lround(output_alpha * 255.0));
    }

    void rect(const int left, const int top, const int right, const int bottom,
              const Color color) noexcept {
        for (int y = top; y < bottom; ++y) {
            for (int x = left; x < right; ++x) pixel(x, y, color);
        }
    }

    void gradient_rect(const int left, const int top, const int right,
                       const int bottom, const Color first,
                       const Color second) noexcept {
        const int height = std::max(1, bottom - top - 1);
        for (int y = top; y < bottom; ++y) {
            const double position = static_cast<double>(y - top) / height;
            rect(left, y, right, y + 1, blend(first, second, position));
        }
    }

    void outline(const int left, const int top, const int right,
                 const int bottom, const Color color) noexcept {
        rect(left, top, right, top + 1, color);
        rect(left, bottom - 1, right, bottom, color);
        rect(left, top, left + 1, bottom, color);
        rect(right - 1, top, right, bottom, color);
    }

    void line(int x0, int y0, const int x1, const int y1,
              const Color color, const int width = 1) noexcept {
        const int delta_x = std::abs(x1 - x0);
        const int step_x = x0 < x1 ? 1 : -1;
        const int delta_y = -std::abs(y1 - y0);
        const int step_y = y0 < y1 ? 1 : -1;
        int error = delta_x + delta_y;
        for (;;) {
            const int radius = std::max(0, width - 1);
            rect(x0 - radius, y0 - radius, x0 + radius + 1,
                 y0 + radius + 1, color);
            if (x0 == x1 && y0 == y1) break;
            const int doubled = 2 * error;
            if (doubled >= delta_y) { error += delta_y; x0 += step_x; }
            if (doubled <= delta_x) { error += delta_x; y0 += step_y; }
        }
    }

    void circle(const int center_x, const int center_y, const int radius,
                const Color color) noexcept {
        for (int y = -radius; y <= radius; ++y) {
            for (int x = -radius; x <= radius; ++x) {
                if (x * x + y * y <= radius * radius) {
                    pixel(center_x + x, center_y + y, color);
                }
            }
        }
    }

    void triangle(gui_forms::Point first, gui_forms::Point second,
                  gui_forms::Point third, const Color color) noexcept {
        const int minimum_x = static_cast<int>(std::floor(
            std::min({first.x, second.x, third.x})));
        const int maximum_x = static_cast<int>(std::ceil(
            std::max({first.x, second.x, third.x})));
        const int minimum_y = static_cast<int>(std::floor(
            std::min({first.y, second.y, third.y})));
        const int maximum_y = static_cast<int>(std::ceil(
            std::max({first.y, second.y, third.y})));
        const auto edge = [](const gui_forms::Point a,
                             const gui_forms::Point b,
                             const gui_forms::Point point) {
            return (point.x - a.x) * (b.y - a.y) -
                   (point.y - a.y) * (b.x - a.x);
        };
        for (int y = minimum_y; y <= maximum_y; ++y) {
            for (int x = minimum_x; x <= maximum_x; ++x) {
                const gui_forms::Point point{
                    static_cast<double>(x) + 0.5,
                    static_cast<double>(y) + 0.5};
                const double first_edge = edge(first, second, point);
                const double second_edge = edge(second, third, point);
                const double third_edge = edge(third, first, point);
                const bool negative = first_edge < 0.0 || second_edge < 0.0 ||
                    third_edge < 0.0;
                const bool positive = first_edge > 0.0 || second_edge > 0.0 ||
                    third_edge > 0.0;
                if (!(negative && positive)) pixel(x, y, color);
            }
        }
    }
};

[[nodiscard]] inline Raster make_icon(const Icon icon,
                                      const std::uint32_t extent) {
    Raster raster(extent);
    const double scale = static_cast<double>(extent) / 48.0;
    const auto coordinate = [scale](const double value) {
        return static_cast<int>(std::lround(value * scale));
    };
    const auto stroke = [scale](const double value = 1.0) {
        return std::max(1, static_cast<int>(std::lround(value * scale)));
    };
    const Color shadow = Color::rgba(21, 38, 55, 68);
    const Color dark_blue = Color::rgba(39, 83, 119);
    const Color blue_top = Color::rgba(168, 220, 255);
    const Color blue_bottom = Color::rgba(22, 91, 141);
    const Color green_top = Color::rgba(205, 233, 183);
    const Color green_bottom = Color::rgba(63, 116, 42);
    const Color violet_top = Color::rgba(224, 197, 239);
    const Color violet_bottom = Color::rgba(91, 50, 120);
    const Color paper_top = Color::rgba(255, 255, 255);
    const Color paper_bottom = Color::rgba(223, 233, 239);
    const Color folder_top = Color::rgba(255, 226, 154);
    const Color folder_bottom = Color::rgba(216, 144, 40);
    const Color folder_front = Color::rgba(255, 207, 101);
    const Color folder_deep = Color::rgba(188, 106, 22);

    if (icon == Icon::back || icon == Icon::forward || icon == Icon::up) {
        const Color top = icon == Icon::up ? green_top : blue_top;
        const Color bottom = icon == Icon::up ? green_bottom : blue_bottom;
        const int top_edge = coordinate(7);
        const int bottom_edge = coordinate(41);
        for (int y = top_edge; y < bottom_edge; ++y) {
            const double position = static_cast<double>(y - top_edge) /
                std::max(1, bottom_edge - top_edge - 1);
            const Color color = Raster::blend(top, bottom, position);
            if (icon == Icon::up) {
                raster.triangle({24.0 * scale, 6.0 * scale},
                                {5.0 * scale, 25.0 * scale},
                                {43.0 * scale, 25.0 * scale}, color);
                raster.rect(coordinate(17), coordinate(22), coordinate(31),
                            coordinate(42), color);
            } else {
                const bool forward = icon == Icon::forward;
                const double tip_x = (forward ? 43.0 : 5.0) * scale;
                const double shoulder_x = (forward ? 24.0 : 24.0) * scale;
                raster.triangle({tip_x, 24.0 * scale},
                                {shoulder_x, 6.0 * scale},
                                {shoulder_x, 42.0 * scale}, color);
                raster.rect(forward ? coordinate(5) : coordinate(24),
                            coordinate(17),
                            forward ? coordinate(29) : coordinate(43),
                            coordinate(31), color);
            }
        }
        return raster;
    }

    if (icon == Icon::folder || icon == Icon::app) {
        raster.rect(coordinate(5), coordinate(12), coordinate(42), coordinate(43), shadow);
        raster.gradient_rect(coordinate(4), coordinate(9), coordinate(40),
                             coordinate(19), folder_top, folder_bottom);
        raster.rect(coordinate(4), coordinate(9), coordinate(19), coordinate(17), folder_top);
        raster.rect(coordinate(18), coordinate(7), coordinate(34), coordinate(16), folder_top);
        raster.gradient_rect(coordinate(4), coordinate(15), coordinate(43),
                             coordinate(42), folder_front, folder_deep);
        raster.outline(coordinate(4), coordinate(15), coordinate(43),
                       coordinate(42), Color::rgba(158, 91, 22));
        raster.rect(coordinate(7), coordinate(18), coordinate(40),
                    coordinate(20), Color::rgba(255, 242, 190, 218));
        if (icon == Icon::app) {
            raster.circle(coordinate(35), coordinate(34), coordinate(7),
                          Color::rgba(91, 50, 120));
            raster.circle(coordinate(34), coordinate(32), coordinate(5),
                          Color::rgba(176, 126, 203));
            raster.line(coordinate(31), coordinate(34), coordinate(39),
                        coordinate(34), Color::rgba(255, 255, 255), stroke());
            raster.line(coordinate(35), coordinate(30), coordinate(35),
                        coordinate(38), Color::rgba(255, 255, 255), stroke());
        }
        return raster;
    }

    if (icon == Icon::home) {
        raster.triangle({24.0 * scale, 5.0 * scale},
                        {4.0 * scale, 24.0 * scale},
                        {44.0 * scale, 24.0 * scale},
                        Color::rgba(233, 237, 240));
        raster.gradient_rect(coordinate(12), coordinate(20), coordinate(36),
                             coordinate(43), folder_front, folder_deep);
        raster.outline(coordinate(12), coordinate(20), coordinate(36),
                       coordinate(43), Color::rgba(83, 106, 120));
        raster.rect(coordinate(21), coordinate(31), coordinate(28),
                    coordinate(43), Color::rgba(116, 80, 55));
        return raster;
    }

    if (icon == Icon::drive) {
        raster.gradient_rect(coordinate(7), coordinate(10), coordinate(41),
                             coordinate(31), Color::rgba(239, 245, 248),
                             Color::rgba(113, 130, 141));
        raster.outline(coordinate(7), coordinate(10), coordinate(41),
                       coordinate(31), Color::rgba(81, 100, 111));
        raster.gradient_rect(coordinate(5), coordinate(30), coordinate(43),
                             coordinate(41), Color::rgba(164, 179, 188),
                             Color::rgba(105, 121, 130));
        raster.outline(coordinate(5), coordinate(30), coordinate(43),
                       coordinate(41), Color::rgba(81, 100, 111));
        raster.circle(coordinate(36), coordinate(35), coordinate(2),
                      Color::rgba(113, 207, 115));
        raster.rect(coordinate(10), coordinate(34), coordinate(28),
                    coordinate(36), Color::rgba(220, 230, 235));
        return raster;
    }

    if (icon == Icon::archive) {
        raster.gradient_rect(coordinate(7), coordinate(14), coordinate(41),
                             coordinate(43), Color::rgba(214, 174, 118),
                             Color::rgba(155, 103, 47));
        raster.outline(coordinate(7), coordinate(14), coordinate(41),
                       coordinate(43), Color::rgba(110, 74, 34));
        raster.gradient_rect(coordinate(9), coordinate(7), coordinate(39),
                             coordinate(19), Color::rgba(232, 196, 146),
                             Color::rgba(174, 121, 64));
        raster.outline(coordinate(9), coordinate(7), coordinate(39),
                       coordinate(19), Color::rgba(110, 74, 34));
        raster.rect(coordinate(22), coordinate(7), coordinate(29),
                    coordinate(33), Color::rgba(103, 125, 138));
        raster.rect(coordinate(21), coordinate(30), coordinate(30),
                    coordinate(38), Color::rgba(232, 238, 241));
        raster.outline(coordinate(21), coordinate(30), coordinate(30),
                       coordinate(38), Color::rgba(64, 80, 90));
        return raster;
    }

    if (icon == Icon::audio) {
        for (int x = coordinate(18); x < coordinate(23); ++x) {
            raster.gradient_rect(x, coordinate(8), x + 1, coordinate(37),
                                 violet_top, violet_bottom);
        }
        raster.line(coordinate(21), coordinate(12), coordinate(39),
                    coordinate(7), Color::rgba(109, 65, 137), stroke(2));
        raster.line(coordinate(38), coordinate(7), coordinate(38),
                    coordinate(31), Color::rgba(109, 65, 137), stroke(2));
        raster.circle(coordinate(13), coordinate(37), coordinate(7), violet_bottom);
        raster.circle(coordinate(31), coordinate(32), coordinate(7),
                      Color::rgba(132, 76, 164));
        return raster;
    }

    if (icon == Icon::transfer) {
        Raster folder = make_icon(Icon::folder, extent);
        raster.pixels = std::move(folder.pixels);
        raster.line(coordinate(7), coordinate(34), coordinate(34),
                    coordinate(34), dark_blue, stroke(2));
        raster.triangle({42.0 * scale, 34.0 * scale},
                        {31.0 * scale, 27.0 * scale},
                        {31.0 * scale, 41.0 * scale}, dark_blue);
        return raster;
    }

    if (icon == Icon::remove) {
        raster.gradient_rect(coordinate(11), coordinate(13), coordinate(37),
                             coordinate(43), Color::rgba(238, 244, 247),
                             Color::rgba(158, 174, 184));
        raster.outline(coordinate(11), coordinate(13), coordinate(37),
                       coordinate(43), Color::rgba(89, 107, 117));
        raster.gradient_rect(coordinate(8), coordinate(8), coordinate(40),
                             coordinate(14), Color::rgba(255, 255, 255),
                             Color::rgba(185, 199, 208));
        raster.outline(coordinate(8), coordinate(8), coordinate(40),
                       coordinate(14), Color::rgba(89, 107, 117));
        raster.rect(coordinate(17), coordinate(4), coordinate(31),
                    coordinate(9), Color::rgba(201, 214, 221));
        raster.line(coordinate(19), coordinate(20), coordinate(19),
                    coordinate(36), Color::rgba(122, 79, 96), stroke());
        raster.line(coordinate(29), coordinate(20), coordinate(29),
                    coordinate(36), Color::rgba(122, 79, 96), stroke());
        return raster;
    }

    if (icon == Icon::view || icon == Icon::sort) {
        raster.gradient_rect(coordinate(5), coordinate(5), coordinate(43),
                             coordinate(43), paper_top, paper_bottom);
        raster.outline(coordinate(5), coordinate(5), coordinate(43),
                       coordinate(43), Color::rgba(78, 104, 120));
        if (icon == Icon::view) {
            for (int row = 0; row < 3; ++row) {
                for (int column = 0; column < 3; ++column) {
                    raster.rect(coordinate(9 + column * 11),
                                coordinate(9 + row * 11),
                                coordinate(17 + column * 11),
                                coordinate(17 + row * 11),
                                Color::rgba(106, 154, 190));
                }
            }
        } else {
            for (int row = 0; row < 4; ++row) {
                raster.rect(coordinate(9), coordinate(10 + row * 8),
                            coordinate(15), coordinate(16 + row * 8),
                            Color::rgba(106, 154, 190));
                raster.rect(coordinate(19), coordinate(12 + row * 8),
                            coordinate(38), coordinate(14 + row * 8),
                            Color::rgba(71, 103, 124));
            }
        }
        return raster;
    }

    if (icon == Icon::properties) {
        raster.circle(coordinate(24), coordinate(24), coordinate(18), dark_blue);
        raster.circle(coordinate(23), coordinate(21), coordinate(14),
                      Color::rgba(92, 164, 211));
        raster.circle(coordinate(24), coordinate(15), coordinate(2), paper_top);
        raster.rect(coordinate(22), coordinate(21), coordinate(26),
                    coordinate(35), paper_top);
        return raster;
    }

    raster.rect(coordinate(10), coordinate(5), coordinate(39), coordinate(44), shadow);
    raster.gradient_rect(coordinate(9), coordinate(3), coordinate(39),
                         coordinate(44), paper_top, paper_bottom);
    raster.outline(coordinate(9), coordinate(3), coordinate(39),
                   coordinate(44), Color::rgba(99, 121, 136));
    raster.triangle({30.0 * scale, 3.0 * scale},
                    {39.0 * scale, 12.0 * scale},
                    {30.0 * scale, 12.0 * scale},
                    Color::rgba(199, 217, 228));
    if (icon == Icon::image) {
        raster.gradient_rect(coordinate(13), coordinate(15), coordinate(35),
                             coordinate(37), Color::rgba(217, 239, 247),
                             Color::rgba(143, 190, 210));
        raster.triangle({13.0 * scale, 37.0 * scale},
                        {22.0 * scale, 24.0 * scale},
                        {30.0 * scale, 37.0 * scale},
                        Color::rgba(108, 156, 80));
        raster.circle(coordinate(29), coordinate(20), coordinate(4),
                      Color::rgba(241, 173, 59));
    } else if (icon == Icon::code) {
        raster.line(coordinate(21), coordinate(17), coordinate(14),
                    coordinate(25), Color::rgba(49, 116, 91), stroke(2));
        raster.line(coordinate(14), coordinate(25), coordinate(21),
                    coordinate(33), Color::rgba(49, 116, 91), stroke(2));
        raster.line(coordinate(27), coordinate(17), coordinate(34),
                    coordinate(25), Color::rgba(144, 97, 172), stroke(2));
        raster.line(coordinate(34), coordinate(25), coordinate(27),
                    coordinate(33), Color::rgba(144, 97, 172), stroke(2));
    } else {
        raster.rect(coordinate(14), coordinate(20), coordinate(34),
                    coordinate(22), Color::rgba(93, 125, 146));
        raster.rect(coordinate(14), coordinate(26), coordinate(32),
                    coordinate(28), Color::rgba(93, 125, 146));
        raster.rect(coordinate(14), coordinate(32), coordinate(34),
                    coordinate(34), Color::rgba(93, 125, 146));
    }
    return raster;
}

} // namespace detail

[[nodiscard]] inline std::shared_ptr<gui_forms::ImageList> make_image_list(
    gui_forms::Window& window, const double logical_extent) {
    auto images = std::make_shared<gui_forms::ImageList>(
        window, gui_forms::Size{logical_extent, logical_extent});
    const std::uint32_t first_extent = static_cast<std::uint32_t>(
        std::lround(logical_extent));
    const std::uint32_t dense_extent = first_extent * 2U;
    for (const auto& [icon, name] : icon_keys) {
        auto first = detail::make_icon(icon, first_extent);
        const auto first_loaded = window.load_bgra32_premultiplied(
            first_extent, first_extent, first_extent * 4U,
            std::span<const std::byte>(first.pixels));
        if (!first_loaded) {
            throw std::runtime_error("GUI.Forms rejected a generated House icon");
        }
        images->add_image(std::string(name), first_loaded.image, 1.0);
        auto dense = detail::make_icon(icon, dense_extent);
        const auto dense_loaded = window.load_bgra32_premultiplied(
            dense_extent, dense_extent, dense_extent * 4U,
            std::span<const std::byte>(dense.pixels));
        if (!dense_loaded) {
            throw std::runtime_error("GUI.Forms rejected a generated dense House icon");
        }
        images->add_image(std::string(name), dense_loaded.image, 2.0);
    }
    return images;
}

} // namespace file_manager::house_art
