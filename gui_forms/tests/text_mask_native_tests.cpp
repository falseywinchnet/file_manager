#include "../src/render/text/text_mask/text_mask_native.hpp"
#include "prepared_text_test_support.hpp"

#include <cmath>
#include <iostream>

namespace {
using namespace gui_forms;
using namespace gui_forms::detail;
using namespace gui_forms::detail::mask_native;
using prepared_test::require;

std::vector<std::byte> read_native_fixture(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    require(static_cast<bool>(input), "native fixture opens");
    const std::streamsize length = input.tellg();
    require(length > 0 && length <= 4*1024*1024, "native fixture bounded length");
    std::vector<std::byte> bytes(static_cast<std::size_t>(length));
    input.seekg(0);
    input.read(reinterpret_cast<char*>(bytes.data()), length);
    require(static_cast<bool>(input), "native fixture read");
    return bytes;
}

struct NativeFixture final {
    TextMaskService service{};
    EncodedFontLease bank{};
    std::unique_ptr<TextMaskSession> session{};
    std::shared_ptr<MaskLedger> ledger{};
    explicit NativeFixture(const std::span<const std::byte> bytes) {
        ledger = (*TextMaskAccess::state(service)).ledger;
        const std::array<PreparedFontSource, 1> sources{{{.encoded = bytes}}};
        TextMaskResult result = service.create_font_bank(sources, bank);
        require(result.status == TextMaskStatus::success, "native fixture bank admission");
        result = service.open_session(nullptr, session);
        require(result.status == TextMaskStatus::success, "native fixture session");
    }
    std::shared_ptr<const MaskKey> key(const TextMaskRequest& request) {
        MaskOptions options{};
        const TextMaskResult result = normalize_mask_request(request, options);
        require(result.status == TextMaskStatus::success, "native fixture request normalizes");
        MaskCharge count{};
        count.reserve(ledger, MaskResource::keys, 1);
        const MaskAllocator<MaskKey> allocator(ledger);
        std::shared_ptr<MaskKey> key = std::allocate_shared<MaskKey>(allocator, ledger, std::move(count));
        (*key).fonts = (*TextMaskAccess::state(*session)).find_bank(bank);
        (*key).options = options;
        (*key).text.resize(request.utf8.size());
        std::copy(request.utf8.begin(), request.utf8.end(), (*key).text.begin());
        return key;
    }
};

void test_shape_and_scale(const std::span<const std::byte> bytes) {
    NativeFixture fixture(bytes);
    const std::string text = "office cafe\xCC\x81 AV";
    const std::shared_ptr<const MaskKey> first = fixture.key({.utf8 = text, .size = 17.0, .device_scale = 1.0});
    const std::shared_ptr<const MaskKey> second = fixture.key({.utf8 = text, .size = 17.0, .device_scale = 2.0});
    NativeFonts first_fonts(*first);
    NativeFonts second_fonts(*second);
    std::cout << "Fixture coverage: a=" << FT_Get_Char_Index(first_fonts.faces[0].value, 0x61)
        << " acute=" << FT_Get_Char_Index(first_fonts.faces[0].value, 0x301)
        << " alef=" << FT_Get_Char_Index(first_fonts.faces[0].value, 0x5D0)
        << " arabic-beh=" << FT_Get_Char_Index(first_fonts.faces[0].value, 0x628) << '\n';
    NativeWork first_work{};
    NativeWork second_work{};
    NativeParagraph first_paragraph(fixture.ledger, first_fonts, first_work, text);
    NativeParagraph second_paragraph(fixture.ledger, second_fonts, second_work, text);
    NativeGlyphBuffer first_glyphs{MaskAllocator<NativeGlyph>(fixture.ledger, MaskResource::shaping)};
    NativeGlyphBuffer second_glyphs{MaskAllocator<NativeGlyph>(fixture.ledger, MaskResource::shaping)};
    const NativeShape first_shape = first_paragraph.shape(0, text.size(), first_glyphs);
    const NativeShape second_shape = second_paragraph.shape(0, text.size(), second_glyphs);
    require(first_shape.glyph_count > 0 && first_shape.glyph_count == second_shape.glyph_count, "logical glyph count independent of device scale");
    require(first_shape.advance == second_shape.advance && first_shape.ascent == second_shape.ascent, "logical metrics independent of device scale");
    for (std::size_t index = 0; index < first_shape.glyph_count; ++index) {
        const NativeGlyph& a = first_glyphs[index];
        const NativeGlyph& b = second_glyphs[index];
        require(a.glyph == b.glyph && a.cluster == b.cluster && a.x == b.x && a.y == b.y && a.advance == b.advance,
            "exact logical positions and clusters independent of scale");
    }
    const NativeShape fragment = first_paragraph.shape(0, 2, first_glyphs);
    require(fragment.glyph_count > 0, "line context reshapes selected fragment");
    for (std::size_t index = 0; index < fragment.glyph_count; ++index) require(first_glyphs[index].cluster < 2, "no glyph cluster crosses selected line");
    require(first_work.shape_calls > 0 && first_work.context_bytes > 0, "native calls and context bytes accounted");
}

void test_raster_profiles(const std::span<const std::byte> bytes) {
    NativeFixture fixture(bytes);
    const std::string text = "Agj cafe\xCC\x81";
    for (unsigned profile = 0; profile < 2; ++profile) {
        for (unsigned scale = 1; scale <= 4; ++scale) {
            TextMaskRequest request{.utf8 = text, .size = 19.0, .device_scale = static_cast<double>(scale)};
            if (profile == 1) request.raster = TextMaskRaster::true_mono;
            const std::shared_ptr<const MaskKey> key = fixture.key(request);
            NativeFonts fonts(*key);
            NativeWork work{};
            NativeParagraph paragraph(fixture.ledger, fonts, work, text);
            NativeGlyphBuffer glyphs{MaskAllocator<NativeGlyph>(fixture.ledger, MaskResource::shaping)};
            const NativeShape shape = paragraph.shape(0, text.size(), glyphs);
            for (std::size_t index = 0; index < shape.glyph_count; ++index) glyphs[index].y += shape.ascent;
            const std::span<const NativeGlyph> live(glyphs.data(), shape.glyph_count);
            const NativeInk ink = measure_native_ink(fonts, (*key).options, live);
            require(ink.present && ink.right > ink.left && ink.bottom > ink.top, "native ink measured");
            TextMaskMetrics metrics{};
            metrics.device_scale = request.device_scale;
            metrics.ink_left_px = static_cast<std::int32_t>(ink.left);
            metrics.ink_top_px = static_cast<std::int32_t>(ink.top);
            metrics.width_px = static_cast<std::uint32_t>(ink.right - ink.left);
            metrics.height_px = static_cast<std::uint32_t>(ink.bottom - ink.top);
            metrics.stride_bytes = metrics.width_px;
            std::shared_ptr<TextMaskStorage> mask = allocate_mask_storage(fixture.ledger, key, metrics, 1);
            fill_native_mask(fonts, (*key).options, live, *mask);
            bool ink_found = false;
            bool fractional = false;
            for (std::size_t index = 0; index < (*mask).pixels.size(); ++index) {
                const std::uint8_t value = (*mask).pixels[index];
                if (value != 0) ink_found = true;
                if (value != 0 && value != 255) fractional = true;
            }
            require(ink_found, "native raster contains coverage");
            if (profile == 1) require(!fractional, "true FT mono expands only zero and255");
            else require(fractional, "outline gray retains partial coverage");
        }
    }
}

void test_native_failure_and_work(const std::span<const std::byte> bytes) {
    NativeFixture fixture(bytes);
    const std::shared_ptr<const MaskKey> key = fixture.key({.utf8 = "a"});
    NativeFonts fonts(*key);
    NativeWork work{};
    NativeParagraph paragraph(fixture.ledger, fonts, work, "a");
    NativeGlyphBuffer glyphs{MaskAllocator<NativeGlyph>(fixture.ledger, MaskResource::shaping)};
    work.shape_calls = 2048;
    bool refused = false;
    try { const NativeShape shape = paragraph.shape(0, 1, glyphs); static_cast<void>(shape); }
    catch (const MaskLimitFailure& failure) { refused = failure.limit == TextMaskLimit::shape_work; }
    require(refused, "native call budget checked before call");
    work.shape_calls = 0;
    work.context_bytes = 4U*1024U*1024U;
    refused = false;
    try { const NativeShape shape = paragraph.shape(0, 1, glyphs); static_cast<void>(shape); }
    catch (const MaskLimitFailure& failure) { refused = failure.limit == TextMaskLimit::shape_work; }
    require(refused, "aggregate context checked before call");
    const std::array<std::byte, 8> invalid{};
    NativeFixture malformed(invalid);
    const std::shared_ptr<const MaskKey> bad = malformed.key({});
    refused = false;
    try { NativeFonts invalid_fonts(*bad); }
    catch (const NativeFailure& failure) { refused = failure.status == TextMaskStatus::unsupported_profile; }
    require(refused, "malformed encoded font fails native opening explicitly");
    const std::string missing = "\xF4\x8F\xBF\xBF";
    refused = false;
    try { NativeParagraph unavailable(fixture.ledger, fonts, work, missing); }
    catch (const NativeFailure& failure) { refused = failure.status == TextMaskStatus::missing_font_coverage; }
    require(refused, "missing coverage never uses system fonts or replacement glyphs");
}

void test_paragraph_bidi(const std::span<const std::byte> bytes) {
    NativeFixture fixture(bytes);
    const std::string text = "\xD7\x90\xD7\x91 123.";
    const std::shared_ptr<const MaskKey> key = fixture.key({.utf8 = text});
    NativeFonts fonts(*key);
    NativeWork work{};
    NativeParagraph paragraph(fixture.ledger, fonts, work, text);
    NativeGlyphBuffer glyphs{MaskAllocator<NativeGlyph>(fixture.ledger, MaskResource::shaping)};
    const NativeShape line = paragraph.shape(5, text.size(), glyphs);
    require(line.glyph_count == 4 && glyphs[0].cluster == text.size() - 1U,
        "wrapped numeric line retains original RTL paragraph context");
    const std::string standalone = "123.";
    NativeParagraph independent(fixture.ledger, fonts, work, standalone);
    const NativeShape comparison = independent.shape(0, standalone.size(), glyphs);
    require(comparison.glyph_count == 4 && glyphs[3].cluster == 3,
        "standalone numeric paragraph has different visual punctuation order");
    NativeParagraph empty(fixture.ledger, fonts, work, {});
    const NativeShape empty_line = empty.shape(0, 0, glyphs);
    require(empty_line.glyph_count == 0 && empty_line.advance == 0.0 && empty_line.ascent > 0.0 && empty_line.descent > 0.0,
        "empty line uses primary face metrics with no glyph or ink");
}

void test_mono_native_oracle(const std::span<const std::byte> bytes) {
    NativeFixture fixture(bytes);
    const std::shared_ptr<const MaskKey> key = fixture.key({.utf8 = "j", .size = 13.0, .device_scale = 2.0, .raster = TextMaskRaster::true_mono});
    NativeFonts fonts(*key);
    NativeWork work{};
    NativeParagraph paragraph(fixture.ledger, fonts, work, "j");
    NativeGlyphBuffer glyphs{MaskAllocator<NativeGlyph>(fixture.ledger, MaskResource::shaping)};
    const NativeShape shape = paragraph.shape(0, 1, glyphs);
    require(shape.glyph_count == 1, "mono oracle single glyph");
    glyphs[0].y += shape.ascent;
    const std::span<const NativeGlyph> live(glyphs.data(), 1);
    const NativeInk ink = measure_native_ink(fonts, (*key).options, live);
    TextMaskMetrics metrics{};
    metrics.device_scale = 2.0;
    metrics.ink_left_px = static_cast<std::int32_t>(ink.left);
    metrics.ink_top_px = static_cast<std::int32_t>(ink.top);
    metrics.width_px = static_cast<std::uint32_t>(ink.right - ink.left);
    metrics.height_px = static_cast<std::uint32_t>(ink.bottom - ink.top);
    metrics.stride_bytes = metrics.width_px;
    std::shared_ptr<TextMaskStorage> mask = allocate_mask_storage(fixture.ledger, key, metrics, 1);
    fill_native_mask(fonts, (*key).options, live, *mask);
    // Independent native call supplies the oracle's packed bits and bearings.
    const FT_Face face = fonts.faces[0].value;
    FT_Error status = FT_Load_Glyph(face, glyphs[0].glyph, FT_LOAD_NO_BITMAP | FT_LOAD_TARGET_MONO | FT_LOAD_MONOCHROME);
    require(status == 0, "mono oracle glyph load");
    status = FT_Render_Glyph((*face).glyph, FT_RENDER_MODE_MONO);
    require(status == 0, "mono oracle packed render");
    const FT_GlyphSlotRec& slot = *(*face).glyph;
    const FT_Bitmap& bitmap = slot.bitmap;
    require(bitmap.pixel_mode == FT_PIXEL_MODE_MONO && bitmap.width == metrics.width_px && bitmap.rows == metrics.height_px,
        "mask dimensions equal native mono bitmap");
    const std::int64_t expected_left = std::llround(glyphs[0].x * 2.0) + slot.bitmap_left;
    const std::int64_t expected_top = std::llround(glyphs[0].y * 2.0) - slot.bitmap_top;
    require(expected_left == metrics.ink_left_px && expected_top == metrics.ink_top_px, "signed native bearings preserved");
    for (std::size_t row = 0; row < bitmap.rows; ++row) {
        for (std::size_t column = 0; column < bitmap.width; ++column) {
            const unsigned packed = bitmap.buffer[row * static_cast<std::size_t>(bitmap.pitch) + column / 8U];
            const unsigned bit = 0x80U >> (column % 8U);
            std::uint8_t expected = 0;
            if ((packed & bit) != 0) expected = 255;
            require((*mask).pixels[row * metrics.stride_bytes + column] == expected, "mono coverage equals actual packed FT bits");
        }
    }
}
void test_arabic_line_context(const std::span<const std::byte> bytes) {
    NativeFixture fixture(bytes);
    const std::string text = "\xD8\xA8\xD8\xA8";
    const std::shared_ptr<const MaskKey> key = fixture.key({.utf8 = text});
    NativeFonts fonts(*key);
    require(FT_Get_Char_Index(fonts.faces[0].value, 0x628) != 0, "approved Amiri covers Arabic beh");
    NativeWork work{};
    NativeParagraph paragraph(fixture.ledger, fonts, work, text);
    NativeGlyphBuffer full_glyphs{MaskAllocator<NativeGlyph>(fixture.ledger, MaskResource::shaping)};
    const NativeShape full = paragraph.shape(0, text.size(), full_glyphs);
    require(full.glyph_count == 2, "two Arabic letters shape contextually");
    NativeGlyphBuffer first_glyphs{MaskAllocator<NativeGlyph>(fixture.ledger, MaskResource::shaping)};
    NativeGlyphBuffer second_glyphs{MaskAllocator<NativeGlyph>(fixture.ledger, MaskResource::shaping)};
    const NativeShape first = paragraph.shape(0, 2, first_glyphs);
    const NativeShape second = paragraph.shape(2, 4, second_glyphs);
    require(first.glyph_count == 1 && second.glyph_count == 1 && first_glyphs[0].glyph == second_glyphs[0].glyph,
        "line-boundary reshaping isolates each Arabic letter");
    require(full_glyphs[0].glyph != first_glyphs[0].glyph && full_glyphs[1].glyph != first_glyphs[0].glyph,
        "full-word joining forms differ from isolated line forms");
    require(first_glyphs[0].cluster == 0 && second_glyphs[0].cluster == 2, "line reshaping retains original source offsets");
}
}

int main(const int argc, char** const argv) {
    try {
        require(argc == 3, "pass approved font directory and Amiri fixture path");
        const std::filesystem::path directory(argv[1]);
        const std::vector<std::byte> bytes = read_native_fixture(directory / "Cousine-Regular.ttf");
        const std::vector<std::byte> arabic = read_native_fixture(std::filesystem::path(argv[2]));
        std::cout << "native group: shape and scale\n";
        test_shape_and_scale(bytes);
        std::cout << "native group: raster profiles\n";
        test_raster_profiles(bytes);
        std::cout << "native group: failures and work bounds\n";
        test_native_failure_and_work(bytes);
        test_paragraph_bidi(bytes);
        test_mono_native_oracle(bytes);
        test_arabic_line_context(arabic);
        std::cout << "text mask native components: six groups passed\n";
        return EXIT_SUCCESS;
    } catch (const NativeFailure& failure) {
        std::cerr << "Native status: " << static_cast<int>(failure.status) << '\n';
        return EXIT_FAILURE;
    } catch (const MaskLimitFailure& failure) {
        std::cerr << "Native limit: " << static_cast<int>(failure.limit) << '\n';
        return EXIT_FAILURE;
    } catch (const std::exception& failure) {
        std::cerr << failure.what() << '\n';
        return EXIT_FAILURE;
    }
}
