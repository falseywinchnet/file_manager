#include "gui_forms/live_surface.hpp"
#include "gui_forms/window.hpp"
#include "../src/host/macos/application/live_surface_damage.hpp"

#include <array>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace {
using namespace gui_forms;

void require(const bool condition, const char* message) {
    if (condition) return;
    std::cerr << message << '\n';
    std::exit(1);
}

void description_and_frame_lifetime() {
    const LiveSurfaceDescription defaults{};
    require(!defaults.opaque, "opaque defaults false");
    const std::shared_ptr<LiveSurface> surface = LiveSurface::create(
        {.width = 2U, .height = 2U, .opaque = true});
    require(surface && (*surface).snapshot().description.opaque, "create preserves opaque");
    LiveSurfaceWriteLease write = (*surface).try_acquire_write();
    require(static_cast<bool>(write), "write lease acquired");
    for (std::byte& pixel : write.pixels()) pixel = std::byte{255};
    require(write.publish() != 0U, "opaque frame published");
    const LiveSurfaceFrame old_frame = (*surface).acquire_latest();
    require(old_frame.opaque(), "frame holds opaque promise");
    require(!(*surface).reconfigure({.opaque = false}), "invalid reconfigure refused");
    require((*surface).snapshot().description.opaque, "refusal preserves promise");
    require((*surface).reconfigure({.width = 3U, .height = 2U}), "reconfigure to translucent");
    require(!(*surface).snapshot().description.opaque, "snapshot follows reconfigure");
    require(old_frame.opaque() && old_frame.width() == 2U, "old frame keeps its own description");
    write = (*surface).try_acquire_write();
    require(static_cast<bool>(write), "replacement write lease acquired");
    require(write.publish() != 0U, "translucent frame published");
    const LiveSurfaceFrame new_frame = (*surface).acquire_latest();
    require(!new_frame.opaque(), "replacement frame is translucent");
    require((*surface).reconfigure({.width = 2U, .height = 3U, .opaque = true}), "reconfigure to opaque");
    require((*surface).snapshot().description.opaque, "reconfigure preserves true");
    const LiveSurfaceFrame empty{};
    require(!empty.opaque(), "empty frame grants no coverage");
}

void damage_coverage() {
    const Rect dirty{0.0, 0.0, 10.0, 8.0};
    const std::array<Rect, 2> adjacent{{{0.0, 0.0, 4.0, 8.0}, {4.0, 0.0, 6.0, 8.0}}};
    require(detail::opaque_live_damage_covers(dirty, 1.0, adjacent, true, false), "union covers live-only damage");
    require(!detail::opaque_live_damage_covers(dirty, 1.0, adjacent, true, true), "retained damage requires paint");
    require(!detail::opaque_live_damage_covers(dirty, 1.0, adjacent, false, false), "first frame/resize/recovery requires paint");
    require(!detail::opaque_live_damage_covers(dirty, 1.0, {}, true, false), "translucent/absent frames cannot cover damage");
    const std::array<Rect, 4> hole{{{0.0, 0.0, 10.0, 2.0}, {0.0, 6.0, 10.0, 2.0},
                                  {0.0, 2.0, 2.0, 4.0}, {8.0, 2.0, 2.0, 4.0}}};
    require(!detail::opaque_live_damage_covers(dirty, 1.0, hole, true, false), "overlay hole is not bounding-box coverage");
    require(detail::opaque_live_damage_covers({0.0, 0.0, 10.0, 2.0}, 1.0, hole, true, false), "damage outside overlay remains covered");
    const std::array<Rect, 3> overlapping{{{0.0, 0.0, 6.0, 8.0}, {3.0, 0.0, 7.0, 4.0}, {3.0, 4.0, 7.0, 4.0}}};
    require(detail::opaque_live_damage_covers(dirty, 1.0, overlapping, true, false), "overlap and vertical union cover");
    const double scale = 1.25;
    const Rect fraction{0.1, 0.1, 7.8, 7.8};
    const std::array<Rect, 1> insufficient{{detail::opaque_live_clip(fraction, scale)}};
    require(!detail::opaque_live_damage_covers(fraction, scale, insufficient, true, false), "fractional boundary cannot promise outward pixels");
    const std::array<Rect, 1> complete{{detail::opaque_live_clip({0.0, 0.0, 8.0, 8.0}, scale)}};
    require(detail::opaque_live_damage_covers(fraction, scale, complete, true, false), "fractional damage rounds outward to covered device pixels");
    const std::array<Rect, 2> fractional_gap{{detail::opaque_live_clip({0.0, 0.0, 3.9, 8.0}, scale),
                                           detail::opaque_live_clip({4.1, 0.0, 3.9, 8.0}, scale)}};
    require(!detail::opaque_live_damage_covers({0.0, 0.0, 8.0, 8.0}, scale, fractional_gap, true, false), "subpixel overlay gap is not rounded away");
    require(!detail::opaque_live_damage_covers(dirty, 0.0, adjacent, true, false), "invalid scale refused");
    require(detail::opaque_live_clip(dirty, std::numeric_limits<double>::infinity()).empty(), "nonfinite scale refused");
}

void refreshed_presentations() {
    const std::shared_ptr<Control> root = make_control<Control>(StableId("root"));
    const std::shared_ptr<Control> first = make_control<Control>(StableId("first"));
    const std::shared_ptr<Control> second = make_control<Control>(StableId("second"));
    const std::shared_ptr<Control> overlay = make_control<Control>(StableId("overlay"));
    (*first).set_requested_bounds({0.0, 0.0, 4.0, 8.0});
    (*second).set_requested_bounds({4.0, 0.0, 4.0, 8.0});
    (*overlay).set_requested_bounds({0.0, 0.0, 4.0, 8.0});
    (*overlay).set_paint_plane(PaintPlane::overlay);
    (*overlay).set_visible(false);
    (*root).add_child(first);
    (*root).add_child(second);
    (*root).add_child(overlay);
    Window window(root, {8.0, 8.0});
    window.perform_layout();
    const std::shared_ptr<LiveSurface> surface_a = LiveSurface::create({.width = 4U, .height = 8U, .opaque = true});
    const std::shared_ptr<LiveSurface> surface_b = LiveSurface::create({.width = 4U, .height = 8U, .opaque = true});
    require(surface_a && surface_b, "retry surfaces created");
    LiveSurfaceWriteLease write_a = (*surface_a).try_acquire_write();
    LiveSurfaceWriteLease write_b = (*surface_b).try_acquire_write();
    require(write_a && write_b, "retry write leases acquired");
    for (std::byte& value : write_a.pixels()) value = std::byte{255};
    for (std::byte& value : write_b.pixels()) value = std::byte{255};
    const std::uint64_t generation_a = write_a.publish();
    const std::uint64_t generation_b = write_b.publish();
    require(generation_a != 0U && generation_b != 0U, "initial generations published");
    require(window.queue_live_surface_presentation(first, surface_a), "first registered");
    require(window.queue_live_surface_presentation(second, surface_b), "second registered");
    require(window.take_live_surface_presentations().size() == 2U, "initial batch sampled");
    require(window.take_live_surface_presentations().empty(), "unchanged drain stays empty by default");
    // A failed to present; only B changes before the retry.
    write_b = (*surface_b).try_acquire_write(true);
    require(static_cast<bool>(write_b), "second replacement acquired");
    require(write_b.publish() != 0U, "only second advances");
    const std::vector<LiveSurfacePresentation> retry = window.take_live_surface_presentations(true);
    require(retry.size() == 2U, "refresh retains unchanged failed surface A");
    (*overlay).set_visible(true);
    (*second).set_visible(false);
    window.perform_layout();
    require(window.take_live_surface_presentations(true).empty(), "refresh discards newly covered and hidden surfaces");
}
} // namespace

int main() {
    description_and_frame_lifetime();
    damage_coverage();
    refreshed_presentations();
    return 0;
}
