#include "prepared_text_test_support.hpp"
#include "../src/core/text/prepared/prepared_storage.hpp"
#include "../src/host/windows/raster/prepared_text_compositor.hpp"

#include <cstdlib>
#include <iostream>

namespace gui_forms::host {
void run_windows_prepared_text_fixture(const std::span<const std::byte> fonts);
}

namespace {
using namespace prepared_test;

void test_compositor() {
    std::unique_ptr<detail::PreparedMaskStorage> storage = std::make_unique<detail::PreparedMaskStorage>();
    (*storage).width = 8;
    (*storage).height = 8;
    (*storage).pixel_count = 64;
    (*storage).pixels = std::make_unique<std::uint8_t[]>(64);
    std::fill_n((*storage).pixels.get(), 64, static_cast<std::uint8_t>(255));
    GrayTextMask mask{};
    detail::PreparedTextAccess::mask(mask) = std::move(storage);
    std::array<std::uint32_t, 80> pixels{};
    pixels.fill(0xffffffffU);
    const host::detail::PreparedCompositeTarget target{.pixels = pixels, .width = 8, .height = 8, .stride = 10};
    host::detail::PreparedCompositeClip clip{.rect = {2, 2, 4, 4}};
    PreparedTextStatus status = host::detail::composite_prepared_mask(mask, target, clip, {}, 1, {0, 0, 0, 128});
    require(status == PreparedTextStatus::success, "clipped alpha composite");
    for (std::size_t y = 0; y < 8; ++y) {
        for (std::size_t x = 0; x < 10; ++x) {
            const bool changed = x >= 2 && x < 6 && y >= 2 && y < 6;
            const std::uint32_t expected = changed ? 0xff7f7f7fU : 0xffffffffU;
            require(pixels[y * 10 + x] == expected, "alpha/clip/padding contract");
        }
    }
    pixels.fill(0xffffffffU);
    clip = {.rect = {0, 0, 8, 8}, .rounded_rect = {0, 0, 8, 8}, .radius = 4, .rounded = true};
    status = host::detail::composite_prepared_mask(mask, target, clip, {}, 1, {0, 0, 0, 255});
    require(status == PreparedTextStatus::success && pixels[0] == 0xffffffffU && pixels[44] == 0xff000000U,
        "rounded clipping excludes corner and includes center");
    const std::array<std::uint32_t, 80> previous = pixels;
    status = host::detail::composite_prepared_mask(mask, target, clip, {}, 2, {0, 0, 0, 255});
    require(status == PreparedTextStatus::invalid_geometry && pixels == previous, "scale mismatch preserves candidate bytes");
    host::detail::PreparedCompositeTarget invalid = target;
    invalid.stride = 100;
    status = host::detail::composite_prepared_mask(mask, invalid, clip, {}, 1, {0, 0, 0, 255});
    require(status == PreparedTextStatus::invalid_geometry && pixels == previous, "invalid extent preserves candidate bytes");
    pixels.fill(0xffffffffU);
    clip = {.rect = {0, 0, 8, 8}};
    status = host::detail::composite_prepared_mask(mask, target, clip, {0.5, 0}, 1, {0, 0, 0, 255});
    require(status == PreparedTextStatus::success && pixels[0] == 0xffffffffU && pixels[1] == 0xff000000U,
        "positive half-device placement rounds away from zero");
    pixels.fill(0xffffffffU);
    status = host::detail::composite_prepared_mask(mask, target, clip, {-0.5, 0}, 1, {0, 0, 0, 255});
    require(status == PreparedTextStatus::success && pixels[6] == 0xff000000U && pixels[7] == 0xffffffffU,
        "negative half-device placement rounds away from zero");
}
} // namespace

int main(const int argc, char** const argv) {
    try {
        require(argc == 2, "font directory required");
        test_compositor();
        const std::vector<std::byte> bytes = read_font(std::filesystem::path(argv[1]));
        gui_forms::host::run_windows_prepared_text_fixture(bytes);
        std::cout << "Windows prepared text checks passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
