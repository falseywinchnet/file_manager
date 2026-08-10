#include "gui_forms/controls/range_control/progress_bar/progress_bar.hpp"

#include "../range_control_rendering.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace gui_forms {
using namespace range_control_detail;

ProgressBar::ProgressBar(StableId stable_id)
    : RangeControl(std::move(stable_id)) {
    set_focusable(false);
}

void ProgressBar::set_visual_style(ProgressBarVisualStyle style_value) {
    require_mutable();
    if (visual_style_ == style_value) {
        return;
    }
    visual_style_ = style_value;
    animation_phase_ = 0.0;
    update_animation_registration();
    invalidate(Dirty::paint | Dirty::semantics);
}

void ProgressBar::set_overlay_style(ProgressBarOverlayStyle style_value) {
    require_mutable();
    if (overlay_style_ == style_value) return;
    overlay_style_ = style_value;
    animation_phase_ = 0.0;
    update_animation_registration();
    invalidate(Dirty::paint | Dirty::semantics);
}

void ProgressBar::set_stripe_width(double width) {
    require_mutable();
    require_finite(width, "progress stripe width must be finite");
    if (width < 2.0 || width > 32.0) {
        throw std::invalid_argument(
            "progress stripe width must be between 2 and 32 pixels");
    }
    if (stripe_width_ == width) return;
    stripe_width_ = width;
    invalidate(Dirty::paint);
}

void ProgressBar::set_animation_appearance(
    ProgressBarAnimationAppearance appearance) {
    require_mutable();
    require_finite(appearance.pulse_extent,
                   "progress pulse extent must be finite");
    require_finite(appearance.laser_edge_extent,
                   "progress laser edge extent must be finite");
    require_finite(appearance.laser_phase_pitch,
                   "progress laser phase pitch must be finite");
    if (appearance.pulse_extent < 0.1 || appearance.pulse_extent > 0.8) {
        throw std::invalid_argument(
            "progress pulse extent must be between 0.1 and 0.8");
    }
    if (appearance.laser_edge_extent < 2.0 ||
        appearance.laser_edge_extent > 48.0) {
        throw std::invalid_argument(
            "progress laser edge extent must be between 2 and 48 pixels");
    }
    if (appearance.laser_phase_pitch < 3.0 ||
        appearance.laser_phase_pitch > 64.0) {
        throw std::invalid_argument(
            "progress laser phase pitch must be between 3 and 64 pixels");
    }
    if (animation_appearance_ == appearance) return;
    animation_appearance_ = appearance;
    invalidate(Dirty::paint);
}

void ProgressBar::set_animation_enabled(bool enabled_value) {
    require_mutable();
    if (animation_enabled_ == enabled_value) {
        return;
    }
    animation_enabled_ = enabled_value;
    if (!animation_enabled_) {
        animation_phase_ = 0.0;
    }
    update_animation_registration();
    invalidate(Dirty::paint | Dirty::semantics);
}

void ProgressBar::set_animation_paused(bool paused) {
    MotionPolicy policy = motion_policy_;
    policy.paused = paused;
    set_motion_policy(policy);
}

void ProgressBar::set_reduced_motion(bool reduced) {
    MotionPolicy policy = motion_policy_;
    policy.reduced = reduced;
    set_motion_policy(policy);
}

void ProgressBar::set_motion_policy(bool paused, bool reduced) {
    MotionPolicy policy = motion_policy_;
    policy.paused = paused;
    policy.reduced = reduced;
    set_motion_policy(policy);
}

void ProgressBar::set_motion_policy(MotionPolicy policy) {
    require_mutable();
    if (motion_policy_ == policy) return;
    motion_policy_ = policy;
    update_animation_registration();
    invalidate(Dirty::paint | Dirty::semantics);
}

MotionPolicy ProgressBar::effective_motion_policy() const noexcept {
    MotionPolicy policy = motion_policy_;
    if (window() != nullptr &&
        (*window()).presentation_settings().reduced_motion) {
        policy.reduced = true;
    }
    return policy;
}

void ProgressBar::set_animation_period(FrameInterval period) {
    require_mutable();
    if (period < std::chrono::milliseconds(100)) {
        throw std::invalid_argument(
            "progress animation period must be at least 100 milliseconds");
    }
    if (animation_period_ == period) {
        return;
    }
    animation_period_ = period;
    last_animation_frame_ = FrameClock::now();
    invalidate(Dirty::paint | Dirty::semantics);
}

bool ProgressBar::animated_style() const noexcept {
    return visual_style_ == ProgressBarVisualStyle::marquee ||
           visual_style_ == ProgressBarVisualStyle::pulse ||
           visual_style_ == ProgressBarVisualStyle::marching_stripes ||
           visual_style_ == ProgressBarVisualStyle::laser_etch ||
           overlay_style_ == ProgressBarOverlayStyle::moving_stripes;
}

void ProgressBar::update_animation_registration() {
    animation_frames_.disconnect();
    const MotionPolicy policy = effective_motion_policy();
    if (window() == nullptr || !animation_enabled_ || !policy.active() ||
        !animated_style()) {
        return;
    }
    const FrameInterval interval = policy.frame_interval(
        std::chrono::milliseconds(16));
    last_animation_frame_ = FrameClock::now();
    animation_frames_ = (*window()).activate_surface(
        shared_from_this(), interval, last_animation_frame_ + interval);
}

void ProgressBar::on_attached_to_window() {
    RangeControl::on_attached_to_window();
    if (window() != nullptr) {
        presentation_subscription_ = (*window()).presentation_changed().subscribe(
            *this,
            Delegate<const PresentationSettings&>::bind<
                ProgressBar, &ProgressBar::on_presentation_changed>(*this));
    }
    update_animation_registration();
}

void ProgressBar::on_presentation_changed(const PresentationSettings&) {
    update_animation_registration();
    invalidate(Dirty::paint | Dirty::semantics);
}

void ProgressBar::on_detached_from_window() noexcept {
    presentation_subscription_.disconnect();
    animation_frames_.disconnect();
    RangeControl::on_detached_from_window();
}

void ProgressBar::on_frame(FrameTime now) {
    const MotionPolicy policy = effective_motion_policy();
    if (!animation_enabled_ || !policy.active() ||
        !animated_style()) {
        return;
    }
    const double elapsed = std::max(
        0.0, std::chrono::duration<double>(now - last_animation_frame_).count());
    last_animation_frame_ = now;
    const double period =
        std::chrono::duration<double>(animation_period_).count();
    animation_phase_ = period <= 0.0
        ? 0.0
        : std::fmod(animation_phase_ +
                        elapsed * policy.speed_scale() / period,
                    1.0);
    if (animation_phase_ < 0.0) {
        animation_phase_ += 1.0;
    }
}

Size ProgressBar::measure(Size available) {
    const Rect requested = requested_bounds();
    const Size preferred = orientation() == Orientation::horizontal
        ? Size{requested.width > 0.0 ? requested.width : 160.0,
               requested.height > 0.0 ? requested.height : 20.0}
        : Size{requested.width > 0.0 ? requested.width : 20.0,
               requested.height > 0.0 ? requested.height : 160.0};
    return {std::min(available.width, preferred.width),
            std::min(available.height, preferred.height)};
}

void ProgressBar::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    const ControlVisualRecipe& track_recipe = effective_theme().resolve(
        ControlVisualRole::progress, visual_context());
    paint_surface_material(painter, bounds, track_recipe.material);
    const double ratio = normalized_value();
    Rect fill{2.0, 2.0, std::max(0.0, bounds.width - 4.0),
              std::max(0.0, bounds.height - 4.0)};
    const Rect interior = fill;
    const MotionPolicy policy = effective_motion_policy();
    const double presented_phase = policy.presentation_phase(animation_phase_);
    if (visual_style_ == ProgressBarVisualStyle::marquee) {
        if (orientation() == Orientation::horizontal) {
            const double band = std::max(18.0, fill.width * 0.28);
            fill.x += (fill.width + band) * presented_phase - band;
            fill.width = band;
            fill = Rect::intersection(fill, {2.0, 2.0,
                                             std::max(0.0, bounds.width - 4.0),
                                             std::max(0.0, bounds.height - 4.0)});
        } else {
            const double band = std::max(18.0, fill.height * 0.28);
            fill.y += (fill.height + band) * (1.0 - presented_phase) - band;
            fill.height = band;
            fill = Rect::intersection(fill, {2.0, 2.0,
                                             std::max(0.0, bounds.width - 4.0),
                                             std::max(0.0, bounds.height - 4.0)});
        }
    } else {
        if (orientation() == Orientation::horizontal) {
            fill.width *= ratio;
        } else {
            const double filled_height = fill.height * ratio;
            fill.y += fill.height - filled_height;
            fill.height = filled_height;
        }
    }

    const ControlVisualRecipe& fill_recipe = effective_theme().resolve(
        ControlVisualRole::progress,
        visual_context(false, false, true, false));
    if (visual_style_ == ProgressBarVisualStyle::blocks) {
        constexpr double gap = 2.0;
        constexpr double block = 9.0;
        if (orientation() == Orientation::horizontal) {
            for (double x = fill.x; x + block <= fill.x + fill.width; x += block + gap) {
                paint_surface_material(
                    painter, {x, fill.y, block, fill.height},
                    fill_recipe.material);
            }
        } else {
            for (double y = fill.y + fill.height - block; y >= fill.y; y -= block + gap) {
                paint_surface_material(
                    painter, {fill.x, y, fill.width, block},
                    fill_recipe.material);
            }
        }
    } else {
        paint_surface_material(painter, fill, fill_recipe.material);
    }
    if (fill.width > 0.0 && fill.height > 0.0) {
        if (visual_style_ == ProgressBarVisualStyle::pulse) {
            paint_progress_luminance(
                painter, fill, animation_appearance_.luminance_color,
                animation_appearance_.pulse_extent, presented_phase);
        } else if (visual_style_ == ProgressBarVisualStyle::marching_stripes) {
            paint_progress_stripes(
                painter, fill, animation_appearance_.stripe_color,
                stripe_width_, animation_enabled_ ? presented_phase : 0.0);
        } else if (visual_style_ == ProgressBarVisualStyle::laser_etch) {
            paint_progress_laser(
                painter, fill, interior, orientation(), animation_appearance_,
                presented_phase, policy.reduced);
        } else if (fill.height > 3.0) {
            painter.fill_rect({fill.x, fill.y, fill.width, 3.0},
                              fill_recipe.muted_text);
        }
    }
    if (overlay_style_ == ProgressBarOverlayStyle::moving_stripes &&
        visual_style_ != ProgressBarVisualStyle::marching_stripes &&
        fill.width > 0.0 && fill.height > 0.0) {
        paint_progress_stripes(
            painter, fill, animation_appearance_.stripe_color, stripe_width_,
            animation_enabled_ ? presented_phase : 0.0);
    }
}

bool ProgressBar::hit_test_local(Point) const {
    return false;
}

SemanticDescriptor ProgressBar::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::progress_bar;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    std::ostringstream value_text;
    value_text << value();
    descriptor.value = value_text.str();
    descriptor.numeric_value = value();
    descriptor.minimum_value = minimum();
    descriptor.maximum_value = maximum();
    if (animation_enabled_ && effective_motion_policy().active() &&
        animated_style()) {
        descriptor.states |= SemanticState::busy;
    }
    descriptor.exposed = true;
    return descriptor;
}

} // namespace gui_forms
