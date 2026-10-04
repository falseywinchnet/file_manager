#include "gui_forms/gui_forms.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
constexpr std::uint32_t width = 1024U;
constexpr std::uint64_t row_bytes = width * 4U;
constexpr std::size_t sample_count = 61U;
using Samples = std::array<double, sample_count>;

[[nodiscard]] std::vector<std::byte> read_fixture(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input) throw std::runtime_error("fixture open failed");
    const std::streamsize extent = input.tellg();
    if (extent <= 0 || extent > 16 * 1024 * 1024) throw std::runtime_error("fixture extent refused");
    std::vector<std::byte> bytes(static_cast<std::size_t>(extent));
    input.seekg(0);
    input.read(reinterpret_cast<char*>(bytes.data()), extent);
    if (!input) throw std::runtime_error("fixture read failed");
    return bytes;
}

void retire(gui_forms::Window& window, const gui_forms::ImageId image) {
    const bool removed = window.remove_image(image);
    const gui_forms::ImageRegistrySnapshot state = window.image_resource_snapshot();
    if (!removed || state.resource_count != 0U || state.encoded_bytes != 0U || state.decoded_bytes != 0U) {
        throw std::runtime_error("registry accounting did not retire the admitted image");
    }
}

// The view is borrowed only inside this call, before the caller removes the image.
void check_owned_bytes(const gui_forms::Window& window, const gui_forms::ImageId image,
    const std::span<const std::byte> source) {
    const std::optional<gui_forms::ImageResourceView> view = window.image_resources().find(image);
    if (!view || (*view).encoded.size() != source.size() || (*view).encoded.data() == source.data() ||
        (*view).encoded.front() != std::byte{198} || (*view).encoded.back() != std::byte{255}) {
        throw std::runtime_error("registry did not independently own the prepared raster");
    }
}

void check_ownership(gui_forms::Window& window, std::vector<std::byte>& pixels,
    const std::uint32_t height) {
    const gui_forms::ImageLoadResult loaded = window.load_bgra32_premultiplied(width, height, row_bytes, pixels);
    if (!loaded) throw std::runtime_error("ownership control rejected");
    pixels.front() = std::byte{0};
    check_owned_bytes(window, loaded.image, pixels);
    pixels.front() = std::byte{198};
    const gui_forms::ImageRegistrySnapshot state = window.image_resource_snapshot();
    if (state.resource_count != 1U || state.encoded_bytes != pixels.size() || state.decoded_bytes != pixels.size()) {
        throw std::runtime_error("raw resource accounting differs from its declared extents");
    }
    retire(window, loaded.image);
    if (window.image_resources().find(loaded.image)) throw std::runtime_error("removed image ID remains valid");
}

[[nodiscard]] double admit_raw(gui_forms::Window& window, const std::span<const std::byte> pixels,
    const std::uint32_t height) {
    const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    const gui_forms::ImageLoadResult result = window.load_bgra32_premultiplied(width, height, row_bytes, pixels);
    const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    if (!result) throw std::runtime_error("raw admission refused");
    const std::chrono::duration<double, std::milli> elapsed = end - begin;
    retire(window, result.image);
    const double milliseconds = elapsed.count();
    return milliseconds;
}

[[nodiscard]] double admit_png(gui_forms::Window& window, const std::span<const std::byte> encoded) {
    const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    const gui_forms::ImageLoadResult result = window.load_png(encoded);
    const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    if (!result) throw std::runtime_error("PNG admission refused");
    const std::chrono::duration<double, std::milli> elapsed = end - begin;
    retire(window, result.image);
    const double milliseconds = elapsed.count();
    return milliseconds;
}

void report(const char* const format, const std::uint32_t height, const std::size_t input_bytes,
    const Samples& samples) {
    Samples ordered = samples; // Owned copy; sorting is outside every timed call.
    std::sort(ordered.begin(), ordered.end());
    constexpr std::size_t median_index = (sample_count * 50U + 99U) / 100U - 1U;
    constexpr std::size_t p95_index = (sample_count * 95U + 99U) / 100U - 1U;
    constexpr std::size_t p99_index = (sample_count * 99U + 99U) / 100U - 1U;
    std::cout << format << " height=" << height << " input_bytes=" << input_bytes
              << " p50_ms=" << ordered[median_index] << " p95_ms=" << ordered[p95_index]
              << " p99_ms=" << ordered[p99_index] << " samples_ms=";
    for (const double sample : samples) std::cout << sample << ',';
    std::cout << '\n';
}

void run_case(gui_forms::Window& window, const std::filesystem::path& root, const std::uint32_t height) {
    const std::filesystem::path path = root / ("rgba-" + std::to_string(height) + ".png");
    const std::vector<std::byte> encoded = read_fixture(path);
    const std::filesystem::path compressed_path = root / ("deflate-" + std::to_string(height) + ".png");
    const std::vector<std::byte> compressed = read_fixture(compressed_path);
    const gui_forms::PngValidationResult validated = gui_forms::validate_png(encoded);
    const gui_forms::PngValidationResult compressed_validated = gui_forms::validate_png(compressed);
    if (!validated || validated.metadata.width != width || validated.metadata.height != height ||
        !compressed_validated || compressed_validated.metadata.width != width ||
        compressed_validated.metadata.height != height) {
        throw std::runtime_error("fixture does not match the declared workload");
    }
    const std::size_t pixel_bytes = static_cast<std::size_t>(row_bytes) * height;
    std::vector<std::byte> pixels(pixel_bytes);
    for (std::size_t offset = 0U; offset < pixel_bytes; offset += 4U) {
        pixels[offset] = std::byte{198};
        pixels[offset + 1U] = std::byte{226};
        pixels[offset + 2U] = std::byte{12};
        pixels[offset + 3U] = std::byte{255};
    }
    check_ownership(window, pixels, height);
    // All paths are warmed before collection. Rotate the order to reduce
    // order bias; this is admission-only work, with no native renderer attached.
    (void)admit_png(window, encoded);
    (void)admit_png(window, compressed);
    (void)admit_raw(window, pixels, height);
    Samples raw{};
    Samples png{};
    Samples deflate{};
    for (std::size_t sample = 0U; sample < sample_count; ++sample) {
        if (sample % 3U == 0U) {
            raw[sample] = admit_raw(window, pixels, height);
            png[sample] = admit_png(window, encoded);
            deflate[sample] = admit_png(window, compressed);
        } else if (sample % 3U == 1U) {
            png[sample] = admit_png(window, encoded);
            deflate[sample] = admit_png(window, compressed);
            raw[sample] = admit_raw(window, pixels, height);
        } else {
            deflate[sample] = admit_png(window, compressed);
            raw[sample] = admit_raw(window, pixels, height);
            png[sample] = admit_png(window, encoded);
        }
    }
    report("BGRA", height, pixels.size(), raw);
    report("PNG-store", height, encoded.size(), png);
    report("PNG-deflate", height, compressed.size(), deflate);
    std::cout << "ownership height=" << height << " independent_copy=passed stale_id=refused registry_retirement=passed\n";
}
} // namespace

int main(const int count, char** const arguments) {
    try {
        if (count != 2 && count != 3) throw std::runtime_error("expected a generated fixture directory and optional --reverse");
        const bool reverse = count == 3;
        if (reverse && std::string_view(arguments[2]) != "--reverse") throw std::runtime_error("unknown option");
        const std::filesystem::path root(arguments[1]);
        const gui_forms::StableId root_id("preview.admission.root");
        const std::shared_ptr<gui_forms::Control> panel = std::make_shared<gui_forms::Control>(root_id);
        gui_forms::Window window(panel, {640.0, 480.0});
        std::cout << std::fixed << std::setprecision(6);
        if (reverse) {
            run_case(window, root, 1024U);
            run_case(window, root, 256U);
            run_case(window, root, 64U);
        } else {
            run_case(window, root, 64U);
            run_case(window, root, 256U);
            run_case(window, root, 1024U);
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
