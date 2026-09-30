#import <AppKit/AppKit.h>
#include "gui_forms/platform/macos_host.hpp"

#include <array>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
}
int main() {
    @autoreleasepool {
        NSPasteboard* pasteboard = [NSPasteboard pasteboardWithUniqueName];
        gui_forms::host::MacHostServiceOptions options;
        options.clipboard_name = pasteboard.name.UTF8String;
        const std::unique_ptr<gui_forms::HostServices> services = gui_forms::host::make_macos_host_services(options);
        const std::array<std::byte, 16> pixels{
            std::byte{255}, std::byte{0}, std::byte{0}, std::byte{255},
            std::byte{0}, std::byte{255}, std::byte{0}, std::byte{255},
            std::byte{0}, std::byte{0}, std::byte{255}, std::byte{255},
            std::byte{231}, std::byte{43}, std::byte{19}, std::byte{0}};
        require((*services).write_clipboard_image({2, 2, 8, pixels}).accepted(), "native image write failed");
        gui_forms::HostClipboardImageResult result = (*services).read_clipboard_image();
        require(result.status.accepted() && result.has_image && result.image.width == 2 && result.image.height == 2 &&
                result.image.pixels == std::vector<std::byte>(pixels.begin(), pixels.end()),
                "private native clipboard roundtrip was not exact");
        require((*services).write_clipboard_image({2, 2, 7, pixels}).error == gui_forms::HostServiceError::invalid_argument,
                "invalid image was published");
        require((*services).read_clipboard_image().generation == result.generation,
                "invalid write changed the native pasteboard");
        NSData* png = [pasteboard dataForType:NSPasteboardTypePNG];
        require(png != nil && png.length != 0, "interoperable PNG representation missing");
        [pasteboard clearContents];
        require([pasteboard setData:png forType:NSPasteboardTypePNG], "PNG-only fixture write failed");
        result = (*services).read_clipboard_image();
        require(result.status.accepted() && result.has_image && result.image.width == 2 && result.image.height == 2,
                "PNG from an external clipboard producer was not decoded");
        for (std::size_t index = 0; index < 12U; ++index) {
            require(result.image.pixels[index] == pixels[index], "native PNG changed row orientation or opaque colors");
        }
        require(result.image.pixels[15] == std::byte{0}, "native PNG lost transparent alpha");
        require((*services).write_clipboard_text("clipboard image test").accepted() &&
                !(*services).read_clipboard_image().has_image, "native text did not replace image");
        [pasteboard clearContents];
        NSURL* file = [NSURL fileURLWithPath:@"/tmp/Rainstar copied image Ω.png"];
        require([pasteboard writeObjects:@[file]], "file reference fixture failed");
        require([pasteboard setData:png forType:NSPasteboardTypePNG], "file icon fixture failed");
        gui_forms::HostClipboardFilesResult files = (*services).read_clipboard_files();
        require(files.status.accepted() && files.paths_utf8.size() == 1 &&
                files.paths_utf8.front() == "/tmp/Rainstar copied image Ω.png",
                "file reference was lost when the clipboard also offered an icon");
        require((*services).write_clipboard_text("ordinary text").accepted() &&
                (*services).read_clipboard_files().paths_utf8.empty(),
                "ordinary clipboard text must not become a file reference");
        [pasteboard releaseGlobally];
        std::cout << "isolated macOS clipboard image roundtrip and PNG interchange passed\n";
    }
}
