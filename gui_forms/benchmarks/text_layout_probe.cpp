#include "harfbuzz_font_engine.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
using namespace gui_forms;
using namespace gui_forms::render::text;
using Clock = std::chrono::steady_clock;

void require(bool valid, const char* message) {
    if (!valid) { throw std::runtime_error(message); }
}

std::vector<std::byte> read_font(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    require(static_cast<bool>(stream), "font open failed");
    const std::streamsize length = stream.tellg();
    require(length > 0 && length <= 16 * 1024 * 1024, "fixed font exceeds probe admission");
    const std::size_t count = static_cast<std::size_t>(length);
    std::vector<std::byte> bytes(count);
    stream.seekg(0);
    char* destination = reinterpret_cast<char*>(bytes.data());
    stream.read(destination, length);
    require(static_cast<bool>(stream), "font read failed");
    return bytes;
}

std::size_t register_fonts(HarfBuzzFontEngine& engine, const std::filesystem::path& root) {
    const std::array<const char*, 4> names{
        "Carlito-Regular.ttf", "NotoSansArabic-Regular.ttf",
        "NotoSansHebrew-Regular.ttf", "NotoEmoji-Regular.ttf"};
    std::size_t encoded_capacity = 0;
    for (std::size_t index = 0; index < names.size(); ++index) {
        const std::filesystem::path name(names[index]);
        const std::filesystem::path path = root / name;
        std::vector<std::byte> bytes = read_font(path);
        encoded_capacity += bytes.capacity();
        const std::shared_ptr<const std::vector<std::byte>> owner =
            std::make_shared<const std::vector<std::byte>>(std::move(bytes));
        std::optional<FontRole> role{};
        if (index == 0) { role = FontRole::content; }
        const std::optional<FontFaceId> registered = engine.register_shared_typeface(role, 400, false, owner);
        require(registered.has_value(), "fixed font registration failed");
    }
    return encoded_capacity;
}

std::string repeat_complete(std::string_view unit, std::size_t budget) {
    require(!unit.empty(), "empty unit");
    const std::size_t count = budget / unit.size();
    const std::size_t size = count * unit.size();
    std::string text{};
    text.reserve(size);
    for (std::size_t index = 0; index < count; ++index) { text.append(unit); }
    return text;
}

struct ShapeFacts final {
    std::size_t glyphs{};
    std::size_t capacity_bytes{};
    std::uint64_t signature{1469598103934665603ULL};
};

ShapeFacts inspect(const ShapedText& shaped, const TextStore& source) {
    require(!shaped.missing_primary_face, "primary face missing");
    const bool finite_size = std::isfinite(shaped.width) && std::isfinite(shaped.height);
    require(finite_size && shaped.width >= 0 && shaped.height > 0, "nonfinite or empty shape metrics");
    ShapeFacts facts{};
    facts.capacity_bytes = shaped.runs.capacity() * sizeof(ShapedFontRun);
    const Utf8Offset source_end = source.utf8_size();
    const std::size_t run_count = shaped.runs.size();
    for (std::size_t index = 0; index < run_count; ++index) {
        const ShapedFontRun& run = shaped.runs[index];
        require(run.source_range.start.value() <= run.source_range.end.value() &&
                run.source_range.end.value() <= source_end.value(), "run source extent");
        const std::size_t glyph_capacity = run.glyphs.capacity() * sizeof(ShapedGlyph);
        facts.capacity_bytes += glyph_capacity;
        const std::size_t glyph_count = run.glyphs.size();
        facts.glyphs += glyph_count;
        for (std::size_t glyph_index = 0; glyph_index < glyph_count; ++glyph_index) {
            const ShapedGlyph& glyph = run.glyphs[glyph_index];
            const bool boundary = source.is_grapheme_boundary(glyph.cluster);
            require(boundary && glyph.cluster.value() >= run.source_range.start.value() &&
                    glyph.cluster.value() < run.source_range.end.value(), "glyph cluster boundary/range");
            const bool finite = std::isfinite(glyph.x) && std::isfinite(glyph.y) &&
                std::isfinite(glyph.advance_x) && std::isfinite(glyph.advance_y);
            require(finite, "glyph position finite");
            facts.signature ^= glyph.glyph.value;
            facts.signature *= 1099511628211ULL;
            facts.signature ^= glyph.cluster.value();
            facts.signature *= 1099511628211ULL;
        }
    }
    return facts;
}

double elapsed_ms(Clock::time_point start, Clock::time_point end) {
    const Clock::duration elapsed = end - start;
    const std::chrono::duration<double, std::milli> duration(elapsed);
    const double milliseconds = duration.count();
    return milliseconds;
}

void measure(const std::filesystem::path& fonts, std::string_view name, const std::string& input) {
    constexpr std::size_t samples = 31;
    require(input.size() <= 16 * 1024, "probe source work bound");
    const TextStore oracle(input);
    HarfBuzzFontEngine engine{};
    const std::size_t font_bytes = register_fonts(engine, fonts);
    const FontSpec font{FontRole::content, 16.0, 400, false};
    std::vector<double> timings(samples, 0);
    ShapeFacts first_facts{};
    double first_width = 0;
    double first_ms = 0;
    std::size_t missing = 0;
    std::size_t run_count = 0;
    {
        const Clock::time_point start = Clock::now();
        const ShapedText first = engine.shape(input, font);
        const Clock::time_point end = Clock::now();
        first_ms = elapsed_ms(start, end);
        first_facts = inspect(first, oracle);
        first_width = first.width;
        missing = first.missing_clusters;
        run_count = first.runs.size();
    }
    std::size_t maximum_returned_capacity = first_facts.capacity_bytes;
    for (std::size_t index = 0; index < samples; ++index) {
        const Clock::time_point start = Clock::now();
        const ShapedText result = engine.shape(input, font);
        const Clock::time_point end = Clock::now();
        timings[index] = elapsed_ms(start, end);
        const ShapeFacts facts = inspect(result, oracle);
        require(facts.signature == first_facts.signature && facts.glyphs == first_facts.glyphs &&
                result.width == first_width && result.missing_clusters == missing,
                "repeated shape must match deterministic first result");
        maximum_returned_capacity = std::max(maximum_returned_capacity, facts.capacity_bytes);
    }
    std::sort(timings.begin(), timings.end());
    constexpr std::size_t last = samples - 1;
    constexpr std::size_t p50 = last * 50 / 100;
    constexpr std::size_t p95 = last * 95 / 100;
    constexpr std::size_t p99 = last * 99 / 100;
    std::cout << name << ',' << input.size() << ',' << first_ms << ',' << samples << ','
              << timings[p50] << ',' << timings[p95] << ',' << timings[p99] << ','
              << timings[last] << ',' << run_count << ',' << first_facts.glyphs << ','
              << maximum_returned_capacity << ',' << font_bytes << ',' << missing << '\n';
    // Preserve completed cases if the external deadline terminates a later one.
    std::cout.flush();
}

} // namespace

int main(int argc, char** argv) {
    try {
        require(argc == 2, "usage: text_layout_probe FONT_DIRECTORY");
        const std::filesystem::path fonts(argv[1]);
        std::cout << "case,input_bytes,first_ms,samples,p50_ms,p95_ms,p99_ms,worst_ms,runs,glyphs,returned_capacity,font_bytes,missing_clusters\n";
        const std::string tiny = repeat_complete("office abc ", 64);
        const std::string medium = repeat_complete("office abc ", 4096);
        const std::string long_line = repeat_complete("office abc ", 16384);
        const std::string labels = repeat_complete("[BYTE FF]", 16384);
        const std::string bidi = repeat_complete("ABC שלום العربية 123 ", 16384);
        const std::string emoji = repeat_complete("a👩‍💻b ", 16384);
        std::string combining = "a";
        const std::string marks = repeat_complete("\xcc\x81", 16382);
        combining.append(marks);
        measure(fonts, "ascii_small", tiny);
        measure(fonts, "ascii_4k", medium);
        measure(fonts, "ascii_16k", long_line);
        measure(fonts, "labels_16k", labels);
        measure(fonts, "bidi_16k", bidi);
        measure(fonts, "emoji_16k", emoji);
        measure(fonts, "one_grapheme_16k", combining);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
