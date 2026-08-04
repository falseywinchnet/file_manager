#include "coregraphics_raster.hpp"
#include "../../core/device_damage.hpp"

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#include <ImageIO/ImageIO.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace gui_forms::render {
namespace {

CGRect to_cg_rect(Rect rect) noexcept {
    return CGRectMake(rect.x, rect.y, rect.width, rect.height);
}

void set_color(CGContextRef context, Color color) noexcept {
    constexpr CGFloat divisor = 255.0;
    CGContextSetRGBFillColor(context, color.red / divisor, color.green / divisor,
                             color.blue / divisor, color.alpha / divisor);
    CGContextSetRGBStrokeColor(context, color.red / divisor, color.green / divisor,
                               color.blue / divisor, color.alpha / divisor);
}

CFStringRef fallback_family(FontRole role) noexcept {
    switch (role) {
    case FontRole::control: return CFSTR("Lucida Grande");
    case FontRole::content: return CFSTR("Lucida Grande");
    case FontRole::monospace: return CFSTR("Menlo");
    }
    return CFSTR("Lucida Grande");
}

} // namespace

class CoreGraphicsRaster::Impl final {
public:
    struct RegisteredTypeface final {
        FontRole role{};
        std::uint16_t weight{};
        bool italic{};
        CTFontRef font{};

        RegisteredTypeface(FontRole role_value,
                           std::uint16_t weight_value,
                           bool italic_value,
                           CTFontRef font_value) noexcept
            : role(role_value), weight(weight_value), italic(italic_value),
              font(font_value) {}
        ~RegisteredTypeface() {
            if (font != nullptr) {
                CFRelease(font);
            }
        }
        RegisteredTypeface(RegisteredTypeface&& other) noexcept
            : role(other.role), weight(other.weight), italic(other.italic),
              font(std::exchange(other.font, nullptr)) {}
        RegisteredTypeface& operator=(RegisteredTypeface&& other) noexcept {
            if (this != &other) {
                if (font != nullptr) {
                    CFRelease(font);
                }
                role = other.role;
                weight = other.weight;
                italic = other.italic;
                font = std::exchange(other.font, nullptr);
            }
            return *this;
        }
        RegisteredTypeface(const RegisteredTypeface&) = delete;
        RegisteredTypeface& operator=(const RegisteredTypeface&) = delete;
    };

    struct DecodedImage final {
        std::uint64_t content_hash{};
        CGImageRef image{};

        DecodedImage() = default;
        DecodedImage(std::uint64_t hash, CGImageRef image_value) noexcept
            : content_hash(hash), image(image_value) {}
        ~DecodedImage() {
            if (image != nullptr) {
                CGImageRelease(image);
            }
        }
        DecodedImage(DecodedImage&& other) noexcept
            : content_hash(other.content_hash),
              image(std::exchange(other.image, nullptr)) {}
        DecodedImage& operator=(DecodedImage&& other) noexcept {
            if (this != &other) {
                if (image != nullptr) {
                    CGImageRelease(image);
                }
                content_hash = other.content_hash;
                image = std::exchange(other.image, nullptr);
            }
            return *this;
        }
        DecodedImage(const DecodedImage&) = delete;
        DecodedImage& operator=(const DecodedImage&) = delete;
    };

    ~Impl() {
        if (context != nullptr) {
            CGContextRelease(context);
        }
        if (color_space != nullptr) {
            CGColorSpaceRelease(color_space);
        }
    }

    [[nodiscard]] CTFontRef typeface(FontSpec spec) const {
        const RegisteredTypeface* selected = nullptr;
        int selected_distance = std::numeric_limits<int>::max();
        for (const RegisteredTypeface& candidate : registered_typefaces) {
            if (candidate.role != spec.role || candidate.italic != spec.italic) {
                continue;
            }
            const int distance = std::abs(static_cast<int>(candidate.weight) -
                                          static_cast<int>(spec.weight));
            if (distance < selected_distance) {
                selected = &candidate;
                selected_distance = distance;
            }
        }
        if (selected != nullptr) {
            return CTFontCreateCopyWithAttributes(
                selected->font, static_cast<CGFloat>(spec.size), nullptr, nullptr);
        }
        return CTFontCreateWithName(fallback_family(spec.role),
                                    static_cast<CGFloat>(spec.size), nullptr);
    }

    CGContextRef context{};
    CGColorSpaceRef color_space{CGColorSpaceCreateWithName(kCGColorSpaceSRGB)};
    std::vector<std::byte> storage;
    std::unordered_map<std::uint64_t, DecodedImage> images;
    std::vector<RegisteredTypeface> registered_typefaces;
    Size logical_size{};
    double scale{1.0};
    std::uint32_t width{};
    std::uint32_t height{};
    std::size_t row_bytes{};
    std::size_t nested_saves{};
    bool frame_active{};
    const ImageRegistry* image_registry{};
    std::uint64_t image_registry_revision{std::numeric_limits<std::uint64_t>::max()};
    bool image_registry_synchronized{true};
};

CoreGraphicsRaster::CoreGraphicsRaster() : impl_(std::make_unique<Impl>()) {}
CoreGraphicsRaster::~CoreGraphicsRaster() = default;

bool CoreGraphicsRaster::resize(Size logical_size, double scale) {
    if (!std::isfinite(scale) || scale <= 0.0 || logical_size.width <= 0.0 ||
        logical_size.height <= 0.0 || impl_->color_space == nullptr) {
        return false;
    }
    const double requested_width = std::ceil(logical_size.width * scale);
    const double requested_height = std::ceil(logical_size.height * scale);
    if (requested_width > static_cast<double>(std::numeric_limits<std::uint32_t>::max()) ||
        requested_height > static_cast<double>(std::numeric_limits<std::uint32_t>::max())) {
        return false;
    }
    const std::uint32_t width =
        std::max<std::uint32_t>(1, static_cast<std::uint32_t>(requested_width));
    const std::uint32_t height =
        std::max<std::uint32_t>(1, static_cast<std::uint32_t>(requested_height));
    if (impl_->context != nullptr && width == impl_->width && height == impl_->height &&
        scale == impl_->scale) {
        impl_->logical_size = logical_size;
        return false;
    }
    const std::size_t row_bytes = static_cast<std::size_t>(width) * 4U;
    if (height > std::numeric_limits<std::size_t>::max() / row_bytes) {
        return false;
    }

    if (impl_->context != nullptr) {
        CGContextRelease(impl_->context);
        impl_->context = nullptr;
    }
    impl_->storage.assign(row_bytes * height, std::byte{});
    constexpr CGBitmapInfo bitmap_info =
        static_cast<CGBitmapInfo>(kCGImageAlphaPremultipliedLast) |
        static_cast<CGBitmapInfo>(kCGBitmapByteOrder32Big);
    impl_->context = CGBitmapContextCreate(
        impl_->storage.data(), width, height, 8, row_bytes,
        impl_->color_space, bitmap_info);
    if (impl_->context == nullptr) {
        impl_->storage.clear();
        return false;
    }
    CGContextSetShouldAntialias(impl_->context, true);
    CGContextSetAllowsAntialiasing(impl_->context, true);
    CGContextClearRect(impl_->context, CGRectMake(0, 0, width, height));
    impl_->logical_size = logical_size;
    impl_->scale = scale;
    impl_->width = width;
    impl_->height = height;
    impl_->row_bytes = row_bytes;
    impl_->nested_saves = 0;
    impl_->frame_active = false;
    return true;
}

void CoreGraphicsRaster::begin_frame(const DamageRegion& damage) {
    if (impl_->context == nullptr) {
        return;
    }
    end_frame();
    CGContextSaveGState(impl_->context);
    impl_->frame_active = true;
    // Bitmap storage is addressed bottom-up by Quartz while GUI.Forms and its
    // byte-export contract are top-down. Establish the logical y-down surface
    // before applying any damage or display-chunk command.
    CGContextTranslateCTM(impl_->context, 0, impl_->height);
    CGContextScaleCTM(impl_->context, impl_->scale, -impl_->scale);
    if (!damage.empty()) {
        CGContextBeginPath(impl_->context);
        for (const Rect rect : damage.rectangles()) {
            // Damage is a device-pixel contract. A fractional logical edge
            // must include the whole device pixel or repeated partial paints
            // can leave an antialiased seam that a full paint does not.
            CGContextAddRect(impl_->context, to_cg_rect(
                detail::align_damage_outward(rect, impl_->scale)));
        }
        CGContextClip(impl_->context);
    }
}

void CoreGraphicsRaster::end_frame() {
    if (impl_->context == nullptr || !impl_->frame_active) {
        return;
    }
    while (impl_->nested_saves > 0) {
        CGContextRestoreGState(impl_->context);
        --impl_->nested_saves;
    }
    CGContextRestoreGState(impl_->context);
    impl_->frame_active = false;
}

bool CoreGraphicsRaster::register_typeface(FontRole role,
                                           std::uint16_t weight,
                                           bool italic,
                                           std::span<const std::byte> encoded) {
    if (encoded.empty()) {
        return false;
    }
    CFDataRef data = CFDataCreate(
        kCFAllocatorDefault,
        reinterpret_cast<const UInt8*>(encoded.data()),
        static_cast<CFIndex>(encoded.size()));
    if (data == nullptr) {
        return false;
    }
    CGDataProviderRef provider = CGDataProviderCreateWithCFData(data);
    CFRelease(data);
    if (provider == nullptr) {
        return false;
    }
    CGFontRef graphics_font = CGFontCreateWithDataProvider(provider);
    CGDataProviderRelease(provider);
    if (graphics_font == nullptr) {
        return false;
    }
    CTFontRef font = CTFontCreateWithGraphicsFont(graphics_font, 1.0, nullptr, nullptr);
    CGFontRelease(graphics_font);
    if (font == nullptr) {
        return false;
    }
    impl_->registered_typefaces.emplace_back(role, weight, italic, font);
    return true;
}

bool CoreGraphicsRaster::synchronize_images(const ImageRegistry& registry) {
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

        CFDataRef encoded = CFDataCreate(
            kCFAllocatorDefault,
            reinterpret_cast<const UInt8*>(resource->encoded.data()),
            static_cast<CFIndex>(resource->encoded.size()));
        CGImageSourceRef source = encoded == nullptr
            ? nullptr
            : CGImageSourceCreateWithData(encoded, nullptr);
        if (encoded != nullptr) {
            CFRelease(encoded);
        }
        CGImageRef source_image = source == nullptr
            ? nullptr
            : CGImageSourceCreateImageAtIndex(source, 0, nullptr);
        if (source != nullptr) {
            CFRelease(source);
        }
        if (source_image == nullptr ||
            CGImageGetWidth(source_image) != resource->metadata.width ||
            CGImageGetHeight(source_image) != resource->metadata.height) {
            if (source_image != nullptr) {
                CGImageRelease(source_image);
            }
            impl_->images.erase(id.value);
            synchronized = false;
            continue;
        }

        const std::size_t row_bytes =
            static_cast<std::size_t>(resource->metadata.width) * 4U;
        if (resource->metadata.height >
            std::numeric_limits<std::size_t>::max() / row_bytes) {
            CGImageRelease(source_image);
            impl_->images.erase(id.value);
            synchronized = false;
            continue;
        }
        std::vector<std::byte> pixels(row_bytes * resource->metadata.height);
        constexpr CGBitmapInfo bitmap_info =
            static_cast<CGBitmapInfo>(kCGImageAlphaPremultipliedLast) |
            static_cast<CGBitmapInfo>(kCGBitmapByteOrder32Big);
        CGContextRef decode_context = CGBitmapContextCreate(
            pixels.data(), resource->metadata.width, resource->metadata.height,
            8, row_bytes, impl_->color_space, bitmap_info);
        if (decode_context != nullptr) {
            CGContextTranslateCTM(decode_context, 0, resource->metadata.height);
            CGContextScaleCTM(decode_context, 1, -1);
            CGContextDrawImage(
                decode_context,
                CGRectMake(0, 0, resource->metadata.width, resource->metadata.height),
                source_image);
        }
        CGImageRelease(source_image);
        CGImageRef decoded = decode_context == nullptr
            ? nullptr
            : CGBitmapContextCreateImage(decode_context);
        if (decode_context != nullptr) {
            CGContextRelease(decode_context);
        }
        if (decoded == nullptr) {
            impl_->images.erase(id.value);
            synchronized = false;
            continue;
        }
        impl_->images.insert_or_assign(
            id.value, Impl::DecodedImage(resource->content_hash, decoded));
    }
    for (auto iterator = impl_->images.begin(); iterator != impl_->images.end();) {
        if (!active.contains(iterator->first)) {
            iterator = impl_->images.erase(iterator);
        } else {
            ++iterator;
        }
    }
    impl_->image_registry = &registry;
    impl_->image_registry_revision = snapshot.revision;
    impl_->image_registry_synchronized = synchronized;
    return synchronized;
}

const void* CoreGraphicsRaster::pixels() const noexcept {
    return impl_->storage.empty() ? nullptr : impl_->storage.data();
}

std::size_t CoreGraphicsRaster::row_bytes() const noexcept { return impl_->row_bytes; }
std::uint32_t CoreGraphicsRaster::pixel_width() const noexcept { return impl_->width; }
std::uint32_t CoreGraphicsRaster::pixel_height() const noexcept { return impl_->height; }
std::size_t CoreGraphicsRaster::byte_size() const noexcept { return impl_->storage.size(); }

void CoreGraphicsRaster::save() {
    if (impl_->context != nullptr && impl_->frame_active) {
        CGContextSaveGState(impl_->context);
        ++impl_->nested_saves;
    }
}

void CoreGraphicsRaster::restore() {
    if (impl_->context != nullptr && impl_->frame_active && impl_->nested_saves > 0) {
        CGContextRestoreGState(impl_->context);
        --impl_->nested_saves;
    }
}

void CoreGraphicsRaster::translate(Point offset) {
    if (impl_->context != nullptr) {
        CGContextTranslateCTM(impl_->context, offset.x, offset.y);
    }
}

void CoreGraphicsRaster::clip_rect(Rect rect) {
    if (impl_->context != nullptr) {
        CGContextClipToRect(impl_->context, to_cg_rect(rect));
    }
}

void CoreGraphicsRaster::fill_rect(Rect rect, Color color) {
    if (impl_->context != nullptr) {
        set_color(impl_->context, color);
        CGContextFillRect(impl_->context, to_cg_rect(rect));
    }
}

void CoreGraphicsRaster::stroke_rect(Rect rect, Color color, double width) {
    if (impl_->context != nullptr) {
        set_color(impl_->context, color);
        CGContextSetLineWidth(impl_->context, width);
        CGContextStrokeRect(impl_->context, to_cg_rect(rect));
    }
}

void CoreGraphicsRaster::draw_line(Point from, Point to, Color color, double width) {
    if (impl_->context != nullptr) {
        set_color(impl_->context, color);
        CGContextSetLineWidth(impl_->context, width);
        CGContextBeginPath(impl_->context);
        CGContextMoveToPoint(impl_->context, from.x, from.y);
        CGContextAddLineToPoint(impl_->context, to.x, to.y);
        CGContextStrokePath(impl_->context);
    }
}

void CoreGraphicsRaster::draw_text_utf8(Point origin,
                                        std::string_view text,
                                        FontSpec font_spec,
                                        Color color) {
    if (impl_->context == nullptr || text.empty()) {
        return;
    }
    CFStringRef string = CFStringCreateWithBytes(
        kCFAllocatorDefault,
        reinterpret_cast<const UInt8*>(text.data()),
        static_cast<CFIndex>(text.size()), kCFStringEncodingUTF8, false);
    CTFontRef font = impl_->typeface(font_spec);
    constexpr CGFloat divisor = 255.0;
    CGColorRef foreground = CGColorCreateGenericRGB(
        color.red / divisor, color.green / divisor, color.blue / divisor,
        color.alpha / divisor);
    if (string == nullptr || font == nullptr || foreground == nullptr) {
        if (string != nullptr) CFRelease(string);
        if (font != nullptr) CFRelease(font);
        if (foreground != nullptr) CGColorRelease(foreground);
        return;
    }
    const void* keys[]{kCTFontAttributeName, kCTForegroundColorAttributeName};
    const void* values[]{font, foreground};
    CFDictionaryRef attributes = CFDictionaryCreate(
        kCFAllocatorDefault, keys, values, 2,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFAttributedStringRef attributed = attributes == nullptr
        ? nullptr
        : CFAttributedStringCreate(kCFAllocatorDefault, string, attributes);
    CTLineRef line = attributed == nullptr ? nullptr : CTLineCreateWithAttributedString(attributed);
    if (line != nullptr) {
        CGContextSaveGState(impl_->context);
        CGContextSetTextMatrix(impl_->context, CGAffineTransformMakeScale(1, -1));
        CGContextSetTextPosition(impl_->context, origin.x, origin.y);
        CTLineDraw(line, impl_->context);
        CGContextRestoreGState(impl_->context);
        CFRelease(line);
    }
    if (attributed != nullptr) CFRelease(attributed);
    if (attributes != nullptr) CFRelease(attributes);
    CGColorRelease(foreground);
    CFRelease(font);
    CFRelease(string);
}

void CoreGraphicsRaster::draw_image(ImageId image, Rect destination, double opacity) {
    const auto found = impl_->images.find(image.value);
    if (impl_->context == nullptr || found == impl_->images.end() || opacity <= 0.0) {
        return;
    }
    CGContextSaveGState(impl_->context);
    CGContextSetAlpha(impl_->context, std::clamp(opacity, 0.0, 1.0));
    CGContextTranslateCTM(impl_->context, destination.x,
                          destination.y + destination.height);
    CGContextScaleCTM(impl_->context, 1, -1);
    CGContextSetInterpolationQuality(impl_->context, kCGInterpolationHigh);
    CGContextDrawImage(impl_->context,
                       CGRectMake(0, 0, destination.width, destination.height),
                       found->second.image);
    CGContextRestoreGState(impl_->context);
}

} // namespace gui_forms::render
