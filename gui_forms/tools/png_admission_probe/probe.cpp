#include "gui_forms/resources/image_registry/image_registry.hpp"
#include <array>
#include <chrono>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

int main(const int count, char** const arguments) {
    try {
        if (count != 2) throw std::runtime_error("expected one generated PNG path");
        std::ifstream input(arguments[1], std::ios::binary | std::ios::ate);
        if (!input) throw std::runtime_error("cannot open fixture");
        const std::streamsize extent = input.tellg();
        if (extent <= 0 || extent > 16 * 1024 * 1024) throw std::runtime_error("fixture extent");
        std::vector<std::byte> bytes(static_cast<std::size_t>(extent));
        input.seekg(0);
        input.read(reinterpret_cast<char*>(bytes.data()), extent);
        if (!input) throw std::runtime_error("incomplete fixture");
        gui_forms::ImageRegistry registry{};
        std::array<double, 6> validation_samples{};
        for (double& sample : validation_samples) {
            const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            const gui_forms::PngValidationResult validated = gui_forms::validate_png(bytes);
            const std::chrono::steady_clock::time_point finish = std::chrono::steady_clock::now();
            if (!validated) throw std::runtime_error("fixture validation refused");
            const std::chrono::duration<double, std::milli> elapsed = finish - start;
            sample = elapsed.count();
        }
        std::array<double, 6> samples{};
        for (double& sample : samples) {
            const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            const gui_forms::ImageLoadResult loaded = registry.load_png(bytes);
            const std::chrono::steady_clock::time_point finish = std::chrono::steady_clock::now();
            if (!loaded) throw std::runtime_error("fixture refused");
            const std::chrono::duration<double, std::milli> elapsed = finish - start;
            sample = elapsed.count();
            const bool removed = registry.remove(loaded.image);
            if (!removed) throw std::runtime_error("resource retirement failed");
        }
        std::cout << "encoded_bytes=" << bytes.size() << " admission_ms=";
        for (const double sample : samples) std::cout << sample << ' ';
        std::cout << '\n';
        std::cout << "validation_ms=";
        for (const double sample : validation_samples) std::cout << sample << ' ';
        std::cout << '\n';
        return 0;
    } catch (const std::exception& failure) {
        std::cerr << failure.what() << '\n';
        return 1;
    }
}
