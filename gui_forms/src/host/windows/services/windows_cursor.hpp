#pragma once
#include "gui_forms/types/cursor_image/cursor_image.hpp"
#include <windows.h>
namespace gui_forms::host::windows_detail {
// Native resources are created only from the validated, immutable image set.
inline HCURSOR create_image_cursor(const CursorImages& images, const CursorImage& image) {
        const std::vector<unsigned char> zeros(static_cast<std::size_t>((image.width + 15) / 16 * 2) * image.height, 0);
        BITMAPV5HEADER header{};
        header.bV5Size = sizeof(header);
        header.bV5Width = image.width; header.bV5Height = -image.height;
        header.bV5Planes = 1; header.bV5BitCount = 32;
        header.bV5Compression = BI_BITFIELDS;
        header.bV5RedMask = 0x00ff0000; header.bV5GreenMask = 0x0000ff00;
        header.bV5BlueMask = 0x000000ff; header.bV5AlphaMask = 0xff000000;
        void* bits = nullptr;
        HDC dc = GetDC(nullptr);
        HBITMAP color = CreateDIBSection(dc, reinterpret_cast<BITMAPINFO*>(&header),
                                         DIB_RGB_COLORS, &bits, nullptr, 0);
        ReleaseDC(nullptr, dc);
        if (color == nullptr || bits == nullptr) {
            if (color != nullptr) DeleteObject(color);
            return nullptr;
        }
        std::uint32_t* pixels = static_cast<std::uint32_t*>(bits);
        for (std::size_t i = 0; i < image.rgba.size(); ++i) {
            const Color c = image.rgba[i];
            pixels[i] = (std::uint32_t(c.alpha) << 24) |
                (((std::uint32_t(c.red) * c.alpha + 127) / 255) << 16) |
                (((std::uint32_t(c.green) * c.alpha + 127) / 255) << 8) |
                 ((std::uint32_t(c.blue) * c.alpha + 127) / 255);
        }
        HBITMAP mask = CreateBitmap(image.width, image.height, 1, 1, zeros.data());
        ICONINFO info{};
        info.fIcon = FALSE; info.xHotspot = images.hotspot_x(image);
        info.yHotspot = images.hotspot_y(image); info.hbmColor = color; info.hbmMask = mask;
        HCURSOR cursor = mask == nullptr ? nullptr : static_cast<HCURSOR>(CreateIconIndirect(&info));
        DeleteObject(color);
        if (mask != nullptr) DeleteObject(mask);
        if (cursor == nullptr) return nullptr;
        return cursor;
}
} // namespace gui_forms::host::windows_detail
