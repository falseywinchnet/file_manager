#include "gui_forms/basic_controls.hpp"
#include "gui_forms/window.hpp"
#include "harfbuzz_font_engine.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
using namespace gui_forms;
using Clock = std::chrono::steady_clock;

class ProbeMetrics final : public TextMetricsProvider {
public:
    void load(const std::filesystem::path& path) {
        std::ifstream stream(path, std::ios::binary | std::ios::ate);
        const std::streamsize size = stream.tellg();
        if (!stream || size <= 0 || size > 4 * 1024 * 1024) {
            throw std::runtime_error("font admission failed");
        }
        const std::size_t count = static_cast<std::size_t>(size);
        std::vector<std::byte> bytes(count);
        stream.seekg(0);
        char* destination = reinterpret_cast<char*>(bytes.data());
        stream.read(destination, size);
        if (!stream) { throw std::runtime_error("font read failed"); }
        const std::shared_ptr<const std::vector<std::byte>> owner =
            std::make_shared<const std::vector<std::byte>>(std::move(bytes));
        const std::optional<FontFaceId> face = engine_.register_shared_typeface(
            FontRole::content, 400, false, owner);
        if (!face) { throw std::runtime_error("font registration failed"); }
    }

    ResolvedTextLayout resolve_text_layout_utf8(
        const std::string_view text, const FontSpec font) override {
        ++calls;
        bytes += text.size();
        ResolvedTextLayout result = engine_.resolve(text, font);
        if (!result.exact()) { throw std::runtime_error("inexact probe font coverage"); }
        return result;
    }

    std::size_t calls{};
    std::size_t bytes{};

private:
    render::text::HarfBuzzFontEngine engine_{};
};

void measure(ProbeMetrics& provider, const std::string_view name,
             const std::string& text, const Rect requested) {
    const std::shared_ptr<Label> label = make_control<Label>(StableId("probe.label"), text);
    (*label).set_requested_bounds(requested);
    (*label).set_font({FontRole::content, 13.0, 400, false});
    (*label).set_use_mnemonic(false);
    (*label).set_text_wrapping(TextWrapping::word);
    (*label).set_maximum_lines(7U);
    // The caller's provider owns fonts and outlives this borrowed registration.
    Window window(label, {190.0, 108.0});
    window.set_text_metrics_provider(&provider);
    const Size available{190.0, 108.0};
    const Size expected = (*label).measure(available);
    for (std::size_t index = 0U; index < 3U; ++index) {
        const Size warm = (*label).measure(available);
        if (warm != expected) { throw std::runtime_error("unstable warm dimensions"); }
    }
    provider.calls = 0U;
    provider.bytes = 0U;
    constexpr std::size_t sample_count = 100U;
    std::array<double, sample_count> samples{};
    for (std::size_t index = 0U; index < sample_count; ++index) {
        const Clock::time_point start = Clock::now();
        const Size actual = (*label).measure(available);
        const Clock::time_point end = Clock::now();
        const Clock::duration elapsed = end - start;
        const std::chrono::duration<double, std::micro> duration(elapsed);
        samples[index] = duration.count();
        if (actual != expected) { throw std::runtime_error("measurement changed dimensions"); }
    }
    std::sort(samples.begin(), samples.end());
    std::cout << name << ',' << text.size() << ',' << sample_count << ','
              << samples[49U] << ',' << samples[94U] << ',' << samples[98U] << ','
              << samples[99U] << ',' << provider.calls << ',' << provider.bytes << ','
              << expected.width << ',' << expected.height << '\n';
}
} // namespace

int main(const int argc, char* argv[]) {
    try {
        if (argc != 2) { throw std::runtime_error("usage: label_layout_probe Carlito-Regular.ttf"); }
        const std::filesystem::path font(argv[1]);
        ProbeMetrics provider{};
        provider.load(font);
        std::string words{};
        words.reserve(65536U);
        for (std::size_t index = 0U; index < 4096U; ++index) { words.append("one two three x "); }
        const std::string unbroken(65536U, 'x');
        const std::string short_text("one two three four");
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "workload,source_bytes,samples,p50_us,p95_us,p99_us,max_us,resolve_calls,resolve_bytes,width,height\n";
        measure(provider, "fixed-words", words, {0.0, 0.0, 190.0, 108.0});
        measure(provider, "fixed-unbroken", unbroken, {0.0, 0.0, 190.0, 108.0});
        measure(provider, "fixed-short", short_text, {0.0, 0.0, 190.0, 108.0});
        measure(provider, "content-height-control", short_text, {0.0, 0.0, 80.0, 0.0});
        measure(provider, "content-width-control", short_text, {0.0, 0.0, 0.0, 40.0});
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
