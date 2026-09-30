#include "appkit_clipboard_image.hpp"
#include "../../../core/host/image/clipboard_image_wire.hpp"
#import <ImageIO/ImageIO.h>

#include <algorithm>
#include <cstring>
#include <limits>

namespace gui_forms::host::detail {
namespace {
NSString* const raw_type = @"org.guiforms.rgba8";

template <typename Reference>
class CFScoped final {
public:
    explicit CFScoped(Reference value) noexcept : value_(value) {}
    ~CFScoped() { if (value_ != nullptr) CFRelease(value_); }
    CFScoped(const CFScoped&) = delete;
    CFScoped& operator=(const CFScoped&) = delete;
    [[nodiscard]] Reference get() const noexcept { return value_; }
private:
    Reference value_;
};

HostClipboardImageResult decode_native_image(NSData* data) {
    HostClipboardImageResult result;
    result.status.error = HostServiceError::backend_failure;
    if (data.length > HostImage::maximum_bytes) {
        result.status.error = HostServiceError::too_large;
        return result;
    }
    const CFScoped<CGImageSourceRef> source(CGImageSourceCreateWithData(
        (__bridge CFDataRef)data, (__bridge CFDictionaryRef)@{(__bridge NSString*)kCGImageSourceShouldCache: @NO}));
    if (source.get() == nullptr) return result;
    const CFScoped<CFDictionaryRef> properties(CGImageSourceCopyPropertiesAtIndex(source.get(), 0, nullptr));
    if (properties.get() == nullptr) return result;
    NSDictionary* metadata = (__bridge NSDictionary*)properties.get();
    NSNumber* width_value = metadata[(__bridge NSString*)kCGImagePropertyPixelWidth];
    NSNumber* height_value = metadata[(__bridge NSString*)kCGImagePropertyPixelHeight];
    const std::uint64_t width = width_value.unsignedLongLongValue;
    const std::uint64_t height = height_value.unsignedLongLongValue;
    if (width == 0U || height == 0U) return result;
    if (width > HostImage::maximum_pixels || height > HostImage::maximum_pixels / width) {
        result.status.error = HostServiceError::too_large;
        return result;
    }
    const CFScoped<CGImageRef> native(CGImageSourceCreateImageAtIndex(source.get(), 0, nullptr));
    if (native.get() == nullptr || CGImageGetWidth(native.get()) != width ||
        CGImageGetHeight(native.get()) != height) return result;
    result.image.width = static_cast<std::uint32_t>(width);
    result.image.height = static_cast<std::uint32_t>(height);
    result.image.row_bytes = width * 4U;
    result.image.pixels.resize(static_cast<std::size_t>(width * height * 4U));
    const CFScoped<CGColorSpaceRef> color_space(CGColorSpaceCreateWithName(kCGColorSpaceSRGB));
    if (color_space.get() == nullptr) return result;
    const CFScoped<CGContextRef> context(CGBitmapContextCreate(
        result.image.pixels.data(), width, height, 8, width * 4U, color_space.get(),
        kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big));
    if (context.get() == nullptr) return result;
    CGContextSetBlendMode(context.get(), kCGBlendModeCopy);
    CGContextDrawImage(context.get(), CGRectMake(0, 0, width, height), native.get());
    // Native interchange may have premultiplied its channels. Convert once to
    // the service contract. Fully transparent native pixels have no recoverable
    // color; the private RGBA representation avoids that loss between our apps.
    for (std::size_t offset = 0; offset < result.image.pixels.size(); offset += 4U) {
        const unsigned alpha = std::to_integer<unsigned>(result.image.pixels[offset + 3U]);
        for (std::size_t channel = 0; channel < 3U; ++channel) {
            const unsigned value = std::to_integer<unsigned>(result.image.pixels[offset + channel]);
            result.image.pixels[offset + channel] = static_cast<std::byte>(alpha == 0U ? 0U :
                std::min(255U, (value * 255U + alpha / 2U) / alpha));
        }
    }
    result.has_image = true;
    result.status = {};
    return result;
}
} // namespace

HostClipboardImageResult read_appkit_clipboard_image(NSPasteboard* pasteboard) {
    HostClipboardImageResult result;
    if (pasteboard == nil) {
        result.status.error = HostServiceError::backend_failure;
        return result;
    }
    const std::uint64_t generation = static_cast<std::uint64_t>(pasteboard.changeCount);
    NSData* raw = [pasteboard dataForType:raw_type];
    if (raw != nil) {
        result = gui_forms::detail::decode_clipboard_image(
            {static_cast<const std::byte*>(raw.bytes), raw.length});
    } else {
        NSData* native = [pasteboard dataForType:NSPasteboardTypePNG];
        if (native == nil) native = [pasteboard dataForType:NSPasteboardTypeTIFF];
        if (native != nil) result = decode_native_image(native);
    }
    result.generation = generation;
    if (static_cast<std::uint64_t>(pasteboard.changeCount) != generation) {
        result = {};
        result.status.error = HostServiceError::backend_failure;
    }
    return result;
}

HostServiceStatus write_appkit_clipboard_image(NSPasteboard* pasteboard, HostImageView image) {
    if (pasteboard == nil) return {HostServiceError::backend_failure};
    const std::vector<std::byte> raw = gui_forms::detail::encode_clipboard_image(image);
    const std::size_t row_bytes = static_cast<std::size_t>(image.width) * 4U;
    NSBitmapImageRep* bitmap = [[NSBitmapImageRep alloc]
        initWithBitmapDataPlanes:nullptr pixelsWide:image.width pixelsHigh:image.height
        bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO
        colorSpaceName:NSDeviceRGBColorSpace bitmapFormat:NSBitmapFormatAlphaNonpremultiplied
        bytesPerRow:row_bytes bitsPerPixel:32];
    if (bitmap == nil || bitmap.bitmapData == nullptr) return {HostServiceError::backend_failure};
    for (std::uint32_t row = 0; row < image.height; ++row) {
        std::memcpy(bitmap.bitmapData + static_cast<std::size_t>(bitmap.bytesPerRow) * row,
                    image.pixels.data() + static_cast<std::size_t>(image.row_bytes) * row, row_bytes);
    }
    NSBitmapImageRep* tagged = [bitmap bitmapImageRepByRetaggingWithColorSpace:[NSColorSpace sRGBColorSpace]];
    NSData* png = [tagged representationUsingType:NSBitmapImageFileTypePNG properties:@{}];
    NSData* exact = [NSData dataWithBytes:raw.data() length:raw.size()];
    if (png == nil || exact == nil) return {HostServiceError::backend_failure};
    NSPasteboardItem* item = [[NSPasteboardItem alloc] init];
    if (![item setData:exact forType:raw_type] || ![item setData:png forType:NSPasteboardTypePNG]) {
        return {HostServiceError::backend_failure};
    }
    [pasteboard clearContents];
    return [pasteboard writeObjects:@[item]] ? HostServiceStatus{} :
        HostServiceStatus{HostServiceError::backend_failure};
}
} // namespace gui_forms::host::detail
