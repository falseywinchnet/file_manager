#include "gui_forms/resources.hpp"

#include <cstddef>
#include <cstdint>
#include <span>

#if defined(GUI_FORMS_FUZZ_STANDALONE)
#include <array>
#include <vector>
#endif

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    gui_forms::ImageRegistryLimits limits;
    limits.maximum_encoded_bytes_per_image = 1U * 1024U * 1024U;
    limits.maximum_width = 2048;
    limits.maximum_height = 2048;
    limits.maximum_pixels = 2048ULL * 2048ULL;
    limits.maximum_decoded_bytes_per_image = 16ULL * 1024ULL * 1024ULL;
    limits.maximum_chunks_per_image = 1024;
    const auto* bytes = reinterpret_cast<const std::byte*>(data);
    static_cast<void>(gui_forms::validate_png(
        std::span<const std::byte>(bytes, size), limits));
    return 0;
}

#if defined(GUI_FORMS_FUZZ_STANDALONE)
int main() {
    constexpr std::array<std::uint8_t, 70> seed{
        0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
        0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
        0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0x15, 0xc4, 0x89, 0x00, 0x00, 0x00,
        0x0d, 0x49, 0x44, 0x41, 0x54, 0x78, 0xda, 0x63, 0xf8, 0xcf, 0xc0, 0xf0,
        0x1f, 0x00, 0x05, 0x00, 0x01, 0xff, 0x56, 0xc7, 0x2f, 0x0d, 0x00, 0x00,
        0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
    };
    std::uint64_t state = 0x6a09e667f3bcc909ULL;
    for (std::size_t run = 0; run < 100'000; ++run) {
        state ^= state << 13U;
        state ^= state >> 7U;
        state ^= state << 17U;
        std::vector<std::uint8_t> input(seed.begin(), seed.end());
        const std::size_t mutations = 1U + static_cast<std::size_t>(state % 8U);
        for (std::size_t mutation = 0; mutation < mutations; ++mutation) {
            state ^= state << 13U;
            state ^= state >> 7U;
            state ^= state << 17U;
            if (!input.empty()) {
                const std::size_t offset = static_cast<std::size_t>(state % input.size());
                input[offset] ^= static_cast<std::uint8_t>(1U << (state % 8U));
            }
        }
        if (run % 5U == 0 && !input.empty()) {
            input.resize(static_cast<std::size_t>(state % input.size()));
        } else if (run % 7U == 0) {
            input.insert(input.end(), static_cast<std::size_t>(state % 512U),
                         static_cast<std::uint8_t>(state));
        }
        LLVMFuzzerTestOneInput(input.data(), input.size());
    }
    return 0;
}
#endif
