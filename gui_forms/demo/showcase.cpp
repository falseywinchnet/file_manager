#include "showcase.hpp"

#include "showcase_controls.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace gui_forms::showcase {
namespace {

constexpr std::uint32_t test_card_width = 128U;
constexpr std::uint32_t test_card_height = 80U;

std::vector<std::byte> make_test_card() {
    std::vector<std::byte> pixels(
        static_cast<std::size_t>(test_card_width) * test_card_height * 4U);
    const auto set_pixel = [&pixels](std::uint32_t x, std::uint32_t y,
                                     std::uint8_t red, std::uint8_t green,
                                     std::uint8_t blue) {
        const std::size_t offset =
            (static_cast<std::size_t>(y) * test_card_width + x) * 4U;
        pixels[offset] = static_cast<std::byte>(blue);
        pixels[offset + 1U] = static_cast<std::byte>(green);
        pixels[offset + 2U] = static_cast<std::byte>(red);
        pixels[offset + 3U] = std::byte{0xff};
    };
    for (std::uint32_t y = 0U; y < test_card_height; ++y) {
        for (std::uint32_t x = 0U; x < test_card_width; ++x) {
            const bool checker = ((x / 12U) + (y / 12U)) % 2U == 0U;
            const std::uint8_t lift = checker ? 12U : 0U;
            set_pixel(x, y,
                      static_cast<std::uint8_t>(40U + x * 52U / test_card_width + lift),
                      static_cast<std::uint8_t>(67U + y * 55U / test_card_height + lift),
                      static_cast<std::uint8_t>(91U + x * 42U / test_card_width + lift));
        }
    }
    for (std::uint32_t y = 0U; y < 14U; ++y) {
        for (std::uint32_t x = 0U; x < test_card_width; ++x) {
            set_pixel(x, y, 28U, 52U, 75U);
        }
    }
    constexpr std::array<std::array<std::uint8_t, 3>, 4> swatches{{
        {{39U, 116U, 184U}}, {{49U, 139U, 94U}},
        {{211U, 121U, 42U}}, {{116U, 82U, 153U}},
    }};
    for (std::uint32_t block = 0U; block < swatches.size(); ++block) {
        for (std::uint32_t y = 20U; y < 39U; ++y) {
            for (std::uint32_t x = 8U + block * 30U;
                 x < 31U + block * 30U; ++x) {
                set_pixel(x, y, swatches[block][0], swatches[block][1],
                          swatches[block][2]);
            }
        }
    }
    for (std::uint32_t x = 4U; x < test_card_width - 4U; ++x) {
        const double phase = static_cast<double>(x) * 0.17;
        const auto y = static_cast<std::uint32_t>(
            std::clamp(59.0 + std::sin(phase) * 9.0, 48.0, 72.0));
        set_pixel(x, y, 232U, 241U, 247U);
        if (y + 1U < test_card_height) {
            set_pixel(x, y + 1U, 169U, 207U, 229U);
        }
    }
    return pixels;
}

} // namespace

std::unique_ptr<Window> make_showcase() {
    ShowcaseTree tree = build_showcase_tree();
    auto window = std::make_unique<Window>(std::move(tree.root),
                                           Size{1280.0, 820.0});
    initialize_showcase_runtime(*window);
    const std::vector<std::byte> pixels = make_test_card();
    const ImageLoadResult loaded = window->load_bgra32_premultiplied(
        test_card_width, test_card_height, test_card_width * 4U, pixels);
    if (loaded) {
        for (std::size_t index = 0U; index < 5U; ++index) {
            if (const auto picture = std::dynamic_pointer_cast<PictureBox>(
                    window->find("showcase.images.picture." +
                                 std::to_string(index)))) {
                picture->set_image(loaded.image);
            }
        }
        for (std::size_t index = 0U; index < 3U; ++index) {
            if (const auto picture = std::dynamic_pointer_cast<PictureBox>(
                    window->find("showcase.images.opacity." +
                                 std::to_string(index)))) {
                picture->set_image(loaded.image);
            }
        }
    }
    window->perform_layout();
    return window;
}

} // namespace gui_forms::showcase
