#include "skia_raster.hpp"

#if defined(GUI_FORMS_HAS_HARFBUZZ_TEXT)
#include "harfbuzz_font_engine.hpp"
#endif

#include "include/codec/SkCodec.h"
#include "include/codec/SkPngDecoder.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRegion.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#if defined(__APPLE__)
#include "include/ports/SkFontMgr_mac_ct.h"
#else
#include "include/ports/SkFontMgr_empty.h"
#endif
#include "src/core/SkUTF.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace gui_forms::render {
namespace {

SkColor to_sk_color(Color color) noexcept {
    return SkColorSetARGB(color.alpha, color.red, color.green, color.blue);
}

SkRect to_sk_rect(Rect rect) noexcept {
    return SkRect::MakeXYWH(static_cast<SkScalar>(rect.x),
                            static_cast<SkScalar>(rect.y),
                            static_cast<SkScalar>(rect.width),
                            static_cast<SkScalar>(rect.height));
}

SkPaint make_paint(Color color) {
    SkPaint paint;
    paint.setColor(to_sk_color(color));
    paint.setAntiAlias(true);
    return paint;
}

} // namespace

class SkiaRaster::Impl {
public:
    struct RegisteredTypeface final {
        FontRole role;
        std::uint16_t weight;
        bool italic;
        bool fallback{};
        sk_sp<SkTypeface> face;
#if defined(GUI_FORMS_HAS_HARFBUZZ_TEXT)
        FontFaceId text_face;
#endif
    };

    struct DecodedImage final {
        std::uint64_t content_hash{};
        sk_sp<SkImage> image;
    };

    struct TextRun final {
        std::size_t offset{};
        std::size_t length{};
        sk_sp<SkTypeface> face;
    };

    sk_sp<SkSurface> surface;
    sk_sp<SkFontMgr> fonts{
#if defined(__APPLE__)
        SkFontMgr_New_CoreText(nullptr)
#else
        SkFontMgr_New_Custom_Empty()
#endif
    };
    std::unordered_map<std::uint64_t, DecodedImage> images;
    std::vector<RegisteredTypeface> registered_typefaces;
#if defined(GUI_FORMS_HAS_HARFBUZZ_TEXT)
    text::HarfBuzzFontEngine text_engine;
#endif
    Size logical_size{};
    double scale{1.0};
    int save_floor{1};
    const ImageRegistry* image_registry{};
    std::uint64_t image_registry_revision{std::numeric_limits<std::uint64_t>::max()};
    bool image_registry_synchronized{true};

    [[nodiscard]] SkCanvas* canvas() const noexcept {
        return surface ? surface->getCanvas() : nullptr;
    }

    [[nodiscard]] static SkFontStyle font_style(FontSpec spec) {
        const SkFontStyle::Slant slant = spec.italic
            ? SkFontStyle::kItalic_Slant
            : SkFontStyle::kUpright_Slant;
        return {static_cast<int>(spec.weight), SkFontStyle::kNormal_Width, slant};
    }

    [[nodiscard]] sk_sp<SkTypeface> typeface(FontSpec spec) const {
        const RegisteredTypeface* registered = nullptr;
        int registered_distance = std::numeric_limits<int>::max();
        for (const RegisteredTypeface& candidate : registered_typefaces) {
            if (candidate.fallback || candidate.role != spec.role ||
                candidate.italic != spec.italic) {
                continue;
            }
            const int distance = std::abs(static_cast<int>(candidate.weight) -
                                          static_cast<int>(spec.weight));
            if (distance < registered_distance) {
                registered = &candidate;
                registered_distance = distance;
            }
        }
        if (registered != nullptr) {
            return registered->face;
        }

        const SkFontStyle style = font_style(spec);
        const char* family = nullptr;
        if (spec.role == FontRole::control) {
            family = "Lucida Grande";
        } else if (spec.role == FontRole::monospace) {
            family = "Menlo";
        }
        sk_sp<SkTypeface> face = fonts->matchFamilyStyle(family, style);
        if (!face) {
            face = fonts->legacyMakeTypeface(nullptr, style);
        }
        return face;
    }

#if defined(GUI_FORMS_HAS_HARFBUZZ_TEXT)
    [[nodiscard]] sk_sp<SkTypeface> typeface(FontFaceId id) const {
        for (const RegisteredTypeface& candidate : registered_typefaces) {
            if (candidate.text_face == id) return candidate.face;
        }
        return nullptr;
    }
#endif

    [[nodiscard]] std::vector<TextRun> text_runs(std::string_view text,
                                                 FontSpec spec) const {
        std::vector<TextRun> runs;
        if (text.empty()) {
            return runs;
        }
        const SkFontStyle style = font_style(spec);
        const sk_sp<SkTypeface> primary = typeface(spec);
        sk_sp<SkTypeface> current = primary;
        std::size_t run_start{};
        const char* cursor = text.data();
        const char* const end = cursor + text.size();
        while (cursor < end) {
            const char* const scalar_start = cursor;
            const SkUnichar scalar = SkUTF::NextUTF8(&cursor, end);
            sk_sp<SkTypeface> face = primary;
            if (scalar >= 0 && (!face || face->unicharToGlyph(scalar) == 0U)) {
                for (const RegisteredTypeface& candidate : registered_typefaces) {
                    const bool house_body_fallback =
                        spec.role != FontRole::content && !candidate.fallback &&
                        candidate.role == FontRole::content;
                    if ((candidate.fallback || house_body_fallback) && candidate.face &&
                        candidate.face->unicharToGlyph(scalar) != 0U) {
                        face = candidate.face;
                        break;
                    }
                }
            }
            if (scalar >= 0 && (!face || face->unicharToGlyph(scalar) == 0U)) {
                face = fonts->matchFamilyStyleCharacter(
                    nullptr, style, nullptr, 0, scalar);
                if (!face) {
                    face = primary;
                }
            }
            const bool same_face = (!current && !face) ||
                (current && face && current->uniqueID() == face->uniqueID());
            if (!same_face) {
                const std::size_t scalar_offset = static_cast<std::size_t>(
                    scalar_start - text.data());
                if (scalar_offset > run_start) {
                    runs.push_back({run_start, scalar_offset - run_start, current});
                }
                run_start = scalar_offset;
                current = std::move(face);
            }
        }
        if (run_start < text.size()) {
            runs.push_back({run_start, text.size() - run_start, current});
        }
        return runs;
    }
};

SkiaRaster::SkiaRaster() : impl_(std::make_unique<Impl>()) {}
SkiaRaster::~SkiaRaster() = default;

bool SkiaRaster::register_typeface(FontRole role,
                                   std::uint16_t weight,
                                   bool italic,
                                   std::span<const std::byte> encoded) {
    if (encoded.empty()) {
        return false;
    }
    sk_sp<SkData> data = SkData::MakeWithCopy(encoded.data(), encoded.size());
    sk_sp<SkTypeface> face = impl_->fonts->makeFromData(std::move(data));
    if (!face) {
        return false;
    }
#if defined(GUI_FORMS_HAS_HARFBUZZ_TEXT)
    const std::optional<FontFaceId> text_face =
        impl_->text_engine.register_typeface(role, weight, italic, encoded);
    if (!text_face) return false;
    impl_->registered_typefaces.push_back(
        {role, weight, italic, false, std::move(face), *text_face});
#else
    impl_->registered_typefaces.push_back(
        {role, weight, italic, false, std::move(face)});
#endif
    return true;
}

bool SkiaRaster::register_fallback_typeface(
    std::uint16_t weight, bool italic, std::span<const std::byte> encoded) {
    if (encoded.empty()) return false;
    sk_sp<SkData> data = SkData::MakeWithCopy(encoded.data(), encoded.size());
    sk_sp<SkTypeface> face = impl_->fonts->makeFromData(std::move(data));
    if (!face) return false;
#if defined(GUI_FORMS_HAS_HARFBUZZ_TEXT)
    const std::optional<FontFaceId> text_face =
        impl_->text_engine.register_fallback_typeface(weight, italic, encoded);
    if (!text_face) return false;
    impl_->registered_typefaces.push_back(
        {FontRole::content, weight, italic, true, std::move(face), *text_face});
#else
    impl_->registered_typefaces.push_back(
        {FontRole::content, weight, italic, true, std::move(face)});
#endif
    return true;
}

bool SkiaRaster::resize(Size logical_size, double scale) {
    if (!std::isfinite(scale) || scale <= 0.0 || logical_size.width <= 0.0 ||
        logical_size.height <= 0.0) {
        return false;
    }
    const double requested_width = std::ceil(logical_size.width * scale);
    const double requested_height = std::ceil(logical_size.height * scale);
    if (requested_width > static_cast<double>(std::numeric_limits<int>::max()) ||
        requested_height > static_cast<double>(std::numeric_limits<int>::max())) {
        return false;
    }
    const int width = std::max(1, static_cast<int>(requested_width));
    const int height = std::max(1, static_cast<int>(requested_height));
    if (impl_->surface && width == impl_->surface->width() &&
        height == impl_->surface->height() && scale == impl_->scale) {
        impl_->logical_size = logical_size;
        return false;
    }

    // The host-facing surface contract is premultiplied RGBA8 on every target.
    // Skia's N32 alias follows the build's native channel order (RGBA on the
    // macOS archive, BGRA on the MinGW archive), which makes identical renderer
    // commands expose different bytes across hosts.
    const SkImageInfo info = SkImageInfo::Make(
        width, height, kRGBA_8888_SkColorType, kPremul_SkAlphaType,
        SkColorSpace::MakeSRGB());
    sk_sp<SkSurface> replacement = SkSurfaces::Raster(info);
    if (!replacement) {
        return false;
    }
    replacement->getCanvas()->clear(SK_ColorTRANSPARENT);
    impl_->surface = std::move(replacement);
    impl_->logical_size = logical_size;
    impl_->scale = scale;
    return true;
}

void SkiaRaster::begin_frame(const DamageRegion& damage) {
    SkCanvas* canvas = impl_->canvas();
    if (!canvas) {
        return;
    }
    canvas->restoreToCount(1);
    canvas->resetMatrix();
    canvas->scale(static_cast<SkScalar>(impl_->scale),
                  static_cast<SkScalar>(impl_->scale));
    canvas->save();
    impl_->save_floor = canvas->getSaveCount();

    if (!damage.empty()) {
        SkRegion region;
        for (Rect rect : damage.rectangles()) {
            const SkIRect pixels = SkIRect::MakeLTRB(
                static_cast<int>(std::floor(rect.x * impl_->scale)),
                static_cast<int>(std::floor(rect.y * impl_->scale)),
                static_cast<int>(std::ceil((rect.x + rect.width) * impl_->scale)),
                static_cast<int>(std::ceil((rect.y + rect.height) * impl_->scale)));
            region.op(pixels, SkRegion::kUnion_Op);
        }
        canvas->resetMatrix();
        canvas->clipRegion(region);
        canvas->scale(static_cast<SkScalar>(impl_->scale),
                      static_cast<SkScalar>(impl_->scale));
    }
}

void SkiaRaster::end_frame() {
    if (SkCanvas* canvas = impl_->canvas()) {
        canvas->restoreToCount(1);
    }
}

bool SkiaRaster::synchronize_images(const ImageRegistry& registry) {
    const ImageRegistrySnapshot snapshot = registry.snapshot();
    const bool same_registry = &registry == impl_->image_registry;
    if (same_registry && snapshot.revision == impl_->image_registry_revision) {
        return impl_->image_registry_synchronized;
    }
    if (!same_registry) {
        impl_->images.clear();
    }

    bool synchronized = true;
    std::unordered_set<std::uint64_t> active;
    for (const ImageId id : registry.image_ids()) {
        active.insert(id.value);
        const std::optional<ImageResourceView> resource = registry.find(id);
        if (!resource) {
            synchronized = false;
            continue;
        }
        const auto existing = impl_->images.find(id.value);
        if (existing != impl_->images.end() &&
            existing->second.content_hash == resource->content_hash) {
            continue;
        }

        if (resource->metadata.width >
                static_cast<std::uint32_t>(std::numeric_limits<int>::max()) ||
            resource->metadata.height >
                static_cast<std::uint32_t>(std::numeric_limits<int>::max()) ||
            resource->metadata.decoded_byte_count >
                std::numeric_limits<std::size_t>::max()) {
            impl_->images.erase(id.value);
            synchronized = false;
            continue;
        }

        const SkImageInfo output_info = SkImageInfo::Make(
            static_cast<int>(resource->metadata.width),
            static_cast<int>(resource->metadata.height),
            resource->encoding == ImageResourceEncoding::bgra32_premultiplied
                ? kBGRA_8888_SkColorType : kRGBA_8888_SkColorType,
            kPremul_SkAlphaType,
            SkColorSpace::MakeSRGB());
        const std::size_t row_bytes = output_info.minRowBytes();
        if (row_bytes != static_cast<std::size_t>(resource->metadata.width) * 4U ||
            output_info.computeByteSize(row_bytes) !=
                resource->metadata.decoded_byte_count) {
            impl_->images.erase(id.value);
            synchronized = false;
            continue;
        }
        std::vector<std::byte> pixels;
        if (resource->encoding == ImageResourceEncoding::bgra32_premultiplied) {
            if (resource->row_bytes != row_bytes ||
                resource->encoded.size() != resource->metadata.decoded_byte_count) {
                impl_->images.erase(id.value);
                synchronized = false;
                continue;
            }
            pixels.assign(resource->encoded.begin(), resource->encoded.end());
        } else {
            sk_sp<SkData> encoded = SkData::MakeWithCopy(
                resource->encoded.data(), resource->encoded.size());
            SkCodec::Result codec_result = SkCodec::kInternalError;
            std::unique_ptr<SkCodec> codec =
                SkPngDecoder::Decode(std::move(encoded), &codec_result);
            if (!codec || codec_result != SkCodec::kSuccess ||
                codec->getInfo().width() !=
                    static_cast<int>(resource->metadata.width) ||
                codec->getInfo().height() !=
                    static_cast<int>(resource->metadata.height)) {
                impl_->images.erase(id.value);
                synchronized = false;
                continue;
            }
            pixels.resize(resource->metadata.decoded_byte_count);
            if (codec->getPixels(output_info, pixels.data(), row_bytes) !=
                SkCodec::kSuccess) {
                impl_->images.erase(id.value);
                synchronized = false;
                continue;
            }
        }
        sk_sp<SkData> pixel_data = SkData::MakeWithCopy(pixels.data(), pixels.size());
        sk_sp<SkImage> image =
            SkImages::RasterFromData(output_info, std::move(pixel_data), row_bytes);
        if (!image) {
            impl_->images.erase(id.value);
            synchronized = false;
            continue;
        }
        impl_->images[id.value] = {resource->content_hash, std::move(image)};
    }
    for (auto iterator = impl_->images.begin(); iterator != impl_->images.end();) {
        if (!active.contains(iterator->first)) {
            iterator = impl_->images.erase(iterator);
        } else {
            ++iterator;
        }
    }
    impl_->image_registry_revision = snapshot.revision;
    impl_->image_registry = &registry;
    impl_->image_registry_synchronized = synchronized;
    return synchronized;
}

const void* SkiaRaster::pixels() const noexcept {
    SkPixmap pixmap;
    return impl_->surface && impl_->surface->peekPixels(&pixmap) ? pixmap.addr() : nullptr;
}

std::size_t SkiaRaster::row_bytes() const noexcept {
    SkPixmap pixmap;
    return impl_->surface && impl_->surface->peekPixels(&pixmap) ? pixmap.rowBytes() : 0;
}

std::uint32_t SkiaRaster::pixel_width() const noexcept {
    return impl_->surface ? static_cast<std::uint32_t>(impl_->surface->width()) : 0;
}

std::uint32_t SkiaRaster::pixel_height() const noexcept {
    return impl_->surface ? static_cast<std::uint32_t>(impl_->surface->height()) : 0;
}

std::size_t SkiaRaster::byte_size() const noexcept {
    return row_bytes() * pixel_height();
}

void SkiaRaster::save() {
    if (SkCanvas* canvas = impl_->canvas()) {
        canvas->save();
    }
}

void SkiaRaster::restore() {
    if (SkCanvas* canvas = impl_->canvas();
        canvas && canvas->getSaveCount() > impl_->save_floor) {
        canvas->restore();
    }
}

void SkiaRaster::translate(Point offset) {
    if (SkCanvas* canvas = impl_->canvas()) {
        canvas->translate(static_cast<SkScalar>(offset.x), static_cast<SkScalar>(offset.y));
    }
}

void SkiaRaster::clip_rect(Rect rect) {
    if (SkCanvas* canvas = impl_->canvas()) {
        canvas->clipRect(to_sk_rect(rect), SkClipOp::kIntersect, false);
    }
}

void SkiaRaster::fill_rect(Rect rect, Color color) {
    if (SkCanvas* canvas = impl_->canvas()) {
        canvas->drawRect(to_sk_rect(rect), make_paint(color));
    }
}

void SkiaRaster::stroke_rect(Rect rect, Color color, double width) {
    if (SkCanvas* canvas = impl_->canvas()) {
        SkPaint paint = make_paint(color);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(static_cast<SkScalar>(width));
        canvas->drawRect(to_sk_rect(rect), paint);
    }
}

void SkiaRaster::draw_line(Point from, Point to, Color color, double width) {
    if (SkCanvas* canvas = impl_->canvas()) {
        SkPaint paint = make_paint(color);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(static_cast<SkScalar>(width));
        canvas->drawLine(static_cast<SkScalar>(from.x), static_cast<SkScalar>(from.y),
                         static_cast<SkScalar>(to.x), static_cast<SkScalar>(to.y), paint);
    }
}

void SkiaRaster::draw_text_utf8(Point origin,
                                std::string_view text,
                                FontSpec font_spec,
                                Color color) {
    if (SkCanvas* canvas = impl_->canvas(); canvas && !text.empty()) {
#if defined(GUI_FORMS_HAS_HARFBUZZ_TEXT)
        const text::ShapedText shaped = impl_->text_engine.shape(text, font_spec);
        const SkPaint paint = make_paint(color);
        for (const text::ShapedFontRun& run : shaped.runs) {
            const sk_sp<SkTypeface> face = impl_->typeface(run.face);
            if (!face || run.glyphs.empty()) continue;
            SkFont font(face, static_cast<SkScalar>(font_spec.size));
            font.setEdging(SkFont::Edging::kAntiAlias);
            font.setSubpixel(true);
            std::vector<SkGlyphID> glyphs;
            std::vector<SkPoint> positions;
            std::vector<std::uint32_t> clusters;
            glyphs.reserve(run.glyphs.size());
            positions.reserve(run.glyphs.size());
            clusters.reserve(run.glyphs.size());
            for (const text::ShapedGlyph& glyph : run.glyphs) {
                glyphs.push_back(static_cast<SkGlyphID>(glyph.glyph.value));
                positions.push_back({glyph.x, glyph.y});
                clusters.push_back(static_cast<std::uint32_t>(glyph.cluster.value()));
            }
            canvas->drawGlyphs(SkSpan<const SkGlyphID>(glyphs),
                               SkSpan<const SkPoint>(positions),
                               SkSpan<const std::uint32_t>(clusters),
                               SkSpan<const char>(text.data(), text.size()),
                               {static_cast<SkScalar>(origin.x),
                                static_cast<SkScalar>(origin.y)},
                               font, paint);
        }
#else
        SkScalar x = static_cast<SkScalar>(origin.x);
        const SkPaint paint = make_paint(color);
        for (const Impl::TextRun& run : impl_->text_runs(text, font_spec)) {
            SkFont font(run.face, static_cast<SkScalar>(font_spec.size));
            font.setEdging(SkFont::Edging::kAntiAlias);
            const char* bytes = text.data() + run.offset;
            canvas->drawSimpleText(bytes, run.length, SkTextEncoding::kUTF8,
                                   x, static_cast<SkScalar>(origin.y), font, paint);
            x += font.measureText(bytes, run.length, SkTextEncoding::kUTF8);
        }
#endif
    }
}

Size SkiaRaster::measure_text_utf8(std::string_view text,
                                   FontSpec font_spec) {
    if (text.empty()) return {0.0, font_spec.size};
#if defined(GUI_FORMS_HAS_HARFBUZZ_TEXT)
    const text::ShapedText shaped = impl_->text_engine.shape(text, font_spec);
    return {std::max(0.0, shaped.width), std::max(0.0, shaped.height)};
#else
    double width{};
    double height{};
    for (const Impl::TextRun& run : impl_->text_runs(text, font_spec)) {
        SkFont font(run.face, static_cast<SkScalar>(font_spec.size));
        const char* bytes = text.data() + run.offset;
        width += font.measureText(bytes, run.length, SkTextEncoding::kUTF8);
        SkFontMetrics metrics{};
        font.getMetrics(&metrics);
        height = std::max(height,
            static_cast<double>(metrics.fDescent - metrics.fAscent));
    }
    return {std::max(0.0, width), std::max(0.0, height)};
#endif
}

void SkiaRaster::draw_image(ImageId image, Rect destination, double opacity) {
    const auto found = impl_->images.find(image.value);
    if (SkCanvas* canvas = impl_->canvas();
        canvas && found != impl_->images.end() && opacity > 0.0) {
        SkPaint paint;
        paint.setAlphaf(static_cast<float>(std::clamp(opacity, 0.0, 1.0)));
        paint.setAntiAlias(true);
        canvas->drawImageRect(found->second.image.get(), to_sk_rect(destination),
                              SkSamplingOptions(SkFilterMode::kLinear), &paint);
    }
}

} // namespace gui_forms::render
