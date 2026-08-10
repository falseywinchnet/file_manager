#include "harfbuzz_font_engine.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using namespace gui_forms;
using namespace gui_forms::render::text;

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

std::vector<std::byte> read_file(const char* path) {
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream) throw std::runtime_error("font fixture could not be opened");
    const std::streamsize length = stream.tellg();
    if (length <= 0) throw std::runtime_error("font fixture is empty");
    std::vector<std::byte> bytes(static_cast<std::size_t>(length));
    stream.seekg(0);
    stream.read(reinterpret_cast<char*>(bytes.data()), length);
    if (!stream) throw std::runtime_error("font fixture could not be read");
    return bytes;
}

bool near(double left, double right, double tolerance) {
    return std::abs(left - right) <= tolerance;
}

bool shaped_text_uses_face(const ShapedText& shaped, FontFaceId id) {
    for (const ShapedFontRun& run : shaped.runs) {
        if (run.face == id) return true;
    }
    return false;
}

void test_owned_registration_style_selection_and_ligatures() {
    HarfBuzzFontEngine engine;
    std::vector<std::byte> regular = read_file(GUI_FORMS_TEST_CARLITO_REGULAR);
    std::vector<std::byte> bold = read_file(GUI_FORMS_TEST_CARLITO_BOLD);
    const std::optional<FontFaceId> regular_id = engine.register_typeface(
        FontRole::content, 400, false, regular);
    const std::optional<FontFaceId> bold_id = engine.register_typeface(
        FontRole::content, 700, false, bold);
    require(regular_id && bold_id && *regular_id != *bold_id &&
                engine.face_count() == 2U,
            "valid scalable faces must receive distinct stable engine IDs");

    std::fill(regular.begin(), regular.end(), std::byte{});
    regular.clear();
    bold.clear();
    const ShapedText normal = engine.shape(
        "office", {FontRole::content, 16.0, 400, false});
    const ShapedText heavy = engine.shape(
        "office", {FontRole::content, 16.0, 700, false});
    require(normal.runs.size() == 1U && normal.runs.front().face == *regular_id &&
                heavy.runs.size() == 1U && heavy.runs.front().face == *bold_id &&
                normal.runs.front().glyphs.size() < 6U &&
                normal.width > 0.0 && normal.height > 0.0,
            "owned bytes must survive caller release, select weight, and shape ligatures");
}

void test_cluster_fallback_is_bounded_and_absolute() {
    HarfBuzzFontEngine engine;
    const std::vector<std::byte> rapids =
        read_file(GUI_FORMS_TEST_RAPIDS_REGULAR);
    const std::vector<std::byte> carlito =
        read_file(GUI_FORMS_TEST_CARLITO_REGULAR);
    const std::optional<FontFaceId> primary = engine.register_typeface(
        FontRole::control, 400, false, rapids);
    const std::optional<FontFaceId> fallback = engine.register_typeface(
        FontRole::control, 400, false, carlito);
    require(primary && fallback, "fallback fixture faces must register");

    const std::string mixed = "A\xd0\x96"
                              "B";
    const ShapedText shaped = engine.shape(
        mixed, {FontRole::control, 14.0, 400, false});
    require(shaped.runs.size() == 3U &&
                shaped.runs[0].face == *primary &&
                shaped.runs[1].face == *fallback &&
                shaped.runs[2].face == *primary &&
                shaped.runs[0].glyphs.front().cluster == Utf8Offset(0U) &&
                shaped.runs[1].glyphs.front().cluster == Utf8Offset(1U) &&
                shaped.runs[2].glyphs.front().cluster == Utf8Offset(3U) &&
                shaped.missing_clusters == 0U,
            "fallback must preserve whole grapheme boundaries and absolute UTF-8 clusters");

    const ShapedText missing = engine.shape(
        "a\xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x92\xbb"
        "b", {FontRole::control, 14.0, 400, false});
    require(missing.missing_clusters == 1U,
            "an uncovered emoji ZWJ sequence must report one indivisible pack fault");
}

void test_control_navigation_symbols_stay_in_rapids() {
    const std::vector<std::byte> rapids_regular = read_file(
        GUI_FORMS_TEST_RAPIDS_REGULAR);
    const std::vector<std::byte> rapids_bold = read_file(
        GUI_FORMS_TEST_RAPIDS_BOLD);
    const std::vector<std::byte> carlito = read_file(
        GUI_FORMS_TEST_CARLITO_REGULAR);
    HarfBuzzFontEngine engine;
    const std::optional<FontFaceId> regular = engine.register_typeface(
        FontRole::control, 400, false, rapids_regular);
    const std::optional<FontFaceId> bold = engine.register_typeface(
        FontRole::control, 700, false, rapids_bold);
    const std::optional<FontFaceId> body = engine.register_typeface(
        FontRole::content, 400, false, carlito);
    require(regular && bold && body,
            "house control/body faces must register for navigation shaping");
    const std::string text = "Back ←  Up ↑  Forward →  Sort A→Z ▼";
    const ShapedText normal = engine.shape(
        text, {FontRole::control, 14.0, 400, false});
    const ShapedText heavy = engine.shape(
        text, {FontRole::control, 14.0, 700, false});
    require(normal.missing_clusters == 0U && normal.runs.size() == 1U &&
                normal.runs.front().face == *regular &&
                heavy.missing_clusters == 0U && heavy.runs.size() == 1U &&
                heavy.runs.front().face == *bold,
            "navigation controls must shape wholly in Portsmouth Rapids");
}

void test_metrics_scale_and_repeat_deterministically() {
    HarfBuzzFontEngine engine;
    const std::vector<std::byte> carlito =
        read_file(GUI_FORMS_TEST_CARLITO_REGULAR);
    require(engine.register_typeface(FontRole::content, 400, false, carlito)
                .has_value(),
            "metric fixture face must register");
    const ShapedText first = engine.shape(
        "Calibrated 123", {FontRole::content, 12.0, 400, false});
    const ShapedText repeat = engine.shape(
        "Calibrated 123", {FontRole::content, 12.0, 400, false});
    const ShapedText doubled = engine.shape(
        "Calibrated 123", {FontRole::content, 24.0, 400, false});
    require(first.runs.size() == repeat.runs.size() &&
                near(first.width, repeat.width, 0.000001) &&
                near(first.height, repeat.height, 0.000001) &&
                first.runs.front().glyphs == repeat.runs.front().glyphs &&
                doubled.width > first.width * 1.85 &&
                doubled.width < first.width * 2.15 &&
                doubled.height > first.height,
            "same pack/profile must repeat exactly and scale predictably");
}

void test_shared_cjk_and_emoji_fallback_across_roles() {
    HarfBuzzFontEngine engine;
    const std::vector<std::byte> rapids =
        read_file(GUI_FORMS_TEST_RAPIDS_REGULAR);
    const std::vector<std::byte> carlito =
        read_file(GUI_FORMS_TEST_CARLITO_REGULAR);
    const std::vector<std::byte> cousine =
        read_file(GUI_FORMS_TEST_COUSINE_REGULAR);
    const std::vector<std::byte> noto_cjk =
        read_file(GUI_FORMS_TEST_NOTO_CJK_REGULAR);
    const std::vector<std::byte> noto_emoji =
        read_file(GUI_FORMS_TEST_NOTO_EMOJI);
    require(engine.register_typeface(FontRole::control, 400, false, rapids) &&
                engine.register_typeface(FontRole::content, 400, false, carlito) &&
                engine.register_typeface(FontRole::monospace, 400, false, cousine),
            "each public font role must have an explicit primary face");
    const std::optional<FontFaceId> cjk = engine.register_fallback_typeface(
        400, false, noto_cjk);
    const std::optional<FontFaceId> emoji = engine.register_fallback_typeface(
        400, false, noto_emoji);
    require(cjk && emoji && engine.face_count() == 5U,
            "shared fallback faces must register once rather than once per role");

    for (const FontRole role : std::array{
             FontRole::control, FontRole::content, FontRole::monospace}) {
        const std::string sample = "A日本語🚀Z";
        const ShapedText shaped = engine.shape(
            sample, {role, 15.0, 400, false});
        const bool complete = !shaped.missing_primary_face &&
            shaped.missing_clusters == 0U &&
            shaped_text_uses_face(shaped, *cjk) &&
            shaped_text_uses_face(shaped, *emoji) &&
            shaped.width > 0.0;
        if (!complete) {
            std::cerr << "fallback diagnostic role=" << static_cast<int>(role)
                      << " missing=" << shaped.missing_clusters
                      << " runs=" << shaped.runs.size()
                      << " cjk=" << shaped_text_uses_face(shaped, *cjk)
                      << " emoji=" << shaped_text_uses_face(shaped, *emoji)
                      << " faces=";
            for (const ShapedFontRun& run : shaped.runs) {
                std::cerr << run.face.value << ',';
            }
            TextStore store(sample);
            std::cerr << " bytes=" << sample.size()
                      << " graphemes=" << store.grapheme_count().value()
                      << " ranges=";
            for (std::size_t index = 0; index < store.grapheme_count().value();
                 ++index) {
                const Utf8Range range = store.grapheme_range(GraphemeIndex(index));
                std::cerr << range.start.value() << '-' << range.end.value() << ',';
            }
            std::cerr << '\n';
        }
        require(complete,
                "CJK and emoji fallback must be complete and role-independent");
    }
}

void test_letter_spacing_is_a_shaped_layout_input() {
    HarfBuzzFontEngine engine;
    const std::vector<std::byte> carlito =
        read_file(GUI_FORMS_TEST_CARLITO_REGULAR);
    require(engine.register_typeface(FontRole::content, 400, false, carlito)
                .has_value(),
            "tracking fixture face must register");
    const ShapedText native = engine.shape(
        "AB", {FontRole::content, 12.0, 400, false, 0.0});
    const ShapedText tracked = engine.shape(
        "AB", {FontRole::content, 12.0, 400, false, 0.75});
    require(native.runs.size() == 1U && tracked.runs.size() == 1U &&
                native.runs.front().glyphs.size() == 2U &&
                tracked.runs.front().glyphs.size() == 2U &&
                near(tracked.width - native.width, 0.75, 0.001) &&
                near(tracked.runs.front().glyphs[1].x -
                         native.runs.front().glyphs[1].x, 0.75, 0.001),
            "letter spacing must shift following clusters and measurement identically");

    FontSpec invalid{FontRole::content, 12.0, 400, false, 13.0};
    require(engine.shape("AB", invalid).runs.empty(),
            "pathological tracking must be rejected at the shaping boundary");
}

void test_rejection_and_missing_role_are_honest() {
    HarfBuzzFontEngine engine;
    const std::vector<std::byte> invalid(64U, std::byte{0x7f});
    require(!engine.register_typeface(FontRole::content, 400, false, {}) &&
                !engine.register_typeface(FontRole::content, 400, false, invalid) &&
                engine.face_count() == 0U,
            "empty and malformed font data must be rejected without partial state");
    const ShapedText missing = engine.shape(
        "No host fallback", {FontRole::content, 12.0, 400, false});
    require(missing.missing_primary_face && missing.runs.empty() &&
                missing.width == 0.0,
            "an absent bundled role must be reported rather than resolved from the host");
}

} // namespace

int main() {
    try {
        test_owned_registration_style_selection_and_ligatures();
        test_cluster_fallback_is_bounded_and_absolute();
        test_control_navigation_symbols_stay_in_rapids();
        test_metrics_scale_and_repeat_deterministically();
        test_shared_cjk_and_emoji_fallback_across_roles();
        test_letter_spacing_is_a_shaped_layout_input();
        test_rejection_and_missing_role_are_honest();
        std::cout << "harfbuzz font engine tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "harfbuzz font engine tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
