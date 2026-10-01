// Exercise the private native painter in the same translation unit.
#include "gui_forms/canvas.hpp"
#include "gui_forms/basic_controls.hpp"
#include "gui_forms/window.hpp"
#include "../src/host/windows/application/windows_host.cpp"
#include <fstream>
#include <iterator>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <chrono>
#include <string_view>

using namespace gui_forms;

void require_at(bool value, int line) { if (!value) { std::cerr << "failed at " << line << std::endl; std::abort(); } }
#define require(value) require_at((value), __LINE__)

void compare(ImageSampling sampling, double zoom, double scale, bool edited) {
    const std::shared_ptr<gui_drawing::Bitmap> bitmap = std::make_shared<gui_drawing::Bitmap>(1025, 1025);
    gui_drawing::BitmapEditView edit = (*bitmap).begin_edit({0, 0, 1025, 1025});
    for (int y = 0; y < 1025; ++y) for (int x = 0; x < 1025; ++x) {
        std::byte* pixel = edit.writable_data + y * edit.row_bytes + x * 4;
        const unsigned alpha = 80 + (x * 3 + y * 5) % 176;
        pixel[0] = std::byte((x % 251) * alpha / 255);
        pixel[1] = std::byte((y % 251) * alpha / 255);
        pixel[2] = std::byte(((x + y) % 251) * alpha / 255);
        pixel[3] = std::byte(alpha);
    }
    static_cast<void>((*bitmap).commit_edit(edit.token));
    const std::shared_ptr<RasterCanvas> canvas = make_control<RasterCanvas>(StableId("raster"));
    (*canvas).set_bitmap(bitmap);
    (*canvas).set_transparency_grid(false);
    (*canvas).set_canvas_background(Color::rgba(23, 37, 51));
    (*canvas).set_sampling(sampling);
    (*canvas).set_view(zoom, {470.3, 480.7});
    const Size size{180, 150};
    Window window(canvas, size);
    window.perform_layout();
    host::DibPainter tiled;
    require(tiled.resize(size, scale));
    require(tiled.synchronize_images(window.image_resources()));
    DamageRegion damage;
    damage.add({0, 0, size.width, size.height});
    tiled.begin_frame();
    require(window.paint(tiled).has_value());

    if (edited) {
        (*bitmap).set_pixel(511, 511, gui_drawing::Color::from_name("red"));
        (*bitmap).set_pixel(512, 512, gui_drawing::Color::from_name("blue"));
        require((*canvas).synchronize_bitmap());
        require(tiled.synchronize_images(window.image_resources()));
        tiled.begin_frame();
        require(window.paint(tiled).has_value());
    
    }
    const gui_drawing::ImageSnapshot snapshot = (*bitmap).snapshot();
    ImageRegistry registry;
    const ImageLoadResult loaded = registry.load_bgra32_premultiplied(1025, 1025, snapshot.row_bytes(), snapshot.pixels());
    require(bool(loaded));
    host::DibPainter reference;
    require(reference.resize(size, scale));
    require(reference.synchronize_images(registry));
    reference.begin_frame();
    reference.fill_rect({0, 0, size.width, size.height}, Color::rgba(23, 37, 51));
    reference.draw_image_region_sampled(loaded.image, {0, 0, 1025, 1025},
        {-470.3 * zoom, -480.7 * zoom, 1025 * zoom, 1025 * zoom}, sampling, 1);

    require(tiled.save_bmp(L"canvas_tiled_test.bmp"));
    require(reference.save_bmp(L"canvas_reference_test.bmp"));
    std::ifstream af("canvas_tiled_test.bmp", std::ios::binary);
    std::ifstream bf("canvas_reference_test.bmp", std::ios::binary);
    std::vector<unsigned char> a((std::istreambuf_iterator<char>(af)), {});
    std::vector<unsigned char> b((std::istreambuf_iterator<char>(bf)), {});
    require(a.size() == b.size());
    af.close(); bf.close();
    std::remove("canvas_tiled_test.bmp"); std::remove("canvas_reference_test.bmp");
    unsigned maximum{};
    std::size_t differences{};
    for (std::size_t index = 0; index < a.size(); ++index) {
        const unsigned distance = static_cast<unsigned>(std::abs(int(a[index]) - int(b[index])));
        maximum = std::max(maximum, distance);
        if (distance > 1) {

            ++differences;
        }
    }
    std::cout << "sampling=" << int(sampling) << " zoom=" << zoom << " scale=" << scale
              << " edited=" << edited << " max_channel_error=" << maximum << " differences=" << differences << '\n';
    require(maximum <= 1);
}
void disclosure_render() {
    const std::shared_ptr<DropDownButton> button = make_control<DropDownButton>(
        StableId("dropdown"), "Rotate", DropDownButtonMode::menu);
    Window window(button, {120, 40});
    window.perform_layout();
    host::DibPainter painter;
    require(painter.resize({120, 40}, 1));
    painter.begin_frame();
    require(window.paint(painter).has_value());
    require(painter.save_bmp(L"windows-dropdown-test.bmp"));
    std::ifstream stream("windows-dropdown-test.bmp", std::ios::binary);
    const std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(stream)), {});
    require(bytes.size() >= 54 + 120 * 40 * 4);
    const std::size_t pixel_offset = bytes[10] | (bytes[11] << 8) | (bytes[12] << 16) | (bytes[13] << 24);
    // GDI paints the seven, five, three, one pixel scanlines of the filled
    // indicator in the same text color as the control foreground.
    const Color color = (*button).style().text;
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 7 - row * 2; ++column) {
            const int x = 109 + row + column, y = 19 + row;
            const std::size_t offset = pixel_offset + (static_cast<std::size_t>(y) * 120 + x) * 4;
            require(offset + 3 < bytes.size());
            require(bytes[offset] == color.blue && bytes[offset + 1] == color.green && bytes[offset + 2] == color.red);
        }
    }
}
void clipboard_files() {
    const std::shared_ptr<Panel> root = make_control<Panel>(StableId("clipboard-root"));
    Window model(root, {1, 1});
    const HWND owner = CreateWindowExW(0, L"STATIC", L"Clipboard file fixture", 0,
                                        0, 0, 1, 1, HWND_MESSAGE, nullptr, nullptr, nullptr);
    require(owner != nullptr);
    const wchar_t path[] = L"C:\\Pictures\\Rainstar copied image Ω.png\0";
    const SIZE_T size = sizeof(DROPFILES) + sizeof(path);
    HGLOBAL allocation = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, size);
    require(allocation != nullptr);
    void* memory = GlobalLock(allocation);
    require(memory != nullptr);
    DROPFILES* header = static_cast<DROPFILES*>(memory);
    (*header).pFiles = sizeof(DROPFILES);
    (*header).fWide = TRUE;
    std::memcpy(static_cast<std::byte*>(memory) + sizeof(DROPFILES), path, sizeof(path));
    GlobalUnlock(allocation);
    require(OpenClipboard(owner));
    require(EmptyClipboard());
    require(SetClipboardData(CF_HDROP, allocation) != nullptr);
    CloseClipboard();
    host::WindowsHostServices services;
    services.bind_owner(owner, model);
    const HostClipboardFilesResult files = services.read_clipboard_files();
    require(files.status.accepted());
    require(files.paths_utf8.size() == 1);
    require(files.paths_utf8.front() == "C:\\Pictures\\Rainstar copied image Ω.png");
    require(OpenClipboard(owner));
    require(EmptyClipboard());
    CloseClipboard();
    require(services.read_clipboard_files().paths_utf8.empty());
    require(DestroyWindow(owner));
}
#include "support/windows_shape_reference.hpp"

int main() {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    shape_scanline_equivalence();
    clipboard_files();
    disclosure_render();
    for (ImageSampling sampling : {ImageSampling::nearest, ImageSampling::linear})
        for (double zoom : {1.0 / 64, 0.1, 0.25, 1.0, 1.25, 3.1})
            for (double scale : {1.0, 1.5, 2.0})
                for (bool edited : {false, true}) compare(sampling, zoom, scale, edited);
    CoUninitialize();
}
