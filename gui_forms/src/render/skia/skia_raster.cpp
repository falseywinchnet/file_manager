#include "skia_raster.hpp"

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
#if defined(__APPLE__)
#include "include/ports/SkFontMgr_mac_ct.h"
#else
#include "include/ports/SkFontMgr_empty.h"
#endif

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
        sk_sp<SkTypeface> face;
    };

    struct DecodedImage final {
        std::uint64_t content_hash{};
        sk_sp<SkImage> image;
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
    Size logical_size{};
    double scale{1.0};
    int save_floor{1};
    const ImageRegistry* image_registry{};
    std::uint64_t image_registry_revision{std::numeric_limits<std::uint64_t>::max()};
    bool image_registry_synchronized{true};

    [[nodiscard]] SkCanvas* canvas() const noexcept {
        return surface ? surface->getCanvas() : nullptr;
    }

    [[nodiscard]] sk_sp<SkTypeface> typeface(FontSpec spec) const {
        const RegisteredTypeface* registered = nullptr;
        int registered_distance = std::numeric_limits<int>::max();
        for (const RegisteredTypeface& candidate : registered_typefaces) {
            if (candidate.role != spec.role || candidate.italic != spec.italic) {
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

        const SkFontStyle::Slant slant = spec.italic
            ? SkFontStyle::kItalic_Slant
            : SkFontStyle::kUpright_Slant;
        const SkFontStyle style(static_cast<int>(spec.weight),
                                SkFontStyle::kNormal_Width,
                                slant);
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
    impl_->registered_typefaces.push_back({role, weight, italic, std::move(face)});
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

    const SkImageInfo info = SkImageInfo::MakeN32Premul(
        width, height, SkColorSpace::MakeSRGB());
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

        sk_sp<SkData> encoded =
            SkData::MakeWithCopy(resource->encoded.data(), resource->encoded.size());
        SkCodec::Result codec_result = SkCodec::kInternalError;
        std::unique_ptr<SkCodec> codec =
            SkPngDecoder::Decode(std::move(encoded), &codec_result);
        if (!codec || codec_result != SkCodec::kSuccess ||
            codec->getInfo().width() != static_cast<int>(resource->metadata.width) ||
            codec->getInfo().height() != static_cast<int>(resource->metadata.height)) {
            impl_->images.erase(id.value);
            synchronized = false;
            continue;
        }

        const SkImageInfo output_info = SkImageInfo::Make(
            static_cast<int>(resource->metadata.width),
            static_cast<int>(resource->metadata.height),
            kRGBA_8888_SkColorType, kPremul_SkAlphaType,
            SkColorSpace::MakeSRGB());
        const std::size_t row_bytes = output_info.minRowBytes();
        if (row_bytes != static_cast<std::size_t>(resource->metadata.width) * 4U ||
            output_info.computeByteSize(row_bytes) !=
                resource->metadata.decoded_byte_count) {
            impl_->images.erase(id.value);
            synchronized = false;
            continue;
        }
        std::vector<std::byte> pixels(resource->metadata.decoded_byte_count);
        if (codec->getPixels(output_info, pixels.data(), row_bytes) != SkCodec::kSuccess) {
            impl_->images.erase(id.value);
            synchronized = false;
            continue;
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
        SkFont font(impl_->typeface(font_spec), static_cast<SkScalar>(font_spec.size));
        font.setEdging(SkFont::Edging::kAntiAlias);
        canvas->drawSimpleText(text.data(), text.size(), SkTextEncoding::kUTF8,
                               static_cast<SkScalar>(origin.x),
                               static_cast<SkScalar>(origin.y), font, make_paint(color));
    }
}

Size SkiaRaster::measure_text_utf8(std::string_view text,
                                   FontSpec font_spec) {
    if (text.empty()) return {0.0, font_spec.size};
    SkFont font(impl_->typeface(font_spec), static_cast<SkScalar>(font_spec.size));
    SkRect bounds{};
    const SkScalar width = font.measureText(text.data(), text.size(),
                                            SkTextEncoding::kUTF8, &bounds);
    SkFontMetrics metrics{};
    font.getMetrics(&metrics);
    return {std::max(0.0, static_cast<double>(width)),
            std::max(0.0, static_cast<double>(metrics.fDescent - metrics.fAscent))};
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
