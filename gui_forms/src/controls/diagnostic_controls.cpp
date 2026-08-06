#include "gui_forms/diagnostic_controls.hpp"

#include "gui_forms/window.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace gui_forms {
namespace {

using namespace std::chrono_literals;

[[nodiscard]] std::vector<EasingPreviewTrack> default_easing_tracks(
    const BasicControlStyle& style) {
    return {
        {EasingCurve::linear, "Linear", style.accent},
        {EasingCurve::ease_in, "Ease in", style.accent},
        {EasingCurve::ease_out, "Ease out", style.accent},
        {EasingCurve::ease_in_out, "Ease in/out", style.visited_link},
        {EasingCurve::smooth_step, "Smooth step", style.visited_link},
        {EasingCurve::back_out, "Back out", style.visited_link},
        {EasingCurve::bounce_out, "Bounce", Color::rgba(211, 121, 42)},
        {EasingCurve::elastic_out, "Elastic", Color::rgba(211, 121, 42)},
    };
}

} // namespace

DrawingSurface::DrawingSurface(StableId stable_id)
    : Control(std::move(stable_id)) {}

void DrawingSurface::set_paint_callback(PaintCallback callback) {
    require_mutable();
    paint_callback_ = std::move(callback);
    invalidate(Dirty::paint | Dirty::semantics);
}

void DrawingSurface::set_hit_test_visible(bool visible) {
    require_mutable();
    if (hit_test_visible_ == visible) return;
    hit_test_visible_ = visible;
    invalidate(Dirty::hit_test | Dirty::semantics);
}

void DrawingSurface::set_semantic_role(SemanticRole role) {
    require_mutable();
    if (semantic_role_ == role) return;
    semantic_role_ = role;
    invalidate(Dirty::semantics);
}

void DrawingSurface::on_paint(Painter& painter, Rect local_damage) {
    if (paint_callback_) {
        paint_callback_(painter,
                        {0.0, 0.0, committed_arranged_bounds().width,
                         committed_arranged_bounds().height},
                        local_damage);
    }
}

bool DrawingSurface::hit_test_local(Point local_point) const {
    return hit_test_visible_ && Control::hit_test_local(local_point);
}

SemanticDescriptor DrawingSurface::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = semantic_role_;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    descriptor.exposed = !descriptor.name.empty() || !descriptor.description.empty();
    return descriptor;
}

MetricsView::MetricsView(StableId stable_id, std::string title)
    : Control(std::move(stable_id)), title_(std::move(title)) {
    set_accessible_name(title_);
}

void MetricsView::set_title(std::string title) {
    require_mutable();
    if (title_ == title) return;
    title_ = std::move(title);
    set_accessible_name(title_);
    invalidate(Dirty::paint | Dirty::semantics);
}

void MetricsView::set_style(BasicControlStyle style) {
    require_mutable();
    if (style_ == style) return;
    style_ = style;
    invalidate(Dirty::paint);
}

std::string MetricsView::metrics_text() const {
    if (window() == nullptr) return "Detached";
    const MetricsSnapshot metrics = window()->metrics_snapshot();
    std::ostringstream text;
    text << metrics.control_count << " controls · "
         << metrics.paint_passes << " paints · "
         << metrics.display_chunks_reused << " chunks reused · "
         << metrics.input_events << " inputs · "
         << metrics.active_surface_count << " active surfaces · "
         << metrics.focus_scope_depth << " focus scopes · damage "
         << static_cast<unsigned long long>(std::round(metrics.painted_damage_area))
         << " px";
    return text.str();
}

void MetricsView::on_paint(Painter& painter, Rect) {
    const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                      committed_arranged_bounds().height};
    painter.fill_rect(bounds, style_.text);
    painter.fill_rect({0.0, 0.0, 5.0, bounds.height}, style_.accent);
    painter.draw_text_utf8({16.0, 24.0}, title_,
                           {FontRole::control, 12.0, 700, false},
                           style_.face_light);
    const std::string metrics = metrics_text();
    const std::size_t split = metrics.find(" · ", metrics.size() / 2U);
    const std::string first = split == std::string::npos
        ? metrics : metrics.substr(0U, split);
    const std::string second = split == std::string::npos
        ? std::string{} : metrics.substr(split + 3U);
    painter.draw_text_utf8({16.0, 48.0}, first,
                           {FontRole::content, 11.0, 400, false},
                           style_.face);
    if (!second.empty()) {
        painter.draw_text_utf8({16.0, 68.0}, second,
                               {FontRole::content, 11.0, 400, false},
                               style_.face);
    }
}

bool MetricsView::hit_test_local(Point) const { return false; }

SemanticDescriptor MetricsView::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::group;
    descriptor.name = title_;
    descriptor.value = metrics_text();
    descriptor.exposed = true;
    return descriptor;
}

EasingPreview::EasingPreview(StableId stable_id)
    : Control(std::move(stable_id)) {
    AnimationSpec specification;
    specification.duration = 2400ms;
    specification.infinite = true;
    specification.easing = EasingCurve::linear;
    timeline_.set_specification(specification);
    tracks_ = default_easing_tracks(style_);
    set_accessible_name("Animation easing preview");
    set_accessible_description(
        "Retained timeline tracks comparing configured easing curves");
}

void EasingPreview::set_title(std::string title) {
    require_mutable();
    if (title_ == title) return;
    title_ = std::move(title);
    invalidate(Dirty::paint | Dirty::semantics);
}

void EasingPreview::set_style(BasicControlStyle style) {
    require_mutable();
    if (style_ == style) return;
    style_ = style;
    invalidate(Dirty::paint);
}

void EasingPreview::set_specification(AnimationSpec specification) {
    require_mutable();
    timeline_.set_specification(specification);
    if (timeline_started_) {
        const FrameTime now = FrameClock::now();
        timeline_.start(now);
        if (!motion_policy_.active()) timeline_.pause(now);
        phase_ = timeline_.sample(now).progress;
    }
    frames_.disconnect();
    if (motion_policy_.active()) register_frames();
    invalidate(Dirty::paint | Dirty::semantics);
}

void EasingPreview::set_tracks(std::vector<EasingPreviewTrack> tracks) {
    require_mutable();
    if (tracks.empty() || tracks.size() > 32U) {
        throw std::invalid_argument(
            "easing preview requires between one and 32 tracks");
    }
    for (const EasingPreviewTrack& track : tracks) {
        if (track.label.empty()) {
            throw std::invalid_argument("easing preview track label may not be empty");
        }
    }
    if (tracks_ == tracks) return;
    tracks_ = std::move(tracks);
    invalidate(Dirty::paint | Dirty::semantics);
}

void EasingPreview::set_motion_policy(MotionPolicy policy) {
    require_mutable();
    if (motion_policy_ == policy) return;
    const bool was_live = motion_policy_.active();
    const bool becomes_live = policy.active();
    frames_.disconnect();
    if (timeline_started_) {
        const FrameTime transition = FrameClock::now();
        if (was_live && !becomes_live) {
            timeline_.pause(transition);
            phase_ = timeline_.sample(transition).progress;
        } else if (!was_live && becomes_live) {
            timeline_.resume(transition);
        }
    }
    motion_policy_ = policy;
    if (becomes_live) register_frames();
    invalidate(Dirty::paint | Dirty::semantics);
}

void EasingPreview::on_frame(FrameTime now) {
    if (!motion_policy_.active()) return;
    const AnimationSample sample = timeline_.sample(now);
    phase_ = sample.progress;
    if (sample.finished) frames_.disconnect();
}

std::string EasingPreview::motion_readout(double presented_phase) const {
    const char* name = !motion_policy_.enabled
        ? "DISABLED"
        : motion_policy_.reduced && motion_policy_.paused
            ? "REDUCED + PAUSED"
            : motion_policy_.reduced
                ? "REDUCED"
                : motion_policy_.paused ? "PAUSED" : "LIVE";
    char readout[96]{};
    std::snprintf(readout, sizeof(readout), "%s · %03d%%", name,
                  static_cast<int>(std::round(presented_phase * 100.0)));
    return readout;
}

void EasingPreview::on_paint(Painter& painter, Rect) {
    const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                      committed_arranged_bounds().height};
    painter.fill_rect(bounds, style_.paper);
    painter.stroke_rect({0.5, 0.5, std::max(0.0, bounds.width - 1.0),
                         std::max(0.0, bounds.height - 1.0)},
                        style_.border, 1.0);
    painter.draw_text_utf8({18.0, 25.0}, title_,
                           {FontRole::control, 13.0, 700, false},
                           style_.dark_border);
    const double presented_phase = motion_policy_.presentation_phase(phase_);
    const std::string readout = motion_readout(presented_phase);
    painter.draw_text_utf8({bounds.width - 150.0, 25.0}, readout,
                           {FontRole::control, 11.0, 600, false},
                           motion_policy_.active() ? style_.accent
                                                   : Color::rgba(211, 121, 42));

    const double row_height = tracks_.empty()
        ? 0.0 : std::max(18.0, (bounds.height - 48.0) / tracks_.size());
    const double directed = presented_phase < 0.5
        ? presented_phase * 2.0 : (1.0 - presented_phase) * 2.0;
    for (std::size_t index = 0U; index < tracks_.size(); ++index) {
        const double y = 48.0 + static_cast<double>(index) * row_height;
        painter.draw_text_utf8({18.0, y + 15.0}, tracks_[index].label,
                               {FontRole::content, 11.0, 400, false}, style_.text);
        const double track_x = 112.0;
        const double track_width = std::max(40.0, bounds.width - 140.0);
        painter.fill_rect({track_x, y + 8.0, track_width, 3.0}, style_.face);
        const double eased = std::clamp(
            apply_easing(tracks_[index].curve, directed), -0.08, 1.08);
        const double x = std::clamp(
            track_x + eased * std::max(0.0, track_width - 14.0),
            track_x, track_x + std::max(0.0, track_width - 14.0));
        painter.fill_rect({x, y + 2.0, 14.0, 14.0}, tracks_[index].color);
        painter.stroke_rect({x + 0.5, y + 2.5, 13.0, 13.0},
                            style_.dark_border, 1.0);
    }
}

bool EasingPreview::hit_test_local(Point) const { return false; }

SemanticDescriptor EasingPreview::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::image;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    const double presented = motion_policy_.presentation_phase(phase_);
    descriptor.value = motion_readout(presented);
    descriptor.numeric_value = presented;
    descriptor.minimum_value = 0.0;
    descriptor.maximum_value = 1.0;
    if (motion_policy_.active()) descriptor.states |= SemanticState::busy;
    descriptor.exposed = true;
    return descriptor;
}

void EasingPreview::on_attached_to_window() {
    Control::on_attached_to_window();
    const FrameTime attached = FrameClock::now();
    if (!timeline_started_) {
        timeline_.start(attached);
        timeline_started_ = true;
    } else if (motion_policy_.active()) {
        timeline_.resume(attached);
    }
    if (!motion_policy_.active()) timeline_.pause(attached);
    else register_frames();
}

void EasingPreview::on_detached_from_window() noexcept {
    frames_.disconnect();
    if (timeline_started_) {
        const FrameTime detached = FrameClock::now();
        phase_ = timeline_.sample(detached).progress;
        timeline_.pause(detached);
    }
    Control::on_detached_from_window();
}

void EasingPreview::register_frames() {
    if (window() == nullptr || !motion_policy_.active()) return;
    const FrameInterval interval = motion_policy_.frame_interval(16ms);
    frames_ = window()->activate_surface(
        shared_from_this(), interval, FrameClock::now() + interval);
}

} // namespace gui_forms
