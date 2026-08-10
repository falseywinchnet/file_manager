#pragma once

#include "gui_forms/animation.hpp"
#include "gui_forms/basic_controls.hpp"

#include <string>
#include <vector>

namespace gui_forms {

struct PresentationSettings;

struct EasingPreviewTrack final {
    EasingCurve curve{EasingCurve::linear};
    std::string label;
    Color color{};

    friend bool operator==(const EasingPreviewTrack& left,
                           const EasingPreviewTrack& right) noexcept(noexcept(
        left.curve == right.curve && left.label == right.label &&
        left.color == right.color)) {
        return left.curve == right.curve && left.label == right.label &&
               left.color == right.color;
    }
};

// A retained animation/easing conformance surface.
class EasingPreview final : public Control {
public:
    explicit EasingPreview(StableId stable_id);

    [[nodiscard]] const std::string& title() const noexcept { return title_; }
    void set_title(std::string title);
    [[nodiscard]] const BasicControlStyle& style() const noexcept {
        return style_;
    }
    void set_style(BasicControlStyle style);
    [[nodiscard]] const AnimationSpec& specification() const noexcept {
        return timeline_.specification();
    }
    void set_specification(AnimationSpec specification);
    [[nodiscard]] const std::vector<EasingPreviewTrack>& tracks() const noexcept {
        return tracks_;
    }
    void set_tracks(std::vector<EasingPreviewTrack> tracks);
    [[nodiscard]] MotionPolicy motion_policy() const noexcept {
        return motion_policy_;
    }
    [[nodiscard]] MotionPolicy effective_motion_policy() const noexcept;
    void set_motion_policy(MotionPolicy policy);
    [[nodiscard]] double marker_size() const noexcept { return marker_size_; }
    void set_marker_size(double size);
    [[nodiscard]] double phase() const noexcept { return phase_; }

    void on_frame(FrameTime now) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] bool hit_test_local(Point local_point) const override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

protected:
    void on_attached_to_window() override;
    void on_detached_from_window() noexcept override;

private:
    void register_frames();
    void on_presentation_changed(const PresentationSettings& settings);
    [[nodiscard]] std::string motion_readout(double presented_phase) const;

    FrameRequestToken frames_;
    SubscriptionToken presentation_subscription_;
    AnimationTimeline timeline_;
    std::vector<EasingPreviewTrack> tracks_;
    std::string title_{"Animation timeline and easing"};
    BasicControlStyle style_;
    MotionPolicy motion_policy_{};
    double marker_size_{14.0};
    double phase_{};
    bool timeline_started_{};
};

} // namespace gui_forms
