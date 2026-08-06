#pragma once

#include "gui_forms/animation.hpp"
#include "gui_forms/basic_controls.hpp"

#include <functional>
#include <string>
#include <vector>

namespace gui_forms {

// Public owner-render surface. Applications configure drawing through the
// renderer-neutral Painter vocabulary without creating a Control subclass.
class DrawingSurface final : public Control {
public:
    using PaintCallback = std::function<void(Painter&, Rect, Rect)>;

    explicit DrawingSurface(StableId stable_id);

    void set_paint_callback(PaintCallback callback);
    [[nodiscard]] bool has_paint_callback() const noexcept {
        return static_cast<bool>(paint_callback_);
    }
    [[nodiscard]] bool hit_test_visible() const noexcept {
        return hit_test_visible_;
    }
    void set_hit_test_visible(bool visible);
    [[nodiscard]] SemanticRole semantic_role() const noexcept {
        return semantic_role_;
    }
    void set_semantic_role(SemanticRole role);

    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] bool hit_test_local(Point local_point) const override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    PaintCallback paint_callback_;
    SemanticRole semantic_role_{SemanticRole::image};
    bool hit_test_visible_{};
};

// Renderer-neutral view over the structured Window metrics snapshot. It is a
// library diagnostic control so demonstrations and host tools do not carry a
// private paint implementation.
class MetricsView final : public Control {
public:
    explicit MetricsView(StableId stable_id, std::string title = "Runtime metrics");

    [[nodiscard]] const std::string& title() const noexcept { return title_; }
    void set_title(std::string title);
    [[nodiscard]] const BasicControlStyle& style() const noexcept { return style_; }
    void set_style(BasicControlStyle style);

    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] bool hit_test_local(Point local_point) const override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    [[nodiscard]] std::string metrics_text() const;
    std::string title_;
    BasicControlStyle style_;
};

struct EasingPreviewTrack final {
    EasingCurve curve{EasingCurve::linear};
    std::string label;
    Color color{};

    friend bool operator==(const EasingPreviewTrack&,
                           const EasingPreviewTrack&) = default;
};

// A retained animation/easing conformance surface. It owns a public timeline,
// frame lease, motion policy, track collection, paint, and semantics; consumers
// only configure it and never need a demonstration-only subclass.
class EasingPreview final : public Control {
public:
    explicit EasingPreview(StableId stable_id);

    [[nodiscard]] const std::string& title() const noexcept { return title_; }
    void set_title(std::string title);
    [[nodiscard]] const BasicControlStyle& style() const noexcept { return style_; }
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
    void set_motion_policy(MotionPolicy policy);
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
    [[nodiscard]] std::string motion_readout(double presented_phase) const;

    FrameRequestToken frames_;
    AnimationTimeline timeline_;
    std::vector<EasingPreviewTrack> tracks_;
    std::string title_{"Animation timeline and easing"};
    BasicControlStyle style_;
    MotionPolicy motion_policy_{};
    double phase_{};
    bool timeline_started_{};
};

} // namespace gui_forms
