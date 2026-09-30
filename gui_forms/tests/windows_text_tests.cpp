// Exercise the actual Windows measurement and retained-button rendering path.
#include "gui_forms/basic_controls.hpp"
#include "gui_forms/window.hpp"
#include "../src/host/windows/application/windows_host.cpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>

using namespace gui_forms;

static void require_text(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}

class PrivateFonts final {
public:
    explicit PrivateFonts(const std::filesystem::path& directory) {
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directory)) {
            const std::filesystem::path path = entry.path();
            if (path.extension() != ".ttf" && path.extension() != ".otf") continue;
            const std::wstring name = path.wstring();
            require_text(AddFontResourceExW(name.c_str(), FR_PRIVATE, nullptr) > 0,
                         "Could not load fixture font");
            paths_.push_back(name);
        }
    }
    ~PrivateFonts() {
        for (const std::wstring& path : paths_) RemoveFontResourceExW(path.c_str(), FR_PRIVATE, nullptr);
    }
private:
    std::vector<std::wstring> paths_;
};

static void check_caption(const std::string& text, double scale, std::uint16_t weight) {
    host::DibPainter painter;
    require_text(painter.resize({240, 32}, scale), "DIB allocation failed");
    painter.set_bundled_fonts_ready(true);
    const FontSpec font{FontRole::control, 14, weight, false, 0.05};
    const ResolvedTextLayout layout = painter.resolve_text_layout_utf8(text, font);
    std::cout << text << " scale=" << scale << " weight=" << weight
              << " width=" << layout.logical_size.width << " status=" << int(layout.status) << std::endl;
    require_text(layout.status == TextResolutionStatus::exact, "Fallback-only caption rejected");
    require_text(layout.logical_size.width > 0 && layout.logical_size.height > 0,
                 "Caption has empty layout");
    require_text(!layout.primary_family.empty() && !layout.runs.empty(), "Font provenance missing");
    const Size direct = painter.measure_text_utf8(text, font);
    require_text(std::abs(layout.logical_size.width - direct.width) < 0.01,
                 "Retained and direct widths differ");
    const std::shared_ptr<Button> button = make_control<Button>(StableId("caption"), text);
    (*button).set_font(font);
    (*button).set_use_mnemonic(false);
    BasicControlStyle style = (*button).style();
    style.text = Color::rgba(255, 0, 255);
    (*button).set_style(style);
    Window window(button, {240, 32});
    window.set_text_metrics_provider(&painter);
    window.perform_layout();
    painter.begin_frame();
    require_text(window.paint(painter).has_value(), "Retained caption did not paint");
    require_text(painter.save_bmp(L"windows-text-test.bmp"), "Capture failed");
    if (text == "Главная" && scale == 1.5 && weight == 700) {
        require_text(painter.save_bmp(L"windows-russian-tab.bmp"), "Russian evidence capture failed");
    }
    std::ifstream stream("windows-text-test.bmp", std::ios::binary);
    const std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(stream)), {});
    stream.close();
    host::DibPainter fresh;
    require_text(fresh.resize({240, 32}, scale), "Fresh DIB allocation failed");
    fresh.set_bundled_fonts_ready(true);
    fresh.begin_frame();
    (*button).invalidate(Dirty::paint);
    require_text(window.paint(fresh).has_value(), "Cold caption did not paint");
    GdiFlush();
    require_text(fresh.save_bmp(L"windows-text-cold.bmp"), "Cold capture failed");
    std::ifstream cold_stream("windows-text-cold.bmp", std::ios::binary);
    const std::vector<unsigned char> cold_bytes((std::istreambuf_iterator<char>(cold_stream)), {});
    cold_stream.close();
    require_text(bytes == cold_bytes, "Cached and cold text pixels differ");
    std::filesystem::remove("windows-text-cold.bmp");
    require_text(bytes.size() > 54, "Empty capture");
    const std::size_t start = bytes[10] | (bytes[11] << 8) | (bytes[12] << 16) | (bytes[13] << 24);
    std::size_t ink{};
    for (std::size_t i = start; i + 3 < bytes.size(); i += 4) {
        if (bytes[i] > 170 && bytes[i + 2] > 170 && bytes[i + 1] < 100) ++ink;
    }
    require_text(ink >= 8, "Caption has no visible text pixels");
    stream.close();
    std::filesystem::remove("windows-text-test.bmp");
}

static void check_cache_lifetime() {
    const DWORD before = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
    {
        host::DibPainter painter;
        require_text(painter.resize({320, 80}, 1.0), "Cache test DIB allocation failed");
        painter.set_bundled_fonts_ready(true);
        const FontSpec font{FontRole::content, 13, 400, false, 0.0};
        const ResolvedTextLayout original = painter.resolve_text_layout_utf8("File Manager", font);
        for (int index = 0; index < 520; ++index) {
            require_text(painter.resolve_text_layout_utf8("Folder " + std::to_string(index), font).status ==
                         TextResolutionStatus::exact, "Text cache rollover lost coverage");
        }
        require_text(painter.resolve_text_layout_utf8("File Manager", font).logical_size.width ==
                     original.logical_size.width, "Text cache rollover changed width");
        for (int size = 8; size < 80; ++size) {
            FontSpec varied = font;
            varied.size = size;
            require_text(painter.resolve_text_layout_utf8("Files", varied).status ==
                         TextResolutionStatus::exact, "Bounded font cache fallback failed");
        }
        for (const double scale : {1.5, 2.0, 1.0}) {
            painter.resize({320, 80}, scale);
            host::DibPainter fresh;
            fresh.resize({320, 80}, scale);
            fresh.set_bundled_fonts_ready(true);
            const FontSpec varied{FontRole::control, 14, 700, false, 0.5};
            const ResolvedTextLayout reused = painter.resolve_text_layout_utf8("Файл 日本語", varied);
            const ResolvedTextLayout expected = fresh.resolve_text_layout_utf8("Файл 日本語", varied);
            require_text(reused.status == expected.status && reused.logical_size.width == expected.logical_size.width,
                         "DPI change reused stale text metrics");
        }
    }
    GdiFlush();
    require_text(GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS) <= before,
                 "Painter leaked cached native fonts or bitmaps");
}

static void benchmark_captions() {
    host::DibPainter painter;
    require_text(painter.resize({1340, 850}, 1.0), "DIB allocation failed");
    painter.set_bundled_fonts_ready(true);
    const FontSpec font{FontRole::content, 13, 400, false, 0.0};
    for (int pass = 0; pass < 4; ++pass) {
        const std::chrono::steady_clock::time_point started = std::chrono::steady_clock::now();
        for (int frame = 0; frame < 5; ++frame) {
            painter.begin_frame();
            for (int index = 0; index < 100; ++index) {
                const std::string caption = "File Manager folder " + std::to_string(index);
                const ResolvedTextLayout layout = painter.resolve_text_layout_utf8(caption, font);
                require_text(layout.status == TextResolutionStatus::exact, "Benchmark text unavailable");
                painter.draw_text_utf8({double(index % 5 * 260), double(index / 5 * 35 + 20)},
                                      caption, font, Color::rgba(20, 30, 40));
            }
        }
        GdiFlush();
        const double milliseconds = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - started).count();
        std::cout << "caption_benchmark pass=" << pass << " labels=100 frames=5 ms_per_frame="
                  << milliseconds / 5.0 << std::endl;
    }
}

int main(int argc, char** argv) {
    try {
        require_text(argc == 2 || argc == 3, "Pass fixture font directory");
        const PrivateFonts fonts{std::filesystem::path(argv[1])};
        if (argc == 3 && std::string_view(argv[2]) == "--benchmark") {
            benchmark_captions();
            return 0;
        }
        check_cache_lifetime();
        for (const double scale : {1.0, 1.5, 2.0}) {
            for (const std::uint16_t weight : {400, 700}) {
                for (const std::string& text : std::vector<std::string>{
                         "Главная", "File", "Файл 123", "Українська", "Вид", "Материалы", "Инструменты", "Вставить",
                         "Основной", "日本語", "العربية", "עברית", "हिन्दी", "বাংলা", "ਪੰਜਾਬੀ"}) {
                    check_caption(text, scale, weight);
                }
            }
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }
}
