#include "skia_raster.hpp"

#include "gui_forms/live_surface.hpp"
#if defined(GUI_FORMS_PREPARED_TEXT)
#include "prepared_skia_frame.hpp"
#endif

#if defined(GUI_FORMS_HAS_HARFBUZZ_TEXT)
#include "../../text/harfbuzz/harfbuzz_font_engine.hpp"
#endif

#include "include/codec/SkCodec.h"
#include "include/codec/SkPngDecoder.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRegion.h"
#include "include/core/SkRRect.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkGradient.h"
#if defined(__APPLE__)
#include "include/ports/SkFontMgr_mac_ct.h"
#else
#include "include/ports/SkFontMgr_empty.h"
#endif
#include "src/core/SkUTF.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
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

SkRRect to_sk_rrect(Rect rect, double radius) noexcept {
    const SkScalar bounded = static_cast<SkScalar>(std::clamp(
        radius, 0.0, std::max(0.0, std::min(rect.width, rect.height) * 0.5)));
    return SkRRect::MakeRectXY(to_sk_rect(rect), bounded, bounded);
}

struct SkGradientData final {
    std::vector<SkColor4f> colors;
    std::vector<float> positions;
};

SkGradientData to_sk_gradient(std::span<const GradientStop> stops) {
    SkGradientData result;
    result.colors.reserve(stops.size());
    result.positions.reserve(stops.size());
    for (const GradientStop& stop : stops) {
        result.colors.push_back(SkColor4f::FromColor(to_sk_color(stop.color)));
        result.positions.push_back(static_cast<float>(stop.offset));
    }
    return result;
}

SkTileMode to_sk_tile_mode(GradientSpreadMode spread) noexcept {
    switch (spread) {
    case GradientSpreadMode::pad: return SkTileMode::kClamp;
    case GradientSpreadMode::repeat: return SkTileMode::kRepeat;
    case GradientSpreadMode::reflect: return SkTileMode::kMirror;
    }
    return SkTileMode::kClamp;
}

SkGradient make_sk_gradient(
    const SkGradientData& data,
    GradientSpreadMode spread = GradientSpreadMode::pad) {
    const SkGradient::Colors colors(
        SkSpan<const SkColor4f>(data.colors.data(), data.colors.size()),
        SkSpan<const float>(data.positions.data(), data.positions.size()),
        to_sk_tile_mode(spread), SkColorSpace::MakeSRGB());
    SkGradient::Interpolation interpolation;
    interpolation.fInPremul = SkGradient::Interpolation::InPremul::kYes;
    interpolation.fColorSpace = SkGradient::Interpolation::ColorSpace::kSRGB;
    return SkGradient(colors, interpolation);
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

    using ImageMap = std::unordered_map<std::uint64_t, DecodedImage>;

    struct TextRun final {
        std::size_t offset{};
        std::size_t length{};
        sk_sp<SkTypeface> face;
    };

    sk_sp<SkSurface> surface;
#if defined(GUI_FORMS_PREPARED_TEXT)
    PreparedSkiaFrame prepared_frame{};
    bool prepared_mode{false};
#endif
    sk_sp<SkFontMgr> fonts{
#if defined(__APPLE__)
        SkFontMgr_New_CoreText(nullptr)
#else
        SkFontMgr_New_Custom_Empty()
#endif
    };
    ImageMap images;
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

    [[nodiscard]] bool register_font_data(std::optional<FontRole> role,
                                          std::uint16_t weight, bool italic,
                                          sk_sp<SkData> data) {
        if (!data || (*data).isEmpty() || (*data).size() > 64U * 1024U * 1024U) return false;
        sk_sp<SkTypeface> face = (*fonts).makeFromData(data);
        if (!face) return false;
#if defined(GUI_FORMS_HAS_HARFBUZZ_TEXT)
        const std::span<const std::byte> bytes(
            static_cast<const std::byte*>((*data).data()), (*data).size());
        const std::shared_ptr<const sk_sp<SkData>> owner =
            std::make_shared<const sk_sp<SkData>>(std::move(data));
        const std::optional<FontFaceId> text_face = text_engine.register_owned_typeface(
            role, weight, italic, bytes, owner);
        if (!text_face) return false;
        registered_typefaces.push_back(
            {role.value_or(FontRole::content), weight, italic, !role, std::move(face), *text_face});
#else
        registered_typefaces.push_back(
            {role.value_or(FontRole::content), weight, italic, !role, std::move(face)});
#endif
        return true;
    }

    [[nodiscard]] SkCanvas* canvas() const noexcept {
        SkCanvas* result = nullptr;
#if defined(GUI_FORMS_PREPARED_TEXT)
        if (prepared_mode) {
            result = prepared_frame.canvas();
            return result;
        }
#endif
        if (surface) result = (*surface).getCanvas();
        return result;
    }

    [[nodiscard]] SkSurface* front_surface() const noexcept {
        SkSurface* result = surface.get();
#if defined(GUI_FORMS_PREPARED_TEXT)
        if (prepared_mode) result = prepared_frame.front();
#endif
        return result;
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
            return (*registered).face;
        }

        const SkFontStyle style = font_style(spec);
        const char* family = nullptr;
        if (spec.role == FontRole::control) {
            family = "Lucida Grande";
        } else if (spec.role == FontRole::monospace) {
            family = "Menlo";
        }
        sk_sp<SkTypeface> face = (*fonts).matchFamilyStyle(family, style);
        if (!face) {
            face = (*fonts).legacyMakeTypeface(nullptr, style);
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
            if (scalar >= 0 && (!face || (*face).unicharToGlyph(scalar) == 0U)) {
                for (const RegisteredTypeface& candidate : registered_typefaces) {
                    const bool house_body_fallback =
                        spec.role != FontRole::content && !candidate.fallback &&
                        candidate.role == FontRole::content;
                    if ((candidate.fallback || house_body_fallback) && candidate.face &&
                        (*candidate.face).unicharToGlyph(scalar) != 0U) {
                        face = candidate.face;
                        break;
                    }
                }
            }
            if (scalar >= 0 && (!face || (*face).unicharToGlyph(scalar) == 0U)) {
                face = (*fonts).matchFamilyStyleCharacter(
                    nullptr, style, nullptr, 0, scalar);
                if (!face) {
                    face = primary;
                }
            }
            const bool same_face = (!current && !face) ||
                (current && face && (*current).uniqueID() == (*face).uniqueID());
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

bool SkiaRaster::register_typeface(FontRole role, std::uint16_t weight,
                                   bool italic, std::span<const std::byte> encoded) {
    if (encoded.empty() || encoded.size() > 64U * 1024U * 1024U) return false;
    sk_sp<SkData> data = SkData::MakeWithCopy(encoded.data(), encoded.size());
    const bool registered = (*impl_).register_font_data(role, weight, italic, std::move(data));
    return registered;
}

bool SkiaRaster::register_fallback_typeface(
    std::uint16_t weight, bool italic, std::span<const std::byte> encoded) {
    if (encoded.empty() || encoded.size() > 64U * 1024U * 1024U) return false;
    sk_sp<SkData> data = SkData::MakeWithCopy(encoded.data(), encoded.size());
    const bool registered = (*impl_).register_font_data(std::nullopt, weight, italic, std::move(data));
    return registered;
}

bool SkiaRaster::register_typeface_file(FontRole role, std::uint16_t weight,
                                        bool italic, const char* path) {
    sk_sp<SkData> data = SkData::MakeFromFileName(path);
    const bool registered = (*impl_).register_font_data(role, weight, italic, std::move(data));
    return registered;
}

bool SkiaRaster::register_fallback_typeface_file(
    std::uint16_t weight, bool italic, const char* path) {
    sk_sp<SkData> data = SkData::MakeFromFileName(path);
    const bool registered = (*impl_).register_font_data(std::nullopt, weight, italic, std::move(data));
    return registered;
}

bool SkiaRaster::resize(Size logical_size, double scale) {
#if defined(GUI_FORMS_PREPARED_TEXT)
    // A host using transactions must not replace its coherent front via the
    // historical resize path. Size admission belongs to begin_prepared_frame.
    if ((*impl_).prepared_mode) return false;
#endif
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
    if ((*impl_).surface && width == (*(*impl_).surface).width() &&
        height == (*(*impl_).surface).height() && scale == (*impl_).scale) {
        (*impl_).logical_size = logical_size;
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
    (*(*replacement).getCanvas()).clear(SK_ColorTRANSPARENT);
    (*impl_).surface = std::move(replacement);
    (*impl_).logical_size = logical_size;
    (*impl_).scale = scale;
    return true;
}

void SkiaRaster::begin_frame(const DamageRegion& damage) {
    SkCanvas* canvas = (*impl_).canvas();
    if (!canvas) {
        return;
    }
    (*canvas).restoreToCount(1);
    (*canvas).resetMatrix();
    (*canvas).scale(static_cast<SkScalar>((*impl_).scale),
                  static_cast<SkScalar>((*impl_).scale));
    (*canvas).save();
    (*impl_).save_floor = (*canvas).getSaveCount();

    if (!damage.empty()) {
        SkRegion region;
        for (Rect rect : damage.rectangles()) {
            const SkIRect pixels = SkIRect::MakeLTRB(
                static_cast<int>(std::floor(rect.x * (*impl_).scale)),
                static_cast<int>(std::floor(rect.y * (*impl_).scale)),
                static_cast<int>(std::ceil((rect.x + rect.width) * (*impl_).scale)),
                static_cast<int>(std::ceil((rect.y + rect.height) * (*impl_).scale)));
            region.op(pixels, SkRegion::kUnion_Op);
        }
        (*canvas).resetMatrix();
        (*canvas).clipRegion(region);
        (*canvas).scale(static_cast<SkScalar>((*impl_).scale),
                      static_cast<SkScalar>((*impl_).scale));
    }
}

void SkiaRaster::end_frame() {
    if (SkCanvas* canvas = (*impl_).canvas()) {
        (*canvas).restoreToCount(1);
    }
}

#if defined(GUI_FORMS_PREPARED_TEXT)
PreparedTextStatus SkiaRaster::begin_prepared_frame(const Size logical_size,
    const double scale, DamageRegion& damage) {
    if (!(*impl_).prepared_frame.on_executor()) return PreparedTextStatus::wrong_executor;
    const PreparedTextStatus status = (*impl_).prepared_frame.begin(logical_size, scale, damage);
    if (status == PreparedTextStatus::success) {
        // Select the route only after admission. Failed initial geometry and
        // allocation preserve any ordinary surface; it has no prepared receipt.
        (*impl_).prepared_mode = true;
        (*impl_).surface.reset();
        (*impl_).logical_size = logical_size;
        (*impl_).scale = scale;
        try { begin_frame(damage); }
        catch (...) { (*impl_).prepared_frame.abort(); throw; }
    }
    return status;
}
PreparedTextStatus SkiaRaster::commit_prepared_frame(const PaintReceipt receipt) {
    const PreparedTextStatus status = (*impl_).prepared_frame.commit(receipt);
    return status;
}
void SkiaRaster::abort_prepared_frame() noexcept { (*impl_).prepared_frame.abort(); }
PaintReceipt SkiaRaster::prepared_front_receipt() const noexcept {
    const PaintReceipt result = (*impl_).prepared_frame.receipt();
    return result;
}
double SkiaRaster::prepared_front_scale() const noexcept {
    const double result = (*impl_).prepared_frame.front_scale();
    return result;
}
bool SkiaRaster::prepared_front_matches(const Size logical_size, const double scale) const noexcept {
    const bool result = (*impl_).prepared_frame.matches(logical_size, scale);
    return result;
}
PreparedTextPaintResult SkiaRaster::draw_prepared_text(const PreparedTextLayout& layout,
    const LayoutAuthority expected, const Point baseline, const Color color) {
    const PreparedTextPaintResult result = (*impl_).prepared_frame.draw(layout, expected, baseline, color);
    return result;
}
#endif

bool SkiaRaster::synchronize_images(const ImageRegistry& registry) {
    const ImageRegistrySnapshot snapshot = registry.snapshot();
    const bool same_registry = &registry == (*impl_).image_registry;
    if (same_registry && snapshot.revision == (*impl_).image_registry_revision) {
        return (*impl_).image_registry_synchronized;
    }
    if (!same_registry) {
        (*impl_).images.clear();
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
        const Impl::ImageMap::const_iterator existing = (*impl_).images.find(id.value);
        if (existing != (*impl_).images.end() &&
            (*existing).second.content_hash == (*resource).content_hash) {
            continue;
        }

        if ((*resource).metadata.width >
                static_cast<std::uint32_t>(std::numeric_limits<int>::max()) ||
            (*resource).metadata.height >
                static_cast<std::uint32_t>(std::numeric_limits<int>::max()) ||
            (*resource).metadata.decoded_byte_count >
                std::numeric_limits<std::size_t>::max()) {
            (*impl_).images.erase(id.value);
            synchronized = false;
            continue;
        }

        const SkImageInfo output_info = SkImageInfo::Make(
            static_cast<int>((*resource).metadata.width),
            static_cast<int>((*resource).metadata.height),
            (*resource).encoding == ImageResourceEncoding::bgra32_premultiplied
                ? kBGRA_8888_SkColorType : kRGBA_8888_SkColorType,
            kPremul_SkAlphaType,
            SkColorSpace::MakeSRGB());
        const std::size_t row_bytes = output_info.minRowBytes();
        if (row_bytes != static_cast<std::size_t>((*resource).metadata.width) * 4U ||
            output_info.computeByteSize(row_bytes) !=
                (*resource).metadata.decoded_byte_count) {
            (*impl_).images.erase(id.value);
            synchronized = false;
            continue;
        }
        std::vector<std::byte> pixels;
        sk_sp<SkData> pixel_data;
        if ((*resource).encoding == ImageResourceEncoding::bgra32_premultiplied) {
            if ((*resource).row_bytes != row_bytes ||
                (*resource).encoded.size() != (*resource).metadata.decoded_byte_count) {
                (*impl_).images.erase(id.value);
                synchronized = false;
                continue;
            }
            pixel_data = SkData::MakeWithCopy(
                (*resource).encoded.data(), (*resource).encoded.size());
        } else {
            sk_sp<SkData> encoded = SkData::MakeWithCopy(
                (*resource).encoded.data(), (*resource).encoded.size());
            SkCodec::Result codec_result = SkCodec::kInternalError;
            std::unique_ptr<SkCodec> codec =
                SkPngDecoder::Decode(std::move(encoded), &codec_result);
            if (!codec || codec_result != SkCodec::kSuccess ||
                (*codec).getInfo().width() !=
                    static_cast<int>((*resource).metadata.width) ||
                (*codec).getInfo().height() !=
                    static_cast<int>((*resource).metadata.height)) {
                (*impl_).images.erase(id.value);
                synchronized = false;
                continue;
            }
            pixels.resize((*resource).metadata.decoded_byte_count);
            if ((*codec).getPixels(output_info, pixels.data(), row_bytes) !=
                SkCodec::kSuccess) {
                (*impl_).images.erase(id.value);
                synchronized = false;
                continue;
            }
        }
        if (!pixel_data) {
            pixel_data = SkData::MakeWithCopy(pixels.data(), pixels.size());
        }
        sk_sp<SkImage> image =
            SkImages::RasterFromData(output_info, std::move(pixel_data), row_bytes);
        if (!image) {
            (*impl_).images.erase(id.value);
            synchronized = false;
            continue;
        }
        (*impl_).images[id.value] = {(*resource).content_hash, std::move(image)};
    }
    for (Impl::ImageMap::iterator iterator = (*impl_).images.begin();
         iterator != (*impl_).images.end();) {
        if (!active.contains((*iterator).first)) {
            iterator = (*impl_).images.erase(iterator);
        } else {
            ++iterator;
        }
    }
    (*impl_).image_registry_revision = snapshot.revision;
    (*impl_).image_registry = &registry;
    (*impl_).image_registry_synchronized = synchronized;
    return synchronized;
}

const void* SkiaRaster::pixels() const noexcept {
    SkPixmap pixmap{};
    SkSurface* const surface = (*impl_).front_surface();
    const void* result = nullptr;
    if (surface != nullptr && (*surface).peekPixels(&pixmap)) result = pixmap.addr();
    return result;
}

std::size_t SkiaRaster::row_bytes() const noexcept {
    SkPixmap pixmap{};
    SkSurface* const surface = (*impl_).front_surface();
    std::size_t result = 0;
    if (surface != nullptr && (*surface).peekPixels(&pixmap)) result = pixmap.rowBytes();
    return result;
}

std::uint32_t SkiaRaster::pixel_width() const noexcept {
    SkSurface* const surface = (*impl_).front_surface();
    std::uint32_t result = 0;
    if (surface != nullptr) result = static_cast<std::uint32_t>((*surface).width());
    return result;
}

std::uint32_t SkiaRaster::pixel_height() const noexcept {
    SkSurface* const surface = (*impl_).front_surface();
    std::uint32_t result = 0;
    if (surface != nullptr) result = static_cast<std::uint32_t>((*surface).height());
    return result;
}

std::size_t SkiaRaster::byte_size() const noexcept {
    return row_bytes() * pixel_height();
}

void SkiaRaster::save() {
    if (SkCanvas* canvas = (*impl_).canvas()) {
        (*canvas).save();
    }
}

void SkiaRaster::restore() {
    if (SkCanvas* canvas = (*impl_).canvas();
        canvas && (*canvas).getSaveCount() > (*impl_).save_floor) {
        (*canvas).restore();
    }
}

void SkiaRaster::translate(Point offset) {
    if (SkCanvas* canvas = (*impl_).canvas()) {
        (*canvas).translate(static_cast<SkScalar>(offset.x), static_cast<SkScalar>(offset.y));
    }
}

void SkiaRaster::clip_rect(Rect rect) {
    if (SkCanvas* canvas = (*impl_).canvas()) {
        (*canvas).clipRect(to_sk_rect(rect), SkClipOp::kIntersect, false);
    }
}

void SkiaRaster::clip_rounded_rect(Rect rect, double radius) {
    if (SkCanvas* canvas = (*impl_).canvas()) {
        (*canvas).clipRRect(to_sk_rrect(rect, radius), SkClipOp::kIntersect, true);
    }
}

void SkiaRaster::fill_rect(Rect rect, Color color) {
    if (SkCanvas* canvas = (*impl_).canvas()) {
        (*canvas).drawRect(to_sk_rect(rect), make_paint(color));
    }
}

void SkiaRaster::fill_rounded_rect(Rect rect, double radius, Color color) {
    if (SkCanvas* canvas = (*impl_).canvas()) {
        (*canvas).drawRRect(to_sk_rrect(rect, radius), make_paint(color));
    }
}

void SkiaRaster::stroke_rect(Rect rect, Color color, double width) {
    if (SkCanvas* canvas = (*impl_).canvas()) {
        SkPaint paint = make_paint(color);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(static_cast<SkScalar>(width));
        (*canvas).drawRect(to_sk_rect(rect), paint);
    }
}

void SkiaRaster::stroke_rounded_rect(Rect rect, double radius, Color color,
                                    double width) {
    if (SkCanvas* canvas = (*impl_).canvas()) {
        SkPaint paint = make_paint(color);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(static_cast<SkScalar>(width));
        (*canvas).drawRRect(to_sk_rrect(rect, radius), paint);
    }
}

void SkiaRaster::fill_linear_gradient(
    Rect rect, Point start, Point end,
    std::span<const GradientStop> stops) {
    fill_linear_gradient_spread(rect, start, end, stops,
                                GradientSpreadMode::pad);
}

void SkiaRaster::fill_linear_gradient_spread(
    Rect rect, Point start, Point end,
    std::span<const GradientStop> stops, GradientSpreadMode spread) {
    SkCanvas* canvas = (*impl_).canvas();
    if (canvas == nullptr || rect.empty() || !valid_gradient_stops(stops) ||
        (spread != GradientSpreadMode::pad &&
         spread != GradientSpreadMode::repeat &&
         spread != GradientSpreadMode::reflect)) {
        return;
    }
    const SkGradientData data = to_sk_gradient(stops);
    const SkPoint points[2]{{static_cast<SkScalar>(start.x),
                             static_cast<SkScalar>(start.y)},
                            {static_cast<SkScalar>(end.x),
                             static_cast<SkScalar>(end.y)}};
    sk_sp<SkShader> shader = SkShaders::LinearGradient(
        points, make_sk_gradient(data, spread));
    if (!shader) return;
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setShader(std::move(shader));
    (*canvas).drawRect(to_sk_rect(rect), paint);
}

void SkiaRaster::fill_radial_gradient(
    Rect rect, Point center, Size radii,
    std::span<const GradientStop> stops) {
    SkCanvas* canvas = (*impl_).canvas();
    if (canvas == nullptr || rect.empty() || radii.width <= 0.0 ||
        radii.height <= 0.0 || !valid_gradient_stops(stops)) {
        return;
    }
    const SkGradientData data = to_sk_gradient(stops);
    sk_sp<SkShader> shader = SkShaders::RadialGradient(
        {0.0F, 0.0F}, 1.0F, make_sk_gradient(data));
    if (!shader) return;
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setShader(std::move(shader));
    (*canvas).save();
    (*canvas).clipRect(to_sk_rect(rect), SkClipOp::kIntersect, true);
    (*canvas).translate(static_cast<SkScalar>(center.x),
                      static_cast<SkScalar>(center.y));
    (*canvas).scale(static_cast<SkScalar>(radii.width),
                  static_cast<SkScalar>(radii.height));
    (*canvas).drawRect(SkRect::MakeLTRB(
        static_cast<SkScalar>((rect.x - center.x) / radii.width),
        static_cast<SkScalar>((rect.y - center.y) / radii.height),
        static_cast<SkScalar>((rect.x + rect.width - center.x) / radii.width),
        static_cast<SkScalar>((rect.y + rect.height - center.y) / radii.height)),
        paint);
    (*canvas).restore();
}

void SkiaRaster::draw_box_shadow(Rect rect, double corner_radius, Point offset,
                                 double blur_radius, double spread,
                                 Color color) {
    SkCanvas* canvas = (*impl_).canvas();
    if (canvas == nullptr || rect.empty() || color.alpha == 0U ||
        blur_radius < 0.0) {
        return;
    }
    Rect shadow{rect.x + offset.x - spread, rect.y + offset.y - spread,
                rect.width + spread * 2.0, rect.height + spread * 2.0};
    if (shadow.empty()) return;
    SkPaint paint = make_paint(color);
    if (blur_radius > 0.0) {
        paint.setMaskFilter(SkMaskFilter::MakeBlur(
            kNormal_SkBlurStyle, static_cast<SkScalar>(blur_radius * 0.5),
            false));
    }
    (*canvas).drawRRect(to_sk_rrect(
        shadow, std::max(0.0, corner_radius + spread)), paint);
}

void SkiaRaster::draw_inset_box_shadow(
    Rect rect, double corner_radius, Point offset, double blur_radius,
    double spread, Color color) {
    SkCanvas* canvas = (*impl_).canvas();
    if (canvas == nullptr || rect.empty() || color.alpha == 0U ||
        blur_radius < 0.0) {
        return;
    }
    const Rect hole{rect.x + offset.x + spread,
                    rect.y + offset.y + spread,
                    rect.width - spread * 2.0,
                    rect.height - spread * 2.0};
    const double reach = std::max(rect.width, rect.height) +
                         std::abs(offset.x) + std::abs(offset.y) +
                         std::abs(spread) + blur_radius * 3.0 + 1.0;
    const Rect outside{rect.x - reach, rect.y - reach,
                       rect.width + reach * 2.0,
                       rect.height + reach * 2.0};
    SkPathBuilder complement_builder;
    complement_builder.setFillType(SkPathFillType::kEvenOdd);
    complement_builder.addRect(to_sk_rect(outside));
    if (!hole.empty()) {
        complement_builder.addRRect(to_sk_rrect(
            hole, std::max(0.0, corner_radius - spread)));
    }
    const SkPath complement = complement_builder.detach();
    SkPaint paint = make_paint(color);
    if (blur_radius > 0.0) {
        paint.setMaskFilter(SkMaskFilter::MakeBlur(
            kNormal_SkBlurStyle, static_cast<SkScalar>(blur_radius * 0.5),
            false));
    }
    (*canvas).save();
    (*canvas).clipRRect(to_sk_rrect(rect, corner_radius),
                       SkClipOp::kIntersect, true);
    (*canvas).drawPath(complement, paint);
    (*canvas).restore();
}

void SkiaRaster::draw_line(Point from, Point to, Color color, double width) {
    if (SkCanvas* canvas = (*impl_).canvas()) {
        SkPaint paint = make_paint(color);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(static_cast<SkScalar>(width));
        (*canvas).drawLine(static_cast<SkScalar>(from.x), static_cast<SkScalar>(from.y),
                         static_cast<SkScalar>(to.x), static_cast<SkScalar>(to.y), paint);
    }
}

void SkiaRaster::draw_text_utf8(Point origin,
                                std::string_view text,
                                FontSpec font_spec,
                                Color color) {
    if (SkCanvas* canvas = (*impl_).canvas(); canvas && !text.empty()) {
#if defined(GUI_FORMS_HAS_HARFBUZZ_TEXT)
        const text::ShapedText shaped = (*impl_).text_engine.shape(text, font_spec);
        const SkPaint paint = make_paint(color);
        for (const text::ShapedFontRun& run : shaped.runs) {
            const sk_sp<SkTypeface> face = (*impl_).typeface(run.face);
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
            (*canvas).drawGlyphs(SkSpan<const SkGlyphID>(glyphs),
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
        for (const Impl::TextRun& run : (*impl_).text_runs(text, font_spec)) {
            SkFont font(run.face, static_cast<SkScalar>(font_spec.size));
            font.setEdging(SkFont::Edging::kAntiAlias);
            const char* bytes = text.data() + run.offset;
            (*canvas).drawSimpleText(bytes, run.length, SkTextEncoding::kUTF8,
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
    const text::ShapedText shaped = (*impl_).text_engine.shape(text, font_spec);
    return {std::max(0.0, shaped.width), std::max(0.0, shaped.height)};
#else
    double width{};
    double height{};
    for (const Impl::TextRun& run : (*impl_).text_runs(text, font_spec)) {
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

ResolvedTextLayout SkiaRaster::resolve_text_layout_utf8(
    std::string_view text, FontSpec font_spec) {
#if defined(GUI_FORMS_HAS_HARFBUZZ_TEXT)
    return (*impl_).text_engine.resolve(text, font_spec);
#else
    ResolvedTextLayout result = estimate_text_layout_utf8(text, font_spec);
    result.logical_size = measure_text_utf8(text, font_spec);
    return result;
#endif
}

void SkiaRaster::draw_image(ImageId image, Rect destination, double opacity) {
    const Impl::ImageMap::const_iterator found = (*impl_).images.find(image.value);
    if (SkCanvas* canvas = (*impl_).canvas();
        canvas && found != (*impl_).images.end() && opacity > 0.0) {
        SkPaint paint;
        paint.setAlphaf(static_cast<float>(std::clamp(opacity, 0.0, 1.0)));
        paint.setAntiAlias(true);
        (*canvas).drawImageRect((*found).second.image.get(), to_sk_rect(destination),
                              SkSamplingOptions(SkFilterMode::kLinear), &paint);
    }
}

void SkiaRaster::draw_live_surface(const std::shared_ptr<LiveSurface> surface,
                                   const Rect destination, const double opacity) {
    if (!surface) return;
    const LiveSurfaceFrame frame = (*surface).acquire_latest();
    static_cast<void>(draw_live_surface_frame(frame, destination, opacity));
}

bool SkiaRaster::draw_live_surface_frame(const LiveSurfaceFrame& frame,
                                         const Rect destination, const double opacity) {
    SkCanvas* const canvas = (*impl_).canvas();
    if (canvas == nullptr || destination.empty() ||
        !destination.finite() || !std::isfinite(opacity) || opacity <= 0.0) {
        return false;
    }
    if (!frame || frame.width() == 0U || frame.height() == 0U ||
        frame.row_bytes() < static_cast<std::uint64_t>(frame.width()) * 4U ||
        frame.pixels().empty()) {
        return false;
    }
    const bool copy = frame.opaque() && opacity >= 1.0;
    const SkAlphaType alpha = copy ? kOpaque_SkAlphaType : kPremul_SkAlphaType;
    if (fail_next_live_image_) {
        fail_next_live_image_ = false;
        return false;
    }
    const SkImageInfo info = SkImageInfo::Make(
        static_cast<int>(frame.width()), static_cast<int>(frame.height()),
        kBGRA_8888_SkColorType, alpha,
        SkColorSpace::MakeSRGB());
    sk_sp<SkData> data = SkData::MakeWithoutCopy(
        frame.pixels().data(), frame.pixels().size());
    sk_sp<SkImage> image = SkImages::RasterFromData(
        info, std::move(data), static_cast<size_t>(frame.row_bytes()));
    if (!image) return false;

    SkPaint paint{};
    if (copy) paint.setBlendMode(SkBlendMode::kSrc);
    paint.setAlphaf(static_cast<float>(std::clamp(opacity, 0.0, 1.0)));
    paint.setAntiAlias(false);
    (*canvas).drawImageRect(
        image.get(),
        SkRect::MakeWH(static_cast<SkScalar>(frame.width()),
                       static_cast<SkScalar>(frame.height())),
        to_sk_rect(destination),
        SkSamplingOptions(SkFilterMode::kLinear), &paint,
        SkCanvas::kStrict_SrcRectConstraint);
    return true;
}

void SkiaRaster::draw_image_region(ImageId image, Rect source,
                                   Rect destination, double opacity) {
    draw_image_region_sampled(image, source, destination,
                              ImageSampling::linear, opacity);
}

void SkiaRaster::draw_image_region_sampled(
    ImageId image, Rect source, Rect destination, ImageSampling sampling,
    double opacity) {
    const Impl::ImageMap::const_iterator found = (*impl_).images.find(image.value);
    SkCanvas* canvas = (*impl_).canvas();
    if (canvas == nullptr || found == (*impl_).images.end() || source.empty() ||
        destination.empty() || !source.finite() || !destination.finite() ||
        !std::isfinite(opacity) || opacity <= 0.0) {
        return;
    }
    const Rect image_bounds{
        0.0, 0.0,
        static_cast<double>((*(*found).second.image).width()),
        static_cast<double>((*(*found).second.image).height())};
    if (!image_bounds.contains(source)) return;
    SkPaint paint;
    paint.setAlphaf(static_cast<float>(std::clamp(opacity, 0.0, 1.0)));
    // Sampling filters pixels; rectangle coverage must stay hard-edged.
    // Coverage AA attenuates a tile edge even through a disjoint hard clip
    // when its one-pixel sampling gutter is subpixel after minification.
    paint.setAntiAlias(false);
    (*canvas).drawImageRect((*found).second.image.get(), to_sk_rect(source),
                          to_sk_rect(destination),
                          SkSamplingOptions(sampling == ImageSampling::nearest
                              ? SkFilterMode::kNearest : SkFilterMode::kLinear),
                          &paint,
                          SkCanvas::kStrict_SrcRectConstraint);
}

void SkiaRaster::fill_image_pattern(
    ImageId image, Size source_pixel_size, Rect destination,
    Size logical_tile_size, ImagePatternWrap wrap, double opacity) {
    const Impl::ImageMap::const_iterator found = (*impl_).images.find(image.value);
    SkCanvas* canvas = (*impl_).canvas();
    if (canvas == nullptr || found == (*impl_).images.end() ||
        wrap != ImagePatternWrap::tile || destination.empty() ||
        !destination.finite() || !std::isfinite(source_pixel_size.width) ||
        !std::isfinite(source_pixel_size.height) ||
        !std::isfinite(logical_tile_size.width) ||
        !std::isfinite(logical_tile_size.height) ||
        source_pixel_size.width != (*(*found).second.image).width() ||
        source_pixel_size.height != (*(*found).second.image).height() ||
        logical_tile_size.width <= 0.0 || logical_tile_size.height <= 0.0 ||
        !std::isfinite(opacity) || opacity <= 0.0) {
        return;
    }
    SkMatrix local;
    local.setScaleTranslate(
        static_cast<SkScalar>(logical_tile_size.width /
                              source_pixel_size.width),
        static_cast<SkScalar>(logical_tile_size.height /
                              source_pixel_size.height),
        static_cast<SkScalar>(destination.x),
        static_cast<SkScalar>(destination.y));
    SkPaint paint;
    paint.setAlphaf(static_cast<float>(std::clamp(opacity, 0.0, 1.0)));
    paint.setAntiAlias(false);
    paint.setShader((*(*found).second.image).makeShader(
        SkTileMode::kRepeat, SkTileMode::kRepeat,
        SkSamplingOptions(SkFilterMode::kLinear), local));
    (*canvas).drawRect(to_sk_rect(destination), paint);
}

} // namespace gui_forms::render
