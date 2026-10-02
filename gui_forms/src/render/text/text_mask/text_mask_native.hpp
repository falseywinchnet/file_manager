#pragma once

#include "../../../core/text/text_mask/text_mask_state.hpp"
#include "../../../core/text/unicode/unicode_grapheme.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <hb-ft.h>
#include <SheenBidi/SheenBidi.h>

namespace gui_forms::detail::mask_native {

struct NativeFailure final : std::exception {
    explicit NativeFailure(const TextMaskStatus status) noexcept : status(status) {}
    TextMaskStatus status{TextMaskStatus::native_failure};
};

struct NativeLibrary final {
    FT_Library value{};
    NativeLibrary();
    ~NativeLibrary();
    NativeLibrary(const NativeLibrary&) = delete;
    NativeLibrary& operator=(const NativeLibrary&) = delete;
};
struct NativeFace final {
    FT_Face value{};
    hb_font_t* shaping{};
    double ascent{0.0};
    double descent{0.0};
    NativeFace() = default;
    ~NativeFace();
    NativeFace(const NativeFace&) = delete;
    NativeFace& operator=(const NativeFace&) = delete;
};

struct NativeFonts final {
    // Destruction reverses declaration: HB/FT faces before their library.
    NativeLibrary library{};
    std::array<NativeFace, 8> faces{};
    std::size_t count{0};
    std::uint32_t primary{0};
    explicit NativeFonts(const MaskKey& key);
    [[nodiscard]] std::uint32_t select_face(const std::string_view cluster) const;
    void set_raster_size(const MaskOptions& options);
};

struct NativeGlyph final {
    std::uint32_t face{0};
    std::uint32_t glyph{0};
    std::uint32_t cluster{0};
    double x{0.0};
    double y{0.0};
    double advance{0.0};
};
using NativeGlyphBuffer = std::vector<NativeGlyph, MaskAllocator<NativeGlyph>>;
// Growth follows actual native glyph counts, capped at 65,536. Capacity is
// reused across line trials; allocation/initialization precedes glyph kernels.
void resize_native_glyphs(NativeGlyphBuffer& glyphs, const std::size_t count);
struct NativeCluster final {
    std::size_t begin{0};
    std::size_t end{0};
    std::uint32_t face{0};
    hb_script_t script{HB_SCRIPT_UNKNOWN};
};
struct NativeShape final {
    std::size_t glyph_count{0};
    double advance{0.0};
    double ascent{0.0};
    double descent{0.0};
};
struct NativeWork final {
    std::size_t shape_calls{0};
    std::size_t context_bytes{0};
    void admit(const std::size_t bytes);
};

// One paragraph's resolved bidi levels survive all candidate line trials.
// Source borrows the admitted key; fonts, ledger and work outlive this object.
// First-party arrays use counted allocators; native library allocations remain
// opaque. Caller owns the counted output, reused across trials.
class NativeParagraph final {
public:
    NativeParagraph(const std::shared_ptr<MaskLedger>& ledger, NativeFonts& fonts,
        NativeWork& work, const std::string_view text);
    ~NativeParagraph();
    NativeParagraph(const NativeParagraph&) = delete;
    NativeParagraph& operator=(const NativeParagraph&) = delete;
    [[nodiscard]] NativeShape shape(const std::size_t begin, const std::size_t end,
        NativeGlyphBuffer& output);
    [[nodiscard]] std::span<const std::size_t> edges() const noexcept;
private:
    NativeFonts& fonts_;
    NativeWork& work_;
    std::string_view text_{};
    std::vector<std::size_t, MaskAllocator<std::size_t>> edges_;
    std::vector<NativeCluster, MaskAllocator<NativeCluster>> clusters_;
    SBAlgorithmRef algorithm_{};
    SBParagraphRef paragraph_{};
    hb_buffer_t* buffer_{};
    void release() noexcept;
    void append_run(const std::size_t line_begin, const std::size_t line_end,
        const std::size_t run_begin, const std::size_t run_end,
        const NativeCluster& cluster, const bool rtl,
        NativeGlyphBuffer& output, NativeShape& shape);
};

struct NativeInk final {
    std::int64_t left{0};
    std::int64_t top{0};
    std::int64_t right{0};
    std::int64_t bottom{0};
    bool present{false};
};

[[nodiscard]] NativeInk measure_native_ink(NativeFonts& fonts, const MaskOptions& options,
    const std::span<const NativeGlyph> glyphs);
void fill_native_mask(NativeFonts& fonts, const MaskOptions& options,
    const std::span<const NativeGlyph> glyphs, TextMaskStorage& storage);

} // namespace gui_forms::detail::mask_native
