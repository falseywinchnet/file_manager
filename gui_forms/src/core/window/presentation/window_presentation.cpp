#include "gui_forms/live_surface.hpp"
#include "gui_forms/window.hpp"

#include "../../display/chunk/display_chunk.hpp"
#include "../../display/recording_painter/recording_painter.hpp"
#include "../../display/replay/replay_display_chunk.hpp"
#include "../popup/popup_attachment.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

namespace gui_forms {

void Window::collect_paint_checkpoints(
    const Control::Ptr& control, PaintControlCheckpointList& checkpoints) {
    if (!control) return;
    checkpoints.push_back({control, (*control).display_chunk_});
    for (const Control::Ptr& child : (*control).children_) {
        collect_paint_checkpoints(child, checkpoints);
    }
}

void Window::abandon_paint(
    const PaintControlCheckpointList& checkpoints,
    std::uint64_t generation_before, Rect paint_bounds) {
    for (const PaintControlCheckpoint& checkpoint : checkpoints) {
        if (!checkpoint.control || !(*checkpoint.control).is_alive()) continue;
        (*checkpoint.control).display_chunk_ = checkpoint.chunk;
        (*checkpoint.control).dirty_ |= Dirty::paint;
    }
    if (root_ && (*root_).is_alive()) {
        static_cast<void>(recompute_subtree_dirty(root_));
    }
    display_generation_ = generation_before;
    add_damage_all_planes(paint_bounds);
    paint_dirty_ = true;
    dirty_after_render_ = true;
    in_paint_ = false;
    ++paint_leases_abandoned_;
    update_paint_lease_state();
    update_display_cache_metrics();
    if (!root_ || !(*root_).is_alive()) {
        abandon_deferred_input();
    } else {
        schedule_deferred_input_drain();
    }
}

Rect Window::paint_plane_bounds(std::size_t index, Rect requested_damage,
                                Rect paint_bounds,
                                Rect window_bounds) const {
    if (!requested_damage.empty()) return paint_bounds;
    return Rect::intersection(plane_damage_[index].bounds(), window_bounds);
}

bool Window::has_popup_paint_dirty() const noexcept {
    for (const std::shared_ptr<detail::PopupAttachment>& popup : popups_) {
        const Control::Ptr overlay = (*popup).popup();
        if (overlay && has_dirty((*overlay).subtree_dirty_, Dirty::paint)) {
            return true;
        }
    }
    return false;
}

bool Window::has_plane_damage() const noexcept {
    for (const DamageRegion& damage : plane_damage_) {
        if (!damage.empty()) return true;
    }
    return false;
}

std::optional<PaintReceipt> Window::paint(Painter& painter,
                                          Rect requested_damage) {
    require_ui_thread("paint");
    if (in_paint_) {
        // Refresh/Update/native paint recursion requests a later pass. It must
        // never re-enter application painting while the exclusive lease is held.
        ++reentrant_paint_requests_deferred_;
        dirty_after_render_ = true;
        paint_dirty_ = true;
        paint_lease_state_ = PaintLeaseState::rendering_dirty;
        add_damage_all_planes({0.0, 0.0, client_size_.width, client_size_.height});
        request_paint_wake();
        return std::nullopt;
    }
    if (occluded_) {
        paint_lease_state_ = PaintLeaseState::occluded_dirty;
        return std::nullopt;
    }
    ensure_layout(true);

    if (!(*root_).is_alive()) {
        return std::nullopt;
    }

    Rect paint_bounds = requested_damage;
    if (paint_bounds.empty()) {
        DamageRegion aggregate;
        for (const DamageRegion& plane : plane_damage_) {
            for (const Rect rect : plane.rectangles()) {
                aggregate.add(rect);
            }
        }
        paint_bounds = aggregate.bounds();
    }
    const Rect window_bounds{0.0, 0.0, client_size_.width, client_size_.height};
    paint_bounds = Rect::intersection(paint_bounds, window_bounds);
    if (paint_bounds.empty()) {
        return std::nullopt;
    }

    PaintControlCheckpointList checkpoints;
    collect_paint_checkpoints(root_, checkpoints);
    for (const std::shared_ptr<detail::PopupAttachment>& popup : popups_) {
        collect_paint_checkpoints((*popup).popup(), checkpoints);
    }

    const std::uint64_t lease_revision = content_revision_;
    const std::uint64_t lease_epoch = surface_epoch_;
    const std::uint64_t generation_before = display_generation_;
    ++paint_leases_started_;
    in_paint_ = true;
    dirty_after_render_ = false;
    paint_lease_state_ = PaintLeaseState::rendering;
    std::uint64_t visited_nodes = 0;
    std::uint64_t painted_controls = 0;
    std::uint64_t consumed_invalidations = 0;
    std::uint64_t chunks_rebuilt = 0;
    std::uint64_t chunks_reused = 0;
    std::uint64_t commands_replayed = 0;
    try {
        // Application callbacks and retained chunk rebuilding record into a
        // complete candidate command list. No candidate command reaches the
        // host raster until every callback has returned successfully.
        detail::RecordingPainter candidate;
        // A top-level retained surface owns an explicit backplane even when
        // its application root is a transparent layout container. Replaying
        // only children cannot erase pixels formerly occupied by an overlay
        // in the gaps between those children. Resolve the immutable Window
        // recipe in full-window coordinates, then clip it to this transaction
        // so partial gradient repainting never shifts its authored geometry.
        ControlVisualContext backplane_context;
        backplane_context.surface = active_
            ? ControlSurfaceState::normal
            : ControlSurfaceState::deactivated;
        backplane_context.high_contrast = presentation_settings_.high_contrast;
        candidate.save();
        candidate.clip_rect(paint_bounds);
        paint_surface_material(
            candidate, window_bounds,
            (*theme_).resolve(ControlVisualRole::window,
                            backplane_context).material);
        candidate.restore();
        std::vector<Control::Ptr> popup_roots;
        popup_roots.reserve(popups_.size());
        for (const std::shared_ptr<gui_forms::detail::PopupAttachment>& popup : popups_) popup_roots.push_back((*popup).popup());
        // Complete the application root across every paint plane before any
        // popup root. A popup is one composited retained surface; interleaving
        // its backplane with later application planes lets ordinary content
        // paint over the popup and leaves only overlay-plane pixels visible.
        for (std::size_t index = 0; index < paint_plane_count; ++index) {
            const Rect plane_bounds = Window::paint_plane_bounds(
                index, requested_damage, paint_bounds, window_bounds);
            if (plane_bounds.empty()) continue;
            const PaintPlane plane = static_cast<PaintPlane>(index);
            paint_recursive(root_, candidate, plane_bounds, plane, visited_nodes,
                            painted_controls, consumed_invalidations, chunks_rebuilt,
                            chunks_reused, commands_replayed);
        }
        // Window-owned popup roots are composited after all application
        // content, in opening order and independently of consumer layout.
        for (const Control::Ptr& overlay : popup_roots) {
            if (!overlay || !(*overlay).is_alive() || (*overlay).window_ != this) {
                continue;
            }
            for (std::size_t index = 0; index < paint_plane_count; ++index) {
                const Rect plane_bounds = Window::paint_plane_bounds(
                    index, requested_damage, paint_bounds, window_bounds);
                if (!plane_bounds.empty()) {
                    const PaintPlane plane = static_cast<PaintPlane>(index);
                    paint_recursive(overlay, candidate, plane_bounds, plane,
                                    visited_nodes, painted_controls,
                                    consumed_invalidations, chunks_rebuilt,
                                    chunks_reused, commands_replayed);
                }
            }
        }
        const std::shared_ptr<const detail::DisplayChunk> transaction =
            candidate.finish(
            display_generation_, PaintPlane::control, window_bounds);
        if (lease_epoch != surface_epoch_ || !(*root_).is_alive()) {
            abandon_paint(checkpoints, generation_before, paint_bounds);
            return std::nullopt;
        }
        static_cast<void>(detail::replay_display_chunk(*transaction, painter));
        // A backend painter may enter a native callback while replaying. A
        // resize, scale transition, or owner retirement at that boundary
        // invalidates the candidate even though every draw command returned.
        // The host receives no receipt and therefore cannot publish it.
        if (lease_epoch != surface_epoch_ || !(*root_).is_alive()) {
            abandon_paint(checkpoints, generation_before, paint_bounds);
            return std::nullopt;
        }
    } catch (...) {
        abandon_paint(checkpoints, generation_before, paint_bounds);
        throw;
    }

    in_paint_ = false;
    ++paint_leases_completed_;
    rendered_revision_ = lease_revision;

    // Damage that arrived while callbacks were running belongs to the next
    // revision. Never consume it as though it were part of this lease.
    const bool changed_while_rendering =
        dirty_after_render_ || content_revision_ != lease_revision;
    if (!changed_while_rendering) {
        for (std::size_t index = 0; index < paint_plane_count; ++index) {
            Rect plane_bounds = paint_bounds;
            if (requested_damage.empty()) {
                plane_bounds = Rect::intersection(plane_damage_[index].bounds(),
                                                  window_bounds);
            }
            const Rect pending_bounds = plane_damage_[index].bounds();
            if (pending_bounds.empty() ||
                Rect::intersection(plane_bounds, pending_bounds) == pending_bounds) {
                plane_damage_[index].clear();
            }
        }
    }

    const bool full_window = paint_bounds.area() >= window_bounds.area();
    metrics_.record_paint(visited_nodes, painted_controls, consumed_invalidations,
                          chunks_rebuilt, chunks_reused, commands_replayed,
                          paint_bounds.area(), full_window);
    update_display_cache_metrics();

    const bool popup_paint_dirty = has_popup_paint_dirty();
    paint_dirty_ = dirty_after_render_ ||
                   has_dirty((*root_).subtree_dirty_, Dirty::paint) ||
                   popup_paint_dirty ||
                   has_plane_damage();
    update_paint_lease_state();
    schedule_deferred_input_drain();
    return PaintReceipt{lease_revision, lease_epoch};
}

bool Window::notify_presented(PaintReceipt receipt,
                              std::uint64_t duration_nanoseconds) {
    require_ui_thread("paint presentation release");
    const bool current_epoch = receipt.surface_epoch == surface_epoch_;
    const bool complete_revision = receipt.rendered_revision != 0U &&
        receipt.rendered_revision <= rendered_revision_;
    const bool monotonic = receipt.rendered_revision > presented_revision_;
    if (!current_epoch || !complete_revision || !monotonic) {
        ++presentation_receipts_rejected_;
        update_paint_lease_state();
        return false;
    }
    presented_revision_ = receipt.rendered_revision;
    ++presentation_receipts_accepted_;
    metrics_.record_present(duration_nanoseconds);
    update_paint_lease_state();
    return true;
}

void Window::notify_presented(std::uint64_t duration_nanoseconds) {
    static_cast<void>(notify_presented(
        PaintReceipt{rendered_revision_, surface_epoch_}, duration_nanoseconds));
}

PaintLeaseSnapshot Window::paint_lease_snapshot() const noexcept {
    return {paint_lease_state_, content_revision_, rendered_revision_,
            presented_revision_, surface_epoch_, paint_leases_started_,
            paint_leases_completed_, paint_leases_abandoned_,
            reentrant_paint_requests_deferred_, paint_wakes_queued_,
            paint_wakes_coalesced_, presentation_receipts_accepted_,
            presentation_receipts_rejected_, paint_wake_pending_,
            dirty_after_render_};
}

bool Window::queue_live_surface_presentation(
    const Control::Ptr& control, std::shared_ptr<LiveSurface> surface) {
    require_ui_thread("live-surface presentation");
    if (!control || !surface || !(*control).is_alive() ||
        (*control).window_ != this) {
        return false;
    }
    std::pair<
        std::unordered_map<std::uint64_t, LiveSurfaceRegistration>::iterator,
        bool> insertion = live_surface_registrations_.try_emplace(
            (*control).runtime_id().value,
            LiveSurfaceRegistration{control, surface, 0U, 0U, false});
    std::unordered_map<std::uint64_t, LiveSurfaceRegistration>::iterator entry =
        insertion.first;
    const bool inserted = insertion.second;
    if (!inserted && (*entry).second.surface != surface) {
        (*entry).second =
            LiveSurfaceRegistration{control, std::move(surface), 0U, 0U, false};
    } else {
        (*entry).second.control = control;
        (*entry).second.surface = std::move(surface);
    }
    return true;
}

void Window::collect_visible_overlay_rectangles(
    const Control::Ptr& control, std::vector<Rect>& rectangles) const {
    if (!control || !(*control).is_alive() || (*control).window_ != this ||
        !(*control).effectively_visible()) {
        return;
    }
    if ((*control).paint_plane_ == PaintPlane::overlay) {
        const Rect bounds = Rect::intersection(
            absolute_bounds_of(*control),
            {0.0, 0.0, client_size_.width, client_size_.height});
        if (!bounds.empty()) rectangles.push_back(bounds);
    }
    for (const Control::Ptr& child : (*control).children_) {
        collect_visible_overlay_rectangles(child, rectangles);
    }
}

std::vector<Rect> Window::subtract_rectangle(Rect source, Rect cover) {
    std::vector<Rect> fragments;
    const Rect overlap = Rect::intersection(source, cover);
    if (overlap.empty()) {
        fragments.push_back(source);
        return fragments;
    }
    const double source_right = source.x + source.width;
    const double source_bottom = source.y + source.height;
    const double overlap_right = overlap.x + overlap.width;
    const double overlap_bottom = overlap.y + overlap.height;
    const Rect candidates[] = {
        {source.x, source.y, source.width, overlap.y - source.y},
        {source.x, overlap_bottom, source.width,
         source_bottom - overlap_bottom},
        {source.x, overlap.y, overlap.x - source.x, overlap.height},
        {overlap_right, overlap.y, source_right - overlap_right,
         overlap.height},
    };
    for (const Rect& candidate : candidates) {
        if (!candidate.empty()) fragments.push_back(candidate);
    }
    return fragments;
}

std::vector<LiveSurfacePresentation>
Window::take_live_surface_presentations() {
    require_ui_thread("live-surface presentation drain");
    std::vector<LiveSurfacePresentation> result;
    result.reserve(live_surface_registrations_.size());

    // Direct live presentation is a fast path through retained composition,
    // not an always-on-top plane. Gather overlay ownership once, then subtract
    // those pixels from each live presentation. Uncovered spectrum/video pixels
    // therefore keep advancing while a menu is open without allowing the live
    // lane to paint over the menu.
    std::vector<Rect> overlay_rectangles;
    collect_visible_overlay_rectangles(root_, overlay_rectangles);
    for (const std::shared_ptr<gui_forms::detail::PopupAttachment>& popup : popups_) {
        if (popup) {
            collect_visible_overlay_rectangles(
                (*popup).popup(), overlay_rectangles);
        }
    }
    for (LiveSurfaceRegistrationMap::iterator iterator =
             live_surface_registrations_.begin();
         iterator != live_surface_registrations_.end();) {
        const Control::Ptr control = (*iterator).second.control.lock();
        if (!control || !(*control).is_alive() || (*control).window_ != this ||
            !(*iterator).second.surface) {
            iterator = live_surface_registrations_.erase(iterator);
            continue;
        }
        if (occluded_ || !popups_.empty() || !(*control).visible_ ||
            !(*control).effectively_visible()) {
            ++iterator;
            continue;
        }

        const LiveSurfaceSnapshot snapshot = (*(*iterator).second.surface).snapshot();
        if (!snapshot.has_frame) {
            ++iterator;
            continue;
        }

        const Rect destination = absolute_bounds_of(*control);
        Rect clip = Rect::intersection(
            destination, {0.0, 0.0, client_size_.width, client_size_.height});
        bool valid = !destination.empty() && !clip.empty();
        for (Control::Ptr ancestor = (*control).parent();
             ancestor && valid && !clip.empty();
             ancestor = (*ancestor).parent()) {
            if (!(*ancestor).is_alive() || (*ancestor).window_ != this ||
                !(*ancestor).visible_) {
                valid = false;
                break;
            }
            const Rect ancestor_bounds = absolute_bounds_of(*ancestor);
            const Rect viewport = (*ancestor).child_viewport_rectangle();
            clip = Rect::intersection(
                clip, {ancestor_bounds.x + viewport.x,
                       ancestor_bounds.y + viewport.y,
                       viewport.width, viewport.height});
        }
        if (valid && !clip.empty()) {
            std::vector<Rect> clips{clip};
            bool clipped_by_overlay = false;
            for (const Rect& overlay : overlay_rectangles) {
                std::vector<Rect> remaining;
                for (const Rect& candidate : clips) {
                    if (!Rect::intersection(candidate, overlay).empty()) {
                        clipped_by_overlay = true;
                    }
                    std::vector<Rect> fragments = Window::subtract_rectangle(
                        candidate, overlay);
                    remaining.insert(remaining.end(), fragments.begin(),
                                     fragments.end());
                }
                clips = std::move(remaining);
                if (clips.empty()) break;
            }
            const bool same_generation =
                (*iterator).second.sampled_epoch == snapshot.epoch &&
                (*iterator).second.sampled_generation ==
                    snapshot.published_generation;
            if (!same_generation || clipped_by_overlay ||
                (*iterator).second.sampled_with_overlay_clip) {
                for (const Rect& visible_clip : clips) {
                    result.push_back(LiveSurfacePresentation{
                        (*control).runtime_id(), (*iterator).second.surface,
                        destination, visible_clip});
                }
            }
            (*iterator).second.sampled_epoch = snapshot.epoch;
            (*iterator).second.sampled_generation = snapshot.published_generation;
            (*iterator).second.sampled_with_overlay_clip = clipped_by_overlay;
        }
        ++iterator;
    }
    return result;
}

void Window::set_paint_wake_handler(std::function<void()> wake) {
    require_ui_thread("paint wake-handler mutation");
    paint_wake_handler_ = std::move(wake);
    paint_wake_pending_ = false;
    if (paint_wake_handler_ && paint_dirty_ && !occluded_) {
        request_paint_wake();
    }
}

DamageRegion Window::take_damage() {
    require_ui_thread("damage mutation");
    // Hosts request damage before entering their native paint transaction.
    // Commit layout first so geometry changes discovered during arrange are
    // included in that same transaction instead of becoming stranded after
    // the host has already consumed the old region.
    ensure_layout(true);
    DamageRegion result;
    for (DamageRegion& plane : plane_damage_) {
        for (const Rect rect : plane.rectangles()) {
            result.add(rect);
        }
        plane = {};
    }
    acknowledge_paint_wake_if_damage_drained();
    return result;
}

DamageRegion Window::take_damage(PaintPlane plane) {
    require_ui_thread("plane damage mutation");
    if (!is_valid_paint_plane(plane)) {
        throw std::invalid_argument("invalid paint plane");
    }
    ensure_layout(true);
    const std::size_t index = paint_plane_index(plane);
    DamageRegion result = std::move(plane_damage_[index]);
    plane_damage_[index] = {};
    const bool popup_paint_dirty = has_popup_paint_dirty();
    paint_dirty_ = has_dirty((*root_).subtree_dirty_, Dirty::paint) ||
                   popup_paint_dirty ||
                   has_plane_damage();
    acknowledge_paint_wake_if_damage_drained();
    return result;
}

} // namespace gui_forms
