#include "gui_forms/gui_forms.hpp"
#include "skia_raster.hpp"

#include <array>
#include <cstdlib>
#include <iostream>

namespace gui_forms::render {
struct SkiaLiveSurfaceTestAccess final {
    static void reject_next_image(SkiaRaster& raster) {
        raster.fail_next_live_image_ = true;
    }
};
} // namespace gui_forms::render

namespace {
using namespace gui_forms;

void require(const bool condition, const char* message) {
    if (condition) return;
    std::cerr << message << '\n';
    std::exit(1);
}

[[nodiscard]] std::shared_ptr<LiveSurface> make_surface(const bool opaque, const std::byte alpha) {
    std::shared_ptr<LiveSurface> surface = LiveSurface::create(
        {.width = 8U, .height = 8U, .opaque = opaque});
    require(static_cast<bool>(surface), "surface created");
    LiveSurfaceWriteLease write = (*surface).try_acquire_write();
    require(static_cast<bool>(write), "write lease acquired");
    const std::span<std::byte> pixels = write.pixels();
    for (std::size_t index = 0; index < pixels.size(); index += 4U) {
        pixels[index] = alpha;
        pixels[index + 1U] = std::byte{0};
        pixels[index + 2U] = std::byte{0};
        pixels[index + 3U] = alpha;
    }
    require(write.publish() != 0U, "frame published");
    return surface;
}

[[nodiscard]] std::array<unsigned, 4> pixel(const render::SkiaRaster& raster, const std::size_t x, const std::size_t y) {
    const std::byte* const bytes = static_cast<const std::byte*>(raster.pixels());
    require(bytes != nullptr, "raster readable");
    const std::size_t offset = y * raster.row_bytes() + x * 4U;
    std::array<unsigned, 4> result{};
    for (std::size_t channel = 0; channel < result.size(); ++channel) {
        result[channel] = std::to_integer<unsigned>(bytes[offset + channel]);
    }
    return result;
}

void blend_cases() {
    render::SkiaRaster raster{};
    require(raster.resize({8.0, 8.0}, 1.0), "raster allocated");
    DamageRegion damage{};
    damage.add({0.0, 0.0, 8.0, 8.0});
    const std::shared_ptr<LiveSurface> opaque = make_surface(true, std::byte{255});
    const std::shared_ptr<LiveSurface> translucent = make_surface(false, std::byte{128});
    raster.begin_frame(damage);
    raster.fill_rect({0.0, 0.0, 8.0, 8.0}, Color::rgba(255, 0, 0));
    raster.draw_live_surface(opaque, {0.0, 0.0, 8.0, 8.0}, 1.0);
    raster.end_frame();
    const std::array<unsigned, 4> blue{0U, 0U, 255U, 255U};
    for (std::size_t y = 0; y < 8U; ++y) {
        for (std::size_t x = 0; x < 8U; ++x) require(pixel(raster, x, y) == blue, "opaque pixels exactly replace red");
    }
    raster.begin_frame(damage);
    raster.fill_rect({0.0, 0.0, 8.0, 8.0}, Color::rgba(255, 0, 0));
    raster.draw_live_surface(translucent, {0.0, 0.0, 8.0, 8.0}, 1.0);
    raster.end_frame();
    const std::array<unsigned, 4> mixed = pixel(raster, 4U, 4U);
    require(mixed[0] == 127U && mixed[2] == 128U && mixed[3] == 255U, "nonopaque surface still source-over blends");
    raster.begin_frame(damage);
    raster.fill_rect({0.0, 0.0, 8.0, 8.0}, Color::rgba(255, 0, 0));
    raster.draw_live_surface(opaque, {0.0, 0.0, 8.0, 8.0}, 0.5);
    raster.end_frame();
    const std::array<unsigned, 4> faded = pixel(raster, 4U, 4U);
    require(faded[0] >= 127U && faded[0] <= 128U && faded[2] >= 127U && faded[2] <= 128U, "opacity below one still blends opaque producer");
}

void image_failure() {
    render::SkiaRaster raster{};
    const std::shared_ptr<LiveSurface> surface = make_surface(true, std::byte{255});
    const LiveSurfaceFrame frame = (*surface).acquire_latest();
    DamageRegion damage{};
    damage.add({0.0, 0.0, 8.0, 8.0});
#if defined(GUI_FORMS_PREPARED_TEXT)
    require(raster.begin_prepared_frame({8.0, 8.0}, 1.0, damage) == PreparedTextStatus::success, "initial prepared frame admitted");
#else
    require(raster.resize({8.0, 8.0}, 1.0), "failure raster allocated");
    raster.begin_frame(damage);
#endif
    raster.fill_rect({0.0, 0.0, 8.0, 8.0}, Color::rgba(255, 0, 0));
    raster.end_frame();
#if defined(GUI_FORMS_PREPARED_TEXT)
    const PaintReceipt previous{1U, 1U};
    require(raster.commit_prepared_frame(previous) == PreparedTextStatus::success, "previous complete frame published");
    require(raster.begin_prepared_frame({8.0, 8.0}, 1.0, damage) == PreparedTextStatus::success, "full live candidate admitted");
    require(raster.draw_live_surface_frame(frame, {0.0, 0.0, 4.0, 8.0}, 1.0), "first live fragment succeeds");
#else
    raster.begin_frame(damage);
#endif
    render::SkiaLiveSurfaceTestAccess::reject_next_image(raster);
    const bool drawn = raster.draw_live_surface_frame(frame, {0.0, 0.0, 8.0, 8.0}, 1.0);
    require(!drawn, "image failure reported to host");
    raster.end_frame();
#if defined(GUI_FORMS_PREPARED_TEXT)
    raster.abort_prepared_frame();
    require(raster.prepared_front_receipt() == previous, "failed live composition retains previous receipt");
#endif
    const std::array<unsigned, 4> red{255U, 0U, 0U, 255U};
    require(pixel(raster, 1U, 1U) == red && pixel(raster, 7U, 7U) == red, "failure does not publish cleared or partial pixels");
}

class SolidControl final : public Control {
public:
    SolidControl(StableId id, const Color color) : Control(std::move(id)), color_(color) {}
    void on_paint(Painter& painter, Rect) override {
        painter.fill_rect(client_rectangle(), color_);
    }
private:
    Color color_{};
};

void retained_overlay() {
    const std::shared_ptr<SolidControl> root = make_control<SolidControl>(StableId("root"), Color::rgba(255, 0, 0));
    const std::shared_ptr<Control> live = make_control<Control>(StableId("live"));
    const std::shared_ptr<SolidControl> overlay = make_control<SolidControl>(StableId("overlay"), Color::rgba(0, 255, 0));
    (*live).set_requested_bounds({0.0, 0.0, 8.0, 8.0});
    (*overlay).set_requested_bounds({2.0, 2.0, 4.0, 4.0});
    (*overlay).set_paint_plane(PaintPlane::overlay);
    (*root).add_child(live);
    (*root).add_child(overlay);
    Window window(root, {8.0, 8.0});
    window.perform_layout();
    const std::shared_ptr<LiveSurface> surface = make_surface(true, std::byte{255});
    require(window.queue_live_surface_presentation(live, surface), "live presentation registered");
    render::SkiaRaster raster{};
    require(raster.resize({8.0, 8.0}, 1.0), "overlay raster allocated");
    DamageRegion damage{};
    damage.add({0.0, 0.0, 8.0, 8.0});
    raster.begin_frame(damage);
    require(window.paint(raster).has_value(), "retained overlay painted");
    const std::vector<LiveSurfacePresentation> updates = window.take_live_surface_presentations();
    require(updates.size() == 4U, "overlay subtracted from live clips");
    for (const LiveSurfacePresentation& update : updates) {
        raster.save();
        raster.clip_rect(update.clip);
        raster.draw_live_surface(update.surface, update.destination, 1.0);
        raster.restore();
    }
    raster.end_frame();
    const std::array<unsigned, 4> green{0U, 255U, 0U, 255U};
    const std::array<unsigned, 4> blue{0U, 0U, 255U, 255U};
    for (std::size_t y = 0; y < 8U; ++y) {
        for (std::size_t x = 0; x < 8U; ++x) {
            const bool covered = x >= 2U && x < 6U && y >= 2U && y < 6U;
            require(pixel(raster, x, y) == (covered ? green : blue), "overlay pixels remain retained; uncovered pixels advance");
        }
    }
}
} // namespace

int main() {
    blend_cases();
    image_failure();
    retained_overlay();
    return 0;
}
