#include "gui_forms/live_surface.hpp"
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
} // namespace

int main() {
    description_and_frame_lifetime();
    damage_coverage();
    return 0;
}
