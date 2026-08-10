#include "gui_forms/gui_forms.hpp"

#include <cassert>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace {

void fill(gui_forms::LiveSurfaceWriteLease& lease, std::byte value) {
    for (std::byte& pixel : lease.pixels()) pixel = value;
}

class ObservePresentationWake final {
public:
    ObservePresentationWake(gui_forms::LiveSurface& surface,
                            std::atomic<std::uint64_t>& wakes)
        : surface_(surface), wakes_(wakes) {}

    void operator()() const {
        // Publication must not hold the surface mutex while signaling. A
        // scheduler is allowed to inspect the newest state from this thread.
        assert(surface_.snapshot().description.width != 0U);
        ++wakes_;
    }

private:
    gui_forms::LiveSurface& surface_;
    std::atomic<std::uint64_t>& wakes_;
};

} // namespace

int main() {
    using namespace gui_forms;

    assert(!LiveSurface::create({}));
    assert(!LiveSurface::create(
        {4U, 3U, LiveSurfacePixelFormat::bgra32_premultiplied_srgb, 1U}));
    assert(!LiveSurface::create(
        {4U, 3U, LiveSurfacePixelFormat::bgra32_premultiplied_srgb, 9U}));
    std::shared_ptr<LiveSurface> surface = LiveSurface::create({4U, 3U});
    assert(surface);
    assert((*surface).snapshot().description.buffer_count ==
           default_live_surface_buffer_count);
    assert(!(*surface).acquire_latest());

    std::atomic<std::uint64_t> wakes{};
    LiveSurfaceWakeConnection wake = (*surface).connect_presentation_wake(
        ObservePresentationWake(*surface, wakes));
    assert(wake.connected());

    LiveSurfaceWriteLease first_write = (*surface).try_acquire_write();
    assert(first_write);
    assert(first_write.width() == 4U);
    assert(first_write.height() == 3U);
    assert(first_write.row_bytes() == 16U);
    fill(first_write, std::byte{0x11});
    assert(first_write.publish({1.0, 1.0, 2.0, 1.0}) == 1U);
    assert(wakes == 1U);

    LiveSurfaceFrame first = (*surface).acquire_latest();
    assert(first);
    assert(first.generation() == 1U);
    assert(first.damage() == Rect(1.0, 1.0, 2.0, 1.0));
    assert(first.pixels().front() == std::byte{0x11});

    LiveSurfaceWriteLease second_write = (*surface).try_acquire_write(true);
    assert(second_write);
    assert(second_write.pixels().front() == std::byte{0x11});
    fill(second_write, std::byte{0x22});
    assert(second_write.publish() == 2U);
    assert(wakes == 2U);
    LiveSurfaceFrame second = (*surface).acquire_latest();
    assert(second && second.generation() == 2U);
    assert(second.pixels().front() == std::byte{0x22});
    // A compositor lease remains immutable across newer publications.
    assert(first.pixels().front() == std::byte{0x11});

    LiveSurfaceWriteLease third_write = (*surface).try_acquire_write();
    assert(third_write);
    fill(third_write, std::byte{0x33});
    assert(third_write.publish() == 3U);
    assert(wakes == 3U);
    LiveSurfaceFrame third = (*surface).acquire_latest();
    assert(third && third.pixels().front() == std::byte{0x33});
    // All three candidates are leased. A high-rate producer drops instead of
    // blocking or mutating a frame currently owned by the compositor.
    assert(!(*surface).try_acquire_write());
    assert((*surface).snapshot().dropped_acquires == 1U);

    first = {};
    LiveSurfaceWriteLease recovered = (*surface).try_acquire_write();
    assert(recovered);
    recovered.abandon();
    assert(!(*surface).snapshot().write_active);

    second = {};
    third = {};
    assert((*surface).reconfigure({2U, 2U}));
    assert(wakes == 4U);
    const LiveSurfaceSnapshot reconfigured = (*surface).snapshot();
    assert(reconfigured.epoch == 2U);
    assert(!reconfigured.has_frame);
    assert(reconfigured.description.width == 2U);
    assert(reconfigured.description.height == 2U);

    LiveSurfaceWriteLease final_write = (*surface).try_acquire_write();
    assert(final_write && final_write.pixels().size() == 16U);
    fill(final_write, std::byte{0x7f});
    assert(final_write.publish() == 1U);
    assert(wakes == 5U);
    LiveSurfaceFrame final_frame = (*surface).acquire_latest();
    assert(final_frame && final_frame.epoch() == 2U);
    assert(final_frame.pixels().back() == std::byte{0x7f});

    wake.disconnect();
    assert(!wake.connected());
    final_frame = {};
    LiveSurfaceWriteLease disconnected_write = (*surface).try_acquire_write();
    assert(disconnected_write);
    fill(disconnected_write, std::byte{0x55});
    assert(disconnected_write.publish() == 2U);
    assert(wakes == 5U);

    std::shared_ptr<LiveSurface> low_latency_surface = LiveSurface::create(
        {2U, 2U, LiveSurfacePixelFormat::bgra32_premultiplied_srgb, 2U});
    assert(low_latency_surface);
    LiveSurfaceWriteLease low_latency_write =
        (*low_latency_surface).try_acquire_write();
    assert(low_latency_write && low_latency_write.publish() == 1U);
    LiveSurfaceFrame low_latency_first =
        (*low_latency_surface).acquire_latest();
    low_latency_write = (*low_latency_surface).try_acquire_write();
    assert(low_latency_write && low_latency_write.publish() == 2U);
    LiveSurfaceFrame low_latency_second =
        (*low_latency_surface).acquire_latest();
    assert(low_latency_first && low_latency_second);
    assert(!(*low_latency_surface).try_acquire_write());

    std::shared_ptr<gui_forms::Control> root = make_control<Control>(StableId("live.root"));
    std::shared_ptr<gui_forms::Control> live_control = make_control<Control>(StableId("live.content"));
    std::shared_ptr<gui_forms::Control> overlay = make_control<Control>(StableId("live.overlay"));
    (*root).set_requested_bounds({0.0, 0.0, 100.0, 80.0});
    (*live_control).set_requested_bounds({10.0, 10.0, 70.0, 50.0});
    (*overlay).set_requested_bounds({20.0, 20.0, 30.0, 20.0});
    (*overlay).set_paint_plane(PaintPlane::overlay);
    (*root).add_child(live_control);
    (*root).add_child(overlay);
    Window window(root, {100.0, 80.0});
    window.perform_layout();

    std::shared_ptr<LiveSurface> composited_surface = LiveSurface::create({4U, 3U});
    assert(composited_surface);
    LiveSurfaceWriteLease composited_write =
        (*composited_surface).try_acquire_write();
    assert(composited_write);
    fill(composited_write, std::byte{0x31});
    assert(composited_write.publish() == 1U);
    assert(window.queue_live_surface_presentation(live_control,
                                                  composited_surface));
    // An intersecting retained overlay owns only its pixels. The direct lane
    // continues presenting every uncovered fragment around it.
    const std::vector<LiveSurfacePresentation> clipped = window.take_live_surface_presentations();
    assert(clipped.size() == 4U);
    for (const gui_forms::LiveSurfacePresentation& presentation : clipped) {
        assert(presentation.control == (*live_control).runtime_id());
        assert(Rect::intersection(presentation.clip,
                                  {20.0, 20.0, 30.0, 20.0}).empty());
    }

    (*overlay).set_visible(false);
    LiveSurfaceWriteLease resumed_write =
        (*composited_surface).try_acquire_write();
    assert(resumed_write);
    fill(resumed_write, std::byte{0x32});
    assert(resumed_write.publish() == 2U);
    const std::vector<LiveSurfacePresentation> resumed = window.take_live_surface_presentations();
    assert(resumed.size() == 1U &&
           resumed.front().control == (*live_control).runtime_id());
}
