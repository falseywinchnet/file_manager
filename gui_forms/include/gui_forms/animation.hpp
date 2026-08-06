#pragma once

#include "gui_forms/scheduler.hpp"

#include <cstdint>

namespace gui_forms {

enum class EasingCurve : std::uint8_t {
    linear,
    ease_in,
    ease_out,
    ease_in_out,
    smooth_step,
    back_out,
    bounce_out,
    elastic_out,
};

enum class AnimationDirection : std::uint8_t {
    normal,
    reverse,
    alternate,
    alternate_reverse,
};

struct AnimationSpec final {
    FrameInterval delay{};
    FrameInterval duration{std::chrono::milliseconds(300)};
    double iterations{1.0};
    EasingCurve easing{EasingCurve::linear};
    AnimationDirection direction{AnimationDirection::normal};
    bool infinite{};
};

struct AnimationSample final {
    double progress{};
    double eased_progress{};
    std::uint64_t iteration{};
    bool delayed{};
    bool active{};
    bool finished{};
};

[[nodiscard]] double apply_easing(EasingCurve easing, double progress) noexcept;

class AnimationTimeline final {
public:
    AnimationTimeline() = default;
    explicit AnimationTimeline(AnimationSpec specification);

    void set_specification(AnimationSpec specification);
    [[nodiscard]] const AnimationSpec& specification() const noexcept {
        return specification_;
    }
    void start(FrameTime start_time) noexcept;
    void stop() noexcept;
    [[nodiscard]] bool running() const noexcept { return running_; }
    [[nodiscard]] AnimationSample sample(FrameTime now) const noexcept;

private:
    AnimationSpec specification_{};
    FrameTime start_time_{};
    bool running_{};
};

} // namespace gui_forms
