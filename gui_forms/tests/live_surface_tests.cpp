#include "gui_forms/gui_forms.hpp"

#include <cassert>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace {

void fill(gui_forms::LiveSurfaceWriteLease& lease, std::byte value) {
    for (std::byte& pixel : lease.pixels()) pixel = value;
}

} // namespace

int main() {
    using namespace gui_forms;

    assert(!LiveSurface::create({}));
    auto surface = LiveSurface::create({4U, 3U});
    assert(surface);
    assert(!surface->acquire_latest());

    std::atomic<std::uint64_t> wakes{};
    auto wake = surface->connect_presentation_wake([&] {
        // Publication must not hold the surface mutex while signaling. A
        // scheduler is allowed to inspect the newest state from this thread.
        assert(surface->snapshot().description.width != 0U);
        ++wakes;
    });
    assert(wake.connected());

    auto first_write = surface->try_acquire_write();
    assert(first_write);
    assert(first_write.width() == 4U);
    assert(first_write.height() == 3U);
    assert(first_write.row_bytes() == 16U);
    fill(first_write, std::byte{0x11});
    assert(first_write.publish({1.0, 1.0, 2.0, 1.0}) == 1U);
    assert(wakes == 1U);

    auto first = surface->acquire_latest();
    assert(first);
    assert(first.generation() == 1U);
    assert(first.damage() == Rect(1.0, 1.0, 2.0, 1.0));
    assert(first.pixels().front() == std::byte{0x11});

    auto second_write = surface->try_acquire_write(true);
    assert(second_write);
    assert(second_write.pixels().front() == std::byte{0x11});
    fill(second_write, std::byte{0x22});
    assert(second_write.publish() == 2U);
    assert(wakes == 2U);
    auto second = surface->acquire_latest();
    assert(second && second.generation() == 2U);
    assert(second.pixels().front() == std::byte{0x22});
    // A compositor lease remains immutable across newer publications.
    assert(first.pixels().front() == std::byte{0x11});

    auto third_write = surface->try_acquire_write();
    assert(third_write);
    fill(third_write, std::byte{0x33});
    assert(third_write.publish() == 3U);
    assert(wakes == 3U);
    auto third = surface->acquire_latest();
    assert(third && third.pixels().front() == std::byte{0x33});
    // All three candidates are leased. A high-rate producer drops instead of
    // blocking or mutating a frame currently owned by the compositor.
    assert(!surface->try_acquire_write());
    assert(surface->snapshot().dropped_acquires == 1U);

    first = {};
    auto recovered = surface->try_acquire_write();
    assert(recovered);
    recovered.abandon();
    assert(!surface->snapshot().write_active);

    second = {};
    third = {};
    assert(surface->reconfigure({2U, 2U}));
    assert(wakes == 4U);
    const auto reconfigured = surface->snapshot();
    assert(reconfigured.epoch == 2U);
    assert(!reconfigured.has_frame);
    assert(reconfigured.description.width == 2U);
    assert(reconfigured.description.height == 2U);

    auto final_write = surface->try_acquire_write();
    assert(final_write && final_write.pixels().size() == 16U);
    fill(final_write, std::byte{0x7f});
    assert(final_write.publish() == 1U);
    assert(wakes == 5U);
    auto final_frame = surface->acquire_latest();
    assert(final_frame && final_frame.epoch() == 2U);
    assert(final_frame.pixels().back() == std::byte{0x7f});

    wake.disconnect();
    assert(!wake.connected());
    final_frame = {};
    auto disconnected_write = surface->try_acquire_write();
    assert(disconnected_write);
    fill(disconnected_write, std::byte{0x55});
    assert(disconnected_write.publish() == 2U);
    assert(wakes == 5U);

    auto root = make_control<Control>(StableId("live.root"));
    auto live_control = make_control<Control>(StableId("live.content"));
    auto overlay = make_control<Control>(StableId("live.overlay"));
    root->set_requested_bounds({0.0, 0.0, 100.0, 80.0});
    live_control->set_requested_bounds({10.0, 10.0, 70.0, 50.0});
    overlay->set_requested_bounds({20.0, 20.0, 30.0, 20.0});
    overlay->set_paint_plane(PaintPlane::overlay);
    root->add_child(live_control);
    root->add_child(overlay);
    Window window(root, {100.0, 80.0});
    window.perform_layout();

    auto composited_surface = LiveSurface::create({4U, 3U});
    assert(composited_surface);
    auto composited_write = composited_surface->try_acquire_write();
    assert(composited_write);
    fill(composited_write, std::byte{0x31});
    assert(composited_write.publish() == 1U);
    assert(window.queue_live_surface_presentation(live_control,
                                                  composited_surface));
    // An intersecting retained overlay owns only its pixels. The direct lane
    // continues presenting every uncovered fragment around it.
    const auto clipped = window.take_live_surface_presentations();
    assert(clipped.size() == 4U);
    for (const auto& presentation : clipped) {
        assert(presentation.control == live_control->runtime_id());
        assert(Rect::intersection(presentation.clip,
                                  {20.0, 20.0, 30.0, 20.0}).empty());
    }

    overlay->set_visible(false);
    auto resumed_write = composited_surface->try_acquire_write();
    assert(resumed_write);
    fill(resumed_write, std::byte{0x32});
    assert(resumed_write.publish() == 2U);
    const auto resumed = window.take_live_surface_presentations();
    assert(resumed.size() == 1U &&
           resumed.front().control == live_control->runtime_id());
}
