#include "gui_forms/live_surface.hpp"
#include "skia_raster.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string_view>

namespace {
using namespace gui_forms;
using Clock = std::chrono::steady_clock;

constexpr std::size_t batch_count = 9U;
constexpr int draws_per_batch = 300;
constexpr Rect destination{0.0, 0.0, 1060.0, 618.0};

void require(const bool condition) {
    if (condition) return;
    std::cerr << "live-surface benchmark setup/draw failed\n";
    std::exit(1);
}

[[nodiscard]] std::shared_ptr<LiveSurface> make_surface(const LiveSurfacePixelFormat format) {
    const std::shared_ptr<LiveSurface> surface = LiveSurface::create(
        {.width = 1060U, .height = 618U, .pixel_format = format, .opaque = true});
    require(static_cast<bool>(surface));
    LiveSurfaceWriteLease write = (*surface).try_acquire_write();
    require(static_cast<bool>(write));
    const std::span<std::byte> pixels = write.pixels();
    for (std::size_t offset = 0U; offset < pixels.size(); offset += 4U) {
        pixels[offset] = std::byte{37};
        pixels[offset + 1U] = std::byte{103};
        pixels[offset + 2U] = std::byte{211};
        pixels[offset + 3U] = std::byte{255};
    }
    require(write.publish() != 0U);
    return surface;
}

[[nodiscard]] double measure(render::SkiaRaster& raster, const LiveSurfaceFrame& frame) {
    const Clock::time_point start = Clock::now();
    for (int index = 0; index < draws_per_batch; ++index) {
        require(raster.draw_live_surface_frame(frame, destination, 1.0));
    }
    const Clock::time_point end = Clock::now();
    const std::chrono::duration<double, std::milli> elapsed = end - start;
    const double per_draw = elapsed.count() / draws_per_batch;
    return per_draw;
}

void report(const char* label, std::array<double, batch_count> samples) {
    std::sort(samples.begin(), samples.end());
    std::cout << label << " ms/draw min=" << samples.front()
              << " median=" << samples[batch_count / 2U] << " max=" << samples.back() << '\n';
}
} // namespace

int main(const int argc, const char* const* argv) {
    const bool bgra_only = argc > 1 && std::string_view(argv[1]) == "--bgra-only";
    render::SkiaRaster raster{};
    require(raster.resize({1060.0, 618.0}, 1.0));
    const std::shared_ptr<LiveSurface> bgra = make_surface(LiveSurfacePixelFormat::bgra32_premultiplied_srgb);
    const std::shared_ptr<LiveSurface> rgba = make_surface(LiveSurfacePixelFormat::rgba32_premultiplied_srgb);
    const LiveSurfaceFrame bgra_frame = (*bgra).acquire_latest();
    const LiveSurfaceFrame rgba_frame = (*rgba).acquire_latest();
    DamageRegion damage{};
    damage.add(destination);
    raster.begin_frame(damage);
    static_cast<void>(measure(raster, bgra_frame));
    if (!bgra_only) static_cast<void>(measure(raster, rgba_frame));
    std::array<double, batch_count> bgra_samples{};
    std::array<double, batch_count> rgba_samples{};
    for (std::size_t batch = 0U; batch < batch_count; ++batch) {
        // Alternate order to reduce a systematic warmup/thermal bias.
        if (!bgra_only && batch % 2U != 0U) rgba_samples[batch] = measure(raster, rgba_frame);
        bgra_samples[batch] = measure(raster, bgra_frame);
        if (!bgra_only && batch % 2U == 0U) rgba_samples[batch] = measure(raster, rgba_frame);
    }
    raster.end_frame();
    std::cout << std::fixed << std::setprecision(6)
              << "opaque 1060x618, scale 1, full rectangular clip; " << batch_count
              << " batches of " << draws_per_batch << " synchronous lease draws\n";
    report("BGRA", bgra_samples);
    if (!bgra_only) report("RGBA", rgba_samples);
    return 0;
}
