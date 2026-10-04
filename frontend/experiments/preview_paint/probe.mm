#import <AppKit/AppKit.h>

#include "gui_forms/gui_forms.hpp"
#include "gui_forms/platform/macos_host.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {
constexpr std::uint32_t image_width = 1024U;
constexpr std::size_t sample_count = 30U;
using Samples = std::array<double, sample_count>;
using Clock = std::chrono::steady_clock;

enum class Format : std::size_t { raw, stored, deflated };
constexpr std::array<const char*, 3> format_names{"BGRA", "PNG-store", "PNG-deflate"};
constexpr std::array<std::array<Format, 3>, 6> orders{{
    {Format::raw, Format::stored, Format::deflated},
    {Format::raw, Format::deflated, Format::stored},
    {Format::stored, Format::raw, Format::deflated},
    {Format::stored, Format::deflated, Format::raw},
    {Format::deflated, Format::raw, Format::stored},
    {Format::deflated, Format::stored, Format::raw}}};

struct Sources final {
    std::uint32_t height{0U};
    std::array<std::vector<std::byte>, 3> bytes{};
};

struct Durations final {
    double admission{0.0};
    double bind_and_paint{0.0};
    double total{0.0};
    double cached_paint{0.0};
    double retirement{0.0};
};

struct Series final {
    Samples admission{};
    Samples bind_and_paint{};
    Samples total{};
    Samples cached_paint{};
    Samples retirement{};
};

struct State final {
    // run_macos owns the model. The callback checks closed before this borrow.
    gui_forms::Window* model{nullptr};
    std::shared_ptr<gui_forms::PictureBox> picture{};
    std::array<Sources, 3> sources{};
    std::function<void()> close{};
    bool closed{false};
    bool complete{false};
    bool reverse{false};
    std::string failure{};
};

void require(const bool condition, const char* const message) {
    if (!condition) throw std::runtime_error(message);
}

[[nodiscard]] double milliseconds(const Clock::time_point begin, const Clock::time_point end) {
    const std::chrono::duration<double, std::milli> elapsed = end - begin;
    const double value = elapsed.count();
    return value;
}

[[nodiscard]] std::vector<std::byte> read_png(const std::filesystem::path& path,
                                           const std::uint32_t height) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    require(static_cast<bool>(input), "cannot open generated PNG");
    const std::streamsize extent = input.tellg();
    require(extent > 0 && extent <= 16 * 1024 * 1024, "PNG extent refused");
    std::vector<std::byte> bytes(static_cast<std::size_t>(extent));
    input.seekg(0);
    input.read(reinterpret_cast<char*>(bytes.data()), extent);
    require(static_cast<bool>(input), "cannot read generated PNG");
    const gui_forms::PngValidationResult valid = gui_forms::validate_png(bytes);
    require(valid && valid.metadata.width == image_width && valid.metadata.height == height,
            "PNG dimensions differ from the declared workload");
    return bytes;
}

[[nodiscard]] Sources prepare_sources(const std::filesystem::path& root, const std::uint32_t height) {
    Sources sources{};
    sources.height = height;
    std::vector<std::byte>& raw = sources.bytes[0];
    raw.resize(static_cast<std::size_t>(image_width) * height * 4U);
    for (std::size_t offset = 0U; offset < raw.size(); offset += 4U) {
        raw[offset] = std::byte{198};
        raw[offset + 1U] = std::byte{226};
        raw[offset + 2U] = std::byte{12};
        raw[offset + 3U] = std::byte{255};
    }
    const std::string suffix = std::to_string(height) + ".png";
    sources.bytes[1] = read_png(root / ("rgba-" + suffix), height);
    sources.bytes[2] = read_png(root / ("deflate-" + suffix), height);
    return sources;
}

[[nodiscard]] NSWindow* find_window() {
    for (NSWindow* window in [NSApp windows]) {
        if ([window.title isEqualToString:@"File Manager preview paint measurement"]) return window;
    }
    throw std::runtime_error("measurement window unavailable");
}

void force_display(NSView* const view) {
    [view setNeedsDisplay:YES];
    [view displayIfNeeded];
}

void check_paint(const gui_forms::MetricsSnapshot& before, const gui_forms::Window& model) {
    const gui_forms::MetricsSnapshot after = model.metrics_snapshot();
    require(after.paint_passes == before.paint_passes + 1U &&
            after.frames_presented == before.frames_presented + 1U,
            "forced draw did not produce exactly one retained paint and host presentation");
}

// Snapshot and color checks are outside timers. They may trigger another draw;
// they certify this fixture's sampled pixels, not physical screen scan-out.
void check_pixels(const State& state, NSView* const view, const bool image_expected) {
    NSBitmapImageRep* const compatible = [view bitmapImageRepForCachingDisplayInRect:view.bounds];
    require(compatible != nil, "native bitmap unavailable");
    // Choose the destination profile before drawing. Retagging captured pixels
    // afterward would only relabel device values and could hide a color error.
    NSColorSpace* const color_space = NSColorSpace.sRGBColorSpace;
    NSBitmapImageRep* const bitmap = [compatible bitmapImageRepByRetaggingWithColorSpace:color_space];
    require(bitmap != nil, "sRGB snapshot destination unavailable");
    require([bitmap.colorSpace isEqual:color_space] == YES, "snapshot destination is not sRGB");
    [view cacheDisplayInRect:view.bounds toBitmapImageRep:bitmap];
    require(bitmap.pixelsWide > 8 && bitmap.pixelsHigh > 8, "native bitmap too small");
    require([bitmap.colorSpace isEqual:color_space] == YES, "captured bitmap is not sRGB");
    require(bitmap.bitsPerSample == 8 && bitmap.samplesPerPixel == 4 &&
            bitmap.bitsPerPixel == 32 && bitmap.hasAlpha == YES && bitmap.isPlanar == NO,
            "snapshot requires packed four-channel eight-bit samples");
    const NSBitmapFormat refused_formats = NSBitmapFormatAlphaFirst | NSBitmapFormatFloatingPointSamples;
    require((bitmap.bitmapFormat & refused_formats) == 0, "snapshot requires integer RGBA sample order");
    for (NSInteger vertical = 1; vertical <= 3; ++vertical) {
        for (NSInteger horizontal = 1; horizontal <= 3; ++horizontal) {
            const NSInteger x = bitmap.pixelsWide / 4 * horizontal;
            const NSInteger y = bitmap.pixelsHigh / 4 * vertical;
            std::array<NSUInteger, 4> components{};
            [bitmap getPixel:components.data() atX:x y:y];
            const double red = static_cast<double>(components[0U]) / 255.0;
            const double green = static_cast<double>(components[1U]) / 255.0;
            const double blue = static_cast<double>(components[2U]) / 255.0;
            const double alpha = static_cast<double>(components[3U]) / 255.0;
            const bool match = std::abs(red - 12.0 / 255.0) < 0.02 &&
                std::abs(green - 226.0 / 255.0) < 0.02 &&
                std::abs(blue - 198.0 / 255.0) < 0.02 && alpha > 0.98;
            if (horizontal == 1 && vertical == 1) {
                // Diagnose NSColor's intermediate profile without using its
                // conversion as the oracle for an explicitly tagged bitmap.
                NSColor* const sample = [bitmap colorAtX:x y:y];
                NSColor* const color = [sample colorUsingColorSpace:color_space];
                NSColorSpace* const sample_color_space = sample.colorSpace;
                NSString* const sample_color_space_name = sample_color_space.localizedName;
                const char* const sample_space = [sample_color_space_name UTF8String];
                std::cout << "pixel-control|image_expected=" << image_expected
                          << "|raw_rgba=" << components[0U] << ',' << components[1U] << ','
                          << components[2U] << ',' << components[3U]
                          << "|sample_space=" << (sample_space == nullptr ? "unavailable" : sample_space);
                if (color != nil) {
                    std::cout << "|converted_rgba=" << color.redComponent << ',' << color.greenComponent
                              << ',' << color.blueComponent << ',' << color.alphaComponent;
                }
                std::cout << '\n';
            }
            if (match != image_expected) {
                const gui_forms::Rect arranged = (*state.picture).arranged_bounds();
                const gui_forms::Rect image = (*state.picture).image_bounds();
                NSString* const filename = state.reverse
                    ? @"preview-paint-reverse-failure.png" : @"preview-paint-forward-failure.png";
                NSData* const encoded = [bitmap representationUsingType:NSBitmapImageFileTypePNG properties:@{}];
                const BOOL saved = encoded != nil && [encoded writeToFile:filename atomically:YES];
                std::cerr << "pixel-mismatch|image_expected=" << image_expected
                          << "|sample=" << horizontal << ',' << vertical
                           << "|rgba=" << red << ',' << green << ',' << blue << ',' << alpha
                          << "|bitmap=" << bitmap.pixelsWide << 'x' << bitmap.pixelsHigh
                          << "|arranged=" << arranged.x << ',' << arranged.y << ','
                          << arranged.width << ',' << arranged.height
                          << "|image_bounds=" << image.x << ',' << image.y << ','
                          << image.width << ',' << image.height
                          << "|snapshot_saved=" << (saved == YES) << '\n';
                throw std::runtime_error("painted pixels disagree with the fixture or retirement");
            }
        }
    }
}

[[nodiscard]] Durations measure(State& state, NSView* const view, const Sources& sources,
                               const Format format, const bool validate_pixels) {
    gui_forms::Window& model = *state.model;
    gui_forms::PictureBox& picture = *state.picture;
    const std::span<const std::byte> bytes = sources.bytes[static_cast<std::size_t>(format)];
    const gui_forms::MetricsSnapshot before = model.metrics_snapshot();
    const Clock::time_point started = Clock::now();
    const gui_forms::ImageLoadResult image = format == Format::raw
        ? model.load_bgra32_premultiplied(image_width, sources.height, image_width * 4U, bytes)
        : model.load_png(bytes);
    const Clock::time_point admitted = Clock::now();
    require(static_cast<bool>(image), "image admission failed");
    picture.set_image(image.image);
    force_display(view);
    const Clock::time_point painted = Clock::now();
    check_paint(before, model);
    const gui_forms::ImageRegistrySnapshot active = model.image_resource_snapshot();
    require(active.resource_count == 1U, "unexpected active registry count");
    if (validate_pixels) check_pixels(state, view, true);

    const gui_forms::MetricsSnapshot cached_before = model.metrics_snapshot();
    const Clock::time_point cache_started = Clock::now();
    force_display(view);
    const Clock::time_point cache_ended = Clock::now();
    check_paint(cached_before, model);

    const gui_forms::MetricsSnapshot retire_before = model.metrics_snapshot();
    const Clock::time_point retire_started = Clock::now();
    picture.clear_image();
    const bool removed = model.remove_image(image.image);
    force_display(view);
    const Clock::time_point retire_ended = Clock::now();
    require(removed, "image retirement failed");
    check_paint(retire_before, model);
    const gui_forms::ImageRegistrySnapshot retired = model.image_resource_snapshot();
    require(retired.resource_count == 0U && retired.encoded_bytes == 0U && retired.decoded_bytes == 0U,
            "registry accounting did not return to zero");
    require(!model.image_resources().find(image.image), "retired image identity remains valid");
    if (validate_pixels) check_pixels(state, view, false);
    const Durations durations{
        .admission = milliseconds(started, admitted),
        .bind_and_paint = milliseconds(admitted, painted),
        .total = milliseconds(started, painted),
        .cached_paint = milliseconds(cache_started, cache_ended),
        .retirement = milliseconds(retire_started, retire_ended)};
    return durations;
}

void report(const char* const phase, const char* const format, const std::uint32_t height,
            const std::size_t input_bytes, const Samples& samples) {
    Samples ordered = samples;
    std::sort(ordered.begin(), ordered.end());
    constexpr std::size_t p50 = (sample_count * 50U + 99U) / 100U - 1U;
    constexpr std::size_t p95 = (sample_count * 95U + 99U) / 100U - 1U;
    std::cout << "paint|phase=" << phase << "|format=" << format << "|height=" << height
              << "|input_bytes=" << input_bytes << "|p50_ms=" << ordered[p50]
              << "|p95_ms=" << ordered[p95] << "|max_ms=" << ordered.back() << "|samples_ms=";
    for (const double sample : samples) std::cout << sample << ',';
    std::cout << '\n';
}

void run_case(State& state, NSView* const view, const Sources& sources) {
    // Complete warmup, including native pixel and blank-retirement controls.
    for (const Format format : orders[0]) {
        @autoreleasepool {
            std::cout << "warmup|height=" << sources.height << "|format="
                      << format_names[static_cast<std::size_t>(format)] << '\n';
            (void)measure(state, view, sources, format, true);
        }
    }
    std::array<Series, 3> series{};
    for (std::size_t sample = 0U; sample < sample_count; ++sample) {
        for (const Format format : orders[sample % orders.size()]) {
            // Drain native temporary objects after this sample's five timed
            // spans, so they do not accumulate across the complete batch.
            @autoreleasepool {
                const Durations durations = measure(state, view, sources, format, false);
                Series& current = series[static_cast<std::size_t>(format)];
                current.admission[sample] = durations.admission;
                current.bind_and_paint[sample] = durations.bind_and_paint;
                current.total[sample] = durations.total;
                current.cached_paint[sample] = durations.cached_paint;
                current.retirement[sample] = durations.retirement;
            }
        }
    }
    for (std::size_t index = 0U; index < series.size(); ++index) {
        const Series& current = series[index];
        const char* const format = format_names[index];
        const std::size_t bytes = sources.bytes[index].size();
        report("admission", format, sources.height, bytes, current.admission);
        report("bind-and-native-paint", format, sources.height, bytes, current.bind_and_paint);
        report("admission-through-native-paint", format, sources.height, bytes, current.total);
        report("cached-native-paint", format, sources.height, bytes, current.cached_paint);
        report("clear-remove-native-paint", format, sources.height, bytes, current.retirement);
    }
    std::cout << "controls|height=" << sources.height
              << "|nine_pixels_per_format=passed|cleared_pixels=passed|registry_retirement=passed\n";
}

void run_measurement(void* const context) {
    const std::unique_ptr<std::shared_ptr<State>> owner(static_cast<std::shared_ptr<State>*>(context));
    State& state = **owner;
    if (state.closed || state.model == nullptr) return;
    @autoreleasepool {
        try {
            NSWindow* const window = find_window();
            NSView* const view = window.contentView;
            require(window.isVisible && view != nil && !(*state.model).occluded(),
                    "native measurement window is not visible");
            require(std::abs(view.bounds.size.width - 640.0) < 0.01 &&
                    std::abs(view.bounds.size.height - 480.0) < 0.01,
                    "native client extent changed");
            force_display(view);
            const gui_forms::MetricsSnapshot metrics = (*state.model).metrics_snapshot();
            std::cout << std::fixed << std::setprecision(6)
                      << "workload|client=640x480|scale=" << window.backingScaleFactor
                      << "|renderer=" << metrics.renderer_name << "|samples=" << sample_count
                      << "|geometry_order=" << (state.reverse ? "reverse" : "forward") << '\n';
            for (std::size_t position = 0U; position < state.sources.size(); ++position) {
                const std::size_t index = state.reverse ? state.sources.size() - 1U - position : position;
                run_case(state, view, state.sources[index]);
            }
            state.complete = true;
        } catch (const std::exception& error) {
            state.failure = error.what();
        }
        state.close();
    }
}

void mark_closed(const std::shared_ptr<State>& state) { (*state).closed = true; }

void host_ready(const std::shared_ptr<State>& state, std::function<void()>,
    std::function<void()> close,
    std::function<gui_forms::HostDialogResult(const gui_forms::HostDialogRequest&)>,
    std::function<gui_forms::HostServiceStatus(const gui_forms::HostTooltipRequest&)>,
    std::function<void()>, std::function<gui_forms::HostClipboardTextResult()>,
    std::function<gui_forms::HostServiceStatus(std::string_view)>) {
    (*state).close = std::move(close);
    // One named continuation retains only State. It refuses the model borrow
    // after close; the host callbacks and main owner survive run_macos.
    std::unique_ptr<std::shared_ptr<State>> retained = std::make_unique<std::shared_ptr<State>>(state);
    const dispatch_time_t deadline = dispatch_time(DISPATCH_TIME_NOW, 250 * NSEC_PER_MSEC);
    dispatch_after_f(deadline, dispatch_get_main_queue(), retained.release(), run_measurement);
}
} // namespace

int main(const int count, char** const arguments) {
    @autoreleasepool {
        try {
            require(count == 2 || count == 3, "expected fixtures directory and optional --reverse");
            const bool reverse = count == 3;
            require(!reverse || std::string_view(arguments[2]) == "--reverse", "unknown option");
            const std::filesystem::path root(arguments[1]);
            const std::shared_ptr<State> state = std::make_shared<State>();
            (*state).reverse = reverse;
            (*state).sources = {prepare_sources(root, 64U), prepare_sources(root, 256U),
                                prepare_sources(root, 1024U)};
            const gui_forms::StableId root_id("preview.paint.picture");
            (*state).picture = std::make_shared<gui_forms::PictureBox>(root_id);
            (*(*state).picture).set_size_mode(gui_forms::PictureBoxSizeMode::stretch_image);
            std::unique_ptr<gui_forms::Window> model =
                std::make_unique<gui_forms::Window>((*state).picture, gui_forms::Size{640.0, 480.0});
            (*state).model = model.get();
            gui_forms::host::MacHostOptions options{};
            options.title = "File Manager preview paint measurement";
            options.initial_size = {640.0, 480.0};
            options.print_metrics_on_close = false;
            options.host_ready = std::bind_front(host_ready, state);
            options.closed = std::bind_front(mark_closed, state);
            const int result = gui_forms::host::run_macos(std::move(model), std::move(options));
            (*state).model = nullptr;
            (*state).closed = true;
            if (result != 0 || !(*state).complete || !(*state).failure.empty()) {
                std::cerr << "preview paint measurement refused: " << (*state).failure << '\n';
                return 1;
            }
            return 0;
        } catch (const std::exception& error) {
            std::cerr << error.what() << '\n';
            return 1;
        }
    }
}
