#include "showcase_controls.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace gui_forms::showcase {
namespace {

using namespace std::chrono_literals;

constexpr Color shell_blue = Color::rgba(39, 71, 101);
constexpr Color shell_blue_dark = Color::rgba(24, 45, 66);
constexpr Color shell_blue_light = Color::rgba(83, 127, 164);
constexpr Color canvas = Color::rgba(239, 243, 247);
constexpr Color paper = Color::rgba(255, 255, 255);
constexpr Color rule = Color::rgba(166, 179, 191);
constexpr Color ink = Color::rgba(28, 39, 50);
constexpr Color muted = Color::rgba(91, 107, 121);
constexpr Color accent = Color::rgba(39, 116, 184);
constexpr Color green = Color::rgba(49, 139, 94);
constexpr Color orange = Color::rgba(211, 121, 42);
constexpr Color violet = Color::rgba(116, 82, 153);

BasicControlStyle showcase_style() {
    BasicControlStyle style;
    style.face = Color::rgba(226, 232, 238);
    style.face_light = Color::rgba(248, 250, 252);
    style.paper = paper;
    style.highlight = Color::rgba(255, 255, 255);
    style.border = Color::rgba(144, 159, 173);
    style.dark_border = Color::rgba(67, 86, 103);
    style.text = ink;
    style.disabled_text = Color::rgba(133, 143, 152);
    style.accent = accent;
    style.accent_light = Color::rgba(203, 229, 247);
    style.link = Color::rgba(27, 88, 143);
    style.visited_link = violet;
    return style;
}

using LayoutPanel = ScaledPanel;
using LayoutGroup = ScaledGroupBox;

using Surface = ScaledPanel;

using EasingBoard = EasingPreview;

using DiagnosticsCard = MetricsView;
using DrawingEffectsBoard = DrawingSurface;

struct ShowcaseContext final {
    std::vector<Control::Ptr> pages;
    std::vector<std::shared_ptr<Button>> navigation;
    std::vector<SubscriptionToken> subscriptions;
    std::shared_ptr<Label> status;
    std::shared_ptr<EasingBoard> easing;
    std::shared_ptr<Button> motion_pause;
    std::vector<std::shared_ptr<ProgressBar>> animated_progress;
    ComponentContainer components;
    std::shared_ptr<Timer> ui_timer;
    std::shared_ptr<ToolTip> tooltips;
    std::shared_ptr<Label> timer_status;
    std::shared_ptr<ProgressBar> timer_progress;
    std::shared_ptr<TrackBar> timer_interval;
    std::shared_ptr<ScaledPanel> timer_motion;
    std::shared_ptr<Button> timer_motion_target;
    std::shared_ptr<Button> timer_start;
    std::shared_ptr<Button> timer_stop;
    std::shared_ptr<Button> tooltip_show_disabled;
    std::shared_ptr<Button> tooltip_disabled_target;
    std::shared_ptr<Label> dispatcher_status;
    std::shared_ptr<Button> dispatcher_invoke;
    std::vector<DispatchOperation> dispatcher_operations;
    std::thread dispatcher_worker;
    std::string dispatcher_trace;
    std::uint64_t dispatcher_batch{};
    std::shared_ptr<Label> host_services_status;
    std::shared_ptr<TextBox> clipboard_editor;
    std::uint64_t timer_ticks{};
    std::uint64_t next_host_request_id{1U};
    std::size_t selected_page{};
    std::string last_dialog_acceptance{"No accepted dialog value yet"};
    bool live_motion{true};
    bool user_paused{};
    bool reduced_motion{};

    ~ShowcaseContext() {
        if (dispatcher_worker.joinable()) dispatcher_worker.join();
    }

    [[nodiscard]] MotionPolicy motion_policy() const noexcept {
        return {live_motion, user_paused, reduced_motion};
    }

    void update_motion_status() {
        if (!status) return;
        const MotionPolicy policy = motion_policy();
        if (!policy.enabled) {
            status->set_text("Motion disabled · zero animation wakeups");
        } else if (policy.paused && policy.reduced) {
            status->set_text(
                "Reduced motion + paused · phase retained · zero animation wakeups");
        } else if (policy.paused) {
            status->set_text("Motion paused · phase retained · zero animation wakeups");
        } else if (policy.reduced) {
            status->set_text(
                "Reduced motion · low cadence · limited excursion · animation active");
        } else {
            status->set_text("Full motion · continuous retained animation active");
        }
    }

    void apply_motion_policy() {
        const MotionPolicy policy = motion_policy();
        if (easing) easing->set_motion_policy(policy);
        for (const auto& progress : animated_progress) {
            progress->set_motion_policy(policy);
        }
        if (motion_pause) {
            motion_pause->set_enabled(live_motion);
            motion_pause->set_text(!live_motion ? "Motion disabled"
                                                   : user_paused ? "Resume motion"
                                                                 : "Pause motion");
        }
        update_motion_status();
    }

    void select_page(std::size_t index) {
        if (index >= pages.size()) {
            return;
        }
        selected_page = index;
        for (std::size_t page = 0; page < pages.size(); ++page) {
            pages[page]->set_visible(page == selected_page);
            BasicControlStyle style = showcase_style();
            if (page == selected_page) {
                style.face = Color::rgba(198, 222, 241);
                style.face_light = Color::rgba(235, 245, 252);
            }
            navigation[page]->set_style(style);
        }
        if (ui_timer) {
            if (selected_page == 11U) ui_timer->start();
            else if (ui_timer->enabled()) ui_timer->stop();
        }
        static constexpr std::array<std::string_view, 16> names{
            "Control spectrum", "Ranges and progress", "Containers and focus",
            "Animation laboratory", "States and diagnostics", "Text and input",
            "Collections and popups", "Values and spinners",
            "Images and drawing", "Tabs and pages", "Checked collections",
            "Timing and tooltips", "Dates and calendar",
            "Dialogs and host services", "Retained layout panels",
            "Dock and anchor"};
        status->set_text(std::string(names[index]) +
                         "  ·  public GUI.Forms behavior  ·  CPU retained renderer");
    }
};

std::shared_ptr<Label> label(std::string id, std::string text,
                             double size = 12.0, int weight = 400,
                             Color color = ink) {
    auto control = make_control<Label>(StableId(std::move(id)), std::move(text));
    control->set_font({FontRole::content, size,
                       static_cast<std::uint16_t>(weight), false});
    control->set_foreground(color);
    return control;
}

std::shared_ptr<LayoutGroup> group(std::string id, std::string title,
                                   Size design_size) {
    auto control = make_control<LayoutGroup>(StableId(std::move(id)),
                                             std::move(title), design_size);
    control->set_background(paper);
    control->set_style(showcase_style());
    control->set_font({FontRole::control, 12.0, 700, false, 0.24});
    return control;
}

DateTimeFormatProvider french_date_provider() {
    DateTimeFormatProvider provider =
        DateTimeFormatProvider::english_united_states();
    provider.month_names = {
        "janvier", "février", "mars", "avril", "mai", "juin",
        "juillet", "août", "septembre", "octobre", "novembre", "décembre"};
    provider.abbreviated_month_names = {
        "janv.", "févr.", "mars", "avr.", "mai", "juin",
        "juil.", "août", "sept.", "oct.", "nov.", "déc."};
    provider.day_names = {
        "dimanche", "lundi", "mardi", "mercredi", "jeudi", "vendredi", "samedi"};
    provider.abbreviated_day_names = {
        "dim.", "lun.", "mar.", "mer.", "jeu.", "ven.", "sam."};
    provider.long_date_pattern = "dddd d MMMM yyyy";
    provider.short_date_pattern = "dd/MM/yyyy";
    provider.time_pattern = "HH:mm";
    provider.am_designator.clear();
    provider.pm_designator.clear();
    return provider;
}

void add_control_spectrum(const std::shared_ptr<Surface>& page,
                          const std::shared_ptr<ShowcaseContext>& context) {
    page->add_at(label("showcase.controls.heading", "CONTROL SPECTRUM", 22.0, 700,
                       shell_blue_dark), {24.0, 18.0, 500.0, 34.0});
    page->add_at(label("showcase.controls.lead",
                       "Every public stock control, variant, state, and keyboard path lives here.",
                       12.0, 400, muted), {26.0, 52.0, 760.0, 24.0});

    auto button_group = group("showcase.controls.buttons", "Buttons · four public styles",
                              {930.0, 142.0});
    page->add_at(button_group, {24.0, 92.0, 930.0, 142.0});
    const std::array<std::pair<ButtonVisualStyle, std::string_view>, 4> buttons{{
        {ButtonVisualStyle::standard, "Standard"},
        {ButtonVisualStyle::flat, "Flat"},
        {ButtonVisualStyle::accent, "Accent"},
        {ButtonVisualStyle::command, "Command"},
    }};
    for (std::size_t index = 0; index < buttons.size(); ++index) {
        auto button = make_control<Button>(
            StableId("showcase.controls.button." + std::to_string(index)),
            std::string(buttons[index].second));
        button->set_visual_style(buttons[index].first);
        button->set_style(showcase_style());
        button_group->add_at(button,
                             {24.0 + static_cast<double>(index) * 215.0,
                              42.0, 185.0, 42.0});
    }
    auto disabled = make_control<Button>(StableId("showcase.controls.button.disabled"),
                                         "Disabled state");
    disabled->set_enabled(false);
    button_group->add_at(disabled, {24.0, 92.0, 185.0, 30.0});
    auto default_button = make_control<Button>(
        StableId("showcase.controls.button.default"), "Default cue");
    default_button->set_default_button(true);
    button_group->add_at(default_button, {239.0, 92.0, 185.0, 30.0});
    auto link = make_control<LinkLabel>(StableId("showcase.controls.link"),
                                        "LinkLabel · visited on activation");
    button_group->add_at(link, {454.0, 92.0, 260.0, 30.0});

    auto check_group = group("showcase.controls.checks",
                             "Check boxes · classic, modern, toggle", {450.0, 210.0});
    page->add_at(check_group, {24.0, 252.0, 450.0, 210.0});
    for (std::size_t index = 0; index < 3U; ++index) {
        auto check = make_control<CheckBox>(
            StableId("showcase.controls.check." + std::to_string(index)),
            index == 0U ? "Classic checkbox" : index == 1U
                ? "Modern checkbox" : "Toggle checkbox");
        check->set_indicator_style(static_cast<ChoiceIndicatorStyle>(index));
        check->set_checked(index != 0U);
        check_group->add_at(check,
                            {24.0, 42.0 + static_cast<double>(index) * 42.0,
                             300.0, 28.0});
    }
    auto tri = make_control<CheckBox>(StableId("showcase.controls.check.tri"),
                                      "Three-state indeterminate");
    tri->set_three_state(true);
    tri->set_check_state(CheckState::indeterminate);
    check_group->add_at(tri, {24.0, 168.0, 300.0, 28.0});

    auto radio_group = group("showcase.controls.radios",
                             "Radio buttons · three indicator systems", {460.0, 210.0});
    page->add_at(radio_group, {494.0, 252.0, 460.0, 210.0});
    for (std::size_t index = 0; index < 3U; ++index) {
        auto radio = make_control<RadioButton>(
            StableId("showcase.controls.radio." + std::to_string(index)),
            index == 0U ? "Classic radio" : index == 1U
                ? "Modern radio" : "Toggle radio");
        radio->set_indicator_style(static_cast<ChoiceIndicatorStyle>(index));
        radio->set_group_name("showcase-radio-style");
        radio->set_checked(index == 1U);
        radio_group->add_at(radio,
                            {24.0, 42.0 + static_cast<double>(index) * 42.0,
                             310.0, 28.0});
    }

    auto typography = group("showcase.controls.typography", "Labels and typography roles",
                            {930.0, 150.0});
    page->add_at(typography, {24.0, 480.0, 930.0, 150.0});
    auto title = label("showcase.controls.type.title", "Portsmouth Rapids control title",
                       19.0, 700, shell_blue_dark);
    title->set_font({FontRole::control, 19.0, 700, false});
    typography->add_at(title, {24.0, 38.0, 420.0, 30.0});
    auto content = label("showcase.controls.type.content",
        "Carlito body specimen: editable and field text stays quiet, clear, and host-independent.",
                         12.0, 400, ink);
    content->set_text_wrapping(TextWrapping::word);
    content->set_vertical_alignment(VerticalAlignment::near);
    typography->add_at(content, {24.0, 76.0, 430.0, 48.0});
    auto center = label("showcase.controls.type.center", "Centered label", 12.0, 600, accent);
    center->set_alignment(HorizontalAlignment::center);
    typography->add_at(center, {24.0, 118.0, 250.0, 20.0});

    typography->add_at(label("showcase.controls.sound.title",
                             "SEMANTIC SOUND CUES · OPTIONAL HOST", 11.0, 700,
                             shell_blue_dark), {500.0, 34.0, 390.0, 22.0});
    const std::array<std::pair<HostSoundCue, std::string_view>, 4> cues{{
        {HostSoundCue::notification, "Notice"},
        {HostSoundCue::success, "Success"},
        {HostSoundCue::warning, "Warning"},
        {HostSoundCue::error, "Error"},
    }};
    auto sound_enabled = make_control<CheckBox>(
        StableId("showcase.controls.sound.enabled"), "Sound enabled");
    sound_enabled->set_checked(true);
    sound_enabled->set_indicator_style(ChoiceIndicatorStyle::modern);
    typography->add_at(sound_enabled, {500.0, 108.0, 160.0, 26.0});
    for (std::size_t index = 0U; index < cues.size(); ++index) {
        auto cue_button = make_control<Button>(
            StableId("showcase.controls.sound." + std::to_string(index)),
            std::string(cues[index].second));
        cue_button->set_visual_style(index == 3U ? ButtonVisualStyle::standard
                                                : ButtonVisualStyle::command);
        typography->add_at(cue_button,
                           {500.0 + static_cast<double>(index) * 98.0,
                            64.0, 88.0, 32.0});
        const HostSoundCue cue = cues[index].first;
        const std::weak_ptr<CheckBox> weak_enabled = sound_enabled;
        context->subscriptions.push_back(cue_button->clicked().subscribe(
            *typography, [weak_enabled, cue, context](ButtonBase& source) {
                HostServices* services = source.attached_window() == nullptr
                    ? nullptr : source.attached_window()->host_services();
                HostServiceStatus result{HostServiceError::unsupported};
                if (services != nullptr) {
                    const auto now = std::chrono::steady_clock::now()
                        .time_since_epoch();
                    const auto stamp = std::chrono::duration_cast<
                        std::chrono::nanoseconds>(now).count();
                    const auto enabled = weak_enabled.lock();
                    result = services->play_sound_cue(
                        {cue, enabled && enabled->checked() ? 0.72 : 0.0,
                         static_cast<std::uint64_t>(stamp)});
                }
                context->status->set_text(
                    std::string("Sound cue ") + host_sound_cue_name(cue) +
                    " · " + host_service_error_name(result.error) +
                    " · visual meaning remains complete when muted");
            }));
    }
}

void add_ranges(const std::shared_ptr<Surface>& page,
                const std::shared_ptr<ShowcaseContext>& context) {
    page->add_at(label("showcase.ranges.heading", "RANGES AND PROGRESS", 22.0, 700,
                       shell_blue_dark), {24.0, 18.0, 500.0, 34.0});
    page->add_at(label("showcase.ranges.lead",
                       "Live captured dragging, keyboard control, vertical orientation, and retained animation.",
                       12.0, 400, muted), {26.0, 52.0, 850.0, 24.0});

    auto sliders = group("showcase.ranges.sliders", "TrackBar · classic, filled, compact · ScrollBar",
                         {930.0, 230.0});
    page->add_at(sliders, {24.0, 92.0, 930.0, 230.0});
    std::vector<std::shared_ptr<TrackBar>> tracks;
    for (std::size_t index = 0; index < 3U; ++index) {
        auto caption = label("showcase.ranges.slider.caption." + std::to_string(index),
                             index == 0U ? "Classic ticks" : index == 1U
                                 ? "Filled rail" : "Compact rail", 11.0, 600, muted);
        sliders->add_at(caption,
                        {24.0, 38.0 + static_cast<double>(index) * 58.0,
                         130.0, 24.0});
        auto track = make_control<TrackBar>(
            StableId("showcase.ranges.slider." + std::to_string(index)));
        track->set_accessible_name(index == 0U ? "Classic ticks"
            : index == 1U ? "Filled rail" : "Compact rail");
        track->set_visual_style(static_cast<TrackBarVisualStyle>(index));
        track->set_value(28.0 + static_cast<double>(index) * 24.0);
        sliders->add_at(track,
                        {155.0, 35.0 + static_cast<double>(index) * 58.0,
                         690.0, 32.0});
        tracks.push_back(track);
    }
    auto horizontal_scroll = make_control<HScrollBar>(
        StableId("showcase.ranges.scroll.horizontal"));
    horizontal_scroll->set_accessible_name("Horizontal viewport scrollbar");
    horizontal_scroll->set_range(0.0, 100.0);
    horizontal_scroll->set_large_change(18.0);
    horizontal_scroll->set_value(36.0);
    sliders->add_at(horizontal_scroll, {155.0, 200.0, 690.0, 18.0});
    sliders->add_at(label("showcase.ranges.scroll.caption", "Viewport", 11.0, 600,
                          muted), {24.0, 197.0, 120.0, 22.0});

    auto vertical_scroll = make_control<VScrollBar>(
        StableId("showcase.ranges.scroll.vertical"));
    vertical_scroll->set_accessible_name("Vertical viewport scrollbar");
    vertical_scroll->set_range(0.0, 100.0);
    vertical_scroll->set_large_change(18.0);
    vertical_scroll->set_value(36.0);
    sliders->add_at(vertical_scroll, {874.0, 38.0, 18.0, 160.0});
    const std::weak_ptr<VScrollBar> weak_vertical_scroll = vertical_scroll;
    context->subscriptions.push_back(horizontal_scroll->value_changed().subscribe(
        *vertical_scroll, [weak_vertical_scroll](double value) {
            if (const auto vertical = weak_vertical_scroll.lock();
                vertical && vertical->value() != value) {
                vertical->set_value(value);
            }
        }));
    const std::weak_ptr<HScrollBar> weak_horizontal_scroll = horizontal_scroll;
    context->subscriptions.push_back(vertical_scroll->value_changed().subscribe(
        *horizontal_scroll, [weak_horizontal_scroll](double value) {
            if (const auto horizontal = weak_horizontal_scroll.lock();
                horizontal && horizontal->value() != value) {
                horizontal->set_value(value);
            }
        }));

    auto progress_group = group("showcase.ranges.progress",
                                "ProgressBar · blocks, continuous, marquee, pulse, moving stripes",
                                {930.0, 280.0});
    page->add_at(progress_group, {24.0, 340.0, 930.0, 280.0});
    for (std::size_t index = 0; index < 5U; ++index) {
        auto caption = label("showcase.ranges.progress.caption." + std::to_string(index),
                             index == 0U ? "Blocks" : index == 1U ? "Continuous"
                                 : index == 2U ? "Marquee" : index == 3U ? "Pulse"
                                 : "Moving stripes", 11.0, 600, muted);
        progress_group->add_at(caption,
                               {24.0, 36.0 + static_cast<double>(index) * 45.0,
                                110.0, 24.0});
        auto progress = make_control<ProgressBar>(
            StableId("showcase.ranges.progress." + std::to_string(index)));
        progress->set_accessible_name(index == 0U ? "Blocks progress"
            : index == 1U ? "Continuous progress"
            : index == 2U ? "Marquee progress" : index == 3U ? "Pulse progress"
            : "Moving striped progress");
        progress->set_visual_style(index < 4U
            ? static_cast<ProgressBarVisualStyle>(index)
            : ProgressBarVisualStyle::continuous);
        if (index == 4U) {
            progress->set_overlay_style(ProgressBarOverlayStyle::moving_stripes);
            progress->set_animation_period(std::chrono::milliseconds(750));
        }
        progress->set_value(64.0);
        progress_group->add_at(progress,
                               {155.0, 38.0 + static_cast<double>(index) * 45.0,
                                700.0, 24.0});
        if (index >= 2U) {
            context->animated_progress.push_back(progress);
        }
        if (index < tracks.size()) {
            context->subscriptions.push_back(tracks[index]->value_changed().subscribe(
                *progress, [progress](double value) { progress->set_value(value); }));
        }
    }
}

void add_containers(const std::shared_ptr<Surface>& page,
                    const std::shared_ptr<ShowcaseContext>& context) {
    page->add_at(label("showcase.containers.heading", "CONTAINERS AND FOCUS", 22.0, 700,
                       shell_blue_dark), {24.0, 18.0, 500.0, 34.0});
    page->add_at(label("showcase.containers.lead",
                       "Real retained composition: split allocation, wide hit target, collapse, focus transfer, nested scope.",
                       12.0, 400, muted), {26.0, 52.0, 900.0, 24.0});

    auto split = make_control<SplitContainer>(StableId("showcase.containers.split"));
    split->set_splitter_distance(310.0);
    split->set_first_minimum(180.0);
    split->set_second_minimum(280.0);
    page->add_at(split, {24.0, 100.0, 930.0, 430.0});
    split->first_panel()->set_background(Color::rgba(225, 235, 243));
    split->second_panel()->set_background(paper);

    auto first_title = label("showcase.containers.first.title", "NAVIGATION PANE", 13.0, 700,
                             shell_blue_dark);
    first_title->set_requested_bounds({20.0, 22.0, 240.0, 28.0});
    split->first_panel()->add_child(first_title);
    for (std::size_t index = 0; index < 6U; ++index) {
        auto item = make_control<Button>(
            StableId("showcase.containers.item." + std::to_string(index)),
            "Retained item " + std::to_string(index + 1U));
        item->set_visual_style(ButtonVisualStyle::command);
        item->set_requested_bounds({20.0, 62.0 + static_cast<double>(index) * 48.0,
                                    255.0, 36.0});
        split->first_panel()->add_child(item);
    }
    auto second_title = label("showcase.containers.second.title", "CONTENT PANE", 13.0, 700,
                              shell_blue_dark);
    second_title->set_requested_bounds({24.0, 22.0, 260.0, 28.0});
    split->second_panel()->add_child(second_title);
    auto explanation = label("showcase.containers.second.copy",
                             "Drag the seam continuously. Focus it and use arrow keys.\nCollapse and restore preserve allocation.",
                             12.0, 400, ink);
    explanation->set_text_wrapping(TextWrapping::word);
    explanation->set_vertical_alignment(VerticalAlignment::near);
    explanation->set_requested_bounds({24.0, 62.0, 520.0, 52.0});
    split->second_panel()->add_child(explanation);
    auto collapse = make_control<Button>(StableId("showcase.containers.collapse"),
                                         "Collapse navigation");
    collapse->set_requested_bounds({24.0, 132.0, 190.0, 34.0});
    split->second_panel()->add_child(collapse);
    const std::weak_ptr<SplitContainer> weak_split = split;
    const std::weak_ptr<Button> weak_collapse = collapse;
    context->subscriptions.push_back(collapse->clicked().subscribe(
        *split, [weak_split, weak_collapse](ButtonBase&) {
            const auto split = weak_split.lock();
            const auto collapse = weak_collapse.lock();
            if (!split || !collapse) {
                return;
            }
            const bool next = !split->first_collapsed();
            split->set_first_collapsed(next);
            collapse->set_text(next ? "Restore navigation" : "Collapse navigation");
        }));
    auto focus_button = make_control<Button>(StableId("showcase.containers.focus"),
                                             "Focus the splitter");
    focus_button->set_visual_style(ButtonVisualStyle::accent);
    focus_button->set_requested_bounds({228.0, 132.0, 170.0, 34.0});
    split->second_panel()->add_child(focus_button);
    context->subscriptions.push_back(focus_button->clicked().subscribe(
        *split, [weak_split](ButtonBase&) {
            if (const auto split = weak_split.lock()) {
                static_cast<void>(
                    split->request_active_control(split->splitter_control()));
            }
        }));

    auto base_container = make_control<ContainerControl>(
        StableId("showcase.containers.base"));
    base_container->set_accessible_name("Base ContainerControl specimen");
    page->add_at(base_container, {24.0, 548.0, 450.0, 82.0});
    auto base_face = make_control<Panel>(
        StableId("showcase.containers.base.face"));
    base_face->set_background(Color::rgba(232, 239, 245));
    base_face->set_border_style(BorderStyle::line);
    base_face->set_dock(DockStyle::fill);
    base_container->add_child(base_face);
    auto base_copy = label(
        "showcase.containers.base.copy",
        "ContainerControl · active descendant + retained child composition",
        11.0, 600, shell_blue_dark);
    base_copy->set_requested_bounds({14.0, 12.0, 390.0, 24.0});
    base_face->add_child(base_copy);
    auto base_focus = make_control<Button>(
        StableId("showcase.containers.base.focus"), "Focus descendant");
    base_focus->set_requested_bounds({14.0, 42.0, 150.0, 28.0});
    base_face->add_child(base_focus);
    const std::weak_ptr<ContainerControl> weak_base = base_container;
    const std::weak_ptr<Button> weak_base_focus = base_focus;
    context->subscriptions.push_back(base_focus->clicked().subscribe(
        *base_container, [weak_base, weak_base_focus](ButtonBase&) {
            const auto container = weak_base.lock();
            const auto focus = weak_base_focus.lock();
            if (container && focus) {
                static_cast<void>(container->request_active_control(focus));
            }
        }));

    auto user_control = make_control<UserControl>(
        StableId("showcase.containers.user"));
    user_control->set_accessible_name("UserControl lifecycle specimen");
    page->add_at(user_control, {488.0, 548.0, 466.0, 82.0});
    auto user_face = make_control<Panel>(
        StableId("showcase.containers.user.face"));
    user_face->set_background(Color::rgba(238, 244, 236));
    user_face->set_border_style(BorderStyle::line);
    user_face->set_dock(DockStyle::fill);
    user_control->add_child(user_face);
    auto user_status = label(
        "showcase.containers.user.status",
        "UserControl · awaiting one-shot Loaded lifecycle",
        11.0, 600, green);
    user_status->set_requested_bounds({14.0, 12.0, 420.0, 24.0});
    user_face->add_child(user_status);
    auto user_action = make_control<Button>(
        StableId("showcase.containers.user.action"), "Reusable child action");
    user_action->set_visual_style(ButtonVisualStyle::command);
    user_action->set_requested_bounds({14.0, 42.0, 178.0, 28.0});
    user_face->add_child(user_action);
    const std::weak_ptr<Label> weak_user_status = user_status;
    context->subscriptions.push_back(user_control->loaded().subscribe(
        *user_control, [weak_user_status] {
            if (const auto status = weak_user_status.lock()) {
                status->set_text(
                    "UserControl · Loaded fired once · attachment committed");
            }
        }));
}

void add_layout_panels(const std::shared_ptr<Surface>& page,
                       const std::shared_ptr<ShowcaseContext>& context) {
    page->add_at(label("showcase.layout.heading", "RETAINED LAYOUT PANELS", 22.0,
                       700, shell_blue_dark), {24.0, 18.0, 620.0, 34.0});
    page->add_at(label(
                     "showcase.layout.lead",
                     "Preferred bounds remain authored data while flow and table containers assign independent retained slots.",
                     12.0, 400, muted),
                 {26.0, 52.0, 920.0, 24.0});

    auto flow_group = group(
        "showcase.layout.flow.group",
        "FlowLayoutPanel · four directions · wrap · physical margins · FlowBreak",
        {930.0, 218.0});
    page->add_at(flow_group, {24.0, 92.0, 930.0, 218.0});
    constexpr std::array<std::pair<FlowDirection, std::string_view>, 4> directions{{
        {FlowDirection::left_to_right, "Left to right + break"},
        {FlowDirection::right_to_left, "Right to left"},
        {FlowDirection::top_down, "Top down"},
        {FlowDirection::bottom_up, "Bottom up"},
    }};
    for (std::size_t index = 0U; index < directions.size(); ++index) {
        auto card = group(
            "showcase.layout.flow.card." + std::to_string(index),
            std::string(directions[index].second), {210.0, 156.0});
        flow_group->add_at(card,
                           {18.0 + static_cast<double>(index) * 224.0,
                            40.0, 210.0, 156.0});
        auto flow = make_control<FlowLayoutPanel>(
            StableId("showcase.layout.flow." + std::to_string(index)));
        flow->set_accessible_name(std::string(directions[index].second) +
                                  " flow layout");
        flow->set_flow_direction(directions[index].first);
        flow->set_padding({5.0, 5.0, 5.0, 5.0});
        card->add_at(flow, {6.0, 26.0, 198.0, 122.0});
        std::shared_ptr<Button> break_item;
        for (std::size_t item_index = 0U; item_index < 5U; ++item_index) {
            auto item = make_control<Button>(
                StableId("showcase.layout.flow." + std::to_string(index) +
                         ".item." + std::to_string(item_index)),
                std::string(1U, static_cast<char>('A' + item_index)));
            item->set_visual_style(item_index % 2U == 0U
                                       ? ButtonVisualStyle::command
                                       : ButtonVisualStyle::standard);
            item->set_requested_bounds(
                {0.0, 0.0,
                 index < 2U ? 48.0 + static_cast<double>(item_index % 2U) * 10.0
                            : 52.0,
                 index < 2U ? 26.0 : 19.0});
            item->set_margin({3.0, 2.0, 3.0, 2.0});
            flow->add_child(item);
            if (item_index == 1U) break_item = item;
        }
        if (index == 0U && break_item) flow->set_flow_break(*break_item, true);
    }

    auto table_group = group(
        "showcase.layout.table.group",
        "TableLayoutPanel · absolute + auto + weighted percent tracks · spans + automatic placement",
        {930.0, 292.0});
    page->add_at(table_group, {24.0, 330.0, 930.0, 292.0});
    auto table = make_control<TableLayoutPanel>(
        StableId("showcase.layout.table"));
    table->set_accessible_name("Mixed-track table layout specimen");
    table->set_padding({6.0, 6.0, 6.0, 6.0});
    table->set_column_count(4U);
    table->set_row_count(3U);
    table->set_column_style(0U, {TableSizeMode::absolute, 140.0});
    table->set_column_style(1U, {TableSizeMode::auto_size, 0.0});
    table->set_column_style(2U, {TableSizeMode::percent, 1.0});
    table->set_column_style(3U, {TableSizeMode::percent, 2.0});
    table->set_row_style(0U, {TableSizeMode::absolute, 42.0});
    table->set_row_style(1U, {TableSizeMode::auto_size, 0.0});
    table->set_row_style(2U, {TableSizeMode::percent, 1.0});
    table->set_cell_border_style(TableCellBorderStyle::inset);
    table_group->add_at(table, {18.0, 38.0, 894.0, 218.0});

    auto table_header = label(
        "showcase.layout.table.header",
        "AUTHORED PREFERRED SIZE  →  RETAINED CELL SLOT  →  COMMITTED BOUNDS",
        12.0, 700, shell_blue_dark);
    table_header->set_requested_bounds({0.0, 0.0, 720.0, 26.0});
    table_header->set_margin({8.0, 7.0, 8.0, 5.0});
    table->add_child(table_header);
    table->set_cell_position(*table_header, {0U, 0U});
    table->set_column_span(*table_header, 4U);

    auto absolute = label("showcase.layout.table.absolute", "Absolute · 140 px",
                          11.0, 600, ink);
    absolute->set_requested_bounds({0.0, 0.0, 112.0, 24.0});
    table->add_child(absolute);
    table->set_cell_position(*absolute, {0U, 1U});

    auto automatic = make_control<Button>(
        StableId("showcase.layout.table.auto"), "Auto content");
    automatic->set_visual_style(ButtonVisualStyle::accent);
    automatic->set_requested_bounds({0.0, 0.0, 128.0, 30.0});
    table->add_child(automatic);
    table->set_cell_position(*automatic, {1U, 1U});

    auto percent_one = make_control<CheckBox>(
        StableId("showcase.layout.table.percent.one"), "Percent · 1 share");
    percent_one->set_checked(true);
    percent_one->set_indicator_style(ChoiceIndicatorStyle::modern);
    percent_one->set_requested_bounds({0.0, 0.0, 140.0, 28.0});
    table->add_child(percent_one);
    table->set_cell_position(*percent_one, {2U, 1U});

    auto percent_two = make_control<TrackBar>(
        StableId("showcase.layout.table.percent.two"));
    percent_two->set_accessible_name("Percent two-share cell slider");
    percent_two->set_visual_style(TrackBarVisualStyle::filled);
    percent_two->set_value(68.0);
    percent_two->set_requested_bounds({0.0, 0.0, 230.0, 30.0});
    table->add_child(percent_two);
    table->set_cell_position(*percent_two, {3U, 1U});

    auto span_note = label(
        "showcase.layout.table.span",
        "ColumnSpan = 2 · lookup covers both cells",
        11.0, 600, green);
    span_note->set_requested_bounds({0.0, 0.0, 260.0, 28.0});
    table->add_child(span_note);
    table->set_cell_position(*span_note, {0U, 2U});
    table->set_column_span(*span_note, 2U);

    auto auto_placed = make_control<Button>(
        StableId("showcase.layout.table.auto.placed"), "Auto-placed cell");
    auto_placed->set_visual_style(ButtonVisualStyle::command);
    auto_placed->set_requested_bounds({0.0, 0.0, 136.0, 30.0});
    table->add_child(auto_placed);

    auto table_progress = make_control<ProgressBar>(
        StableId("showcase.layout.table.progress"));
    table_progress->set_value(76.0);
    table_progress->set_requested_bounds({0.0, 0.0, 230.0, 24.0});
    table->add_child(table_progress);

    auto note = label(
        "showcase.layout.note",
        "Resize proof is framework-owned: authored bounds never become the parent-assigned slot.",
        11.0, 600, green);
    page->add_at(note, {26.0, 624.0, 800.0, 22.0});
    static_cast<void>(context);
}

void add_dock_and_anchor(const std::shared_ptr<Surface>& page,
                         const std::shared_ptr<ShowcaseContext>& context) {
    page->add_at(label("showcase.dock.heading", "DOCK AND ANCHOR", 22.0, 700,
                       shell_blue_dark), {24.0, 18.0, 620.0, 34.0});
    page->add_at(label(
                     "showcase.dock.lead",
                     "Base Control layout owns z-ordered docking, padded client consumption, compound edge anchors, and runtime rebasing.",
                     12.0, 400, muted),
                 {26.0, 52.0, 920.0, 24.0});

    auto dock_group = group(
        "showcase.dock.group",
        "Dock · Left + Top + Top + Right + Bottom + Fill · z-order commands",
        {930.0, 260.0});
    page->add_at(dock_group, {24.0, 92.0, 930.0, 260.0});
    auto dock_canvas = make_control<Panel>(StableId("showcase.dock.canvas"));
    dock_canvas->set_accessible_name("Dock layout specimen");
    dock_canvas->set_background(Color::rgba(238, 243, 247));
    dock_canvas->set_border_style(BorderStyle::sunken);
    dock_canvas->set_padding({8.0, 8.0, 8.0, 8.0});
    dock_group->add_at(dock_canvas, {18.0, 38.0, 610.0, 190.0});

    auto fill = make_control<Button>(StableId("showcase.dock.fill"),
                                     "Fill · remaining client");
    fill->set_visual_style(ButtonVisualStyle::accent);
    fill->set_requested_bounds({0.0, 0.0, 80.0, 30.0});
    fill->set_dock(DockStyle::fill);
    auto bottom = make_control<Button>(StableId("showcase.dock.bottom"),
                                       "Bottom · 26 px");
    bottom->set_requested_bounds({0.0, 0.0, 100.0, 26.0});
    bottom->set_dock(DockStyle::bottom);
    auto right = make_control<Button>(StableId("showcase.dock.right"),
                                      "Right");
    right->set_requested_bounds({0.0, 0.0, 76.0, 30.0});
    right->set_dock(DockStyle::right);
    auto top_second = make_control<Button>(
        StableId("showcase.dock.top.second"), "Top B · z-order");
    top_second->set_visual_style(ButtonVisualStyle::command);
    top_second->set_requested_bounds({0.0, 0.0, 100.0, 25.0});
    top_second->set_dock(DockStyle::top);
    auto top_first = make_control<Button>(
        StableId("showcase.dock.top.first"), "Top A · topmost");
    top_first->set_visual_style(ButtonVisualStyle::command);
    top_first->set_requested_bounds({0.0, 0.0, 100.0, 25.0});
    top_first->set_dock(DockStyle::top);
    auto left = make_control<Button>(StableId("showcase.dock.left"), "Left");
    left->set_requested_bounds({0.0, 0.0, 82.0, 30.0});
    left->set_dock(DockStyle::left);
    dock_canvas->add_child(fill);
    dock_canvas->add_child(bottom);
    dock_canvas->add_child(right);
    dock_canvas->add_child(top_second);
    dock_canvas->add_child(top_first);
    dock_canvas->add_child(left);

    auto toggle_left = make_control<Button>(
        StableId("showcase.dock.toggle-left"), "Hide left edge");
    toggle_left->set_visual_style(ButtonVisualStyle::accent);
    dock_group->add_at(toggle_left, {652.0, 52.0, 236.0, 34.0});
    auto swap_top = make_control<Button>(
        StableId("showcase.dock.swap-top"), "Bring Top B to front");
    dock_group->add_at(swap_top, {652.0, 98.0, 236.0, 34.0});
    auto dock_note = label(
        "showcase.dock.note",
        "Hidden docked controls consume zero extent. SetChildIndex changes edge order without rewriting preferred bounds.",
        11.0, 600, green);
    dock_note->set_text_wrapping(TextWrapping::word);
    dock_note->set_vertical_alignment(VerticalAlignment::near);
    dock_group->add_at(dock_note, {652.0, 148.0, 238.0, 70.0});

    const std::weak_ptr<Button> weak_left = left;
    const std::weak_ptr<Button> weak_toggle = toggle_left;
    const std::weak_ptr<ShowcaseContext> weak_context = context;
    context->subscriptions.push_back(toggle_left->clicked().subscribe(
        *page, [weak_left, weak_toggle, weak_context](ButtonBase&) {
            const auto left = weak_left.lock();
            const auto toggle = weak_toggle.lock();
            if (!left || !toggle) return;
            left->set_visible(!left->visible());
            toggle->set_text(left->visible() ? "Hide left edge"
                                             : "Restore left edge");
            if (const auto context = weak_context.lock()) {
                context->status->set_text(
                    left->visible()
                        ? "Dock · left edge restored · fill contracted"
                        : "Dock · hidden edge released · fill expanded");
            }
        }));
    const std::weak_ptr<Panel> weak_dock_canvas = dock_canvas;
    const std::weak_ptr<Button> weak_top_first = top_first;
    const std::weak_ptr<Button> weak_top_second = top_second;
    const std::weak_ptr<Button> weak_swap = swap_top;
    auto top_b_front = std::make_shared<bool>(false);
    context->subscriptions.push_back(swap_top->clicked().subscribe(
        *page, [weak_dock_canvas, weak_top_first, weak_top_second, weak_swap,
                weak_context, top_b_front](ButtonBase&) {
            const auto canvas = weak_dock_canvas.lock();
            const auto first = weak_top_first.lock();
            const auto second = weak_top_second.lock();
            const auto swap = weak_swap.lock();
            if (!canvas || !first || !second || !swap) return;
            *top_b_front = !*top_b_front;
            static_cast<void>(canvas->set_child_index(
                (*top_b_front ? second : first)->runtime_id(), 0U));
            swap->set_text(*top_b_front ? "Bring Top A to front"
                                       : "Bring Top B to front");
            if (const auto context = weak_context.lock()) {
                context->status->set_text(
                    *top_b_front ? "Dock · Top B now consumes the edge first"
                                 : "Dock · Top A now consumes the edge first");
            }
        }));

    auto anchor_group = group(
        "showcase.anchor.group",
        "Anchor · fixed · stretch · centered · bottom-right · live parent resize",
        {930.0, 250.0});
    page->add_at(anchor_group, {24.0, 374.0, 930.0, 250.0});
    auto anchor_canvas = make_control<Panel>(StableId("showcase.anchor.canvas"));
    anchor_canvas->set_accessible_name("Compound anchor specimen");
    anchor_canvas->set_background(Color::rgba(247, 249, 251));
    anchor_canvas->set_border_style(BorderStyle::line);
    anchor_canvas->set_padding({8.0, 8.0, 8.0, 8.0});
    anchor_group->add_at(anchor_canvas, {18.0, 38.0, 600.0, 170.0});

    auto fixed = make_control<Button>(StableId("showcase.anchor.fixed"),
                                      "Left + Top");
    fixed->set_requested_bounds({18.0, 16.0, 126.0, 28.0});
    auto stretch = make_control<Button>(StableId("showcase.anchor.stretch"),
                                        "Left + Right · stretches");
    stretch->set_visual_style(ButtonVisualStyle::accent);
    stretch->set_requested_bounds({18.0, 56.0, 280.0, 28.0});
    stretch->set_anchor(AnchorStyles::left | AnchorStyles::right |
                        AnchorStyles::top);
    auto centered = make_control<Button>(StableId("showcase.anchor.centered"),
                                         "No edges · centered");
    centered->set_requested_bounds({190.0, 106.0, 152.0, 28.0});
    centered->set_anchor(AnchorStyles::none);
    auto bottom_right = make_control<Button>(
        StableId("showcase.anchor.bottom-right"), "Right + Bottom");
    bottom_right->set_visual_style(ButtonVisualStyle::command);
    bottom_right->set_requested_bounds({424.0, 126.0, 142.0, 28.0});
    bottom_right->set_anchor(AnchorStyles::right | AnchorStyles::bottom);
    anchor_canvas->add_child(fixed);
    anchor_canvas->add_child(stretch);
    anchor_canvas->add_child(centered);
    anchor_canvas->add_child(bottom_right);

    auto resize = make_control<Button>(StableId("showcase.anchor.resize"),
                                       "Expand specimen");
    resize->set_visual_style(ButtonVisualStyle::accent);
    anchor_group->add_at(resize, {758.0, 54.0, 150.0, 34.0});
    auto anchor_note = label(
        "showcase.anchor.note",
        "Runtime size and Anchor changes rebase from committed geometry. Authored bounds remain queryable and unchanged.",
        11.0, 600, green);
    anchor_note->set_text_wrapping(TextWrapping::word);
    anchor_note->set_vertical_alignment(VerticalAlignment::near);
    anchor_group->add_at(anchor_note, {758.0, 104.0, 150.0, 104.0});

    const std::weak_ptr<ScaledGroupBox> weak_anchor_group = anchor_group;
    const std::weak_ptr<Panel> weak_anchor_canvas = anchor_canvas;
    const std::weak_ptr<Button> weak_resize = resize;
    auto expanded = std::make_shared<bool>(false);
    context->subscriptions.push_back(resize->clicked().subscribe(
        *page, [weak_anchor_group, weak_anchor_canvas, weak_resize,
                weak_context, expanded](ButtonBase&) {
            const auto group = weak_anchor_group.lock();
            const auto canvas = weak_anchor_canvas.lock();
            const auto resize = weak_resize.lock();
            if (!group || !canvas || !resize) return;
            *expanded = !*expanded;
            group->set_design_bounds(*canvas,
                *expanded ? Rect{18.0, 38.0, 730.0, 190.0}
                          : Rect{18.0, 38.0, 600.0, 170.0});
            resize->set_text(*expanded ? "Restore specimen"
                                      : "Expand specimen");
            if (const auto context = weak_context.lock()) {
                context->status->set_text(
                    *expanded
                        ? "Anchor · parent expanded · stretch/right/center updated"
                        : "Anchor · parent restored from retained design slot");
            }
        }));
}

void add_animation(const std::shared_ptr<Surface>& page,
                   const std::shared_ptr<ShowcaseContext>& context) {
    page->add_at(label("showcase.animation.heading", "ANIMATION LABORATORY", 22.0, 700,
                       shell_blue_dark), {24.0, 18.0, 520.0, 34.0});
    page->add_at(label("showcase.animation.lead",
                       "Eight public easing curves, alternate direction, retained frame scheduling, hidden-page suspension.",
                       12.0, 400, muted), {26.0, 52.0, 900.0, 24.0});
    auto easing = make_control<EasingBoard>(StableId("showcase.animation.easing"));
    easing->set_title("TIMELINE AND EASING LAB");
    easing->set_style(showcase_style());
    easing->set_tracks({
        {EasingCurve::linear, "Linear", accent},
        {EasingCurve::ease_in, "Ease in", accent},
        {EasingCurve::ease_out, "Ease out", accent},
        {EasingCurve::ease_in_out, "Ease in/out", violet},
        {EasingCurve::smooth_step, "Smooth step", violet},
        {EasingCurve::back_out, "Back out", violet},
        {EasingCurve::bounce_out, "Bounce", orange},
        {EasingCurve::elastic_out, "Elastic", orange},
    });
    page->add_at(easing, {24.0, 92.0, 930.0, 340.0});
    context->easing = easing;

    auto controls = group("showcase.animation.controls",
                          "Animation controls and motion policy", {930.0, 168.0});
    page->add_at(controls, {24.0, 454.0, 930.0, 168.0});
    auto pause = make_control<Button>(StableId("showcase.animation.pause"), "Pause motion");
    pause->set_visual_style(ButtonVisualStyle::accent);
    controls->add_at(pause, {24.0, 42.0, 160.0, 36.0});
    context->motion_pause = pause;
    const std::weak_ptr<ShowcaseContext> weak_context = context;
    context->subscriptions.push_back(pause->clicked().subscribe(
        *easing, [weak_context](ButtonBase&) {
            const auto context = weak_context.lock();
            if (!context || !context->live_motion) return;
            context->user_paused = !context->user_paused;
            context->apply_motion_policy();
        }));
    auto copy = label("showcase.animation.policy",
                      "Animation is deadline-driven and quiescent when hidden or paused. No perpetual redraw loop.",
                      12.0, 400, ink);
    controls->add_at(copy, {210.0, 46.0, 650.0, 28.0});
    auto reduced = make_control<CheckBox>(StableId("showcase.animation.reduced"),
                                          "Reduced-motion substitution");
    reduced->set_indicator_style(ChoiceIndicatorStyle::toggle);
    controls->add_at(reduced, {24.0, 96.0, 310.0, 28.0});
    context->subscriptions.push_back(reduced->checked_changed().subscribe(
        *easing, [weak_context](bool enabled) {
            if (const auto context = weak_context.lock()) {
                context->reduced_motion = enabled;
                context->apply_motion_policy();
            }
        }));
}

void add_states(const std::shared_ptr<Surface>& page,
                const std::shared_ptr<ShowcaseContext>& context) {
    page->add_at(label("showcase.states.heading", "STATES AND DIAGNOSTICS", 22.0, 700,
                       shell_blue_dark), {24.0, 18.0, 560.0, 34.0});
    page->add_at(label("showcase.states.lead",
                       "Focus, disabled, checked, indeterminate, visited, border, active-surface, and runtime accounting.",
                       12.0, 400, muted), {26.0, 52.0, 900.0, 24.0});

    const std::array<std::pair<BorderStyle, std::string_view>, 4> borders{{
        {BorderStyle::none, "No border"}, {BorderStyle::line, "Line"},
        {BorderStyle::sunken, "Sunken"}, {BorderStyle::raised, "Raised"},
    }};
    for (std::size_t index = 0; index < borders.size(); ++index) {
        auto panel = make_control<Panel>(
            StableId("showcase.states.border." + std::to_string(index)));
        panel->set_background(index % 2U == 0U ? paper : Color::rgba(230, 238, 245));
        panel->set_border_style(borders[index].first);
        page->add_at(panel, {24.0 + static_cast<double>(index) * 232.0,
                             98.0, 210.0, 104.0});
        auto caption = label("showcase.states.border.caption." + std::to_string(index),
                             std::string(borders[index].second), 12.0, 600, shell_blue_dark);
        caption->set_requested_bounds({16.0, 18.0, 170.0, 24.0});
        panel->add_child(caption);
        auto state = label("showcase.states.border.state." + std::to_string(index),
                           "Retained panel", 11.0, 400, muted);
        state->set_requested_bounds({16.0, 52.0, 170.0, 24.0});
        panel->add_child(state);
    }

    auto lifecycle = group("showcase.states.lifecycle", "Lifecycle and interaction states",
                           {450.0, 240.0});
    page->add_at(lifecycle, {24.0, 226.0, 450.0, 240.0});
    auto checked = make_control<CheckBox>(StableId("showcase.states.checked"), "Checked");
    checked->set_checked(true);
    lifecycle->add_at(checked, {24.0, 42.0, 180.0, 28.0});
    auto indeterminate = make_control<CheckBox>(StableId("showcase.states.indeterminate"),
                                                "Indeterminate");
    indeterminate->set_three_state(true);
    indeterminate->set_check_state(CheckState::indeterminate);
    lifecycle->add_at(indeterminate, {24.0, 82.0, 180.0, 28.0});
    auto unavailable = make_control<Button>(StableId("showcase.states.unavailable"),
                                            "Unavailable action");
    unavailable->set_enabled(false);
    lifecycle->add_at(unavailable, {230.0, 42.0, 180.0, 34.0});
    auto visited = make_control<LinkLabel>(StableId("showcase.states.visited"),
                                           "Visited link state");
    visited->set_visited(true);
    lifecycle->add_at(visited, {230.0, 88.0, 180.0, 28.0});
    auto cursor_note = label("showcase.states.cursor",
                             "Cursor roles: arrow · text · hand · resize · wait · forbidden",
                             11.0, 400, muted);
    lifecycle->add_at(cursor_note, {24.0, 142.0, 390.0, 42.0});

    auto diagnostics = make_control<DiagnosticsCard>(StableId("showcase.states.diagnostics"));
    diagnostics->set_title("RETAINED RUNTIME");
    BasicControlStyle diagnostics_style = showcase_style();
    diagnostics_style.text = Color::rgba(28, 39, 50);
    diagnostics_style.face = Color::rgba(175, 203, 221);
    diagnostics_style.face_light = Color::rgba(226, 238, 246);
    diagnostics_style.accent = green;
    diagnostics->set_style(diagnostics_style);
    page->add_at(diagnostics, {494.0, 226.0, 460.0, 240.0});
    auto dispatcher = group(
        "showcase.states.dispatcher", "UI dispatcher · posted, bounded, cancellable",
        {930.0, 134.0});
    page->add_at(dispatcher, {24.0, 488.0, 930.0, 134.0});
    auto post = make_control<Button>(StableId("showcase.dispatcher.post"),
                                     "Post A · B · C → D");
    post->set_visual_style(ButtonVisualStyle::accent);
    auto cancel = make_control<Button>(StableId("showcase.dispatcher.cancel"),
                                       "Post then cancel");
    auto invoke = make_control<Button>(StableId("showcase.dispatcher.invoke"),
                                       "Worker Invoke");
    context->dispatcher_invoke = invoke;
    dispatcher->add_at(post, {22.0, 40.0, 174.0, 36.0});
    dispatcher->add_at(cancel, {208.0, 40.0, 154.0, 36.0});
    dispatcher->add_at(invoke, {374.0, 40.0, 170.0, 36.0});
    context->dispatcher_status = label(
        "showcase.dispatcher.status",
        "Ready · BeginInvoke never executes inside the click callback",
        11.0, 600, green);
    context->dispatcher_status->set_text_wrapping(TextWrapping::word);
    context->dispatcher_status->set_vertical_alignment(VerticalAlignment::near);
    dispatcher->add_at(context->dispatcher_status, {566.0, 36.0, 328.0, 58.0});

    const std::weak_ptr<ShowcaseContext> weak_context = context;
    context->subscriptions.push_back(post->clicked().subscribe(
        *post, [weak_context](ButtonBase& source) {
            const auto context = weak_context.lock();
            if (!context) return;
            ++context->dispatcher_batch;
            context->dispatcher_trace.clear();
            context->dispatcher_operations.clear();
            context->dispatcher_status->set_text(
                "Click returned · batch " +
                std::to_string(context->dispatcher_batch) + " is pending");
            const auto append = [weak_context](char value) {
                if (const auto context = weak_context.lock()) {
                    context->dispatcher_trace.push_back(value);
                }
            };
            context->dispatcher_operations.push_back(source.begin_invoke(
                [append] { append('A'); }));
            context->dispatcher_operations.push_back(source.begin_invoke(
                [weak_context, append] {
                    append('B');
                    if (const auto context = weak_context.lock()) {
                        context->dispatcher_operations.push_back(
                            context->dispatcher_status->begin_invoke(
                                [weak_context, append] {
                                    append('D');
                                    if (const auto state = weak_context.lock()) {
                                        const DispatcherSnapshot snapshot =
                                            state->dispatcher_status
                                                ->attached_window()
                                                ->dispatcher_snapshot();
                                        state->dispatcher_status->set_text(
                                            "Turn 2 committed · order " +
                                            state->dispatcher_trace +
                                            " · invoked " +
                                            std::to_string(snapshot.invoked) +
                                            " · faults " +
                                            std::to_string(snapshot.faulted));
                                    }
                                }));
                    }
                }));
            context->dispatcher_operations.push_back(source.begin_invoke(
                [weak_context, append] {
                    append('C');
                    if (const auto context = weak_context.lock()) {
                        context->dispatcher_status->set_text(
                            "Turn 1 committed · order " +
                            context->dispatcher_trace +
                            " · nested D remains posted");
                    }
                }));
        }));
    context->subscriptions.push_back(cancel->clicked().subscribe(
        *cancel, [weak_context](ButtonBase& source) {
            const auto context = weak_context.lock();
            if (!context) return;
            context->dispatcher_operations.clear();
            DispatchOperation operation = source.begin_invoke([weak_context] {
                if (const auto state = weak_context.lock()) {
                    state->dispatcher_status->set_text(
                        "ERROR · cancelled callback executed");
                }
            });
            const std::uint64_t sequence = operation.sequence();
            const bool cancelled = operation.cancel();
            context->dispatcher_operations.push_back(std::move(operation));
            context->dispatcher_status->set_text(
                "Cancelled posted operation #" + std::to_string(sequence) +
                (cancelled ? " before dispatch" : " too late"));
        }));
    context->subscriptions.push_back(invoke->clicked().subscribe(
        *invoke, [weak_context](ButtonBase&) {
            const auto context = weak_context.lock();
            if (!context || !context->dispatcher_invoke->enabled()) return;
            context->dispatcher_invoke->set_enabled(false);
            context->dispatcher_status->set_text(
                "Worker blocked · no nested pump · waiting for UI turn");
            if (context->dispatcher_worker.joinable()) {
                context->dispatcher_worker.join();
            }
            context->dispatcher_worker = std::thread([weak_context] {
                const auto context = weak_context.lock();
                if (!context) return;
                try {
                    context->dispatcher_status->invoke([weak_context] {
                        if (const auto state = weak_context.lock()) {
                            const DispatcherSnapshot snapshot =
                                state->dispatcher_status->attached_window()
                                    ->dispatcher_snapshot();
                            state->dispatcher_status->set_text(
                                "Worker released after UI callback · marshalled " +
                                std::to_string(snapshot.marshalled_invocations));
                            state->dispatcher_invoke->set_enabled(true);
                        }
                    });
                } catch (const DispatchCancelledError&) {
                    // Window shutdown is the terminal owner of this worker.
                }
            });
        }));
}

void add_text_input(const std::shared_ptr<Surface>& page,
                    const std::shared_ptr<ShowcaseContext>& context) {
    page->add_at(label("showcase.text.heading", "TEXT AND INPUT", 22.0, 700,
                       shell_blue_dark), {24.0, 18.0, 500.0, 34.0});
    page->add_at(label("showcase.text.lead",
                       "Unicode editing, word navigation, host clipboard commands, protected text, captured drag, caret deadlines, and history.",
                       12.0, 400, muted), {26.0, 52.0, 900.0, 24.0});

    auto editing = group("showcase.text.editing", "Editable TextBox states",
                         {930.0, 290.0});
    page->add_at(editing, {24.0, 92.0, 930.0, 290.0});
    editing->add_at(label("showcase.text.primary.caption", "Primary field", 11.0,
                          600, muted), {24.0, 42.0, 130.0, 24.0});
    auto primary = make_control<TextBox>(StableId("showcase.text.primary"));
    primary->set_accessible_name("Primary field");
    primary->set_placeholder_text("Click, type, drag-select, use Shift+Arrow, Cmd/Ctrl+A, Z, Y");
    editing->add_at(primary, {154.0, 36.0, 700.0, 36.0});

    editing->add_at(label("showcase.text.unicode.caption", "Unicode field", 11.0,
                          600, muted), {24.0, 96.0, 130.0, 24.0});
    auto unicode = make_control<TextBox>(StableId("showcase.text.unicode"),
                                         "Graphemes: a\xCC\x81 · Ω · 日本語 · 🚀");
    unicode->set_accessible_name("Unicode field");
    editing->add_at(unicode, {154.0, 90.0, 700.0, 36.0});

    editing->add_at(label("showcase.text.readonly.caption", "Read-only", 11.0,
                          600, muted), {24.0, 150.0, 130.0, 24.0});
    auto read_only = make_control<TextBox>(StableId("showcase.text.readonly"),
                                           "Selection remains available; mutation is blocked.");
    read_only->set_accessible_name("Read-only field");
    read_only->set_read_only(true);
    editing->add_at(read_only, {154.0, 144.0, 700.0, 36.0});

    editing->add_at(label("showcase.text.password.caption", "Protected", 11.0,
                          600, muted), {24.0, 204.0, 130.0, 24.0});
    auto password = make_control<TextBox>(StableId("showcase.text.password"),
                                           "Portsmouth-Ω-2026");
    password->set_accessible_name("Protected credential field");
    password->set_accessible_description(
        "Password text is editable but excluded from paint traces, clipboard export, and semantic values.");
    password->set_use_system_password_character(true);
    editing->add_at(password, {154.0, 198.0, 700.0, 36.0});

    auto state = label("showcase.text.state", "Selection: empty · History: clean",
                       11.0, 600, green);
    editing->add_at(state, {154.0, 250.0, 700.0, 26.0});
    const std::weak_ptr<TextBox> weak_primary = primary;
    const std::weak_ptr<Label> weak_state = state;
    const auto update_state = [weak_primary, weak_state] {
        const auto field = weak_primary.lock();
        const auto state = weak_state.lock();
        if (!field || !state) {
            return;
        }
        const TextSelection selection = field->selection();
        state->set_text("Selection: " + std::to_string(selection.start().value()) +
                        ".." + std::to_string(selection.end().value()) +
                        " bytes · Undo: " + (field->can_undo() ? "ready" : "clean") +
                        " · Redo: " + (field->can_redo() ? "ready" : "clean"));
    };
    context->subscriptions.push_back(primary->text_changed().subscribe(
        *state, [update_state](const std::string&) { update_state(); }));
    context->subscriptions.push_back(primary->selection_changed().subscribe(
        *state, [update_state](const TextSelection&) { update_state(); }));

    auto commands = group("showcase.text.commands", "Programmatic editing surface",
                          {930.0, 216.0});
    page->add_at(commands, {24.0, 404.0, 930.0, 216.0});
    const std::array<std::pair<std::size_t, std::string_view>, 7> command_specs{{
        {0U, "Select all"}, {4U, "Copy"}, {5U, "Cut"}, {6U, "Paste"},
        {1U, "Replace"}, {2U, "Undo"}, {3U, "Redo"},
    }};
    std::array<std::shared_ptr<Button>, 7> command_buttons;
    for (std::size_t position = 0; position < command_specs.size(); ++position) {
        const std::size_t index = command_specs[position].first;
        auto button = make_control<Button>(
            StableId("showcase.text.command." + std::to_string(index)),
            std::string(command_specs[position].second));
        button->set_visual_style(index == 1U ? ButtonVisualStyle::accent
                                             : ButtonVisualStyle::standard);
        commands->add_at(button,
            {24.0 + static_cast<double>(position) * 126.0, 42.0, 114.0, 36.0});
        command_buttons[index] = button;
    }
    context->subscriptions.push_back(command_buttons[0]->clicked().subscribe(
        *primary, [weak_primary](ButtonBase&) {
            if (const auto field = weak_primary.lock()) field->select_all();
        }));
    context->subscriptions.push_back(command_buttons[1]->clicked().subscribe(
        *primary, [weak_primary](ButtonBase&) {
            if (const auto field = weak_primary.lock()) {
                static_cast<void>(field->replace_selection("GUI.Forms"));
            }
        }));
    context->subscriptions.push_back(command_buttons[2]->clicked().subscribe(
        *primary, [weak_primary](ButtonBase&) {
            if (const auto field = weak_primary.lock()) static_cast<void>(field->undo());
        }));
    context->subscriptions.push_back(command_buttons[3]->clicked().subscribe(
        *primary, [weak_primary](ButtonBase&) {
            if (const auto field = weak_primary.lock()) static_cast<void>(field->redo());
        }));
    auto clipboard_status = label(
        "showcase.text.clipboard.status",
        "Clipboard: select text, then use buttons or Cmd/Ctrl+C, X, V",
        11.0, 600, green);
    commands->add_at(clipboard_status, {24.0, 92.0, 830.0, 26.0});
    const std::weak_ptr<Label> weak_clipboard_status = clipboard_status;
    const auto update_clipboard_status =
        [weak_primary, weak_clipboard_status](std::string_view command,
                                              bool accepted) {
            if (const auto status = weak_clipboard_status.lock()) {
                const auto field = weak_primary.lock();
                status->set_text(std::string("Clipboard ") + std::string(command) +
                    (accepted ? " · accepted" : " · unavailable or empty") +
                    (field ? " · " + std::to_string(field->text().size()) +
                                 " UTF-8 bytes retained" : std::string{}));
            }
        };
    context->subscriptions.push_back(command_buttons[4]->clicked().subscribe(
        *primary, [weak_primary, update_clipboard_status](ButtonBase&) {
            const auto field = weak_primary.lock();
            update_clipboard_status("copy", field && field->copy());
        }));
    context->subscriptions.push_back(command_buttons[5]->clicked().subscribe(
        *primary, [weak_primary, update_clipboard_status](ButtonBase&) {
            const auto field = weak_primary.lock();
            update_clipboard_status("cut", field && field->cut());
        }));
    context->subscriptions.push_back(command_buttons[6]->clicked().subscribe(
        *primary, [weak_primary, update_clipboard_status](ButtonBase&) {
            const auto field = weak_primary.lock();
            update_clipboard_status("paste", field && field->paste());
        }));
    auto guarantee = label("showcase.text.guarantee",
                           "TextBox + TextStore own editing. HostServices only transports bounded UTF-8 clipboard data; protected fields never export values.",
                           11.0, 600, shell_blue_dark);
    guarantee->set_text_wrapping(TextWrapping::word);
    guarantee->set_vertical_alignment(VerticalAlignment::near);
    commands->add_at(guarantee, {24.0, 132.0, 830.0, 52.0});
}

void add_collections(const std::shared_ptr<Surface>& page,
                     const std::shared_ptr<ShowcaseContext>& context) {
    page->add_at(label("showcase.collections.heading", "COLLECTIONS AND POPUPS", 22.0,
                       700, shell_blue_dark), {24.0, 18.0, 620.0, 34.0});
    page->add_at(label("showcase.collections.lead",
                       "Single and extended selection, virtualized rows, keyboard traversal, overlay dismissal, and focus restoration.",
                       12.0, 400, muted), {26.0, 52.0, 900.0, 24.0});

    auto single_group = group("showcase.collections.single", "ListBox · single selection",
                              {450.0, 360.0});
    page->add_at(single_group, {24.0, 92.0, 450.0, 360.0});
    auto single = make_control<ListBox>(StableId("showcase.collections.single.list"));
    single->set_accessible_name("Single-selection locations");
    single->set_items({"Local volumes", "Recent locations", "Pinned folders",
                       "Saved searches", "Plugin surfaces", "Diagnostics",
                       "Appearance", "Keyboard map", "Security policy",
                       "About GUI.Forms"});
    single->select_index(2U);
    single_group->add_at(single, {24.0, 38.0, 400.0, 280.0});

    auto multi_group = group("showcase.collections.multi",
                             "ListBox · extended selection", {460.0, 360.0});
    page->add_at(multi_group, {494.0, 92.0, 460.0, 360.0});
    auto multi = make_control<ListBox>(StableId("showcase.collections.multi.list"));
    multi->set_accessible_name("Extended-selection framework features");
    multi->set_selection_mode(ListSelectionMode::multiple_extended);
    multi->set_items({"CPU renderer", "Retained tree", "Damage regions",
                      "Tokenized events", "Unicode store", "Focus scopes",
                      "Popup controller", "Frame deadlines", "Host protocol",
                      "Drawing recorder"});
    multi->select_index(1U);
    multi->select_index(4U, true, false);
    multi_group->add_at(multi, {24.0, 38.0, 410.0, 280.0});

    auto combo_group = group("showcase.collections.combos",
                             "ComboBox · retained overlay popup", {930.0, 160.0});
    page->add_at(combo_group, {24.0, 474.0, 930.0, 160.0});
    auto profile = make_control<ComboBox>(StableId("showcase.collections.combo.profile"));
    profile->set_accessible_name("Visual profile");
    profile->set_items({"Windows 7 professional", "Windows 10 professional",
                        "System neutral", "High contrast"});
    profile->set_selected_index(1U);
    combo_group->add_at(profile, {24.0, 42.0, 300.0, 36.0});
    auto density = make_control<ComboBox>(StableId("showcase.collections.combo.density"));
    density->set_accessible_name("Control density");
    density->set_items({"Compact", "Comfortable", "Spacious", "Touch"});
    density->set_placeholder_text("Choose density");
    combo_group->add_at(density, {348.0, 42.0, 250.0, 36.0});
    auto status = label("showcase.collections.status",
                        "Popup overlays are root-attached, owner-revoked, and focus-contained.",
                        11.0, 600, green);
    combo_group->add_at(status, {24.0, 96.0, 780.0, 26.0});
    const std::weak_ptr<Label> weak_status = status;
    const std::weak_ptr<ComboBox> weak_profile = profile;
    context->subscriptions.push_back(profile->selected_index_changed().subscribe(
        *status, [weak_status, weak_profile](std::optional<std::size_t>) {
            if (const auto status = weak_status.lock(); status) {
                const auto profile = weak_profile.lock();
                if (!profile) return;
                status->set_text("Profile committed: " + std::string(profile->selected_text()) +
                                 " · overlay detached · focus restored");
            }
        }));
    context->subscriptions.push_back(density->drop_down_changed().subscribe(
        *status, [weak_status](bool open) {
            if (const auto status = weak_status.lock()) {
                status->set_text(open
                    ? "Density popup: OPEN · contained ListBox focus scope"
                    : "Density popup: CLOSED · owner focus restored");
            }
        }));
}

void add_values(const std::shared_ptr<Surface>& page,
                const std::shared_ptr<ShowcaseContext>& context) {
    page->add_at(label("showcase.values.heading", "VALUES AND SPINNERS", 22.0, 700,
                       shell_blue_dark), {24.0, 18.0, 600.0, 34.0});
    page->add_at(label("showcase.values.lead",
                       "Compound TextBox editing, bounded numeric commit, decimal and hexadecimal formats, keys, wheel, and spinner buttons.",
                       12.0, 400, muted), {26.0, 52.0, 900.0, 24.0});
    auto group_box = group("showcase.values.numeric", "NumericUpDown · public compound control",
                           {930.0, 360.0});
    page->add_at(group_box, {24.0, 92.0, 930.0, 360.0});

    const std::array<std::string_view, 4> captions{
        "Integer range", "Decimal increment", "Signed range", "Hexadecimal"};
    std::vector<std::shared_ptr<NumericUpDown>> values;
    for (std::size_t index = 0; index < captions.size(); ++index) {
        group_box->add_at(label("showcase.values.caption." + std::to_string(index),
                                std::string(captions[index]), 11.0, 600, muted),
                          {24.0, 44.0 + static_cast<double>(index) * 62.0, 170.0, 26.0});
        auto numeric = make_control<NumericUpDown>(
            StableId("showcase.values.numeric." + std::to_string(index)));
        numeric->set_accessible_name(std::string(captions[index]));
        if (index == 0U) {
            numeric->set_range(0.0, 100.0);
            numeric->set_value(42.0);
        } else if (index == 1U) {
            numeric->set_range(0.0, 10.0);
            numeric->set_increment(0.25);
            numeric->set_decimal_places(2U);
            numeric->set_value(3.75);
        } else if (index == 2U) {
            numeric->set_range(-120.0, 120.0);
            numeric->set_increment(5.0);
            numeric->set_value(-15.0);
        } else {
            numeric->set_range(0.0, 255.0);
            numeric->set_hexadecimal(true);
            numeric->set_value(175.0);
        }
        group_box->add_at(numeric,
                          {194.0, 38.0 + static_cast<double>(index) * 62.0,
                           260.0, 36.0});
        values.push_back(numeric);
    }
    auto status = label("showcase.values.status", "Edit a value or use arrows, wheel, and spinner buttons.",
                        11.0, 600, green);
    group_box->add_at(status, {500.0, 44.0, 380.0, 80.0});
    status->set_text_wrapping(TextWrapping::word);
    status->set_vertical_alignment(VerticalAlignment::near);
    for (std::size_t index = 0; index < values.size(); ++index) {
        const std::weak_ptr<Label> weak_status = status;
        context->subscriptions.push_back(values[index]->value_changed().subscribe(
            *status, [weak_status, index](double value) {
                if (const auto status = weak_status.lock()) {
                    status->set_text("Numeric " + std::to_string(index + 1U) +
                                     " committed " + std::to_string(value) +
                                     " · event emitted after value and editor synchronization");
                }
            }));
    }
    auto note = label("showcase.values.note",
                      "Intermediate invalid text does not corrupt Value. Enter restores the last valid formatted value.",
                      12.0, 400, ink);
    note->set_text_wrapping(TextWrapping::word);
    note->set_vertical_alignment(VerticalAlignment::near);
    group_box->add_at(note, {500.0, 154.0, 380.0, 80.0});
}

void add_images_and_drawing(const std::shared_ptr<Surface>& page,
                            const std::shared_ptr<ShowcaseContext>& context) {
    page->add_at(label("showcase.images.heading", "IMAGES AND DRAWING", 22.0, 700,
                       shell_blue_dark), {24.0, 18.0, 620.0, 34.0});
    page->add_at(label("showcase.images.lead",
                       "Window-owned image resources, every PictureBox sizing policy, opacity, clipping, and retained painter primitives.",
                       12.0, 400, muted), {26.0, 52.0, 920.0, 24.0});

    auto modes = group("showcase.images.modes",
                       "PictureBox · five canonical sizing modes", {930.0, 290.0});
    page->add_at(modes, {24.0, 92.0, 930.0, 290.0});
    constexpr std::array<std::pair<PictureBoxSizeMode, std::string_view>, 5> specs{{
        {PictureBoxSizeMode::normal, "Normal"},
        {PictureBoxSizeMode::stretch_image, "StretchImage"},
        {PictureBoxSizeMode::auto_size, "AutoSize"},
        {PictureBoxSizeMode::center_image, "CenterImage"},
        {PictureBoxSizeMode::zoom, "Zoom"},
    }};
    for (std::size_t index = 0; index < specs.size(); ++index) {
        const double x = 22.0 + static_cast<double>(index) * 180.0;
        auto picture = make_control<PictureBox>(
            StableId("showcase.images.picture." + std::to_string(index)));
        picture->set_size_mode(specs[index].first);
        picture->set_border_style(BorderStyle::sunken);
        picture->set_background(index % 2U == 0U ? Color::rgba(244, 247, 249)
                                                  : Color::rgba(226, 234, 240));
        picture->set_accessible_name(std::string(specs[index].second) +
                                     " PictureBox test card");
        modes->add_at(picture, {x, 40.0, 158.0, 166.0});
        auto caption = label("showcase.images.caption." + std::to_string(index),
                             std::string(specs[index].second), 11.0, 700,
                             shell_blue_dark);
        caption->set_alignment(HorizontalAlignment::center);
        modes->add_at(caption, {x, 216.0, 158.0, 24.0});
        auto behavior = label("showcase.images.behavior." + std::to_string(index),
                              index == 0U ? "Native pixels" : index == 1U
                                  ? "Fill bounds" : index == 2U
                                  ? "Intrinsic measure" : index == 3U
                                  ? "Native centered" : "Aspect fit",
                              10.0, 400, muted);
        behavior->set_alignment(HorizontalAlignment::center);
        modes->add_at(behavior, {x, 242.0, 158.0, 22.0});
    }

    auto opacity = group("showcase.images.opacity",
                         "Image opacity · renderer-neutral", {450.0, 220.0});
    page->add_at(opacity, {24.0, 404.0, 450.0, 220.0});
    constexpr std::array<double, 3> opacities{1.0, 0.60, 0.25};
    for (std::size_t index = 0; index < opacities.size(); ++index) {
        auto picture = make_control<PictureBox>(
            StableId("showcase.images.opacity." + std::to_string(index)));
        picture->set_size_mode(PictureBoxSizeMode::zoom);
        picture->set_image_opacity(opacities[index]);
        picture->set_border_style(BorderStyle::line);
        picture->set_background(Color::rgba(230, 236, 241));
        picture->set_accessible_name(
            "Test card at " + std::to_string(static_cast<int>(opacities[index] * 100.0)) +
            " percent opacity");
        opacity->add_at(picture,
                        {22.0 + static_cast<double>(index) * 136.0, 42.0,
                         120.0, 112.0});
        auto caption = label("showcase.images.opacity.caption." +
                                 std::to_string(index),
                             std::to_string(static_cast<int>(opacities[index] * 100.0)) +
                                 "%",
                             10.0, 700, shell_blue_dark);
        caption->set_alignment(HorizontalAlignment::center);
        opacity->add_at(caption,
                        {22.0 + static_cast<double>(index) * 136.0, 164.0,
                         120.0, 22.0});
    }

    auto primitives = group("showcase.images.primitives",
                             "Painter · clipped composition", {460.0, 220.0});
    page->add_at(primitives, {494.0, 404.0, 460.0, 220.0});
    auto board = make_control<DrawingEffectsBoard>(
        StableId("showcase.images.drawing.board"));
    board->set_accessible_name("Renderer-neutral drawing primitive composition");
    board->set_accessible_description(
        "Nested clipping, translation, fills, strokes, lines, and text");
    board->set_paint_callback([](Painter& painter, Rect bounds, Rect) {
        const double board_width = bounds.width;
        const double board_height = bounds.height;
        painter.fill_rect(bounds,
                          Color::rgba(26, 39, 52));
        painter.fill_rect({0.0, 0.0, board_width, 4.0}, accent);
        painter.draw_text_utf8({18.0, 27.0}, "RETAINED PAINTER COMPOSITION",
                               {FontRole::control, 12.0, 700, false},
                               Color::rgba(230, 240, 247));
        painter.draw_text_utf8(
            {18.0, 48.0},
            "save · clip · translate · fill · stroke · line · UTF-8",
            {FontRole::content, 10.0, 400, false},
            Color::rgba(164, 192, 211));
        painter.save();
        painter.clip_rect({18.0, 62.0, board_width - 36.0, board_height - 78.0});
        painter.translate({18.0, 62.0});
        const double width = board_width - 36.0;
        for (std::size_t index = 0; index < 12U; ++index) {
            const double x = static_cast<double>(index) * width / 11.0;
            const Color color = index % 3U == 0U ? accent
                : index % 3U == 1U ? green : orange;
            painter.draw_line({0.0, 66.0}, {x, 0.0}, color, 1.5);
        }
        painter.fill_rect({20.0, 20.0, 92.0, 38.0}, Color::rgba(43, 82, 112));
        painter.stroke_rect({20.5, 20.5, 91.0, 37.0},
                            Color::rgba(211, 228, 239), 1.0);
        painter.fill_rect({126.0, 12.0, 72.0, 54.0}, violet);
        painter.stroke_rect({126.5, 12.5, 71.0, 53.0},
                            Color::rgba(234, 220, 244), 1.0);
        painter.fill_rect({212.0, 28.0, 118.0, 22.0}, green);
        painter.restore();
    });
    primitives->add_at(board, {20.0, 38.0, 420.0, 158.0});
    static_cast<void>(context);
}

void add_tabs_and_pages(const std::shared_ptr<Surface>& page,
                        const std::shared_ptr<ShowcaseContext>& context) {
    page->add_at(label("showcase.tabs.heading", "TABS AND RETAINED PAGES", 22.0,
                       700, shell_blue_dark), {24.0, 18.0, 680.0, 34.0});
    page->add_at(label("showcase.tabs.lead",
                       "Owned page identity, selection events, focus restoration, keyboard traversal, four alignments, and three appearances.",
                       12.0, 400, muted), {26.0, 52.0, 920.0, 24.0});

    auto primary_group = group("showcase.tabs.primary.group",
                               "TabControl · interactive retained pages",
                               {930.0, 280.0});
    page->add_at(primary_group, {24.0, 92.0, 930.0, 280.0});
    auto primary = make_control<TabControl>(StableId("showcase.tabs.primary"));
    primary->set_style(showcase_style());
    primary->set_accessible_name("Primary workspace tabs");
    primary->set_item_size({150.0, 30.0});
    primary_group->add_at(primary, {20.0, 36.0, 890.0, 220.0});
    constexpr std::array<std::string_view, 4> primary_names{
        "Overview", "Activity", "Security", "Diagnostics"};
    for (std::size_t index = 0U; index < primary_names.size(); ++index) {
        auto tab_page = make_control<TabPage>(
            StableId("showcase.tabs.primary." +
                     std::string(index == 0U ? "overview" : index == 1U
                         ? "activity" : index == 2U ? "security" : "diagnostics")),
            std::string(primary_names[index]));
        auto title = label("showcase.tabs.primary.page.title." +
                               std::to_string(index),
                           std::string(primary_names[index]) + " workspace",
                           16.0, 700, shell_blue_dark);
        title->set_requested_bounds({22.0, 22.0, 420.0, 30.0});
        tab_page->add_child(title);
        auto copy = label("showcase.tabs.primary.page.copy." +
                              std::to_string(index),
                          index == 0U
                              ? "A retained page owns ordinary controls and remains stable across selection cycles."
                              : index == 1U
                              ? "Ctrl+Tab and Ctrl+Shift+Tab traverse pages from focused child controls."
                              : index == 2U
                              ? "Hidden pages leave paint, hit testing, focus traversal, and accessibility."
                              : "Removal and direct disposal reconcile the surviving selected page.",
                          11.0, 400, ink);
        copy->set_requested_bounds({22.0, 58.0, 660.0, 26.0});
        tab_page->add_child(copy);
        if (index == 0U) {
            auto action = make_control<Button>(
                StableId("showcase.tabs.primary.action"), "Run page action");
            action->set_visual_style(ButtonVisualStyle::accent);
            action->set_requested_bounds({22.0, 104.0, 170.0, 36.0});
            tab_page->add_child(action);
        } else if (index == 1U) {
            auto toggle = make_control<CheckBox>(
                StableId("showcase.tabs.primary.activity.toggle"),
                "Keep activity visible");
            toggle->set_indicator_style(ChoiceIndicatorStyle::toggle);
            toggle->set_checked(true);
            toggle->set_requested_bounds({22.0, 104.0, 220.0, 30.0});
            tab_page->add_child(toggle);
        } else if (index == 2U) {
            auto radio = make_control<RadioButton>(
                StableId("showcase.tabs.primary.security.radio"),
                "Professional policy");
            radio->set_checked(true);
            radio->set_requested_bounds({22.0, 104.0, 220.0, 30.0});
            tab_page->add_child(radio);
        } else {
            auto value = make_control<ProgressBar>(
                StableId("showcase.tabs.primary.diagnostics.progress"));
            value->set_range(0.0, 100.0);
            value->set_value(78.0);
            value->set_accessible_name("Diagnostics completion");
            value->set_requested_bounds({22.0, 108.0, 360.0, 24.0});
            tab_page->add_child(value);
        }
        primary->add_page(tab_page);
    }
    primary->set_selected_index(1U);

    auto status = label("showcase.tabs.status",
                        "Activity selected · hidden pages are fully quiescent",
                        11.0, 600, green);
    primary_group->add_at(status, {620.0, 7.0, 286.0, 24.0});
    const std::weak_ptr<Label> weak_status = status;
    const std::weak_ptr<TabControl> weak_primary = primary;
    context->subscriptions.push_back(primary->selected_index_changed().subscribe(
        *status, [weak_status, weak_primary](const TabSelectionChange&) {
            const auto status = weak_status.lock();
            const auto tabs = weak_primary.lock();
            if (status && tabs && tabs->selected_tab()) {
                status->set_text(tabs->selected_tab()->text() +
                                 " selected · focus and semantics reconciled");
            }
        }));

    auto variants = group("showcase.tabs.variants",
                          "Alignment and appearance matrix", {930.0, 230.0});
    page->add_at(variants, {24.0, 394.0, 930.0, 230.0});
    constexpr std::array<std::tuple<TabAlignment, TabAppearance,
                                    std::string_view>, 3> variants_spec{{
        {TabAlignment::bottom, TabAppearance::buttons, "Bottom · buttons"},
        {TabAlignment::left, TabAppearance::flat_buttons, "Left · flat"},
        {TabAlignment::right, TabAppearance::normal, "Right · normal"},
    }};
    for (std::size_t index = 0U; index < variants_spec.size(); ++index) {
        const double x = 20.0 + static_cast<double>(index) * 303.0;
        auto caption = label("showcase.tabs.variant.caption." +
                                 std::to_string(index),
                             std::string(std::get<2>(variants_spec[index])),
                             10.0, 700, shell_blue_dark);
        variants->add_at(caption, {x, 34.0, 280.0, 22.0});
        auto tabs = make_control<TabControl>(
            StableId("showcase.tabs.variant." + std::to_string(index)));
        tabs->set_accessible_name(std::string(std::get<2>(variants_spec[index])) +
                                  " tabs");
        tabs->set_style(showcase_style());
        tabs->set_alignment(std::get<0>(variants_spec[index]));
        tabs->set_appearance(std::get<1>(variants_spec[index]));
        tabs->set_item_size(index == 0U ? Size{86.0, 28.0} : Size{82.0, 28.0});
        variants->add_at(tabs, {x, 60.0, 280.0, 148.0});
        for (std::size_t page_index = 0U; page_index < 3U; ++page_index) {
            auto tab_page = make_control<TabPage>(
                StableId("showcase.tabs.variant." + std::to_string(index) +
                         ".page." + std::to_string(page_index)),
                page_index == 0U ? "One" : page_index == 1U ? "Two" : "Three");
            auto content = label("showcase.tabs.variant.content." +
                                     std::to_string(index) + "." +
                                     std::to_string(page_index),
                                 "Retained page " + std::to_string(page_index + 1U),
                                 10.0, 600, muted);
            content->set_requested_bounds({12.0, 14.0, 150.0, 24.0});
            tab_page->add_child(content);
            tabs->add_page(tab_page);
        }
        tabs->set_selected_index(index % 3U);
    }
}

void add_checked_collections(const std::shared_ptr<Surface>& page,
                             const std::shared_ptr<ShowcaseContext>& context) {
    page->add_at(label("showcase.checked.heading", "CHECKED COLLECTIONS", 22.0,
                       700, shell_blue_dark), {24.0, 18.0, 620.0, 34.0});
    page->add_at(label("showcase.checked.lead",
                       "Check state remains independent of selection, with cancellable pre-change events, keyboard control, and checkable virtual rows.",
                       12.0, 400, muted), {26.0, 52.0, 920.0, 24.0});

    auto deliberate_group = group("showcase.checked.deliberate",
                                  "CheckedListBox · deliberate two-click policy",
                                  {450.0, 400.0});
    page->add_at(deliberate_group, {24.0, 92.0, 450.0, 400.0});
    auto deliberate = make_control<CheckedListBox>(
        StableId("showcase.checked.deliberate.list"));
    deliberate->set_accessible_name("Deliberate feature selection");
    deliberate->add_item("Renderer-neutral core", CheckState::checked);
    deliberate->add_item("Window-owned resources", CheckState::checked);
    deliberate->add_item("Native accessibility", CheckState::indeterminate);
    deliberate->add_item("Stable semantic IDs", CheckState::checked);
    deliberate->add_item("Renderer-free build", CheckState::unchecked);
    deliberate->add_item("Sustained soak gate", CheckState::unchecked);
    deliberate_group->add_at(deliberate, {24.0, 40.0, 400.0, 282.0});
    auto deliberate_note = label("showcase.checked.deliberate.note",
                                 "First click selects. Click the selected row again or press Space to toggle.",
                                 10.0, 400, muted);
    deliberate_note->set_text_wrapping(TextWrapping::word);
    deliberate_note->set_vertical_alignment(VerticalAlignment::near);
    deliberate_group->add_at(deliberate_note, {24.0, 332.0, 400.0, 48.0});

    auto immediate_group = group("showcase.checked.immediate",
                                 "CheckedListBox · CheckOnClick + extended selection",
                                 {460.0, 400.0});
    page->add_at(immediate_group, {494.0, 92.0, 460.0, 400.0});
    auto immediate = make_control<CheckedListBox>(
        StableId("showcase.checked.immediate.list"));
    immediate->set_accessible_name("Immediate capability selection");
    immediate->set_check_on_click(true);
    immediate->set_selection_mode(ListSelectionMode::multiple_extended);
    immediate->add_item("Pointer activation", CheckState::checked);
    immediate->add_item("Space-key toggle", CheckState::checked);
    immediate->add_item("Cancelled transition", CheckState::unchecked);
    immediate->add_item("Indeterminate state", CheckState::indeterminate);
    immediate->add_item("Mutation remapping", CheckState::checked);
    immediate->add_item("Semantic press", CheckState::unchecked);
    immediate->select_index(1U);
    immediate->select_index(4U, true, false);
    immediate_group->add_at(immediate, {24.0, 40.0, 410.0, 282.0});
    auto immediate_note = label("showcase.checked.immediate.note",
                                "CheckOnClick toggles immediately; Shift/Ctrl selection remains a separate model.",
                                10.0, 400, muted);
    immediate_note->set_text_wrapping(TextWrapping::word);
    immediate_note->set_vertical_alignment(VerticalAlignment::near);
    immediate_group->add_at(immediate_note, {24.0, 332.0, 410.0, 48.0});

    auto commands = group("showcase.checked.commands",
                          "Check-state commands and ordered telemetry", {930.0, 112.0});
    page->add_at(commands, {24.0, 514.0, 930.0, 112.0});
    auto check_all = make_control<Button>(StableId("showcase.checked.command.all"),
                                          "Check all");
    auto clear = make_control<Button>(StableId("showcase.checked.command.clear"),
                                      "Clear all");
    auto mixed = make_control<Button>(StableId("showcase.checked.command.mixed"),
                                      "Set mixed");
    check_all->set_visual_style(ButtonVisualStyle::accent);
    commands->add_at(check_all, {22.0, 42.0, 130.0, 34.0});
    commands->add_at(clear, {166.0, 42.0, 130.0, 34.0});
    commands->add_at(mixed, {310.0, 42.0, 130.0, 34.0});
    auto status = label("showcase.checked.status",
                        "Ready · pre-change event precedes committed state",
                        10.0, 600, green);
    status->set_text_wrapping(TextWrapping::word);
    status->set_vertical_alignment(VerticalAlignment::near);
    commands->add_at(status, {470.0, 38.0, 420.0, 48.0});
    const std::weak_ptr<CheckedListBox> weak_immediate = immediate;
    const std::weak_ptr<Label> weak_status = status;
    context->subscriptions.push_back(immediate->item_check_state_changed().subscribe(
        *status, [weak_status](std::size_t index, CheckState state) {
            if (const auto status = weak_status.lock()) {
                status->set_text("Item " + std::to_string(index + 1U) +
                                 " committed " +
                                 (state == CheckState::checked ? "checked" :
                                  state == CheckState::indeterminate ? "mixed" :
                                  "unchecked") +
                                 " · selection unchanged");
            }
        }));
    context->subscriptions.push_back(check_all->clicked().subscribe(
        *immediate, [weak_immediate](ButtonBase&) {
            if (const auto list = weak_immediate.lock()) {
                for (std::size_t index = 0U; index < list->items().size(); ++index) {
                    list->set_item_checked(index, true);
                }
            }
        }));
    context->subscriptions.push_back(clear->clicked().subscribe(
        *immediate, [weak_immediate](ButtonBase&) {
            if (const auto list = weak_immediate.lock()) {
                for (std::size_t index = 0U; index < list->items().size(); ++index) {
                    list->set_item_checked(index, false);
                }
            }
        }));
    context->subscriptions.push_back(mixed->clicked().subscribe(
        *immediate, [weak_immediate](ButtonBase&) {
            if (const auto list = weak_immediate.lock(); list && !list->items().empty()) {
                list->set_item_check_state(3U, CheckState::indeterminate);
            }
    }));
}

void add_timing_and_tooltips(const std::shared_ptr<Surface>& page,
                             const std::shared_ptr<ShowcaseContext>& context) {
    page->add_at(label("showcase.timing.heading", "TIMING AND TOOLTIPS", 22.0,
                       700, shell_blue_dark), {24.0, 18.0, 620.0, 34.0});
    page->add_at(label("showcase.timing.lead",
                       "UI-thread component deadlines, coalesced cadence, focus/hover help, owner revocation, and retained overlays.",
                       12.0, 400, muted), {26.0, 52.0, 920.0, 24.0});

    auto timer_group = group("showcase.timing.timer.group",
                             "Timer · deterministic UI-loop component", {930.0, 248.0});
    page->add_at(timer_group, {24.0, 92.0, 930.0, 248.0});
    context->timer_status = label("showcase.timing.tick.status",
                                  "Idle · enter this page to start", 12.0, 700, green);
    timer_group->add_at(context->timer_status, {22.0, 38.0, 510.0, 28.0});
    context->timer_start = make_control<Button>(StableId("showcase.timing.start"),
                                                "Start timer");
    context->timer_start->set_visual_style(ButtonVisualStyle::accent);
    context->timer_stop = make_control<Button>(StableId("showcase.timing.stop"),
                                               "Stop timer");
    timer_group->add_at(context->timer_start, {22.0, 78.0, 132.0, 34.0});
    timer_group->add_at(context->timer_stop, {166.0, 78.0, 132.0, 34.0});
    timer_group->add_at(label("showcase.timing.interval.label", "Interval", 10.0,
                              700, shell_blue_dark), {326.0, 76.0, 80.0, 20.0});
    context->timer_interval = make_control<TrackBar>(
        StableId("showcase.timing.interval"));
    context->timer_interval->set_range(80.0, 600.0);
    context->timer_interval->set_value(250.0);
    context->timer_interval->set_tick_frequency(4U);
    context->timer_interval->set_accessible_name("UI timer interval in milliseconds");
    timer_group->add_at(context->timer_interval, {326.0, 98.0, 260.0, 44.0});
    context->timer_progress = make_control<ProgressBar>(
        StableId("showcase.timing.progress"));
    context->timer_progress->set_range(0.0, 100.0);
    context->timer_progress->set_value(0.0);
    context->timer_progress->set_accessible_name("Timer cycle");
    timer_group->add_at(context->timer_progress, {22.0, 164.0, 564.0, 24.0});
    context->timer_motion = make_control<ScaledPanel>(
        StableId("showcase.timing.motion"), Size{294.0, 170.0});
    context->timer_motion->set_background(Color::rgba(247, 249, 251));
    context->timer_motion->set_border_style(BorderStyle::line);
    context->timer_motion->add_at(
        label("showcase.timing.motion.caption", "WINDOW-QUEUE MOTION · 4 Hz",
              10.0, 700, muted),
        {14.0, 8.0, 266.0, 24.0});
    auto motion_rule = make_control<Panel>(
        StableId("showcase.timing.motion.rule"));
    motion_rule->set_background(rule);
    context->timer_motion->add_at(motion_rule, {14.0, 84.0, 266.0, 1.0});
    context->timer_motion_target = make_control<Button>(
        StableId("showcase.timing.motion.target"), "Moving tooltip target");
    context->timer_motion_target->set_visual_style(ButtonVisualStyle::accent);
    context->timer_motion_target->set_style(showcase_style());
    context->timer_motion->add_at(
        context->timer_motion_target, {14.0, 39.0, 168.0, 34.0});
    timer_group->add_at(context->timer_motion, {612.0, 38.0, 294.0, 170.0});
    auto cadence = label("showcase.timing.cadence.note",
                         "Late deadlines coalesce to one ordered tick; no catch-up burst and no hidden paint control.",
                         10.0, 400, muted);
    cadence->set_text_wrapping(TextWrapping::word);
    cadence->set_vertical_alignment(VerticalAlignment::near);
    timer_group->add_at(cadence, {22.0, 200.0, 564.0, 36.0});

    auto tips_group = group("showcase.timing.tooltip.group",
                            "ToolTip · attached provider and retained overlay", {930.0, 264.0});
    page->add_at(tips_group, {24.0, 362.0, 930.0, 264.0});
    auto hover = make_control<Button>(StableId("showcase.timing.tooltip.hover"),
                                      "Hover for detail");
    auto keyboard = make_control<Button>(StableId("showcase.timing.tooltip.focus"),
                                         "Tab here for help");
    auto persistent = make_control<Button>(
        StableId("showcase.timing.tooltip.persistent"), "Show persistent help");
    persistent->set_visual_style(ButtonVisualStyle::accent);
    keyboard->set_visual_style(ButtonVisualStyle::command);
    tips_group->add_at(hover, {22.0, 44.0, 176.0, 36.0});
    tips_group->add_at(keyboard, {214.0, 44.0, 176.0, 36.0});
    tips_group->add_at(persistent, {406.0, 44.0, 190.0, 36.0});
    context->tooltip_disabled_target = make_control<Button>(
        StableId("showcase.timing.tooltip.disabled"), "Disabled target");
    context->tooltip_disabled_target->set_enabled(false);
    context->tooltip_show_disabled = make_control<Button>(
        StableId("showcase.timing.tooltip.show-disabled"), "Explain disabled control");
    tips_group->add_at(context->tooltip_disabled_target,
                       {22.0, 102.0, 176.0, 36.0});
    tips_group->add_at(context->tooltip_show_disabled,
                       {214.0, 102.0, 206.0, 36.0});
    auto policy = label("showcase.timing.tooltip.policy",
                        "Hover: 420 ms · Reshow: 90 ms · Auto-pop: 4 s\nKeyboard focus uses the same provider. Popups follow moving targets and disappear with their owner.",
                        10.0, 400, muted);
    policy->set_text_wrapping(TextWrapping::word);
    policy->set_vertical_alignment(VerticalAlignment::near);
    tips_group->add_at(policy, {22.0, 166.0, 574.0, 70.0});
    auto proof = label("showcase.timing.tooltip.proof",
                       "TRY IT\nHover each target, tab through the controls, adjust the interval, and watch the anchored moving tooltip.",
                       11.0, 700, shell_blue_dark);
    proof->set_text_wrapping(TextWrapping::word);
    proof->set_vertical_alignment(VerticalAlignment::near);
    tips_group->add_at(proof, {628.0, 44.0, 270.0, 126.0});
}

void add_dates_and_calendar(const std::shared_ptr<Surface>& page,
                            const std::shared_ptr<ShowcaseContext>& context) {
    page->add_at(label("showcase.date.heading", "DATES AND CALENDAR", 22.0,
                       700, shell_blue_dark), {24.0, 18.0, 620.0, 34.0});
    page->add_at(label(
        "showcase.date.lead",
        "Validated civil time, caller-owned formats, nullable and spinner modes, retained popup focus, and semantic date cells.",
        12.0, 400, muted), {26.0, 52.0, 920.0, 24.0});

    const DateTimeValue specimen{2026, 8U, 5U, 14U, 7U, 9U, 125U};
    auto formats = group("showcase.date.formats",
                         "Formatting · four modes and an owned provider",
                         {930.0, 180.0});
    page->add_at(formats, {24.0, 92.0, 930.0, 180.0});
    const std::array<std::string_view, 4> captions{
        "Long date", "Short date", "Time", "Custom"};
    for (std::size_t index = 0U; index < captions.size(); ++index) {
        const double column = index % 2U == 0U ? 22.0 : 474.0;
        const double row = index < 2U ? 40.0 : 102.0;
        formats->add_at(label("showcase.date.format.caption." + std::to_string(index),
                              std::string(captions[index]), 10.0, 700,
                              shell_blue_dark),
                        {column, row, 86.0, 22.0});
        auto picker = make_control<DateTimePicker>(
            StableId("showcase.date.format." + std::to_string(index)));
        picker->set_style(showcase_style());
        picker->set_value(specimen);
        picker->set_accessible_name(std::string(captions[index]) + " picker");
        if (index == 1U) {
            picker->set_format(DateTimePickerFormat::short_date);
        } else if (index == 2U) {
            picker->set_format(DateTimePickerFormat::time);
        } else if (index == 3U) {
            picker->set_custom_format("yyyy-MM-dd '·' HH:mm");
            picker->set_format(DateTimePickerFormat::custom);
        }
        formats->add_at(picker, {column + 92.0, row - 5.0, 320.0, 30.0});
    }

    auto modes = group("showcase.date.modes",
                       "Modes and states · nullable, spinner, provider, disabled",
                       {930.0, 180.0});
    page->add_at(modes, {24.0, 290.0, 930.0, 180.0});

    auto optional = make_control<DateTimePicker>(
        StableId("showcase.date.optional"));
    optional->set_style(showcase_style());
    optional->set_value(specimen);
    optional->set_format(DateTimePickerFormat::short_date);
    optional->set_show_check_box(true);
    optional->set_checked(false);
    optional->set_accessible_name("Optional unchecked date");
    modes->add_at(label("showcase.date.optional.caption", "Optional value", 10.0,
                        700, shell_blue_dark), {22.0, 40.0, 106.0, 22.0});
    modes->add_at(optional, {132.0, 35.0, 284.0, 30.0});

    auto spinner = make_control<DateTimePicker>(
        StableId("showcase.date.spinner"));
    spinner->set_style(showcase_style());
    spinner->set_value(specimen);
    spinner->set_format(DateTimePickerFormat::short_date);
    spinner->set_show_up_down(true);
    spinner->set_accessible_name("Date spinner");
    modes->add_at(label("showcase.date.spinner.caption", "Up/down date", 10.0,
                        700, shell_blue_dark), {474.0, 40.0, 106.0, 22.0});
    modes->add_at(spinner, {584.0, 35.0, 320.0, 30.0});

    auto provider = make_control<DateTimePicker>(
        StableId("showcase.date.provider"));
    provider->set_style(showcase_style());
    provider->set_value(specimen);
    provider->set_format_provider(french_date_provider());
    provider->set_accessible_name("Caller-owned French date provider");
    modes->add_at(label("showcase.date.provider.caption", "Owned provider", 10.0,
                        700, shell_blue_dark), {22.0, 102.0, 106.0, 22.0});
    modes->add_at(provider, {132.0, 97.0, 284.0, 30.0});

    auto disabled = make_control<DateTimePicker>(
        StableId("showcase.date.disabled"));
    disabled->set_style(showcase_style());
    disabled->set_value(specimen);
    disabled->set_custom_format("MMM d, yyyy");
    disabled->set_format(DateTimePickerFormat::custom);
    disabled->set_enabled(false);
    disabled->set_accessible_name("Disabled date picker");
    modes->add_at(label("showcase.date.disabled.caption", "Disabled state", 10.0,
                        700, shell_blue_dark), {474.0, 102.0, 106.0, 22.0});
    modes->add_at(disabled, {584.0, 97.0, 320.0, 30.0});

    auto lifecycle = group("showcase.date.lifecycle",
                           "Popup lifecycle · commit, cancel, bounds, semantics",
                           {930.0, 138.0});
    page->add_at(lifecycle, {24.0, 488.0, 930.0, 138.0});
    auto primary = make_control<DateTimePicker>(
        StableId("showcase.date.primary"));
    primary->set_style(showcase_style());
    primary->set_value(specimen);
    primary->set_range({2026, 8U, 1U}, {2026, 8U, 31U, 23U, 59U, 59U, 999U});
    primary->set_accessible_name("Bounded appointment date");
    lifecycle->add_at(primary, {22.0, 38.0, 310.0, 31.0});

    auto previous = make_control<Button>(
        StableId("showcase.date.previous"), "Previous day");
    auto next = make_control<Button>(StableId("showcase.date.next"), "Next day");
    auto open = make_control<Button>(StableId("showcase.date.open"), "Open calendar");
    open->set_visual_style(ButtonVisualStyle::accent);
    lifecycle->add_at(previous, {352.0, 38.0, 138.0, 32.0});
    lifecycle->add_at(next, {504.0, 38.0, 124.0, 32.0});
    lifecycle->add_at(open, {642.0, 38.0, 164.0, 32.0});
    auto status = label("showcase.date.status",
                        "Ready · August 1–31, 2026 · F4 or Alt+Down opens",
                        10.0, 600, green);
    status->set_text_wrapping(TextWrapping::word);
    status->set_vertical_alignment(VerticalAlignment::near);
    lifecycle->add_at(status, {22.0, 86.0, 884.0, 34.0});

    const std::weak_ptr<DateTimePicker> weak_primary = primary;
    const std::weak_ptr<Label> weak_status = status;
    const auto publish_value = [weak_primary, weak_status](std::string_view verb) {
        const auto picker = weak_primary.lock();
        const auto message = weak_status.lock();
        if (picker && message) {
            message->set_text(std::string(verb) + " · " +
                              picker->formatted_value() +
                              " · retained focus restored");
        }
    };
    context->subscriptions.push_back(primary->value_changed().subscribe(
        *status, [publish_value](DateTimeValue) { publish_value("Committed"); }));
    context->subscriptions.push_back(primary->drop_down_changed().subscribe(
        *status, [weak_status](bool expanded) {
            if (const auto message = weak_status.lock()) {
                message->set_text(expanded
                    ? "Calendar open · arrows navigate · Enter commits · Esc cancels"
                    : "Calendar closed · popup and focus scope revoked");
            }
        }));
    context->subscriptions.push_back(optional->checked_changed().subscribe(
        *status, [weak_status](bool checked) {
            if (const auto message = weak_status.lock()) {
                message->set_text(checked
                    ? "Optional date enabled · value retained"
                    : "Optional date unchecked · value retained but inactive");
            }
        }));
    context->subscriptions.push_back(previous->clicked().subscribe(
        *primary, [weak_primary](ButtonBase&) {
            if (const auto picker = weak_primary.lock()) {
                picker->set_value(add_days(picker->value(), -1));
            }
        }));
    context->subscriptions.push_back(next->clicked().subscribe(
        *primary, [weak_primary](ButtonBase&) {
            if (const auto picker = weak_primary.lock()) {
                picker->set_value(add_days(picker->value(), 1));
            }
        }));
    context->subscriptions.push_back(open->clicked().subscribe(
        *primary, [weak_primary](ButtonBase&) {
            if (const auto picker = weak_primary.lock()) {
                picker->set_dropped_down(true);
            }
        }));
}

void add_dialogs_and_host_services(
    const std::shared_ptr<Surface>& page,
    const std::shared_ptr<ShowcaseContext>& context) {
    page->add_at(label("showcase.host.heading", "DIALOGS AND HOST SERVICES", 22.0,
                       700, shell_blue_dark), {24.0, 18.0, 620.0, 34.0});
    page->add_at(label(
        "showcase.host.lead",
        "One portable contract for modal results, clipboard, monitor geometry, and semantic sound across native hosts.",
        12.0, 400, muted), {26.0, 52.0, 900.0, 24.0});

    auto messages = group("showcase.host.messages",
                          "Message dialogs · buttons, icons, default choices",
                          {930.0, 132.0});
    page->add_at(messages, {24.0, 92.0, 930.0, 132.0});
    struct MessageCase final {
        std::string_view label;
        HostMessageButtons buttons;
        HostMessageIcon icon;
        HostDialogChoice default_choice;
    };
    constexpr std::array<MessageCase, 4> message_cases{{
        {"Information · OK", HostMessageButtons::ok,
         HostMessageIcon::information, HostDialogChoice::ok},
        {"Warning · OK/Cancel", HostMessageButtons::ok_cancel,
         HostMessageIcon::warning, HostDialogChoice::cancel},
        {"Question · Yes/No", HostMessageButtons::yes_no,
         HostMessageIcon::question, HostDialogChoice::yes},
        {"Error · Retry/Cancel", HostMessageButtons::retry_cancel,
         HostMessageIcon::error, HostDialogChoice::retry},
    }};

    const auto publish_dialog_result = [context](const HostDialogResult& result) {
        if (!context->host_services_status) return;
        if (!result.status.accepted()) {
            context->host_services_status->set_text(
                std::string("Host rejected request · ") +
                host_service_error_name(result.status.error));
            return;
        }
        std::visit([context](const auto& value) {
            using Value = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<Value, HostMessageDialogResult>) {
                const std::string detail = std::string("Message ") +
                    host_dialog_outcome_name(value.outcome) + " · " +
                    host_dialog_choice_name(value.choice);
                if (value.outcome == HostDialogOutcome::accepted) {
                    context->last_dialog_acceptance = detail;
                }
                context->host_services_status->set_text(
                    value.outcome == HostDialogOutcome::cancelled
                        ? detail + " · preserved: " + context->last_dialog_acceptance
                        : detail);
            } else if constexpr (std::is_same_v<Value, HostPathDialogResult>) {
                if (value.outcome == HostDialogOutcome::accepted) {
                    context->last_dialog_acceptance =
                        std::to_string(value.paths.size()) + " path" +
                        (value.paths.size() == 1U ? "" : "s") + " · " +
                        value.paths.front();
                    context->host_services_status->set_text(
                        "Accepted · " + context->last_dialog_acceptance);
                } else {
                    context->host_services_status->set_text(
                        "Cancelled · preserved: " + context->last_dialog_acceptance);
                }
            } else {
                if (value.outcome == HostDialogOutcome::accepted) {
                    char encoded[16]{};
                    std::snprintf(encoded, sizeof(encoded), "#%08X", value.rgba);
                    context->last_dialog_acceptance =
                        std::string("color ") + encoded;
                    context->host_services_status->set_text(
                        "Accepted · " + context->last_dialog_acceptance);
                } else {
                    context->host_services_status->set_text(
                        "Cancelled · preserved: " + context->last_dialog_acceptance);
                }
            }
        }, result.payload);
    };

    const auto invoke_dialog = [context, publish_dialog_result](
                                   ButtonBase& source,
                                   HostDialogRequestPayload payload) {
        HostServices* services = source.attached_window() == nullptr
            ? nullptr : source.attached_window()->host_services();
        if (services == nullptr) {
            HostDialogResult unavailable;
            unavailable.status.error = HostServiceError::unsupported;
            publish_dialog_result(unavailable);
            return;
        }
        HostDialogRequest request;
        request.request_id = context->next_host_request_id++;
        request.owner_id = std::string(source.stable_id().value());
        request.payload = std::move(payload);
        publish_dialog_result(services->show_dialog(request));
    };

    for (std::size_t index = 0U; index < message_cases.size(); ++index) {
        const MessageCase value = message_cases[index];
        auto button = make_control<Button>(
            StableId("showcase.host.message." + std::to_string(index)),
            std::string(value.label));
        button->set_visual_style(index == 2U ? ButtonVisualStyle::accent
                                             : ButtonVisualStyle::command);
        messages->add_at(button, {24.0 + static_cast<double>(index) * 220.0,
                                  44.0, 202.0, 42.0});
        context->subscriptions.push_back(button->clicked().subscribe(
            *messages, [invoke_dialog, value](ButtonBase& source) {
                HostMessageDialogRequest request;
                request.title = "GUI.Forms message contract";
                request.message = "This modal result travels through the portable HostServices boundary.";
                request.buttons = value.buttons;
                request.icon = value.icon;
                request.default_choice = value.default_choice;
                invoke_dialog(source, std::move(request));
            }));
    }

    auto paths = group("showcase.host.paths",
                       "Path and color dialogs · typed requests and cancel preservation",
                       {930.0, 142.0});
    page->add_at(paths, {24.0, 242.0, 930.0, 142.0});
    const std::array<std::string_view, 5> path_labels{
        "Open file", "Open multiple", "Save file", "Choose folder", "Choose color"};
    for (std::size_t index = 0U; index < path_labels.size(); ++index) {
        auto button = make_control<Button>(
            StableId("showcase.host.dialog." + std::to_string(index)),
            std::string(path_labels[index]));
        button->set_visual_style(index == 4U ? ButtonVisualStyle::accent
                                             : ButtonVisualStyle::standard);
        paths->add_at(button, {24.0 + static_cast<double>(index) * 176.0,
                              44.0, 158.0, 40.0});
        context->subscriptions.push_back(button->clicked().subscribe(
            *paths, [invoke_dialog, index](ButtonBase& source) {
                if (index <= 1U) {
                    HostOpenFileDialogRequest request;
                    request.title = index == 0U ? "Open one fixture" : "Open fixture set";
                    request.filters = {{"Text and logs", {"txt", "log"}},
                                       {"PNG images", {"png"}}};
                    request.allow_multiple = index == 1U;
                    invoke_dialog(source, std::move(request));
                } else if (index == 2U) {
                    HostSaveFileDialogRequest request;
                    request.title = "Save GUI.Forms evidence";
                    request.suggested_name = "gui-forms-evidence";
                    request.default_extension = "txt";
                    request.filters = {{"Text evidence", {"txt"}}};
                    request.confirm_overwrite = true;
                    invoke_dialog(source, std::move(request));
                } else if (index == 3U) {
                    invoke_dialog(source, HostFolderDialogRequest{
                        "Choose an evidence directory", {}});
                } else {
                    invoke_dialog(source, HostColorDialogRequest{
                        "Choose an accent color", 0x2774B8FFU, false});
                }
            }));
    }
    paths->add_at(label("showcase.host.paths.note",
                        "Cancel never overwrites the last accepted path, choice, or color.",
                        11.0, 400, muted), {24.0, 98.0, 700.0, 24.0});

    auto services_group = group(
        "showcase.host.services",
        "Host services · clipboard, monitors, completion cue",
        {930.0, 224.0});
    page->add_at(services_group, {24.0, 402.0, 930.0, 224.0});
    context->clipboard_editor = make_control<TextBox>(
        StableId("showcase.host.clipboard.editor"));
    context->clipboard_editor->set_text("GUI.Forms clipboard probe Ω");
    context->clipboard_editor->set_accessible_name("Clipboard probe text");
    services_group->add_at(context->clipboard_editor, {24.0, 42.0, 432.0, 32.0});

    const std::array<std::string_view, 5> service_labels{
        "Copy to host", "Paste from host", "Inspect monitors",
        "Completion cue", "Warning cue"};
    for (std::size_t index = 0U; index < service_labels.size(); ++index) {
        auto button = make_control<Button>(
            StableId("showcase.host.service." + std::to_string(index)),
            std::string(service_labels[index]));
        button->set_visual_style(ButtonVisualStyle::command);
        services_group->add_at(button,
            {24.0 + static_cast<double>(index) * 176.0, 88.0, 158.0, 36.0});
        context->subscriptions.push_back(button->clicked().subscribe(
            *services_group, [context, index](ButtonBase& source) {
                HostServices* services = source.attached_window() == nullptr
                    ? nullptr : source.attached_window()->host_services();
                if (services == nullptr) {
                    context->host_services_status->set_text(
                        "Host service unavailable · portable control remains responsive");
                    return;
                }
                if (index == 0U) {
                    const HostServiceStatus result = services->write_clipboard_text(
                        context->clipboard_editor->text());
                    context->host_services_status->set_text(
                        std::string("Clipboard write · ") +
                        host_service_error_name(result.error));
                } else if (index == 1U) {
                    const HostClipboardTextResult result =
                        services->read_clipboard_text();
                    if (result.status.accepted() && result.has_text) {
                        context->clipboard_editor->set_text(result.text_utf8);
                    }
                    context->host_services_status->set_text(
                        std::string("Clipboard read · ") +
                        host_service_error_name(result.status.error) +
                        (result.has_text ? " · text restored" : " · empty"));
                } else if (index == 2U) {
                    const HostMonitorResult result = services->query_monitors();
                    if (!result.status.accepted() || result.monitors.empty()) {
                        context->host_services_status->set_text(
                            std::string("Monitor query · ") +
                            host_service_error_name(result.status.error));
                    } else {
                        const auto primary = std::find_if(
                            result.monitors.begin(), result.monitors.end(),
                            [](const HostMonitor& monitor) { return monitor.primary; });
                        const HostMonitor& monitor = primary == result.monitors.end()
                            ? result.monitors.front() : *primary;
                        context->host_services_status->set_text(
                            "Monitors " + std::to_string(result.monitors.size()) +
                            " · primary " + monitor.id + " · scale " +
                            std::to_string(monitor.scale));
                    }
                } else {
                    const auto now = std::chrono::steady_clock::now().time_since_epoch();
                    const auto stamp = std::chrono::duration_cast<
                        std::chrono::nanoseconds>(now).count();
                    const HostSoundCue cue = index == 3U
                        ? HostSoundCue::operation_complete : HostSoundCue::warning;
                    const HostServiceStatus result = services->play_sound_cue(
                        {cue, 0.72, static_cast<std::uint64_t>(stamp)});
                    context->host_services_status->set_text(
                        std::string("Sound ") + host_sound_cue_name(cue) +
                        " · " + host_service_error_name(result.error));
                }
            }));
    }
    context->host_services_status = label(
        "showcase.host.status",
        "Ready · native host results remain typed, bounded, and owner-modal",
        11.0, 600, green);
    context->host_services_status->set_text_wrapping(TextWrapping::word);
    context->host_services_status->set_vertical_alignment(VerticalAlignment::near);
    services_group->add_at(context->host_services_status,
                           {24.0, 142.0, 872.0, 56.0});
}

} // namespace

ShowcaseTree build_showcase_tree() {
    auto context = std::make_shared<ShowcaseContext>();
    auto root = make_control<ScaledPanel>(StableId("showcase.root"),
                                          Size{1280.0, 820.0});
    root->set_background(canvas);
    root->set_tag(context);

    auto header = make_control<Surface>(StableId("showcase.header"),
                                        Size{1280.0, 88.0});
    header->set_background(shell_blue);
    root->add_at(header, {0.0, 0.0, 1280.0, 88.0});
    auto header_light = make_control<Panel>(StableId("showcase.header.light-rule"));
    header_light->set_background(shell_blue_light);
    header->add_at(header_light, {0.0, 0.0, 1280.0, 4.0});
    auto header_dark = make_control<Panel>(StableId("showcase.header.dark-rule"));
    header_dark->set_background(shell_blue_dark);
    header->add_at(header_dark, {0.0, 86.0, 1280.0, 2.0});
    auto title = label("showcase.header.title", "GUI.FORMS COMPLETE SHOWCASE",
                       24.0, 700, Color::rgba(255, 255, 255));
    title->set_font({FontRole::control, 24.0, 700, false});
    header->add_at(title, {28.0, 18.0, 650.0, 34.0});
    auto subtitle = label("showcase.header.subtitle",
                          "Retained desktop controls · CPU rendering · Windows 7/10 professional lineage",
                          12.0, 400, Color::rgba(205, 224, 238));
    header->add_at(subtitle, {30.0, 52.0, 780.0, 24.0});
    auto build = label("showcase.header.build", "M11h · LIVE CONFORMANCE BOARD",
                       11.0, 700, Color::rgba(218, 237, 196));
    build->set_alignment(HorizontalAlignment::far);
    header->add_at(build, {930.0, 30.0, 310.0, 26.0});

    auto sidebar = make_control<Surface>(StableId("showcase.sidebar"),
                                         Size{238.0, 694.0});
    sidebar->set_background(Color::rgba(220, 229, 237));
    root->add_at(sidebar, {0.0, 88.0, 238.0, 694.0});
    auto sidebar_rule = make_control<Panel>(StableId("showcase.sidebar.rule"));
    sidebar_rule->set_background(rule);
    sidebar->add_at(sidebar_rule, {237.0, 0.0, 1.0, 694.0});
    auto nav_title = label("showcase.sidebar.title", "SHOWCASE INDEX", 12.0, 700,
                           shell_blue_dark);
    sidebar->add_at(nav_title, {20.0, 22.0, 190.0, 24.0});
    const std::array<std::string_view, 16> page_names{
        "01  Controls", "02  Ranges", "03  Containers",
        "04  Animation", "05  States", "06  Text & Input",
        "07  Collections", "08  Values", "09  Images & Drawing",
        "10  Tabs & Pages", "11  Checked Lists", "12  Timing & ToolTips",
        "13  Dates & Calendar", "14  Dialogs & Services",
        "15  Layout Panels", "16  Dock & Anchor"};
    for (std::size_t index = 0; index < page_names.size(); ++index) {
        auto navigation = make_control<Button>(
            StableId("showcase.navigation." + std::to_string(index)),
            std::string(page_names[index]));
        navigation->set_font({FontRole::control, 12.0, 400, false, 0.24});
        navigation->set_visual_style(ButtonVisualStyle::command);
        navigation->set_style(showcase_style());
        sidebar->add_at(navigation, {16.0, 54.0 + static_cast<double>(index) * 33.0,
                                     205.0, 30.0});
        context->navigation.push_back(navigation);
    }
    auto proof = label("showcase.sidebar.proof",
                       "No browser · No GPU · Retained core · Public controls",
                       11.0, 600, muted);
    proof->set_text_wrapping(TextWrapping::word);
    proof->set_vertical_alignment(VerticalAlignment::near);
    sidebar->add_at(proof, {20.0, 606.0, 196.0, 38.0});
    auto live = make_control<CheckBox>(StableId("showcase.sidebar.live"),
                                       "Live animation");
    live->set_checked(true);
    live->set_indicator_style(ChoiceIndicatorStyle::toggle);
    sidebar->add_at(live, {20.0, 662.0, 190.0, 28.0});

    auto content = make_control<Surface>(StableId("showcase.content"),
                                         Size{1042.0, 650.0});
    content->set_background(canvas);
    root->add_at(content, {238.0, 88.0, 1042.0, 650.0});
    for (std::size_t index = 0; index < page_names.size(); ++index) {
        auto page = make_control<Surface>(
            StableId("showcase.page." + std::to_string(index)),
            Size{1000.0, 660.0});
        page->set_background(canvas);
        page->set_visible(index == 0U);
        content->add_at(page, {20.0, 0.0, 1000.0, 650.0});
        context->pages.push_back(page);
    }
    add_control_spectrum(std::static_pointer_cast<Surface>(context->pages[0]), context);
    add_ranges(std::static_pointer_cast<Surface>(context->pages[1]), context);
    add_containers(std::static_pointer_cast<Surface>(context->pages[2]), context);
    add_animation(std::static_pointer_cast<Surface>(context->pages[3]), context);
    add_states(std::static_pointer_cast<Surface>(context->pages[4]), context);
    add_text_input(std::static_pointer_cast<Surface>(context->pages[5]), context);
    add_collections(std::static_pointer_cast<Surface>(context->pages[6]), context);
    add_values(std::static_pointer_cast<Surface>(context->pages[7]), context);
    add_images_and_drawing(std::static_pointer_cast<Surface>(context->pages[8]),
                           context);
    add_tabs_and_pages(std::static_pointer_cast<Surface>(context->pages[9]),
                       context);
    add_checked_collections(std::static_pointer_cast<Surface>(context->pages[10]),
                            context);
    add_timing_and_tooltips(std::static_pointer_cast<Surface>(context->pages[11]),
                            context);
    add_dates_and_calendar(std::static_pointer_cast<Surface>(context->pages[12]),
                           context);
    add_dialogs_and_host_services(
        std::static_pointer_cast<Surface>(context->pages[13]), context);
    add_layout_panels(std::static_pointer_cast<Surface>(context->pages[14]),
                      context);
    add_dock_and_anchor(std::static_pointer_cast<Surface>(context->pages[15]),
                        context);

    auto status_surface = make_control<Surface>(StableId("showcase.status"),
                                                Size{1042.0, 44.0});
    status_surface->set_background(Color::rgba(225, 232, 238));
    root->add_at(status_surface, {238.0, 738.0, 1042.0, 44.0});
    auto status_rule = make_control<Panel>(StableId("showcase.status.rule"));
    status_rule->set_background(rule);
    status_surface->add_at(status_rule, {0.0, 0.0, 1042.0, 1.0});
    context->status = label("showcase.status.text", "", 11.0, 600, shell_blue_dark);
    status_surface->add_at(context->status, {18.0, 10.0, 980.0, 24.0});

    for (std::size_t index = 0; index < context->navigation.size(); ++index) {
        const std::weak_ptr<ShowcaseContext> weak_context = context;
        context->subscriptions.push_back(context->navigation[index]->clicked().subscribe(
            *root, [weak_context, index](ButtonBase&) {
                if (const auto context = weak_context.lock()) {
                    context->select_page(index);
                }
            }));
    }
    const std::weak_ptr<ShowcaseContext> weak_context = context;
    context->subscriptions.push_back(live->checked_changed().subscribe(
        *root, [weak_context](bool enabled) {
            const auto context = weak_context.lock();
            if (!context) {
                return;
            }
            context->live_motion = enabled;
            context->apply_motion_policy();
        }));
    context->apply_motion_policy();
    context->select_page(0U);
    return {root};
}

void initialize_showcase_runtime(Window& window) {
    const auto root = std::dynamic_pointer_cast<ScaledPanel>(window.root());
    if (!root) throw std::logic_error("showcase runtime requires its retained root");
    const auto* retained_context =
        std::any_cast<std::shared_ptr<ShowcaseContext>>(&root->tag());
    if (retained_context == nullptr || !*retained_context) {
        throw std::logic_error("showcase runtime requires a retained context tag");
    }
    const auto context = *retained_context;
    if (context->ui_timer) return;

    context->ui_timer = std::make_shared<Timer>(window, 250ms);
    context->tooltips = std::make_shared<ToolTip>(window);
    context->components.add(context->ui_timer);
    context->components.add(context->tooltips);
    context->tooltips->set_initial_delay(420ms);
    context->tooltips->set_reshow_delay(90ms);
    context->tooltips->set_auto_pop_delay(4s);
    context->tooltips->set_show_always(true);

    const auto hover = window.find("showcase.timing.tooltip.hover");
    const auto keyboard = window.find("showcase.timing.tooltip.focus");
    const auto persistent = std::dynamic_pointer_cast<Button>(
        window.find("showcase.timing.tooltip.persistent"));
    context->tooltips->set_tool_tip(
        hover, "Retained hover help is delayed, cancellable, clipped to the client, and input-transparent.");
    context->tooltips->set_tool_tip(
        keyboard, "Keyboard focus exposes the same factual help without changing accessible names.");
    context->tooltips->set_tool_tip(
        persistent, "Click to hold this tooltip open until another interaction dismisses it.");
    context->tooltips->set_tool_tip(
        context->tooltip_disabled_target,
        "This action is disabled because no compatible device is selected.");
    context->tooltips->set_tool_tip(
        context->timer_motion_target,
        "The overlay re-anchors when this retained target moves on UI Timer ticks.");
    for (const auto& navigation : context->navigation) {
        context->tooltips->set_tool_tip(
            navigation, "Open " + navigation->text() +
                            " in the independent GUI.Forms conformance board.");
    }

    const std::weak_ptr<ShowcaseContext> weak = context;
    context->subscriptions.push_back(context->ui_timer->tick().subscribe(
        *window.root(), [weak] {
            const auto context = weak.lock();
            if (!context) return;
            ++context->timer_ticks;
            const double cycle = static_cast<double>(context->timer_ticks % 20U) / 19.0;
            context->timer_progress->set_value(cycle * 100.0);
            constexpr double travel = 98.0;
            const double eased =
                0.5 - std::cos(cycle * 6.283185307179586) * 0.5;
            context->timer_motion->set_design_bounds(
                *context->timer_motion_target,
                {14.0 + travel * eased, 39.0, 168.0, 34.0});
            context->timer_status->set_text(
                "Running · tick " + std::to_string(context->timer_ticks) +
                " · " + std::to_string(
                    static_cast<int>(context->ui_timer->interval().count())) +
                " ms · UI thread");
        }));
    context->subscriptions.push_back(context->timer_interval->value_changed().subscribe(
        *window.root(), [weak](double value) {
            if (const auto context = weak.lock()) {
                context->ui_timer->set_interval(
                    std::chrono::milliseconds(static_cast<int>(std::round(value))));
            }
        }));
    context->subscriptions.push_back(context->timer_start->clicked().subscribe(
        *window.root(), [weak](ButtonBase&) {
            if (const auto context = weak.lock()) {
                context->ui_timer->start();
                context->timer_status->set_text(
                    "Running · deadline armed on the UI queue");
            }
        }));
    context->subscriptions.push_back(context->timer_stop->clicked().subscribe(
        *window.root(), [weak](ButtonBase&) {
            if (const auto context = weak.lock()) {
                context->ui_timer->stop();
                context->timer_status->set_text("Stopped · zero idle wakeups");
            }
        }));
    context->subscriptions.push_back(persistent->clicked().subscribe(
        *window.root(), [weak, persistent](ButtonBase&) {
            if (const auto context = weak.lock()) {
                context->tooltips->show(persistent, 0ms);
            }
        }));
    context->subscriptions.push_back(context->tooltip_show_disabled->clicked().subscribe(
        *window.root(), [weak](ButtonBase&) {
            if (const auto context = weak.lock()) {
                context->tooltips->show(context->tooltip_disabled_target, 0ms);
            }
        }));
    context->select_page(context->selected_page);
}

} // namespace gui_forms::showcase
