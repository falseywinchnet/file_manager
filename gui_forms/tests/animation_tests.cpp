#include "gui_forms/animation.hpp"
#include "gui_forms/basic_controls.hpp"
#include "gui_forms/range_controls.hpp"
#include "gui_forms/window.hpp"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {

using namespace gui_forms;
using namespace std::chrono_literals;

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

bool close_to(double left, double right, double tolerance = 0.000001) {
    return std::abs(left - right) <= tolerance;
}

void test_easing_endpoints_and_curves() {
    for (const EasingCurve curve : {
             EasingCurve::linear, EasingCurve::ease_in,
             EasingCurve::ease_out, EasingCurve::ease_in_out,
             EasingCurve::smooth_step, EasingCurve::back_out,
             EasingCurve::bounce_out, EasingCurve::elastic_out}) {
        require(close_to(apply_easing(curve, 0.0), 0.0) &&
                    close_to(apply_easing(curve, 1.0), 1.0),
                "every easing curve must preserve exact endpoints");
    }
    require(apply_easing(EasingCurve::ease_in, 0.5) < 0.5 &&
                apply_easing(EasingCurve::ease_out, 0.5) > 0.5 &&
                close_to(apply_easing(EasingCurve::smooth_step, 0.5), 0.5),
            "easing families must retain their declared timing character");
}

void test_timeline_delay_repeat_direction_and_finish() {
    AnimationSpec specification;
    specification.delay = 50ms;
    specification.duration = 100ms;
    specification.iterations = 2.0;
    specification.easing = EasingCurve::linear;
    specification.direction = AnimationDirection::alternate;
    AnimationTimeline timeline(specification);
    const FrameTime start{};
    timeline.start(start);

    const AnimationSample delayed = timeline.sample(start + 25ms);
    const AnimationSample first = timeline.sample(start + 100ms);
    const AnimationSample second = timeline.sample(start + 200ms);
    const AnimationSample finished = timeline.sample(start + 250ms);
    require(delayed.active && delayed.delayed && close_to(delayed.progress, 0.0),
            "timeline delay must preserve the directed initial value");
    require(first.active && first.iteration == 0U &&
                close_to(first.progress, 0.5),
            "first timeline iteration must advance normally");
    require(second.active && second.iteration == 1U &&
                close_to(second.progress, 0.5),
            "alternate timeline iteration must reverse deterministically");
    require(finished.finished && !finished.active &&
                close_to(finished.progress, 0.0),
            "finite alternate timeline must finish at its directed endpoint");
}

void test_invalid_timeline_contract() {
    bool duration_rejected = false;
    bool iterations_rejected = false;
    try {
        AnimationSpec invalid;
        invalid.duration = FrameInterval::zero();
        AnimationTimeline timeline(invalid);
        static_cast<void>(timeline);
    } catch (const std::invalid_argument&) {
        duration_rejected = true;
    }
    try {
        AnimationSpec invalid;
        invalid.iterations = 0.0;
        AnimationTimeline timeline(invalid);
        static_cast<void>(timeline);
    } catch (const std::invalid_argument&) {
        iterations_rejected = true;
    }
    require(duration_rejected && iterations_rejected,
            "timeline must reject degenerate duration and finite iteration count");
}

void test_progress_animation_and_hidden_suspension() {
    auto root = make_control<Panel>(StableId("animation.root"));
    auto progress = make_control<ProgressBar>(StableId("animation.progress"));
    progress->set_requested_bounds({0.0, 0.0, 240.0, 20.0});
    progress->set_visual_style(ProgressBarVisualStyle::marquee);
    progress->set_animation_period(1000ms);
    root->add_child(progress);
    Window window(root, {240.0, 20.0});
    require(window.next_wake().has_value(),
            "animated progress style must register a retained frame source");

    const FrameTime origin = FrameClock::now();
    progress->on_frame(origin);
    progress->on_frame(origin + 250ms);
    require(progress->animation_phase() > 0.24 &&
                progress->animation_phase() < 0.26,
            "progress animation phase must follow its declared period");

    progress->set_visible(false);
    require(!window.next_wake().has_value(),
            "hidden active surfaces must not keep the host wake loop alive");
    progress->set_visible(true);
    require(window.next_wake().has_value(),
            "reshown active surface must become schedulable without reconstruction");
    progress->set_visual_style(ProgressBarVisualStyle::continuous);
    require(!window.next_wake().has_value(),
            "static progress style must revoke its retained frame source");
}

void test_visual_style_round_trips() {
    auto button = make_control<Button>(StableId("animation.button"), "Button");
    auto check = make_control<CheckBox>(StableId("animation.check"), "Check");
    auto radio = make_control<RadioButton>(StableId("animation.radio"), "Radio");
    auto slider = make_control<TrackBar>(StableId("animation.slider"));
    button->set_visual_style(ButtonVisualStyle::accent);
    check->set_indicator_style(ChoiceIndicatorStyle::toggle);
    radio->set_indicator_style(ChoiceIndicatorStyle::modern);
    slider->set_visual_style(TrackBarVisualStyle::filled);
    require(button->visual_style() == ButtonVisualStyle::accent &&
                check->indicator_style() == ChoiceIndicatorStyle::toggle &&
                radio->indicator_style() == ChoiceIndicatorStyle::modern &&
                slider->visual_style() == TrackBarVisualStyle::filled,
            "public control visual variants must retain exact state");
}

} // namespace

int main() {
    try {
        test_easing_endpoints_and_curves();
        test_timeline_delay_repeat_direction_and_finish();
        test_invalid_timeline_contract();
        test_progress_animation_and_hidden_suspension();
        test_visual_style_round_trips();
        std::cout << "gui_forms_animation_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_animation_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
