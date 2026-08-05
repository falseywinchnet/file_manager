#include "drawing_platform.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <cstring>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <vector>

namespace gui_drawing::abi::platform {
namespace {

struct HdcLease final {
    Bitmap* bitmap{};
    std::thread::id owner;
    HDC device{};
    HBITMAP dib{};
    HGDIOBJ previous{};
    void* pixels{};
    std::size_t byte_count{};
};

std::mutex lease_mutex;
std::unordered_map<std::uint64_t, HdcLease> leases;
std::uint64_t next_lease{1U};

BITMAPINFO bitmap_info(std::uint32_t width, std::uint32_t height) {
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = static_cast<LONG>(width);
    info.bmiHeader.biHeight = -static_cast<LONG>(height);
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    return info;
}

void copy_as_bgra(const ImageSnapshot& snapshot, void* destination) {
    const std::span<const std::byte> source = snapshot.pixels();
    auto* output = static_cast<std::byte*>(destination);
    std::copy(source.begin(), source.end(), output);
    if (snapshot.pixel_format == PixelFormat::rgba32_premultiplied) {
        for (std::size_t offset = 0; offset < source.size(); offset += 4U) {
            std::swap(output[offset], output[offset + 2U]);
        }
    }
}

} // namespace

gd_result export_hbitmap(Bitmap& bitmap, Color background, std::uintptr_t& output) {
    const ImageSnapshot snapshot = bitmap.snapshot();
    BITMAPINFO info = bitmap_info(snapshot.width, snapshot.height);
    void* pixels = nullptr;
    HBITMAP native = CreateDIBSection(nullptr, &info, DIB_RGB_COLORS,
                                      &pixels, nullptr, 0U);
    if (native == nullptr || pixels == nullptr) {
        if (native != nullptr) DeleteObject(native);
        throw std::runtime_error("CreateDIBSection failed");
    }
    const std::uint8_t br = background.is_empty() ? 0U : background.red();
    const std::uint8_t bg = background.is_empty() ? 0U : background.green();
    const std::uint8_t bb = background.is_empty() ? 0U : background.blue();
    const std::span<const std::byte> source = snapshot.pixels();
    auto* destination = static_cast<std::uint8_t*>(pixels);
    for (std::size_t offset = 0; offset < source.size(); offset += 4U) {
        const auto channel = [&](std::size_t index) {
            return std::to_integer<std::uint8_t>(source[offset + index]);
        };
        const bool bgra = snapshot.pixel_format == PixelFormat::bgra32_premultiplied;
        const std::uint8_t blue = channel(bgra ? 0U : 2U);
        const std::uint8_t green = channel(1U);
        const std::uint8_t red = channel(bgra ? 2U : 0U);
        const std::uint8_t alpha = channel(3U);
        const auto composite = [alpha](std::uint8_t foreground,
                                        std::uint8_t backdrop) {
            return static_cast<std::uint8_t>(foreground +
                (static_cast<unsigned>(backdrop) * (255U - alpha) + 127U) / 255U);
        };
        destination[offset] = composite(blue, bb);
        destination[offset + 1U] = composite(green, bg);
        destination[offset + 2U] = composite(red, br);
        destination[offset + 3U] = 255U;
    }
    output = reinterpret_cast<std::uintptr_t>(native);
    return GD_OK;
}

gd_result import_hbitmap(std::uintptr_t source, std::unique_ptr<Bitmap>& output) {
    const HBITMAP native = reinterpret_cast<HBITMAP>(source);
    BITMAP details{};
    if (GetObjectW(native, sizeof(details), &details) != sizeof(details) ||
        details.bmWidth <= 0 || details.bmHeight == 0) {
        throw std::invalid_argument("native HBITMAP is invalid");
    }
    const auto width = static_cast<std::uint32_t>(details.bmWidth);
    const auto height = static_cast<std::uint32_t>(
        details.bmHeight < 0 ? -static_cast<std::int64_t>(details.bmHeight) :
                               details.bmHeight);
    auto bitmap = std::make_unique<Bitmap>(width, height,
                                           PixelFormat::bgra32_premultiplied);
    BitmapLockView lock = bitmap->lock(BitmapLockMode::write);
    BITMAPINFO info = bitmap_info(width, height);
    HDC device = GetDC(nullptr);
    if (device == nullptr) throw std::runtime_error("GetDC failed");
    const int rows = GetDIBits(device, native, 0U, height,
                               lock.writable_data, &info, DIB_RGB_COLORS);
    ReleaseDC(nullptr, device);
    if (rows != static_cast<int>(height)) {
        bitmap->unlock(lock.token);
        throw std::runtime_error("GetDIBits failed");
    }
    for (std::uint32_t y = 0; y < height; ++y) {
        auto* row = reinterpret_cast<std::uint8_t*>(lock.writable_data) +
                    static_cast<std::size_t>(y) * lock.row_bytes;
        for (std::uint32_t x = 0; x < width; ++x) row[x * 4U + 3U] = 255U;
    }
    bitmap->unlock(lock.token);
    output = std::move(bitmap);
    return GD_OK;
}

gd_result capture_surface(std::uintptr_t source, std::uint32_t kind,
                          CapturedSurface& output) {
    const bool window_surface = kind == GD_NATIVE_SURFACE_HWND;
    const HWND window = window_surface ? reinterpret_cast<HWND>(source) : nullptr;
    HDC device = window_surface ? GetDC(window) : reinterpret_cast<HDC>(source);
    if (device == nullptr) throw std::invalid_argument("surface has no device context");
    RECT bounds{};
    bool have_bounds = window_surface ? GetClientRect(window, &bounds) != 0 :
                                        GetClipBox(device, &bounds) != ERROR;
    if (!have_bounds || bounds.right <= bounds.left || bounds.bottom <= bounds.top) {
        bounds = {0, 0, GetDeviceCaps(device, HORZRES), GetDeviceCaps(device, VERTRES)};
    }
    const std::int64_t width64 = static_cast<std::int64_t>(bounds.right) - bounds.left;
    const std::int64_t height64 = static_cast<std::int64_t>(bounds.bottom) - bounds.top;
    if (width64 <= 0 || height64 <= 0) {
        if (window_surface) ReleaseDC(window, device);
        throw std::invalid_argument("native surface has empty bounds");
    }
    if (width64 > Bitmap::maximum_dimension || height64 > Bitmap::maximum_dimension) {
        if (window_surface) ReleaseDC(window, device);
        throw std::length_error("native surface exceeds bitmap limits");
    }
    const auto width = static_cast<std::uint32_t>(width64);
    const auto height = static_cast<std::uint32_t>(height64);
    std::vector<std::byte> captured(static_cast<std::size_t>(width) * height * 4U);
    BITMAPINFO info = bitmap_info(width, height);
    void* pixels = nullptr;
    HBITMAP dib = CreateDIBSection(device, &info, DIB_RGB_COLORS,
                                   &pixels, nullptr, 0U);
    HDC memory = dib == nullptr ? nullptr : CreateCompatibleDC(device);
    HGDIOBJ previous = memory == nullptr ? nullptr : SelectObject(memory, dib);
    const bool ok = previous != nullptr && previous != HGDI_ERROR &&
        BitBlt(memory, 0, 0, static_cast<int>(width), static_cast<int>(height),
               device, bounds.left, bounds.top, SRCCOPY | CAPTUREBLT) != 0;
    if (window_surface) ReleaseDC(window, device);
    if (!ok) {
        if (memory != nullptr && previous != nullptr && previous != HGDI_ERROR) {
            SelectObject(memory, previous);
        }
        if (memory != nullptr) DeleteDC(memory);
        if (dib != nullptr) DeleteObject(dib);
        throw std::runtime_error("BitBlt failed");
    }
    std::memcpy(captured.data(), pixels, captured.size());
    SelectObject(memory, previous);
    DeleteDC(memory);
    DeleteObject(dib);
    auto bitmap = std::make_unique<Bitmap>(width, height,
                                           PixelFormat::bgra32_premultiplied);
    BitmapLockView lock = bitmap->lock(BitmapLockMode::write);
    std::memcpy(lock.writable_data, captured.data(), captured.size());
    for (std::uint32_t y = 0; y < height; ++y) {
        auto* row = reinterpret_cast<std::uint8_t*>(lock.writable_data) +
                    static_cast<std::size_t>(y) * lock.row_bytes;
        for (std::uint32_t x = 0; x < width; ++x) row[x * 4U + 3U] = 255U;
    }
    bitmap->unlock(lock.token);
    output.bitmap = std::move(bitmap);
    output.bounds = {static_cast<double>(bounds.left), static_cast<double>(bounds.top),
                     static_cast<double>(width), static_cast<double>(height)};
    return GD_OK;
}

gd_result present_surface(std::uintptr_t destination, std::uint32_t kind,
                          Bitmap& bitmap) {
    const ImageSnapshot snapshot = bitmap.snapshot();
    std::vector<std::byte> converted(snapshot.pixels().size());
    copy_as_bgra(snapshot, converted.data());
    const bool window_surface = kind == GD_NATIVE_SURFACE_HWND;
    const HWND window = window_surface ? reinterpret_cast<HWND>(destination) : nullptr;
    HDC device = window_surface ? GetDC(window) : reinterpret_cast<HDC>(destination);
    if (device == nullptr) throw std::invalid_argument("surface has no device context");
    RECT bounds{};
    if (window_surface) GetClientRect(window, &bounds);
    else if (GetClipBox(device, &bounds) == ERROR) bounds = {};
    const int width = bounds.right > bounds.left ? bounds.right - bounds.left :
                                                      static_cast<int>(snapshot.width);
    const int height = bounds.bottom > bounds.top ? bounds.bottom - bounds.top :
                                                        static_cast<int>(snapshot.height);
    BITMAPINFO info = bitmap_info(snapshot.width, snapshot.height);
    const int copied = StretchDIBits(device, bounds.left, bounds.top, width, height,
                                     0, 0, snapshot.width, snapshot.height,
                                     converted.data(), &info, DIB_RGB_COLORS, SRCCOPY);
    if (window_surface) ReleaseDC(window, device);
    if (copied == GDI_ERROR) throw std::runtime_error("StretchDIBits failed");
    return GD_OK;
}

gd_result acquire_hdc(Bitmap& bitmap, std::uintptr_t& output,
                      std::uint64_t& lease_token) {
    const ImageSnapshot snapshot = bitmap.snapshot();
    BITMAPINFO info = bitmap_info(snapshot.width, snapshot.height);
    void* pixels = nullptr;
    HBITMAP dib = CreateDIBSection(nullptr, &info, DIB_RGB_COLORS,
                                   &pixels, nullptr, 0U);
    HDC device = dib == nullptr ? nullptr : CreateCompatibleDC(nullptr);
    HGDIOBJ previous = device == nullptr ? nullptr : SelectObject(device, dib);
    if (pixels == nullptr || previous == nullptr || previous == HGDI_ERROR) {
        if (device != nullptr && previous != nullptr && previous != HGDI_ERROR) {
            SelectObject(device, previous);
        }
        if (device != nullptr) DeleteDC(device);
        if (dib != nullptr) DeleteObject(dib);
        throw std::runtime_error("bitmap-backed HDC creation failed");
    }
    copy_as_bgra(snapshot, pixels);
    std::scoped_lock lock(lease_mutex);
    const std::uint64_t token = next_lease++;
    if (token == 0U) {
        SelectObject(device, previous);
        DeleteDC(device);
        DeleteObject(dib);
        throw std::length_error("HDC lease token space exhausted");
    }
    try {
        leases.emplace(token, HdcLease{&bitmap, std::this_thread::get_id(),
                       device, dib, previous, pixels, snapshot.pixels().size()});
    } catch (...) {
        SelectObject(device, previous);
        DeleteDC(device);
        DeleteObject(dib);
        throw;
    }
    output = reinterpret_cast<std::uintptr_t>(device);
    lease_token = token;
    return GD_OK;
}

gd_result release_hdc(Bitmap& bitmap, std::uint64_t lease_token) {
    HdcLease lease;
    {
        std::scoped_lock lock(lease_mutex);
        const auto found = leases.find(lease_token);
        if (found == leases.end()) throw std::invalid_argument("HDC lease token is stale");
        if (found->second.bitmap != &bitmap) {
            throw std::invalid_argument("HDC lease belongs to a different bitmap");
        }
        if (found->second.owner != std::this_thread::get_id()) {
            return GD_ERROR_WRONG_THREAD;
        }
        lease = found->second;
        leases.erase(found);
    }
    struct LeaseCleanup final {
        HdcLease& lease;
        ~LeaseCleanup() {
            SelectObject(lease.device, lease.previous);
            DeleteDC(lease.device);
            DeleteObject(lease.dib);
        }
    } cleanup{lease};
    auto* pixels = static_cast<std::uint8_t*>(lease.pixels);
    for (std::size_t offset = 0; offset < lease.byte_count; offset += 4U) {
        if (pixels[offset + 3U] == 0U &&
            (pixels[offset] != 0U || pixels[offset + 1U] != 0U ||
             pixels[offset + 2U] != 0U)) pixels[offset + 3U] = 255U;
    }
    BitmapLockView lock = bitmap.lock(BitmapLockMode::write);
    std::memcpy(lock.writable_data, lease.pixels, lease.byte_count);
    bitmap.unlock(lock.token);
    return GD_OK;
}

bool has_hdc_lease(Bitmap& bitmap) {
    std::scoped_lock lock(lease_mutex);
    return std::any_of(leases.begin(), leases.end(), [&](const auto& entry) {
        return entry.second.bitmap == &bitmap;
    });
}

} // namespace gui_drawing::abi::platform
