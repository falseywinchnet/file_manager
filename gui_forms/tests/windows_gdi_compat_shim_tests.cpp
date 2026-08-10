#include "gui_forms/c_api.h"

#include <windows.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>

namespace {

using ShimGetDC = HDC(WINAPI*)(HWND);
using ShimReleaseDC = int(WINAPI*)(HWND, HDC);
using ShimBitBlt = BOOL(WINAPI*)(HDC, int, int, int, int, HDC, int, int, DWORD);
using ShimCreateCompatibleBitmap = HBITMAP(WINAPI*)(HDC, int, int);
using ShimCreateCompatibleDC = HDC(WINAPI*)(HDC);
using ShimScrollWindowEx = int(WINAPI*)(HWND, int, int, const RECT*,
                                        const RECT*, HRGN, LPRECT, UINT);
using ShimSendMessageW = LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM);
using ShimSelectObject = HGDIOBJ(WINAPI*)(HDC, HGDIOBJ);
using ShimDeleteObject = BOOL(WINAPI*)(HGDIOBJ);

template <typename Function>
Function entry(HMODULE module, const char* name) {
    const FARPROC raw = GetProcAddress(module, name);
    Function result{};
    static_assert(sizeof(result) == sizeof(raw));
    std::memcpy(&result, &raw, sizeof(result));
    return result;
}

void run_resize_writer(std::atomic<bool>* start,
                       std::atomic<bool>* writer_ok,
                       ShimBitBlt bit_blt,
                       HDC endpoint_dc,
                       HDC producer) {
    while (!(*start).load(std::memory_order_acquire)) {
        std::this_thread::yield();
    }
    for (int frame = 0; frame < 4000; ++frame) {
        if (bit_blt(endpoint_dc, 0, 0, 24, 48, producer,
                    frame % 100, frame % 100, SRCCOPY) == FALSE) {
            (*writer_ok).store(false, std::memory_order_release);
            return;
        }
    }
}

void run_release_writer(std::uintptr_t endpoint_dc,
                        std::atomic<bool>* lease_acquired,
                        std::atomic<bool>* allow_end,
                        std::atomic<bool>* writer_ok) {
    std::uint64_t write_lease{};
    if (gf_windows_paint_endpoint_begin_write_v1(
            endpoint_dc, &write_lease) != GF_OK) {
        (*lease_acquired).store(true, std::memory_order_release);
        return;
    }
    (*lease_acquired).store(true, std::memory_order_release);
    while (!(*allow_end).load(std::memory_order_acquire)) {
        std::this_thread::yield();
    }
    (*writer_ok).store(
        gf_windows_paint_endpoint_end_write_v1(write_lease, 0U) == GF_OK,
        std::memory_order_release);
}

void run_endpoint_releaser(std::uint64_t endpoint,
                           std::atomic<bool>* lease_acquired,
                           std::atomic<bool>* release_started,
                           std::atomic<bool>* release_ok) {
    while (!(*lease_acquired).load(std::memory_order_acquire)) {
        std::this_thread::yield();
    }
    (*release_started).store(true, std::memory_order_release);
    (*release_ok).store(
        gf_windows_paint_endpoint_release_v1(endpoint) == GF_OK,
        std::memory_order_release);
}

gf_string_view text(const char* value) {
    return {value, static_cast<std::uint64_t>(std::strlen(value))};
}

bool run_lifetime(const gf_api_v0& api, HMODULE shim, int lifetime,
                  int width, int height, int resized_width,
                  int resized_height) {
    const ShimGetDC get_dc = entry<ShimGetDC>(shim, "GetDC");
    const ShimReleaseDC release_dc =
        entry<ShimReleaseDC>(shim, "ReleaseDC");
    const ShimBitBlt bit_blt = entry<ShimBitBlt>(shim, "BitBlt");
    const ShimCreateCompatibleBitmap create_bitmap =
        entry<ShimCreateCompatibleBitmap>(
        shim, "CreateCompatibleBitmap");
    const ShimCreateCompatibleDC create_dc = entry<ShimCreateCompatibleDC>(
        shim, "CreateCompatibleDC");
    const ShimSelectObject select_object =
        entry<ShimSelectObject>(shim, "SelectObject");
    const ShimDeleteObject delete_object =
        entry<ShimDeleteObject>(shim, "DeleteObject");
    const ShimScrollWindowEx scroll_window =
        entry<ShimScrollWindowEx>(shim, "ScrollWindowEx");
    const ShimSendMessageW send_message =
        entry<ShimSendMessageW>(shim, "SendMessageW");
    if (!get_dc || !release_dc || !bit_blt || !create_bitmap || !create_dc ||
        !select_object || !delete_object || !scroll_window || !send_message) {
        std::fprintf(stderr, "shim-gate=failure|stage:exports\n");
        return false;
    }

    gf_handle control{};
    std::uint64_t endpoint{};
    std::uintptr_t window_value{};
    HDC producer{};
    HBITMAP producer_bitmap{};
    HGDIOBJ producer_previous{};
    HDC endpoint_dc{};
    bool ok = false;

    if (api.control_create_kind(GF_CONTROL_CUSTOM,
                                text("shim.endpoint.test"), &control) != GF_OK) {
        std::fprintf(stderr,
                     "shim-gate=failure|lifetime:%d|stage:control-create\n",
                     lifetime);
        goto cleanup;
    }
    if (gf_windows_paint_endpoint_acquire_v1(
            control, static_cast<std::uint32_t>(width),
            static_cast<std::uint32_t>(height), &endpoint,
            &window_value) != GF_OK || endpoint == 0U || window_value == 0U) {
        std::fprintf(stderr,
                     "shim-gate=failure|lifetime:%d|stage:endpoint-acquire\n",
                     lifetime);
        goto cleanup;
    }
    if (IsWindow(reinterpret_cast<HWND>(window_value)) != FALSE) {
        std::fprintf(stderr,
                     "shim-gate=failure|lifetime:%d|stage:allocated-hwnd\n",
                     lifetime);
        goto cleanup;
    }

    endpoint_dc = get_dc(reinterpret_cast<HWND>(window_value));
    if (endpoint_dc == nullptr) {
        std::fprintf(stderr,
                     "shim-gate=failure|lifetime:%d|stage:get-dc\n", lifetime);
        goto cleanup;
    }
    producer_bitmap = create_bitmap(endpoint_dc, width, height);
    producer = create_dc(endpoint_dc);
    if (producer_bitmap == nullptr || producer == nullptr) {
        std::fprintf(stderr,
                     "shim-gate=failure|lifetime:%d|stage:compatible-create\n",
                     lifetime);
        goto cleanup;
    }
    producer_previous = select_object(producer, producer_bitmap);
    if (producer_previous == nullptr || producer_previous == HGDI_ERROR) {
        std::fprintf(stderr,
                     "shim-gate=failure|lifetime:%d|stage:select\n", lifetime);
        goto cleanup;
    }

    for (int frame = 0; frame < 64; ++frame) {
        const COLORREF color = RGB((frame * 37 + lifetime) & 0xff,
                                   (frame * 67 + lifetime * 3) & 0xff,
                                   (frame * 97 + lifetime * 7) & 0xff);
        HBRUSH brush = CreateSolidBrush(color);
        RECT bounds{0, 0, width, height};
        if (brush == nullptr || FillRect(producer, &bounds, brush) == 0) {
            if (brush != nullptr) DeleteObject(brush);
            std::fprintf(stderr,
                         "shim-gate=failure|lifetime:%d|frame:%d|stage:fill\n",
                         lifetime, frame);
            goto cleanup;
        }
        DeleteObject(brush);
        if (bit_blt(endpoint_dc, 0, 0, width, height, producer, 0, 0,
                    SRCCOPY) == FALSE) {
            std::fprintf(stderr,
                         "shim-gate=failure|lifetime:%d|frame:%d|stage:bitblt\n",
                         lifetime, frame);
            goto cleanup;
        }
    }

    {
        std::array<char, 256> snapshot{};
        std::uint64_t required{};
        if (gf_windows_paint_endpoint_snapshot_v1(
                endpoint, snapshot.data(), snapshot.size(), &required) != GF_OK) {
            std::fprintf(stderr,
                         "shim-gate=failure|lifetime:%d|stage:snapshot\n",
                         lifetime);
            goto cleanup;
        }
        const std::string state(snapshot.data(),
                                static_cast<std::size_t>(required));
        if (state.find("state:live") == std::string::npos ||
            state.find("content:64") == std::string::npos) {
            std::fprintf(stderr,
                         "shim-gate=failure|lifetime:%d|stage:publish|state:%s\n",
                         lifetime, state.c_str());
            goto cleanup;
        }
    }

    if (lifetime == 0) {
        const COLORREF marker = RGB(241, 17, 93);
        if (SetPixel(producer, 0, 0, marker) == CLR_INVALID ||
            bit_blt(endpoint_dc, 0, 0, width, height, producer, 0, 0,
                    SRCCOPY) == FALSE) {
            std::fprintf(stderr, "shim-gate=failure|stage:scroll-seed\n");
            goto cleanup;
        }
        RECT scroll_bounds{0, 0, width, height};
        RECT exposed{};
        const COLORREF producer_pixel = GetPixel(producer, 0, 0);
        const COLORREF before_scroll = GetPixel(endpoint_dc, 0, 0);
        if (scroll_window(reinterpret_cast<HWND>(window_value), 1, 0,
                          &scroll_bounds, &scroll_bounds, nullptr, &exposed,
                          SW_INVALIDATE) == ERROR ||
            GetPixel(endpoint_dc, 1, 0) != marker) {
            std::fprintf(stderr,
                         "shim-gate=failure|stage:virtual-scroll|before:%lu|"
                         "after:%lu|producer:%lu|marker:%lu\n",
                         static_cast<unsigned long>(before_scroll),
                         static_cast<unsigned long>(GetPixel(endpoint_dc, 1, 0)),
                         static_cast<unsigned long>(producer_pixel),
                         static_cast<unsigned long>(marker));
            DIBSECTION producer_section{};
            DIBSECTION endpoint_section{};
            const HGDIOBJ endpoint_bitmap =
                GetCurrentObject(endpoint_dc, OBJ_BITMAP);
            const int producer_object_size = GetObjectW(
                producer_bitmap, sizeof(producer_section), &producer_section);
            const int endpoint_object_size = GetObjectW(
                endpoint_bitmap, sizeof(endpoint_section), &endpoint_section);
            std::fprintf(stderr,
                         "shim-gate=diagnostic|producer-object:%d|bm-height:%ld|"
                         "bi-height:%ld|endpoint-object:%d|bm-height:%ld|"
                         "bi-height:%ld\n",
                         producer_object_size,
                         static_cast<long>(producer_section.dsBm.bmHeight),
                         static_cast<long>(producer_section.dsBmih.biHeight),
                         endpoint_object_size,
                         static_cast<long>(endpoint_section.dsBm.bmHeight),
                         static_cast<long>(endpoint_section.dsBmih.biHeight));
            goto cleanup;
        }
        if (send_message(reinterpret_cast<HWND>(window_value), 0x83f1U,
                         0, 0) != 0) {
            std::fprintf(stderr, "shim-gate=failure|stage:virtual-message\n");
            goto cleanup;
        }
    }

    if (gf_windows_paint_endpoint_configure_v1(
            endpoint, static_cast<std::uint32_t>(resized_width),
            static_cast<std::uint32_t>(resized_height)) != GF_OK ||
        get_dc(reinterpret_cast<HWND>(window_value)) != endpoint_dc) {
        std::fprintf(stderr,
                     "shim-gate=failure|lifetime:%d|stage:resize-stability\n",
                     lifetime);
        goto cleanup;
    }
    ok = true;

cleanup:
    if (producer != nullptr && producer_previous != nullptr &&
        producer_previous != HGDI_ERROR) {
        select_object(producer, producer_previous);
    }
    if (producer_bitmap != nullptr) delete_object(producer_bitmap);
    if (producer != nullptr) DeleteDC(producer);
    if (endpoint_dc != nullptr) {
        release_dc(reinterpret_cast<HWND>(window_value), endpoint_dc);
    }
    if (endpoint != 0U) gf_windows_paint_endpoint_release_v1(endpoint);
    if (control.slot != 0U || control.generation != 0U) api.dispose(control);
    return ok;
}

bool run_concurrent_resize_gate(const gf_api_v0& api, HMODULE shim) {
    const ShimGetDC get_dc = entry<ShimGetDC>(shim, "GetDC");
    const ShimReleaseDC release_dc =
        entry<ShimReleaseDC>(shim, "ReleaseDC");
    const ShimBitBlt bit_blt = entry<ShimBitBlt>(shim, "BitBlt");
    const ShimCreateCompatibleBitmap create_bitmap =
        entry<ShimCreateCompatibleBitmap>(
        shim, "CreateCompatibleBitmap");
    const ShimCreateCompatibleDC create_dc = entry<ShimCreateCompatibleDC>(
        shim, "CreateCompatibleDC");
    const ShimSelectObject select_object =
        entry<ShimSelectObject>(shim, "SelectObject");
    const ShimDeleteObject delete_object =
        entry<ShimDeleteObject>(shim, "DeleteObject");
    if (!get_dc || !release_dc || !bit_blt || !create_bitmap || !create_dc ||
        !select_object || !delete_object) return false;

    gf_handle control{};
    std::uint64_t endpoint{};
    std::uintptr_t window_value{};
    if (api.control_create_kind(GF_CONTROL_CUSTOM, text("shim.race.test"),
                                &control) != GF_OK ||
        gf_windows_paint_endpoint_acquire_v1(
            control, 332U, 324U, &endpoint, &window_value) != GF_OK) {
        std::fprintf(stderr, "shim-gate=failure|stage:race-acquire\n");
        return false;
    }

    HDC endpoint_dc = get_dc(reinterpret_cast<HWND>(window_value));
    HDC producer = create_dc(endpoint_dc);
    HBITMAP producer_bitmap = create_bitmap(endpoint_dc, 332, 324);
    HGDIOBJ producer_previous = producer != nullptr && producer_bitmap != nullptr
        ? select_object(producer, producer_bitmap) : nullptr;
    if (endpoint_dc == nullptr || producer == nullptr ||
        producer_bitmap == nullptr || producer_previous == nullptr ||
        producer_previous == HGDI_ERROR) {
        std::fprintf(stderr, "shim-gate=failure|stage:race-backing\n");
        return false;
    }
    RECT producer_bounds{0, 0, 332, 324};
    HBRUSH brush = CreateSolidBrush(RGB(25, 80, 170));
    const bool filled = brush != nullptr &&
        FillRect(producer, &producer_bounds, brush) != 0;
    if (brush != nullptr) DeleteObject(brush);

    std::atomic<bool> start{};
    std::atomic<bool> writer_ok{filled};
    std::thread writer(&run_resize_writer, &start, &writer_ok, bit_blt,
                       endpoint_dc, producer);

    constexpr std::array<std::array<std::uint32_t, 2>, 6> sizes{{
        {{150U, 150U}}, {{90U, 150U}}, {{30U, 150U}},
        {{332U, 324U}}, {{303U, 324U}}, {{128U, 64U}},
    }};
    start.store(true, std::memory_order_release);
    bool configure_ok = true;
    for (int resize = 0; resize < 1200; ++resize) {
        const std::array<std::uint32_t, 2> size =
            sizes[static_cast<std::size_t>(resize) % sizes.size()];
        if (gf_windows_paint_endpoint_configure_v1(
                endpoint, size[0], size[1]) != GF_OK) {
            configure_ok = false;
            break;
        }
    }
    writer.join();

    if (producer_previous != nullptr && producer_previous != HGDI_ERROR) {
        select_object(producer, producer_previous);
    }
    delete_object(producer_bitmap);
    DeleteDC(producer);
    release_dc(reinterpret_cast<HWND>(window_value), endpoint_dc);
    gf_windows_paint_endpoint_release_v1(endpoint);
    api.dispose(control);
    if (!configure_ok || !writer_ok.load(std::memory_order_acquire)) {
        std::fprintf(stderr, "shim-gate=failure|stage:race-operation\n");
        return false;
    }
    return true;
}

bool run_dib_overlap_case(const gf_api_v0& api, HMODULE shim, bool top_down) {
    const ShimBitBlt bit_blt = entry<ShimBitBlt>(shim, "BitBlt");
    const ShimCreateCompatibleBitmap create_bitmap =
        entry<ShimCreateCompatibleBitmap>(
        shim, "CreateCompatibleBitmap");
    const ShimCreateCompatibleDC create_dc = entry<ShimCreateCompatibleDC>(
        shim, "CreateCompatibleDC");
    if (bit_blt == nullptr || create_bitmap == nullptr || create_dc == nullptr) {
        return false;
    }

    constexpr int width = 8;
    constexpr int height = 6;
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    gf_handle endpoint_control{};
    std::uint64_t endpoint{};
    std::uintptr_t endpoint_window{};
    std::uintptr_t endpoint_dc_value{};
    HDC compatibility_dc{};
    if (top_down &&
        (api.control_create_kind(GF_CONTROL_CUSTOM,
                                 text("shim.top-down.overlap"),
                                 &endpoint_control) != GF_OK ||
         gf_windows_paint_endpoint_acquire_v1(
             endpoint_control, width, height, &endpoint,
             &endpoint_window) != GF_OK ||
         gf_windows_paint_endpoint_get_dc_v1(
             endpoint_window, &endpoint_dc_value) != GF_OK)) {
        if (endpoint != 0U) gf_windows_paint_endpoint_release_v1(endpoint);
        if (endpoint_control.slot != 0U || endpoint_control.generation != 0U) {
            api.dispose(endpoint_control);
        }
        return false;
    }
    compatibility_dc = reinterpret_cast<HDC>(endpoint_dc_value);
    void* storage{};
    HBITMAP bitmap = top_down
        ? create_bitmap(compatibility_dc, width, height)
        : CreateDIBSection(nullptr, &info, DIB_RGB_COLORS,
                           &storage, nullptr, 0);
    HDC dc = top_down ? create_dc(compatibility_dc)
                      : CreateCompatibleDC(nullptr);
    HGDIOBJ previous = dc != nullptr && bitmap != nullptr
        ? SelectObject(dc, bitmap) : nullptr;
    if (bitmap == nullptr || dc == nullptr || (!top_down && storage == nullptr) ||
        previous == nullptr || previous == HGDI_ERROR) {
        if (bitmap != nullptr) DeleteObject(bitmap);
        if (dc != nullptr) DeleteDC(dc);
        if (endpoint_dc_value != 0U) {
            gf_windows_paint_endpoint_release_dc_v1(
                endpoint_window, endpoint_dc_value);
        }
        if (endpoint != 0U) gf_windows_paint_endpoint_release_v1(endpoint);
        if (endpoint_control.slot != 0U || endpoint_control.generation != 0U) {
            api.dispose(endpoint_control);
        }
        return false;
    }

    DIBSECTION observed{};
    const int observed_size = GetObjectW(bitmap, sizeof(observed), &observed);
    if (observed_size < static_cast<int>(sizeof(observed))) {
        SelectObject(dc, previous);
        DeleteObject(bitmap);
        DeleteDC(dc);
        if (endpoint_dc_value != 0U) {
            gf_windows_paint_endpoint_release_dc_v1(
                endpoint_window, endpoint_dc_value);
        }
        if (endpoint != 0U) gf_windows_paint_endpoint_release_v1(endpoint);
        if (endpoint_control.slot != 0U || endpoint_control.generation != 0U) {
            api.dispose(endpoint_control);
        }
        return false;
    }
    if (top_down) storage = observed.dsBm.bmBits;
    if (storage == nullptr) {
        SelectObject(dc, previous);
        DeleteObject(bitmap);
        DeleteDC(dc);
        gf_windows_paint_endpoint_release_dc_v1(
            endpoint_window, endpoint_dc_value);
        gf_windows_paint_endpoint_release_v1(endpoint);
        api.dispose(endpoint_control);
        return false;
    }
    std::uint32_t* pixels = static_cast<std::uint32_t*>(storage);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int storage_y = top_down ? y : height - 1 - y;
            pixels[storage_y * width + x] =
                0xff000000U | static_cast<std::uint32_t>((y + 1) * 0x010101);
        }
    }
    const bool copied = bit_blt(dc, 0, 1, width, height - 1,
                                dc, 0, 0, SRCCOPY) != FALSE;
    bool rows_match = copied;
    for (int y = 1; y < height && rows_match; ++y) {
        const std::uint32_t expected =
            0xff000000U | static_cast<std::uint32_t>(y * 0x010101);
        for (int x = 0; x < width; ++x) {
            const int storage_y = top_down ? y : height - 1 - y;
            if (pixels[storage_y * width + x] != expected) {
                std::fprintf(stderr,
                             "shim-gate=diagnostic|stage:dib-overlap|y:%d|"
                             "x:%d|actual:%08lx|expected:%08lx\n",
                             y, x,
                             static_cast<unsigned long>(
                                 pixels[storage_y * width + x]),
                             static_cast<unsigned long>(expected));
                rows_match = false;
                break;
            }
        }
    }
    SelectObject(dc, previous);
    DeleteObject(bitmap);
    DeleteDC(dc);
    if (endpoint_dc_value != 0U) {
        gf_windows_paint_endpoint_release_dc_v1(
            endpoint_window, endpoint_dc_value);
    }
    if (endpoint != 0U) gf_windows_paint_endpoint_release_v1(endpoint);
    if (endpoint_control.slot != 0U || endpoint_control.generation != 0U) {
        api.dispose(endpoint_control);
    }
    if (!rows_match) {
        std::fprintf(stderr,
                     "shim-gate=failure|stage:dib-overlap|orientation:%s|"
                     "bm-height:%ld|bi-height:%ld\n",
                     top_down ? "top-down" : "bottom-up",
                     static_cast<long>(observed.dsBm.bmHeight),
                     static_cast<long>(observed.dsBmih.biHeight));
    }
    return rows_match;
}

bool run_dib_overlap_gate(const gf_api_v0& api, HMODULE shim) {
    return run_dib_overlap_case(api, shim, false) &&
        run_dib_overlap_case(api, shim, true);
}

bool run_concurrent_release_gate(const gf_api_v0& api) {
    gf_handle control{};
    std::uint64_t endpoint{};
    std::uintptr_t window_value{};
    std::uintptr_t endpoint_dc{};
    if (api.control_create_kind(GF_CONTROL_CUSTOM, text("shim.release.test"),
                                &control) != GF_OK ||
        gf_windows_paint_endpoint_acquire_v1(
            control, 332U, 324U, &endpoint, &window_value) != GF_OK ||
        gf_windows_paint_endpoint_get_dc_v1(
            window_value, &endpoint_dc) != GF_OK) {
        std::fprintf(stderr, "shim-gate=failure|stage:release-acquire\n");
        return false;
    }

    std::atomic<bool> lease_acquired{};
    std::atomic<bool> allow_end{};
    std::atomic<bool> release_started{};
    std::atomic<bool> writer_ok{};
    std::atomic<bool> release_ok{};
    std::thread writer(&run_release_writer, endpoint_dc, &lease_acquired,
                       &allow_end, &writer_ok);
    std::thread releaser(&run_endpoint_releaser, endpoint, &lease_acquired,
                         &release_started, &release_ok);
    while (!release_started.load(std::memory_order_acquire)) {
        std::this_thread::yield();
    }
    // Give release an opportunity to reach the endpoint state lock. It must
    // wait there until the destination operation relinquishes its short lease.
    for (int spin = 0; spin < 1000; ++spin) std::this_thread::yield();
    allow_end.store(true, std::memory_order_release);
    writer.join();
    releaser.join();
    api.dispose(control);
    if (!writer_ok.load(std::memory_order_acquire) ||
        !release_ok.load(std::memory_order_acquire)) {
        std::fprintf(stderr, "shim-gate=failure|stage:release-operation\n");
        return false;
    }
    return true;
}

} // namespace

int main() {
    gf_api_v0 api{};
    api.struct_size = sizeof(api);
    if (gf_get_api_v0(GF_ABI_VERSION_0_24, &api) != GF_OK) {
        std::fprintf(stderr, "shim-gate=failure|stage:api\n");
        return 1;
    }
    HMODULE shim = LoadLibraryW(L"gui_forms_win32_compat.dll");
    if (shim == nullptr) {
        std::fprintf(stderr, "shim-gate=failure|stage:load\n");
        return 1;
    }
    constexpr std::array<std::array<int, 2>, 6> sizes{{
        {{150, 150}}, {{90, 150}}, {{30, 150}},
        {{332, 324}}, {{303, 324}}, {{128, 64}},
    }};
    for (int lifetime = 0; lifetime < 500; ++lifetime) {
        const std::array<int, 2> size =
            sizes[static_cast<std::size_t>(lifetime) % sizes.size()];
        const std::array<int, 2> resized = sizes[
            (static_cast<std::size_t>(lifetime) + 1U) % sizes.size()];
        if (!run_lifetime(api, shim, lifetime, size[0], size[1], resized[0],
                          resized[1])) {
            FreeLibrary(shim);
            return 1;
        }
    }
    if (!run_concurrent_resize_gate(api, shim)) {
        FreeLibrary(shim);
        return 1;
    }
    if (!run_dib_overlap_gate(api, shim)) {
        FreeLibrary(shim);
        return 1;
    }
    if (!run_concurrent_release_gate(api)) {
        FreeLibrary(shim);
        return 1;
    }
    FreeLibrary(shim);
    std::puts("shim-gate=pass|lifetimes:500|frames:36000|resize:concurrent|release:serialized");
    return 0;
}
