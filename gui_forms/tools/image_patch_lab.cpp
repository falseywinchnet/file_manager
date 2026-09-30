#include "gui_forms/resources.hpp"
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

int main() {
    constexpr std::uint32_t edge = 2048U;
    constexpr std::uint32_t patch_edge = 16U;
    constexpr std::size_t iterations = 100U;
    gui_forms::ImageRegistry registry;
    std::vector<std::byte> pixels(static_cast<std::size_t>(edge) * edge * 4U);
    std::array<std::byte, patch_edge * patch_edge * 4U> patch{};
    gui_forms::ImageLoadResult current = registry.load_bgra32_premultiplied(
        edge, edge, edge * 4U, pixels);
    if (!current) throw std::runtime_error("image load failed");
    const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    for (std::size_t iteration = 0; iteration < iterations; ++iteration) {
        patch[0] = static_cast<std::byte>(iteration);
        current = registry.patch_bgra32_premultiplied(
            current.image, 512U, 512U, patch_edge, patch_edge, patch_edge * 4U, patch);
        if (!current) throw std::runtime_error("image patch failed");
    }
    const std::chrono::steady_clock::time_point finish = std::chrono::steady_clock::now();
    const std::chrono::nanoseconds elapsed =
        std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start);
    std::cout << "image_edge,patch_edge,iterations,total_ns,ns_per_patch,revision\n"
              << edge << ',' << patch_edge << ',' << iterations << ',' << elapsed.count()
              << ',' << static_cast<double>(elapsed.count()) / iterations
              << ',' << registry.snapshot().revision << '\n';
}
