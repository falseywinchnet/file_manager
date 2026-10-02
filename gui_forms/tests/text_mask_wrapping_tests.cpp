#include "gui_forms/text_mask.hpp"

#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {
using namespace gui_forms;
void require(const bool condition, const char* const message) {
    if (!condition) throw std::runtime_error(message);
}
std::vector<std::byte> read_font(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    require(static_cast<bool>(input), "approved font opens");
    const std::streamsize length = input.tellg();
    require(length > 0 && length <= 4*1024*1024, "approved font length");
    std::vector<std::byte> bytes(static_cast<std::size_t>(length));
    input.seekg(0);
    input.read(reinterpret_cast<char*>(bytes.data()), length);
    require(static_cast<bool>(input), "approved font read");
    return bytes;
}
struct Fixture final {
    TextMaskService service{};
    EncodedFontLease bank{};
    std::unique_ptr<TextMaskSession> session{};
    Fixture(const std::span<const std::byte> carlito, const std::span<const std::byte> cousine,
        const std::span<const std::byte> arabic) {
        const std::array<PreparedFontSource, 3> sources{{{.encoded = carlito}, {.encoded = cousine}, {.encoded = arabic}}};
        TextMaskResult result = service.create_font_bank(sources, bank);
        require(result.status == TextMaskStatus::success, "three approved faces admitted");
        result = service.open_session(nullptr, session);
        require(result.status == TextMaskStatus::success, "native wrapping session opens");
    }
    TextMaskResult prepare(const TextMaskRequest& request, TextMaskLease& output) {
        TextMaskRequestId id{};
        TextMaskResult result = (*session).submit(bank, request, id);
        if (result.status != TextMaskStatus::success) return result;
        const std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
        for (;;) {
            result = (*session).take(id, output);
            if (result.status != TextMaskStatus::pending) return result;
            require(std::chrono::steady_clock::now() < deadline, "native completion deadline");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
};
void check_source_cover(const TextMaskLease& mask) {
    const std::span<const TextMaskLine> lines = mask.lines();
    std::uint32_t previous = 0;
    for (std::size_t index = 0; index < lines.size(); ++index) {
        const TextMaskLine& line = lines[index];
        require(line.source_begin == previous && line.source_begin <= line.text_end && line.text_end <= line.consumed_end,
            "line records cover exact source monotonically");
        previous = line.consumed_end;
    }
    require(previous == mask.source_utf8().size(), "line records consume exact entire source");
}
void same_logical_lines(const TextMaskLease& first, const TextMaskLease& second) {
    const std::span<const TextMaskLine> a = first.lines();
    const std::span<const TextMaskLine> b = second.lines();
    require(a.size() == b.size(), "line count invariant across scale and raster mode");
    for (std::size_t index = 0; index < a.size(); ++index) {
        require(a[index].source_begin == b[index].source_begin && a[index].text_end == b[index].text_end &&
            a[index].consumed_end == b[index].consumed_end && a[index].baseline == b[index].baseline &&
            a[index].advance == b[index].advance && a[index].ascent == b[index].ascent && a[index].descent == b[index].descent &&
            a[index].hard_break == b[index].hard_break && a[index].horizontal_overflow == b[index].horizontal_overflow,
            "logical line metrics and source boundaries exactly invariant");
    }
}
void test_empty_and_hard_breaks(Fixture& fixture) {
    TextMaskLease mask{};
    TextMaskResult result = fixture.prepare({}, mask);
    require(result.status == TextMaskStatus::success && mask.has_value() && mask.coverage().empty(), "empty input is a successful zero-ink owner");
    require(mask.lines().size() == 1 && mask.lines()[0].advance == 0.0 && mask.metrics().logical_height > 0.0,
        "empty line has primary font height and zero advance");
    const TextMaskBudgetSnapshot empty_budget = fixture.service.budget_snapshot();
    require(empty_budget.shaping_payload.peak_admitted == 0, "empty input allocates no glyph output buffers");
    result = fixture.prepare({.utf8 = "one\r\n\nthree\n"}, mask);
    require(result.status == TextMaskStatus::success, "LF CRLF empty trailing lines render");
    const std::span<const TextMaskLine> lines = mask.lines();
    require(lines.size() == 4, "hard breaks retain empty and trailing lines");
    require(lines[0].text_end == 3 && lines[0].consumed_end == 5 && lines[0].hard_break, "CRLF consumes two source bytes");
    require(lines[1].source_begin == 5 && lines[1].text_end == 5 && lines[1].consumed_end == 6, "empty hard line retains offset");
    require(lines[3].source_begin == 12 && lines[3].consumed_end == 12 && !lines[3].hard_break, "trailing empty line retained");
    double expected_height = 0.0;
    for (std::size_t index = 0; index < lines.size(); ++index) expected_height += lines[index].ascent + lines[index].descent;
    expected_height += 3.0 * static_cast<double>(mask.metrics().additional_gap_64) / 64.0;
    require(std::abs(mask.metrics().logical_height - expected_height) < 1e-10, "one gap between lines and none after last");
    check_source_cover(mask);
}
void test_wrap_scale_and_mono(Fixture& fixture) {
    const std::string text = "alpha beta gamma delta";
    TextMaskLease baseline{};
    TextMaskResult result = fixture.prepare({.utf8 = text, .primary_face = 1, .wrap_width = 60.0}, baseline);
    require(result.status == TextMaskStatus::success && baseline.lines().size() == 4, "greedy word wrapping produces four words");
    require(baseline.lines()[0].text_end == 5 && baseline.lines()[0].consumed_end == 6, "soft space consumed without trailing advance");
    check_source_cover(baseline);
    const std::array<double, 7> scales{0.5, 1.0, 1.25, 1.5, 2.0, 3.0, 4.0};
    for (std::size_t index = 0; index < scales.size(); ++index) {
        TextMaskLease candidate{};
        result = fixture.prepare({.utf8 = text, .primary_face = 1, .wrap_width = 60.0, .device_scale = scales[index]}, candidate);
        require(result.status == TextMaskStatus::success, "fractional gray scale renders");
        same_logical_lines(baseline, candidate);
    }
    for (unsigned scale = 1; scale <= 4; ++scale) {
        TextMaskLease candidate{};
        result = fixture.prepare({.utf8 = text, .primary_face = 1, .wrap_width = 60.0, .device_scale = static_cast<double>(scale),
            .raster = TextMaskRaster::true_mono}, candidate);
        require(result.status == TextMaskStatus::success, "integer mono scale renders");
        same_logical_lines(baseline, candidate);
        for (const std::uint8_t coverage : candidate.coverage()) require(coverage == 0 || coverage == 255, "public mono mask contains only binary coverage");
    }
}
void test_clusters_fallback_and_unicode(Fixture& fixture) {
    TextMaskLease mask{};
    const std::string combining = "e\xCC\x81";
    TextMaskResult result = fixture.prepare({.utf8 = combining, .wrap_width = 1.0}, mask);
    require(result.status == TextMaskStatus::success && mask.lines().size() == 1 && mask.lines()[0].horizontal_overflow,
        "Cousine fallback retains overwide combining cluster whole");
    require(mask.lines()[0].text_end == combining.size() && !mask.coverage().empty(), "combining bytes and ink retained");
    const std::string joined = "office";
    result = fixture.prepare({.utf8 = joined, .wrap_width = 1.0}, mask);
    require(result.status == TextMaskStatus::success && mask.metrics().horizontal_overflow, "overwide shaped units explicitly reported");
    check_source_cover(mask);
    const std::string spaces = "  a  b";
    result = fixture.prepare({.utf8 = spaces, .primary_face = 1}, mask);
    require(result.status == TextMaskStatus::success && mask.lines().size() == 1, "leading and repeated spaces preserved");
    const double spaced_advance = mask.lines()[0].advance;
    result = fixture.prepare({.utf8 = "ab", .primary_face = 1}, mask);
    require(result.status == TextMaskStatus::success && spaced_advance > mask.lines()[0].advance * 2.5, "spaces retain logical advances");
    const std::string no_break_space = "a\xC2\xA0" "b c";
    result = fixture.prepare({.utf8 = no_break_space, .primary_face = 1, .wrap_width = 35.0}, mask);
    require(result.status == TextMaskStatus::success && mask.lines()[0].text_end == 4 && mask.lines()[0].consumed_end == 5,
        "Unicode NBSP stays within a word while ordinary space permits wrap");
    const std::string arabic = "\xD8\xA8\xD8\xA8 \xD8\xA8\xD8\xA8";
    result = fixture.prepare({.utf8 = arabic, .primary_face = 2, .wrap_width = 25.0}, mask);
    require(result.status == TextMaskStatus::success && !mask.coverage().empty(), "approved Arabic font renders contextual wrapped text");
    check_source_cover(mask);
}
void test_typed_limits(Fixture& fixture) {
    TextMaskLease mask{};
    TextMaskResult result = fixture.prepare({.utf8 = "kept"}, mask);
    require(result.status == TextMaskStatus::success, "preserved output baseline");
    const std::string wide(1000, 'W');
    result = fixture.prepare({.utf8 = wide, .size = 128.0, .device_scale = 4.0}, mask);
    require(result.status == TextMaskStatus::limit_exceeded && result.limit == TextMaskLimit::mask_dimension && mask.source_utf8() == "kept",
        "device-axis limit refuses without replacing visible result");
    std::string area{};
    for (unsigned line = 0; line < 32; ++line) { area.append(60, 'W'); if (line != 31) area.push_back('\n'); }
    result = fixture.prepare({.utf8 = area, .primary_face = 1, .size = 32.0, .device_scale = 2.0}, mask);
    require(result.status == TextMaskStatus::limit_exceeded && result.limit == TextMaskLimit::mask_bytes, "per-mask coverage limit before allocation");
    const std::string lines(257, 'a');
    result = fixture.prepare({.utf8 = lines, .wrap_width = 1.0}, mask);
    require(result.status == TextMaskStatus::limit_exceeded && result.limit == TextMaskLimit::line_count, "soft lines enforce256 limit");
    std::string work{};
    for (unsigned word = 0; word < 5000; ++word) work += "a ";
    result = fixture.prepare({.utf8 = work, .primary_face = 1, .wrap_width = 8192.0}, mask);
    require(result.status == TextMaskStatus::limit_exceeded && result.limit == TextMaskLimit::shape_work, "repeated contextual trials hit explicit native-work limit");
    require(mask.source_utf8() == "kept", "all late failures preserve previous immutable mask");
    const TextMaskBudgetSnapshot usage = fixture.service.budget_snapshot();
    require(usage.shaping_payload.live == 0 && usage.workspace.live == 0 && usage.workspace.reserved == 0,
        "native job releases all first-party transient workspace");
}
}

int main(const int argc, char** const argv) {
    try {
        require(argc == 3, "pass font directory and approved Amiri path");
        const std::filesystem::path directory(argv[1]);
        const std::vector<std::byte> carlito = read_font(directory / "Carlito-Regular.ttf");
        const std::vector<std::byte> cousine = read_font(directory / "Cousine-Regular.ttf");
        const std::vector<std::byte> arabic = read_font(std::filesystem::path(argv[2]));
        Fixture fixture(carlito, cousine, arabic);
        test_empty_and_hard_breaks(fixture);
        test_wrap_scale_and_mono(fixture);
        test_clusters_fallback_and_unicode(fixture);
        test_typed_limits(fixture);
        std::cout << "text mask public wrapping: four groups passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& failure) {
        std::cerr << failure.what() << '\n';
        return EXIT_FAILURE;
    }
}
